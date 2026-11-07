// props.c: the model cache. entries are allocated one by one so the Mdl
// pointers handed out stay put while the cache grows
#include "props.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char id[128];
  Mdl m;
  int ok;
} Prop;
static Prop **ps;
static int np;

Mdl *prop_get(const char *id) {
  for (int i = 0; i < np; i++)
    if (!strcmp(ps[i]->id, id))
      return ps[i]->ok ? &ps[i]->m : NULL;
  ps = realloc(ps, sizeof *ps * (np + 1));
  Prop *p = ps[np++] = calloc(1, sizeof *p);
  snprintf(p->id, sizeof p->id, "%s", id);
  p->ok = mdl_load(&p->m, id);
  if (!p->ok)
    TraceLog(LOG_WARNING, "props: %s isn't baked", id);
  return p->ok ? &p->m : NULL;
}

void prop_clear(void) {
  for (int i = 0; i < np; i++) {
    if (ps[i]->ok)
      mdl_free(&ps[i]->m);
    free(ps[i]);
  }
  free(ps);
  ps = NULL;
  np = 0;
}
