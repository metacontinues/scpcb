// ini.h: read-only ini files. sections + keys are case-insensitive, values come
// back trimmed
#pragma once

typedef struct {
  char *sec, *key, *val;
} IniKV;
typedef struct {
  IniKV *kv;
  int n;
  char *buf;
} Ini;

int ini_load(Ini *o, const char *path); // 0 if unreadable
void ini_free(Ini *o);
const char *ini_str(const Ini *o, const char *sec, const char *key,
                    const char *def);
int ini_int(const Ini *o, const char *sec, const char *key, int def);
float ini_float(const Ini *o, const char *sec, const char *key, float def);
// sections in file order, for walking every [room] in rooms.ini. returns the
// count, fills up to max
int ini_sections(const Ini *o, const char **out, int max);
