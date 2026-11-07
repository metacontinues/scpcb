// model.c: mdl (.b3d / .x) -> <id>.glb + <id>.txt. right-handed on the way out
// (z flipped, winding flipped). a model w/ keys or bones becomes a skinned glb:
// every node is a joint (names kept, so attach points are bones), BONE weights
// skin their vertices and every other vertex follows its own node w/ weight 1,
// so rigid parts animate the same way. keys land at frame / 60 s, which is
// exactly raylib's glTF sampling rate, so raylib keyframe n = blitz frame n. a
// model w/o either is one flat mesh in its rest pose (what LoadMesh made).
//
// the .txt sidecar:
//   frames n                     anim length in frames (skinned models)
//   mat i r g b a flags blend tex  gltf material i (raylib: model.materials[i +
//   1]), color/alpha 0..1,
//                                  flags: f fullbright, v vertex color, a
//                                  vertex alpha, n no fog, 2 two-sided
//                                  ('-' none), blend: alpha|mul|add, the
//                                  texture's id or '-'
#define _POSIX_C_SOURCE 200809L
#include "bake.h"
#include "mdl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void qrows(float w, float x, float y, float z, float r[3][3]); // b3d.c

typedef float m4[4][4]; // rows, column vectors

static void m4mul(const m4 a, const m4 b, m4 o) {
  m4 t;
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++) {
      t[r][c] = 0;
      for (int k = 0; k < 4; k++)
        t[r][c] += a[r][k] * b[k][c];
    }
  memcpy(o, t, sizeof t);
}
static void m4inv(const m4 a, m4 o) { // affine
  float d = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
            a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
            a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
  m4 t = {{0}};
  if (fabsf(d) < 1e-20f)
    d = 1e-20f;
  t[0][0] = (a[1][1] * a[2][2] - a[1][2] * a[2][1]) / d;
  t[0][1] = (a[0][2] * a[2][1] - a[0][1] * a[2][2]) / d;
  t[0][2] = (a[0][1] * a[1][2] - a[0][2] * a[1][1]) / d;
  t[1][0] = (a[1][2] * a[2][0] - a[1][0] * a[2][2]) / d;
  t[1][1] = (a[0][0] * a[2][2] - a[0][2] * a[2][0]) / d;
  t[1][2] = (a[0][2] * a[1][0] - a[0][0] * a[1][2]) / d;
  t[2][0] = (a[1][0] * a[2][1] - a[1][1] * a[2][0]) / d;
  t[2][1] = (a[0][1] * a[2][0] - a[0][0] * a[2][1]) / d;
  t[2][2] = (a[0][0] * a[1][1] - a[0][1] * a[1][0]) / d;
  for (int r = 0; r < 3; r++)
    t[r][3] = -(t[r][0] * a[0][3] + t[r][1] * a[1][3] + t[r][2] * a[2][3]);
  t[3][3] = 1;
  memcpy(o, t, sizeof t);
}
static void flip(m4 a) {
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      if ((r == 2) != (c == 2))
        a[r][c] = -a[r][c];
} // F a F
static void local(const mnode *n, m4 o) {
  m4 t = {{0}};
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++)
      t[r][c] = n->rot[r][c] * n->scl[c];
    t[r][3] = n->pos[r];
  }
  t[3][3] = 1;
  memcpy(o, t, sizeof t);
}
static void quat(float r[3][3], float *q) { // rows -> x y z w
  float tr = r[0][0] + r[1][1] + r[2][2], s;
  if (tr > 0) {
    s = sqrtf(tr + 1) * 2;
    q[3] = s / 4;
    q[0] = (r[2][1] - r[1][2]) / s;
    q[1] = (r[0][2] - r[2][0]) / s;
    q[2] = (r[1][0] - r[0][1]) / s;
  } else if (r[0][0] > r[1][1] && r[0][0] > r[2][2]) {
    s = sqrtf(1 + r[0][0] - r[1][1] - r[2][2]) * 2;
    q[3] = (r[2][1] - r[1][2]) / s;
    q[0] = s / 4;
    q[1] = (r[0][1] + r[1][0]) / s;
    q[2] = (r[0][2] + r[2][0]) / s;
  } else if (r[1][1] > r[2][2]) {
    s = sqrtf(1 + r[1][1] - r[0][0] - r[2][2]) * 2;
    q[3] = (r[0][2] - r[2][0]) / s;
    q[0] = (r[0][1] + r[1][0]) / s;
    q[1] = s / 4;
    q[2] = (r[1][2] + r[2][1]) / s;
  } else {
    s = sqrtf(1 + r[2][2] - r[0][0] - r[1][1]) * 2;
    q[3] = (r[1][0] - r[0][1]) / s;
    q[0] = (r[0][2] + r[2][0]) / s;
    q[1] = (r[1][2] + r[2][1]) / s;
    q[2] = s / 4;
  }
  float l = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  for (int i = 0; i < 4; i++)
    q[i] /= l > 0 ? l : 1;
}
static void rflip(float r[3][3]) {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if ((i == 2) != (j == 2))
        r[i][j] = -r[i][j];
}
static void ortho(float r[3][3]) { // blitz re-orthonormalizes key rotations too
  for (int c = 0; c < 3; c++) {
    for (int p = 0; p < c; p++) {
      float d = 0;
      for (int i = 0; i < 3; i++)
        d += r[i][c] * r[i][p];
      for (int i = 0; i < 3; i++)
        r[i][c] -= d * r[i][p];
    }
    float l = sqrtf(r[0][c] * r[0][c] + r[1][c] * r[1][c] + r[2][c] * r[2][c]);
    for (int i = 0; i < 3; i++)
      r[i][c] /= l > 0 ? l : 1;
  }
}

