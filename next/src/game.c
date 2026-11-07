// game.c: the game so far: generate the facility, drop you in the start room,
// walk it, open doors, blink. run from the repo root (it reads options.ini,
// Data/rooms.ini and next/data).
//
// states: main menu -> loading screen -> play (esc pauses) -> back to the menu.
//
// dev env:
//   SCPCB_SEED=n           fixed map, skips the menu + loading screen
//   SCPCB_LOADING=1        w/ SCPCB_SEED: the loading screen anyway (+
//   SCPCB_SHOT:
//                          shoot it once loaded)
//   SCPCB_TAB=n            open the menu on tab n (1 new game, 2 load, 3
//   options) SCPCB_OPT=n            + the options on tab n (0 graphics .. 3
//   advanced) SCPCB_SHOT=a.png       screenshot at frame SCPCB_SHOT_FRAME (60),
//   then quit
//                          (in the menu too, w/o SCPCB_SEED)
//   SCPCB_WALK=ticks       hold "forward" that long
//   SCPCB_YAW=deg          start facing
//   SCPCB_ROOM=name        start in the middle of the first room w/ that name
//   SCPCB_NOFOG=1          see past the fog
//   SCPCB_DOOR=1|2|3       start at the nearest door's button (2: in front of
//                          the door, 3: same, opened); SCPCB_CLICK=frame clicks
//   SCPCB_MSG=text         a hud message
//   SCPCB_DEBUG=1          the debug line (f2 toggles it)
//   SCPCB_SPRINT=1         sprint while SCPCB_WALK walks
//   SCPCB_HUD=mode         auto|always|off, over options.ini's "hud mode"
//   SCPCB_NOMUSIC=1        no music; SCPCB_SNDLOG=1 prints every sound played
//   SCPCB_PAUSE=1          start paused (for shots of the pause menu)
#define _POSIX_C_SOURCE 200809L // strcasecmp
#include "coll.h"
#include "difficulty.h"
#include "door.h"
#include "hud.h"
#include "ini.h"
#include "loading.h"
#include "menu.h"
#include "player.h"
#include "props.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "sfx.h"
#include "shd.h"
#include "snd.h"
#include "ui.h"
#include "util.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    for (int o = 0; o < ROOM_OBJS; o++) { // FillRoom's, hidden hitboxes too
      Mdl *om = r->obj[o].coll && r->obj[o].model[0] ? prop_get(r->obj[o].model)
                                                     : NULL;
      for (int m = 0; om && m < om->m.meshCount; m++)
        coll_mesh(om->m.meshes[m], r->obj[o].m);
    }
  }
  door_solids(w);
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
      for (int o = 0; o < ROOM_OBJS; o++) {
        RoomObj *ob = &r->obj[o];
        Mdl *om =
            ob->on && !ob->hide && ob->model[0] ? prop_get(ob->model) : NULL;
        if (om)
          mdl_draw(om, ob->m, pass);
      }
    }
    door_draw(w, eye, HIDE, pass);
    if (pass)
      rlEnableDepthMask();
  }
}

// the original's GenerateSeedNumber: each char xor'd in, shifted 0..23
static unsigned seednum(const char *s) {
  unsigned t = 0;
  for (int sh = 0; *s; s++, sh = (sh + 1) % 24)
    t ^= (unsigned)(unsigned char)*s << sh;
  return t;
}

static const char *shot;
static int shotf;

// a frame of the loading screen
static void loadframe(const Loading *l, int pct) {
  BeginDrawing();
  loading_draw(l, pct);
  EndDrawing();
}

// settings that apply live (pause menu's options)
static void apply(const Settings *s, Player *p) {
  snd_volumes(s->sound, getenv("SCPCB_NOMUSIC") ? 0 : s->music);
  hud_mode(s->hud);
  settings_apply(s);
  p->sens = s->sens;
  p->smooth = s->smooth > 0 ? s->smooth : 1;
  p->invert = s->invert;
}

