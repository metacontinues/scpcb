// door.c: the original's doors (CreateDoor / UseDoor / UpdateDoors). in
// right-handed space, so the original's local +z offsets come out as -z here.
// leaves slide by sin(state)/k per tick of v state; summed that's
// (1 - cos(state)) * 180 / (pi * v * k), so a leaf's offset is just a function
// of state
#include "door.h"
#include "coll.h"
#include "hud.h"
#include "props.h"
#include "raylib.h"
#include "raymath.h"
#include "snd.h"
#include "util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Mdl *leaf, *frame, *heavy[2], *big[2], *bigcoll, *elev, *btns[4];
static Vector3 leafscale;
// per kind: state per tick, leaf 0's + leaf 1's k
static const float speed[] = {2, .8f, 2, 2}, k0[] = {80, 180, 85, 162},
                   k1[] = {80, 180, 120, 162};

void door_init(void) {
  leaf = prop_get("gfx/map/door01");
  frame = prop_get("gfx/map/doorframe");
  heavy[0] = prop_get("gfx/map/heavydoor1");
  heavy[1] = prop_get("gfx/map/heavydoor2");
  big[0] = prop_get("gfx/map/contdoorleft");
  big[1] = prop_get("gfx/map/contdoorright");
  bigcoll = prop_get("gfx/map/doorcoll");
  elev = prop_get("gfx/map/elevatordoor");
  btns[0] = prop_get("gfx/map/button");
  btns[1] = prop_get("gfx/map/buttonkeycard");
  btns[2] = prop_get("gfx/map/buttonscanner");
  btns[3] = prop_get("gfx/map/buttoncode");
  leafscale = (Vector3){1, 1, 1};
  if (leaf) { // the original stretched door01 to 204 x 312 x 16 room units
    BoundingBox b = GetModelBoundingBox(leaf->m);
    Vector3 s = Vector3Subtract(b.max, b.min);
    leafscale =
        (Vector3){204 * RSCALE / s.x, 312 * RSCALE / s.y, 16 * RSCALE / s.z};
  }
}

static Matrix place(Vector3 p, float yaw) {
  return MatrixMultiply(MatrixRotateY(yaw * DEG2RAD),
                        MatrixTranslate(p.x, p.y, p.z));
}

int door_add(World *w, Vector3 p, int angle, int kind, int room, int open) {
  w->doors = realloc(w->doors, sizeof *w->doors * (w->ndoors + 1));
  Door *d = &w->doors[w->ndoors];
  *d = (Door){.pos = p,
              .angle = angle,
              .kind = kind,
              .room = room,
              .open = open,
              .state = open ? 180 : 0,
              .autoclose = open && kind == DOOR_NORMAL && irnd(1, 8) == 1,
              .link = -1,
              .mtf = 1,
              .btn = {1, 1}};
  // the buttons: big doors' off to the sides, facing in
  Vector3 o[2] = {{.6f, .7f, .1f}, {-.6f, .7f, -.1f}};
  int y[2] = {0, 180};
  if (kind == DOOR_BIG) {
    o[0] = (Vector3){-432 * RSCALE, .7f, -192 * RSCALE},
    o[1] = (Vector3){432 * RSCALE, .7f, 192 * RSCALE};
    y[0] = 90, y[1] = 270;
  }
  for (int i = 0; i < 2; i++) {
    d->bp[i] = Vector3Transform(o[i], place(p, angle));
    d->byaw[i] = angle + y[i];
  }
  return w->ndoors++;
}

// leaf i's model + matrix: scale, slide along its local x, turn, place
static Matrix leafm(const Door *d, int i, Mdl **m) { // *m = NULL: no such leaf
  float c = (1 - cosf(d->state * DEG2RAD)) * 180 /
            (PI * speed[d->kind] * (i ? k1 : k0)[d->kind]);
  Matrix s = MatrixScale(RSCALE, RSCALE, RSCALE);
  float turn = i ? 180 : 0, z = 0;
  switch (d->kind) {
  case DOOR_HEAVY:
    *m = heavy[i];
    break;
  case DOOR_BIG: // both face the same way, leaf 1 slides back
    *m = big[i];
    s = MatrixScale(55 * RSCALE, 55 * RSCALE, 55 * RSCALE);
    turn = 0, c = i ? -c : c;
    break;
  case DOOR_ELEVATOR:
    *m = elev;
    break;
  default:
    *m = leaf;
    s = MatrixScale(leafscale.x, leafscale.y, leafscale.z);
    z = -8 * RSCALE;
  }
  if (i && d->one)
    *m = NULL;
  return MatrixMultiply(MatrixMultiply(s, MatrixTranslate(c, 0, z)),
                        place(d->pos, d->angle + turn));
}

