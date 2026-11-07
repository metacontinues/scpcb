// room.h: a baked room mesh (tools/bake/room.c has the file format).
// everything's in room space, a placed room just draws w/ its own transform
#pragma once
#include "raylib.h"

typedef struct {
  Vector3 p, dir;
  float range;
  Color c;
  int spot, inner, outer;
} RLight;
typedef struct {
  Vector3 p;
  int id;
  float range;
} RSound;
typedef struct {
  Vector3 p;
  char img[128];
} RScreen;
typedef struct {
  char model[128];
  Matrix m;
} RProp;
typedef struct {
  char name[64];
  BoundingBox box;
} RTrig;

typedef struct {
  Model vis, col;
  int *kind; // per vis material (MK_*)
  RLight *lights;
  int nlights;
  RSound *sounds;
  int nsounds;
  RScreen *screens;
  int nscreens;
  RProp *props;
  int nprops;
  RTrig *trigs;
  int ntrigs;
  Vector3 *wps;
  int nwps;
  int hasstart;
  Vector3 start, startdir;
} RoomMesh;

int room_load(RoomMesh *r, const char *id); // 0 on failure
void room_free(RoomMesh *r);
// opaque pass, then alpha pass (call between BeginBlendMode(BLEND_ALPHA) +
// rlDisableDepthMask)
void room_draw(const RoomMesh *r, Matrix m, int alpha);
