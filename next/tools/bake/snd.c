// snd.c: every sound under SFX/ -> <id>.qoa (raylib reads + writes qoa itself).
// files are recognised by content: the game hides an ogg behind ".cwm"
#define _POSIX_C_SOURCE 200809L
#include "bake.h"
#include "raylib.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int nsnd, nbad;

static const char *kind(const unsigned char *d, int n) {
  if (n >= 4 && !memcmp(d, "OggS", 4))
    return ".ogg";
  if (n >= 12 && !memcmp(d, "RIFF", 4) && !memcmp(d + 8, "WAVE", 4))
    return ".wav";
  return NULL;
}

static void one(const char *src) {
  char id[1024], out[2048];
  mkid(src, id, sizeof id);
  snprintf(out, sizeof out, OUT "%s.qoa", id);
  if (!stale(src, out)) {
    nsnd++;
    return;
  }
  int n;
  unsigned char *d = LoadFileData(src, &n);
  const char *k = d ? kind(d, n) : NULL;
  if (!k) { // not audio (the readme txts)
    UnloadFileData(d);
    return;
  }
  Wave w = LoadWaveFromMemory(k, d, n);
  UnloadFileData(d);
  if (!w.data) {
    warn("can't decode %s", src);
    nbad++;
    return;
  }
  WaveFormat(&w, w.sampleRate, 16, w.channels); // qoa wants 16 bit
  mkdirs(out);
  if (!ExportWave(w, out)) {
    warn("can't write %s", out);
    nbad++;
  } else
    nsnd++;
  UnloadWave(w);
}

static void walk(const char *dir) {
  DIR *d = opendir(dir);
  if (!d)
    return;
  for (struct dirent *e; (e = readdir(d));) {
    if (e->d_name[0] == '.')
      continue;
    char p[2048];
    snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
    struct stat st;
    if (stat(p, &st))
      continue;
    if (S_ISDIR(st.st_mode))
      walk(p);
    else if (S_ISREG(st.st_mode))
      one(p);
  }
  closedir(d);
}

int bake_sounds(int *count) {
  nsnd = nbad = 0;
  walk("SFX");
  *count = nsnd;
  return nbad;
}
