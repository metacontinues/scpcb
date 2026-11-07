// shd.c: world shaders. raylib binds the attributes + mvp/matModel/matView/matNormal + albedo (texture0) +
// colDiffuse itself; rooms' lightmap rides in MATERIAL_MAP_OCCLUSION, whose loc we point at "lightmap"
#include <stdio.h>
#include "raylib.h"
#include "shd.h"

Shader shd_world[MK_N];

static const char *room_vs =
	"#version 100\n"
	"attribute vec3 vertexPosition; attribute vec2 vertexTexCoord, vertexTexCoord2; attribute vec4 vertexColor;\n"
	"uniform mat4 mvp, matModel, matView;\n"
	"varying vec2 uv0, uv1; varying vec4 vc; varying float vd;\n"
	"void main() {\n"
	"  uv0 = vertexTexCoord; uv1 = vertexTexCoord2; vc = vertexColor;\n"
	"  vd = -(matView * matModel * vec4(vertexPosition, 1.0)).z;\n"
	"  gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
	"}\n";

static const char *room_fs =
	"precision mediump float;\n"
	"uniform sampler2D texture0, lightmap;\n"
	"uniform vec4 colDiffuse;\n"
	"uniform float amb;\n"
	"uniform vec4 fogc; uniform vec2 fogr;\n" // near, far
	"varying vec2 uv0, uv1; varying vec4 vc; varying float vd;\n"
	"void main() {\n"
	"  vec4 d = texture2D(texture0, uv0) * colDiffuse;\n"
	"#if KIND == 0\n"
	"  vec4 c = vec4((vc.rgb * amb * 2.0 + texture2D(lightmap, uv1).rgb) * d.rgb * 2.0, 1.0);\n"
	"#elif KIND == 1\n"
	"  vec4 c = vec4(vc.rgb * d.rgb * 2.0, 1.0);\n"
	"#elif KIND == 2\n"
	"  vec4 c = vec4(vc.rgb, 1.0);\n"
	"#else\n"
	"  vec4 c = vc * d;\n"
	"  if (c.a < 0.004) discard;\n"
	"#endif\n"
	"  if (fogr.y > fogr.x) c.rgb = mix(c.rgb, fogc.rgb, clamp((vd - fogr.x) / (fogr.y - fogr.x), 0.0, 1.0));\n"
	"  gl_FragColor = clamp(c, 0.0, 1.0);\n"
	"}\n";

// per-vertex lighting like blitz's fixed function: base (brush or vertex color) * (ambient + lights)
static const char *model_vs =
	"attribute vec3 vertexPosition, vertexNormal; attribute vec2 vertexTexCoord; attribute vec4 vertexColor;\n"
	"uniform mat4 mvp, matModel, matView, matNormal;\n"
	"uniform vec4 colDiffuse;\n"
	"uniform vec3 lamb; uniform int nl; uniform vec4 lpos[4], lcol[4];\n"
	"varying vec2 uv0; varying vec4 vc; varying float vd;\n"
	"void main() {\n"
	"  vec4 wp = matModel * vec4(vertexPosition, 1.0);\n"
	"#if VCOL\n"
	"  vec4 base = vec4(vertexColor.rgb, colDiffuse.a * vertexColor.a);\n"
	"#else\n"
	"  vec4 base = colDiffuse;\n"
	"#endif\n"
	"#if FULLBRIGHT\n"
	"  vec3 l = vec3(1.0);\n"
	"#else\n"
	"  vec3 l = lamb, n = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);\n"
	"  for (int i = 0; i < 4; i++) {\n"
	"    if (i >= nl) break;\n"
	"    vec3 L = -lpos[i].xyz; float at = 1.0;\n"
	"    if (lpos[i].w > 0.5) { vec3 v = lpos[i].xyz - wp.xyz; float d = length(v); L = v / max(d, 1e-4); at = 1.0 / (1.0 + d / lcol[i].w); }\n"
	"    l += lcol[i].rgb * max(dot(n, normalize(L)), 0.0) * at;\n"
	"  }\n"
	"#endif\n"
	"  vc = vec4(base.rgb * l, base.a);\n"
	"  uv0 = vertexTexCoord;\n"
	"  vd = -(matView * wp).z;\n"
	"  gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
	"}\n";

static const char *model_fs =
	"precision mediump float;\n"
	"uniform sampler2D texture0;\n"
	"uniform vec4 fogc; uniform vec2 fogr;\n"
	"varying vec2 uv0; varying vec4 vc; varying float vd;\n"
	"void main() {\n"
	"  vec4 c = vc * texture2D(texture0, uv0);\n"
	"  if (c.a < 0.004) discard;\n"
	"  if (fogr.y > fogr.x) c.rgb = mix(c.rgb, fogc.rgb, clamp((vd - fogr.x) / (fogr.y - fogr.x), 0.0, 1.0));\n"
	"  gl_FragColor = clamp(c, 0.0, 1.0);\n"
	"}\n";

void shd_init(void) {
	char vs[4096], fs[4096];
	for (int k = 0; k < MK_N; k++) {
		if (k < MK_MODEL) {
			snprintf(fs, sizeof fs, "#version 100\n#define KIND %d\n%s", k, room_fs);
			shd_world[k] = LoadShaderFromMemory(room_vs, fs);
			shd_world[k].locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(shd_world[k], "lightmap");
		} else {
			int v = k - MK_MODEL;
			snprintf(vs, sizeof vs, "#version 100\n#define FULLBRIGHT %d\n#define VCOL %d\n%s", v & 1, v >> 1 & 1, model_vs);
			snprintf(fs, sizeof fs, "#version 100\n%s", model_fs);
			shd_world[k] = LoadShaderFromMemory(vs, fs);
		}
	}
	shd_ambient(0);
	shd_fog(0, 0, BLACK);
	shd_lights((Vector3){1, 1, 1}, 0, NULL, NULL);
}

void shd_fog(float near, float far, Color c) {
	float r[2] = {near, far}, col[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, 1};
	for (int k = 0; k < MK_N; k++) {
		SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogr"), r, SHADER_UNIFORM_VEC2);
		SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogc"), col, SHADER_UNIFORM_VEC4);
	}
}

void shd_ambient(float a) {
	for (int k = 0; k < MK_MODEL; k++) SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "amb"), &a, SHADER_UNIFORM_FLOAT);
}

void shd_lights(Vector3 amb, int n, const Vector4 *pos, const Vector4 *col) {
	if (n > SHD_LIGHTS) n = SHD_LIGHTS;
	for (int k = MK_MODEL; k < MK_N; k++) {
		Shader s = shd_world[k];
		SetShaderValue(s, GetShaderLocation(s, "lamb"), &amb, SHADER_UNIFORM_VEC3);
		SetShaderValue(s, GetShaderLocation(s, "nl"), &n, SHADER_UNIFORM_INT);
		if (n) {
			SetShaderValueV(s, GetShaderLocation(s, "lpos"), pos, SHADER_UNIFORM_VEC4, n);
			SetShaderValueV(s, GetShaderLocation(s, "lcol"), col, SHADER_UNIFORM_VEC4, n);
		}
	}
}
