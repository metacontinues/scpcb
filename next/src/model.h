// model.h: a baked model (tools/bake/model.c has the file format): raylib Model + its animation, if any.
// skinned models pose on the cpu (UpdateModelAnimation), so one Mdl shown in several poses gets posed per draw
#pragma once
#include "raylib.h"

typedef struct { int alpha, twosided; BlendMode blend; } MdlMat; // per raylib material

typedef struct {
	Model m;
	MdlMat *mat;
	ModelAnimation *anims; int nanims;
	int frames;
} Mdl;

int mdl_load(Mdl *o, const char *id); // 0 on failure
void mdl_free(Mdl *o);
void mdl_pose(Mdl *o, float frame);   // no-op w/o an animation
void mdl_draw(const Mdl *o, Matrix m, int alpha); // opaque pass, then the alpha pass (depth writes off)
int mdl_bone(const Mdl *o, const char *name);      // -1 if missing
