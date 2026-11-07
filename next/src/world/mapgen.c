// mapgen.c: the facility layout, the original's CreateMap algorithm. a grid of
// cells (1 = room), walked into hallways, counted per zone, topped up so every
// zone has the rooms the game needs, then each cell becomes a room of the shape
// its neighbours make. rng is raylib's, so seeds don't match the original's
// maps.
#include "core/util.h"
#include "raylib.h"
#include "raymath.h"
#include "world/door.h"
#include "world/world.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ZONES 3
static int W, H, *grid, junk;
static int *T(int x, int y) {
  return x < 0 || y < 0 || x > W + 1 || y > H + 1 ? (junk = 0, &junk)
                                                  : &grid[x * (H + 2) + y];
}
#define G(x, y) (*T(x, y))
static int m1(int v) { return v < 1 ? v : 1; }
static int nb(int x, int y) {
  return m1(G(x + 1, y)) + m1(G(x - 1, y)) + m1(G(x, y + 1)) + m1(G(x, y - 1));
}
static int zoneof(int y) {
  return (int)minf(floorf((float)(W - y) / W * ZONES), ZONES - 1);
}

static void hallways(void) {
  int x = W / 2, y = H - 2, temp = 0;
  for (int i = y; i <= H - 1; i++)
    G(x, i) = 1;
  do {
    int width = irnd(10, 15);
    if (x > W * .6f)
      width = -width;
    else if (x > W * .4f)
      x -= width / 2;
    if (x + width > W - 3)
      width = W - 3 - x;
    else if (x + width < 2)
      width = -x + 2;
    x = x < x + width ? x : x + width;
    width = abs(width);
    for (int i = x; i <= x + width; i++)
      G(i < W ? i : W, y) = 1;
    int height = irnd(3, 4);
    if (y - height < 1)
      height = y - 1;
    int yhall = irnd(4, 5);
    if (zoneof(y - height) != zoneof(y - height + 1))
      height--;
    for (int i = 1; i <= yhall; i++) {
      int x2 = (int)maxf(minf(irnd(x, x + width - 1), W - 2), 2), th;
      while (G(x2, y - 1) || G(x2 - 1, y - 1) || G(x2 + 1, y - 1))
        x2++;
      if (x2 >= x + width)
        continue;
      if (i == 1) {
        th = height;
        x2 = irnd(1, 2) == 1 ? x : x + width;
      } else
        th = irnd(1, height);
      for (int y2 = y - th; y2 <= y; y2++)
        G(x2, y2) = zoneof(y2) != zoneof(y2 + 1) ? 255 : 1; // 255: checkpoint
      if (th == height)
        temp = x2;
    }
    x = temp;
    y -= height;
  } while (y >= 2);
}

static int R[NSHAPES][ZONES]; // room counts per shape + zone

static int straight(int x, int y) {
  return m1(G(x + 1, y)) + m1(G(x - 1, y)) == 2 ||
         m1(G(x, y + 1)) + m1(G(x, y - 1)) == 2;
}

static void count(void) {
  memset(R, 0, sizeof R);
  for (int y = 1; y <= H - 1; y++)
    for (int x = 1; x <= W - 1; x++) {
      if (G(x, y) <= 0)
        continue;
      if (G(x, y) < 255)
        G(x, y) = nb(x, y);
      int z = zoneof(y), n = G(x, y);
      if (n == 1)
        R[ROOM1][z]++;
      else if (n == 2)
        R[straight(x, y) ? ROOM2 : ROOM2C][z]++;
      else if (n == 3)
        R[ROOM3][z]++;
      else if (n == 4)
        R[ROOM4][z]++;
    }
}

