// tl.c: object alloc, deferred free, int handles (handles are never reused, so a stale one just reads as NULL)
#include <stdlib.h>
#include "tl.h"

void *tl_new(tlist *l, size_t sz) { lnode *x = calloc(1, sz); tl_ins(l, x, l->last); return x; }

typedef struct { lnode *x; void (*dtor)(void *); } grave;
static grave *gv;
static int ng, cg;
static lnode **ht;
static int nh, ch;

void tl_del(lnode *x) {
	tlist *l = x->ls;
	if (!l) return; // already deleted
	tl_unlink(x);
	if (x->h) ht[x->h] = NULL;
	if (ng >= cg) gv = realloc(gv, sizeof *gv * (cg = cg ? cg * 2 : 256));
	gv[ng++] = (grave){x, l->dtor};
}
void tl_flush(void) {
	// a dtor may delete more objects, so walk by index while the list grows
	for (int i = 0; i < ng; i++) if (gv[i].dtor) gv[i].dtor(gv[i].x);
	for (int i = 0; i < ng; i++) free(gv[i].x);
	ng = 0;
}

int tl_handle(lnode *x) {
	if (x->h) return x->h;
	if (nh + 1 >= ch) ht = realloc(ht, sizeof *ht * (ch = ch ? ch * 2 : 256));
	ht[++nh] = x;
	return x->h = nh;
}
lnode *tl_object(int h) { return h > 0 && h <= nh && ht[h] && ht[h]->ls ? ht[h] : NULL; }
