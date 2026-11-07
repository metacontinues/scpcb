// x.c: directx .x -> mdl. text + binary, plus mszip ("bzip") if it's one block.
// one tokenizer over both encodings, one parser on top. frames -> nodes, each
// Mesh gets its own child node. skin weights / AnimationSet are skipped (none
// of the game's .x files animate).
#include "bake.h"
#include "mdl.h"
#include "raylib.h"
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define PUSH(a, n, v)                                                          \
  do {                                                                         \
    (a) = realloc((a), sizeof *(a) * ((n) + 1));                               \
    (a)[(n)++] = (v);                                                          \
  } while (0)

enum { T_EOF, T_NAME, T_STR, T_NUM, T_OB, T_CB };
typedef struct {
  int k;
  double n;
  char s[256];
} tok;

typedef struct {
  unsigned char *b;
  size_t n, i;
  int bin, f64, lk;
  size_t ln; // binary: pending list kind (6 int, 7 float) + count
  mdl *m;
  struct {
    char name[128];
    int brush;
  } *mats;
  int nmat;
} xr;

// ---- tokenizer
static int delim(int c) { return !c || isspace(c) || strchr(",;{}\"<", c); }
static uint32_t u32(xr *r) {
  uint32_t v = 0;
  if (r->i + 4 <= r->n)
    memcpy(&v, r->b + r->i, 4);
  r->i += 4;
  return v;
}
static uint16_t u16(xr *r) {
  uint16_t v = 0;
  if (r->i + 2 <= r->n)
    memcpy(&v, r->b + r->i, 2);
  r->i += 2;
  return v;
}

static tok bnext(xr *r) {
  tok t = {0};
  for (;;) {
    if (r->ln) {
      r->ln--;
      t.k = T_NUM;
      if (r->lk == 6)
        t.n = u32(r);
      else if (r->f64) {
        double d = 0;
        if (r->i + 8 <= r->n)
          memcpy(&d, r->b + r->i, 8);
        r->i += 8;
        t.n = d;
      } else {
        float f;
        uint32_t v = u32(r);
        memcpy(&f, &v, 4);
        t.n = f;
      }
      return t;
    }
    if (r->i + 2 > r->n)
      return t;
    int c = u16(r);
    switch (c) {
    case 1:
    case 2: {
      uint32_t l = u32(r);
      if (r->i + l > r->n)
        return t;
      snprintf(t.s, sizeof t.s, "%.*s", (int)l, r->b + r->i);
      r->i += l;
      if (c == 2)
        r->i += 2;
      t.k = c == 1 ? T_NAME : T_STR;
      return t;
    }
    case 3:
      t.k = T_NUM;
      t.n = (int)u32(r);
      return t;
    case 5:
      r->i += 16;
      break;
    case 6:
    case 7:
      r->lk = c;
      r->ln = u32(r);
      break;
    case 10:
      t.k = T_OB;
      return t;
    case 11:
      t.k = T_CB;
      return t;
    case 31:
      strcpy(t.s, "template");
      t.k = T_NAME;
      return t;
    default:
      if (c >= 40 && c <= 52) {
        t.k = T_NAME;
        strcpy(t.s, "_type");
        return t;
      }
      break;
    }
  }
}

static tok tnext(xr *r) {
  tok t = {0};
  for (;;) {
    while (r->i < r->n &&
           (isspace(r->b[r->i]) || r->b[r->i] == ',' || r->b[r->i] == ';'))
      r->i++;
    if (r->i >= r->n)
      return t;
    char *p = (char *)r->b + r->i;
    if (p[0] == '#' || (p[0] == '/' && p[1] == '/')) {
      while (r->i < r->n && r->b[r->i] != '\n')
        r->i++;
      continue;
    }
    if (p[0] == '<') {
      while (r->i < r->n && r->b[r->i] != '>')
        r->i++;
      r->i++;
      continue;
    }
    break;
  }
  char *p = (char *)r->b + r->i, *e;
  if (*p == '{') {
    r->i++;
    t.k = T_OB;
    return t;
  }
  if (*p == '}') {
    r->i++;
    t.k = T_CB;
    return t;
  }
  if (*p == '"') {
    size_t j = ++r->i;
    while (r->i < r->n && r->b[r->i] != '"')
      r->i++;
    snprintf(t.s, sizeof t.s, "%.*s", (int)(r->i - j), r->b + j);
    r->i++;
    t.k = T_STR;
    return t;
  }
  t.n = strtod(p, &e);
  if (e != p && (size_t)(e - (char *)r->b) <= r->n && delim(*e)) {
    r->i = e - (char *)r->b;
    t.k = T_NUM;
    return t;
  }
  size_t j = r->i;
  while (r->i < r->n && !delim(r->b[r->i]))
    r->i++;
  snprintf(t.s, sizeof t.s, "%.*s", (int)(r->i - j), r->b + j);
  t.k = T_NAME;
  return t;
}

