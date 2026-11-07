// shd.h: the world shaders, one per room material kind (glsl es 100, so angle/es2 runs them)
#pragma once
#include "raylib.h"

enum { MK_LIT, MK_VERT, MK_FLAT, MK_ALPHA, MK_N };
extern Shader shd_world[MK_N];

void shd_init(void);
void shd_fog(float near, float far, Color c); // far <= near: off
void shd_ambient(float a);                    // the rooms' ambient light, 0..1
