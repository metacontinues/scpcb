// rlconfig.h: the game's overrides for raylib's config.h, force-included ahead
// of it (see meson.build). config.h #ifndef-guards every flag, so this only
// lists what differs. the game loads glb, qoi, qoa + ttf, everything else gets
// baked into those by tools/bake. don't define CONFIG_H here.
#pragma once

#define SUPPORT_GESTURES_SYSTEM 0
#define SUPPORT_MOUSE_GESTURES 0
#define SUPPORT_SSH_KEYBOARD_RPI 0
#define SUPPORT_SCREEN_CAPTURE 0
#define SUPPORT_AUTOMATION_EVENTS 0
#define SUPPORT_CLIPBOARD_IMAGE 0

#define SUPPORT_FILEFORMAT_PNG 0
#define SUPPORT_FILEFORMAT_BMP 0
#define SUPPORT_FILEFORMAT_GIF 0
#define SUPPORT_FILEFORMAT_DDS 0
#define SUPPORT_FILEFORMAT_FNT 0

#define SUPPORT_FILEFORMAT_OBJ 0
#define SUPPORT_FILEFORMAT_MTL 0
#define SUPPORT_FILEFORMAT_IQM 0
#define SUPPORT_FILEFORMAT_VOX 0
#define SUPPORT_FILEFORMAT_M3D 0

#define SUPPORT_FILEFORMAT_WAV 0
#define SUPPORT_FILEFORMAT_OGG 0
#define SUPPORT_FILEFORMAT_MP3 0
#define SUPPORT_FILEFORMAT_XM 0
#define SUPPORT_FILEFORMAT_MOD 0
