// world.h: the facility. a room is a template's baked mesh drawn w/ its own
// matrix (every copy of a template shares one model). grid cell (gx, gy) sits
// at world (gx * 8, 0, -gy * 8): the original's +z is our -z
#pragma once
#include "raylib.h"
#include "room.h"

enum { ROOM1, ROOM2, ROOM2C, ROOM3, ROOM4, NSHAPES };
enum { DOOR_NORMAL, DOOR_BIG, DOOR_HEAVY, DOOR_ELEVATOR };

typedef struct {
  char name[64], mesh[128]; // mesh: baked asset id, "" = no geometry
  int shape, zone[5], common, large, nooverlap;
  RoomMesh rm;
  int loaded;
  BoundingBox ext; // room space, from the visible mesh
} RoomTmpl;

typedef struct {
  RoomTmpl *t;
  int gx, gy, zone,
      angle; // angle: yaw in degrees, raylib's sense (MatrixRotateY)
  Vector3 pos;
  Matrix m;
  BoundingBox ext; // world, shrunk a bit, for the overlap fix
} Room;

typedef struct {
  Vector3 pos;
  int angle, kind, room, autoclose;
  int open;    // where it's headed
  float state; // 0 closed .. 180 open
} Door;

typedef struct {
  RoomTmpl *tmpl;
  int ntmpl;
  Room *rooms;
  int nrooms;
  Door *doors;
  int ndoors;
  int w, h; // grid size ("map size" in options.ini)
  Vector3 spawn;
  float spawnyaw;
} World;

int world_templates(World *w, const char *path); // 0 if the file's missing
void world_gen(World *w, unsigned seed);         // mapgen.c
void world_free(World *w);
RoomTmpl *world_tmpl(World *w, const char *name);
Matrix room_matrix(Vector3 pos, int angle);
