// snd.c: the sound cache, the voice pool, the original's 3d + music fades
#include "snd.h"
#include "asset.h"
#include "raylib.h"
#include "raymath.h"
#include "util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char id[128];
  Sound s;
} Cached;
static Cached *cache;
static int ncache;

#define VOICES 64
typedef struct {
  Sound a; // alias of a cached sound
  unsigned gen;
  int used;
} Voice;
static Voice voices[VOICES];
static int nextv;
static float sfxvol = 1, musvol = 1;
static Vector3 eye;
static float eyeyaw;

static Music mus;
static char want[128], playing[128];
static float musfade; // current music volume, eases toward musvol

void snd_init(float sfx, float music) {
  InitAudioDevice();
  sfxvol = sfx;
  musvol = music;
}

void snd_close(void) {
  if (mus.stream.buffer)
    UnloadMusicStream(mus);
  for (int i = 0; i < VOICES; i++)
    if (voices[i].used)
      UnloadSoundAlias(voices[i].a);
  for (int i = 0; i < ncache; i++)
    UnloadSound(cache[i].s);
  free(cache);
  cache = NULL;
  ncache = 0;
  CloseAudioDevice();
}

void snd_volumes(float sfx, float music) {
  sfxvol = sfx;
  musvol = music;
}

void snd_listener(Vector3 e, float yaw) {
  eye = e;
  eyeyaw = yaw;
}

static Sound *get(const char *id) {
  for (int i = 0; i < ncache; i++)
    if (!strcmp(cache[i].id, id))
      return cache[i].s.frameCount ? &cache[i].s : NULL;
  cache = realloc(cache, sizeof *cache * (ncache + 1));
  Cached *c = &cache[ncache++];
  snprintf(c->id, sizeof c->id, "%s", id);
  c->s = LoadSound(asset_path(id, ".qoa"));
  if (!c->s.frameCount)
    TraceLog(LOG_WARNING, "snd: %s isn't baked", id);
  return c->s.frameCount ? &c->s : NULL;
}

static Voice *voice(int v) {
  int i = v & 0xff;
  if (!v || i >= VOICES || !voices[i].used || voices[i].gen != (unsigned)v >> 8)
    return NULL;
  return &voices[i];
}

// a free voice, else the next one round-robin (steals the oldest-ish)
static int grab(Sound *s) {
  int i = nextv;
  for (int k = 0; k < VOICES; k++) {
    int j = (nextv + k) % VOICES;
    if (!voices[j].used || !IsSoundPlaying(voices[j].a)) {
      i = j;
      break;
    }
  }
  nextv = (i + 1) % VOICES;
  Voice *x = &voices[i];
  if (x->used) {
    StopSound(x->a);
    UnloadSoundAlias(x->a);
  }
  x->a = LoadSoundAlias(*s);
  x->used = 1;
  x->gen = (x->gen + 1) & 0x7fffff; // handle = gen << 8 | slot, stays positive
  return (int)(x->gen << 8 | (unsigned)i);
}

int snd_play(const char *id, float vol) {
  static int log = -1;
  if (log < 0)
    log = getenv("SCPCB_SNDLOG") != NULL;
  if (log) // dev: what plays when
    printf("snd %s %.2f\n", id, vol);
  Sound *s = get(id);
  if (!s)
    return 0;
  int v = grab(s);
  Voice *x = voice(v);
  SetSoundVolume(x->a, clampf(vol * sfxvol, 0, 1));
  SetSoundPan(x->a, 0);
  PlaySound(x->a);
  return v;
}

// the original's PlaySound2: distance / range -> volume, view angle -> pan
static int audible(Vector3 at, float range, float *gain, float *pan) {
  Vector3 d = Vector3Subtract(at, eye);
  float dist = Vector3Length(d) / maxf(range, 1);
  float to = atan2f(-d.x, -d.z) * RAD2DEG,
        delta = wrapdeg(to - eyeyaw + 180) - 180; // > 0: to the left
  *gain = 1 - dist;
  *pan = sinf(-delta * DEG2RAD);
  return *gain > 0 && *gain < 1;
}

int snd_play3d(const char *id, Vector3 at, float range, float vol) {
  float g, pan;
  if (vol <= 0 || !audible(at, range, &g, &pan))
    return 0;
  int v = snd_play(id, vol * g);
  Voice *x = voice(v);
  if (x)
    SetSoundPan(x->a, pan);
  return v;
}

void snd_place(int v, Vector3 at, float range, float vol) {
  Voice *x = voice(v);
  if (!x)
    return;
  float g, pan;
  audible(at, range, &g, &pan);
  SetSoundVolume(x->a, clampf(vol * maxf(g, 0) * sfxvol, 0, 1));
  SetSoundPan(x->a, pan);
}

int snd_playing(int v) {
  Voice *x = voice(v);
  return x && IsSoundPlaying(x->a);
}

void snd_stop(int v) {
  Voice *x = voice(v);
  if (x)
    StopSound(x->a);
}

void snd_music(const char *id) { snprintf(want, sizeof want, "%s", id); }

// the original: a wrong track fades out over 250 ticks, then the right one
// starts and eases in 10% of the way per tick
void snd_tick(void) {
  if (strcmp(want, playing)) {
    musfade = maxf(musfade - 1 / 250.0f, 0);
    if (musfade > 0 && playing[0])
      return;
    if (mus.stream.buffer)
      UnloadMusicStream(mus);
    mus = (Music){0};
    snprintf(playing, sizeof playing, "%s", want);
    if (playing[0]) {
      mus = LoadMusicStream(asset_path(playing, ".qoa"));
      if (mus.stream.buffer)
        PlayMusicStream(mus);
    }
  } else
    musfade += (musvol - musfade) * .1f;
  if (mus.stream.buffer)
    SetMusicVolume(mus, musfade);
}

void snd_frame(void) {
  if (mus.stream.buffer)
    UpdateMusicStream(mus);
}
