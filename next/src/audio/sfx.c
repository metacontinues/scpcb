// sfx.c: the original's MovePlayer / main-loop sounds
#define _POSIX_C_SOURCE 200809L
#include "audio/sfx.h"
#include "audio/snd.h"
#include "core/ini.h"
#include "core/util.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

static Ini mats;
static int breath, amb;
static Vector3 ambat;

void sfx_init(void) { ini_load(&mats, "Data/materials.ini"); }

// the map's +z rows: entrance zone up to row 6, heavy up to 12, then light
int world_zone(Vector3 pos) {
  float row = -pos.z / 8;
  return row < 6 ? 2 : row < 12 ? 1 : 0;
}

// what the floor under pos is made of: 1 = metal (materials.ini stepsound)
static int stepset(World *w, Vector3 pos) {
  Ray down = {pos, {0, -1, 0}};
  RayCollision best = {0};
  const char *tex = NULL;
  for (int i = 0; i < w->nrooms; i++) {
    Room *r = &w->rooms[i];
    if (!r->t || r->t->loaded <= 0 || pos.x < r->ext.min.x ||
        pos.x > r->ext.max.x || pos.z < r->ext.min.z || pos.z > r->ext.max.z)
      continue;
    Model *m = &r->t->rm.vis;
    for (int k = 0; k < m->meshCount; k++) {
      RayCollision h = GetRayCollisionMesh(down, m->meshes[k],
                                           MatrixMultiply(m->transform, r->m));
      if (h.hit && h.distance < 1 &&
          (!best.hit || h.distance < best.distance)) {
        best = h;
        tex = r->t->rm.diff[m->meshMaterial[k]];
      }
    }
  }
  if (!tex)
    return 0;
  const char *base = strrchr(tex, '/');
  base = base ? base + 1 : tex;
  char sec[160];
  snprintf(sec, sizeof sec, "%s.jpg",
           base); // materials.ini names the source file
  int s = ini_int(&mats, sec, "stepsound", -1);
  if (s < 0) {
    snprintf(sec, sizeof sec, "%s.png", base);
    s = ini_int(&mats, sec, "stepsound", 0);
  }
  return s == 1;
}

static void step(World *w, const Player *p, int run) {
  char id[64];
  snprintf(id, sizeof id, "sfx/step/%s%s%d", run ? "run" : "step",
           stepset(w, p->pos) ? "metal" : "", irnd(1, 8));
  snd_play(id, 1 - p->crouch * .6f);
}

void sfx_tick(World *w, const Player *p, int sprintheld) {
  if (p->step)
    step(w, p, p->step == 2);
  if (p->landed)
    step(w, p, 0);
  // out of breath (the original only does it while you hold sprint)
  if (sprintheld) {
    char id[48];
    if (p->stamina < 5) {
      if (!snd_playing(breath))
        breath = snd_play("sfx/character/d9341/breath0", 1);
    } else if (p->stamina < 50 && !snd_playing(breath)) {
      snprintf(id, sizeof id, "sfx/character/d9341/breath%d", irnd(1, 3));
      breath = snd_play(id, minf((70 - p->stamina) / 70, 1));
    }
  }
  // now and then something somewhere in the zone (a third of the time:
  // anywhere)
  int zone = world_zone(p->pos);
  if (irnd(1, 1500) == 1) {
    static const int count[] = {11, 11, 12, 15};
    int z = irnd(1, 3) == 1 ? 3 : zone;
    char id[64];
    if (z == 3)
      snprintf(id, sizeof id, "sfx/ambient/general/ambient%d",
               irnd(1, count[3]));
    else
      snprintf(id, sizeof id, "sfx/ambient/zone%d/ambient%d", z + 1,
               irnd(1, count[z]));
    Vector3 e = player_eye(p);
    ambat = (Vector3){e.x + frnd(-1, 1), 0, e.z + frnd(-1, 1)};
    amb = snd_play3d(id, ambat, 10, 1);
  }
  snd_place(amb, ambat, 10, 1);
  static const char *music[] = {"sfx/music/the_dread",
                                "sfx/music/heavycontainment",
                                "sfx/music/entrancezone"};
  snd_music(music[zone]);
}
