// settings.c: options.ini <-> Settings + the options tabs, laid out like the
// original's main menu + pause menu versions (Menu.bb / Main.bb). only the
// options next/ actually has are shown
#define _POSIX_C_SOURCE 200809L
#include "settings.h"
#include "asset.h"
#include "hud.h"
#include "ini.h"
#include "player.h"
#include "raylib.h"
#include "ui.h"
#include <stdio.h>
#include <strings.h>

static const char *kini[K_N] = {
    "up key",     "left key",      "down key",   "right key", "blink key",
    "sprint key", "inventory key", "crouch key", "save key",  "console key"};
static const int kdef[K_N] = {17, 30, 31, 32, 57, 42, 15, 29, 63, 61};
// KeyName.bb
static const char *kname[212] = {[1] = "Esc",         [2] = "1",
                                 [3] = "2",           [4] = "3",
                                 [5] = "4",           [6] = "5",
                                 [7] = "6",           [8] = "7",
                                 [9] = "8",           [10] = "9",
                                 [11] = "0",          [12] = "-",
                                 [13] = "=",          [14] = "Backspace",
                                 [15] = "TAB",        [16] = "Q",
                                 [17] = "W",          [18] = "E",
                                 [19] = "R",          [20] = "T",
                                 [21] = "Y",          [22] = "U",
                                 [23] = "I",          [24] = "O",
                                 [25] = "P",          [26] = "[",
                                 [27] = "]",          [28] = "Enter",
                                 [29] = "Left Ctrl",  [30] = "A",
                                 [31] = "S",          [32] = "D",
                                 [33] = "F",          [34] = "G",
                                 [35] = "H",          [36] = "J",
                                 [37] = "K",          [38] = "L",
                                 [39] = ";",          [40] = "'",
                                 [42] = "Left Shift", [43] = "\\",
                                 [44] = "Z",          [45] = "X",
                                 [46] = "C",          [47] = "V",
                                 [48] = "B",          [49] = "N",
                                 [50] = "M",          [51] = ",",
                                 [52] = ".",          [54] = "Right Shift",
                                 [56] = "Left Alt",   [57] = "Space",
                                 [58] = "Caps Lock",  [59] = "F1",
                                 [60] = "F2",         [61] = "F3",
                                 [62] = "F4",         [63] = "F5",
                                 [64] = "F6",         [65] = "F7",
                                 [66] = "F8",         [67] = "F9",
                                 [68] = "F10",        [157] = "Right Ctrl",
                                 [184] = "Right Alt", [200] = "Up",
                                 [203] = "Left",      [205] = "Right",
                                 [208] = "Down"};

void settings_load(Settings *s, const char *path) {
  Ini o = {0};
  ini_load(&o, path);
  const char *hud = ini_str(&o, "options", "hud mode", NULL);
  *s = (Settings){
      .width = ini_int(&o, "options", "width", 1280),
      .height = ini_int(&o, "options", "height", 720),
      .mapsize = ini_int(&o, "options", "map size", 18),
      .music = ini_float(&o, "audio", "music volume", .5f),
      .sound = ini_float(&o, "audio", "sound volume", 1),
      .sens = ini_float(&o, "options", "mouse sensitivity", 0),
      .smooth = ini_float(&o, "options", "mouse smoothing", 1),
      .invert = ini_int(&o, "options", "invert mouse y", 0),
      .vsync = ini_int(&o, "options", "vsync", 1),
      .fps = ini_int(&o, "options", "show fps", 0),
      .intro = ini_int(&o, "options", "intro enabled", 1),
      .framelimit = ini_int(&o, "options", "framelimit", 0),
      // "hud mode = auto|always|off", else the original's "HUD enabled"
      .hud =
          !hud ? (ini_int(&o, "options", "hud enabled", 1) ? HUD_AUTO : HUD_OFF)
          : !strcasecmp(hud, "off")    ? HUD_OFF
          : !strcasecmp(hud, "always") ? HUD_ALWAYS
                                       : HUD_AUTO,
      .fognear = ini_float(&o, "options", "camera fog near", .5f),
      .fogfar = ini_float(&o, "options", "camera fog far", 6),
  };
  for (int i = 0; i < K_N; i++) // [binds], or older files' [options]
    s->keys[i] =
        ini_int(&o, "binds", kini[i], ini_int(&o, "options", kini[i], kdef[i]));
  ini_free(&o);
}

