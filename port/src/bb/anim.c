// anim.c: keyframes (lerp/slerp per channel) + cpu skinning, blitz style
#include <stdlib.h>
#include <string.h>
#include "in.h"

m3 qmat(float w, float x, float y, float z) {
	float xx = x*x, yy = y*y, zz = z*z, xy = x*y, xz = x*z, yz = y*z, wx = w*x, wy = w*y, wz = w*z;
	return (m3){{1-2*(yy+zz), 2*(xy-wz), 2*(xz+wy)}, {2*(xy+wz), 1-2*(xx+zz), 2*(yz-wx)}, {2*(xz-wy), 2*(yz+wx), 1-2*(xx+yy)}};
}
static void slerp(const float *a, const float *b, float t, float *o) {
	float d = a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3], s = 1;
	if (d < 0) { d = -d; s = -1; } // shortest path
	float ka = 1 - t, kb = t;
	if (d < 0.9995f) {
		float th = acosf(d), st = sinf(th);
		ka = sinf((1 - t) * th) / st; kb = sinf(t * th) / st;
	}
	float l = 0;
	for (int i = 0; i < 4; i++) { o[i] = a[i] * ka + b[i] * kb * s; l += o[i] * o[i]; }
	l = sqrtf(l);
	if (l > 0) for (int i = 0; i < 4; i++) o[i] /= l;
}

// keys for one channel around frame f: *a = last <= f, *b = first > f (-1 if none)
static void around(E *x, int ch, float f, int *a, int *b) {
	*a = *b = -1;
	for (int i = 0; i < x->nk; i++) {
		if (!(x->keys[i].has & ch)) continue;
		if (x->keys[i].t <= f) *a = i;
		else if (*b < 0) *b = i;
	}
}
static void posenode(ent e, float f) {
	E *x = bb_e(e);
	if (!x->nk) return;
	for (int ch = 1; ch <= 4; ch <<= 1) {
		int a, b;
		around(x, ch, f, &a, &b);
		if (a < 0 && b < 0) continue;
		if (a < 0) a = b;
		if (b < 0) b = a;
		key *p = &x->keys[a], *q = &x->keys[b];
		float t = q->t > p->t ? (f - p->t) / (q->t - p->t) : 0;
		if (ch == 1) x->pos = v3add(p->p, v3mul(v3sub(q->p, p->p), t));
		else if (ch == 2) x->scl = v3add(p->s, v3mul(v3sub(q->s, p->s), t));
		else { float o[4]; slerp(p->q, q->q, t, o); x->rot = m3ortho(qmat(o[0], o[1], o[2], o[3])); }
	}
	bb_dirty(e);
}
static void posetree(ent e, float f) {
	posenode(e, f);
	for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) posetree(c, f);
}

static ent animroot(ent e) {
	for (ent p = e; p; p = bb_e(p)->par) if (bb_e(p)->anim) return p;
	ent r = e;
	while (bb_e(r)->par) r = bb_e(r)->par;
	return r;
}
static void skinmesh(ent e) {
	E *x = bb_e(e);
	skin *k = x->md->sk;
	if (!x->bones || x->nbones != k->nb) {
		free(x->bones);
		ent r = animroot(e);
		ent *bs = malloc(sizeof(ent) * k->nb);
		for (int i = 0; i < k->nb; i++) bs[i] = bbFindChild(r, k->name[i]);
		x = bb_e(e); x->bones = bs; x->nbones = k->nb;
	}
	tf mi = tfinv(bb_wt(e)), *M = malloc(sizeof(tf) * k->nb);
	for (int i = 0; i < k->nb; i++)
		M[i] = x->bones[i] && bbEntityExists(x->bones[i]) ? tfmul(tfmul(mi, bb_wt(x->bones[i])), k->binv[i]) : tfid();
	mdat *m = x->md;
	for (int si = 0; si < m->ns; si++) {
		surf *s = m->s[si];
		if (!s->bw) continue;
		for (int v = 0; v < s->nv; v++) {
			float *w = s->bw + v*4, *bp = s->bpos + v*3, *bn = s->bnrm + v*3;
			if (w[0] <= 0) continue; // unweighted: stays in bind pose
			v3 p = {0}, n = {0}, P = v3n(bp[0], bp[1], bp[2]), N = v3n(bn[0], bn[1], bn[2]);
			for (int j = 0; j < 4 && w[j] > 0; j++) {
				tf *t = &M[s->bi[v*4+j]];
				p = v3add(p, v3mul(tfp(*t, P), w[j]));
				n = v3add(n, v3mul(m3v(t->m, N), w[j]));
			}
			n = v3norm(n);
			memcpy(s->pos + v*3, &p, 12); memcpy(s->nrm + v*3, &n, 12);
		}
		s->dirty = 1; s->ver++;
	}
	m->bdirty = 1;
	free(M);
}
static void skintree(ent e) {
	E *x = bb_e(e);
	if (x->md && x->md->sk) skinmesh(e);
	for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) skintree(c);
}

void anim_pose(ent e, float f) { posetree(e, f); skintree(e); }

static float seqa(E *x) { return x->nseq && x->aseq < x->nseq ? x->seqs[x->aseq].a : 0; }
static float seqlen(E *x) { return x->nseq && x->aseq < x->nseq ? x->seqs[x->aseq].b - x->seqs[x->aseq].a : 0; }

void bbSetAnimTime(ent e, float t, int seq) {
	E *x = bb_e(e);
	x->atime = t;
	if (seq >= 0 && seq < x->nseq) x->aseq = seq;
	anim_pose(e, seqa(x) + t);
}
float bbAnimTime(ent e) { return bb_e(e)->atime; }
int bbAnimSeq(ent e) { return bb_e(e)->aseq; }
int bbAnimLength(ent e) { return seqlen(bb_e(e)); }
int bbAnimating(ent e) { return bb_e(e)->amode != 0; }
void bbAnimate(ent e, int mode, float speed, int seq, float trans) {
	(void)trans; // TODO: transition blending
	E *x = bb_e(e);
	x->amode = mode; x->aspeed = speed;
	if (seq >= 0 && seq < x->nseq && seq != x->aseq) { x->aseq = seq; x->atime = 0; }
	if (mode) anim_pose(e, seqa(x) + x->atime);
}
int bbExtractAnimSeq(ent e, int first, int last, int seq) {
	E *x = bb_e(e);
	int base = seq >= 0 && seq < x->nseq ? x->seqs[seq].a : 0;
	x->seqs = realloc(x->seqs, sizeof *x->seqs * (x->nseq + 1));
	x->seqs[x->nseq].a = base + first; x->seqs[x->nseq].b = base + last;
	return x->nseq++;
}

void anim_step(float k) {
	for (ent e = 1; e < bb_nents; e++) {
		E *x = &bb_ents[e];
		if (!x->used || !x->amode || !k) continue;
		float len = seqlen(x), t = x->atime + x->aspeed * k;
		if (len <= 0) continue;
		if (x->amode == 1) { t = fmodf(t, len); if (t < 0) t += len; }
		else if (x->amode == 2) { // pingpong
			if (t > len) { t = 2*len - t; x->aspeed = -x->aspeed; }
			else if (t < 0) { t = -t; x->aspeed = -x->aspeed; }
		} else if (t >= len || t <= 0) { t = fmaxf(0, fminf(len, t)); x->amode = 0; } // one-shot
		x->atime = t;
		anim_pose(e, seqa(x) + t);
	}
}

const char *bbEntityClass(ent e) {
	static const char *n[] = {"Pivot", "Mesh", "Camera", "Light", "Sprite"};
	return n[bb_e(e)->kind];
}
