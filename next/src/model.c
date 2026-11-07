// model.c: load + draw a baked model (<id>.glb + <id>.txt)
#include "model.h"
#include "asset.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "shd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void mat(Mdl *o, int i, float r, float g, float b, float a,
                const char *fl, const char *bl, const char *tex) {
  if (i + 1 >= o->m.materialCount)
    return; // raylib puts gltf material i at i + 1 (0 = default)
  Material *m = &o->m.materials[i + 1];
  MdlMat *x = &o->mat[i + 1];
  int full = strchr(fl, 'f') != NULL, vcol = strchr(fl, 'v') != NULL;
  m->shader = shd_world[MK_MODEL + full + vcol * 2];
  m->maps[MATERIAL_MAP_ALBEDO].texture = tex_get(tex);
  m->maps[MATERIAL_MAP_ALBEDO].color = (Color){
      roundf(r * 255), roundf(g * 255), roundf(b * 255), roundf(a * 255)};
  x->twosided = strchr(fl, '2') != NULL;
  x->blend = !strcmp(bl, "mul")   ? BLEND_MULTIPLIED
             : !strcmp(bl, "add") ? BLEND_ADDITIVE
                                  : BLEND_ALPHA;
  size_t n = strlen(tex);
  int texa =
      n > 2 && (!strcmp(tex + n - 2, ".a") || !strcmp(tex + n - 2, ".m") ||
                !strcmp(tex + n - 3, ".am"));
  x->alpha = a < 1 || texa || strchr(fl, 'a') || x->blend != BLEND_ALPHA;
}

int mdl_load(Mdl *o, const char *id) {
  *o = (Mdl){0};
  char *txt = LoadFileText(asset_path(id, ".txt"));
  if (!txt)
    return 0;
  o->m = LoadModel(asset_path(id, ".glb"));
  o->mat = calloc(o->m.materialCount, sizeof *o->mat);
  for (char *l = txt, *e; l && *l; l = e) {
    if ((e = strchr(l, '\n')))
      *e++ = 0;
    int i;
    float r, g, b, a;
    char fl[16], bl[16], tex[512];
    if (sscanf(l, "frames %d", &o->frames) == 1)
      continue;
    if (sscanf(l, "mat %d %f %f %f %f %15s %15s %511s", &i, &r, &g, &b, &a, fl,
               bl, tex) == 8)
      mat(o, i, r, g, b, a, fl, bl, tex);
  }
  UnloadFileText(txt);
  if (o->frames)
    o->anims = LoadModelAnimations(asset_path(id, ".glb"), &o->nanims);
  return o->m.meshCount > 0;
}

void mdl_free(Mdl *o) {
  for (int i = 0; i < o->m.materialCount;
       i++) { // textures + shaders are shared
    Material *m = &o->m.materials[i];
    for (int k = 0; k <= MATERIAL_MAP_BRDF; k++)
      m->maps[k].texture.id = rlGetTextureIdDefault();
    m->shader = (Shader){rlGetShaderIdDefault(), rlGetShaderLocsDefault()};
  }
  if (o->anims)
    UnloadModelAnimations(o->anims, o->nanims);
  UnloadModel(o->m);
  free(o->mat);
  *o = (Mdl){0};
}

void mdl_pose(Mdl *o, float frame) {
  if (o->nanims)
    UpdateModelAnimation(o->m, o->anims[0], frame);
}

void mdl_draw(const Mdl *o, Matrix m, int alpha) {
  Matrix t = MatrixMultiply(o->m.transform, m);
  for (int i = 0; i < o->m.meshCount; i++) {
    int mi = o->m.meshMaterial[i];
    const MdlMat *x = &o->mat[mi];
    if (x->alpha != (alpha != 0))
      continue;
    if (x->twosided)
      rlDisableBackfaceCulling();
    if (alpha)
      BeginBlendMode(x->blend);
    DrawMesh(o->m.meshes[i], o->m.materials[mi], t);
    if (alpha)
      EndBlendMode();
    if (x->twosided)
      rlEnableBackfaceCulling();
  }
}

int mdl_bone(const Mdl *o, const char *name) {
  for (unsigned i = 0; i < o->m.skeleton.boneCount; i++)
    if (!strcmp(o->m.skeleton.bones[i].name, name))
      return i;
  return -1;
}
