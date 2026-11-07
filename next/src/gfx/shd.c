// shd.c: world shaders (the glsl's in next/shaders). raylib binds the
// attributes + mvp/matModel/matView/matNormal + albedo (texture0) + colDiffuse
// itself; rooms' lightmap rides in MATERIAL_MAP_OCCLUSION, whose loc we point
// at "lightmap"
#include "gfx/shd.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Shader shd_world[MK_N];

// next/shaders/<name> (or $SCPCB_SHADERS/<name>) w/ "#version 100" + defs
// on top. NULL if it's missing (raylib then uses its default for that stage)
static char *src(const char *name, const char *defs) {
  static const char *root;
  if (!root)
    root = getenv("SCPCB_SHADERS") ? getenv("SCPCB_SHADERS") : "next/shaders";
  char p[1024];
  snprintf(p, sizeof p, "%s/%s", root, name);
  char *t = LoadFileText(p);
  if (!t) {
    TraceLog(LOG_ERROR, "shd: no %s", p);
    return NULL;
  }
  size_t n = strlen(t) + strlen(defs) + 16;
  char *o = malloc(n);
  snprintf(o, n, "#version 100\n%s%s", defs, t);
  UnloadFileText(t);
  return o;
}

static Shader load(const char *vs, const char *fs, const char *defs) {
  char *v = src(vs, defs), *f = src(fs, defs);
  Shader s = LoadShaderFromMemory(v, f);
  free(v);
  free(f);
  return s;
}

void shd_init(void) {
  char defs[128];
  for (int k = 0; k < MK_N; k++) {
    if (k < MK_MODEL) {
      snprintf(defs, sizeof defs, "#define KIND %d\n", k);
      shd_world[k] = load("room.vert", "room.frag", defs);
      shd_world[k].locs[SHADER_LOC_MAP_OCCLUSION] =
          GetShaderLocation(shd_world[k], "lightmap");
    } else {
      int v = k - MK_MODEL;
      snprintf(defs, sizeof defs,
               "#define FULLBRIGHT %d\n#define VCOL %d\n#define SKIN %d\n",
               v & 1, v >> 1 & 1, v >> 2 & 1);
      shd_world[k] = load("model.vert", "model.frag", defs);
    }
  }
  shd_ambient(0);
  shd_fog(0, 0, BLACK);
  shd_lights((Vector3){1, 1, 1}, 0, NULL, NULL);
}

void shd_fog(float near, float far, Color c) {
  float r[2] = {near, far}, col[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, 1};
  for (int k = 0; k < MK_N; k++) {
    SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogr"), r,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogc"), col,
                   SHADER_UNIFORM_VEC4);
  }
}

void shd_ambient(float a) {
  for (int k = 0; k < MK_MODEL; k++)
    SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "amb"), &a,
                   SHADER_UNIFORM_FLOAT);
}

void shd_lights(Vector3 amb, int n, const Vector4 *pos, const Vector4 *col) {
  if (n > SHD_LIGHTS)
    n = SHD_LIGHTS;
  for (int k = MK_MODEL; k < MK_N; k++) {
    Shader s = shd_world[k];
    SetShaderValue(s, GetShaderLocation(s, "lamb"), &amb, SHADER_UNIFORM_VEC3);
    SetShaderValue(s, GetShaderLocation(s, "nl"), &n, SHADER_UNIFORM_INT);
    if (n) {
      SetShaderValueV(s, GetShaderLocation(s, "lpos"), pos, SHADER_UNIFORM_VEC4,
                      n);
      SetShaderValueV(s, GetShaderLocation(s, "lcol"), col, SHADER_UNIFORM_VEC4,
                      n);
    }
  }
}
