// world.h: the facility. a room is a template's baked mesh drawn w/ its own
// matrix (every copy of a template shares one model). grid cell (gx, gy) sits
// at world (gx * 8, 0, -gy * 8): the original's +z is our -z
#pragma once
#include "gfx/room.h"
#include "raylib.h"

enum { ROOM1, ROOM2, ROOM2C, ROOM3, ROOM4, NSHAPES };
enum { DOOR_NORMAL, DOOR_BIG, DOOR_HEAVY, DOOR_ELEVATOR };

typedef struct {
  char name[64], mesh[128]; // mesh: baked asset id, "" = no geometry
  int shape, zone[5], common, large, nooverlap;
  RoomMesh rm;
  int loaded;
  BoundingBox ext; // room space, from the visible mesh
} RoomTmpl;

// a room's Objects[n]: a model placed by its fill code, or just a point
// (model "") events use later. m is world space
typedef struct {
  char model[64];
  Matrix m;
  int on, coll, hide; // on: the slot's used
} RoomObj;

enum { ROOM_OBJS = 31, ROOM_DOORS = 8 }; // blitz arrays: [30], [7]

typedef struct {
  RoomTmpl *t;
  int gx, gy, zone,
      angle; // angle: yaw in degrees, raylib's sense (MatrixRotateY)
  Vector3 pos;
  Matrix m;
  BoundingBox ext; // world, shrunk a bit, for the overlap fix
  RoomObj obj[ROOM_OBJS];
  int door[ROOM_DOORS]; // RoomDoors: indices into World.doors, -1 = none
  int lever[12];        // Levers: the handle's Objects slot
} Room;

typedef struct {
  Vector3 pos;
  int angle, kind, room, autoclose;
  int open;            // where it's headed
  float state;         // 0 closed .. 180 open
  int key;             // keycard level > 0, hand scanner < 0 (-1 / -2), 0 plain
  char code[8];        // keypad code, "" = none
  int locked, link;    // link: the door that opens w/ this one, -1 none
  float timer, tleft;  // open doors close again after timer ticks
  int nowp, mtf, one;  // DisableWaypoint, MTFClose, one leaf only
  int btn[2], byaw[2]; // has button i + its yaw
  Vector3 bp[2];       // button positions
} Door;

// an item (play/item.c): lying around, or carried (picked)
typedef struct {
  char name[48], tmpl[24];
  int t;       // its template + 1, 0 = not looked up yet
  Vector3 pos; // the original's collider
  float yaw, state, state2;
  Vector3 v; // speed per tick (DropSpeed + x/zspeed)
  int room, hidden, picked, gone;
} Item;

typedef struct {
  RoomTmpl *tmpl;
  int ntmpl;
  Room *rooms;
  int nrooms;
  Door *doors;
  int ndoors;
  Item *items;
  int nitems;
  int w, h; // grid size ("map size" in options.ini)
  Vector3 spawn;
  float spawnyaw;
  int accesscode; // the 4 digits the facility's papers give away
  int gwbroken;   // a room2gw got the broken door already
} World;

int world_templates(World *w, const char *path); // 0 if the file's missing
void world_gen(World *w, unsigned seed);         // mapgen.c
void world_free(World *w);
RoomTmpl *world_tmpl(World *w, const char *name);
Matrix room_matrix(Vector3 pos, int angle);
void world_fill(World *w); // fill.c: every room's FillRoom
