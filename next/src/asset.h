// asset.h: baked data lookup (next/data, or $SCPCB_DATA) + a texture cache by id
#pragma once
#include "raylib.h"

const char *asset_path(const char *id, const char *ext); // static buf
Texture2D tex_get(const char *id);                       // mipmapped, repeat, cached; id "-" or missing -> white
void tex_clear(void);
