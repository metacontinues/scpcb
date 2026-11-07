// glb.c: just enough of a gltf 2.0 binary writer for what raylib's LoadModel/LoadModelAnimations read
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "bake.h"

static void sep(sb *b) { if (b->n) sbf(b, ","); }
static const char *js(const sb *b) { return b->s ? b->s : ""; }

gbprim gb_prim0(void) { gbprim p = {-1, -1, -1, -1, -1, -1, -1, -1, -1}; return p; }

int gb_view(glb *g, const void *d, size_t n, int target) {
	while (g->bin.n % 4) sbraw(&g->bin, "", 1);
	sep(&g->views);
	sbf(&g->views, "{\"buffer\":0,\"byteOffset\":%zu,\"byteLength\":%zu", g->bin.n, n);
	if (target) sbf(&g->views, ",\"target\":%d", target);
	sbf(&g->views, "}");
	sbraw(&g->bin, d, n);
	return g->nview++;
}

int gb_acc(glb *g, int view, int ctype, int count, const char *type, int norm, int ncomp, const float *mn, const float *mx) {
	sep(&g->accs);
	sbf(&g->accs, "{\"bufferView\":%d,\"componentType\":%d,\"count\":%d,\"type\":\"%s\"", view, ctype, count, type);
	if (norm) sbf(&g->accs, ",\"normalized\":true");
	if (mn) {
		sbf(&g->accs, ",\"min\":[");
		for (int i = 0; i < ncomp; i++) sbf(&g->accs, "%s%.9g", i ? "," : "", mn[i]);
		sbf(&g->accs, "],\"max\":[");
		for (int i = 0; i < ncomp; i++) sbf(&g->accs, "%s%.9g", i ? "," : "", mx[i]);
		sbf(&g->accs, "]");
	}
	sbf(&g->accs, "}");
	return g->nacc++;
}

static void bounds(const float *v, int n, int k, float *mn, float *mx) {
	for (int i = 0; i < k; i++) { mn[i] = n ? FLT_MAX : 0; mx[i] = n ? -FLT_MAX : 0; }
	for (int i = 0; i < n * k; i++) { if (v[i] < mn[i % k]) mn[i % k] = v[i]; if (v[i] > mx[i % k]) mx[i % k] = v[i]; }
}
int gb_vec3(glb *g, const float *v, int n) {
	float mn[3], mx[3];
	bounds(v, n, 3, mn, mx);
	return gb_acc(g, gb_view(g, v, n * 12, 34962), GB_F32, n, "VEC3", 0, 3, mn, mx);
}
int gb_times(glb *g, const float *v, int n) {
	float mn[1], mx[1];
	bounds(v, n, 1, mn, mx);
	return gb_acc(g, gb_view(g, v, n * 4, 0), GB_F32, n, "SCALAR", 0, 1, mn, mx);
}
int gb_vec2(glb *g, const float *v, int n) { return gb_acc(g, gb_view(g, v, n * 8, 34962), GB_F32, n, "VEC2", 0, 0, 0, 0); }
int gb_vec4(glb *g, const float *v, int n) { return gb_acc(g, gb_view(g, v, n * 16, 34962), GB_F32, n, "VEC4", 0, 0, 0, 0); }
int gb_rgba8(glb *g, const uint8_t *v, int n) { return gb_acc(g, gb_view(g, v, n * 4, 34962), GB_U8, n, "VEC4", 1, 0, 0, 0); }
int gb_u8x4(glb *g, const uint8_t *v, int n) { return gb_acc(g, gb_view(g, v, n * 4, 34962), GB_U8, n, "VEC4", 0, 0, 0, 0); }
int gb_idx16(glb *g, const uint16_t *v, int n) { return gb_acc(g, gb_view(g, v, n * 2, 34963), GB_U16, n, "SCALAR", 0, 0, 0, 0); }
int gb_mat4(glb *g, const float *v, int n) { return gb_acc(g, gb_view(g, v, n * 64, 0), GB_F32, n, "MAT4", 0, 0, 0, 0); }

