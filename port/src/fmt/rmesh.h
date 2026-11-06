// rmesh.h: scpcb room meshes. mirrors LoadRMesh in MapSystem.bb, minus the
// game-side bits (lights/waypoints/etc come back as plain records).
#pragma once
#include "../bb/bb.h"

enum { RP_SCREEN, RP_WAYPOINT, RP_LIGHT, RP_SPOT, RP_SOUND, RP_START, RP_MODEL };

typedef struct {
	int kind;
	float x, y, z;       // already * room scale for screen/waypoint/light/spot/sound
	float range;         // light: /2000, sound: raw
	int r, g, b;         // light color * intensity
	float pitch, yaw, roll;
	int inner, outer;    // spot cone
	int snd;             // sound emitter id
	char str[256];       // screen img / model file
	ent ent;             // model: the loaded prop
} rpoint;

typedef struct {
	ent obj;     // root pivot: opaque mesh (alpha merged in, props under it), hidden, coll
	ent coll;    // pivot of collision child meshes (hidden-alpha'd, double-sided)
	ent hidden;  // invisible collision mesh
	int ntrig; ent trig[32]; char trigname[32][64];
	rpoint *pts; int npts;
} rmesh;

#define ROOMSCALE (8.0f / 2048.0f)
// amb: the shared "AmbientLightRoomTex" (0 = none). returns 0 on failure.
int rmesh_load(const char *path, htex amb, rmesh *out);
void rmesh_free(rmesh *r);
