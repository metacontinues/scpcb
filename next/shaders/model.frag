precision mediump float;
uniform sampler2D texture0;
uniform vec4 fogc;
uniform vec2 fogr; // near, far
varying vec2 uv0;
varying vec4 vc;
varying float vd;

void main() {
  vec4 c = vc * texture2D(texture0, uv0);
  if (c.a < 0.004) discard;
  if (fogr.y > fogr.x)
    c.rgb = mix(c.rgb, fogc.rgb, clamp((vd - fogr.x) / (fogr.y - fogr.x), 0.0, 1.0));
  gl_FragColor = clamp(c, 0.0, 1.0);
}
