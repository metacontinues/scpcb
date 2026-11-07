// bake.h: source assets -> what raylib loads (glb, qoi). run from the repo root, writes next/data/<id>.*
// an id is the source path lowercased, '/' separated, no extension, spaces -> '_'.
#pragma once
#include <stddef.h>
#include <stdint.h>

#define OUT "next/data/"
#define ROOMSCALE (8.0f / 2048.0f)
#define D2R 0.017453292519943295f

// ---- util (bake.c)
typedef struct { char *s; size_t n, c; } sb; // growable string
void sbf(sb *b, const char *fmt, ...);
void sbraw(sb *b, const void *d, size_t n);
const char *ci(const char *path);           // case-insensitive resolve ('\\' ok), static buf, NULL if missing
void mkid(const char *path, char *o, int n); // path -> id
void mkdirs(const char *path);              // parents of path
int stale(const char *src, const char *out); // out missing or older than src
int hasci(const char *s, const char *sub);  // case-insensitive substring
extern int nwarn;
void warn(const char *fmt, ...);

// ---- glb writer (glb.c). accessors/views go into one bin chunk; nodes/skins/anims are json fragments
enum { GB_F32 = 5126, GB_U16 = 5123, GB_U8 = 5121 };
typedef struct { int pos, nrm, uv0, uv1, col, jnt, wgt, idx, mat; } gbprim; // accessor ids, -1 = none
typedef struct { sb bin, views, accs, meshes, mats, nodes, roots, skins, anims; int nview, nacc, nmesh, nmat, nnode; } glb;
int gb_view(glb *g, const void *d, size_t n, int target);
int gb_acc(glb *g, int view, int ctype, int count, const char *type, int norm, int ncomp, const float *mn, const float *mx);
int gb_mat(glb *g, const char *name);
int gb_mesh(glb *g, const gbprim *p, int np);
int gb_node(glb *g, int root, const char *json); // the inside of the node's {}, eg "\"mesh\":0"
void gb_skin(glb *g, const char *json);
void gb_anim(glb *g, const char *json);
int gb_write(glb *g, const char *path);         // frees g
gbprim gb_prim0(void);                           // all -1
// attribute helpers
int gb_vec3(glb *g, const float *v, int n);     // w/ min/max (POSITION wants them)
int gb_vec2(glb *g, const float *v, int n);
int gb_vec4(glb *g, const float *v, int n);
int gb_rgba8(glb *g, const uint8_t *v, int n);
int gb_u8x4(glb *g, const uint8_t *v, int n);   // JOINTS_0
int gb_idx16(glb *g, const uint16_t *v, int n);
int gb_mat4(glb *g, const float *v, int n);     // column-major
int gb_times(glb *g, const float *v, int n);    // anim input, w/ min/max

// ---- images (img.c). returns the id (+ ".a" / ".m" / ".am" by mode), NULL if missing/unreadable
// alpha: keeps the alpha channel, or derives it from brightness if there's none. mask: pure black -> clear
enum { IM_OPAQUE = 0, IM_ALPHA = 1, IM_MASK = 2 };
const char *bake_tex(const char *path, int mode);

// ---- rooms (room.c), models (model.c)
int bake_room(const char *rmesh);
int bake_model(const char *path);
