// coll.c: swept ellipsoid vs triangles w/ sliding (fauerby's "improved
// collision detection and response"): scale the world by 1/radii so the mover
// is a unit sphere, sweep it, slide along the plane of the first contact,
// repeat a few times. the static triangles sit in a uniform xz grid, dynamic
// ones are just a list
#include "coll.h"
#include "raylib.h"
#include "raymath.h"
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Vector3 a, b, c;
} Tri;

#define CELL 2.0f
static Tri *st, *dyn;
static int nst, cst, ndyn, cdyn;
static uint32_t *stamp, epoch;
static int *cellstart, *cellitems, gw, gh;
static float gx0, gz0;

#define PUSH(a, n, c, v)                                                       \
  do {                                                                         \
    if ((n) >= (c))                                                            \
      (a) = realloc((a), sizeof *(a) * ((c) = (c) ? (c) * 2 : 1024));          \
    (a)[(n)++] = (v);                                                          \
  } while (0)

void coll_reset(void) {
  free(st);
  free(dyn);
  free(stamp);
  free(cellstart);
  free(cellitems);
  st = dyn = NULL;
  stamp = NULL;
  cellstart = cellitems = NULL;
  nst = cst = ndyn = cdyn = gw = gh = 0;
}

void coll_mesh(Mesh m, Matrix t) {
  if (!m.vertices)
    return;
  Vector3 *v = (Vector3 *)m.vertices;
  int n = m.indices ? m.triangleCount * 3 : m.vertexCount;
  for (int i = 0; i + 2 < n; i += 3) {
    int a = m.indices ? m.indices[i] : i,
        b = m.indices ? m.indices[i + 1] : i + 1,
        c = m.indices ? m.indices[i + 2] : i + 2;
    Tri x = {Vector3Transform(v[a], t), Vector3Transform(v[b], t),
             Vector3Transform(v[c], t)};
    PUSH(st, nst, cst, x);
  }
}

static int cellx(float x) { return (int)floorf((x - gx0) / CELL); }
static int cellz(float z) { return (int)floorf((z - gz0) / CELL); }

void coll_build(void) {
  float x1 = -FLT_MAX, z1 = -FLT_MAX;
  gx0 = gz0 = FLT_MAX;
  for (int i = 0; i < nst; i++) {
    Tri *t = &st[i];
    gx0 = fminf(gx0, fminf(t->a.x, fminf(t->b.x, t->c.x)));
    gz0 = fminf(gz0, fminf(t->a.z, fminf(t->b.z, t->c.z)));
    x1 = fmaxf(x1, fmaxf(t->a.x, fmaxf(t->b.x, t->c.x)));
    z1 = fmaxf(z1, fmaxf(t->a.z, fmaxf(t->b.z, t->c.z)));
  }
  if (!nst)
    gx0 = gz0 = x1 = z1 = 0;
  gw = cellx(x1) + 1;
  gh = cellz(z1) + 1;
  free(cellstart);
  free(cellitems);
  free(stamp);
  cellstart = calloc((size_t)gw * gh + 1, sizeof *cellstart);
  stamp = calloc(nst ? nst : 1, sizeof *stamp);
  // two passes: count per cell, then fill (csr)
  for (int pass = 0; pass < 2; pass++) {
    int *fill = pass ? calloc((size_t)gw * gh, sizeof *fill) : NULL;
    for (int i = 0; i < nst; i++) {
      Tri *t = &st[i];
      int xa = cellx(fminf(t->a.x, fminf(t->b.x, t->c.x))),
          xb = cellx(fmaxf(t->a.x, fmaxf(t->b.x, t->c.x))),
          za = cellz(fminf(t->a.z, fminf(t->b.z, t->c.z))),
          zb = cellz(fmaxf(t->a.z, fmaxf(t->b.z, t->c.z)));
      for (int z = za; z <= zb; z++)
        for (int x = xa; x <= xb; x++) {
          int c = z * gw + x;
          if (pass)
            cellitems[cellstart[c] + fill[c]++] = i;
          else
            cellstart[c + 1]++;
        }
    }
    if (!pass) {
      for (int c = 0; c < gw * gh; c++)
        cellstart[c + 1] += cellstart[c];
      cellitems = malloc(sizeof *cellitems * (cellstart[gw * gh] + 1));
    }
    free(fill);
  }
}

void coll_dyn_reset(void) { ndyn = 0; }

