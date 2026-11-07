// ui.h: the original's menu widgets, immediate mode (call them every frame,
// they draw + return what happened). sizes are the original's 1024-tall
// layout; U() scales a length to the screen
#pragma once
#include "raylib.h"

void ui_init(int sw, int sh);
void ui_begin(void); // once per frame, before any widget
int U(float v);      // v * screen height / 1024, rounded
Font ui_font(int big);
// text at x, y; cx / cy: centre it on that point instead
void ui_text(const char *s, int x, int y, int cx, int cy, int big, Color c);
// word-wrapped into w x h, optionally centred per line. returns the rows drawn
int ui_wrap(const char *s, int x, int y, int w, int h, int center, Color c);
int ui_lines(const char *s, int w); // rows s wraps to at width w
int ui_hover(int x, int y, int w, int h);
int ui_sliding(void); // the slider being dragged, 0 = none
// a 1:1 slice (sx, sy) of a repeating texture into x, y, w, h
void ui_tile(const char *id, int sx, int sy, int x, int y, int w, int h,
             Color c);
void ui_frame(int x, int y, int w, int h);
int ui_button(int x, int y, int w, int h, const char *t, int big); // clicked
int ui_tick(int x, int y, int v, int locked);          // the new value
float ui_slider(int x, int y, int w, float v, int id); // v 0..1, "LOW" / "HIGH"
// a box showing t; clicking it focuses it (id), clicking elsewhere doesn't.
// returns whether it has focus
int ui_box(int x, int y, int w, int h, const char *t, int id);
void ui_unfocus(void);
// a text box editing buf in place (cap incl. the 0); id picks which box has
// focus
void ui_input(int x, int y, int w, int h, char *buf, int cap, int id);
void ui_cursor(void); // the original's cursor image at the mouse
