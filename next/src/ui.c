// ui.c: frames are the original's tiled menuwhite/menublack images, buttons
// darken under the mouse, the fonts are Courier New 19 + 58 (bold) at 1024
#include "ui.h"
#include "asset.h"
#include "raylib.h"
#include "snd.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static float S = 1;
static int focus, slider;
static Font f1, f2;
static Vector2 m;
static int hit, down;

void ui_init(int sw, int sh) {
  (void)sw;
  S = sh / 1024.0f;
  f1 = font_get("gfx/font/cour/courier_new", (int)(19 * S));
  f2 = font_get("gfx/font/courbd/courier_new", (int)(58 * S));
}

int U(float v) { return (int)roundf(v * S); }
Font ui_font(int big) { return big ? f2 : f1; }

void ui_begin(void) {
  m = GetMousePosition();
  hit = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  if (!down)
    slider = 0;
}

static int on(int x, int y, int w, int h) {
  return m.x >= x && m.x < x + w && m.y >= y && m.y < y + h;
}

void ui_text(const char *s, int x, int y, int cx, int cy, int big, Color c) {
  Font f = ui_font(big);
  Vector2 sz = MeasureTextEx(f, s, f.baseSize, 0);
  DrawTextEx(f, s, (Vector2){cx ? x - sz.x / 2 : x, cy ? y - sz.y / 2 : y},
             f.baseSize, 0, c);
}

static int wrap(const char *s, int x, int y, int w, int h, int center, Color c,
                int draw) {
  char line[512] = "";
  int lh = f1.baseSize + 1, row = 0;
  while (*s) {
    const char *e = strchr(s, ' ');
    size_t n = e ? (size_t)(e - s) : strlen(s), l = strlen(line);
    char try
      [512];
    snprintf(try, sizeof try, "%s%s%.*s", line, l ? " " : "", (int)n, s);
    if (l && MeasureTextEx(f1, try, f1.baseSize, 0).x > w) { // next row
      if ((row + 1) * lh > h)
        return row;
      if (draw)
        ui_text(line, center ? x + w / 2 : x, y + row * lh, center, 0, 0, c);
      row++;
      snprintf(line, sizeof line, "%.*s", (int)n, s);
    } else
      snprintf(line, sizeof line, "%s", try);
    s += n;
    while (*s == ' ')
      s++;
  }
  if (!*line || (row + 1) * lh > h)
    return row;
  if (draw)
    ui_text(line, center ? x + w / 2 : x, y + row * lh, center, 0, 0, c);
  return row + 1;
}

int ui_wrap(const char *s, int x, int y, int w, int h, int center, Color c) {
  return wrap(s, x, y, w, h, center, c, 1);
}
int ui_lines(const char *s, int w) {
  return wrap(s, 0, 0, w, 1 << 20, 0, WHITE, 0);
}

// the original's DrawTiledImageRect: a 1:1 slice of a repeating texture
void ui_tile(const char *id, int sx, int sy, int x, int y, int w, int h,
             Color c) {
  Texture2D t = tex_ui(id);
  DrawTexturePro(t, (Rectangle){sx, sy, w, h}, (Rectangle){x, y, w, h},
                 (Vector2){0}, 0, c);
}

void ui_frame(int x, int y, int w, int h) {
  ui_tile("gfx/menu/menuwhite.mu", 0, y % 256, x, y, w, h, WHITE);
  ui_tile("gfx/menu/menublack.u", 0, y % 256, x + U(3), y + U(3), w - U(6),
          h - U(6), WHITE);
}

int ui_button(int x, int y, int w, int h, const char *t, int big) {
  ui_frame(x, y, w, h);
  int c = 0;
  if (on(x, y, w, h)) {
    DrawRectangle(x + 4, y + 4, w - 8, h - 8, (Color){30, 30, 30, 255});
    if (hit && !slider) {
      c = 1;
      snd_play("sfx/interact/button", 1);
    }
  }
  ui_text(t, x + w / 2, y + h / 2, 1, 1, big, WHITE);
  return c;
}

int ui_tick(int x, int y, int v, int locked) {
  int w = U(20), hl = on(x, y, w, w) && !locked;
  ui_tile("gfx/menu/menuwhite.mu", x % 256, y % 256, x, y, w, w, WHITE);
  if (hl && hit) {
    v = !v;
    snd_play("sfx/interact/button", 1);
  }
  DrawRectangle(x + 2, y + 2, w - 4, w - 4,
                hl ? (Color){50, 50, 50, 255} : BLACK);
  if (v)
    ui_tile("gfx/menu/menuwhite.mu", x % 256, y % 256, x + 4, y + 4, w - 8,
            w - 8, hl ? WHITE : (Color){200, 200, 200, 255});
  return v;
}

float ui_slider(int x, int y, int w, float v, int id) {
  if (down && (slider == id || (!slider && on(x, y, w + 14, 20))))
    slider = id;
  if (slider == id)
    v = (m.x - x) / w;
  v = v < 0 ? 0 : v > 1 ? 1 : v;
  DrawRectangleLines(x, y, w + 14, 20, WHITE);
  DrawTexture(tex_ui("gfx/blinkmeter.mu"), (int)roundf(x + w * v + 3), y + 3,
              WHITE);
  Color g = {170, 170, 170, 255};
  ui_text("LOW", x - U(50), y + U(4), 0, 0, 0, g);
  ui_text("HIGH", x + w + U(38), y + U(4), 0, 0, 0, g);
  return v;
}

int ui_box(int x, int y, int w, int h, const char *t, int id) {
  ui_tile("gfx/menu/menuwhite.mu", x % 256, y % 256, x, y, w, h, WHITE);
  int hl = on(x, y, w, h);
  if (hit)
    focus = hl ? id : focus == id ? 0 : focus;
  DrawRectangle(x + 2, y + 2, w - 4, h - 4,
                hl ? (Color){50, 50, 50, 255} : BLACK);
  if (focus == id && fmodf(GetTime(), .8f) < .4f) { // the caret
    float tw = MeasureTextEx(f1, t, f1.baseSize, 0).x;
    DrawRectangle(x + w / 2 + (int)(tw / 2) + 2, y + h / 2 - 5, 2, 12, WHITE);
  }
  ui_text(t, x + w / 2, y + h / 2, 1, 1, 0, WHITE);
  return focus == id;
}

void ui_unfocus(void) { focus = 0; }

void ui_input(int x, int y, int w, int h, char *buf, int cap, int id) {
  if (!ui_box(x, y, w, h, buf, id))
    return;
  size_t n = strlen(buf);
  for (int c; (c = GetCharPressed());)
    if (c >= 32 && c < 127 && n + 1 < (size_t)cap) {
      buf[n++] = (char)c;
      buf[n] = 0;
    }
  if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && n)
    buf[--n] = 0;
}

int ui_hover(int x, int y, int w, int h) { return on(x, y, w, h); }
int ui_sliding(void) { return slider; }

void ui_cursor(void) {
  DrawTexture(tex_ui("gfx/cursor.mu"), (int)m.x, (int)m.y, WHITE);
}