static tok next(xr *r) { return r->bin ? bnext(r) : tnext(r); }
static double num(xr *r) {
  tok t = next(r);
  return t.k == T_NUM ? t.n : 0;
}
static int cnt(xr *r, int max) {
  int n = num(r);
  return n < 0 ? 0 : n > max ? max : n;
}
static void skip(xr *r) {
  for (int d = 1; d;) {
    tok t = next(r);
    if (!t.k)
      return;
    d += t.k == T_OB ? 1 : t.k == T_CB ? -1 : 0;
  }
}
static void close_(xr *r) {
  for (;;) {
    tok t = next(r);
    if (!t.k || t.k == T_CB)
      return;
    if (t.k == T_OB)
      skip(r);
  }
}
static int open_(xr *r, char *name) { // "Type [name] {"
  name[0] = 0;
  tok t = next(r);
  if (t.k == T_NAME) {
    snprintf(name, 128, "%s", t.s);
    t = next(r);
  }
  return t.k == T_OB;
}

// ---- materials -> brushes (+ a texture each)
static int material(xr *r) {
  mbrush b = {0};
  for (int i = 0; i < 4; i++)
    b.col[i] = num(r);
  for (int i = 0; i < 7; i++)
    num(r); // power, specular, emissive
  if (b.col[3] < 1)
    b.blend = 1;
  char n[128];
  for (;;) {
    tok t = next(r);
    if (!t.k || t.k == T_CB)
      break;
    if (t.k == T_OB) {
      skip(r);
      continue;
    }
    if (t.k != T_NAME || !open_(r, n))
      continue;
    if (!strcasecmp(t.s, "TextureFilename")) {
      tok s = next(r);
      if (s.k == T_STR && s.s[0]) {
        mtex x = {.flags = 1, .blend = 2, .sx = 1, .sy = 1};
        snprintf(x.name, sizeof x.name, "%s", s.s);
        b.tex[b.ntex++] = r->m->nt;
        PUSH(r->m->t, r->m->nt, x);
      }
      close_(r);
    } else
      skip(r);
  }
  PUSH(r->m->b, r->m->nb, b);
  return r->m->nb - 1;
}
static int matref(xr *r, const char *n) {
  for (int i = r->nmat - 1; i >= 0; i--)
    if (!strcmp(r->mats[i].name, n))
      return r->mats[i].brush;
  return -1;
}