static int btnkind(const Door *d) {
  return d->code[0] ? 3 : d->key > 0 ? 1 : d->key < 0 ? 2 : 0;
}

static Matrix btnm(const Door *d, int i) {
  return MatrixMultiply(MatrixScale(.03f, .03f, .03f),
                        place(d->bp[i], d->byaw[i]));
}

int door_button(World *w, Vector3 eye, Vector3 body, Vector3 *at) {
  int best = -1;
  float bd = 1e9f;
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    if (d->state > 0 && d->state < 180)
      continue;
    for (int b = 0; b < 2; b++) {
      if (!d->btn[b])
        continue;
      Vector3 p = d->bp[b];
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
      if (at)
        *at = p;
    }
  }
  return best;
}

// OpenDoorSFX / CloseDoorSFX: 3 per kind (big doors have 2)
static void sound(const Door *d) {
  static const char *k[] = {"door", "bigdoor", "door2", "elevator"};
  char id[48];
  snprintf(id, sizeof id, "sfx/door/%s%s%d", k[d->kind],
           d->open ? "open" : "close", irnd(1, d->kind == DOOR_BIG ? 2 : 3));
  snd_play3d(id, d->pos, 10, 1);
}

static void toggle(World *w, Door *d) {
  d->open = !d->open;
  if (d->link >= 0)
    w->doors[d->link].open = !w->doors[d->link].open;
  if (d->open) {
    if (d->link >= 0)
      w->doors[d->link].tleft = w->doors[d->link].timer;
    d->tleft = d->timer;
  }
  sound(d);
}

void door_use(World *w, int i) {
  if (i < 0 || i >= w->ndoors)
    return;
  Door *d = &w->doors[i];
  if (d->key > 0) { // no inventory yet, so never the right card
    hud_msg("A keycard is required to operate this door.", 70 * 7);
    return;
  }
  if (d->key < 0) {
    snd_play("sfx/interact/scanneruse2", 1);
    hud_msg("You placed your palm onto the scanner. The scanner reads: \"DNA "
            "does not match known sample. Access "
            "denied.\"",
            70 * 10);
    return;
  }
  if (d->code[0]) // the keypad comes w/ items
    return;
  if (d->locked) {
    snd_play("sfx/interact/button2", 1);
    const Room *r =
        d->room >= 0 && d->room < w->nrooms ? &w->rooms[d->room] : NULL;
    hud_msg(r && r->t && !strcmp(r->t->name, "room2elevator")
                ? "The elevator appears to be broken."
            : d->open ? "You pushed the button but nothing happened."
                      : "The door appears to be locked.",
            70 * 5);
    return;
  }
  toggle(w, d);
}

void door_tick(World *w, Vector3 eye) {
  for (int i = 0; i < w->ndoors; i++) {
    Door *d = &w->doors[i];
    float v = speed[d->kind];
    d->state = d->open ? minf(180, d->state + v) : maxf(0, d->state - v);
    if (!d->open || d->state < 180)
      continue;
    if (d->tleft > 0) { // a timed door: the caution beep, then it shuts
      d->tleft = maxf(0, d->tleft - 1);
      if (d->tleft + 1 > 110 && d->tleft <= 110)
        snd_play3d("sfx/room/lockroomsiren", d->pos, 10, 1);
      if (d->tleft == 0) {
        d->open = 0;
        sound(d);
      }
    }
    if (d->autoclose && Vector3Distance(eye, d->pos) < 2.1f) {
      d->open = 0;
      d->autoclose = 0;
      sound(d);
    }
  }
}

void door_solids(World *w) { // the big doors' invisible frames
  for (int i = 0; bigcoll && i < w->ndoors; i++)
    if (w->doors[i].kind == DOOR_BIG)
      for (int m = 0; m < bigcoll->m.meshCount; m++)
        coll_mesh(bigcoll->m.meshes[m],
                  MatrixMultiply(MatrixScale(RSCALE, RSCALE, RSCALE),
                                 place(w->doors[i].pos, w->doors[i].angle)));
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
      if (d->btn[k] && btns[btnkind(d)])
        mdl_draw(btns[btnkind(d)], btnm(d, k), alpha);
    }
    if (frame && d->kind != DOOR_BIG)
      mdl_draw(frame,
               MatrixMultiply(MatrixScale(RSCALE, RSCALE, RSCALE),
                              place(d->pos, d->angle)),
               alpha);
  }
}