// every zone gets at least 5 dead ends: grow them off 2- and 3-way rooms
static void more1(void) {
  for (int i = 0; i < ZONES; i++) {
    int need = 5 - R[ROOM1][i];
    if (need <= 0)
      continue;
    int zh = H / ZONES; // whole rows per zone
    int y0 = zh * (2 - i) + 1, y1 = (int)roundf(zh * (3.0f - i) - 2);
    for (int y = y0; y <= y1 && need; y++)
      for (int x = 2; x <= W - 2 && need; x++) {
        if (G(x, y) || nb(x, y) != 1)
          continue;
        int x2 = x, y2 = y;
        if (G(x + 1, y))
          x2 = x + 1;
        else if (G(x - 1, y))
          x2 = x - 1;
        else if (G(x, y + 1))
          y2 = y + 1;
        else
          y2 = y - 1;
        int n = G(x2, y2), ok = 0;
        if (n == 2 && straight(x2, y2)) {
          R[ROOM2][i]--;
          R[ROOM3][i]++;
          ok = 1;
        } else if (n == 3) {
          R[ROOM3][i]--;
          R[ROOM4][i]++;
          ok = 1;
        }
        if (ok) {
          G(x2, y2)++;
          G(x, y) = 1;
          R[ROOM1][i]++;
          need--;
        }
      }
  }
}

// a 2-way corner + dead end off a dead end at (x, y), growing away from its
// neighbour (dx, dy)
static int try2c(int x, int y, int dx, int dy) {
  int px = dy != 0, py = dx != 0;
  if (G(x - px, y - py) + G(x + px, y + py) + G(x + 2 * dx, y + 2 * dy))
    return 0;
  for (int s = -1; s <= 1; s += 2) {
    int cx = x + dx + s * px, cy = y + dy + s * py;
    if (G(cx + s * px, cy + s * py) + G(cx + dx, cy + dy) + G(cx, cy) == 0) {
      G(x, y) = 2;
      G(x + dx, y + dy) = 2;
      G(cx, cy) = 1;
      return 1;
    }
  }
  return 0;
}

// every zone gets a 4-way and a corner
static void more4and2c(void) {
  for (int i = 0; i < ZONES; i++) {
    int lo = i == 2 ? 2 : i == 1 ? H / 3 + 1 : (int)roundf(H * (2.0f / 3) + 1);
    int hi = i == 2 ? H / 3 : i == 1 ? (int)roundf(H * (2.0f / 3) - 1) : H - 2;
    if (R[ROOM4][i] < 1) {
      int done = 0;
      for (int y = lo; y <= hi && !done; y++)
        for (int x = 2; x <= W - 2 && !done; x++) {
          if (G(x, y) != 3)
            continue;
          static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
          for (int k = 0; k < 4 && !done; k++) {
            int ax = x + d[k][0], ay = y + d[k][1], px = d[k][1] != 0,
                py = d[k][0] != 0;
            if ((G(ax, ay) | G(ax + px, ay + py) | G(ax - px, ay - py) |
                 G(ax + d[k][0], ay + d[k][1])) == 0) {
              G(ax, ay) = 1;
              done = 1;
            }
          }
          if (done) {
            G(x, y) = 4;
            R[ROOM4][i]++;
            R[ROOM3][i]--;
            R[ROOM1][i]++;
          }
        }
    }
    if (R[ROOM2C][i] < 1) {
      int done = 0;
      for (int y = lo + 1; y <= hi - 1 && !done; y++)
        for (int x = 3; x <= W - 3 && !done; x++) {
          if (G(x, y) != 1)
            continue;
          if (G(x - 1, y) > 0)
            done = try2c(x, y, 1, 0);
          else if (G(x + 1, y) > 0)
            done = try2c(x, y, -1, 0);
          else if (G(x, y - 1) > 0)
            done = try2c(x, y, 0, 1);
          else if (G(x, y + 1) > 0)
            done = try2c(x, y, 0, -1);
          if (done) {
            R[ROOM2C][i]++;
            R[ROOM2][i]++;
          }
        }
    }
  }
}

