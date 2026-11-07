// door.h: sliding doors: normal, big (containment), heavy, elevator
#pragma once
#include "raylib.h"
#include "world.h"

void door_init(void); // the door models
// a door w/ the original's buttons for its kind; its index. autoclose like
// CreateDoor (1 in 8 open normal doors)
int door_add(World *w, Vector3 p, int angle, int kind, int room, int open);
void door_solids(World *w); // static collision, before coll_build
// closest usable button's door, -1 if none; *at = the button
int door_button(World *w, Vector3 eye, Vector3 body, Vector3 *at);
void door_use(World *w, int i);
void door_tick(World *w, Vector3 eye); // slide + auto-close
void door_coll(World *w, Vector3 near,
               float r); // leaves within r -> dynamic collision
void door_draw(World *w, Vector3 eye, float r, int alpha);
