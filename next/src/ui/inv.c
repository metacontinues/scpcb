// inv.c: Main.bb's DrawGUI inventory + item in hand. like the original the
// slots are sized in pixels, the images it holds up in MenuScale
#include "ui/inv.h"
#include "audio/snd.h"
#include "core/asset.h"
#include "core/util.h"
#include "ui/hud.h"
#include "ui/ui.h"
#include <stdio.h>
#include <string.h>

enum { SW = 70, SH = 70, SP = 35, NONE = 66 };
static double lastclick;
static int dclick;

static int is(const Item *it, const char *k) { return item_is(it, k); }

static void center(Texture2D t, float k) {
  DrawTextureEx(t,
                (Vector2){GetScreenWidth() / 2.0f - t.width * k / 2,
                          GetScreenHeight() / 2.0f - t.height * k / 2},
                0, k, WHITE);
}

static void picksound(const Item *it) {
  char s[48];
  snprintf(s, sizeof s, "sfx/interact/pickitem%d",
           item_tmpl(it) ? item_tmpl(it)->sound : 1);
  snd_play(s, 1);
}

static void nostalgia(int lo, int hi) {
  char s[48];
  snprintf(s, sizeof s, "sfx/scp/1162/nostalgiacancer%d", irnd(lo, hi));
  snd_play(s, 1);
}

// let go of the dragged item over slot (NONE: outside them, drop it)
static void release(World *w, Inv *v, int slot, Vector3 eye, float yaw,
                    float pitch) {
  Item *it = &w->items[v->sel];
  if (slot == NONE) {
    item_drop(w, v, v->sel, eye, yaw, pitch);
    v->open = 0;
  } else if (v->slot[slot] < 0) { // into an empty slot
    v->slot[inv_has(v, v->sel)] = -1;
    v->slot[slot] = v->sel;
  } else if (v->slot[slot] != v->sel) // onto another item: combining comes next
    hud_msg("You cannot combine these two items.", 70 * 5);
  (void)it;
  v->sel = -1;
}

void inv_screen(World *w, Inv *v, Vector3 eye, float yaw, float pitch) {
  ui_begin();
  Vector2 m = GetMousePosition();
  int hit = IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
      down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  if (hit) { // two clicks within 800 ms
    dclick = GetTime() - lastclick < .8;
    lastclick = GetTime();
  }
  int x0 = GetScreenWidth() / 2 -
           (SW * INV_SLOTS / 2 + SP * (INV_SLOTS / 2 - 1)) / 2,
      x = x0, y = GetScreenHeight() / 2 - SH, over = NONE;
  for (int n = 0; n < INV_SLOTS; n++) {
    int on = m.x > x && m.x < x + SW && m.y > y && m.y < y + SH, i = v->slot[n];
    Item *it = i >= 0 ? &w->items[i] : NULL;
    const ItemTmpl *t = it ? item_tmpl(it) : NULL;
    if (on) {
      over = n;
      DrawRectangle(x - 1, y - 1, SW + 2, SH + 2, RED);
    }
    ui_frame(x, y, SW, SH);
    if (t && (v->sel != i || on))
      DrawTexture(tex_ui(t->inv), x + SW / 2 - 32, y + SH / 2 - 32, WHITE);
    if (it && v->sel != i && on && v->sel < 0) {
      if (hit) {
        v->sel = i;
        if (dclick) { // use it
          picksound(it);
          v->open = 0;
          dclick = 0;
          return;
        }
      }
      ui_text(it->name, x + SW / 2 + 1, y + SH + SP - 15 + 1, 1, 0, 0, BLACK);
      ui_text(it->name, x + SW / 2, y + SH + SP - 15, 1, 0, 0, WHITE);
    }
    x += SW + SP;
    if (n == 4)
      y += SH * 2, x = x0;
  }
  if (v->sel >= 0) {
    const ItemTmpl *t = item_tmpl(&w->items[v->sel]);
    if (down) { // dragging it
      if (t && (over == NONE || v->slot[over] != v->sel)) {
        Texture2D tx = tex_ui(t->inv);
        DrawTexture(tx, (int)m.x - tx.width / 2, (int)m.y - tx.height / 2,
                    WHITE);
      }
    } else
      release(w, v, over, eye, yaw, pitch);
  }
  ui_cursor();
}

