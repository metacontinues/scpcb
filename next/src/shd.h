// shd.h: the world shaders (glsl es 100, so angle/es2 runs them). rooms get one per material kind,
// models one per (fullbright, vertex color) pair: MK_MODEL + (f ? 1 : 0) + (v ? 2 : 0)
#pragma once
#include "raylib.h"

enum { MK_LIT, MK_VERT, MK_FLAT, MK_ALPHA, MK_MODEL, MK_N = MK_MODEL + 4 };
enum { SHD_LIGHTS = 4 };
extern Shader shd_world[MK_N];

void shd_init(void);
void shd_fog(float near, float far, Color c); // far <= near: off
void shd_ambient(float a);                    // the rooms' ambient light, 0..1
// models' lighting: ambient + up to SHD_LIGHTS lights. pos.w: 0 directional (xyz = direction it shines), 1 point;
// col.w = point light range
void shd_lights(Vector3 amb, int n, const Vector4 *pos, const Vector4 *col);
