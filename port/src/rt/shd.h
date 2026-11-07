// shd.h: the one blitz-fixed-function-ish shader. glsl es 100 so it runs on angle es2.
#pragma once

static const char *shd_vs =
"#version 100\n"
"attribute vec3 vertexPosition; attribute vec2 vertexTexCoord, vertexTexCoord2;\n"
"attribute vec3 vertexNormal; attribute vec4 vertexColor;\n"
"uniform mat4 mvp, matModel, matView, matNormal;\n"
"uniform vec4 bcol, fx;\n"           // fx: fullbright, vcolor, valpha, nofog
"uniform vec3 amb;\n"
"uniform int nl;\n"
"uniform vec4 lpos[8], ldir[8], lcol[8];\n" // lpos.w type, ldir.w range, lcol.w cos(outer)
"uniform vec4 tm0, tm1, tm2, tm3;\n"  // su sv u0 v0
"uniform vec4 tc;\n"                  // coord set per layer
"varying vec2 uv0, uv1, uv2, uv3;\n"
"varying vec4 vc; varying float vd;\n"
"vec2 tuv(vec4 m, float s) { vec2 t = s > 0.5 ? vertexTexCoord2 : vertexTexCoord; return t / m.xy + m.zw; }\n"
"void main() {\n"
"  vec4 wp = matModel * vec4(vertexPosition, 1.0);\n"
"  vec3 d = fx.y > 0.5 ? vertexColor.rgb : bcol.rgb;\n"
"  vec3 l = vec3(1.0);\n"
"  if (fx.x < 0.5) {\n"
"    l = amb;\n"
"    vec3 n = (matNormal * vec4(vertexNormal, 0.0)).xyz;\n"
"    if (dot(n, n) > 1e-8) {\n"
"      n = normalize(n);\n"
"      for (int i = 0; i < 8; i++) {\n"
"        if (i >= nl) break;\n"
"        vec3 L = -ldir[i].xyz; float at = 1.0;\n"
"        if (lpos[i].w > 1.5) {\n"
"          vec3 v = lpos[i].xyz - wp.xyz; float dd = length(v);\n"
"          L = v / max(dd, 1e-4); at = 1.0 / (1.0 + dd / ldir[i].w);\n"
"          if (lpos[i].w > 2.5) at *= smoothstep(lcol[i].w, 1.0, dot(-L, ldir[i].xyz));\n"
"        }\n"
"        l += lcol[i].rgb * max(dot(n, L), 0.0) * at;\n"
"      }\n"
"    }\n"
"  }\n"
"  vc = vec4(d * l, bcol.a * (fx.z > 0.5 ? vertexColor.a : 1.0));\n"
"  uv0 = tuv(tm0, tc.x); uv1 = tuv(tm1, tc.y); uv2 = tuv(tm2, tc.z); uv3 = tuv(tm3, tc.w);\n"
"  vd = (matView * wp).z;\n"
"  gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
"}\n";

static const char *shd_fs =
"#version 100\n"
"precision mediump float;\n"
"uniform sampler2D texture0, texture1, texture2, texture3;\n"
"uniform int nt;\n"
"uniform vec4 tb, tmask;\n" // blend + mask per layer
"uniform vec4 fogc;\n"
"uniform float nofog;\n"
"uniform vec3 fogr;\n" // near, far, on
"varying vec2 uv0, uv1, uv2, uv3;\n"
"varying vec4 vc; varying float vd;\n"
"vec4 lay(vec4 c, vec4 t, float b, float m) {\n"
"  if (m > 0.5 && t.a < 0.5) discard;\n"
"  if (b < 0.5) return t;\n"
"  if (b < 1.5) return vec4(mix(c.rgb, t.rgb, t.a), c.a);\n"
"  if (b < 2.5) return c * t;\n"
"  if (b < 3.5) return vec4(c.rgb + t.rgb, c.a);\n"
"  if (b > 4.5) return vec4(c.rgb * t.rgb * 2.0, c.a * t.a);\n"
"  return c * t;\n"
"}\n"
"void main() {\n"
"  vec4 c = vc;\n"
"  if (nt > 0) c = lay(c, texture2D(texture0, uv0), tb.x, tmask.x);\n"
"  if (nt > 1) c = lay(c, texture2D(texture1, uv1), tb.y, tmask.y);\n"
"  if (nt > 2) c = lay(c, texture2D(texture2, uv2), tb.z, tmask.z);\n"
"  if (nt > 3) c = lay(c, texture2D(texture3, uv3), tb.w, tmask.w);\n"
"  if (fogr.z > 0.5 && nofog < 0.5) c.rgb = mix(c.rgb, fogc.rgb, clamp((vd - fogr.x) / max(fogr.y - fogr.x, 1e-4), 0.0, 1.0));\n"
"  gl_FragColor = clamp(c, 0.0, 1.0);\n"
"}\n";