// the planned rooms: per shape, a list of names that cells of that shape take
// in order
static const char **plan[NSHAPES];
static int planmax, planid[NSHAPES];
static int put(int shape, const char *name, int pos, int lo, int hi) {
  if (hi < lo)
    return 0;
  int looped = 0;
  while (pos <= planmax && plan[shape][pos]) {
    if (++pos > hi) {
      if (looped)
        return 0;
      pos = lo + 1;
      looped = 1;
    }
  }
  if (pos < 0 || pos > planmax)
    return 0;
  plan[shape][pos] = name;
  return 1;
}
static void set(int shape, int pos, const char *name) {
  if (pos >= 0 && pos <= planmax)
    plan[shape][pos] = name;
}
#define F(k, a) ((int)floorf((k) * (float)(a)))

static void assign(void) {
  planmax = 55 * W / 20;
  for (int s = 0; s < NSHAPES; s++)
    if (R[s][0] + R[s][1] + R[s][2] + 1 > planmax)
      planmax = R[s][0] + R[s][1] + R[s][2] + 1;
  for (int s = 0; s < NSHAPES; s++) {
    free(plan[s]);
    plan[s] = calloc(planmax + 1, sizeof *plan[s]);
    planid[s] = 0;
  }
  int r1 = R[ROOM1][0], r2 = R[ROOM2][0];
  // light containment
  set(ROOM1, 0, "start");
  put(ROOM1, "roompj", F(.1f, r1), 1, r1 - 1);
  put(ROOM1, "914", F(.3f, r1), 1, r1 - 1);
  put(ROOM1, "room1archive", F(.5f, r1), 1, r1 - 1);
  put(ROOM1, "room205", F(.6f, r1), 1, r1 - 1);
  set(ROOM2C, 0, "lockroom");
  set(ROOM2, 0, "room2closets");
  static const struct {
    const char *n;
    float k;
  } z1[] = {
      {"room2testroom2", .1f}, {"room2scps", .2f}, {"room2storage", .3f},
      {"room2gw_b", .4f},      {"room2sl", .5f},   {"room012", .55f},
      {"room2scps2", .6f},     {"room1123", .7f},  {"room2elevator", .85f}};
  for (int i = 0; i < 9; i++)
    put(ROOM2, z1[i].n, F(z1[i].k, r2), 1, r2 - 1);
  set(ROOM3, (int)floorf(frnd(.2f, .8f) * R[ROOM3][0]), "room3storage");
  set(ROOM2C, F(.5f, R[ROOM2C][0]), "room1162");
  set(ROOM4, F(.3f, R[ROOM4][0]), "room4info");
  // heavy containment
  static const struct {
    const char *n;
    float k;
  } z2a[] = {{"room079", .15f},
             {"room106", .3f},
             {"008", .4f},
             {"room035", .5f},
             {"coffin", .7f}};
  for (int i = 0; i < 5; i++)
    put(ROOM1, z2a[i].n, r1 + F(z2a[i].k, R[ROOM1][1]), r1,
        r1 + R[ROOM1][1] - 1);
  set(ROOM2, r2 + F(.1f, R[ROOM2][1]), "room2nuke");
  static const struct {
    const char *n;
    float k;
  } z2b[] = {{"room2tunnel", .25f},
             {"room049", .4f},
             {"room2shaft", .6f},
             {"testroom", .7f},
             {"room2servers", .9f}};
  for (int i = 0; i < 5; i++)
    put(ROOM2, z2b[i].n, r2 + F(z2b[i].k, R[ROOM2][1]), r2,
        r2 + R[ROOM2][1] - 1);
  set(ROOM3, R[ROOM3][0] + F(.3f, R[ROOM3][1]), "room513");
  set(ROOM3, R[ROOM3][0] + F(.6f, R[ROOM3][1]), "room966");
  set(ROOM2C, R[ROOM2C][0] + F(.5f, R[ROOM2C][1]), "room2cpit");
  // entrance zone
  int r1all = r1 + R[ROOM1][1] + R[ROOM1][2];
  set(ROOM1, r1all - 2, "exit1");
  set(ROOM1, r1all - 1, "gateaentrance");
  set(ROOM1, r1 + R[ROOM1][1], "room1lifts");
  int lo = r2 + R[ROOM2][1], hi = lo + R[ROOM2][2] - 1;
  set(ROOM2, lo + F(.1f, R[ROOM2][2]), "room2poffices");
  static const struct {
    const char *n;
    float k;
  } z3[] = {
      {"room2cafeteria", .2f}, {"room2sroom", .3f},     {"room2servers2", .4f},
      {"room2offices", .45f},  {"room2offices4", .5f},  {"room860", .6f},
      {"medibay", .7f},        {"room2poffices2", .8f}, {"room2offices2", .9f}};
  for (int i = 0; i < 9; i++)
    put(ROOM2, z3[i].n, lo + F(z3[i].k, R[ROOM2][2]), lo, hi);
  int c = R[ROOM2C][0] + R[ROOM2C][1], t3 = R[ROOM3][0] + R[ROOM3][1];
  set(ROOM2C, c, "room2ccont");
  set(ROOM2C, c + 1, "lockroom2");
  set(ROOM3, t3 + F(.3f, R[ROOM3][2]), "room3servers");
  set(ROOM3, t3 + F(.7f, R[ROOM3][2]), "room3servers2");
  set(ROOM3, t3 + F(.5f, R[ROOM3][2]), "room3offices");
}