// one game, from the loading screen until it's quit. MENU_TOMENU or MENU_QUIT
static MenuAct play(Settings *s, const NewGame *ng, unsigned seed, int dev) {
  int sw = GetScreenWidth(), sh = GetScreenHeight();
  Loading ld;
  loading_pick(&ld);
  if (!dev)
    loadframe(&ld, 0);
  World w = {0};
  w.w = w.h = s->mapsize;
  world_templates(&w, "Data/rooms.ini");
  world_gen(&w, seed);
  if (!dev)
    loadframe(&ld, 80);
  solids(&w);
  door_init();
  printf("seed %u: %d rooms, %d doors\n", seed, w.nrooms, w.ndoors);

  Player p = {0};
  blink_range(ng->d.otherfactors, &p.blinklo, &p.blinkhi);
  apply(s, &p);
  player_spawn(&p, w.spawn, envf("SCPCB_YAW", w.spawnyaw));
  const char *rn = getenv("SCPCB_ROOM");
  for (int i = 0; rn && i < w.nrooms;
       i++) // dev: start in the middle of that room, SCPCB_YAW relative to it
    if (w.rooms[i].t && !strcmp(w.rooms[i].t->name, rn)) {
      player_spawn(&p, Vector3Add(w.rooms[i].pos, (Vector3){0, .5f, 0}),
                   envf("SCPCB_YAW", 0) + w.rooms[i].angle);
      printf("room %s at %.0f %.0f, angle %d\n", rn, w.rooms[i].pos.x,
             w.rooms[i].pos.z, w.rooms[i].angle);
      break;
    }
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
  // press any key, like the original
  for (int f = 0; !dev && !WindowShouldClose(); f++) {
    loadframe(&ld, 100);
    if (shot &&
        f == 3) { // dev: SCPCB_LOADING. a few frames in, the swap's settled
      TakeScreenshot(shot);
      dev = -1;
      break;
    }
    if (GetKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      break;
  }

  MenuAct out = MENU_QUIT;
  if (dev < 0)
    goto done;
  int click = (int)envf("SCPCB_CLICK", -1), walk = (int)envf("SCPCB_WALK", 0);
  int sprintdev = getenv("SCPCB_SPRINT") != NULL;
  float fovy = 2 * atanf(tanf(45 * DEG2RAD) * sh / (float)sw) *
               RAD2DEG; // the original's 90 degrees across

  float acc = 0;
  int ticks = 0, crouch = 0, blink = 0, btn = -1,
      debug = getenv("SCPCB_DEBUG") != NULL,
      paused = getenv("SCPCB_PAUSE") != NULL;
  Vector3 btnat = {0};
  if (getenv("SCPCB_MSG")) // dev: show a message
    hud_msg(getenv("SCPCB_MSG"), 70 * 6);
  if (!shot && !paused)
    DisableCursor();
  else if (paused)
    HideCursor();
  Vector2 mouse = {0};
  for (int f = 0; !WindowShouldClose(); f++) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      paused = !paused;
      if (paused) {
        EnableCursor();
        HideCursor(); // ui_cursor draws the original's
      } else
        DisableCursor();
      mouse = Vector2Zero();
    }
    if (!paused) {
      acc = minf(acc + (shot ? 1 / TICK : GetFrameTime()), .25f);
      mouse = Vector2Add(mouse, shot ? Vector2Zero() : GetMouseDelta());
      crouch |= IsKeyPressed(settings_key(s, K_CROUCH));
      blink |= IsKeyPressed(settings_key(s, K_BLINK));
    }
    while (!paused && acc >= 1 / TICK) {
      int fw = ticks < walk;
      PlayerIn in = {.up = IsKeyDown(settings_key(s, K_UP)) || fw,
                     .down = IsKeyDown(settings_key(s, K_DOWN)),
                     .left = IsKeyDown(settings_key(s, K_LEFT)),
                     .right = IsKeyDown(settings_key(s, K_RIGHT)),
                     .sprint = IsKeyDown(settings_key(s, K_SPRINT)) ||
                               (fw && sprintdev),
                     .crouch = crouch,
                     .blink = blink,
                     .blinkheld = IsKeyDown(settings_key(s, K_BLINK)),
                     .mouse = mouse};
      mouse = Vector2Zero();
      crouch = blink = 0;
      door_coll(&w, p.pos, 4);
      player_tick(&p, in);
      door_tick(&w, player_eye(&p));
      snd_listener(player_eye(&p), p.yaw);
      sfx_tick(&w, &p,
               IsKeyDown(settings_key(s, K_SPRINT)) || (fw && sprintdev));
      snd_tick();
      hud_tick(&(HudIn){
          .stamina = p.stamina, .blink = p.blink, .blinkfreq = p.blinkfreq});
      acc -= 1 / TICK;
      ticks++;
    }
    btn = paused ? -1 : door_button(&w, player_eye(&p), p.pos, &btnat);
    HudIn hi = hudin(&p, btn, btnat);
    if (btn >= 0 && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || f == click)) {
      snd_play3d("sfx/interact/button", btnat, 10, 1);
      door_use(&w, btn);
    }
    snd_frame();

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(player_camera(&p, fovy));
    draw(&w, player_eye(&p));
    EndMode3D();
    if (!paused)
      hud_draw(&hi);
    if (IsKeyPressed(KEY_F2))
      debug = !debug;
    if (debug)
      DrawText(TextFormat("%d fps  (%.2f %.2f %.2f)  stamina %.0f", GetFPS(),
                          p.pos.x, p.pos.y, p.pos.z, p.stamina),
               10, 10, 20, DARKGRAY);
    else if (s->fps)
      DrawText(TextFormat("FPS: %d", GetFPS()), 20, 20, 20, WHITE);
    MenuAct a = MENU_NONE;
    if (paused) {
      a = menu_pause(ng, s, 0);
      apply(s, &p);
      ui_cursor();
    }
    EndDrawing();
    if (a == MENU_RESUME) {
      paused = 0;
      DisableCursor();
      mouse = Vector2Zero();
    } else if (a == MENU_TOMENU) {
      out = MENU_TOMENU;
      break;
    }
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
done:
  EnableCursor();
  prop_clear();
  world_free(&w);
  coll_reset();
  return out;
}

