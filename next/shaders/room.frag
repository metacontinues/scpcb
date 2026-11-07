// KIND (MK_*): 0 lightmapped, 1 vertex lit, 2 vertex color only, 3 alpha
precision mediump float;
uniform sampler2D texture0, lightmap;
uniform vec4 colDiffuse;
uniform float amb;
uniform vec4 fogc;
uniform vec2 fogr; // near, far
varying vec2 uv0, uv1;
varying vec4 vc;
varying float vd;

void main() {
  vec4 d = texture2D(texture0, uv0) * colDiffuse;
#if KIND == 0
  vec4 c = vec4((vc.rgb * amb * 2.0 + texture2D(lightmap, uv1).rgb) * d.rgb * 2.0, 1.0);
#elif KIND == 1
  vec4 c = vec4(vc.rgb * d.rgb * 2.0, 1.0);
#elif KIND == 2
  vec4 c = vec4(vc.rgb, 1.0);
#else
  vec4 c = vc * d;
  if (c.a < 0.004) discard;
#endif
  if (fogr.y > fogr.x)
    c.rgb = mix(c.rgb, fogc.rgb, clamp((vd - fogr.x) / (fogr.y - fogr.x), 0.0, 1.0));
  gl_FragColor = clamp(c, 0.0, 1.0);
}
