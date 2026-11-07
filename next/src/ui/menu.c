// menu.c: the original's Menu.bb layout, scaled by height / 1024: the
// background w/ 173 flickering in the corner + whispers, four buttons on the
// left, each opening a page next to a title frame
#include "ui/menu.h"
#include "audio/snd.h"
#include "core/asset.h"
#include "core/util.h"
#include "play/difficulty.h"
#include "raylib.h"
#include "ui/ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int tab, opttab; // tab: 0 main, 1 new game, 2 load game, 3 options
static float blink173 = 1, blinklen173, whisper = 1, whisperlen;
static int wx, wy;
static const char *wstr = "DON'T BLINK";

static const char *whispers[] = {
    "DON'T BLINK",
    "Secure. Contain. Protect.",
    "You want happy endings? Fuck you.",
    "Sometimes we would have had time to scream.",
    "NIL",
    "NO",
    "black white black white black white gray",
    "Stone does not care",
    "9341",
    "It controls the doors",
    "e8m106]af173o+079m895w914",
    "It has taken over everything",
    "The spiral is growing",
    "\"Some kind of gestalt effect due to massive reality damage.\""};
// a new game's seed: random letters + digits, now + then one of these
static const char *eggs[] = {"NIL",        "NO",     "d9341", "5CP_I73",
                             "DONTBLINK",  "CRUNCH", "die",   "HTAED",
                             "rustledjim", "larry",  "JORGE", "dirtymetal",
                             "whatpumpkin"};

void menu_init(int t, int ot) { tab = t, opttab = ot; }

static void scaled(const char *id, int x, int y) {
  Texture2D t = tex_ui(id);
  DrawTextureEx(t, (Vector2){x, y}, 0, U(1000) / 1000.0f, WHITE);
}

static void background(void) {
  int W = GetScreenWidth(), H = GetScreenHeight();
  ClearBackground(BLACK);
  scaled("gfx/menu/back.mu", 0, 0);
  // 173 in the corner, gone for a moment now + then
  Texture2D t = tex_ui("gfx/menu/173back.mu");
  float k = U(1000) / 1000.0f;
  if (fmodf(GetTime() * 1000, blink173) >= irnd(1, (int)maxf(blinklen173, 1)))
    DrawTextureEx(t, (Vector2){W - t.width * k, H - t.height * k}, 0, k, WHITE);
  if (irnd(1, 300) == 1) {
    blink173 = irnd(4000, 8000);
    blinklen173 = irnd(200, 500);
  }
  // a whisper, jittering somewhere on the right for a moment
  whisper -= GetFrameTime() * TICK;
  if (whisper < whisperlen) {
    ui_text(wstr, wx + irnd(-5, 5), wy + irnd(-5, 5), 1, 0, 0,
            (Color){50, 50, 50, 255});
    if (whisper < 0) {
      whisper = irnd(700, 800);
      whisperlen = irnd(10, 35);
      wx = U(irnd(700, 1000));
      wy = U(irnd(100, 600));
      wstr = whispers[irnd(0, sizeof whispers / sizeof *whispers - 1)];
    }
  }
  Texture2D st = tex_ui("gfx/menu/scptext.mu");
  scaled("gfx/menu/scptext.mu", W / 2.0f - st.width * k / 2,
         H - U(20) - st.height * k);
  if (W > U(1240)) // back.jpg's line, carried on to a wide screen's edge
    ui_tile("gfx/menu/menuwhite.mu", 0, 5, U(985), U(407), W - U(1240) + 300,
            U(7), WHITE);
}

static void newseed(char *s, int n) {
  if (irnd(1, 15) == 1) {
    snprintf(s, n, "%s", eggs[irnd(0, sizeof eggs / sizeof *eggs - 1)]);
    return;
  }
  int k = irnd(4, 8);
  for (int i = 0; i < k && i < n - 1; i++)
    s[i] = irnd(1, 3) == 1 ? '0' + irnd(0, 9) : 'a' + irnd(0, 25);
  s[k < n - 1 ? k : n - 1] = 0;
}