int gb_mat(glb *g, const char *name) {
	sep(&g->mats);
	sbf(&g->mats, "{\"name\":\"%s\"}", name);
	return g->nmat++;
}

int gb_mesh(glb *g, const gbprim *p, int np) {
	static const char *an[] = {"POSITION", "NORMAL", "TEXCOORD_0", "TEXCOORD_1", "COLOR_0", "JOINTS_0", "WEIGHTS_0"};
	sep(&g->meshes);
	sbf(&g->meshes, "{\"primitives\":[");
	for (int i = 0; i < np; i++) {
		const int at[] = {p[i].pos, p[i].nrm, p[i].uv0, p[i].uv1, p[i].col, p[i].jnt, p[i].wgt};
		sbf(&g->meshes, "%s{\"attributes\":{", i ? "," : "");
		for (int a = 0, k = 0; a < 7; a++) if (at[a] >= 0) sbf(&g->meshes, "%s\"%s\":%d", k++ ? "," : "", an[a], at[a]);
		sbf(&g->meshes, "}");
		if (p[i].idx >= 0) sbf(&g->meshes, ",\"indices\":%d", p[i].idx);
		if (p[i].mat >= 0) sbf(&g->meshes, ",\"material\":%d", p[i].mat);
		sbf(&g->meshes, "}");
	}
	sbf(&g->meshes, "]}");
	return g->nmesh++;
}

int gb_node(glb *g, int root, const char *json) {
	sep(&g->nodes);
	sbf(&g->nodes, "{%s}", json);
	if (root) { sep(&g->roots); sbf(&g->roots, "%d", g->nnode); }
	return g->nnode++;
}
void gb_skin(glb *g, const char *json) { sep(&g->skins); sbf(&g->skins, "{%s}", json); }
void gb_anim(glb *g, const char *json) { sep(&g->anims); sbf(&g->anims, "{%s}", json); }

static void put32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }

int gb_write(glb *g, const char *path) {
	sb j = {0};
	while (g->bin.n % 4) sbraw(&g->bin, "", 1);
	sbf(&j, "{\"asset\":{\"version\":\"2.0\",\"generator\":\"scpcb bake\"},\"scene\":0,\"scenes\":[{\"nodes\":[%s]}]", js(&g->roots));
	sbf(&j, ",\"nodes\":[%s],\"meshes\":[%s]", js(&g->nodes), js(&g->meshes));
	if (g->nmat) sbf(&j, ",\"materials\":[%s]", js(&g->mats));
	if (g->skins.n) sbf(&j, ",\"skins\":[%s]", js(&g->skins));
	if (g->anims.n) sbf(&j, ",\"animations\":[%s]", js(&g->anims));
	sbf(&j, ",\"accessors\":[%s],\"bufferViews\":[%s],\"buffers\":[{\"byteLength\":%zu}]}", js(&g->accs), js(&g->views), g->bin.n);
	while (j.n % 4) sbf(&j, " ");
	mkdirs(path);
	FILE *f = fopen(path, "wb");
	int ok = f != NULL;
	if (f) {
		put32(f, 0x46546c67); put32(f, 2); put32(f, 12 + 8 + j.n + 8 + g->bin.n);
		put32(f, j.n); put32(f, 0x4e4f534a); fwrite(j.s, 1, j.n, f);
		put32(f, g->bin.n); put32(f, 0x004e4942); fwrite(g->bin.s, 1, g->bin.n, f);
		ok = !ferror(f);
		fclose(f);
	}
	sb *all[] = {&j, &g->bin, &g->views, &g->accs, &g->meshes, &g->mats, &g->nodes, &g->roots, &g->skins, &g->anims};
	for (size_t i = 0; i < sizeof all / sizeof *all; i++) free(all[i]->s);
	*g = (glb){0};
	return ok;
}
