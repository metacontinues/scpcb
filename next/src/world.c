// world.c: room templates (Data/rooms.ini) + the bits of a placed room that
// don't care how the map was generated
#define _POSIX_C_SOURCE 200809L
#include "world.h"
#include "ini.h"
#include "raylib.h"
#include "raymath.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void lower(char *s) {
  for (; *s; s++)
    if (*s >= 'A' && *s <= 'Z')
      *s += 32;
}

int world_templates(World *w, const char *path) {
  Ini ini;
  if (!ini_load(&ini, path))
    return 0;
  const char *secs[512];
  int n = ini_sections(&ini, secs, 512);
  if (n > 512)
    n = 512;
  w->tmpl = calloc(n, sizeof *w->tmpl);
  for (int i = 0; i < n; i++) {
    const char *mesh = ini_str(&ini, secs[i], "mesh path", NULL),
               *shape = ini_str(&ini, secs[i], "shape", "");
    if (!mesh)
      continue; // [room ambience] & co
    RoomTmpl *t = &w->tmpl[w->ntmpl++];
    snprintf(t->name, sizeof t->name, "%s", secs[i]);
    lower(t->name);
    // "GFX\map\room2_opt.rmesh" -> "gfx/map/room2_opt", the baker's id
    int k = 0;
    for (const char *c = mesh; *c && k < (int)sizeof t->mesh - 1; c++)
      t->mesh[k++] = *c == '\\' ? '/' : *c == ' ' ? '_' : *c;
    t->mesh[k] = 0;
    lower(t->mesh);
    char *dot = strrchr(t->mesh, '.');
    if (dot)
      *dot = 0;
    static const char *sh[] = {"room1", "room2", "room2c", "room3", "room4"},
                      *sh2[] = {"1", "2", "2c", "3", "4"};
    for (int s = 0; s < NSHAPES; s++)
      if (!strcasecmp(shape, sh[s]) || !strcasecmp(shape, sh2[s]))
        t->shape = s;
    for (int z = 0; z < 5; z++) {
      char key[8];
      snprintf(key, sizeof key, "zone%d", z + 1);
      t->zone[z] = ini_int(&ini, secs[i], key, 0);
    }
    t->common = (int)clampf(ini_int(&ini, secs[i], "commonness", 0), 0, 100);
    t->large = ini_int(&ini, secs[i], "large", 0);
    t->nooverlap = ini_int(&ini, secs[i], "disableoverlapcheck", 0);
  }
  ini_free(&ini);
  return 1;
}

RoomTmpl *world_tmpl(World *w, const char *name) {
  for (int i = 0; i < w->ntmpl; i++)
    if (!strcasecmp(w->tmpl[i].name, name))
      return &w->tmpl[i];
  return NULL;
}

Matrix room_matrix(Vector3 pos, int angle) {
  return MatrixMultiply(MatrixRotateY(angle * DEG2RAD),
                        MatrixTranslate(pos.x, pos.y, pos.z));
}

void world_free(World *w) {
  for (int i = 0; i < w->ntmpl; i++)
    if (w->tmpl[i].loaded)
      room_free(&w->tmpl[i].rm);
  free(w->tmpl);
  free(w->rooms);
  free(w->doors);
  free(w->items);
  *w = (World){0};
}