// name + seed boxes, the intro tick, the difficulties + their description
// (custom: its own ticks instead), load map + start
static MenuAct newgame(NewGame *ng, Settings *s) {
  int x = U(160), y = U(286 + 70 + 20), w = U(580), h = U(330);
  ui_frame(x, y, w, h);
  ui_text("Name:", x + U(20), y + U(20), 0, 0, 0, WHITE);
  ui_input(x + U(150), y + U(15), U(200), U(30), ng->name, sizeof ng->name, 1);
  char *o = ng->name; // no path chars, it names the save
  for (char *c = o; *c; c++)
    if (!strchr(":./\\<>|?\"*", *c))
      *o++ = *c;
  *o = 0;
  ui_text("Map seed:", x + U(20), y + U(60), 0, 0, 0, WHITE);
  ui_input(x + U(150), y + U(55), U(200), U(30), ng->seed, sizeof ng->seed, 3);
  ui_text("Enable intro sequence:", x + U(20), y + U(110), 0, 0, 0, WHITE);
  s->intro = ui_tick(x + U(280), y + U(110), s->intro, 0);
  ui_text("Difficulty:", x + U(20), y + U(150), 0, 0, 0, WHITE);
  for (int i = 0; i < D_N; i++) {
    if (ui_tick(x + U(20), y + U(180 + 30 * i), ng->sel == i, 0) &&
        ng->sel != i)
      ng->d = difficulties[ng->sel = i];
    ui_text(difficulties[i].name, x + U(60), y + U(180 + 30 * i), 0, 0, 0,
            difficulties[i].c);
  }
  ui_frame(x + U(150), y + U(155), U(410), U(150));
  Difficulty *d = &ng->d;
  if (d->custom) {
    static const char *f[] = {"Easy", "Normal", "Hard"};
    d->permadeath = ui_tick(x + U(160), y + U(165), d->permadeath, 0);
    ui_text("Permadeath", x + U(200), y + U(165), 0, 0, 0, WHITE);
    d->save = ui_tick(x + U(160), y + U(195),
                      d->save == SAVE_ANYWHERE && !d->permadeath, d->permadeath)
                  ? SAVE_ANYWHERE
                  : SAVE_SCREENS;
    ui_text("Save anywhere", x + U(200), y + U(195), 0, 0, 0, WHITE);
    d->aggressive = ui_tick(x + U(160), y + U(225), d->aggressive, 0);
    ui_text("Aggressive NPCs", x + U(200), y + U(225), 0, 0, 0, WHITE);
    // the arrow (pointing right) cycles easy -> normal -> hard
    Texture2D ar = tex_ui("gfx/menu/arrow.mu");
    int ax = x + U(155), ay = y + U(251);
    DrawTexturePro(ar, (Rectangle){0, 0, ar.width, ar.height},
                   (Rectangle){ax + ar.height / 2.0f, ay + ar.width / 2.0f,
                               ar.width, ar.height},
                   (Vector2){ar.width / 2.0f, ar.height / 2.0f}, 90, WHITE);
    Vector2 m = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m.x >= ax &&
        m.x < ax + ar.height && m.y >= ay && m.y < ay + ar.width) {
      d->otherfactors = (d->otherfactors + 1) % 3;
      snd_play("sfx/interact/button", 1);
    }
    char t[48];
    snprintf(t, sizeof t, "Other difficulty factors: %s", f[d->otherfactors]);
    ui_text(t, x + U(200), y + U(255), 0, 0, 0, WHITE);
  } else
    ui_wrap(d->desc, x + U(160), y + U(160), U(390), 200, 0, WHITE);
  ui_button(x, y + h + U(20), U(160), U(70), "Load map",
            0); // no saved maps yet
  if (ui_button(x + U(420), y + h + U(20), U(160), U(70), "START", 0)) {
    if (!ng->name[0])
      snprintf(ng->name, sizeof ng->name, "untitled");
    if (!ng->seed[0])
      snprintf(ng->seed, sizeof ng->seed, "%d", (int)(GetTime() * 1000));
    settings_save(s, "options.ini");
    return MENU_START;
  }
  return MENU_NONE;
}

