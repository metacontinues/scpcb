// coll.h: the world's collision. static triangles (rooms, props) live in a
// grid, dynamic ones (door leaves) get re-added every tick. movers are
// ellipsoids that slide (double-sided triangles, like the original's rooms)
#pragma once
#include "raylib.h"

void coll_reset(void);            // drops everything
void coll_mesh(Mesh m, Matrix t); // static, before coll_build
void coll_build(void);
void coll_dyn_reset(void);
void coll_dyn_box(BoundingBox b,
                  Matrix t); // 12 triangles, until the next coll_dyn_reset
// move an ellipsoid (radii r) from pos by vel, sliding along whatever it hits.
// *low is set when it touched something below `belowy` (feet on a floor), left
// alone otherwise
Vector3 coll_move(Vector3 pos, Vector3 vel, Vector3 r, float belowy, int *low);
// first hit on the segment a..b, static + dynamic
RayCollision coll_ray(Vector3 a, Vector3 b);
