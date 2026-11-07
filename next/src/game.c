// game.c: the game so far: generate the facility, drop you in the start room,
// walk it, open doors, blink. run from the repo root (it reads options.ini,
// Data/rooms.ini and next/data).
//
// dev env:
//   SCPCB_SEED=n           fixed map
//   SCPCB_SHOT=a.png       screenshot at frame SCPCB_SHOT_FRAME (60), then quit
//   SCPCB_WALK=ticks       hold "forward" that long
//   SCPCB_YAW=deg          start facing
//   SCPCB_NOFOG=1          see past the fog
//   SCPCB_DOOR=1|2|3       start at the nearest door's button (2: in front of
//                          the door, 3: same, opened); SCPCB_CLICK=frame clicks
//   SCPCB_MSG=text         a hud message
//   SCPCB_DEBUG=1          the debug line (f2 toggles it)
//   SCPCB_SPRINT=1         sprint while SCPCB_WALK walks
//   SCPCB_HUD=mode         auto|always|off, over options.ini's "hud mode"
#define _POSIX_C_SOURCE 200809L // strcasecmp
#include "coll.h"
#include "door.h"
#include "hud.h"
#include "ini.h"
#include "player.h"
#include "props.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "shd.h"
#include "util.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#define HIDE                                                                   \
  15.0f // rooms + doors further than this aren't drawn (HideDistance)

// where a point is relative to the view, in the original's hand-icon terms:
// yaw > 0 = to the right, pitch > 0 = above
static float handyaw(const Player *p, Vector3 at) {
  Vector3 d = Vector3Subtract(at, player_eye(p));
  return p->yaw - atan2f(-d.x, -d.z) * RAD2DEG;
}
static float handpitch(const Player *p, Vector3 at) {
  Vector3 d = Vector3Subtract(at, player_eye(p));
  return p->pitch - atan2f(-d.y, sqrtf(d.x * d.x + d.z * d.z)) * RAD2DEG;
}

static HudIn hudin(const Player *p, int btn, Vector3 at) {
  return (HudIn){.dark = player_dark(p),
                 .blink = p->blink,
                 .blinkfreq = p->blinkfreq,
                 .stamina = p->stamina,
                 .crouch = p->crouch,
                 .hand = btn >= 0,
                 .handyaw = handyaw(p, at),
                 .handpitch = handpitch(p, at)};
}

// "hud mode = auto|always|off", else the original's "HUD enabled" (1 -> auto)
static int hudmode(const Ini *o) {
  const char *m = ini_str(o, "options", "hud mode", NULL);
  if (!m)
    return ini_int(o, "options", "hud enabled", 1) ? HUD_AUTO : HUD_OFF;
  return !strcasecmp(m, "off")      ? HUD_OFF
         : !strcasecmp(m, "always") ? HUD_ALWAYS
                                    : HUD_AUTO;
}

static float envf(const char *k, float def) {
  const char *v = getenv(k);
  return v ? (float)atof(v) : def;
}

// everything solid: every room's collision mesh + its props
static void solids(World *w) {
  coll_reset();
  for (int i = 0; i < w->nrooms; i++) {
    Room *r = &w->rooms[i];
    if (!r->t || r->t->loaded <= 0)
      continue;
    for (int m = 0; m < r->t->rm.col.meshCount; m++)
      coll_mesh(r->t->rm.col.meshes[m], r->m);
    for (int p = 0; p < r->t->rm.nprops; p++) {
      Mdl *pm = prop_get(r->t->rm.props[p].model);
      if (!pm)
        continue;
      Matrix t = MatrixMultiply(r->t->rm.props[p].m, r->m);
      for (int m = 0; m < pm->m.meshCount; m++)
        coll_mesh(pm->m.meshes[m], t);
    }
  }
  coll_build();
}

// big rooms reach far past their origin, so cull by the nearest point of the
// box
static float boxdist(Vector3 p, BoundingBox b) {
  return Vector3Distance(p, Vector3Clamp(p, b.min, b.max));
}

