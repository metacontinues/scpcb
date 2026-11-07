// ini.c: the whole file in one buffer, cut up in place
#define _POSIX_C_SOURCE 200809L
#include "ini.h"
#include "raylib.h"
#include <ctype.h>
#include <stdio.h>
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

// is line (up to its end) the key `key` (case-insensitive, spaces around ok)?
static int iskey(const char *l, const char *end, const char *key) {
  while (l < end && isspace((unsigned char)*l))
    l++;
  size_t n = strlen(key);
  if ((size_t)(end - l) < n || strncasecmp(l, key, n))
    return 0;
  l += n;
  while (l < end && (*l == ' ' || *l == '\t'))
    l++;
  return l < end && *l == '=';
}

static int issec(const char *l, const char *end, const char *sec) {
  while (l < end && isspace((unsigned char)*l))
    l++;
  size_t n = strlen(sec);
  return l < end && *l == '[' && (size_t)(end - l) > n + 1 &&
         !strncasecmp(l + 1, sec, n) && l[n + 1] == ']';
}

int ini_set(const char *path, const char *sec, const char *key,
            const char *val) {
  char *t = LoadFileText(path);
  const char *src = t ? t : "";
  const char *nl = strstr(src, "\r\n") ? "\r\n" : "\n";
  size_t n = strlen(src),
         cap = n + strlen(sec) + strlen(key) + strlen(val) + 32;
  char *o = malloc(cap), *w = o;
  int insec = 0, done = 0;
  const char *at = NULL; // where to add the key: after the section's last line
  for (const char *l = src; *l;) {
    const char *e = strchr(l, '\n');
    e = e ? e + 1 : src + n;
    const char *le = e; // line w/o its newline
    while (le > l && (le[-1] == '\n' || le[-1] == '\r'))
      le--;
    const char *p = l;
    while (p < le && isspace((unsigned char)*p))
      p++;
    if (p < le && *p == '[')
      insec = issec(l, le, sec);
    if (insec && !done && iskey(l, le, key)) {
      w += snprintf(w, cap - (size_t)(w - o), "%s = %s%s", key, val, nl);
      done = 1;
    } else {
      memcpy(w, l, e - l);
      w += e - l;
    }
    if (insec && p < le)
      at = w;
    l = e;
  }
  *w = 0;
  if (!done) { // insert after the section's last non-empty line, or append it
    char add[512];
    if (at)
      snprintf(add, sizeof add, "%s = %s%s", key, val, nl);
    else
      snprintf(add, sizeof add, "%s[%s]%s%s = %s%s", nl, sec, nl, key, val, nl);
    size_t k = strlen(add), off = at ? (size_t)(at - o) : (size_t)(w - o);
    o = realloc(o, cap + k);
    memmove(o + off + k, o + off, strlen(o + off) + 1);
    memcpy(o + off, add, k);
  }
  int ok = SaveFileText(path, o);
  free(o);
  if (t)
    UnloadFileText(t);
  return ok;
}
