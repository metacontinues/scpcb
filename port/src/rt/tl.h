// tl.h: intrusive object lists. the node goes first in every type:
//   typedef struct Doors { lnode n; int open; str name; } Doors;   TLIST(Doors);  (in one .c)
//   Doors *d = lnew(Doors);   each(Doors, d) { ... }   ldel(d);
// objects are calloc'd. ldel unlinks right away (each() and lfirst/lafter skip it) but the memory stays until
// tl_flush (the main loop calls it once a frame), so deleting inside each(), even the next object, is fine and
// llive(p) tells a dead object apart for the rest of the frame. refs kept across frames must be cleared by
// whoever deletes (the Remove* fns do that). TLIST_DTOR(T, fn) runs fn on an object right before it's freed.
#pragma once
#include <stddef.h>

typedef struct tlist tlist;
typedef struct lnode { struct lnode *prev, *next; tlist *ls; int h; } lnode;
struct tlist { lnode *first, *last; void (*dtor)(void *); };

#define TLIST(T) tlist T##_l
#define TLIST_DTOR(T, fn) tlist T##_l = {.dtor = (void (*)(void *))(fn)}
#define TLIST_DECL(T) extern tlist T##_l

static inline void tl_ins(tlist *l, lnode *x, lnode *after) { // after NULL = front
	x->ls = l; x->prev = after;
	x->next = after ? after->next : l->first;
	if (x->next) x->next->prev = x; else l->last = x;
	if (after) after->next = x; else l->first = x;
}
// takes x out of its list. x->next keeps pointing on, so an each() standing on x can still step forward
static inline void tl_unlink(lnode *x) {
	tlist *l = x->ls;
	if (!l) return;
	if (x->prev) x->prev->next = x->next; else l->first = x->next;
	if (x->next) x->next->prev = x->prev; else l->last = x->prev;
	x->prev = NULL; x->ls = NULL;
}
static inline lnode *tl_live(lnode *x) { while (x && !x->ls) x = x->next; return x; }
void *tl_new(tlist *l, size_t sz);
void tl_del(lnode *x);   // unlink now, free at the next tl_flush
void tl_flush(void);     // run dtors + free everything deleted since the last flush

#define lnew(T) ((T *)tl_new(&T##_l, sizeof(T)))
#define ldel(p) tl_del(&(p)->n)
#define ldel_each(T) do { while (T##_l.first) tl_del(T##_l.first); } while (0)
#define llive(p) ((p) && (p)->n.ls ? (p) : NULL) // NULL if deleted this frame
#define lfirst(T) ((T *)T##_l.first)
#define llast(T) ((T *)T##_l.last)
#define lafter(p) ((__typeof__(p))tl_live((p)->n.next))
#define lbefore(p) ((__typeof__(p))(p)->n.prev)
#define linsert_before(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_->prev); } while (0)
#define linsert_after(p, q) do { lnode *x_ = &(p)->n, *y_ = &(q)->n; tlist *l_ = x_->ls; tl_unlink(x_); tl_ins(l_, x_, y_); } while (0)
#define each(T, v) for (T *v = lfirst(T); v; v = lafter(v))

int tl_handle(lnode *x);
lnode *tl_object(int h); // NULL for 0, unknown or deleted
#define lhandle(p) ((p) ? tl_handle(&(p)->n) : 0)
#define lobject(T, h) ((T *)tl_obj_of(&T##_l, h))
static inline lnode *tl_obj_of(tlist *l, int h) { lnode *x = tl_object(h); return x && x->ls == l ? x : NULL; }
