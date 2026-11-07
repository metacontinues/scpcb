// models: per-vertex lighting like blitz's fixed function, base (brush or
// vertex color) * (ambient + lights). shd.c prepends #version + FULLBRIGHT,
// VCOL, SKIN
attribute vec3 vertexPosition, vertexNormal;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;
#if SKIN
attribute vec4 vertexBoneIndices, vertexBoneWeights;
uniform mat4 boneMatrices[128];
#endif
uniform mat4 mvp, matModel, matView, matNormal;
uniform vec4 colDiffuse;
uniform vec3 lamb;
uniform int nl;
uniform vec4 lpos[4], lcol[4]; // lpos.w: 1 point, 0 directional. lcol.w: range
varying vec2 uv0;
varying vec4 vc;
varying float vd;

void main() {
  vec4 lp = vec4(vertexPosition, 1.0), ln = vec4(vertexNormal, 0.0);
#if SKIN
  mat4 sk = boneMatrices[int(vertexBoneIndices.x)] * vertexBoneWeights.x +
            boneMatrices[int(vertexBoneIndices.y)] * vertexBoneWeights.y +
            boneMatrices[int(vertexBoneIndices.z)] * vertexBoneWeights.z +
            boneMatrices[int(vertexBoneIndices.w)] * vertexBoneWeights.w;
  lp = sk * lp;
  ln = sk * ln;
#endif
  vec4 wp = matModel * lp;
#if VCOL
  vec4 base = vec4(vertexColor.rgb, colDiffuse.a * vertexColor.a);
#else
  vec4 base = colDiffuse;
#endif
#if FULLBRIGHT
  vec3 l = vec3(1.0);
#else
  vec3 l = lamb, n = normalize((matNormal * ln).xyz);
  for (int i = 0; i < 4; i++) {
    if (i >= nl) break;
    vec3 L = -lpos[i].xyz;
    float at = 1.0;
    if (lpos[i].w > 0.5) {
      vec3 v = lpos[i].xyz - wp.xyz;
      float d = length(v);
      L = v / max(d, 1e-4);
      at = 1.0 / (1.0 + d / lcol[i].w);
    }
    l += lcol[i].rgb * max(dot(n, normalize(L)), 0.0) * at;
  }
#endif
  vc = vec4(base.rgb * l, base.a);
  uv0 = vertexTexCoord;
  vd = -(matView * wp).z;
  gl_Position = mvp * lp;
}
