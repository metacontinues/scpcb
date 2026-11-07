// player.c: walking, falling, looking. the original's MovePlayer/MouseLook
// minus everything that needs the rest of the game (injuries, items, sounds,
// blinking)
#include "player.h"
#include "coll.h"
#include "raylib.h"
#include "raymath.h"
#include "util.h"
#include <math.h>

static const Vector3 RADII = {.15f, .30f, .15f};

void player_spawn(Player *p, Vector3 pos, float yaw) {
  float sens = p->sens, smooth = p->smooth;
  int inv = p->invert;
  float f = frnd(455, 665); // normal difficulty's blink interval
  *p = (Player){
      .pos = pos, .yaw = yaw, .stamina = 100, .blink = f, .blinkfreq = f};
  p->sens = sens;
  p->smooth = smooth > 0 ? smooth : 1;
  p->invert = inv;
}

Vector3 player_forward(const Player *p) {
  float y = p->yaw * DEG2RAD, x = p->pitch * DEG2RAD;
  return (Vector3){-sinf(y) * cosf(x), -sinf(x), -cosf(y) * cosf(x)};
}

Vector3 player_eye(const Player *p) {
  float up = sinf(p->shake * DEG2RAD) / (20 + p->crouchstate * 20) * .6f;
  return Vector3Add(p->pos, (Vector3){0, up + .6f - p->crouchstate * .3f, 0});
}

Camera3D player_camera(const Player *p, float fovy) {
  Vector3 e = player_eye(p);
  return (Camera3D){
      e, Vector3Add(e, player_forward(p)), {0, 1, 0}, fovy, CAMERA_PERSPECTIVE};
}

static void look(Player *p, Vector2 m) {
  float s = p->sens + .6f, sm = 6 / (p->sens + 1) * p->smooth;
  p->msx = curve(m.x * s, p->msx, sm);
  p->msy = curve((p->invert ? -m.y : m.y) * s, p->msy, sm);
  p->yaw = wrapdeg(p->yaw - p->msx * .3f);
  p->pitch = clampf(p->pitch + p->msy * .3f, -70, 70);
}

// eyes: the timer runs down at .6/tick, at 0 you blink for 20 ticks (shut for
// 10 of them), then a new random interval. the key blinks early / keeps them
// shut
static void blink(Player *p, PlayerIn in) {
  if (in.blink)
    p->blink = 0;
  if (in.blinkheld && p->blink < -10)
    p->blink = -10;
  if (p->blink >= 0) {
    p->blink -= .6f;
    return;
  }
  if (p->blink <= -20)
    p->blink = p->blinkfreq = frnd(455, 665);
  p->blink -= 1;
}

float player_dark(const Player *p) {
  float b = p->blink;
  if (b >= 0)
    return 0;
  if (b > -5)
    return sinf(fabsf(b * 18) * DEG2RAD);
  if (b > -15)
    return 1;
  return fabsf(sinf(b * 18 * DEG2RAD));
}

void player_tick(Player *p, PlayerIn in) {
  p->step = p->landed = 0;
  look(p, in.mouse);
  blink(p, in);
  p->stamina =
      minf(p->stamina + .15f * (p->speed > 0 ? 1 / 1.25f : 1.25f), 100);
  if (in.crouch)
    p->crouch = !p->crouch;
  p->crouchstate = fabsf(p->crouchstate - p->crouch) < .001f
                       ? p->crouch
                       : curve(p->crouch, p->crouchstate, 10);

  int moving = in.up || in.down || in.left || in.right;
  float sprint = 1, angle = 0;
  if (moving) {
    if (!p->crouch && in.sprint && p->stamina > 0) {
      sprint = 2.5f;
      p->stamina -= .4f;
      if (p->stamina <= 0)
        p->stamina = -20;
    }
    float was = fmodf(p->shake, 360);
    p->shake = fmodf(p->shake + minf(sprint, 1.5f) * 7, 720);
    if (was < 180 && fmodf(p->shake, 360) >= 180) // the head bob's low point
      p->step = sprint != 1 ? 2 : 1;
  }
  // which way, relative to where you look: 0 ahead, 90 left, 180 back
  if (in.down)
    angle = in.left ? 135 : in.right ? -135 : 180;
  else if (in.up)
    angle = in.left ? 45 : in.right ? -45 : 0;
  else if (in.left)
    angle = 90;
  else if (in.right)
    angle = -90;
  float want = .018f * sprint / (1 + p->crouchstate);
  p->speed = moving ? curve(want, p->speed, 20) : 0;

  float a = (p->yaw + angle) * DEG2RAD;
  Vector3 v = {-sinf(a) * p->speed, 0, -cosf(a) * p->speed};
  // gravity, but only w/ ground somewhere below (the original's falling pick)
  if (!p->floor &&
      coll_ray(p->pos, Vector3Add(p->pos, (Vector3){0, -10, 0})).hit)
    p->drop = clampf(p->drop - .006f, -2, 0);
  else {
    p->landed = p->floor && p->drop < -.07f;
    p->drop = 0;
  }
  v.y = p->drop;
  p->floor = 0;
  p->pos = coll_move(p->pos, v, RADII, p->pos.y - .25f, &p->floor);
}

