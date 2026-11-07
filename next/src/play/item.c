// item.c: Items.bb's templates, UpdateItems, PickItem, DropItem, RemoveItem.
// an item is a point (the original's 0.01 radius collider) w/ its model on
// top; it falls until something's under it
#define _POSIX_C_SOURCE 200809L
#include "play/item.h"
#include "audio/snd.h"
#include "core/asset.h"
#include "core/util.h"
#include "gfx/props.h"
#include "raymath.h"
#include "ui/hud.h"
#include "world/coll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static ItemTmpl *tmpls;
static int ntmpl;

static char *trim(char *s) {
  while (*s == ' ')
    s++;
  for (char *e = s + strlen(s); e > s && (e[-1] == ' ' || e[-1] == '\r');)
    *--e = 0;
  return s;
}

// "GFX\items\INVkey1.jpg" -> "gfx/items/invkey1" + suffix, the baker's id
static void id(char *o, size_t n, const char *path, const char *suffix) {
  if (!*path) {
    *o = 0;
    return;
  }
  size_t k = 0;
  for (const char *c = path; *c && k + 1 < n; c++)
    o[k++] = *c == '\\'               ? '/'
             : *c == ' '              ? '_'
             : *c >= 'A' && *c <= 'Z' ? *c + 32
                                      : *c;
  o[k] = 0;
  char *dot = strrchr(o, '.'), *sl = strrchr(o, '/');
  if (dot && (!sl || dot > sl))
    *dot = 0;
  strncat(o, suffix, n - strlen(o) - 1);
}

int items_init(void) {
  if (tmpls)
    return 1;
  char *t = LoadFileText("next/items.txt");
  if (!t)
    return 0;
  for (char *l = strtok(t, "\n"); l; l = strtok(NULL, "\n")) {
    if (*l == '#')
      continue;
    char *f[12] = {0};
    int n = 0;
    for (char *c = l; n < 12; n++) {
      char *e = strchr(c, '|');
      if (e)
        *e = 0;
      f[n] = trim(c);
      if (!e)
        break;
      c = e + 1;
    }
    if (n < 10)
      continue;
    tmpls = realloc(tmpls, sizeof *tmpls * (ntmpl + 1));
    ItemTmpl *it = &tmpls[ntmpl++];
    *it = (ItemTmpl){.scale = (float)atof(f[5]),
                     .anim = atoi(f[8]),
                     .sound = atoi(f[10]),
                     .col = WHITE};
    snprintf(it->name, sizeof it->name, "%s", f[0]);
    snprintf(it->tmpl, sizeof it->tmpl, "%s", f[1]);
    id(it->model, sizeof it->model, f[2], "");
    id(it->inv, sizeof it->inv, f[3], ".pu");
    id(it->img, sizeof it->img, f[4], ".pu");
    id(it->tex, sizeof it->tex, f[6], atoi(f[9]) & 2 ? ".a" : "");
    id(it->inv2, sizeof it->inv2, f[7], ".pu");
    int r, g, b;
    if (n > 11 && sscanf(f[11], "%d %d %d", &r, &g, &b) == 3)
      it->col = (Color){r, g, b, 255};
  }
  UnloadFileText(t);
  return 1;
}

const ItemTmpl *item_tmpl(const Item *it) {
  if (!it->t && tmpls) // CreateItem: by name + tempname, ignoring case
    for (int i = 0; i < ntmpl; i++)
      if (!strcasecmp(tmpls[i].name, it->name) &&
          !strcasecmp(tmpls[i].tmpl, it->tmpl)) {
        ((Item *)it)->t = i + 1;
        break;
      }
  return it->t ? &tmpls[it->t - 1] : NULL;
}
const char *item_kind(const Item *it) {
  const ItemTmpl *t = item_tmpl(it);
  return t ? t->tmpl : "";
}
int item_is(const Item *it, const char *kind) {
  return !strcmp(item_kind(it), kind);
}

void inv_reset(Inv *v) {
  for (int i = 0; i < INV_SLOTS; i++)
    v->slot[i] = -1;
  v->sel = -1;
  v->open = 0;
}

int inv_has(const Inv *v, int item) {
  for (int i = 0; i < INV_SLOTS; i++)
    if (v->slot[i] == item)
      return i;
  return -1;
}

static int lying(const Item *it) {
  return !it->picked && !it->hidden && !it->gone && item_tmpl(it);
}

int item_closest(World *w, Vector3 eye, Vector3 fwd) {
  int best = -1;
  float bd = 1.2f;
  for (int i = 0; i < w->nitems; i++) {
    Item *it = &w->items[i];
    if (!lying(it))
      continue;
    Vector3 d = Vector3Subtract(it->pos, eye);
    float l = Vector3Length(d);
    if (l >= bd || Vector3DotProduct(fwd, Vector3Scale(d, 1 / maxf(l, 1e-4f))) <
                       .5f) // EntityInView, roughly
      continue;
    RayCollision h = coll_ray(eye, it->pos); // EntityVisible
    if (h.hit && h.distance < l - .02f)
      continue;
    best = i, bd = l;
  }
  return best;
}

