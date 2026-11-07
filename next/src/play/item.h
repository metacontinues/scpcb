// item.h: the original's items (Items.bb). templates come from next/items.txt;
// the items themselves live in World.items (fill puts them down), the ones
// you carry sit in Inv's slots. ticks are the original's 1/70 s
#pragma once
#include "raylib.h"
#include "world/world.h"

enum { INV_SLOTS = 10 }; // MaxItemAmount

typedef struct {
  char name[64], tmpl[24];
  char model[96], inv[96], img[96], tex[96], inv2[96]; // asset ids, "" = none
  float scale;
  int anim, sound; // sound: SFX/Interact/PickItem<n>
  Color col;
} ItemTmpl;

typedef struct {
  int slot[INV_SLOTS]; // World.items indices, -1 = empty
  int sel;             // the item in your hand (SelectedItem), -1 = none
  int open;            // the inventory screen
} Inv;

int items_init(void); // reads next/items.txt, 0 if it's missing
const ItemTmpl *item_tmpl(const Item *it); // resolves + caches it->t
const char *item_kind(const Item *it); // its template's tempname, "" if none
int item_is(const Item *it, const char *kind);

void inv_reset(Inv *v);
int inv_has(const Inv *v, int item); // its slot, -1 if not carried
// the closest item you could pick up (in reach, in view, nothing in between),
// -1 if none
int item_closest(World *w, Vector3 eye, Vector3 fwd);
void item_tick(World *w, Vector3 eye);     // falling, pushing apart
int item_pick(World *w, Inv *v, int item); // PickItem. 1 if it's yours now
void item_drop(World *w, Inv *v, int item, Vector3 eye, float yaw, float pitch);
void item_remove(World *w, Inv *v, int item); // gone for good (used up)
// one item spawned at runtime (914 & co later), its index
int item_new(World *w, const char *name, const char *kind, Vector3 pos);
void item_draw(World *w, Vector3 eye, float r, int alpha);
