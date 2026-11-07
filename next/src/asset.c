// asset.c: baked files + the texture cache
#include "asset.h"
#include "raylib.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *asset_path(const char *id, const char *ext) {
  static char b[1024];
  static const char *root;
  if (!root)
    root = getenv("SCPCB_DATA") ? getenv("SCPCB_DATA") : "next/data";
  snprintf(b, sizeof b, "%s/%s%s", root, id, ext);
  return b;
}

typedef struct {
  char id[256];
  Texture2D t;
} tc;
static tc *ts;
static int nts;
static Texture2D white;

static Texture2D load(const char *id, int ui);

Texture2D tex_get(const char *id) {
  if (!white.id)
    white = (Texture2D){rlGetTextureIdDefault(), 1, 1, 1,
                        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
  if (!id || !strcmp(id, "-"))
    return white;
  for (int i = 0; i < nts; i++)
    if (!strcmp(ts[i].id, id))
      return ts[i].t;
  Texture2D t = load(id, 0);
  if (!t.id)
    t = white;
  ts = realloc(ts, sizeof *ts * (nts + 1));
  snprintf(ts[nts].id, sizeof ts[nts].id, "%s", id);
  ts[nts++].t = t;
  return t;
}

static Texture2D load(const char *id, int ui) {
  Texture2D t = LoadTexture(asset_path(id, ".qoi"));
  if (!t.id)
    return t;
  if (ui) { // npot on es2: no mips, no repeat
    SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(t, TEXTURE_WRAP_CLAMP);
  } else {
    GenTextureMipmaps(&t);
    SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(t, TEXTURE_WRAP_REPEAT);
  }
  return t;
}

Texture2D tex_ui(const char *id) {
  for (int i = 0; i < nts; i++)
    if (!strcmp(ts[i].id, id))
      return ts[i].t;
  Texture2D t = load(id, 1);
  ts = realloc(ts, sizeof *ts * (nts + 1));
  snprintf(ts[nts].id, sizeof ts[nts].id, "%s", id);
  ts[nts++].t = t;
  return t;
}

typedef struct {
  char id[128];
  int size;
  Font f;
} fc;
static fc *fs;
static int nfs;

Font font_get(const char *id, int size) {
  for (int i = 0; i < nfs; i++)
    if (fs[i].size == size && !strcmp(fs[i].id, id))
      return fs[i].f;
  Font f = LoadFontEx(asset_path(id, ".ttf"), size, NULL, 0);
  if (!f.texture.id)
    f = GetFontDefault();
  else
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
  fs = realloc(fs, sizeof *fs * (nfs + 1));
  snprintf(fs[nfs].id, sizeof fs[nfs].id, "%s", id);
  fs[nfs].size = size;
  fs[nfs++].f = f;
  return f;
}

void tex_clear(void) {
  for (int i = 0; i < nfs; i++)
    if (fs[i].f.texture.id != GetFontDefault().texture.id)
      UnloadFont(fs[i].f);
  free(fs);
  fs = NULL;
  nfs = 0;
  for (int i = 0; i < nts; i++)
    if (ts[i].t.id != white.id)
      UnloadTexture(ts[i].t);
  free(ts);
  ts = NULL;
  nts = 0;
}