// ---- rooms
static int loadtmpl(RoomTmpl *t) {
  if (t->loaded)
    return t->loaded > 0;
  t->loaded = -1;
  if (!t->mesh[0] || !room_load(&t->rm, t->mesh)) {
    if (t->mesh[0])
      TraceLog(LOG_WARNING, "world: room %s has no baked mesh", t->mesh);
    return 0;
  }
  t->loaded = 1;
  t->ext = GetModelBoundingBox(t->rm.vis);
  return 1;
}

static void extents(Room *r) { // the template's box, turned + moved, shrunk a
                               // little (wall-thin overlaps don't count)
  BoundingBox b = r->t->ext, o = {{1e9f, 1e9f, 1e9f}, {-1e9f, -1e9f, -1e9f}};
  Matrix rot = MatrixRotateY(r->angle * DEG2RAD);
  for (int i = 0; i < 8; i++) {
    Vector3 p = Vector3Transform((Vector3){i & 1 ? b.max.x : b.min.x,
                                           i & 2 ? b.max.y : b.min.y,
                                           i & 4 ? b.max.z : b.min.z},
                                 rot);
    o.min = Vector3Min(o.min, p);
    o.max = Vector3Max(o.max, p);
  }
  Vector3 sh = {.05f, .05f, .05f}, off = {r->pos.x, 0, r->pos.z};
  r->ext = (BoundingBox){Vector3Add(Vector3Add(o.min, sh), off),
                         Vector3Subtract(Vector3Add(o.max, off), sh)};
}

static void place(Room *r, int gx, int gy, int angle) {
  r->gx = gx;
  r->gy = gy;
  r->angle = (int)wrapdeg(angle);
  r->pos = (Vector3){gx * 8.0f, r->pos.y, -gy * 8.0f};
  r->m = room_matrix(r->pos, r->angle);
  if (r->t && r->t->loaded > 0)
    extents(r);
}

static Room *mkroom(World *w, int zone, int shape, int gx, int gy, float y,
                    const char *name) {
  RoomTmpl *t = name ? world_tmpl(w, name) : NULL;
  if (!t) { // weighted pick among the templates for this zone + shape
    int sum = 0;
    for (int i = 0; i < w->ntmpl; i++)
      for (int z = 0; z < 5; z++)
        if (w->tmpl[i].zone[z] == zone && w->tmpl[i].shape == shape) {
          sum += w->tmpl[i].common;
          break;
        }
    int pick = irnd(1, sum > 0 ? sum : 1), acc = 0;
    for (int i = 0; i < w->ntmpl && !t; i++)
      for (int z = 0; z < 5; z++)
        if (w->tmpl[i].zone[z] == zone && w->tmpl[i].shape == shape) {
          acc += w->tmpl[i].common;
          if (pick > acc - w->tmpl[i].common && pick <= acc)
            t = &w->tmpl[i];
          break;
        }
  }
  w->rooms = realloc(w->rooms, sizeof *w->rooms * (w->nrooms + 1));
  Room *r = &w->rooms[w->nrooms++];
  *r = (Room){.t = t, .zone = zone, .pos = {0, y, 0}};
  for (int i = 0; i < ROOM_DOORS; i++)
    r->door[i] = -1;
  if (t)
    loadtmpl(t);
  place(r, gx, gy, 0);
  return r;
}

