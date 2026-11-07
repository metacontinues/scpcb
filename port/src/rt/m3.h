// m3.h: blitz3d-style math. left-handed, y up, z fwd. column vecs.
// mat3 stores axes i,j,k as columns: M*v = i*x + j*y + k*z
#pragma once
#include <math.h>

typedef struct { float x, y, z; } v3;
typedef struct { v3 i, j, k; } m3;
typedef struct { m3 m; v3 v; } tf;

#define DTOR 0.017453292519943295f
#define RTOD 57.29577951308232f

static inline v3 v3n(float x, float y, float z) { return (v3){x, y, z}; }
static inline v3 v3add(v3 a, v3 b) { return (v3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline v3 v3sub(v3 a, v3 b) { return (v3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline v3 v3mul(v3 a, float s) { return (v3){a.x*s, a.y*s, a.z*s}; }
static inline v3 v3scl(v3 a, v3 b) { return (v3){a.x*b.x, a.y*b.y, a.z*b.z}; }
static inline float v3dot(v3 a, v3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline v3 v3cross(v3 a, v3 b) { return (v3){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
static inline float v3len(v3 a) { return sqrtf(v3dot(a, a)); }
static inline v3 v3norm(v3 a) { float l = v3len(a); return l > 0 ? v3mul(a, 1/l) : a; }
// blitz vector yaw/pitch (radians)
static inline float v3yaw(v3 a) { return atan2f(-a.x, a.z); }
static inline float v3pitch(v3 a) { return -atan2f(a.y, sqrtf(a.x*a.x + a.z*a.z)); }

static inline m3 m3id(void) { return (m3){{1,0,0},{0,1,0},{0,0,1}}; }
static inline v3 m3v(m3 m, v3 a) { return v3add(v3add(v3mul(m.i, a.x), v3mul(m.j, a.y)), v3mul(m.k, a.z)); }
static inline m3 m3mul(m3 a, m3 b) { return (m3){m3v(a, b.i), m3v(a, b.j), m3v(a, b.k)}; }
static inline m3 m3t(m3 m) { return (m3){{m.i.x,m.j.x,m.k.x},{m.i.y,m.j.y,m.k.y},{m.i.z,m.j.z,m.k.z}}; }
static inline m3 m3scale(v3 s) { return (m3){{s.x,0,0},{0,s.y,0},{0,0,s.z}}; }
static inline m3 m3inv(m3 m) {
	v3 a = v3cross(m.j, m.k), b = v3cross(m.k, m.i), c = v3cross(m.i, m.j);
	float d = v3dot(m.i, a);
	if (d == 0) return m3id();
	d = 1/d;
	return m3t((m3){v3mul(a, d), v3mul(b, d), v3mul(c, d)});
}
// strip scale, re-orthonormalize
static inline m3 m3ortho(m3 m) {
	v3 k = v3norm(m.k), j = v3norm(v3cross(k, m.i)), i = v3cross(j, k);
	return (m3){i, j, k};
}
static inline m3 m3pitch(float q) { float c = cosf(q), s = sinf(q); return (m3){{1,0,0},{0,c,s},{0,-s,c}}; }
static inline m3 m3yaw(float q) { float c = cosf(q), s = sinf(q); return (m3){{c,0,s},{0,1,0},{-s,0,c}}; }
static inline m3 m3roll(float q) { float c = cosf(q), s = sinf(q); return (m3){{c,s,0},{-s,c,0},{0,0,1}}; }
// radians in, yaw*pitch*roll like blitz
static inline m3 m3rot(float p, float y, float r) { return m3mul(m3mul(m3yaw(y), m3pitch(p)), m3roll(r)); }
static inline float m3getpitch(m3 m) { return -atan2f(m.k.y, sqrtf(m.k.x*m.k.x + m.k.z*m.k.z)); }
static inline float m3getyaw(m3 m) { return atan2f(-m.k.x, m.k.z); }
static inline float m3getroll(m3 m) { return atan2f(m.i.y, m.j.y); }

static inline tf tfid(void) { return (tf){m3id(), {0,0,0}}; }
static inline v3 tfp(tf t, v3 a) { return v3add(m3v(t.m, a), t.v); }
static inline tf tfmul(tf a, tf b) { return (tf){m3mul(a.m, b.m), tfp(a, b.v)}; }
static inline tf tfinv(tf t) { m3 i = m3inv(t.m); return (tf){i, v3mul(m3v(i, t.v), -1)}; }
// column-major 16 floats, gl/raylib layout
static inline void tf16(tf t, float *o) {
	float r[16] = {t.m.i.x,t.m.i.y,t.m.i.z,0, t.m.j.x,t.m.j.y,t.m.j.z,0, t.m.k.x,t.m.k.y,t.m.k.z,0, t.v.x,t.v.y,t.v.z,1};
	for (int n = 0; n < 16; n++) o[n] = r[n];
}
static inline float wrapdeg(float d) { while (d <= -180) d += 360; while (d > 180) d -= 360; return d; }
