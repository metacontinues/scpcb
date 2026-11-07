// door.c: doors between rooms (normal + heavy for now). in right-handed space,
// so the original's local +z offsets come out as -z here
#include "door.h"
#include "coll.h"
#include "props.h"
#include "raylib.h"
#include "raymath.h"
#include "util.h"
#include <math.h>

static Mdl *leaf, *frame, *heavy[2], *button;
static Vector3 leafscale;

void door_init(void) {
  leaf = prop_get("gfx/map/door01");
  frame = prop_get("gfx/map/doorframe");
  heavy[0] = prop_get("gfx/map/heavydoor1");
  heavy[1] = prop_get("gfx/map/heavydoor2");
  button = prop_get("gfx/map/button");
  leafscale = (Vector3){1, 1, 1};
  if (leaf) { // the original stretched door01 to 204 x 312 x 16 room units
    BoundingBox b = GetModelBoundingBox(leaf->m);
    Vector3 s = Vector3Subtract(b.max, b.min);
    leafscale =
        (Vector3){204 * RSCALE / s.x, 312 * RSCALE / s.y, 16 * RSCALE / s.z};
  }
}

static Matrix place(const Door *d, float turn) {
  return MatrixMultiply(MatrixRotateY((d->angle + turn) * DEG2RAD),
                        MatrixTranslate(d->pos.x, d->pos.y, d->pos.z));
}

// leaf i's matrix: scale, slide along its local x, turn, place
static Matrix leafm(const Door *d, int i, Mdl **m) {
  float c = 1 - cosf(d->state * DEG2RAD);
  if (d->kind == DOOR_HEAVY) {
    *m = heavy[i];
    float k = i ? 180 / (PI * 240) : 180 / (PI * 170); // /85 + /120 per tick
    return MatrixMultiply(MatrixMultiply(MatrixScale(RSCALE, RSCALE, RSCALE),
                                         MatrixTranslate(c * k, 0, 0)),
                          place(d, i ? 180 : 0));
  }
  *m = leaf;
  float k = 180 / (PI * 160); // /80 per tick
  return MatrixMultiply(
      MatrixMultiply(MatrixScale(leafscale.x, leafscale.y, leafscale.z),
                     MatrixTranslate(c * k, 0, -8 * RSCALE)),
      place(d, i ? 180 : 0));
}

static Vector3 btnpos(const Door *d, int i) {
  Vector3 o = i ? (Vector3){-.6f, .7f, -.1f} : (Vector3){.6f, .7f, .1f};
  return Vector3Transform(o, place(d, 0));
}
static Matrix btnm(const Door *d, int i) {
  Vector3 p = btnpos(d, i);
  return MatrixMultiply(
      MatrixMultiply(MatrixScale(.03f, .03f, .03f),
                     MatrixRotateY((d->angle + (i ? 180 : 0)) * DEG2RAD)),
      MatrixTranslate(p.x, p.y, p.z));
}

int door_button(World *w, Vector3 eye, Vector3 body) {
  int best = -1;
  float bd = 1e9f;
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    if (d->state > 0 && d->state < 180)
      continue;
    for (int b = 0; b < 2; b++) {
      Vector3 p = btnpos(d, b);
      float xz =
                Vector2Distance((Vector2){body.x, body.z}, (Vector2){p.x, p.z}),
            de = Vector3Distance(eye, p);
      if (xz >= .7f || de > .75f || de >= bd)
        continue;
      RayCollision h = coll_ray(eye, p); // nothing in the way
      if (h.hit && h.distance < de - .05f)
        continue;
      best = i;
      bd = de;
    }
  }
  return best;
}

void door_use(World *w, int i) {
  if (i >= 0 && i < w->ndoors)
    w->doors[i].open = !w->doors[i].open;
}

void door_tick(World *w, Vector3 eye) {
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    d->state = d->open ? minf(180, d->state + 2) : maxf(0, d->state - 2);
    if (d->open && d->state >= 180 && d->autoclose &&
        Vector3Distance(eye, d->pos) < 2.1f) {
      d->open = 0;
      d->autoclose = 0;
    }
  }
}

void door_coll(World *w, Vector3 near, float r) {
  coll_dyn_reset();
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    if (Vector3Distance(near, d->pos) > r)
      continue;
    for (int k = 0; k < 2; k++) {
      Mdl *m;
      Matrix t = leafm(d, k, &m);
      if (m)
        coll_dyn_box(GetModelBoundingBox(m->m), t);
    }
  }
}

void door_draw(World *w, Vector3 eye, float r, int alpha) {
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    if (Vector3Distance(eye, d->pos) > r)
      continue;
    for (int k = 0; k < 2; k++) {
      Mdl *m;
      Matrix t = leafm(d, k, &m);
      if (m)
        mdl_draw(m, t, alpha);
      if (button)
        mdl_draw(button, btnm(d, k), alpha);
    }
    if (frame)
      mdl_draw(frame,
               MatrixMultiply(MatrixScale(RSCALE, RSCALE, RSCALE), place(d, 0)),
               alpha);
  }
}