static const char *next(int shape) {
  return planid[shape] <= planmax ? plan[shape][planid[shape]] : NULL;
}

static void rooms(World *w) {
  for (int y = H - 1; y >= 1; y--) {
    int zone = y < H / 3 + 1 ? 3 : y < H * (2.0f / 3) ? 2 : 1;
    for (int x = 1; x <= W - 2; x++) {
      if (G(x, y) == 255) {
        mkroom(w, zone, ROOM2, x, y, 0,
               y > H / 2 ? "checkpoint1" : "checkpoint2");
        continue;
      }
      if (G(x, y) <= 0)
        continue;
      Room *r;
      int n = nb(x, y), a = 0, shape;
      if (n == 1) {
        shape = ROOM1;
        a = G(x, y + 1) ? 180 : G(x - 1, y) ? 270 : G(x + 1, y) ? 90 : 0;
      } else if (n == 2) {
        if (G(x - 1, y) > 0 && G(x + 1, y) > 0) {
          shape = ROOM2;
          a = irnd(1, 2) == 1 ? 90 : 270;
        } else if (G(x, y - 1) > 0 && G(x, y + 1) > 0) {
          shape = ROOM2;
          a = irnd(1, 2) == 1 ? 180 : 0;
        } else {
          shape = ROOM2C;
          a = G(x - 1, y) > 0 && G(x, y + 1) > 0   ? 180
              : G(x + 1, y) > 0 && G(x, y + 1) > 0 ? 90
              : G(x - 1, y) > 0 && G(x, y - 1) > 0 ? 270
                                                   : 0;
        }
      } else if (n == 3) {
        shape = ROOM3;
        a = !G(x, y - 1) ? 180 : !G(x - 1, y) ? 90 : !G(x + 1, y) ? 270 : 0;
      } else
        shape = ROOM4;
      r = mkroom(w, zone, shape, x, y, 0, next(shape));
      place(r, x, y, a);
      planid[shape]++;
    }
  }
  mkroom(w, 0, ROOM1, W - 1, 0, 500, "gatea");
  mkroom(w, 0, ROOM1, W - 1, H - 1, 0, "pocketdimension");
  mkroom(w, 0, ROOM1, 1, 0, 800, "dimension1499");
}

static int skip(Room *r) {
  return !r->t || r->t->nooverlap || r->t->loaded <= 0;
}
static int fixed(Room *r) {
  return !strcmp(r->t->name, "checkpoint1") ||
         !strcmp(r->t->name, "checkpoint2") || !strcmp(r->t->name, "start");
}
static int overlap(BoundingBox a, BoundingBox b) {
  return !(a.max.x <= b.min.x || a.max.y <= b.min.y || a.max.z <= b.min.z ||
           a.min.x >= b.max.x || a.min.y >= b.max.y || a.min.z >= b.max.z);
}
static int overlaps(World *w, Room *r) {
  for (int i = 0; i < w->nrooms; i++)
    if (&w->rooms[i] != r && !skip(&w->rooms[i]) &&
        overlap(r->ext, w->rooms[i].ext))
      return 1;
  return 0;
}

