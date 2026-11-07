// rlconfig_tool.h: raylib for tools/bake. defaults + the source formats the
// game's files come in.
#pragma once
#define SUPPORT_FILEFORMAT_JPG 1
#define SUPPORT_FILEFORMAT_TGA 1
// the tools never paste images; it also pulls in x11 calls that only a
// --gc-sections link (clang/gcc, not tcc) drops
#define SUPPORT_CLIPBOARD_IMAGE 0
