// mdl.h: a source model (.b3d / .x) as plain data, still in blitz's left-handed space. model.c writes it out.
#pragma once

typedef struct { float p[3], n[3], c[4], uv[2][2]; } mvtx;
typedef struct { int brush, *tri, ntri; } msurf;          // tri: 3 vertex ids per triangle, into the node's verts
typedef struct { int t, has; float p[3], s[3], q[4]; } mkey; // has: 1 pos, 2 scale, 4 rot. q = w x y z (b3d)
typedef struct { int node, v; float w; } mweight;          // BONE: vertex v of mesh node `node` follows this node

typedef struct {
	char name[64];
	int parent;                // -1 = root
	float pos[3], scl[3], rot[3][3]; // local rest pose, rot in rows
	mvtx *v; int nv, nrm;      // nrm: normals came w/ the file
	msurf *s; int ns;
	mkey *k; int nk;
	mweight *w; int nw;        // weights onto this node (it's a bone)
} mnode;

typedef struct { char name[256]; int flags, blend; float px, py, sx, sy; } mtex;
typedef struct { float col[4]; int blend, fx, tex[8], ntex; } mbrush;

typedef struct {
	char dir[1024];            // for texture paths
	mnode *n; int nn;
	mtex *t; int nt;
	mbrush *b; int nb;
	int frames, uv1;           // uv1: some vertex has a 2nd uv set
} mdl;

int b3d_read(const char *path, mdl *m);
int x_read(const char *path, mdl *m);
void mdl_free(mdl *m);
int mdl_node(mdl *m, const char *name, int parent); // new node w/ identity rest pose
