// asset.h: baked data lookup (next/data, or $SCPCB_DATA) + a texture cache by
// id
#pragma once
#include "raylib.h"

const char *asset_path(const char *id, const char *ext); // static buf
Texture2D tex_get(
    const char *id); // mipmapped, repeat, cached; id "-" or missing -> white
Texture2D tex_ui(const char *id); // 2d: as baked (npot ok), no mips, clamped
Font font_get(const char *id, int size); // cached per (id, size)
void tex_clear(void);                    // textures + fonts
