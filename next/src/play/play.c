// play.c: one game: generate the facility, the loading screen, then walk it
// (doors, items, blinking) until it's quit. run from the repo root.
//
// dev env:
//   SCPCB_LOADING=1     w/ SCPCB_SEED: show the loading screen anyway
//                       (+ SCPCB_SHOT: shoot it once loaded)
//   SCPCB_WALK=ticks    hold "forward" that long; SCPCB_SPRINT=1 sprints
//   SCPCB_YAW=deg       start facing
//   SCPCB_ROOM=name     start in the middle of the first room w/ that name
//   SCPCB_ITEM=name     start next to the first item called that
//   SCPCB_DOOR=1|2|3    start at the nearest door's button (2: in front of
//                       the door, 3: same, opened)
//   SCPCB_CLICK=frame   click then (doors, items)
//   SCPCB_GIVE=n|t,...  start w/ these items (name|tempname, next/items.txt)
//   SCPCB_HOLD=n        start holding slot n's item
//   SCPCB_INV=1         start w/ the inventory open
//   SCPCB_PAUSE=1       start paused (for shots of the pause menu)
//   SCPCB_MSG=text      a hud message
//   SCPCB_DEBUG=1       the debug line (f2 toggles it)
#define _POSIX_C_SOURCE 200809L // strcasecmp
#include "play/play.h"
#include "audio/sfx.h"
#include "audio/snd.h"
#include "core/ini.h"
#include "core/util.h"
#include "gfx/props.h"
#include "gfx/shd.h"
#include "play/difficulty.h"
#include "play/player.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "ui/hud.h"
#include "ui/inv.h"
#include "ui/loading.h"
#include "ui/menu.h"
#include "ui/ui.h"
#include "world/coll.h"
#include "world/door.h"
#include "world/world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "play/item.h"

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

static HudIn hudin(const Player *p, int btn, Vector3 at, const Item *item) {
  return (HudIn){.dark = player_dark(p),
                 .blink = p->blink,
                 .blinkfreq = p->blinkfreq,
                 .stamina = p->stamina,
                 .crouch = p->crouch,
                 .hand = btn >= 0,
                 .handyaw = handyaw(p, at),
                 .handpitch = handpitch(p, at),
                 .item = item != NULL,
                 .itemyaw = item ? handyaw(p, item->pos) : 0,
                 .itempitch = item ? handpitch(p, item->pos) : 0};
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
    item_draw(w, eye, HIDE * .5f, pass);
    if (pass)
      rlEnableDepthMask();
  }
}

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
MenuAct play(Settings *s, const NewGame *ng, unsigned seed, int dev,
             const char *shot, int shotf) {
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
  const char *in = getenv("SCPCB_ITEM");
  for (int i = 0; in && i < w.nitems;
       i++) // dev: next to that item, looking down at it
    if (!strcmp(w.items[i].name, in)) {
      Vector3 at = w.items[i].pos;
      player_spawn(&p, (Vector3){at.x, at.y + .2f, at.z + .5f}, 0);
      p.pitch = 40;
      printf("item %s at %.2f %.2f %.2f\n", in, at.x, at.y, at.z);
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
  Inv inv;
  inv_reset(&inv);
  // dev: "name|tempname,..." in your pockets
  char give[1024];
  snprintf(give, sizeof give, "%s",
           getenv("SCPCB_GIVE") ? getenv("SCPCB_GIVE") : "");
  for (char *g = strtok(give, ","); g; g = strtok(NULL, ",")) {
    char *bar = strchr(g, '|');
    if (bar)
      *bar = 0, item_pick(&w, &inv, item_new(&w, g, bar + 1, p.pos));
  }
  const char *hold = getenv("SCPCB_HOLD"); // dev: that slot's item in your hand
  if (hold)
    inv.sel = inv.slot[abs(atoi(hold)) % INV_SLOTS];
  if (getenv("SCPCB_INV")) // dev: start w/ the inventory open
    inv.open = 1;
  if (getenv("SCPCB_MSG")) // dev: show a message
    hud_msg(getenv("SCPCB_MSG"), 70 * 6);
  if (!shot && !paused && !inv.open)
    DisableCursor();
  else if (paused || inv.open)
    HideCursor();
  Vector2 mouse = {0};
  for (int f = 0; !WindowShouldClose(); f++) {
    int wasinv = inv.open;
    if (IsKeyPressed(KEY_ESCAPE)) {
      if (inv.open)
        inv.open = 0;
      else
        paused = !paused;
    } else if (!paused && IsKeyPressed(settings_key(s, K_INV))) {
      inv.open = !inv.open;
      inv.sel = -1;
    }
    if (IsKeyPressed(KEY_ESCAPE) || inv.open != wasinv) {
      if (paused || inv.open) {
        EnableCursor();
        HideCursor(); // ui_cursor draws the original's
      } else
        DisableCursor();
      mouse = Vector2Zero();
    }
    int frozen = paused || inv.open; // the original's FPSfactor = 0
    if (!frozen) {
      acc = minf(acc + (shot ? 1 / TICK : GetFrameTime()), .25f);
      mouse = Vector2Add(mouse, shot ? Vector2Zero() : GetMouseDelta());
      crouch |= IsKeyPressed(settings_key(s, K_CROUCH));
      blink |= IsKeyPressed(settings_key(s, K_BLINK));
    }
    while (!frozen && acc >= 1 / TICK) {
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
      item_tick(&w, player_eye(&p));
      snd_listener(player_eye(&p), p.yaw);
      sfx_tick(&w, &p,
               IsKeyDown(settings_key(s, K_SPRINT)) || (fw && sprintdev));
      snd_tick();
      hud_tick(&(HudIn){
          .stamina = p.stamina, .blink = p.blink, .blinkfreq = p.blinkfreq});
      acc -= 1 / TICK;
      ticks++;
    }
    btn = frozen ? -1 : door_button(&w, player_eye(&p), p.pos, &btnat);
    int near =
        frozen ? -1 : item_closest(&w, player_eye(&p), player_forward(&p));
    HudIn hi = hudin(&p, btn, btnat, near >= 0 ? &w.items[near] : NULL);
    int click1 =
        !frozen && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || f == click);
    if (btn >= 0 && click1) {
      snd_play3d("sfx/interact/button", btnat, 10, 1);
      if (door_use(&w, btn, inv.sel >= 0 ? item_kind(&w.items[inv.sel]) : NULL))
        inv.sel = -1;
    } else if (near >= 0 && click1)
      item_pick(&w, &inv, near);
    snd_frame();

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(player_camera(&p, fovy));
    draw(&w, player_eye(&p));
    EndMode3D();
    if (!paused)
      hud_draw(&hi);
    if (inv.open && !paused) {
      inv_screen(&w, &inv, player_eye(&p), p.yaw, p.pitch);
      if (!inv.open) { // used or dropped something: back to looking around
        DisableCursor();
        mouse = Vector2Zero();
      }
    } else if (!paused)
      inv_hand(&w, &inv);
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
      for (int i = 0; i < INV_SLOTS; i++)
        if (inv.slot[i] >= 0)
          printf("slot %d: %s\n", i, w.items[inv.slot[i]].name);
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
