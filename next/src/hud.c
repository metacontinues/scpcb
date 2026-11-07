// hud.c: the original's DrawGUI layout in pixels (it didn't scale the hud
// either), its fonts at their size * screen height / 1024
#include "hud.h"
#include "asset.h"
#include "raylib.h"
#include "util.h"
#include <math.h>
#include <stdio.h>

static int W, H;
static Font msgfont;
static char msg[256];
static float msgt;

void hud_init(int sw, int sh) {
  W = sw;
  H = sh;
  msgfont = font_get("gfx/font/cour/courier_new", (int)(19 * (sh / 1024.0f)));
}

void hud_msg(const char *text, float ticks) {
  snprintf(msg, sizeof msg, "%s", text);
  msgt = ticks;
}

void hud_tick(void) { msgt = maxf(msgt - 1, 0); }

// a bar of 10 px segments in a white frame, the icon left of it in a black box
static void meter(int x, int y, float frac, const char *seg, const char *icon) {
  Texture2D s = tex_ui(seg), ic = tex_ui(icon);
  DrawRectangleLines(x, y, 204, 20, WHITE);
  for (int i = 0; i < (int)(202 * frac / 10); i++)
    DrawTexture(s, x + 3 + 10 * i, y + 3, WHITE);
  DrawRectangle(x - 50, y, 30, 30, BLACK);
  DrawRectangleLines(x - 51, y - 1, 32, 32, WHITE);
  DrawTexture(ic, x - 50, y, WHITE);
}

// past 90 degrees off to a side the hand sticks to the screen's edge
static float edge(float a) {
  a = wrapdeg(a);
  return a > 90 && a <= 180 ? 90 : a > 180 && a < 270 ? 270 : a;
}

static void text(const char *s, int cx, int y, Color c) {
  Vector2 sz = MeasureTextEx(msgfont, s, msgfont.baseSize, 0);
  DrawTextEx(msgfont, s, (Vector2){cx - sz.x / 2, y}, msgfont.baseSize, 0, c);
}

void hud_draw(const HudIn *in) {
  if (in->dark > 0)
    DrawRectangle(0, 0, W, H, Fade(BLACK, clampf(in->dark, 0, 1)));
  if (in->hand) {
    int x = (int)roundf(W / 2.0f +
                        sinf(edge(in->handyaw) * DEG2RAD) * (W / 3.0f) - 32),
        y = (int)roundf(H / 2.0f -
                        sinf(edge(in->handpitch) * DEG2RAD) * (H / 3.0f) - 32);
    DrawTexture(tex_ui("gfx/handsymbol.mu"), x, y, WHITE);
  }
  meter(80, H - 95, in->blinkfreq > 0 ? in->blink / in->blinkfreq : 0,
        "gfx/blinkmeter.mu", "gfx/blinkicon.mu");
  meter(80, H - 55, in->stamina / 100, "gfx/staminameter.mu",
        in->crouch ? "gfx/sneakicon.mu" : "gfx/sprinticon.mu");
  if (msgt > 0) {
    float a = minf(msgt / 2, 255) / 255;
    text(msg, W / 2 + 1, H / 2 + 201, Fade(BLACK, a));
    text(msg, W / 2, H / 2 + 200, Fade(WHITE, a));
  }
}
