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

Texture2D tex_get(const char *id) {
  if (!white.id)
    white = (Texture2D){rlGetTextureIdDefault(), 1, 1, 1,
                        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
  if (!id || !strcmp(id, "-"))
    return white;
  for (int i = 0; i < nts; i++)
    if (!strcmp(ts[i].id, id))
      return ts[i].t;
  Texture2D t = LoadTexture(asset_path(id, ".qoi"));
  if (!t.id)
    t = white;
  else {
    GenTextureMipmaps(&t);
    SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(t, TEXTURE_WRAP_REPEAT);
  }
  ts = realloc(ts, sizeof *ts * (nts + 1));
  snprintf(ts[nts].id, sizeof ts[nts].id, "%s", id);
  ts[nts++].t = t;
  return t;
}

void tex_clear(void) {
  for (int i = 0; i < nts; i++)
    if (ts[i].t.id != white.id)
      UnloadTexture(ts[i].t);
  free(ts);
  ts = NULL;
  nts = 0;
}