// paper & tickets: the document, the burnt note w/ the access code written on
static void document(World *w, Item *it, const ItemTmpl *t) {
  float k = U(1000) / 1000.0f;
  int raw =
      !strcmp(t->name, "Burnt Note") ||
      !strcmp(t->name, "Movie Ticket"); // the original didn't resize these
  Texture2D tx = tex_ui(t->img);
  center(tx, raw ? 1 : k);
  int x = GetScreenWidth() / 2 - (int)(tx.width * (raw ? 1 : k)) / 2,
      y = GetScreenHeight() / 2 - (int)(tx.height * (raw ? 1 : k)) / 2;
  char c[16];
  if (!strcmp(t->name, "Burnt Note")) {
    snprintf(c, sizeof c, "%d", w->accesscode);
    ui_text(c, x + 277, y + 469, 1, 1, 0, BLACK);
  } else if (!strcmp(t->name,
                     "Document SCP-372")) { // the code 3x scrawled on it
    snprintf(c, sizeof c, "%d",
             w->accesscode * 3 % 10000 < 1000 ? w->accesscode * 3 % 10000 + 1000
                                              : w->accesscode * 3 % 10000);
    Font f = font_get("gfx/font/journal/journal", U(58));
    Vector2 sz = MeasureTextEx(f, c, f.baseSize, 0);
    DrawTextEx(f, c, (Vector2){x + 383 * k - sz.x / 2, y + 734 * k - sz.y / 2},
               f.baseSize, 0, (Color){37, 45, 137, 255});
  } else if (!strcmp(t->name, "Movie Ticket") && it->state == 0) {
    hud_msg("\"Hey, I remember this movie!\"", 70 * 10);
    nostalgia(1, 5);
    it->state = 1;
  }
}

void inv_hand(World *w, Inv *v) {
  if (v->sel < 0)
    return;
  Item *it = &w->items[v->sel];
  const ItemTmpl *t = item_tmpl(it);
  if (!t) {
    v->sel = -1;
    return;
  }
  float k = U(1000) / 1000.0f;
  if ((is(it, "paper") || is(it, "ticket")) && t->img[0])
    document(w, it, t);
  else if (is(it, "badge") || is(it, "oldpaper")) {
    center(tex_ui(t->img), is(it, "oldpaper") ? k : 1);
    if (it->state == 0) {
      nostalgia(6, 10);
      if (!strcmp(t->name, "Old Badge"))
        hud_msg("\"Huh? This guy looks just like me!\"", 70 * 10);
      else if (is(it, "oldpaper"))
        hud_msg("\"Why does this seem so familiar?\"", 70 * 10);
      it->state = 1;
    }
  } else if (is(it, "key")) {
    if (it->state == 0) {
      nostalgia(6, 10);
      hud_msg("\"Isn't this the key to that old shack? The one where I... No, "
              "it can't be.\"",
              70 * 10);
    }
    it->state = 1;
    v->sel = -1;
    return;
  } else if (is(it, "coin")) {
    if (it->state == 0)
      nostalgia(1, 5);
    it->state = 1;
    v->sel = -1;
    return;
  } else if (!strncmp(t->tmpl, "key", 3) || is(it, "scp860") ||
             is(it, "hand") || is(it, "hand2") || is(it, "25ct"))
    center(tex_ui(t->inv), 1); // held up, for a door's slot
  // the rest get used in the next pass (kits, masks, radio...)
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { // put it away
    picksound(it);
    v->sel = -1;
  }
}
