// items.c: what next/items.txt's templates need besides their models (those
// bake w/ the rest of GFX): inventory images + document images (pink masked,
// 1:1) and the textures that repaint shared models
#define _POSIX_C_SOURCE 200809L
#include "bake.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *s) {
  while (*s == ' ')
    s++;
  for (char *e = s + strlen(s);
       e > s && (e[-1] == ' ' || e[-1] == '\n' || e[-1] == '\r');)
    *--e = 0;
  return s;
}

static int img(const char *p, int mode) {
  if (!*p || bake_tex(p, mode))
    return 0;
  warn("item image %s missing", p);
  return 1;
}

int bake_items(void) {
  char *t = LoadFileText("next/items.txt");
  if (!t)
    return 1;
  int bad = 0;
  for (char *l = strtok(t, "\n"); l; l = strtok(NULL, "\n")) {
    if (*l == '#')
      continue;
    char *f[12] = {0};
    int n = 0;
    for (char *c = l; n < 12; n++) { // | separated, strtok would skip empties
      char *e = strchr(c, '|');
      if (e)
        *e = 0;
      f[n] = trim(c);
      if (!e)
        break;
      c = e + 1;
    }
    if (n < 9)
      continue;
    // 3 inv image, 4 image, 6 texture, 7 inv image 2, 9 texture flags
    bad += img(f[3], IM_PINK | IM_UI) + img(f[4], IM_PINK | IM_UI) +
           img(f[7], IM_PINK | IM_UI);
    bad += img(f[6], atoi(f[9]) & 2 ? IM_ALPHA : IM_OPAQUE);
  }
  UnloadFileText(t);
  return bad;
}