void coll_dyn_box(BoundingBox b, Matrix t) {
  Vector3 p[8];
  for (int i = 0; i < 8; i++)
    p[i] = Vector3Transform((Vector3){i & 1 ? b.max.x : b.min.x,
                                      i & 2 ? b.max.y : b.min.y,
                                      i & 4 ? b.max.z : b.min.z},
                            t);
  static const int f[6][4] = {{0, 1, 3, 2}, {4, 6, 7, 5}, {0, 4, 5, 1},
                              {2, 3, 7, 6}, {0, 2, 6, 4}, {1, 5, 7, 3}};
  for (int i = 0; i < 6; i++) {
    Tri a = {p[f[i][0]], p[f[i][1]], p[f[i][2]]},
        c = {p[f[i][0]], p[f[i][2]], p[f[i][3]]};
    PUSH(dyn, ndyn, cdyn, a);
    PUSH(dyn, ndyn, cdyn, c);
  }
}

// static triangles touching the xz box, each once
static int *cand;
static int ncand, ccand;
static void gather(Vector3 lo, Vector3 hi) {
  ncand = 0;
  if (!gw)
    return;
  if (!++epoch) { // wrapped: clear the stamps
    memset(stamp, 0, sizeof *stamp * (nst ? nst : 1));
    epoch = 1;
  }
  int xa = cellx(lo.x), xb = cellx(hi.x), za = cellz(lo.z), zb = cellz(hi.z);
  xa = xa < 0 ? 0 : xa;
  za = za < 0 ? 0 : za;
  xb = xb >= gw ? gw - 1 : xb;
  zb = zb >= gh ? gh - 1 : zb;
  for (int z = za; z <= zb; z++)
    for (int x = xa; x <= xb; x++)
      for (int k = cellstart[z * gw + x]; k < cellstart[z * gw + x + 1]; k++) {
        int i = cellitems[k];
        if (stamp[i] == epoch)
          continue;
        stamp[i] = epoch;
        PUSH(cand, ncand, ccand, i);
      }
}

// ---- the sweep, in ellipsoid space (unit sphere)
typedef struct {
  Vector3 vel, base, point;
  float near;
  int found;
} Sweep;

static int lowroot(float a, float b, float c, float max, float *r) {
  float d = b * b - 4 * a * c;
  if (d < 0 || fabsf(a) < 1e-12f)
    return 0;
  float s = sqrtf(d), r1 = (-b - s) / (2 * a), r2 = (-b + s) / (2 * a);
  if (r1 > r2) {
    float t = r1;
    r1 = r2;
    r2 = t;
  }
  if (r1 > 0 && r1 < max) {
    *r = r1;
    return 1;
  }
  if (r2 > 0 && r2 < max) {
    *r = r2;
    return 1;
  }
  return 0;
}

static int inside(Vector3 p, Vector3 a, Vector3 b, Vector3 c) {
  Vector3 v0 = Vector3Subtract(c, a), v1 = Vector3Subtract(b, a),
          v2 = Vector3Subtract(p, a);
  float d00 = Vector3DotProduct(v0, v0), d01 = Vector3DotProduct(v0, v1),
        d02 = Vector3DotProduct(v0, v2), d11 = Vector3DotProduct(v1, v1),
        d12 = Vector3DotProduct(v1, v2), den = d00 * d11 - d01 * d01;
  if (fabsf(den) < 1e-12f)
    return 0;
  float u = (d11 * d02 - d01 * d12) / den, v = (d00 * d12 - d01 * d02) / den;
  return u >= 0 && v >= 0 && u + v <= 1;
}

static void check(Sweep *k, Vector3 p1, Vector3 p2, Vector3 p3) {
  Vector3 n =
      Vector3CrossProduct(Vector3Subtract(p2, p1), Vector3Subtract(p3, p1));
  if (Vector3LengthSqr(n) < 1e-14f)
    return;
  n = Vector3Normalize(n);
  float sd = Vector3DotProduct(n, Vector3Subtract(k->base, p1));
  if (sd < 0) { // double-sided: face the mover
    n = Vector3Negate(n);
    sd = -sd;
  }
  float ndv = Vector3DotProduct(n, k->vel), t0, t1;
  int embedded = 0;
  if (fabsf(ndv) < 1e-7f) {
    if (sd >= 1)
      return;
    embedded = 1;
    t0 = 0;
  } else {
    t0 = (1 - sd) / ndv;
    t1 = (-1 - sd) / ndv;
    if (t0 > t1) {
      float t = t0;
      t0 = t1;
      t1 = t;
    }
    if (t0 > 1 || t1 < 0)
      return;
    t0 = t0 < 0 ? 0 : t0;
  }
  Vector3 cp = {0};
  float t = 1;
  int found = 0;
  if (!embedded) { // the sphere meets the plane inside the triangle
    Vector3 pi =
        Vector3Add(Vector3Subtract(k->base, n), Vector3Scale(k->vel, t0));
    if (inside(pi, p1, p2, p3)) {
      found = 1;
      t = t0;
      cp = pi;
    }
  }
  if (!found) { // else a vertex or an edge
    float vsq = Vector3LengthSqr(k->vel), r;
    Vector3 ps[3] = {p1, p2, p3};
    for (int i = 0; i < 3; i++) {
      Vector3 d = Vector3Subtract(k->base, ps[i]);
      if (lowroot(vsq, 2 * Vector3DotProduct(k->vel, d),
                  Vector3LengthSqr(d) - 1, t, &r)) {
        t = r;
        found = 1;
        cp = ps[i];
      }
    }
    for (int i = 0; i < 3; i++) {
      Vector3 a = ps[i], e = Vector3Subtract(ps[(i + 1) % 3], a),
              bv = Vector3Subtract(a, k->base);
      float esq = Vector3LengthSqr(e), edv = Vector3DotProduct(e, k->vel),
            ebv = Vector3DotProduct(e, bv);
      float qa = esq * -vsq + edv * edv,
            qb = esq * 2 * Vector3DotProduct(k->vel, bv) - 2 * edv * ebv,
            qc = esq * (1 - Vector3LengthSqr(bv)) + ebv * ebv;
      if (lowroot(qa, qb, qc, t, &r)) {
        float f = (edv * r - ebv) / esq;
        if (f >= 0 && f <= 1) {
          t = r;
          found = 1;
          cp = Vector3Add(a, Vector3Scale(e, f));
        }
      }
    }
  }
  if (!found)
    return;
  float d = t * Vector3Length(k->vel);
  if (!k->found || d < k->near) {
    k->near = d;
    k->point = cp;
    k->found = 1;
  }
}

