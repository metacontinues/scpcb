// ini.c: the whole file in one buffer, cut up in place
#define _POSIX_C_SOURCE 200809L
#include "ini.h"
#include "raylib.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static char *trim(char *s) {
  while (isspace((unsigned char)*s))
    s++;
  char *e = s + strlen(s);
  while (e > s && isspace((unsigned char)e[-1]))
    *--e = 0;
  return s;
}

int ini_load(Ini *o, const char *path) {
  *o = (Ini){0};
  char *t = LoadFileText(path);
  if (!t)
    return 0;
  size_t n = strlen(t) + 1;
  o->buf = malloc(n);
  memcpy(o->buf, t, n);
  UnloadFileText(t);
  char *sec = "";
  for (char *l = o->buf, *e; l && *l; l = e) {
    if ((e = strpbrk(l, "\r\n")))
      *e++ = 0;
    l = trim(l);
    if (*l == '[') {
      char *c = strchr(l, ']');
      if (c)
        *c = 0;
      sec = trim(l + 1);
      continue;
    }
    char *eq = strchr(l, '=');
    if (!*l || *l == ';' || !eq)
      continue;
    *eq = 0;
    o->kv = realloc(o->kv, sizeof *o->kv * (o->n + 1));
    o->kv[o->n++] = (IniKV){sec, trim(l), trim(eq + 1)};
  }
  return 1;
}

void ini_free(Ini *o) {
  free(o->kv);
  free(o->buf);
  *o = (Ini){0};
}

const char *ini_str(const Ini *o, const char *sec, const char *key,
                    const char *def) {
  for (int i = 0; i < o->n; i++)
    if (!strcasecmp(o->kv[i].sec, sec) && !strcasecmp(o->kv[i].key, key))
      return o->kv[i].val;
  return def;
}
int ini_int(const Ini *o, const char *sec, const char *key, int def) {
  const char *s = ini_str(o, sec, key, NULL);
  return s ? atoi(s) : def;
}
float ini_float(const Ini *o, const char *sec, const char *key, float def) {
  const char *s = ini_str(o, sec, key, NULL);
  return s ? (float)atof(s) : def;
}

int ini_sections(const Ini *o, const char **out, int max) {
  int n = 0;
  for (int i = 0; i < o->n; i++) {
    if (i && o->kv[i].sec == o->kv[i - 1].sec)
      continue;
    int seen = 0;
    for (int k = 0; k < n && k < max; k++)
      if (out[k] == o->kv[i].sec)
        seen = 1;
    if (seen)
      continue;
    if (n < max)
      out[n] = o->kv[i].sec;
    n++;
  }
  return n;
}
