// loading.c: the original's DrawLoading layout
#define _POSIX_C_SOURCE 200809L
#include "ui/loading.h"
#include "core/asset.h"
#include "core/ini.h"
#include "core/util.h"
#include "raylib.h"
#include "ui/ui.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

static int align(const char *s) {
  return !strcasecmp(s, "right") || !strcasecmp(s, "bottom") ? 1
         : !strcasecmp(s, "left") || !strcasecmp(s, "top")   ? -1
                                                             : 0;
}

void loading_pick(Loading *l) {
  *l = (Loading){0};
  Ini o;
  if (!ini_load(&o, "Loadingscreens/loadingscreens.ini"))
    return;
  const char *secs[64];
  int n = ini_sections(&o, secs, 64);
  n = n > 64 ? 64 : n;
  if (n > 0) {
    const char *s = secs[irnd(0, n - 1)];
    snprintf(l->title, sizeof l->title, "%s", s);
    // "NVG.jpg" -> "loadingscreens/nvg.mu" (baked masked, 1:1)
    char p[128];
    snprintf(p, sizeof p, "%s", ini_str(&o, s, "image path", ""));
    for (char *c = p; *c; c++)
      *c = *c >= 'A' && *c <= 'Z' ? *c + 32 : *c == ' ' ? '_' : *c;
    char *dot = strrchr(p, '.');
    if (dot)
      *dot = 0;
    snprintf(l->img, sizeof l->img, "loadingscreens/%s.mu", p);
    l->ax = align(ini_str(&o, s, "align x", "center"));
    l->ay = align(ini_str(&o, s, "align y", "center"));
    l->nobg = !strcasecmp(ini_str(&o, s, "disablebackground", "false"), "true");
    for (int i = 0; i < 6; i++) {
      char k[8];
      snprintf(k, sizeof k, "text%d", i + 1);
      const char *t = ini_str(&o, s, k, NULL);
      if (!t)
        break;
      snprintf(l->txt[l->ntxt++], sizeof l->txt[0], "%s", t);
    }
  }
  ini_free(&o);
}

void loading_draw(const Loading *l, int percent) {
  int W = GetScreenWidth(), H = GetScreenHeight();
  ClearBackground(BLACK);
  if (!l->nobg) {
    Texture2D b = tex_ui("loadingscreens/loadingback.mu");
    DrawTexture(b, W / 2 - b.width / 2, H / 2 - b.height / 2, WHITE);
  }
  Texture2D t = tex_ui(l->img);
  int x = l->ax == 0  ? W / 2 - t.width / 2
          : l->ax > 0 ? W - t.width
                      : 0,
      y = l->ay == 0  ? H / 2 - t.height / 2
          : l->ay > 0 ? H - t.height
                      : 0;
  DrawTexture(t, x, y, WHITE);
  // the bar, smooth like the hud's
  Texture2D seg = tex_ui("gfx/blinkmeter.mu");
  int bw = 300, bx = W / 2 - bw / 2, by = H / 2 + 30 - 100;
  DrawRectangleLines(bx, by, bw + 4, 20, WHITE);
  DrawTexturePro(
      seg, (Rectangle){seg.width / 2.0f - 1, 0, 2, seg.height},
      (Rectangle){bx + 3, by + 3, (bw - 2) * percent / 100.0f, seg.height},
      (Vector2){0}, 0, WHITE);
  // the page's lines advance w/ the percentage
  int page = l->ntxt ? (int)minf(percent * l->ntxt / 100.0f, l->ntxt - 1) : 0;
  for (int pass = 0; pass < 2; pass++) { // shadow, then text
    int o = !pass;
    Color c = o ? BLACK : WHITE;
    ui_text(l->title, W / 2 + o, H / 2 + 80 + o, 1, 1, 1, c);
    if (l->ntxt)
      ui_wrap(l->txt[page], W / 2 - 200 + o, H / 2 + 120 + o, 400, 300, 1, c);
  }
  char lt[32];
  snprintf(lt, sizeof lt, "LOADING - %d %%", percent);
  ui_text(lt, W / 2 + 1, H / 2 - 100 + 1, 1, 1, 0, BLACK);
  ui_text(lt, W / 2, H / 2 - 100, 1, 1, 0, WHITE);
  if (percent >= 100)
    ui_text("PRESS ANY KEY TO CONTINUE", W / 2, H - 50, 1, 1, 0, WHITE);
}
