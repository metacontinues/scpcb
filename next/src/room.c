// room.c: load a baked room (<id>.glb + <id>.col.glb + <id>.txt)
#include "room.h"
#include "asset.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "shd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PUSH(a, n, v)                                                          \
  do {                                                                         \
    (a) = realloc((a), sizeof *(a) * ((n) + 1));                               \
    (a)[(n)++] = (v);                                                          \
  } while (0)

static void mat(RoomMesh *r, int i, const char *kind, const char *diff,
                const char *lm) {
  static const char *ks[] = {"lit", "vert", "flat", "alpha"};
  int k = MK_FLAT;
  for (int j = 0; j < MK_MODEL; j++)
    if (!strcmp(kind, ks[j]))
      k = j;
  if (i + 1 >= r->vis.materialCount)
    return; // raylib puts gltf material i at i + 1 (0 = default)
  Material *m = &r->vis.materials[i + 1];
  m->shader = shd_world[k];
  m->maps[MATERIAL_MAP_ALBEDO].texture = tex_get(diff);
  m->maps[MATERIAL_MAP_ALBEDO].color = WHITE;
  m->maps[MATERIAL_MAP_OCCLUSION].texture = tex_get(lm);
  r->kind[i + 1] = k;
  free(r->diff[i + 1]);
  r->diff[i + 1] = malloc(strlen(diff) + 1);
  memcpy(r->diff[i + 1], diff, strlen(diff) + 1);
}

int room_load(RoomMesh *r, const char *id) {
  *r = (RoomMesh){0};
  char *txt = LoadFileText(asset_path(id, ".txt"));
  if (!txt)
    return 0;
  r->vis = LoadModel(asset_path(id, ".glb"));
  r->col = LoadModel(asset_path(id, ".col.glb"));
  r->kind = calloc(r->vis.materialCount, sizeof *r->kind);
  r->diff = calloc(r->vis.materialCount, sizeof *r->diff);
  for (int i = 0; i < r->vis.materialCount; i++)
    r->kind[i] = MK_FLAT;
  for (char *l = txt, *e; l && *l; l = e) {
    if ((e = strchr(l, '\n')))
      *e++ = 0;
    char a[256], b[256], c[256];
    RLight li = {0};
    RSound so;
    RScreen sc;
    RProp pr;
    RTrig tr;
    Vector3 v;
    int i, cr, cg, cb;
    float *m = &pr.m.m0;
    if (sscanf(l, "mat %d %255s %255s %255s", &i, a, b, c) == 4)
      mat(r, i, a, b, c);
    else if (sscanf(l, "light %f %f %f %f %d %d %d", &li.p.x, &li.p.y, &li.p.z,
                    &li.range, &cr, &cg, &cb) == 7) {
      li.c = (Color){cr, cg, cb, 255};
      PUSH(r->lights, r->nlights, li);
    } else if (sscanf(l, "spot %f %f %f %f %d %d %d %f %f %f %d %d", &li.p.x,
                      &li.p.y, &li.p.z, &li.range, &cr, &cg, &cb, &li.dir.x,
                      &li.dir.y, &li.dir.z, &li.inner, &li.outer) == 12) {
      li.c = (Color){cr, cg, cb, 255};
      li.spot = 1;
      PUSH(r->lights, r->nlights, li);
    } else if (sscanf(l, "sound %f %f %f %d %f", &so.p.x, &so.p.y, &so.p.z,
                      &so.id, &so.range) == 5)
      PUSH(r->sounds, r->nsounds, so);
    else if (sscanf(l, "screen %f %f %f %127s", &sc.p.x, &sc.p.y, &sc.p.z,
                    sc.img) == 4)
      PUSH(r->screens, r->nscreens, sc);
    else if (sscanf(l, "waypoint %f %f %f", &v.x, &v.y, &v.z) == 3)
      PUSH(r->wps, r->nwps, v);
    else if (sscanf(l, "start %f %f %f %f %f %f", &r->start.x, &r->start.y,
                    &r->start.z, &r->startdir.x, &r->startdir.y,
                    &r->startdir.z) == 6)
      r->hasstart = 1;
    else if (sscanf(l, "trig %63s %f %f %f %f %f %f", tr.name, &tr.box.min.x,
                    &tr.box.min.y, &tr.box.min.z, &tr.box.max.x, &tr.box.max.y,
                    &tr.box.max.z) == 7)
      PUSH(r->trigs, r->ntrigs, tr);
    else if (sscanf(
                 l,
                 "prop %127s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                 pr.model, m, m + 1, m + 2, m + 3, m + 4, m + 5, m + 6, m + 7,
                 m + 8, m + 9, m + 10, m + 11, m + 12, m + 13, m + 14,
                 m + 15) == 17)
      PUSH(r->props, r->nprops, pr);
  }
  UnloadFileText(txt);
  return r->vis.meshCount > 0;
}

void room_free(RoomMesh *r) {
  for (int i = 0; i < r->vis.materialCount;
       i++) { // textures + shaders are shared, UnloadModel skips the defaults
    Material *m = &r->vis.materials[i];
    for (int k = 0; k <= MATERIAL_MAP_BRDF; k++)
      m->maps[k].texture.id = rlGetTextureIdDefault();
    m->shader = (Shader){rlGetShaderIdDefault(), rlGetShaderLocsDefault()};
  }
  UnloadModel(r->vis);
  UnloadModel(r->col);
  for (int i = 0; r->diff && i < r->vis.materialCount; i++)
    free(r->diff[i]);
  free(r->diff);
  free(r->kind);
  free(r->lights);
  free(r->sounds);
  free(r->screens);
  free(r->props);
  free(r->trigs);
  free(r->wps);
  *r = (RoomMesh){0};
}

void room_draw(const RoomMesh *r, Matrix m, int alpha) {
  for (int i = 0; i < r->vis.meshCount; i++) {
    int mi = r->vis.meshMaterial[i];
    if ((r->kind[mi] == MK_ALPHA) != (alpha != 0))
      continue;
    DrawMesh(r->vis.meshes[i], r->vis.materials[mi],
             MatrixMultiply(r->vis.transform, m));
  }
}