void item_tick(World *w, Vector3 eye) {
  const float hd = 8; // iround(HideDistance * .5)
  for (int i = 0; i < w->nitems; i++) {
    Item *it = &w->items[i];
    if (!lying(it) || Vector3Distance(eye, it->pos) >= hd)
      continue;
    // fall: stop where the 0.01 collider touches something, or if there's
    // nothing below at all
    Vector3 n =
        Vector3Add(it->pos, (Vector3){it->v.x, it->v.y - .0004f, it->v.z});
    Vector3 to = Vector3Add(n, (Vector3){0, -.01f, 0});
    RayCollision h = coll_ray(it->pos, to);
    if (h.hit) {
      it->pos = Vector3Add(h.point, (Vector3){0, .01f, 0});
      it->v = Vector3Zero();
    } else if (!coll_ray(it->pos, Vector3Add(it->pos, (Vector3){0, -10, 0}))
                    .hit)
      it->v = Vector3Zero();
    else {
      it->v.y -= .0004f;
      it->pos = n;
    }
    if (it->pos.y < -35)
      it->gone = 1;
    if (Vector3Distance(eye, it->pos) >= hd * .2f)
      continue;
    for (int k = 0; k < w->nitems;
         k++) { // items on top of each other get pushed apart
      Item *o = &w->items[k];
      if (k == i || !lying(o) || Vector3Distance(eye, o->pos) >= hd * .2f)
        continue;
      Room *r =
          it->room >= 0 && it->room < w->nrooms ? &w->rooms[it->room] : NULL;
      float x = o->pos.x - it->pos.x, z = o->pos.z - it->pos.z,
            e = x * x + z * z;
      if (e >= .07f || fabsf(o->pos.y - it->pos.y) >= .25f ||
          (r && r->t && !strcmp(r->t->name, "room2storage")))
        continue;
      x *= .07f - e, z *= .07f - e;
      while (fabsf(x) + fabsf(z) < .001f)
        x += frnd(-.002f, .002f), z += frnd(-.002f, .002f);
      o->pos.x += x, o->pos.z += z;
      it->pos.x -= x, it->pos.z -= z;
    }
  }
}

static void picksound(const Item *it) {
  const ItemTmpl *t = item_tmpl(it);
  char s[48];
  snprintf(s, sizeof s, "sfx/interact/pickitem%d", t ? t->sound : 1);
  snd_play(s, 1);
}

int item_pick(World *w, Inv *v, int i) {
  Item *it = &w->items[i];
  int n = inv_has(v, -1);
  if (n < 0) {
    hud_msg("You cannot carry any more items.", 70 * 5);
    return 0;
  }
  // 1123 + the strange battery kill you, w/ events + death. hazmat/vest
  // clashes come w/ wearing them
  if (item_is(it, "veryfinevest")) {
    hud_msg("The vest is too heavy to pick up.", 70 * 6);
    return 0;
  }
  if (item_is(it, "firstaid") || item_is(it, "finefirstaid") ||
      item_is(it, "veryfinefirstaid") || item_is(it, "firstaid2"))
    it->state = 0;
  picksound(it);
  it->picked = 1;
  v->slot[n] = i;
  return 1;
}

void item_drop(World *w, Inv *v, int i, Vector3 eye, float yaw, float pitch) {
  Item *it = &w->items[i];
  picksound(it);
  // a bit in front of + below the eye, along the view
  Vector3 f = {-sinf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD),
               -sinf(pitch * DEG2RAD),
               -cosf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD)};
  it->pos =
      Vector3Add(eye, Vector3Add(Vector3Scale(f, .1f), (Vector3){0, -.1f, 0}));
  it->yaw = wrapdeg(yaw + frnd(-110, 110));
  it->v = Vector3Zero();
  it->picked = 0;
  int s = inv_has(v, i);
  if (s >= 0)
    v->slot[s] = -1;
  if (v->sel == i)
    v->sel = -1;
}

void item_remove(World *w, Inv *v, int i) {
  w->items[i].gone = 1;
  int s = inv_has(v, i);
  if (s >= 0)
    v->slot[s] = -1;
  if (v->sel == i)
    v->sel = -1;
}

int item_new(World *w, const char *name, const char *kind, Vector3 pos) {
  w->items = realloc(w->items, sizeof *w->items * (w->nitems + 1));
  Item *it = &w->items[w->nitems];
  *it = (Item){.pos = pos, .yaw = (float)irnd(0, 359), .room = -1};
  snprintf(it->name, sizeof it->name, "%s", name);
  snprintf(it->tmpl, sizeof it->tmpl, "%s", kind);
  return w->nitems++;
}

void item_draw(World *w, Vector3 eye, float r, int alpha) {
  for (int i = 0; i < w->nitems; i++) {
    Item *it = &w->items[i];
    const ItemTmpl *t = item_tmpl(it);
    Mdl *m = t && lying(it) && Vector3Distance(eye, it->pos) < r
                 ? prop_get(t->model)
                 : NULL;
    if (!m)
      continue;
    // some templates repaint a shared model (keycards) or tint it (pills)
    Texture2D tex = t->tex[0] ? tex_get(t->tex) : (Texture2D){0};
    Texture2D was[16];
    Color wc[16];
    int nm = m->m.materialCount < 16 ? m->m.materialCount : 16;
    for (int k = 0; k < nm; k++) {
      MaterialMap *mm = &m->m.materials[k].maps[MATERIAL_MAP_ALBEDO];
      was[k] = mm->texture, wc[k] = mm->color;
      if (tex.id)
        mm->texture = tex;
      mm->color =
          (Color){mm->color.r * t->col.r / 255, mm->color.g * t->col.g / 255,
                  mm->color.b * t->col.b / 255, mm->color.a};
    }
    if (t->anim)
      mdl_pose(m, 0);
    Matrix mx =
        MatrixMultiply(MatrixMultiply(MatrixScale(t->scale, t->scale, t->scale),
                                      MatrixRotateY(it->yaw * DEG2RAD)),
                       MatrixTranslate(it->pos.x, it->pos.y, it->pos.z));
    mdl_draw(m, mx, alpha);
    for (int k = 0; k < nm; k++)
      m->m.materials[k].maps[MATERIAL_MAP_ALBEDO].texture = was[k],
      m->m.materials[k].maps[MATERIAL_MAP_ALBEDO].color = wc[k];
  }
}