static Vector3 er; // radii of the current mover
static float lowy; // world y the "low" contacts are under
static int lowhit;

static Vector3 es(Vector3 v) {
  return (Vector3){v.x / er.x, v.y / er.y, v.z / er.z};
}

static void sweepall(Sweep *k) {
  for (int i = 0; i < ncand; i++)
    check(k, es(st[cand[i]].a), es(st[cand[i]].b), es(st[cand[i]].c));
  for (int i = 0; i < ndyn; i++)
    check(k, es(dyn[i].a), es(dyn[i].b), es(dyn[i].c));
}

#define CLOSE 0.005f
static Vector3 slide(Vector3 pos, Vector3 vel, int depth) {
  if (depth > 5 || Vector3LengthSqr(vel) < CLOSE * CLOSE * 1e-2f)
    return pos;
  Sweep k = {vel, pos, {0}, 0, 0};
  sweepall(&k);
  if (!k.found)
    return Vector3Add(pos, vel);
  if (k.point.y * er.y < lowy)
    lowhit = 1;
  Vector3 dest = Vector3Add(pos, vel), nb = pos;
  if (k.near >= CLOSE) {
    Vector3 v = Vector3Scale(Vector3Normalize(vel), k.near - CLOSE);
    nb = Vector3Add(pos, v);
    k.point =
        Vector3Subtract(k.point, Vector3Scale(Vector3Normalize(v), CLOSE));
  }
  Vector3 n = Vector3Normalize(Vector3Subtract(nb, k.point));
  float dd = Vector3DotProduct(n, Vector3Subtract(dest, k.point));
  Vector3 nd = Vector3Subtract(dest, Vector3Scale(n, dd)),
          nv = Vector3Subtract(nd, k.point);
  if (Vector3Length(nv) < CLOSE)
    return nb;
  return slide(nb, nv, depth + 1);
}

Vector3 coll_move(Vector3 pos, Vector3 vel, Vector3 r, float belowy, int *low) {
  Vector3 a = Vector3Min(pos, Vector3Add(pos, vel)),
          b = Vector3Max(pos, Vector3Add(pos, vel));
  gather(Vector3Subtract(a, r), Vector3Add(b, r));
  er = r;
  lowy = belowy;
  lowhit = 0;
  Vector3 e = slide(es(pos), es(vel), 0);
  if (low && lowhit)
    *low = 1;
  return (Vector3){e.x * r.x, e.y * r.y, e.z * r.z};
}

RayCollision coll_ray(Vector3 a, Vector3 b) {
  RayCollision best = {0};
  Vector3 d = Vector3Subtract(b, a);
  float len = Vector3Length(d);
  if (len <= 0)
    return best;
  Ray ray = {a, Vector3Scale(d, 1 / len)};
  gather(Vector3Min(a, b), Vector3Max(a, b));
  for (int pass = 0; pass < 2; pass++) {
    int n = pass ? ndyn : ncand;
    for (int i = 0; i < n; i++) {
      Tri *t = pass ? &dyn[i] : &st[cand[i]];
      RayCollision h = GetRayCollisionTriangle(ray, t->a, t->b, t->c);
      if (!h.hit)
        h = GetRayCollisionTriangle(ray, t->a, t->c, t->b); // either side
      if (h.hit && h.distance <= len &&
          (!best.hit || h.distance < best.distance))
        best = h;
    }
  }
  return best;
}
