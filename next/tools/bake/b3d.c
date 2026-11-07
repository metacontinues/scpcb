// b3d.c: .b3d -> mdl. chunks: TEXS BRUS NODE(MESH(VRTS TRIS) KEYS ANIM BONE
// NODE...)
#include "bake.h"
#include "mdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PUSH(a, n, v)                                                          \
  do {                                                                         \
    (a) = realloc((a), sizeof *(a) * ((n) + 1));                               \
    (a)[(n)++] = (v);                                                          \
  } while (0)

static FILE *f;
static long ends[64];
static int d;

static int ri(void) {
  int v = 0;
  if (fread(&v, 4, 1, f) != 1)
    v = 0;
  return v;
}
static float rf(void) {
  float v = 0;
  if (fread(&v, 4, 1, f) != 1)
    v = 0;
  return v;
}
static void cstr(char *o, int n) {
  int c, i = 0;
  while ((c = fgetc(f)) > 0)
    if (i < n - 1)
      o[i++] = c;
  o[i] = 0;
}
static int more(void) { return ftell(f) < ends[d]; }
static int tag(char *t) {
  if (!more() || d >= 63 || fread(t, 1, 4, f) != 4)
    return 0;
  t[4] = 0;
  int n = ri();
  ends[d + 1] = ftell(f) + n;
  if (ends[d + 1] > ends[d])
    ends[d + 1] = ends[d];
  d++;
  return 1;
}
static void leave(void) { fseek(f, ends[d--], SEEK_SET); }

// blitz's quaternion -> matrix (b3d stores the conjugate of the usual
// convention), rows
void qrows(float w, float x, float y, float z, float r[3][3]) {
  float xx = x * x, yy = y * y, zz = z * z, xy = x * y, xz = x * z, yz = y * z,
        wx = w * x, wy = w * y, wz = w * z;
  float m[3][3] = {{1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy)},
                   {2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx)},
                   {2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)}};
  memcpy(r, m, sizeof m);
}

