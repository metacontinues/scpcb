// rmesh.c: port of LoadRMesh (MapSystem.bb)
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../bb/in.h"
#include "rmesh.h"

// GetTextureFromCache / CreatePropObj equivalents
typedef struct { char name[128]; htex t; } tcache;
static tcache *tcs; static int ntc, ctc;
typedef struct { char file[256]; ent e; } pcache;
static pcache *pcs; static int npc, cpc;

static const char *strip(const char *p) {
	const char *b = p;
	for (; *p; p++) if (*p == '\\' || *p == '/') b = p + 1;
	return b;
}
static htex tc_get(const char *n) {
	for (int i = 0; i < ntc; i++) if (!strcasecmp(tcs[i].name, strip(n))) return tcs[i].t;
	return 0;
}
static void tc_add(htex t) {
	grow(tcs, ntc, ctc);
	snprintf(tcs[ntc].name, sizeof tcs[ntc].name, "%s", strip(bbTextureName(t)));
	tcs[ntc++].t = t;
}
static ent prop(const char *file) {
	for (int i = 0; i < npc; i++) if (!strcasecmp(pcs[i].file, file)) return bbCopyEntity(pcs[i].e, 0);
	ent e = bbLoadMesh(file, 0);
	if (!e) return 0;
	grow(pcs, npc, cpc);
	snprintf(pcs[npc].file, sizeof pcs[npc].file, "%s", file);
	pcs[npc++].e = e;
	bbHideEntity(e); // template stays hidden, hand out copies
	return bbCopyEntity(e, 0);
}

static void rgeom(FILE *f, surf *s, int dbl) {
	int nv = bbReadInt(f);
	for (int j = 0; j < nv; j++) { float x = bbReadFloat(f), y = bbReadFloat(f), z = bbReadFloat(f); bbAddVertex(s, x, y, z, 0, 0, 0); }
	int nt = bbReadInt(f);
	for (int j = 0; j < nt; j++) {
		int a = bbReadInt(f), b = bbReadInt(f), c = bbReadInt(f);
		bbAddTriangle(s, a, b, c);
		if (dbl) bbAddTriangle(s, a, c, b);
	}
}
static void piece(const char *s, int n, char *o, int on) { // Piece(s, n, " ")
	int i = 1;
	while (*s && i < n) if (*s++ == ' ') i++;
	int k = 0;
	while (*s && *s != ' ' && k < on - 1) o[k++] = *s++;
	o[k] = 0;
}
static float piecef(const char *s, int n) { char b[64]; piece(s, n, b, sizeof b); return atof(b); }
static char *rs(FILE *f, char *o, int n) { char *s = bbReadString(f); snprintf(o, n, "%s", s); free(s); return o; }