static void putf(const char *p, const char *sec, const char *k, float v) {
  char b[32];
  snprintf(b, sizeof b, "%g", v);
  ini_set(p, sec, k, b);
}

void settings_save(const Settings *s, const char *p) {
  static const char *hud[] = {"off", "always", "auto"};
  putf(p, "audio", "music volume", s->music);
  putf(p, "audio", "sound volume", s->sound);
  putf(p, "options", "mouse sensitivity", s->sens);
  putf(p, "options", "mouse smoothing", s->smooth);
  putf(p, "options", "invert mouse y", s->invert);
  putf(p, "options", "vsync", s->vsync);
  putf(p, "options", "show fps", s->fps);
  putf(p, "options", "intro enabled", s->intro);
  putf(p, "options", "framelimit", s->framelimit);
  ini_set(p, "options", "hud mode", hud[s->hud]);
  for (int i = 0; i < K_N; i++)
    putf(p, "binds", kini[i], s->keys[i]);
}

int settings_key(const Settings *s, int k) { return key_dik(s->keys[k]); }

void settings_apply(const Settings *s) {
  static int vs = -1;
  if (s->vsync != vs)
    s->vsync ? SetWindowState(FLAG_VSYNC_HINT)
             : ClearWindowState(FLAG_VSYNC_HINT);
  vs = s->vsync;
  SetTargetFPS(s->framelimit);
}

// the hovered option's tooltip, drawn once the rows are done
static struct {
  const char *t;
  char t2[160];
  Color c;
} tip;

static void hint(int on, const char *t, const char *t2, Color c) {
  if (!on)
    return;
  tip.t = t;
  snprintf(tip.t2, sizeof tip.t2, "%s", t2 ? t2 : "");
  tip.c = c;
}

static void drawtip(int x, int y, int w) { // DrawOptionsTooltip
  if (!tip.t)
    return;
  int fx = x + U(6), fy = y + U(6), fw = w - U(12), lh = ui_font(0).baseSize;
  int n = ui_lines(tip.t, fw), n2 = *tip.t2 ? ui_lines(tip.t2, fw) : 0;
  ui_frame(x, y, w, lh * n + U(10 + n) + (n2 ? lh * n2 + U(10 + n2) : 0));
  ui_wrap(tip.t, fx, fy, fw, 1 << 20, 0, WHITE);
  if (n2)
    ui_wrap(tip.t2, fx, fy + lh * n + U(5 + n), fw, 1 << 20, 0, tip.c);
}

static const Color WH = {255, 255, 255, 255}, YE = {255, 255, 0, 255};

static char *pct(const char *pre, float v, int def) {
  static char b[64];
  snprintf(b, sizeof b, "%s%d%% (default is %d%%)", pre, (int)(v * 100), def);
  return b;
}

static void label(const char *t, int x, int y) {
  ui_text(t, x, y, 0, 0, 0, WHITE);
}

// a tick at cx: hovering it shows its tooltip
static int tick(const char *t, int x, int cx, int y, int v, const char *tt) {
  label(t, x, y);
  v = ui_tick(cx, y + U(1), v, 0);
  hint(ui_hover(cx, y + U(1), U(20), U(20)) && !ui_sliding(), tt, NULL, WH);
  return v;
}

