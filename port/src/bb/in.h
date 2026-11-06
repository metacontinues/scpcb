// in.h: runtime internals, shared by src/bb and src/fmt. not for game code.
#pragma once
#include "bb.h"
#include "raylib.h"

#define die(...) (fprintf(stderr, "bb: " __VA_ARGS__), fputc('\n', stderr), exit(1))
#define grow(p, n, cap) do { if ((n) >= (cap)) { (cap) = (cap) ? (cap)*2 : 16; \
	(p) = realloc((p), sizeof *(p) * (cap)); } } while (0)

typedef struct {
	float r, g, b, a, shine;
	int blend, fx;
	htex tex[BB_NTEX];
} brush;

struct surf {
	int nv, cv, nt, ct;
	float *pos, *nrm, *uv0, *uv1;
	unsigned char *col;
	unsigned short *idx;
	brush br;
	Mesh gpu;
	int dirty;
	unsigned ver; // bumped on every edit, for collision caches
	float *bpos, *bnrm, *bw; unsigned char *bi; // skin: bind pose + 4 weights/bone idx per vert
};

// mesh data, shared by CopyEntity, deep copied by CopyMesh
typedef struct { int nb; char (*name)[64]; tf *binv; } skin; // bones by name, bind inverse (mesh space)

typedef struct {
	surf **s;
	skin *sk;
	int ns, rc;
	v3 lo, hi; int bdirty; // bounds
} mdat;

enum { EK_PIVOT, EK_MESH, EK_CAM, EK_LIGHT, EK_SPRITE };

typedef struct {
	float n, f, zoom;
	int vx, vy, vw, vh; // viewport, vw=0 -> full
	int clsc, clsz, proj, fogm;
	float fogn, fogf;
	Color cls, fog;
} cam;

typedef struct { float t; v3 p, s; float q[4]; int has; } key; // has: 1 pos 2 scl 4 rot. q = w x y z

typedef struct {
	int used, kind;
	ent par, first, last, next, prev; // children list
	v3 pos, scl; m3 rot;
	tf wt; int wdirty;
	char name[64];
	int hidden, order;
	brush br; int hasbr;
	mdat *md;
	cam *cam;
	// light
	int ltype; float lr, lg, lb, lrange, lin, lout;
	// sprite
	float sx, sy, hx, hy, sang; int view;
	// collision / pick
	int ctype, pmode, pobsc;
	float rx, ry; v3 boxo, boxs;
	// anim (anim.c). keys on any node, the ANIM chunk's node owns time/seqs
	key *keys; int nk;
	int anim, amode, aseq, nseq; float atime, aspeed;
	struct { int a, b; } *seqs;
	ent *bones; int nbones; // skinned mesh: resolved bone ents
	// collision state (coll.c)
	v3 opos; int ohas;
	struct hit *hits; int nhit, chit;
	struct ctri *ct; int nct; tf ctf; unsigned cver; v3 clo, chi; // world tri cache + aabb
} E;

extern E *bb_ents;
extern int bb_nents;
extern float bb_amb[3];

E *bb_e(ent e); // checked lookup, dies on bad handle
ent bb_new(int kind, ent par);
tf bb_wt(ent e);
void bb_dirty(ent e);

brush bb_brdef(void);
void br_ref(brush *b);   // tex refcounts: brushes keep textures alive
void br_unref(brush *b); // like blitz, FreeTexture on an in-use tex is fine
brush *bb_brp(hbrush b);
mdat *bb_md(ent e); // dies if not mesh
mdat *md_new(void);
surf *md_addsurf(mdat *m, brush br);
mdat *md_copy(mdat *m);
void md_unref(mdat *m);
void md_bounds(mdat *m);
void surf_upload(surf *s);

typedef struct {
	Texture2D t;
	int used, rc, w, h, flags, blend, coords;
	float su, sv, u0, v0;
	char name[256];
} tex;
tex *bb_t(htex t); // NULL for 0
void tex_ref(htex t);
void tex_unref(htex t);
// format loaders (src/fmt): return the root node tree
ent b3d_load(const char *p, ent par);
ent x_load(const char *p, ent par);
void coll_free(E *x);
m3 qmat(float w, float x, float y, float z); // blitz Quat::getMatrix
void anim_pose(ent e, float t);
void anim_step(float k);
ent bb_pick(v3 from, v3 d, float rad, ent ex1, ent ex2, int obsonly);
void snd_update(void);
void snd_quit(void);
void rend_init(void);
void rend_quit(void);
