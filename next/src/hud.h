// hud.h: what's drawn over the world: the eyes-shut darkness, the blink +
// stamina meters, the hand icon over things you can use, messages
#pragma once
#include "raylib.h"

typedef struct {
  float dark;               // 0..1 black over the world (blinking)
  float blink, blinkfreq;   // the blink meter: blink / blinkfreq full
  float stamina;            // 0..100
  int crouch;               // the stamina meter's icon: crouch or sprint
  int hand;                 // something usable in reach
  float handyaw, handpitch; // where it is, relative to the view (degrees)
} HudIn;

void hud_init(int sw, int sh);
// shown for `ticks`, at alpha min(ticks left / 2, 255) / 255 like the original
void hud_msg(const char *text, float ticks);
void hud_tick(void);
void hud_draw(const HudIn *in);
