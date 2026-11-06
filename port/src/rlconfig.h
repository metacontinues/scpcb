// rlconfig.h: our overrides for raylib 6.2's config.h, force-included (-include) ahead of it, see meson.build.
// 6.2's config.h #ifndef-guards every flag and the modules test `#if SUPPORT_X`, so this only lists what differs:
// the formats the game loads (qoi, qoa, ttf) stay on. assets are converted by tools/assetconv.c (which has its own
// decoders), so no png/jpg/bmp/dds/gif/ogg/mp3/wav decoders end up in the game. stb_image_write still comes in
// w/ SUPPORT_IMAGE_EXPORT, bb/sys.c uses it for BB_SHOT. don't define CONFIG_H here, config.h has to run after this.
#pragma once

// rcore: no cameras/gestures/f12 captures/clipboard/automation, the runtime does its own input + screenshots
#define SUPPORT_CAMERA_SYSTEM       0
#define SUPPORT_GESTURES_SYSTEM     0
#define SUPPORT_MOUSE_GESTURES      0
#define SUPPORT_SSH_KEYBOARD_RPI    0
#define SUPPORT_SCREEN_CAPTURE      0
#define SUPPORT_AUTOMATION_EVENTS   0
#define SUPPORT_CLIPBOARD_IMAGE     0

// rtextures: qoi only
#define SUPPORT_FILEFORMAT_PNG      0
#define SUPPORT_FILEFORMAT_BMP      0
#define SUPPORT_FILEFORMAT_GIF      0
#define SUPPORT_FILEFORMAT_DDS      0

// rtext: ttf only
#define SUPPORT_FILEFORMAT_FNT      0

// rmodels: the runtime has its own b3d/x/rmesh loaders (src/fmt)
#define SUPPORT_FILEFORMAT_OBJ      0
#define SUPPORT_FILEFORMAT_MTL      0
#define SUPPORT_FILEFORMAT_IQM      0
#define SUPPORT_FILEFORMAT_GLTF     0
#define SUPPORT_FILEFORMAT_VOX      0
#define SUPPORT_FILEFORMAT_M3D      0

// raudio: qoa only
#define SUPPORT_FILEFORMAT_WAV      0
#define SUPPORT_FILEFORMAT_OGG      0
#define SUPPORT_FILEFORMAT_MP3      0
#define SUPPORT_FILEFORMAT_XM       0
#define SUPPORT_FILEFORMAT_MOD      0
