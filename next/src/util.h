// util.h: small shared bits. rng is raylib's (SetRandomSeed / GetRandomValue)
#pragma once
#include "raylib.h"
#include <math.h>

// sim rate: everything gameplay is tuned in 1/70 s steps, like the original
#define TICK 70.0f
#define RSCALE (8.0f / 2048.0f) // room units -> world units

static inline int irnd(int lo, int hi) { return GetRandomValue(lo, hi); }
static inline float frnd(float lo, float hi) {
  return lo + (hi - lo) * (GetRandomValue(0, 1 << 24) / 16777216.0f);
}
static inline float minf(float a, float b) { return a < b ? a : b; }
static inline float maxf(float a, float b) { return a > b ? a : b; }
static inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : v > hi ? hi : v;
}
static inline float wrapdeg(float a) {
  a = fmodf(a, 360);
  return a < 0 ? a + 360 : a;
}
// ease old toward v, 1/smooth of the way per tick, never overshooting (blitz's
// CurveValue at 1 tick)
static inline float curve(float v, float old, float smooth) {
  float n = old + (v - old) / smooth;
  return v < old ? maxf(n, v) : minf(n, v);
}