// a room that pokes into a neighbour: turn it around (2-ways), else swap it w/
// a same-shape room in its zone
static void unoverlap(World *w, Room *r) {
  if (skip(r) || fixed(r) || !overlaps(w, r))
    return;
  if (r->t->shape == ROOM2) {
    place(r, r->gx, r->gy, r->angle + 180);
    if (!overlaps(w, r))
      return;
    place(r, r->gx, r->gy, r->angle - 180);
  }
  for (int i = 0; i < w->nrooms; i++) {
    Room *o = &w->rooms[i];
    if (o == r || skip(o) || fixed(o) || o->t->shape != r->t->shape ||
        o->zone != r->zone)
      continue;
    int x = r->gx, y = r->gy, a = r->angle;
    place(r, o->gx, o->gy, o->angle);
    place(o, x, y, a);
    if (!overlaps(w, r) && !overlaps(w, o))
      return;
    place(o, r->gx, r->gy, r->angle);
    place(r, x, y, a);
  }
}

// ---- doors between neighbouring cells: on a room's +x side (angle 90) and its
// -y side (angle 0)
static int doorx(Room *r) {
  int a = r->angle;
  switch (r->t ? r->t->shape : 0) {
  case ROOM1:
    return a == 90;
  case ROOM2:
    return a == 90 || a == 270;
  case ROOM2C:
    return a == 0 || a == 90;
  case ROOM3:
    return a == 0 || a == 180 || a == 90;
  default:
    return 1;
  }
}
static int doorz(Room *r) {
  int a = r->angle;
  switch (r->t ? r->t->shape : 0) {
  case ROOM1:
    return a == 180;
  case ROOM2:
    return a == 0 || a == 180;
  case ROOM2C:
    return a == 180 || a == 90;
  case ROOM3:
    return a == 180 || a == 90 || a == 270;
  default:
    return 1;
  }
}
static void door(World *w, Vector3 p, int angle, int kind, int room) {
  door_add(w, p, angle, kind, room, (int)maxf(irnd(-3, 1), 0));
}
static void doors(World *w) {
  for (int y = H; y >= 0; y--) {
    int zone = y < 6 ? 3 : y < 12 ? 2 : 1;
    for (int x = W; x >= 0; x--) {
      if (G(x, y) <= 0)
        continue;
      for (int i = 0; i < w->nrooms; i++) {
        Room *r = &w->rooms[i];
        if (r->gx != x || r->gy != y || r->pos.y != 0)
          continue;
        int kind = zone == 2 ? DOOR_HEAVY : DOOR_NORMAL;
        if (doorx(r) && x + 1 < W + 1 && G(x + 1, y) > 0)
          door(w, (Vector3){x * 8 + 4.0f, 0, -y * 8.0f}, 90, kind, i);
        if (doorz(r) && y + 1 < H + 1 && G(x, y + 1) > 0)
          door(w, (Vector3){x * 8.0f, 0, -(y * 8 + 4.0f)}, 0, kind, i);
        break;
      }
    }
  }
}

void world_gen(World *w, unsigned seed) {
  SetRandomSeed(seed);
  W = w->w;
  H = w->h;
  grid = calloc((W + 2) * (H + 2), sizeof *grid);
  hallways();
  count();
  more1();
  more4and2c();
  assign();
  rooms(w);
  for (int i = 0; i < w->nrooms; i++)
    unoverlap(w, &w->rooms[i]);
  for (int y = 0; y <= H; y++)
    for (int x = 0; x <= W; x++)
      G(x, y) = m1(G(x, y));
  doors(w);
  w->accesscode = 0;
  for (int i = 0, k = 1; i < 4; i++, k *= 10)
    w->accesscode += irnd(1, 9) * k;
  world_fill(w);
  for (int i = 0; i < w->nrooms; i++)
    if (w->rooms[i].t && !strcmp(w->rooms[i].t->name, "start")) {
      Vector3 p = w->rooms[i].pos;
      w->spawn =
          (Vector3){p.x + 3584 * RSCALE, 704 * RSCALE, p.z - 1024 * RSCALE};
      w->spawnyaw = irnd(160, 200);
    }
  free(grid);
  grid = NULL;
  for (int s = 0; s < NSHAPES; s++) {
    free(plan[s]);
    plan[s] = NULL;
  }
}
