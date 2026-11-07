// game.c: main: the window, then main menu -> play -> back to the menu.
// run from the repo root (it reads options.ini, Data/rooms.ini, next/data).
//
// dev env (play.c has the in-game ones):
//   SCPCB_SEED=n        fixed map, skips the menu + loading screen
//   SCPCB_TAB=n         open the menu on tab n (1 new game, 2 load, 3 options)
//   SCPCB_OPT=n         + the options on tab n (0 graphics .. 3 advanced)
//   SCPCB_SHOT=a.png    screenshot at frame SCPCB_SHOT_FRAME (60), then quit
//                       (in the menu too, w/o SCPCB_SEED)
//   SCPCB_NOFOG=1       see past the fog
//   SCPCB_HUD=mode      auto|always|off, over options.ini's "hud mode"
//   SCPCB_NOMUSIC=1     no music; SCPCB_SNDLOG=1 prints every sound played
#define _POSIX_C_SOURCE 200809L // strcasecmp
#include "audio/sfx.h"
#include "audio/snd.h"
#include "core/util.h"
#include "gfx/shd.h"
#include "play/difficulty.h"
#include "play/play.h"
#include "raylib.h"
#include "rlgl.h"
#include "ui/hud.h"
#include "ui/inv.h"
#include "ui/menu.h"
#include "ui/ui.h"
#include "world/world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// the original's GenerateSeedNumber: each char xor'd in, shifted 0..23
static unsigned seednum(const char *s) {
  unsigned t = 0;
  for (int sh = 0; *s; s++, sh = (sh + 1) % 24)
    t ^= (unsigned)(unsigned char)*s << sh;
  return t;
}

static const char *shot;
static int shotf;

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
  if (!items_init())
    TraceLog(LOG_ERROR, "game: no next/items.txt");
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
      play(&s, &ng, (unsigned)atol(seeds), !getenv("SCPCB_LOADING"), shot,
           shotf);
      break;
    }
    if (mainmenu(&ng, &s) != MENU_START)
      break;
    a = play(&s, &ng, seednum(ng.seed), 0, shot, shotf);
    menu_init(0, 0);
  }
  snd_close();
  CloseWindow();
  return 0;
}