// ---- output buffers, one group per brush, chunked at u16
typedef struct {
  float p[3], n[3], uv[2][2];
  uint8_t c[4], j[4];
  float w[4];
} ovtx;
typedef struct {
  ovtx *v;
  int nv, cv;
  uint16_t *ix;
  int ni, ci;
} chunk;
typedef struct {
  int brush;
  chunk *ch;
  int nch;
} group;

#define GROW(p, n, c)                                                          \
  do {                                                                         \
    if ((n) >= (c)) {                                                          \
      (c) = (c) ? (c) * 2 : 256;                                               \
      (p) = realloc((p), sizeof *(p) * (c));                                   \
    }                                                                          \
  } while (0)

static group *grp(group **gs, int *ngs, int brush) {
  for (int i = 0; i < *ngs; i++)
    if ((*gs)[i].brush == brush)
      return &(*gs)[i];
  *gs = realloc(*gs, sizeof **gs * (*ngs + 1));
  (*gs)[*ngs] = (group){brush, NULL, 0};
  return &(*gs)[(*ngs)++];
}

static const char *texid(const mdl *m, int ti, char *o, int on) {
  if (ti < 0 || ti >= m->nt)
    return NULL;
  const mtex *t = &m->t[ti];
  const char *bn = t->name;
  for (const char *q = t->name; *q; q++)
    if (*q == '/' || *q == '\\')
      bn = q + 1;
  char p[2048];
  int mode = (t->flags & 2 ? IM_ALPHA : 0) | (t->flags & 4 ? IM_MASK : 0);
  const char *r;
  snprintf(p, sizeof p, "%s%s", m->dir, t->name);
  if (!(r = bake_tex(p, mode))) {
    snprintf(p, sizeof p, "%s%s", m->dir, bn);
    r = bake_tex(p, mode);
  } // exporters' abs paths
  if (!r)
    r = bake_tex(t->name, mode);
  if (!r)
    return NULL;
  snprintf(o, on, "%s", r);
  return o;
}

