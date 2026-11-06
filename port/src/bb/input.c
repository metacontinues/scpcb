// input.c: blitz keyboard/mouse. keys are directinput scancodes (options.ini binds use them).
// hits/speeds accumulate per frame in input_update (from bbFlip) and reset when read, like blitz.
#include <stdlib.h>
#include <string.h>
#include "in.h"

static const short sc2rl[256] = {
	[1] = KEY_ESCAPE, [2] = KEY_ONE, [3] = KEY_TWO, [4] = KEY_THREE, [5] = KEY_FOUR, [6] = KEY_FIVE,
	[7] = KEY_SIX, [8] = KEY_SEVEN, [9] = KEY_EIGHT, [10] = KEY_NINE, [11] = KEY_ZERO, [12] = KEY_MINUS,
	[13] = KEY_EQUAL, [14] = KEY_BACKSPACE, [15] = KEY_TAB, [16] = KEY_Q, [17] = KEY_W, [18] = KEY_E,
	[19] = KEY_R, [20] = KEY_T, [21] = KEY_Y, [22] = KEY_U, [23] = KEY_I, [24] = KEY_O, [25] = KEY_P,
	[26] = KEY_LEFT_BRACKET, [27] = KEY_RIGHT_BRACKET, [28] = KEY_ENTER, [29] = KEY_LEFT_CONTROL,
	[30] = KEY_A, [31] = KEY_S, [32] = KEY_D, [33] = KEY_F, [34] = KEY_G, [35] = KEY_H, [36] = KEY_J,
	[37] = KEY_K, [38] = KEY_L, [39] = KEY_SEMICOLON, [40] = KEY_APOSTROPHE, [41] = KEY_GRAVE,
	[42] = KEY_LEFT_SHIFT, [43] = KEY_BACKSLASH, [44] = KEY_Z, [45] = KEY_X, [46] = KEY_C, [47] = KEY_V,
	[48] = KEY_B, [49] = KEY_N, [50] = KEY_M, [51] = KEY_COMMA, [52] = KEY_PERIOD, [53] = KEY_SLASH,
	[54] = KEY_RIGHT_SHIFT, [55] = KEY_KP_MULTIPLY, [56] = KEY_LEFT_ALT, [57] = KEY_SPACE, [58] = KEY_CAPS_LOCK,
	[59] = KEY_F1, [60] = KEY_F2, [61] = KEY_F3, [62] = KEY_F4, [63] = KEY_F5, [64] = KEY_F6, [65] = KEY_F7,
	[66] = KEY_F8, [67] = KEY_F9, [68] = KEY_F10, [69] = KEY_NUM_LOCK, [70] = KEY_SCROLL_LOCK,
	[71] = KEY_KP_7, [72] = KEY_KP_8, [73] = KEY_KP_9, [74] = KEY_KP_SUBTRACT, [75] = KEY_KP_4, [76] = KEY_KP_5,
	[77] = KEY_KP_6, [78] = KEY_KP_ADD, [79] = KEY_KP_1, [80] = KEY_KP_2, [81] = KEY_KP_3, [82] = KEY_KP_0,
	[83] = KEY_KP_DECIMAL, [87] = KEY_F11, [88] = KEY_F12, [156] = KEY_KP_ENTER, [157] = KEY_RIGHT_CONTROL,
	[181] = KEY_KP_DIVIDE, [183] = KEY_PRINT_SCREEN, [184] = KEY_RIGHT_ALT, [197] = KEY_PAUSE, [199] = KEY_HOME,
	[200] = KEY_UP, [201] = KEY_PAGE_UP, [203] = KEY_LEFT, [205] = KEY_RIGHT, [207] = KEY_END, [208] = KEY_DOWN,
	[209] = KEY_PAGE_DOWN, [210] = KEY_INSERT, [211] = KEY_DELETE, [219] = KEY_LEFT_SUPER, [220] = KEY_RIGHT_SUPER,
	[221] = KEY_KB_MENU,
};
static int khit[256], mhit[4], keyq[64], nkq;
static float mdx, mdy, mdz;
static void push(int c) { if (nkq < 64) keyq[nkq++] = c; }