MenuAct menu_main(NewGame *ng, Settings *s) {
  ui_begin();
  background();
  int x = U(159), y = U(286), w = U(400), h = U(70);
  MenuAct act = MENU_NONE;
  if (tab == 0) {
    static const char *b[] = {"NEW GAME", "LOAD GAME", "OPTIONS", "QUIT"};
    for (int i = 0; i < 4; i++)
      if (ui_button(x, U(286 + 100 * i), w, h, b[i], 1)) {
        if (i == 0) {
          newseed(ng->seed, sizeof ng->seed);
          tab = 1;
        } else if (i == 3)
          act = MENU_QUIT;
        else
          tab = i + 1;
      }
  } else {
    ui_frame(x, y, w, h);
    ui_text(tab == 1   ? "NEW GAME"
            : tab == 2 ? "LOAD GAME"
                       : "OPTIONS",
            x + w / 2, y + h / 2, 1, 1, 1, WHITE);
    if (ui_button(x + w + U(20), y, U(580) - w - U(20), h, "BACK", 0)) {
      if (tab == 1 || tab == 3)
        settings_save(s, "options.ini");
      tab = 0;
    }
    if (tab == 1)
      act = newgame(ng, s);
    else if (tab == 2) {
      ui_frame(U(160), y + h + U(20), U(580), U(510));
      ui_text("No saved games.", U(180), y + h + U(40), 0, 0, 0, WHITE);
    } else {
      settings_screen(s, &opttab, U(160), y + h + U(20), 0);
    }
  }
  ui_text("next", 20, GetScreenHeight() - 30, 0, 0, 0, WHITE);
  return act;
}

// a button that can't be pressed yet (no saves / achievements so far)
static void dead(int x, int y, int w, int h, const char *t) {
  ui_frame(x, y, w, h);
  ui_text(t, x + w / 2, y + h / 2, 1, 1, 1, (Color){100, 100, 100, 255});
}

MenuAct menu_pause(const NewGame *ng, Settings *s, int died) {
  ui_begin();
  int W = GetScreenWidth(), H = GetScreenHeight();
  Texture2D bg = tex_ui("gfx/menu/pausemenu.u");
  float k = U(1000) / 1000.0f;
  int x = W / 2 - (int)(bg.width * k) / 2, y = H / 2 - (int)(bg.height * k) / 2;
  DrawTextureEx(bg, (Vector2){x, y}, 0, k, WHITE);
  x += U(132);
  y += U(122);
  static int page; // 0 the buttons, 1 options, 2 quit?
  page = died ? 0 : page;
  static const char *titles[] = {"PAUSED", "OPTIONS", "QUIT?"};
  ui_text(died ? "YOU DIED" : titles[page], x, y - U(122 - 45), 0, 1, 1, WHITE);
  int bw = U(390), bh = U(60);
  if (page == 1) {
    settings_screen(s, &opttab, x, y, 1);
    if (ui_button(x + U(101), y + U(390), U(230), U(60), "Back", 1)) {
      settings_save(s, "options.ini");
      page = 0;
    }
    return MENU_NONE;
  }
  if (page == 2) {
    if (ui_button(x, y + U(60), bw, bh, "Quit", 1))
      return MENU_TOMENU;
    if (ui_button(x + U(101), y + U(344), U(230), U(60), "Back", 1))
      page = 0;
    return MENU_NONE;
  }
  char line[96];
  snprintf(line, sizeof line, "Difficulty: %s", ng->d.name);
  ui_text(line, x, y, 0, 0, 0, WHITE);
  snprintf(line, sizeof line, "Save: %s", ng->name);
  ui_text(line, x, y + U(20), 0, 0, 0, WHITE);
  snprintf(line, sizeof line, "Map seed: %s", ng->seed);
  ui_text(line, x, y + U(40), 0, 0, 0, WHITE);
  y += 10;
  MenuAct act = MENU_NONE;
  if (died) {
    y += U(104);
    if (!ng->d.permadeath) {
      dead(x, y, bw, bh, "Load Game");
      y += U(80);
    }
    if (ui_button(x, y, bw, bh, "Quit to Menu", 1))
      act = MENU_TOMENU;
    return act;
  }
  y += U(72);
  if (ui_button(x, y, bw, bh, "Resume", 1))
    act = MENU_RESUME;
  y += U(75);
  if (!ng->d.permadeath) {
    dead(x, y, bw, bh, "Load Game");
    y += U(75);
  }
  dead(x, y, bw, bh, "Achievements");
  y += U(75);
  if (ui_button(x, y, bw, bh, "Options", 1))
    page = 1;
  y += U(75);
  if (ui_button(x, y, bw, bh, "Quit", 1))
    page = 2;
  return act;
}