int rmesh_load(const char *file, htex amb, rmesh *out) {
	*out = (rmesh){0};
	FILE *f = bbReadFile(file);
	if (!f) { fprintf(stderr, "rmesh: can't open %s\n", file); return 0; }
	char hdr[64], dir[512], b1[256], b2[256];
	rs(f, hdr, sizeof hdr);
	int trig = !strcmp(hdr, "RoomMesh.HasTriggerBox");
	if (!trig && strcmp(hdr, "RoomMesh")) { fprintf(stderr, "rmesh: %s is not rmesh (%s)\n", file, hdr); fclose(f); return 0; }
	snprintf(dir, sizeof dir, "%.*s", (int)(strip(file) - file), file);

	htex blank = bbCreateTexture(4, 4, TF_COLOR, 1);
	bbTextureFill(blank, 255, 255, 255, 255);
	ent coll = bbCreatePivot(0), opaque = bbCreateMesh(0), alpha = bbCreateMesh(0);

	int count = bbReadInt(f);
	for (int i = 0; i < count; i++) {
		ent child = bbCreateMesh(0);
		hbrush br = bbCreateBrush(255, 255, 255);
		htex tx[2] = {0, 0};
		int isa = 0;
		for (int j = 0; j < 2; j++) {
			int t = bbReadByte(f);
			if (!t) continue;
			rs(f, b1, sizeof b1);
			tx[j] = tc_get(b1);
			if (!tx[j]) {
				snprintf(b2, sizeof b2, "%s%s", dir, b1);
				tx[j] = bbLoadTexture(b2, t < 3 ? TF_COLOR | TF_MIP : TF_COLOR | TF_ALPHA | TF_MIP);
				if (tx[j]) {
					if (t == 1) bbTextureBlend(tx[j], TB_MUL2);
					if (strcasestr(b1, "_lm")) bbTextureBlend(tx[j], TB_ADD);
					tc_add(tx[j]);
				}
			}
			if (tx[j]) { isa = t == 3 ? 1 : 2; bbTextureCoords(tx[j], 1 - j); }
		}
		if (isa == 1) {
			if (tx[1]) { bbTextureBlend(tx[1], TB_MUL); bbBrushTexture(br, tx[1], 0, 0); }
			else bbBrushTexture(br, blank, 0, 0);
		} else if (tx[0] && tx[1]) {
			for (int j = 0; j < 2; j++) bbBrushTexture(br, tx[j], 0, j + 1); // bump maps: TODO
			bbBrushTexture(br, amb, 0, 0);
		} else for (int j = 0; j < 2; j++) bbBrushTexture(br, tx[j] ? tx[j] : blank, 0, j);

		surf *s = bbCreateSurface(child, 0);
		if (isa > 0) bbPaintSurface(s, br);
		bbFreeBrush(br);
		int nv = bbReadInt(f);
		for (int j = 0; j < nv; j++) {
			float x = bbReadFloat(f), y = bbReadFloat(f), z = bbReadFloat(f);
			int v = bbAddVertex(s, x, y, z, 0, 0, 0);
			for (int k = 0; k < 2; k++) { float u = bbReadFloat(f), w = bbReadFloat(f); bbVertexTexCoords(s, v, u, w, 0, k); }
			int r = bbReadByte(f), g = bbReadByte(f), b = bbReadByte(f);
			bbVertexColor(s, v, r, g, b, 1);
		}
		int nt = bbReadInt(f);
		for (int j = 0; j < nt; j++) { int a = bbReadInt(f), b = bbReadInt(f), c = bbReadInt(f); bbAddTriangle(s, a, b, c); }

		if (isa == 1) { bbAddMesh(child, alpha); bbFreeEntity(child); continue; }
		bbAddMesh(child, opaque);
		bbEntityParent(child, coll, 1);
		bbEntityAlpha(child, 0);
		bbEntityType(child, 1, 0); // HIT_MAP
		bbEntityPickMode(child, 2, 1);
		ent fl = bbCopyMesh(child, 0); // double-sided collision
		bbFlipMesh(fl); bbAddMesh(fl, child); bbFreeEntity(fl);
	}

	ent hidden = bbCreateMesh(0);
	count = bbReadInt(f);
	for (int i = 0; i < count; i++) rgeom(f, bbCreateSurface(hidden, 0), 1);

	if (trig) {
		out->ntrig = bbReadInt(f);
		for (int t = 0; t < out->ntrig; t++) {
			ent m = bbCreateMesh(0);
			if (t < 32) out->trig[t] = m;
			count = bbReadInt(f);
			for (int i = 0; i < count; i++) rgeom(f, bbCreateSurface(m, 0), 1);
			rs(f, b1, sizeof b1);
			if (t < 32) snprintf(out->trigname[t], 64, "%s", b1);
		}
		if (out->ntrig > 32) out->ntrig = 32;
	}

	count = bbReadInt(f);
	int cap = 0;
	for (int i = 0; i < count; i++) {
		rs(f, b1, sizeof b1);
		rpoint p = {0};
		float S = ROOMSCALE;
		if (!strcmp(b1, "screen")) {
			p.kind = RP_SCREEN; p.x = bbReadFloat(f)*S; p.y = bbReadFloat(f)*S; p.z = bbReadFloat(f)*S;
			rs(f, p.str, sizeof p.str);
		} else if (!strcmp(b1, "waypoint")) {
			p.kind = RP_WAYPOINT; p.x = bbReadFloat(f)*S; p.y = bbReadFloat(f)*S; p.z = bbReadFloat(f)*S;
		} else if (!strcmp(b1, "light") || !strcmp(b1, "spotlight")) {
			p.kind = b1[0] == 'l' ? RP_LIGHT : RP_SPOT;
			p.x = bbReadFloat(f)*S; p.y = bbReadFloat(f)*S; p.z = bbReadFloat(f)*S;
			p.range = bbReadFloat(f) / 2000;
			rs(f, b2, sizeof b2);
			float in = fminf(bbReadFloat(f) * 0.8f, 1);
			p.r = roundf((int)piecef(b2, 1) * in); // blitz float->int rounds
			p.g = roundf((int)piecef(b2, 2) * in);
			p.b = roundf((int)piecef(b2, 3) * in);
			if (p.kind == RP_SPOT) {
				rs(f, b2, sizeof b2);
				p.pitch = piecef(b2, 1); p.yaw = piecef(b2, 2);
				p.inner = bbReadInt(f); p.outer = bbReadInt(f);
			}
		} else if (!strcmp(b1, "soundemitter")) {
			p.kind = RP_SOUND; p.x = bbReadFloat(f)*S; p.y = bbReadFloat(f)*S; p.z = bbReadFloat(f)*S;
			p.snd = bbReadInt(f); p.range = bbReadFloat(f);
		} else if (!strcmp(b1, "playerstart")) {
			p.kind = RP_START; p.x = bbReadFloat(f); p.y = bbReadFloat(f); p.z = bbReadFloat(f);
			rs(f, b2, sizeof b2);
			p.pitch = piecef(b2, 1); p.yaw = piecef(b2, 2); p.roll = piecef(b2, 3);
		} else if (!strcmp(b1, "model")) {
			p.kind = RP_MODEL;
			rs(f, p.str, sizeof p.str);
			float v[9];
			for (int k = 0; k < 9; k++) v[k] = bbReadFloat(f);
			if (p.str[0]) {
				snprintf(b2, sizeof b2, "GFX\\Map\\Props\\%s", p.str);
				ent m = prop(b2);
				if (m) {
					bbPositionEntity(m, v[0], v[1], v[2], 0); bbRotateEntity(m, v[3], v[4], v[5], 0);
					bbScaleEntity(m, v[6], v[7], v[8], 0);
					bbEntityParent(m, opaque, 1); bbEntityType(m, 1, 0); bbEntityPickMode(m, 2, 1);
					p.ent = m;
				} else fprintf(stderr, "rmesh: prop %s failed\n", b2);
			}
			p.x = v[0]; p.y = v[1]; p.z = v[2];
		} else { fprintf(stderr, "rmesh: unknown point entity '%s', stopping\n", b1); break; }
		grow(out->pts, out->npts, cap);
		out->pts[out->npts++] = p;
	}
	fclose(f);

	ent fl = bbCopyMesh(alpha, 0); // alpha faces double-sided
	bbFlipMesh(fl); bbAddMesh(fl, alpha); bbFreeEntity(fl);
	bbAddMesh(alpha, opaque); bbFreeEntity(alpha);
	bbEntityFX(opaque, FX_FULLBRIGHT | FX_VCOLOR);
	bbEntityAlpha(hidden, 0); bbEntityType(hidden, 1, 0);
	bbFreeTexture(blank);

	ent obj = bbCreatePivot(0);
	bbCreatePivot(obj); // "meshes" slot, kept so child indices match the bb code
	bbEntityParent(opaque, obj, 1); bbEntityParent(hidden, obj, 1);
	bbCreatePivot(obj); bbCreatePivot(obj);
	bbEntityParent(coll, obj, 1);
	for (int t = 0; t < out->ntrig; t++) bbEntityParent(out->trig[t], obj, 1);
	out->obj = obj; out->coll = coll; out->hidden = hidden;
	return 1;
}

void rmesh_free(rmesh *r) { if (r->obj) bbFreeEntity(r->obj); free(r->pts); *r = (rmesh){0}; }