// dev hook: BB_INPUT="frame:ev ev,frame:ev" plays input at flip n (counted like BB_SHOT_FRAME).
// ev: m<x>x<y> move the mouse (virtual, the real pointer isn't touched), l left click,
//     k<scancode> key hit, h<scancode> key held this flip, c<ascii> typed char. eg BB_INPUT="12:c13,20:m300x400 l"
static int sx = -1, sy, sdown, sframe, shold[256];
static void script(void) {
	static const char *p; static int init;
	if (!init) { init = 1; p = getenv("BB_INPUT"); }
	sframe++; sdown = 0; memset(shold, 0, sizeof shold);
	for (const char *c = p; c && *c;) {
		int f = atoi(c);
		const char *e = strchr(c, ':'), *nx = strchr(c, ',');
		if (!e || (nx && e > nx)) break;
		for (e++; *e && *e != ','; ) {
			if (f == sframe) switch (*e) {
				case 'm': sx = atoi(e + 1); sy = atoi(strchr(e, 'x') ? strchr(e, 'x') + 1 : e + 1); break;
				case 'l': mhit[1]++; sdown = 1; break;
				case 'k': { int k = atoi(e + 1); if (k > 0 && k < 256) khit[k]++; } break;
				case 'c': push(atoi(e + 1)); break;
				case 'h': { int k = atoi(e + 1); if (k > 0 && k < 256) shold[k] = 1; } break;
			}
			while (*e && *e != ' ' && *e != ',') e++;
			while (*e == ' ') e++;
		}
		c = nx ? nx + 1 : NULL;
	}
}

static int rl(int sc) { return sc > 0 && sc < 256 ? sc2rl[sc] : 0; }
int bbKeyDown(int sc) { return (sc > 0 && sc < 256 && shold[sc]) || (rl(sc) && IsKeyDown(rl(sc))); }
int bbKeyHit(int sc) { if (sc <= 0 || sc >= 256) return 0; int n = khit[sc]; khit[sc] = 0; return n; }
void bbFlushKeys(void) { memset(khit, 0, sizeof khit); nkq = 0; }
int bbGetKey(void) { // ascii of next typed key, 0 if none
	if (!nkq) return 0;
	int c = keyq[0];
	memmove(keyq, keyq + 1, --nkq * sizeof *keyq);
	return c;
}
int bbWaitKey(void) { int c; while (!(c = bbGetKey()) && !WindowShouldClose()) { PollInputEvents(); input_update(); WaitTime(0.01); } return c; }

static int mb(int b) { return b == 1 ? MOUSE_BUTTON_LEFT : b == 2 ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_MIDDLE; }
int bbMouseDown(int b) { return (b == 1 && sdown) || (b >= 1 && b <= 3 && IsMouseButtonDown(mb(b))); }
int bbMouseHit(int b) { if (b < 1 || b > 3) return 0; int n = mhit[b]; mhit[b] = 0; return n; }
int bbMouseX(void) { return sx >= 0 ? sx : GetMouseX(); }
int bbMouseY(void) { return sx >= 0 ? sy : GetMouseY(); }
int bbMouseXSpeed(void) { int r = mdx; mdx -= r; return r; }
int bbMouseYSpeed(void) { int r = mdy; mdy -= r; return r; }
int bbMouseZSpeed(void) { int r = mdz; mdz -= r; return r; }
void bbFlushMouse(void) { memset(mhit, 0, sizeof mhit); mdx = mdy = mdz = 0; }

void input_update(void) {
	for (int sc = 1; sc < 256; sc++) if (sc2rl[sc] && IsKeyPressed(sc2rl[sc])) khit[sc]++;
	for (int b = 1; b <= 3; b++) if (IsMouseButtonPressed(mb(b))) mhit[b]++;
	Vector2 d = GetMouseDelta();
	mdx += d.x; mdy += d.y; mdz += GetMouseWheelMove();
	// GetKey wants control chars too
	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) push(8);
	if (IsKeyPressed(KEY_TAB)) push(9);
	if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) push(13);
	if (IsKeyPressed(KEY_ESCAPE)) push(27);
	for (int c; (c = GetCharPressed());) push(c < 256 ? c : '?'); // blitz is ascii/latin1
	script();
}
