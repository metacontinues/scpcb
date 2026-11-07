// ui.c: 2d images the hud/menus draw 1:1 (blitz drew images w/ black as
// clear, so they bake masked) + every font, copied as is (raylib reads ttf)
#define _POSIX_C_SOURCE 200809L
#include "bake.h"
#include "raylib.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

static const char *images[] = {
    "GFX/BlinkMeter.jpg",     "GFX/StaminaMeter.jpg",
    "GFX/BlinkIcon.png",      "GFX/sprinticon.png",
    "GFX/sneakicon.png",      "GFX/handsymbol.png",
    "GFX/handsymbol2.png",    "GFX/menu/back.jpg",
    "GFX/menu/173back.jpg",   "GFX/menu/scptext.jpg",
    "GFX/menu/arrow.png",     "GFX/menu/QuickLoading.png",
    "GFX/menu/menuwhite.jpg", "GFX/cursor.png",
};

static int copy(const char *src) {
  char id[1024], out[2048];
  mkid(src, id, sizeof id);
  snprintf(out, sizeof out, OUT "%s.ttf", id);
  if (!stale(src, out))
    return 1;
  int n;
  unsigned char *d = LoadFileData(src, &n);
  if (!d)
    return 0;
  mkdirs(out);
  int ok = SaveFileData(out, d, n);
  UnloadFileData(d);
  return ok;
}

static int fonts(const char *dir) {
  int bad = 0;
  DIR *d = opendir(dir);
  if (!d)
    return 1;
  for (struct dirent *e; (e = readdir(d));) {
    if (e->d_name[0] == '.')
      continue;
    char p[2048];
    snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
    struct stat st;
    if (stat(p, &st))
      continue;
    size_t l = strlen(p);
    if (S_ISDIR(st.st_mode))
      bad += fonts(p);
    else if (l > 4 && !strcasecmp(p + l - 4, ".ttf") && !copy(p)) {
      warn("can't copy %s", p);
      bad++;
    }
  }
  closedir(d);
  return bad;
}

// every loading screen image, masked like the original drew them
static int loadings(void) {
  int bad = 0;
  DIR *d = opendir("Loadingscreens");
  if (!d)
    return 1;
  for (struct dirent *e; (e = readdir(d));) {
    size_t l = strlen(e->d_name);
    if (l < 4 || (strcasecmp(e->d_name + l - 4, ".jpg") &&
                  strcasecmp(e->d_name + l - 4, ".png")))
      continue;
    char p[512];
    snprintf(p, sizeof p, "Loadingscreens/%s", e->d_name);
    if (!bake_tex(p, IM_MASK | IM_UI)) {
      warn("loading screen %s unreadable", p);
      bad++;
    }
  }
  closedir(d);
  return bad;
}

int bake_ui(void) {
  int bad = loadings();
  // the frames' dark fill + the pause menu: the original masked them w/
  // yellow so their black stays
  if (!bake_tex("GFX/menu/menublack.jpg", IM_UI) ||
      !bake_tex("GFX/menu/pausemenu.jpg", IM_UI))
    bad++;
  for (size_t i = 0; i < sizeof images / sizeof *images; i++)
    if (!bake_tex(images[i], IM_MASK | IM_UI)) {
      warn("ui image %s missing", images[i]);
      bad++;
    }
  return bad + fonts("GFX/font");
}
