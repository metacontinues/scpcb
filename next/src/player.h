// player.h: the player's body + view. ticks at TICK hz w/ the original's
// numbers (speed .018/tick, sprint 2.5x, crouch halves it, gravity
// .006/tick^2), collider is a .15 x .30 ellipsoid
#pragma once
#include "raylib.h"

typedef struct {
  int up, down, left, right, sprint, crouch; // held (crouch: pressed this tick)
  Vector2 mouse;                             // pixels moved this tick
} PlayerIn;

typedef struct {
  Vector3 pos;      // collider centre
  float yaw, pitch; // degrees, raylib's sense; pitch > 0 looks down
  float speed, drop, stamina, shake, crouchstate, msx, msy;
  int crouch, floor;
  float sens, smooth; // options: mouse sensitivity, smoothing
  int invert;
} Player;

void player_spawn(Player *p, Vector3 pos, float yaw);
void player_tick(Player *p, PlayerIn in);
Camera3D player_camera(const Player *p, float fovy);
Vector3 player_eye(const Player *p);
Vector3 player_forward(const Player *p);
int key_dik(int dik); // directinput scancode (options.ini's keys) -> raylib key
