// fill.c: FillRoom's helpers + world_fill. the original made a room's things
// at room-local spots, then turned the room; here they're made local + taken
// through the room's matrix. lh -> rh: z flips, yaw stays, pitch flips
#include "fill.h"
#include "door.h"
#include "props.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector3 floc(F *f, float x, float y, float z) {
  return Vector3Transform((Vector3){x, y, -z}, f->r->m);
}

int fdoor(F *f, float x, float y, float z, float a, int open, int kind, int key,
          const char *code) {
  int d = door_add(f->w, floc(f, x, y, z), (int)wrapdeg(a + f->r->angle), kind,
                   f->ri, open);
  D(d)->key = key;
  snprintf(D(d)->code, sizeof D(d)->code, "%s", code ? code : "");
  return d;
}

void fbtn(F *f, int d, int b, float x, float y, float z) {
  Vector3 o = Vector3Transform(D(d)->bp[b],
                               MatrixInvert(f->r->m)); // where it is now, local
  D(d)->bp[b] =
      floc(f, isnan(x) ? o.x : x, isnan(y) ? o.y : y, isnan(z) ? -o.z : z);
}
void fbyaw(F *f, int d, int b, float a) {
  D(d)->byaw[b] = (int)wrapdeg(a + f->r->angle);
}
void fbturn(F *f, int d, int b, float a) {
  D(d)->byaw[b] = (int)wrapdeg(D(d)->byaw[b] + a);
}
void fbmove(F *f, int d, int b, float dz) {
  Vector3 dir = Vector3Transform((Vector3){0, 0, -dz},
                                 MatrixRotateY(D(d)->byaw[b] * DEG2RAD));
  D(d)->bp[b] = Vector3Add(D(d)->bp[b], dir);
}
void fnobtn(F *f, int d, int b) { D(d)->btn[b] = 0; }
void flink(F *f, int a, int b) { D(a)->link = b, D(b)->link = a; }

void fpiv(F *f, int o) {
  f->r->obj[o] = (RoomObj){.on = 1};
  f->set[o] = 1;
  f->op[o] = f->orot[o] = Vector3Zero();
  f->os[o] = Vector3One();
}
void fmesh(F *f, int o, const char *model) {
  fpiv(f, o);
  snprintf(f->r->obj[o].model, sizeof f->r->obj[o].model, "%s", model);
}
void fcopy(F *f, int o, int from) {
  fmesh(f, o, f->r->obj[from].model);
  f->r->obj[o].coll = f->r->obj[from].coll;
}
void fopos(F *f, int o, float x, float y, float z) {
  f->op[o] = (Vector3){x, y, z};
}
void forot(F *f, int o, float p, float yaw, float roll) {
  f->orot[o] = (Vector3){p, yaw, roll};
}
void foturn(F *f, int o, float p, float yaw, float roll) {
  f->orot[o] = Vector3Add(f->orot[o], (Vector3){p, yaw, roll});
}
void foscale(F *f, int o, float x, float y, float z) {
  f->os[o] = (Vector3){x, y, z};
}
void fcoll(F *f, int o) { f->r->obj[o].coll = 1; }
void fhide(F *f, int o) { f->r->obj[o].hide = 1; }
float fox(F *f, int o) { return f->op[o].x; }
float foy(F *f, int o) { return f->op[o].y; }
float foz(F *f, int o) { return f->op[o].z; }

int fitem(F *f, const char *name, const char *tmpl, float x, float y, float z) {
  World *w = f->w;
  w->items = realloc(w->items, sizeof *w->items * (w->nitems + 1));
  ItemSpawn *it = &w->items[w->nitems];
  *it = (ItemSpawn){
      .pos = floc(f, x, y, z), .yaw = (float)f->r->angle, .room = f->ri};
  snprintf(it->name, sizeof it->name, "%s", name);
  snprintf(it->tmpl, sizeof it->tmpl, "%s", tmpl);
  return w->nitems++;
}
void fiyaw(F *f, int it, float a) {
  f->w->items[it].yaw = wrapdeg(a + f->r->angle);
}
void fistate(F *f, int it, float s) { f->w->items[it].state = s; }
void finame(F *f, int it, const char *name) {
  snprintf(f->w->items[it].name, sizeof f->w->items[it].name, "%s", name);
}

int fnamed(F *f, const char *name) {
  for (int i = 0; i < f->w->nrooms; i++)
    if (f->w->rooms[i].t && !strcmp(f->w->rooms[i].t->name, name))
      return i;
  return -1;
}

void fihide(F *f, int it) { f->w->items[it].hidden = 1; }

Vector3 fleafscale(void) {
  Mdl *m = prop_get("gfx/map/door01");
  if (!m)
    return Vector3One();
  BoundingBox b = GetModelBoundingBox(m->m);
  Vector3 s = Vector3Subtract(b.max, b.min);
  return (Vector3){204 * S / s.x, 312 * S / s.y, 16 * S / s.z};
}

int fcell(F *f, int dx, int dz) {
  for (int i = 0; i < f->w->nrooms; i++) // the original's +z is our -gy
    if (f->w->rooms[i].gx == f->r->gx + dx &&
        f->w->rooms[i].gy == f->r->gy + dz && f->w->rooms[i].t)
      return 1;
  return 0;
}

const char *fcode(int n) {
  static char b[8];
  snprintf(b, sizeof b, "%d", n);
  return b;
}

static void (*find(const char *name))(F *) {
  const Fill *t[] = {fills1, fills2, fills3};
  for (int k = 0; k < 3; k++)
    for (const Fill *e = t[k]; e->name; e++)
      if (!strcmp(e->name, name))
        return e->fn;
  return NULL;
}

void world_fill(World *w) {
  for (int i = 0; i < w->nrooms; i++) {
    Room *r = &w->rooms[i];
    void (*fn)(F *) = r->t ? find(r->t->name) : NULL;
    if (!fn)
      continue;
    static F f;
    f = (F){.w = w, .r = r, .ri = i};
    for (int l = 0; l < 12; l++)
      r->lever[l] = -1;
    fn(&f);
    for (int o = 0; o < ROOM_OBJS;
         o++) { // scale, roll, pitch, yaw, place, the room
      if (!f.set[o])
        continue;
      Vector3 p = f.op[o], a = Vector3Scale(f.orot[o], DEG2RAD), s = f.os[o];
      Matrix m = MatrixMultiply(MatrixScale(s.x, s.y, s.z), MatrixRotateZ(a.z));
      m = MatrixMultiply(MatrixMultiply(m, MatrixRotateX(-a.x)),
                         MatrixRotateY(a.y));
      r->obj[o].m = MatrixMultiply(
          MatrixMultiply(m, MatrixTranslate(p.x, p.y, -p.z)), r->m);
    }
  }
}
