// glb.c: just enough of a gltf 2.0 binary writer for what raylib's LoadModel reads
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "bake.h"

static void sep(sb *b) { if (b->n) sbf(b, ","); }

int gb_view(glb *g, const void *d, size_t n, int target) {
	while (g->bin.n % 4) sbraw(&g->bin, "", 1);
	sep(&g->views);
	sbf(&g->views, "{\"buffer\":0,\"byteOffset\":%zu,\"byteLength\":%zu", g->bin.n, n);
	if (target) sbf(&g->views, ",\"target\":%d", target);
	sbf(&g->views, "}");
	sbraw(&g->bin, d, n);
	return g->nview++;
}

int gb_acc(glb *g, int view, int ctype, int count, const char *type, int norm, const float *mn, const float *mx) {
	sep(&g->accs);
	sbf(&g->accs, "{\"bufferView\":%d,\"componentType\":%d,\"count\":%d,\"type\":\"%s\"", view, ctype, count, type);
	if (norm) sbf(&g->accs, ",\"normalized\":true");
	if (mn) sbf(&g->accs, ",\"min\":[%g,%g,%g],\"max\":[%g,%g,%g]", mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]);
	sbf(&g->accs, "}");
	return g->nacc++;
}

int gb_vec3(glb *g, const float *v, int n) {
	float mn[3] = {FLT_MAX, FLT_MAX, FLT_MAX}, mx[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	for (int i = 0; i < n * 3; i++) { if (v[i] < mn[i % 3]) mn[i % 3] = v[i]; if (v[i] > mx[i % 3]) mx[i % 3] = v[i]; }
	return gb_acc(g, gb_view(g, v, n * 12, 34962), GB_F32, n, "VEC3", 0, mn, mx);
}
int gb_vec2(glb *g, const float *v, int n) { return gb_acc(g, gb_view(g, v, n * 8, 34962), GB_F32, n, "VEC2", 0, 0, 0); }
int gb_rgba8(glb *g, const uint8_t *v, int n) { return gb_acc(g, gb_view(g, v, n * 4, 34962), GB_U8, n, "VEC4", 1, 0, 0); }
int gb_idx16(glb *g, const uint16_t *v, int n) { return gb_acc(g, gb_view(g, v, n * 2, 34963), GB_U16, n, "SCALAR", 0, 0, 0); }

int gb_mat(glb *g, const char *name) {
	sep(&g->mats);
	sbf(&g->mats, "{\"name\":\"%s\"}", name);
	return g->nmat++;
}

void gb_mesh(glb *g, const gbprim *p, int np) {
	sep(&g->meshes);
	sbf(&g->meshes, "{\"primitives\":[");
	for (int i = 0; i < np; i++) {
		sbf(&g->meshes, "%s{\"attributes\":{\"POSITION\":%d", i ? "," : "", p[i].pos);
		if (p[i].uv0 >= 0) sbf(&g->meshes, ",\"TEXCOORD_0\":%d", p[i].uv0);
		if (p[i].uv1 >= 0) sbf(&g->meshes, ",\"TEXCOORD_1\":%d", p[i].uv1);
		if (p[i].col >= 0) sbf(&g->meshes, ",\"COLOR_0\":%d", p[i].col);
		sbf(&g->meshes, "}");
		if (p[i].idx >= 0) sbf(&g->meshes, ",\"indices\":%d", p[i].idx);
		if (p[i].mat >= 0) sbf(&g->meshes, ",\"material\":%d", p[i].mat);
		sbf(&g->meshes, "}");
	}
	sbf(&g->meshes, "]}");
	sep(&g->nodes);
	sbf(&g->nodes, "{\"mesh\":%d}", g->nmesh++);
}

static void put32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }

int gb_write(glb *g, const char *path) {
	sb js = {0};
	sbf(&js, "{\"asset\":{\"version\":\"2.0\",\"generator\":\"scpcb bake\"},\"scene\":0,\"scenes\":[{\"nodes\":[");
	for (int i = 0; i < g->nmesh; i++) sbf(&js, "%s%d", i ? "," : "", i);
	sbf(&js, "]}],\"nodes\":[%s],\"meshes\":[%s]", g->nodes.s ? g->nodes.s : "", g->meshes.s ? g->meshes.s : "");
	if (g->nmat) sbf(&js, ",\"materials\":[%s]", g->mats.s);
	while (g->bin.n % 4) sbraw(&g->bin, "", 1);
	sbf(&js, ",\"accessors\":[%s],\"bufferViews\":[%s],\"buffers\":[{\"byteLength\":%zu}]}",
		g->accs.s ? g->accs.s : "", g->views.s ? g->views.s : "", g->bin.n);
	while (js.n % 4) sbf(&js, " ");
	mkdirs(path);
	FILE *f = fopen(path, "wb");
	int ok = f != NULL;
	if (f) {
		put32(f, 0x46546c67); put32(f, 2); put32(f, 12 + 8 + js.n + 8 + g->bin.n);
		put32(f, js.n); put32(f, 0x4e4f534a); fwrite(js.s, 1, js.n, f);
		put32(f, g->bin.n); put32(f, 0x004e4942); fwrite(g->bin.s, 1, g->bin.n, f);
		ok = !ferror(f);
		fclose(f);
	}
	free(js.s); free(g->bin.s); free(g->views.s); free(g->accs.s); free(g->meshes.s); free(g->mats.s); free(g->nodes.s);
	*g = (glb){0};
	return ok;
}