// the main menu until it starts a game or quits
static MenuAct mainmenu(NewGame *ng, Settings *s) {
  HideCursor();
  snd_music("sfx/music/menu");
  float acc = 0;
  for (int f = 0; !WindowShouldClose(); f++) {
    acc = minf(acc + GetFrameTime(), .25f);
    while (acc >= 1 / TICK) {
      snd_tick();
      acc -= 1 / TICK;
    }
    snd_frame();
    BeginDrawing();
    MenuAct a = menu_main(ng, s);
    snd_volumes(s->sound, getenv("SCPCB_NOMUSIC") ? 0 : s->music);
    hud_mode(s->hud);
    settings_apply(s);
    ui_cursor();
    EndDrawing();
    if (shot && f == shotf) {
      TakeScreenshot(shot);
      return MENU_QUIT;
    }
    if (a != MENU_NONE)
      return a;
  }
  return MENU_QUIT;
}

int main(void) {
  Settings s;
  settings_load(&s, "options.ini");
  shot = getenv("SCPCB_SHOT");
  shotf = (int)envf("SCPCB_SHOT_FRAME", 60);
  SetTraceLogLevel(getenv("SCPCB_LOG") ? LOG_INFO : LOG_WARNING);
#ifdef SCPCB_ANGLE
  SetConfigFlags(FLAG_WINDOW_TRANSPARENT); // see tools/view.c
#endif
  InitWindow(s.width, s.height, "SCP - Containment Breach");
  SetExitKey(KEY_NULL); // esc pauses / goes back
  settings_apply(&s);
  shd_init();
  snd_init(s.sound, getenv("SCPCB_NOMUSIC") ? 0 : s.music);
  sfx_init();
  ui_init(s.width, s.height);
  float fogn = s.fognear, fogf = s.fogfar;
  if (getenv("SCPCB_NOFOG")) // dev: see past the fog
    fogf = 60, fogn = 59;
  shd_fog(fogn, fogf, BLACK);
  shd_lights((Vector3){.5f, .5f, .5f}, 0, NULL,
             NULL); // placeholder until rooms light their props/npcs
  rlSetClipPlanes(.05f, fogf);
  World probe = {0};
  if (!world_templates(&probe, "Data/rooms.ini")) {
    TraceLog(LOG_ERROR, "game: no Data/rooms.ini, run me from the repo root");
    CloseWindow();
    return 1;
  }
  world_free(&probe);
  const char *m = getenv("SCPCB_HUD");
  if (m) // dev: auto|always|off
    s.hud = !strcasecmp(m, "off")      ? HUD_OFF
            : !strcasecmp(m, "always") ? HUD_ALWAYS
                                       : HUD_AUTO;
  hud_init(s.width, s.height, s.hud);

  NewGame ng = {.sel = D_SAFE, .d = difficulties[D_SAFE]};
  const char *seeds = getenv("SCPCB_SEED");
  menu_init((int)envf("SCPCB_TAB", 0), (int)envf("SCPCB_OPT", 0));
  for (MenuAct a = MENU_TOMENU; a == MENU_TOMENU;) {
    if (seeds) { // dev: straight in
      snprintf(ng.seed, sizeof ng.seed, "%s", seeds);
      play(&s, &ng, (unsigned)atol(seeds), !getenv("SCPCB_LOADING"));
      break;
    }
    if (mainmenu(&ng, &s) != MENU_START)
      break;
    a = play(&s, &ng, seednum(ng.seed), 0);
    menu_init(0, 0);
  }
  snd_close();
  CloseWindow();
  return 0;
}
