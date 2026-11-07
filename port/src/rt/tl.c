// tl.c: blitz object handles. table is gc memory so it keeps its objects alive
#include "tl.h"

__attribute__((constructor)) static void gcinit(void) { GC_INIT(); }

static lnode **ht;
static int nh, ch;

int tl_handle(lnode *x) {
	if (x->h) return x->h;
	if (nh + 1 >= ch) ht = GC_REALLOC(ht, sizeof *ht * (ch = ch ? ch * 2 : 256));
	ht[++nh] = x;
	return x->h = nh;
}
lnode *tl_object(int h) { return h > 0 && h <= nh && ht[h]->ls ? ht[h] : NULL; }