// ---- meshes
#define MAXN (1 << 22)
static void mesh(xr *r, int ni) {
  int nv = cnt(r, MAXN), nf, tot = 0, cap = 0, nn = 0, nuv = 0, nfi = 0, nb = 0;
  float *v = malloc(sizeof(float) * 3 * (nv + 1)), *nrm = NULL, *uv = NULL,
        *col = NULL;
  int *fc, *fv = NULL, *fn = NULL, *fm = NULL, *bs = NULL;
  for (int i = 0; i < nv * 3; i++)
    v[i] = num(r);
  nf = cnt(r, MAXN);
  fc = malloc(sizeof(int) * (nf + 1));
  for (int f = 0; f < nf; f++) {
    int k = fc[f] = cnt(r, 64);
    if (tot + k > cap)
      fv = realloc(fv, sizeof(int) * (cap = (tot + k) * 2));
    for (int j = 0; j < k; j++) {
      int x = num(r);
      fv[tot++] = x >= 0 && x < nv ? x : 0;
    }
  }
  char n[128];
  for (;;) {
    tok t = next(r);
    if (!t.k || t.k == T_CB)
      break;
    if (t.k == T_OB) {
      skip(r);
      continue;
    }
    if (t.k != T_NAME || !open_(r, n))
      continue;
    if (!strcasecmp(t.s, "MeshNormals")) {
      nn = cnt(r, MAXN);
      free(nrm); // a 2nd block replaces the 1st
      nrm = malloc(sizeof(float) * 3 * (nn + 1));
      for (int i = 0; i < nn * 3; i++)
        nrm[i] = num(r);
      int nf2 = cnt(r, MAXN), o = 0;
      fn = malloc(sizeof(int) * (tot + 1));
      for (int f = 0; f < nf2; f++) {
        int k = cnt(r, 64);
        for (int j = 0; j < k; j++) {
          int x = num(r);
          if (f < nf && o < tot)
            fn[o++] = x >= 0 && x < nn ? x : 0;
        }
      }
      for (; o < tot; o++)
        fn[o] = 0;
      close_(r);
    } else if (!strcasecmp(t.s, "MeshTextureCoords")) {
      nuv = cnt(r, MAXN);
      free(uv); // a 2nd block replaces the 1st
      uv = malloc(sizeof(float) * 2 * (nuv + 1));
      for (int i = 0; i < nuv * 2; i++)
        uv[i] = num(r);
      close_(r);
    } else if (!strcasecmp(t.s, "MeshVertexColors")) {
      int k = cnt(r, MAXN);
      free(col); // a 2nd block replaces the 1st
      col = malloc(sizeof(float) * 4 * (nv + 1));
      for (int i = 0; i < nv * 4; i++)
        col[i] = 1;
      for (int i = 0; i < k; i++) {
        int x = num(r);
        float c[4];
        for (int j = 0; j < 4; j++)
          c[j] = num(r);
        if (x >= 0 && x < nv)
          memcpy(col + x * 4, c, 16);
      }
      close_(r);
    } else if (!strcasecmp(t.s, "MeshMaterialList")) {
      cnt(r, 4096);
      nfi = cnt(r, MAXN);
      free(fm); // a 2nd block replaces the 1st
      fm = malloc(sizeof(int) * (nfi + 1));
      for (int i = 0; i < nfi; i++)
        fm[i] = num(r);
      for (;;) {
        tok u = next(r);
        if (!u.k || u.k == T_CB)
          break;
        int b;
        if (u.k == T_OB) {
          tok rn = next(r);
          b = matref(r, rn.s);
          if (rn.k != T_CB)
            close_(r);
        } else if (u.k == T_NAME && !strcasecmp(u.s, "Material") && open_(r, n))
          b = material(r);
        else {
          if (u.k == T_NAME && open_(r, n))
            skip(r);
          continue;
        }
        PUSH(bs, nb, b);
      }
    } else
      skip(r);
  }

  // corners deduped on (vertex, normal) into the node's verts, one surface per
  // material
  mnode *nd = &r->m->n[ni];
  nd->nrm = nrm != NULL;
  int hs = (tot + 1) * 2, *key = malloc(sizeof(int) * 2 * hs);
  for (int i = 0; i < hs * 2; i++)
    key[i] = -1;
  int ns = nb ? nb : 1;
  for (int s = 0; s < ns; s++) {
    msurf sf = {nb ? bs[s] : -1, NULL, 0};
    int o = 0, tcap = 0;
    for (int f = 0; f < nf; f++) {
      int k = fc[f], mi = fm && nfi ? fm[f < nfi ? f : nfi - 1] : 0, ix[64];
      if (mi < 0 || mi >= ns)
        mi = 0;
      if (mi != s || k < 3) {
        o += k;
        continue;
      }
      for (int j = 0; j < k; j++) {
        int vi = fv[o + j], nix = fn ? fn[o + j] : 0,
            kk = vi * (nn ? nn : 1) + nix;
        unsigned h = (unsigned)kk * 2654435761u % hs;
        while (key[h * 2] >= 0 && key[h * 2] != kk)
          h = (h + 1) % hs;
        if (key[h * 2] < 0) {
          mvtx x = {.c = {1, 1, 1, 1}};
          memcpy(x.p, v + vi * 3, 12);
          if (uv && vi < nuv) {
            x.uv[0][0] = x.uv[1][0] = uv[vi * 2];
            x.uv[0][1] = x.uv[1][1] = uv[vi * 2 + 1];
          }
          if (nrm)
            memcpy(x.n, nrm + nix * 3, 12);
          if (col)
            memcpy(x.c, col + vi * 4, 16);
          key[h * 2] = kk;
          key[h * 2 + 1] = nd->nv;
          PUSH(nd->v, nd->nv, x);
        }
        ix[j] = key[h * 2 + 1];
      }
      for (int j = 2; j < k; j++) {
        if (sf.ntri + 1 > tcap)
          sf.tri =
              realloc(sf.tri, sizeof(int) * 3 * (tcap = tcap ? tcap * 2 : 64));
        sf.tri[sf.ntri * 3] = ix[0];
        sf.tri[sf.ntri * 3 + 1] = ix[j - 1];
        sf.tri[sf.ntri * 3 + 2] = ix[j];
        sf.ntri++;
      }
      o += k;
    }
    if (sf.ntri)
      PUSH(nd->s, nd->ns, sf);
  }
  free(key);
  free(v);
  free(fc);
  free(fv);
  free(fn);
  free(fm);
  free(nrm);
  free(uv);
  free(col);
  free(bs);
}

