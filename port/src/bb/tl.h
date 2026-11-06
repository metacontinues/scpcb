// tl.h: blitz Type lists. objects are gc'd, the node goes first in every type:
//   typedef struct Doors { lnode n; int open; str name; } Doors;   TLIST(Doors);  (in one .c)
//   Doors *d = bbNew(Doors);   each(Doors, d) { ... }   bbDelete(d);
// deleting the current object inside each() is fine (next is grabbed up front).
// a deleted object stays valid memory until unreferenced, its n.ls goes NULL.
#pragma once
#include <gc.h>

typedef struct tlist tlist;
typedef struct lnode { struct lnode *prev, *next; tlist *ls; } lnode;
struct tlist { lnode *first, *last; };

#define TLIST(T) tlist T##_l
#define TLIST_DECL(T) extern tlist T##_l

static inline void tl_ins(tlist *l, lnode *x, lnode *after) { // after NULL = front
	x->ls = l; x->prev = after;
	x->next = after ? after->next : l->first;
	if (x->next) x->next->prev = x; else l->last = x;
	if (after) after->next = x; else l->first = x;
}
static inline void tl_unlink(lnode *x) {
	tlist *l = x->ls;
	if (!l) return;
	if (x->prev) x->prev->next = x->next; else l->first = x->next;
	if (x->next) x->next->prev = x->prev; else l->last = x->prev;
	x->prev = x->next = NULL; x->ls = NULL;
}
static inline void *tl_new(tlist *l, size_t sz) { lnode *x = GC_MALLOC(sz); tl_ins(l, x, l->last); return x; }

#define bbNew(T) ((T *)tl_new(&T##_l, sizeof(T)))
#define bbDelete(p) tl_unlink(&(p)->n)
#define bbDeleteEach(T) do { while (T##_l.first) tl_unlink(T##_l.first); } while (0)
#define bbFirst(T) ((T *)T##_l.first)
#define bbLast(T) ((T *)T##_l.last)
#define bbAfter(p) ((__typeof__(p))(p)->n.next)
#define bbBefore(p) ((__typeof__(p))(p)->n.prev)
#define bbInsertBefore(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_->prev); } while (0)
#define bbInsertAfter(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_); } while (0)
#define each(T, v) for (T *v = bbFirst(T), *v##_nx = v ? bbAfter(v) : NULL; v; v = v##_nx, v##_nx = v ? bbAfter(v) : NULL)