// options.ini stores keys as directinput scancodes
int key_dik(int k) {
  static const int t[256] = {[1] = KEY_ESCAPE,
                             [2] = KEY_ONE,
                             [3] = KEY_TWO,
                             [4] = KEY_THREE,
                             [5] = KEY_FOUR,
                             [6] = KEY_FIVE,
                             [7] = KEY_SIX,
                             [8] = KEY_SEVEN,
                             [9] = KEY_EIGHT,
                             [10] = KEY_NINE,
                             [11] = KEY_ZERO,
                             [12] = KEY_MINUS,
                             [13] = KEY_EQUAL,
                             [14] = KEY_BACKSPACE,
                             [15] = KEY_TAB,
                             [16] = KEY_Q,
                             [17] = KEY_W,
                             [18] = KEY_E,
                             [19] = KEY_R,
                             [20] = KEY_T,
                             [21] = KEY_Y,
                             [22] = KEY_U,
                             [23] = KEY_I,
                             [24] = KEY_O,
                             [25] = KEY_P,
                             [26] = KEY_LEFT_BRACKET,
                             [27] = KEY_RIGHT_BRACKET,
                             [28] = KEY_ENTER,
                             [29] = KEY_LEFT_CONTROL,
                             [30] = KEY_A,
                             [31] = KEY_S,
                             [32] = KEY_D,
                             [33] = KEY_F,
                             [34] = KEY_G,
                             [35] = KEY_H,
                             [36] = KEY_J,
                             [37] = KEY_K,
                             [38] = KEY_L,
                             [39] = KEY_SEMICOLON,
                             [40] = KEY_APOSTROPHE,
                             [41] = KEY_GRAVE,
                             [42] = KEY_LEFT_SHIFT,
                             [43] = KEY_BACKSLASH,
                             [44] = KEY_Z,
                             [45] = KEY_X,
                             [46] = KEY_C,
                             [47] = KEY_V,
                             [48] = KEY_B,
                             [49] = KEY_N,
                             [50] = KEY_M,
                             [51] = KEY_COMMA,
                             [52] = KEY_PERIOD,
                             [53] = KEY_SLASH,
                             [54] = KEY_RIGHT_SHIFT,
                             [55] = KEY_KP_MULTIPLY,
                             [56] = KEY_LEFT_ALT,
                             [57] = KEY_SPACE,
                             [58] = KEY_CAPS_LOCK,
                             [59] = KEY_F1,
                             [60] = KEY_F2,
                             [61] = KEY_F3,
                             [62] = KEY_F4,
                             [63] = KEY_F5,
                             [64] = KEY_F6,
                             [65] = KEY_F7,
                             [66] = KEY_F8,
                             [67] = KEY_F9,
                             [68] = KEY_F10,
                             [87] = KEY_F11,
                             [88] = KEY_F12,
                             [157] = KEY_RIGHT_CONTROL,
                             [184] = KEY_RIGHT_ALT,
                             [199] = KEY_HOME,
                             [200] = KEY_UP,
                             [201] = KEY_PAGE_UP,
                             [203] = KEY_LEFT,
                             [205] = KEY_RIGHT,
                             [207] = KEY_END,
                             [208] = KEY_DOWN,
                             [209] = KEY_PAGE_DOWN,
                             [210] = KEY_INSERT,
                             [211] = KEY_DELETE};
  return k >= 0 && k < 256 ? t[k] : 0;
}