int mdl_node(mdl *m, const char *name, int parent) {
  mnode n = {.parent = parent,
             .scl = {1, 1, 1},
             .rot = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
  snprintf(n.name, sizeof n.name, "%s", name);
  PUSH(m->n, m->nn, n);
  return m->nn - 1;
}

static void mesh(mdl *m, int ni) {
  int mbr = ri();
  char t[5];
  while (tag(t)) {
    mnode *n = &m->n[ni];
    if (!strcmp(t, "VRTS")) {
      int fl = ri(), sets = ri(), sz = ri();
      n->nrm = fl & 1;
      if (sets > 1)
        m->uv1 = 1;
      while (more()) {
        mvtx v = {.c = {1, 1, 1, 1}};
        for (int i = 0; i < 3; i++)
          v.p[i] = rf();
        if (fl & 1)
          for (int i = 0; i < 3; i++)
            v.n[i] = rf();
        if (fl & 2)
          for (int i = 0; i < 4; i++)
            v.c[i] = rf();
        for (int s = 0; s < sets; s++)
          for (int k = 0; k < sz; k++) {
            float x = rf();
            if (s < 2 && k < 2)
              v.uv[s][k] = x;
          }
        PUSH(n->v, n->nv, v);
      }
    } else if (!strcmp(t, "TRIS")) {
      msurf s = {ri(), NULL, 0};
      if (s.brush < 0)
        s.brush = mbr;
      int cap = 0;
      while (more()) {
        if (s.ntri + 1 > cap)
          s.tri = realloc(s.tri, sizeof(int) * 3 * (cap = cap ? cap * 2 : 64));
        for (int k = 0; k < 3; k++) {
          int v = ri();
          s.tri[s.ntri * 3 + k] = v >= 0 && v < n->nv ? v : 0;
        }
        s.ntri++;
      }
      PUSH(n->s, n->ns, s);
    }
    leave();
  }
}

static int
owner(mdl *m,
      int ni) { // a BONE's weights point into the nearest ancestor w/ a mesh
  for (int p = m->n[ni].parent; p >= 0; p = m->n[p].parent)
    if (m->n[p].nv)
      return p;
  return -1;
}

static void node(mdl *m, int parent) {
  char name[256];
  cstr(name, sizeof name);
  int ni = mdl_node(m, name, parent);
  mnode *n = &m->n[ni];
  for (int i = 0; i < 3; i++)
    n->pos[i] = rf();
  for (int i = 0; i < 3; i++)
    n->scl[i] = rf();
  float w = rf(), x = rf(), y = rf(), z = rf();
  qrows(w, x, y, z, n->rot);
  char t[5];
  while (tag(t)) {
    if (!strcmp(t, "MESH"))
      mesh(m, ni);
    else if (!strcmp(t, "NODE"))
      node(m, ni);
    else if (!strcmp(t, "ANIM")) {
      ri();
      int fr = ri();
      rf();
      if (fr > m->frames)
        m->frames = fr;
    } else if (!strcmp(t, "KEYS")) {
      int fl = ri();
      while (more()) {
        mkey k = {.t = ri(), .has = fl, .s = {1, 1, 1}, .q = {1, 0, 0, 0}};
        if (fl & 1)
          for (int i = 0; i < 3; i++)
            k.p[i] = rf();
        if (fl & 2)
          for (int i = 0; i < 3; i++)
            k.s[i] = rf();
        if (fl & 4)
          for (int i = 0; i < 4; i++)
            k.q[i] = rf();
        PUSH(m->n[ni].k, m->n[ni].nk, k);
      }
    } else if (!strcmp(t, "BONE")) {
      int o = owner(m, ni);
      while (more()) {
        mweight w = {o, 0, 0};
        w.v = ri();
        w.w = rf(); // sequenced: initializer order isn't
        if (o >= 0)
          PUSH(m->n[ni].w, m->n[ni].nw, w);
      }
    }
    leave();
  }
}

int b3d_read(const char *path, mdl *m) {
  *m = (mdl){0};
  if (!(f = fopen(path, "rb")))
    return 0;
  snprintf(m->dir, sizeof m->dir, "%s", path);
  char *sl = strrchr(m->dir, '/');
  sl ? (sl[1] = 0) : (m->dir[0] = 0);
  fseek(f, 0, SEEK_END);
  ends[0] = ftell(f);
  fseek(f, 0, SEEK_SET);
  d = 0;
  char t[5];
  int ok = tag(t) && !strcmp(t, "BB3D");
  if (ok) {
    ri();
    while (tag(t)) {
      if (!strcmp(t, "TEXS"))
        while (more()) {
          mtex x;
          cstr(x.name, sizeof x.name);
          x.flags = ri();
          x.blend = ri();
          x.px = rf();
          x.py = rf();
          x.sx = rf();
          x.sy = rf();
          rf();
          PUSH(m->t, m->nt, x);
        }
      else if (!strcmp(t, "BRUS")) {
        int nt = ri();
        while (more()) {
          char nm[256];
          cstr(nm, sizeof nm);
          mbrush b = {0};
          for (int i = 0; i < 4; i++)
            b.col[i] = rf();
          rf();
          b.blend = ri();
          b.fx = ri();
          for (int i = 0; i < nt; i++) {
            int id = ri();
            if (i < 8)
              b.tex[b.ntex++] = id;
          }
          PUSH(m->b, m->nb, b);
        }
      } else if (!strcmp(t, "NODE") && !m->nn)
        node(m, -1);
      leave();
    }
  }
  fclose(f);
  return ok && m->nn;
}

void mdl_free(mdl *m) {
  for (int i = 0; i < m->nn; i++) {
    for (int s = 0; s < m->n[i].ns; s++)
      free(m->n[i].s[s].tri);
    free(m->n[i].v);
    free(m->n[i].s);
    free(m->n[i].k);
    free(m->n[i].w);
  }
  free(m->n);
  free(m->t);
  free(m->b);
  *m = (mdl){0};
}