static void keys(Settings *s, int x, int y, const int *bx, const int *by,
                 const int *lx) {
  static const char *names[K_N] = {
      "Move Forward",         "Strafe Left",  "Move Backward",
      "Strafe Right",         "Manual Blink", "Sprint",
      "Open/Close Inventory", "Crouch",       "Quick Save",
      "Open/Close Console"};
  for (int i = 0; i < K_N; i++) {
    label(names[i], x + lx[i], y + by[i]);
    int k = s->keys[i];
    if (!ui_box(x + bx[i], y + by[i], U(100), U(20),
                k > 0 && k < 212 && kname[k] ? kname[k] : "?", 100 + i))
      continue;
    int rk = GetKeyPressed(); // raylib -> directinput
    for (int d = 1; rk && d < 256; d++)
      if (key_dik(d) == rk) {
        s->keys[i] = d;
        ui_unfocus();
        break;
      }
  }
}

void settings_screen(Settings *s, int *tab, int x, int y, int ingame) {
  static const char *tabs[] = {"GRAPHICS", "AUDIO", "CONTROLS", "ADVANCED"};
  int w = U(580), tx, ty, cx, sw, top;
  tip.t = NULL;
  if (ingame) { // the tabs bare above the rows, controls 270 in
    DrawRectangle(x + U(110 * *tab - 10), y - U(5), U(110), U(40), GREEN);
    for (int i = 0; i < 4; i++)
      if (ui_button(x + U(110 * i - 5), y, U(100), U(30), tabs[i], 0))
        *tab = i;
    Texture2D pm = tex_ui("gfx/menu/pausemenu.u");
    tx = GetScreenWidth() / 2 + (int)(pm.width * U(1000) / 1000.0f) / 2, ty = y;
    y += U(50);
    cx = x + U(270), sw = U(100), top = y;
  } else { // a framed tab strip, then a frame per tab, controls 310 in
    ui_frame(x, y, w, U(60));
    DrawRectangle(x + U(15 + 140 * *tab), y + U(10), w / 5 + U(10),
                  U(60) / 2 + U(10), GREEN);
    for (int i = 0; i < 4; i++)
      if (ui_button(x + U(20 + 140 * i), y + U(15), w / 5, U(60) / 2, tabs[i],
                    0))
        *tab = i;
    y += U(70);
    static const int fh[] = {330, 220, 270, 320};
    ui_frame(x, y, w, U(fh[*tab]));
    tx = x + w, ty = y;
    x += U(20);
    y += U(20);
    cx = x + U(290), sw = U(150), top = y;
  }
  (void)top;
  switch (*tab) {
  case 0:
    s->vsync = tick("VSync:", x, cx, y, s->vsync,
                    "\"Vertical sync\" waits for the display to finish its "
                    "current refresh cycle before calculating the "
                    "next frame, preventing issues such as screen tearing. "
                    "This ties the game's frame rate to your "
                    "display's refresh rate and may cause some input lag.");
    break;
  case 1: {
    int sx = ingame ? x + U(250) : cx;
    s->music = ui_slider(sx, y - U(4), sw, s->music, 1);
    label("Music volume:", x, y);
    hint(ui_hover(sx, y - U(4), sw + 14, 20),
         "Adjusts the volume of background music. Sliding the bar fully to the "
         "left will mute all music.",
         pct("Current value: ", s->music, 50), WH);
    y += U(ingame ? 30 : 40);
    s->sound = ui_slider(sx, y - U(4), sw, s->sound, 2);
    label("Sound volume:", x, y);
    hint(ui_hover(sx, y - U(4), sw + 14, 20),
         "Adjusts the volume of sound effects. Sliding the bar fully to the "
         "left will mute all sounds.",
         pct("Current value: ", s->sound, 100), WH);
    break;
  }
  case 2: {
    s->sens = ui_slider(cx, y - U(4), sw, s->sens + .5f, 3) - .5f;
    label("Mouse sensitivity:", x, y);
    hint(ui_hover(cx, y - U(4), sw + 14, 20),
         "Adjusts the speed of the mouse pointer.",
         pct("Current value: ", s->sens + .5f, 50), WH);
    y += U(ingame ? 30 : 40);
    s->invert = tick("Invert mouse Y-axis:", x, cx, y, s->invert,
                     "\"Invert mouse Y-axis\" is self-explanatory.");
    y += U(40);
    s->smooth = ui_slider(cx, y - U(4), sw, s->smooth / 2, 4) * 2;
    label("Mouse smoothing:", x, y);
    hint(ui_hover(cx, y - U(4), sw + 14, 20),
         "Adjusts the amount of smoothing of the mouse pointer.",
         pct("Current value: ", s->smooth, 100), WH);
    y += U(30);
    label("Control configuration:", x, y);
    y += U(10);
    if (ingame) { // one column
      int bx[K_N], by[K_N], lx[K_N] = {0};
      for (int i = 0; i < K_N; i++)
        bx[i] = U(200), by[i] = U(20 + 20 * i);
      keys(s, x, y, bx, by, lx);
      hint(ui_hover(x, y, U(400), U(220)),
           "Configure the in-game control scheme.", NULL, WH);
    } else { // two: movement + save on the left, the rest on the right
      static const int col[K_N] = {0, 0, 0, 0, 1, 1, 1, 1, 0, 1},
                       row[K_N] = {0, 1, 2, 3, 0, 1, 2, 3, 4, 4};
      int bx[K_N], by[K_N], lx[K_N];
      for (int i = 0; i < K_N; i++)
        lx[i] = U(col[i] * 260), bx[i] = U(col[i] ? 450 : 140),
        by[i] = U(20 + 20 * row[i]);
      keys(s, x, y, bx, by, lx);
      hint(ui_hover(x, y, w - U(40), U(120)),
           "Configure the in-game control scheme.", NULL, WH);
    }
    break;
  }
  case 3: {
    static const char *m[] = {"off", "always", "when needed"};
    label("Show HUD:", x, y);
    if (ui_button(cx, y - U(4), U(150), U(26), m[s->hud], 0))
      s->hud = (s->hud + 2) % 3; // when needed -> always -> off -> ...
    hint(ui_hover(cx, y - U(4), U(150), U(26)),
         "Display the blink and stamina meters. \"When needed\" shows each "
         "only while it matters: stamina once "
         "you've used some, blink as the next one gets close.",
         NULL, WH);
    y += U(40);
    s->fps = tick(
        "Show FPS:", x, cx, y, s->fps,
        "Displays the frames per second counter at the top left-hand corner.");
    y += U(30);
    label("Framelimit:", x, y);
    int on = ui_tick(cx, y, s->framelimit > 0, 0);
    if (on) { // 20..119, like the original's CurrFrameLimit
      float v = ui_slider(
          x + U(130), y + U(30), U(100),
          s->framelimit > 0 ? (s->framelimit - 19) * .99f / 100 : 0, 5);
      v = v * 100 / 99;
      s->framelimit = 19 + (int)((v < .01f ? .01f : v) * 100);
      char b[16];
      snprintf(b, sizeof b, "%d FPS", s->framelimit);
      ui_text(b, x + U(5), y + U(25), 0, 0, 0, YE);
    } else
      s->framelimit = 0;
    char b[160] = "";
    if (s->framelimit > 0 && s->framelimit < 60)
      snprintf(b, sizeof b,
               "Usually, 60 FPS or higher is preferred. If you are noticing "
               "excessive stuttering at "
               "this setting, try lowering it to make your framerate more "
               "consistent.");
    hint(ui_hover(cx, y + U(1), U(20), U(20)) ||
             ui_hover(x + U(130), y + U(30), U(100) + 14, 20),
         "Limits the frame rate that the game can run at to a desired value.",
         *b ? b : NULL, YE);
    break;
  }
  }
  drawtip(tx, ty, U(400));
}