int bake_model(const char *path) {
  char id[1024], out[4096];
  mkid(path, id, sizeof id);
  snprintf(out, sizeof out, OUT "%s.txt", id);
  if (!stale(path, out))
    return 1;
  mdl m;
  size_t pl = strlen(path);
  int ok = pl > 2 && !strcasecmp(path + pl - 2, ".x") ? x_read(path, &m)
                                                      : b3d_read(path, &m);
  if (!ok) {
    warn("can't read %s", path);
    mdl_free(&m);
    return 0;
  }

  int skinned = 0;
  for (int i = 0; i < m.nn; i++)
    if (m.n[i].nk || m.n[i].nw)
      skinned = 1;
  if (skinned && m.nn > 255) {
    warn("%s: %d nodes, more than u8 joints", path, m.nn);
    skinned = 0;
  }

  // rest pose: world matrices, right-handed
  m4 *W = malloc(sizeof(m4) * m.nn);
  for (int i = 0; i < m.nn; i++) {
    m4 l;
    local(&m.n[i], l);
    if (m.n[i].parent >= 0)
      m4mul(W[m.n[i].parent], l, W[i]);
    else
      memcpy(W[i], l, sizeof l);
  }
  for (int i = 0; i < m.nn; i++)
    flip(W[i]);

  // per-vertex joints/weights: bones first, top 4, the rest follow their own
  // node
  uint8_t **J = calloc(m.nn, sizeof *J);
  float **Wt = calloc(m.nn, sizeof *Wt);
  if (skinned) {
    for (int i = 0; i < m.nn; i++) {
      J[i] = calloc(m.n[i].nv * 4 + 4, 1);
      Wt[i] = calloc(m.n[i].nv * 4 + 4, sizeof(float));
    }
    for (int b = 0; b < m.nn; b++)
      for (int k = 0; k < m.n[b].nw; k++) {
        mweight w = m.n[b].w[k];
        if (w.node < 0 || w.v < 0 || w.v >= m.n[w.node].nv || w.w <= 0)
          continue;
        float *ww = Wt[w.node] + w.v * 4;
        uint8_t *jj = J[w.node] + w.v * 4;
        int s = 0;
        for (int q = 1; q < 4; q++)
          if (ww[q] < ww[s])
            s = q;
        if (w.w > ww[s]) {
          ww[s] = w.w;
          jj[s] = b;
        }
      }
    for (int i = 0; i < m.nn; i++)
      for (int v = 0; v < m.n[i].nv; v++) {
        float *ww = Wt[i] + v * 4, t = ww[0] + ww[1] + ww[2] + ww[3];
        if (t <= 0) {
          ww[0] = 1;
          J[i][v * 4] = i;
        } else
          for (int q = 0; q < 4; q++)
            ww[q] /= t;
      }
  }

  // flatten every surface into its brush's group
  group *gs = NULL;
  int ngs = 0;
  for (int i = 0; i < m.nn; i++) {
    mnode *n = &m.n[i];
    if (!n->nv)
      continue;
    float (*nr)[3] = calloc(n->nv, sizeof *nr);
    if (n->nrm)
      for (int v = 0; v < n->nv; v++)
        memcpy(nr[v], n->v[v].n, 12);
    else
      for (int s = 0; s < n->ns; s++)
        for (int t = 0; t < n->s[s].ntri; t++) { // smooth by index
          int *x = n->s[s].tri + t * 3;
          float *a = n->v[x[0]].p, *b = n->v[x[1]].p, *c = n->v[x[2]].p, e1[3],
                e2[3], fn[3];
          for (int k = 0; k < 3; k++) {
            e1[k] = b[k] - a[k];
            e2[k] = c[k] - a[k];
          }
          fn[0] = e1[1] * e2[2] - e1[2] * e2[1];
          fn[1] = e1[2] * e2[0] - e1[0] * e2[2];
          fn[2] = e1[0] * e2[1] - e1[1] * e2[0];
          for (int k = 0; k < 3; k++)
            for (int q = 0; q < 3; q++)
              nr[x[k]][q] += fn[q]; // lh cw: this points out
        }
    m4 ni;
    m4inv(W[i], ni); // normals: inverse transpose
    int *map = malloc(sizeof(int) * n->nv);
    for (int s = 0; s < n->ns; s++) {
      msurf *sf = &n->s[s];
      if (!sf->ntri)
        continue;
      group *g =
          grp(&gs, &ngs, sf->brush >= 0 && sf->brush < m.nb ? sf->brush : -1);
      int used = 0;
      for (int v = 0; v < n->nv; v++)
        map[v] = -1;
      for (int t = 0; t < sf->ntri * 3; t++)
        if (map[sf->tri[t]] < 0)
          map[sf->tri[t]] = used++;
      if (used > 65535) {
        warn("%s: surface w/ %d verts, dropped", path, used);
        continue;
      }
      if (!g->nch || g->ch[g->nch - 1].nv + used > 65535) {
        g->ch = realloc(g->ch, sizeof *g->ch * (g->nch + 1));
        g->ch[g->nch++] = (chunk){0};
      }
      chunk *c = &g->ch[g->nch - 1];
      int base = c->nv;
      for (int v = 0; v < n->nv; v++)
        map[v] = -1;
      for (int t = 0; t < sf->ntri * 3; t++) {
        int vi = sf->tri[t];
        if (map[vi] >= 0)
          continue;
        map[vi] = c->nv;
        mvtx *s0 = &n->v[vi];
        ovtx o = {0};
        float lp[3] = {s0->p[0], s0->p[1], -s0->p[2]},
              ln[3] = {nr[vi][0], nr[vi][1], -nr[vi][2]};
        for (int r = 0; r < 3; r++) {
          o.p[r] = W[i][r][0] * lp[0] + W[i][r][1] * lp[1] +
                   W[i][r][2] * lp[2] + W[i][r][3];
          o.n[r] = ni[0][r] * ln[0] + ni[1][r] * ln[1] + ni[2][r] * ln[2];
        }
        float l = sqrtf(o.n[0] * o.n[0] + o.n[1] * o.n[1] + o.n[2] * o.n[2]);
        for (int r = 0; r < 3; r++)
          o.n[r] = l > 0 ? o.n[r] / l : (r == 1);
        memcpy(o.uv, s0->uv, sizeof o.uv);
        for (int k = 0; k < 4; k++) {
          float x = s0->c[k] * 255 + .5f;
          o.c[k] = x < 0 ? 0 : x > 255 ? 255 : (uint8_t)x;
        }
        if (skinned) {
          memcpy(o.j, J[i] + vi * 4, 4);
          memcpy(o.w, Wt[i] + vi * 4, 16);
        }
        GROW(c->v, c->nv, c->cv);
        c->v[c->nv++] = o;
      }
      for (int t = 0; t < sf->ntri; t++) {
        int *x = sf->tri + t * 3,
            w[3] = {x[0], x[2], x[1]}; // mirrored -> flip winding
        for (int k = 0; k < 3; k++) {
          GROW(c->ix, c->ni, c->ci);
          c->ix[c->ni++] = map[w[k]];
        }
      }
      (void)base;
    }
    free(map);
    free(nr);
  }

  glb b = {0};
  sb txt = {0}, j = {0};
  sbf(&txt, "# %s\n", path);
  if (skinned)
    sbf(&txt, "frames %d\n", m.frames);
  int np = 0;
  for (int i = 0; i < ngs; i++)
    np += gs[i].nch;
  gbprim *ps = calloc(np ? np : 1, sizeof *ps);
  np = 0;
  for (int i = 0; i < ngs; i++) {
    char nm[16], tb[1024];
    snprintf(nm, sizeof nm, "b%d", gs[i].brush);
    int mat = gb_mat(&b, nm);
    mbrush br = gs[i].brush >= 0 ? m.b[gs[i].brush]
                                 : (mbrush){{1, 1, 1, 1}, 0, 0, {0}, 0};
    const char *t = br.ntex ? texid(&m, br.tex[0], tb, sizeof tb) : NULL;
    if (br.ntex && !t && br.tex[0] >= 0 && br.tex[0] < m.nt)
      warn("%s: texture %s missing", path, m.t[br.tex[0]].name);
    char fl[8] = "", *f = fl;
    if (br.fx & 1)
      *f++ = 'f';
    if (br.fx & 2)
      *f++ = 'v';
    if (br.fx & 32)
      *f++ = 'a';
    if (br.fx & 8)
      *f++ = 'n';
    if (br.fx & 16)
      *f++ = '2';
    if (f == fl)
      *f++ = '-';
    *f = 0;
    sbf(&txt, "mat %d %g %g %g %g %s %s %s\n", mat, br.col[0], br.col[1],
        br.col[2], br.col[3], fl,
        br.blend == 2   ? "mul"
        : br.blend == 3 ? "add"
                        : "alpha",
        t ? t : "-");
    for (int c = 0; c < gs[i].nch; c++) {
      chunk *ch = &gs[i].ch[c];
      int nv = ch->nv;
      size_t k = nv ? nv : 1;
      float *p = malloc(k * 12), *nn = malloc(k * 12), *u0 = malloc(k * 8),
            *u1 = malloc(k * 8), *wt = malloc(k * 16);
      uint8_t *cl = malloc(k * 4), *jt = malloc(k * 4);
      for (int v = 0; v < nv; v++) {
        memcpy(p + v * 3, ch->v[v].p, 12);
        memcpy(nn + v * 3, ch->v[v].n, 12);
        memcpy(u0 + v * 2, ch->v[v].uv[0], 8);
        memcpy(u1 + v * 2, ch->v[v].uv[1], 8);
        memcpy(cl + v * 4, ch->v[v].c, 4);
        memcpy(jt + v * 4, ch->v[v].j, 4);
        memcpy(wt + v * 4, ch->v[v].w, 16);
      }
      gbprim q = gb_prim0();
      q.pos = gb_vec3(&b, p, nv);
      q.nrm = gb_vec3(&b, nn, nv);
      q.uv0 = gb_vec2(&b, u0, nv);
      if (m.uv1)
        q.uv1 = gb_vec2(&b, u1, nv);
      q.col = gb_rgba8(&b, cl, nv);
      if (skinned) {
        q.jnt = gb_u8x4(&b, jt, nv);
        q.wgt = gb_vec4(&b, wt, nv);
      }
      q.idx = gb_idx16(&b, ch->ix, ch->ni);
      q.mat = mat;
      ps[np++] = q;
      free(p);
      free(nn);
      free(u0);
      free(u1);
      free(wt);
      free(cl);
      free(jt);
    }
  }
  int mesh = np ? gb_mesh(&b, ps, np) : -1;
  free(ps);

  if (skinned) {
    // joints = nodes 0..nn-1 (same order), local trs
    for (int i = 0; i < m.nn; i++) {
      mnode *n = &m.n[i];
      float r[3][3], q[4];
      memcpy(r, n->rot, sizeof r);
      rflip(r);
      quat(r, q);
      char nm[64];
      int k = 0;
      for (const char *c = n->name; *c && k < 63; c++)
        nm[k++] = *c == '"' || *c == '\\' || (unsigned char)*c < 32 ? '_' : *c;
      nm[k] = 0;
      j.n = 0;
      sbf(&j,
          "\"name\":\"%s\",\"translation\":[%.9g,%.9g,%.9g],\"rotation\":[%.9g,"
          "%.9g,%.9g,%.9g],\"scale\":[%.9g,%.9g,%.9g]",
          nm, n->pos[0], n->pos[1], -n->pos[2], q[0], q[1], q[2], q[3],
          n->scl[0], n->scl[1], n->scl[2]);
      int nc = 0;
      for (int c = 0; c < m.nn; c++)
        if (m.n[c].parent == i)
          sbf(&j, "%s%d", nc++ ? "," : ",\"children\":[", c);
      if (nc)
        sbf(&j, "]");
      gb_node(&b, n->parent < 0, j.s);
    }
    float *ib = malloc(sizeof(float) * 16 * m.nn);
    for (int i = 0; i < m.nn; i++) {
      m4 inv;
      m4inv(W[i], inv);
      for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
          ib[i * 16 + c * 4 + r] = inv[r][c];
    }
    j.n = 0;
    sbf(&j, "\"inverseBindMatrices\":%d,\"joints\":[", gb_mat4(&b, ib, m.nn));
    for (int i = 0; i < m.nn; i++)
      sbf(&j, "%s%d", i ? "," : "", i);
    sbf(&j, "]");
    gb_skin(&b, j.s);
    free(ib);
    // one animation, a sampler per (node, channel)
    sb ch = {0}, sm = {0};
    int ns = 0;
    for (int i = 0; i < m.nn; i++)
      for (int c = 0; c < 3; c++) {
        mnode *n = &m.n[i];
        int bit = c == 0 ? 1 : c == 1 ? 4 : 2, cnt = 0;
        for (int k = 0; k < n->nk; k++)
          if (n->k[k].has & bit)
            cnt++;
        if (!cnt)
          continue;
        float *tm = malloc(sizeof(float) * cnt),
              *v = malloc(sizeof(float) * 4 * cnt), prev[4] = {0, 0, 0, 1};
        int o = 0;
        for (int k = 0; k < n->nk; k++) {
          mkey *x = &n->k[k];
          if (!(x->has & bit))
            continue;
          tm[o] = x->t / 60.0f;
          if (c == 0) {
            v[o * 3] = x->p[0];
            v[o * 3 + 1] = x->p[1];
            v[o * 3 + 2] = -x->p[2];
          } else if (c == 2)
            memcpy(v + o * 3, x->s, 12);
          else {
            float r[3][3], q[4];
            qrows(x->q[0], x->q[1], x->q[2], x->q[3], r);
            ortho(r);
            rflip(r);
            quat(r, q);
            if (q[0] * prev[0] + q[1] * prev[1] + q[2] * prev[2] +
                    q[3] * prev[3] <
                0)
              for (int e = 0; e < 4; e++)
                q[e] = -q[e];
            memcpy(prev, q, 16);
            memcpy(v + o * 4, q, 16);
          }
          o++;
        }
        int in = gb_times(&b, tm, cnt);
        int outa = c == 1 ? gb_acc(&b, gb_view(&b, v, cnt * 16, 0), GB_F32, cnt,
                                   "VEC4", 0, 0, 0, 0)
                          : gb_acc(&b, gb_view(&b, v, cnt * 12, 0), GB_F32, cnt,
                                   "VEC3", 0, 0, 0, 0);
        sbf(&sm, "%s{\"input\":%d,\"output\":%d,\"interpolation\":\"LINEAR\"}",
            ns ? "," : "", in, outa);
        sbf(&ch, "%s{\"sampler\":%d,\"target\":{\"node\":%d,\"path\":\"%s\"}}",
            ns ? "," : "", ns, i,
            c == 0   ? "translation"
            : c == 1 ? "rotation"
                     : "scale");
        ns++;
        free(tm);
        free(v);
      }
    if (ns) {
      j.n = 0;
      sbf(&j, "\"name\":\"all\",\"channels\":[%s],\"samplers\":[%s]", ch.s,
          sm.s);
      gb_anim(&b, j.s);
    }
    free(ch.s);
    free(sm.s);
  }
  if (mesh >= 0) {
    j.n = 0;
    sbf(&j, "\"mesh\":%d%s", mesh, skinned ? ",\"skin\":0" : "");
    gb_node(&b, 1, j.s);
  } else
    warn("%s: no geometry", path);

  char gp[4096];
  snprintf(gp, sizeof gp, OUT "%s.glb", id);
  ok = gb_write(&b, gp);
  if (!ok)
    warn("can't write %s", gp);
  if (ok) {
    FILE *o = fopen(out, "w");
    if (!o || fwrite(txt.s, 1, txt.n, o) != txt.n) {
      warn("can't write %s", out);
      ok = 0;
    }
    if (o)
      fclose(o);
  }
  for (int i = 0; i < ngs; i++) {
    for (int c = 0; c < gs[i].nch; c++) {
      free(gs[i].ch[c].v);
      free(gs[i].ch[c].ix);
    }
    free(gs[i].ch);
  }
  for (int i = 0; i < m.nn; i++) {
    free(J[i]);
    free(Wt[i]);
  }
  free(gs);
  free(J);
  free(Wt);
  free(W);
  free(txt.s);
  free(j.s);
  mdl_free(&m);
  return ok;
}