static void block(xr *r, const char *type, int par);
static void frame(xr *r, int ni) {
  for (;;) {
    tok t = next(r);
    if (!t.k || t.k == T_CB)
      return;
    if (t.k == T_OB) {
      skip(r);
      continue;
    }
    if (t.k == T_NAME)
      block(r, t.s, ni);
  }
}
static void block(xr *r, const char *type, int par) {
  char n[128];
  if (!strcasecmp(type, "template")) {
    if (open_(r, n))
      skip(r);
    return;
  }
  if (!open_(r, n))
    return;
  if (!strcasecmp(type, "Frame"))
    frame(r, mdl_node(r->m, n, par));
  else if (!strcasecmp(type, "FrameTransformMatrix")) {
    float a[16];
    for (int i = 0; i < 16; i++)
      a[i] = num(r);
    mnode *x = &r->m->n[par];
    // dx row vectors: matrix rows are the axes, the 4th row the position
    for (int c = 0; c < 3; c++)
      x->scl[c] = sqrtf(a[c * 4] * a[c * 4] + a[c * 4 + 1] * a[c * 4 + 1] +
                        a[c * 4 + 2] * a[c * 4 + 2]);
    for (int rr = 0; rr < 3; rr++)
      for (int c = 0; c < 3; c++)
        x->rot[rr][c] = x->scl[c] > 0 ? a[c * 4 + rr] / x->scl[c] : 0;
    for (int i = 0; i < 3; i++)
      x->pos[i] = a[12 + i];
    close_(r);
  } else if (!strcasecmp(type, "Mesh"))
    mesh(r, mdl_node(r->m, n, par));
  else if (!strcasecmp(type, "Material")) {
    int b = material(r);
    r->mats = realloc(r->mats, sizeof *r->mats * (r->nmat + 1));
    snprintf(r->mats[r->nmat].name, 128, "%s", n);
    r->mats[r->nmat++].brush = b;
  } else
    skip(r);
}

// mszip: "CK" + raw deflate per block. single block only
static unsigned char *unzip(unsigned char *b, size_t n, size_t *on) {
  if (n < 28)
    return NULL;
  uint32_t full;
  uint16_t un, cn;
  memcpy(&full, b + 16, 4);
  memcpy(&un, b + 20, 2);
  memcpy(&cn, b + 22, 2);
  if (memcmp(b + 24, "CK", 2) || 26 + (size_t)cn - 2 > n)
    return NULL;
  if ((uint32_t)un + 16 < full) {
    warn("x: multi-block mszip not supported");
    return NULL;
  }
  int dn = 0;
  unsigned char *d = DecompressData(b + 26, cn - 2, &dn);
  if (!d)
    return NULL;
  unsigned char *o = malloc(16 + dn);
  memcpy(o, b, 16);
  memcpy(o + 8, "bin ", 4); // NOLINT(bugprone-not-null-terminated-result):
                            // header bytes, not a string
  memcpy(o + 16, d, dn);
  MemFree(d);
  *on = 16 + dn;
  return o;
}

int x_read(const char *path, mdl *m) {
  *m = (mdl){0};
  int n = 0;
  unsigned char *b = LoadFileData(path, &n);
  if (!b || n < 16 || memcmp(b, "xof ", 4)) {
    UnloadFileData(b);
    return 0;
  }
  xr r = {.b = b, .n = n, .i = 16, .m = m};
  if (!memcmp(b + 8, "bzip", 4)) {
    size_t on;
    unsigned char *u = unzip(b, n, &on);
    UnloadFileData(b);
    if (!u)
      return 0;
    r.b = u;
    r.n = on;
  }
  r.bin = !memcmp(r.b + 8, "bin ", 4);
  r.f64 = !memcmp(r.b + 12, "0064", 4);
  snprintf(m->dir, sizeof m->dir, "%s", path);
  char *sl = strrchr(m->dir, '/');
  sl ? (sl[1] = 0) : (m->dir[0] = 0);
  int root = mdl_node(m, "root", -1);
  for (;;) {
    tok t = next(&r);
    if (!t.k)
      break;
    if (t.k == T_NAME)
      block(&r, t.s, root);
    else if (t.k == T_OB)
      skip(&r);
  }
  free(r.mats);
  if (r.b != b)
    free(r.b);
  else
    UnloadFileData(b);
  return 1;
}
