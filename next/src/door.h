// door.h: sliding doors. the original nudged leaves by sin(state)/k every tick;
// summed up that's (1 - cos(state)) * 180 / (pi * 2k), so a leaf's offset is
// just a function of state (0 closed .. 180 open)
#pragma once
#include "raylib.h"
#include "world.h"

void door_init(void); // the door models
// closest usable button's door, -1 if none; *at = the button
int door_button(World *w, Vector3 eye, Vector3 body, Vector3 *at);
void door_use(World *w, int i);
void door_tick(World *w, Vector3 eye); // slide + auto-close
void door_coll(World *w, Vector3 near,
               float r); // leaves within r -> dynamic collision
void door_draw(World *w, Vector3 eye, float r, int alpha);
