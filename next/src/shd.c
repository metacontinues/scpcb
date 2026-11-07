// shd.c: world shaders. raylib binds the attributes + mvp/matModel/matView + albedo (texture0) itself;
// the lightmap rides in MATERIAL_MAP_OCCLUSION, whose loc we point at "lightmap"
#include <stdio.h>
#include "raylib.h"
#include "shd.h"

Shader shd_world[MK_N];

static const char *vs =
	"#version 100\n"
	"attribute vec3 vertexPosition; attribute vec2 vertexTexCoord, vertexTexCoord2; attribute vec4 vertexColor;\n"
	"uniform mat4 mvp, matModel, matView;\n"
	"varying vec2 uv0, uv1; varying vec4 vc; varying float vd;\n"
	"void main() {\n"
	"  uv0 = vertexTexCoord; uv1 = vertexTexCoord2; vc = vertexColor;\n"
	"  vd = -(matView * matModel * vec4(vertexPosition, 1.0)).z;\n"
	"  gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
	"}\n";

static const char *fs =
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

void shd_init(void) {
	for (int k = 0; k < MK_N; k++) {
		char src[4096];
		snprintf(src, sizeof src, "#version 100\n#define KIND %d\n%s", k, fs);
		Shader s = LoadShaderFromMemory(vs, src);
		s.locs[SHADER_LOC_MAP_OCCLUSION] = GetShaderLocation(s, "lightmap");
		shd_world[k] = s;
	}
	shd_ambient(0);
	shd_fog(0, 0, BLACK);
}

void shd_fog(float near, float far, Color c) {
	float r[2] = {near, far}, col[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, 1};
	for (int k = 0; k < MK_N; k++) {
		SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogr"), r, SHADER_UNIFORM_VEC2);
		SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "fogc"), col, SHADER_UNIFORM_VEC4);
	}
}

void shd_ambient(float a) {
	for (int k = 0; k < MK_N; k++) SetShaderValue(shd_world[k], GetShaderLocation(shd_world[k], "amb"), &a, SHADER_UNIFORM_FLOAT);
}
