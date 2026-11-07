// view: look at a baked room or model. run from the repo root: next/build/view
// [id]   (default gfx/map/room2_opt) rooms: wasd + mouse fly, space/ctrl
// up/down, shift fast. f fog, g ambient, t trigger boxes, l lights, p waypoints
// models: mouse/wheel orbit, space play/pause, , . step a frame, b bones
// env: VIEW_CAM="x y z tx ty tz" start camera, VIEW_SHOT=out.png + VIEW_FRAME=n
// screenshot frame n and quit,
//      VIEW_FOG=0, VIEW_POSE=f pose models at anim frame f, VIEW_YAW=deg orbit
//      angle, VIEW_LOG=1 raylib's log
#include "asset.h"
#include "model.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "room.h"
#include "shd.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static float envf(const char *k, float def) {
  const char *v = getenv(k);
  return v ? (float)atof(v) : def;
}

typedef struct {
  char id[128];
  Mdl m;
  int ok;
} prop;
static prop *props;
static int nprops;
static Mdl *propget(const char *id) {
  for (int i = 0; i < nprops; i++)
    if (!strcmp(props[i].id, id))
      return props[i].ok ? &props[i].m : NULL;
  props = realloc(props, sizeof *props * (nprops + 1));
  prop *p = &props[nprops++];
  snprintf(p->id, sizeof p->id, "%s", id);
  p->ok = mdl_load(&p->m, id);
  if (!p->ok)
    fprintf(stderr, "view: prop %s missing\n", id);
  return p->ok ? &p->m : NULL;
}