static void draw(World *w, Vector3 eye) {
  for (int pass = 0; pass < 2; pass++) {
    if (pass)
      rlDisableDepthMask();
    for (int i = 0; i < w->nrooms; i++) {
      Room *r = &w->rooms[i];
      if (!r->t || r->t->loaded <= 0 || boxdist(eye, r->ext) > HIDE)
        continue;
      if (pass)
        BeginBlendMode(BLEND_ALPHA);
      room_draw(&r->t->rm, r->m, pass);
      if (pass)
        EndBlendMode();
      for (int p = 0; p < r->t->rm.nprops; p++) {
        Mdl *pm = prop_get(r->t->rm.props[p].model);
        if (pm)
          mdl_draw(pm, MatrixMultiply(r->t->rm.props[p].m, r->m), pass);
      }
    }
    door_draw(w, eye, HIDE, pass);
    if (pass)
      rlEnableDepthMask();
  }
}

int main(void) {
  Ini opt;
  ini_load(&opt, "options.ini");
  int sw = ini_int(&opt, "options", "width", 1280),
      sh = ini_int(&opt, "options", "height", 720);
  const char *shot = getenv("SCPCB_SHOT");
  int shotf = (int)envf("SCPCB_SHOT_FRAME", 60),
      walk = (int)envf("SCPCB_WALK", 0);
  SetTraceLogLevel(getenv("SCPCB_LOG") ? LOG_INFO : LOG_WARNING);
  SetConfigFlags(FLAG_VSYNC_HINT);
#ifdef SCPCB_ANGLE
  SetConfigFlags(FLAG_WINDOW_TRANSPARENT); // see tools/view.c
#endif
  InitWindow(sw, sh, "SCP - Containment Breach");
  if (!shot)
    DisableCursor();
  shd_init();
  float fogn = ini_float(&opt, "options", "camera fog near", .5f),
        fogf = ini_float(&opt, "options", "camera fog far", 6);
  if (getenv("SCPCB_NOFOG")) // dev: see past the fog
    fogf = 60, fogn = 59;
  shd_fog(fogn, fogf, BLACK);
  shd_lights((Vector3){.5f, .5f, .5f}, 0, NULL,
             NULL); // placeholder until rooms light their props/npcs
  rlSetClipPlanes(.05f, fogf);

  World w = {0};
  w.w = w.h = ini_int(&opt, "options", "map size", 18);
  if (!world_templates(&w, "Data/rooms.ini")) {
    TraceLog(LOG_ERROR, "game: no Data/rooms.ini, run me from the repo root");
    CloseWindow();
    return 1;
  }
  const char *seeds = getenv("SCPCB_SEED");
  unsigned seed = seeds ? (unsigned)atol(seeds) : (unsigned)time(NULL);
  world_gen(&w, seed);
  solids(&w);
  door_init();
  printf("seed %u: %d rooms, %d doors\n", seed, w.nrooms, w.ndoors);

  Player p = {.sens = ini_float(&opt, "options", "mouse sensitivity", 0),
              .smooth = ini_float(&opt, "options", "mouse smoothing", 1),
              .invert = ini_int(&opt, "options", "invert mouse y", 0)};
  player_spawn(&p, w.spawn, envf("SCPCB_YAW", w.spawnyaw));
  if (getenv("SCPCB_DOOR")) { // dev: stand in front of the nearest door's
                              // button, facing it
    int best = -1;
    for (int i = 0; i < w.ndoors; i++)
      if (best < 0 || Vector3Distance(w.doors[i].pos, w.spawn) <
                          Vector3Distance(w.doors[best].pos, w.spawn))
        best = i;
    if (best >= 0) {
      Door *d = &w.doors[best];
      Matrix t = MatrixMultiply(MatrixRotateY(d->angle * DEG2RAD),
                                MatrixTranslate(d->pos.x, d->pos.y, d->pos.z));
      int mode = (int)envf(
          "SCPCB_DOOR",
          1); // 2: 1.5 in front of it, facing through. 3: same, opened
      player_spawn(&p,
                   Vector3Transform(mode > 1 ? (Vector3){0, .5f, 1.5f}
                                             : (Vector3){.6f, .5f, .55f},
                                    t),
                   d->angle);
      p.pitch = mode > 1 ? 0 : 20;
      if (mode == 3)
        d->open = 1, d->state = 180;
      printf("door %d at %.1f %.1f, open %d\n", best, d->pos.x, d->pos.z,
             d->open);
    }
  }
  int click = (int)envf("SCPCB_CLICK", -1);
  int kup = key_dik(ini_int(&opt, "options", "up key", 17)),
      kdown = key_dik(ini_int(&opt, "options", "down key", 31)),
      kleft = key_dik(ini_int(&opt, "options", "left key", 30)),
      kright = key_dik(ini_int(&opt, "options", "right key", 32)),
      ksprint = key_dik(ini_int(&opt, "options", "sprint key", 42)),
      kblink = key_dik(ini_int(&opt, "options", "blink key", 57)),
      kcrouch = key_dik(ini_int(&opt, "options", "crouch key", 29));
  int hud = hudmode(&opt), sprintdev = getenv("SCPCB_SPRINT") != NULL;
  if (getenv("SCPCB_HUD")) { // dev: auto|always|off
    Ini o = {0};
    IniKV kv = {"options", "hud mode", getenv("SCPCB_HUD")};
    o.kv = &kv;
    o.n = 1;
    hud = hudmode(&o);
  }
  ini_free(&opt);
  float fovy = 2 * atanf(tanf(45 * DEG2RAD) * sh / (float)sw) *
               RAD2DEG; // the original's 90 degrees across

  float acc = 0;
  int ticks = 0, crouch = 0, blink = 0, btn = -1,
      debug = getenv("SCPCB_DEBUG") != NULL;
  Vector3 btnat = {0};
  hud_init(sw, sh, hud);
  if (getenv("SCPCB_MSG")) // dev: show a message
    hud_msg(getenv("SCPCB_MSG"), 70 * 6);
  Vector2 mouse = {0};
  for (int f = 0; !WindowShouldClose(); f++) {
    acc = minf(acc + (shot ? 1 / TICK : GetFrameTime()), .25f);
    mouse = Vector2Add(mouse, shot ? Vector2Zero() : GetMouseDelta());
    crouch |= IsKeyPressed(kcrouch);
    blink |= IsKeyPressed(kblink);
    while (acc >= 1 / TICK) {
      int fw = ticks < walk;
      PlayerIn in = {.up = IsKeyDown(kup) || fw,
                     .down = IsKeyDown(kdown),
                     .left = IsKeyDown(kleft),
                     .right = IsKeyDown(kright),
                     .sprint = IsKeyDown(ksprint) || (fw && sprintdev),
                     .crouch = crouch,
                     .blink = blink,
                     .blinkheld = IsKeyDown(kblink),
                     .mouse = mouse};
      mouse = Vector2Zero();
      crouch = blink = 0;
      door_coll(&w, p.pos, 4);
      player_tick(&p, in);
      door_tick(&w, player_eye(&p));
      hud_tick(&(HudIn){
          .stamina = p.stamina, .blink = p.blink, .blinkfreq = p.blinkfreq});
      acc -= 1 / TICK;
      ticks++;
    }
    btn = door_button(&w, player_eye(&p), p.pos, &btnat);
    HudIn hi = hudin(&p, btn, btnat);
    if (btn >= 0 && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || f == click))
      door_use(&w, btn);

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(player_camera(&p, fovy));
    draw(&w, player_eye(&p));
    EndMode3D();
    hud_draw(&hi);
    if (IsKeyPressed(KEY_F2))
      debug = !debug;
    if (debug)
      DrawText(TextFormat("%d fps  (%.2f %.2f %.2f)  stamina %.0f", GetFPS(),
                          p.pos.x, p.pos.y, p.pos.z, p.stamina),
               10, 10, 20, DARKGRAY);
    EndDrawing();
    if (shot && f == shotf) {
      TakeScreenshot(shot);
      printf("pos %.2f %.2f %.2f, button %d, stamina %.0f, blink %.0f/%.0f\n",
             p.pos.x, p.pos.y, p.pos.z, btn, p.stamina, p.blink, p.blinkfreq);
      for (int i = 0; i < w.ndoors; i++)
        if (Vector3Distance(w.doors[i].pos, p.pos) < 3)
          printf("door %d: open %d state %.0f\n", i, w.doors[i].open,
                 w.doors[i].state);
      break;
    }
  }
  prop_clear();
  world_free(&w);
  coll_reset();
  CloseWindow();
  return 0;
}
