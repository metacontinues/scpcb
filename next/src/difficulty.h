// difficulty.h: the original's four (custom: edited in the new game tab).
// only the blink interval uses them so far
// (otherfactors), saving + npc aggression come w/ those systems
#pragma once
#include "raylib.h"

enum { D_SAFE, D_EUCLID, D_KETER, D_CUSTOM, D_N };
enum { F_EASY, F_NORMAL, F_HARD };
enum { SAVE_ANYWHERE, SAVE_SCREENS, SAVE_QUIT };

typedef struct {
  const char *name, *desc;
  Color c;
  int permadeath, aggressive, otherfactors, save, custom;
} Difficulty;

static const Difficulty difficulties[D_N] = {
    {"Safe",
     "The game can be saved any time. However, as in the case of SCP Objects, "
     "a Safe classification does not mean "
     "that handling it does not pose a threat.",
     {120, 150, 50, 255},
     0,
     0,
     F_EASY,
     SAVE_ANYWHERE,
     0},
    {"Euclid",
     "In Euclid difficulty, saving is only allowed at specific locations "
     "marked by lit up computer screens. "
     "Euclid-class objects are inherently unpredictable, so that reliable "
     "containment is not always possible.",
     {200, 200, 0, 255},
     0,
     0,
     F_NORMAL,
     SAVE_SCREENS,
     0},
    {"Keter",
     "Keter-class objects are considered the most dangerous ones in Foundation "
     "containment. The same can be said "
     "for this difficulty level: the SCPs are more aggressive, and you have "
     "only one life - when you die, the game "
     "is over. ",
     {200, 0, 0, 255},
     1,
     1,
     F_HARD,
     SAVE_QUIT,
     0},
    {"Custom", "", {255, 255, 255, 255}, 0, 1, F_EASY, SAVE_ANYWHERE, 1},
};

// ticks between blinks, lowest + range per otherfactors
static inline void blink_range(int f, float *lo, float *hi) {
  *lo = f == F_EASY ? 490 : f == F_NORMAL ? 455 : 420;
  *hi = *lo + 210;
}
