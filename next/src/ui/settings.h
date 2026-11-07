// settings.h: what the options screens edit, kept in options.ini
#pragma once

// the original's binds, in the order the in-game list shows them
enum {
  K_UP,
  K_LEFT,
  K_DOWN,
  K_RIGHT,
  K_BLINK,
  K_SPRINT,
  K_INV,
  K_CROUCH,
  K_SAVE,
  K_CONSOLE,
  K_N
};

typedef struct {
  int width, height, mapsize;
  float music, sound; // volumes 0..1
  float sens, smooth; // mouse: sensitivity -.5..+.5, smoothing 0..2
  int invert, vsync, fps, intro;
  int framelimit; // 0 = off, else 20..119
  int hud;        // HUD_OFF / HUD_ALWAYS / HUD_AUTO
  float fognear, fogfar;
  int keys[K_N]; // directinput scancodes, like the file. key_dik() -> raylib
} Settings;

void settings_load(Settings *s, const char *path);
void settings_save(const Settings *s, const char *path); // the editable ones
int settings_key(const Settings *s, int k);              // the raylib key
void settings_apply(const Settings *s); // the live ones: vsync, frame limit
// the options screen: GRAPHICS / AUDIO / CONTROLS / ADVANCED, edits *s live,
// *tab keeps which one is open. the main menu's (x, y = the tab strip's frame)
// + the pause menu's (x, y = its first row) are laid out differently
void settings_screen(Settings *s, int *tab, int x, int y, int ingame);
