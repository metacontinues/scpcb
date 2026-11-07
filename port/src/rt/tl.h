// tl.h: blitz Type lists. objects are gc'd, the node goes first in every type:
//   typedef struct Doors { lnode n; int open; str name; } Doors;   TLIST(Doors);  (in one .c)
//   Doors *d = lnew(Doors);   each(Doors, d) { ... }   ldel(d);
// deleting the current object inside each() is fine (next is grabbed up front).
// a deleted object stays valid memory until unreferenced, its n.ls goes NULL.
// Handle(p)/Object.T(h): lhandle(p) / lobject(T, h). the handle table pins objects, like blitz.
#pragma once
#include <gc.h>

typedef struct tlist tlist;
typedef struct lnode { struct lnode *prev, *next; tlist *ls; int h; } lnode;
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

#define lnew(T) ((T *)tl_new(&T##_l, sizeof(T)))
#define ldel(p) tl_unlink(&(p)->n)
#define ldel_each(T) do { while (T##_l.first) tl_unlink(T##_l.first); } while (0)
// blitz turns every ref to a deleted object into Null; here they stay readable, so read through llive where that matters
#define llive(p) ((p) && (p)->n.ls ? (p) : NULL)
#define lfirst(T) ((T *)T##_l.first)
#define llast(T) ((T *)T##_l.last)
#define lafter(p) ((__typeof__(p))(p)->n.next)
#define lbefore(p) ((__typeof__(p))(p)->n.prev)
#define linsert_before(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_->prev); } while (0)
#define linsert_after(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_); } while (0)
#define each(T, v) for (T *v = lfirst(T), *v##_nx = v ? lafter(v) : NULL; v; v = v##_nx, v##_nx = v ? lafter(v) : NULL)

int tl_handle(lnode *x);
lnode *tl_object(int h); // NULL for 0, unknown or deleted
#define lhandle(p) ((p) ? tl_handle(&(p)->n) : 0)
#define lobject(T, h) ((T *)tl_obj_of(&T##_l, h))
static inline lnode *tl_obj_of(tlist *l, int h) { lnode *x = tl_object(h); return x && x->ls == l ? x : NULL; }