int main(int argc, char **argv) {
  const char *id = argc > 1 ? argv[1] : "gfx/map/room2_opt";
  const char *shot = getenv("VIEW_SHOT");
  int shotf = (int)envf("VIEW_FRAME", 3);
  SetTraceLogLevel(getenv("VIEW_LOG") ? LOG_INFO : LOG_WARNING);
#ifdef SCPCB_ANGLE
  // angle's configs all have alpha and no EGL_EXT_present_opaque, and glfw's
  // wayland backend skips those unless the framebuffer is "transparent".
  // angle's swapchain presents opaque anyway
  SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
#endif
  InitWindow(1280, 720, "scpcb view");
  SetTargetFPS(60);
  shd_init();

  int isroom = FileExists(asset_path(id, ".col.glb"));
  RoomMesh r = {0};
  Mdl md = {0};
  if (isroom ? !room_load(&r, id) : !mdl_load(&md, id)) {
    fprintf(stderr, "view: can't load %s\n", asset_path(id, ".*"));
    CloseWindow();
    return 1;
  }
  // blitz's CameraZoom 1 = 90 deg across, raylib wants the vertical fov
  float fovy = 2 * atanf(tanf(45 * DEG2RAD) * 720 / 1280.f) * RAD2DEG;
  Camera3D cam = {{0, 1, 0}, {0, 1, -1}, {0, 1, 0}, fovy, CAMERA_PERSPECTIVE};
  float frame = envf("VIEW_POSE", 0), dist = 3, yaw = envf("VIEW_YAW", 30),
        pitch = 15;
  Vector3 ctr = {0};
  int play = !getenv("VIEW_POSE") && !shot, bones = 0;
  if (isroom) {
    printf("%s: %d meshes, %d mats, %d lights, %d waypoints, %d props, %d "
           "trigs, %d col meshes\n",
           id, r.vis.meshCount, r.vis.materialCount - 1, r.nlights, r.nwps,
           r.nprops, r.ntrigs, r.col.meshCount);
    for (int i = 0; i < r.nprops; i++)
      propget(r.props[i].model);
  } else {
    BoundingBox bb = GetModelBoundingBox(md.m);
    ctr = Vector3Scale(Vector3Add(bb.min, bb.max), .5f);
    dist = Vector3Distance(bb.min, bb.max) * .9f + .01f;
    printf("%s: %d meshes, %d mats, %d bones, %d frames, size %.3f x %.3f x "
           "%.3f\n",
           id, md.m.meshCount, md.m.materialCount - 1, md.m.skeleton.boneCount,
           md.frames, bb.max.x - bb.min.x, bb.max.y - bb.min.y,
           bb.max.z - bb.min.z);
    mdl_pose(&md, frame);
    if (md.m.skeleton.boneCount) // raylib only fills animVertices when it skins
                                 // on the cpu
      printf("skinning: %s\n", md.m.meshes[0].animVertices ? "cpu" : "gpu");
  }
  const char *vc = getenv("VIEW_CAM");
  if (vc)
    sscanf(vc, "%f %f %f %f %f %f", &cam.position.x, &cam.position.y,
           &cam.position.z, &cam.target.x, &cam.target.y, &cam.target.z);
  int fog = envf("VIEW_FOG", 1) != 0, amb = 0, trig = 0, lights = 0, wps = 0;
  if (!shot && isroom)
    DisableCursor();
  // a sun + ambient, like port/'s rmview, so models aren't flat
  Vector4 lp = {-.4f, -.8f, -.45f, 0}, lc = {.6f, .6f, .6f, 0};
  shd_lights((Vector3){.5f, .5f, .5f}, 1, &lp, &lc);

  for (int f = 0; !WindowShouldClose(); f++) {
    float dt = GetFrameTime();
    if (IsKeyPressed(KEY_F))
      fog ^= 1;
    if (IsKeyPressed(KEY_G))
      amb ^= 1;
    if (IsKeyPressed(KEY_T))
      trig ^= 1;
    if (IsKeyPressed(KEY_L))
      lights ^= 1;
    if (IsKeyPressed(KEY_P))
      wps ^= 1;
    if (isroom) {
      if (!shot)
        UpdateCamera(&cam, CAMERA_FREE);
    } else {
      if (IsKeyPressed(KEY_SPACE))
        play ^= 1;
      if (IsKeyPressed(KEY_B))
        bones ^= 1;
      if (IsKeyPressed(KEY_PERIOD))
        frame++;
      if (IsKeyPressed(KEY_COMMA))
        frame--;
      if (play)
        frame += dt * 30;
      if (md.frames) {
        frame = fmodf(frame, md.frames + 1);
        if (frame < 0)
          frame += md.frames + 1;
        mdl_pose(&md, frame);
      }
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 d = GetMouseDelta();
        yaw -= d.x * .3f;
        pitch = Clamp(pitch + d.y * .3f, -89, 89);
      }
      dist *= powf(.9f, GetMouseWheelMove());
      cam.target = ctr;
      cam.position = Vector3Add(
          ctr, (Vector3){dist * cosf(pitch * DEG2RAD) * sinf(yaw * DEG2RAD),
                         dist * sinf(pitch * DEG2RAD),
                         dist * cosf(pitch * DEG2RAD) * cosf(yaw * DEG2RAD)});
    }
    shd_fog(fog && isroom ? 0.5f : 0, fog && isroom ? 6 : 0, BLACK);
    shd_ambient(amb ? 15 / 255.f : 0);

    BeginDrawing();
    ClearBackground(isroom ? BLACK : (Color){20, 20, 28, 255});
    BeginMode3D(cam);
    for (int pass = 0; pass < 2; pass++) {
      if (pass)
        rlDisableDepthMask();
      if (isroom) {
        if (pass)
          BeginBlendMode(BLEND_ALPHA);
        room_draw(&r, MatrixIdentity(), pass);
        if (pass)
          EndBlendMode();
        for (int i = 0; i < r.nprops; i++) {
          Mdl *p = propget(r.props[i].model);
          if (p)
            mdl_draw(p, r.props[i].m, pass);
        }
      } else
        mdl_draw(&md, MatrixIdentity(), pass);
      if (pass)
        rlEnableDepthMask();
    }
    if (trig)
      for (int i = 0; i < r.ntrigs; i++)
        DrawBoundingBox(r.trigs[i].box, YELLOW);
    if (lights)
      for (int i = 0; i < r.nlights; i++) {
        DrawSphere(r.lights[i].p, 0.05f, r.lights[i].c);
        if (r.lights[i].spot)
          DrawLine3D(
              r.lights[i].p,
              Vector3Add(r.lights[i].p, Vector3Scale(r.lights[i].dir, 0.5f)),
              r.lights[i].c);
      }
    if (wps)
      for (int i = 0; i < r.nwps; i++)
        DrawCubeWires(r.wps[i], 0.1f, 0.1f, 0.1f, GREEN);
    if (bones && md.m.skeleton.boneCount) {
      rlDisableDepthTest();
      for (unsigned i = 0; i < md.m.skeleton.boneCount; i++) {
        Vector3 a = md.m.currentPose[i].translation;
        int p = md.m.skeleton.bones[i].parent;
        DrawSphere(a, dist * .004f, YELLOW);
        if (p >= 0)
          DrawLine3D(a, md.m.currentPose[p].translation, ORANGE);
      }
      rlEnableDepthTest();
    }
    EndMode3D();
    if (!shot)
      DrawText(isroom
                   ? TextFormat("%s  %d fps  fog %d amb %d  (%.2f %.2f %.2f)",
                                id, GetFPS(), fog, amb, cam.position.x,
                                cam.position.y, cam.position.z)
                   : TextFormat("%s  %d fps  frame %.1f / %d", id, GetFPS(),
                                frame, md.frames),
               10, 10, 20, RAYWHITE);
    EndDrawing();
    if (shot && f == shotf) {
      TakeScreenshot(shot);
      break;
    }
  }
  for (int i = 0; i < nprops; i++)
    if (props[i].ok)
      mdl_free(&props[i].m);
  if (isroom)
    room_free(&r);
  else
    mdl_free(&md);
  tex_clear();
  CloseWindow();
  return 0;
}
