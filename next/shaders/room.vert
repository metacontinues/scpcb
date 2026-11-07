// rooms: diffuse (uv0) x lightmap (uv1) or vertex light. shd.c prepends
// #version + KIND
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord, vertexTexCoord2;
attribute vec4 vertexColor;
uniform mat4 mvp, matModel, matView;
varying vec2 uv0, uv1;
varying vec4 vc;
varying float vd;

void main() {
  uv0 = vertexTexCoord;
  uv1 = vertexTexCoord2;
  vc = vertexColor;
  vd = -(matView * matModel * vec4(vertexPosition, 1.0)).z;
  gl_Position = mvp * vec4(vertexPosition, 1.0);
}
