// fill.h: FillRoom's helpers, shared by fill*.c. everything takes the
// original's room-local numbers: meters (x * RoomScale -> x * S), its
// left-handed axes + yaw sense, before the room's turned. objects keep their
// pos/rot/scale until fill_done bakes them into world matrices
#pragma once
#include "door.h"
#include "util.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define S RSCALE
#define KEEP NAN // a button's y: leave it

typedef struct {
  World *w;
  Room *r;
  int ri;
  Vector3 op[ROOM_OBJS], orot[ROOM_OBJS], os[ROOM_OBJS]; // objects, local
  char set[ROOM_OBJS];                                   // made by this fill
} F;

#define RD(i) (f->r->door[i])
#define D(i) (&f->w->doors[i])

Vector3 floc(F *f, float x, float y, float z); // local -> world
// CreateDoor(x, y, z, angle, open, big, keycard, code): its index
int fdoor(F *f, float x, float y, float z, float a, int open, int kind, int key,
          const char *code);
void fbtn(F *f, int d, int b, float x, float y,
          float z); // PositionEntity(button, global), KEEP keeps
void fbyaw(F *f, int d, int b,
           float a); // RotateEntity(button, 0, a, 0, global)
void fbturn(F *f, int d, int b, float a);  // TurnEntity(button, 0, a, 0)
void fbmove(F *f, int d, int b, float dz); // MoveEntity(button, 0, 0, dz)
void fnobtn(F *f, int d, int b);           // FreeEntity(button)
void flink(F *f, int a, int b);            // LinkedDoor both ways

void fpiv(F *f, int o);                     // a pivot in Objects[o]
void fmesh(F *f, int o, const char *model); // LoadMesh -> Objects[o]
void fcopy(F *f, int o, int from);          // CopyEntity(Objects[from])
void fopos(F *f, int o, float x, float y, float z);
void forot(F *f, int o, float p, float yaw, float roll);
void foturn(F *f, int o, float p, float yaw, float roll);
void foscale(F *f, int o, float x, float y, float z);
void fcoll(F *f, int o); // EntityType HIT_MAP
void fhide(F *f, int o); // HideEntity / alpha 0
float fox(F *f, int o), foy(F *f, int o),
    foz(F *f, int o); // EntityX/Y/Z, local

int fitem(F *f, const char *name, const char *tmpl, float x, float y,
          float z); // CreateItem
void fiyaw(F *f, int it, float a);
void fistate(F *f, int it, float s);
void finame(F *f, int it, const char *name);
void fihide(F *f, int it);
Vector3 fleafscale(void); // the normal door leaf's scale (CreateDoor's)

int fnamed(F *f, const char *name); // the first room called name, -1 if none
int fcell(F *f, int dx,
          int dz); // is there a room dx, dz cells over (the original's x, z)
const char *fcode(int n); // n as a door code

// fill1.c .. fill3.c: one per template, by name, {0}-terminated
typedef struct {
  const char *name;
  void (*fn)(F *f);
} Fill;
extern const Fill fills1[], fills2[], fills3[];
