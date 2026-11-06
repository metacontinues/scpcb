// main.c: Main.bb's top level, in order. the blitz file runs its includes' globals inline,
// so the Init* calls sit where the Includes were.
#define G_DEFINE
#include "g.h"

TLIST(ConsoleMsg);

_Noreturn void RuntimeError(const char *msg) { fprintf(stderr, "runtime error: %s\n", msg); exit(1); }
int MilliSecs2(void) { int r = bbMilliSecs(); return r < 0 ? r + 2147483647 + 1 : r; }

void CreateConsoleMsg(const char *txt, int r, int g, int b, int isCommand) {
	ConsoleMsg *c = bbNew(ConsoleMsg);
	bbInsertBefore(c, bbFirst(ConsoleMsg));
	c->txt = bbSdup(txt); c->isCommand = isCommand;
	c->r = r < 0 ? ConsoleR : r; c->g = g < 0 ? ConsoleG : g; c->b = b < 0 ? ConsoleB : b;
}

int MouseOn(int x, int y, int w, int h) {
	int mx = ScaledMouseX(), my = ScaledMouseY();
	return mx > x && mx < x + w && my > y && my < y + h;
}
int ScaledMouseX(void) {
	return bbInt((bbMouseX() - RealGraphicWidth * .5f * (1 - AspectRatioRatio)) * (float)GraphicWidth / (RealGraphicWidth * AspectRatioRatio));
}
int ScaledMouseY(void) { return bbInt(bbMouseY() * (float)GraphicHeight / RealGraphicHeight); }

void UpdateMusic(void) {} // TODO Main.bb music player

// TODO InitFastResize: the fresize_* camera/sprite are only for borderless upscaling + gamma
void Graphics3DExt(int w, int h, int depth, int mode) {
	bbGraphics3D(w, h, depth, mode);
	bbAntiAlias(GetINIInt(OptionFile, "options", "antialias", 0));
}
// blitz version rendered the image scaled through a camera and grabbed it back
int ResizeImage2(int img, int w, int h) { bbResizeImage(img, w, h); return img; }

// the gfx mode list w/o duplicates, shared by the no-launcher path and UpdateLauncher
void CollectGfxModes(void) {
	for (int i = 1; i <= TotalGFXModes; i++) {
		int same = 0;
		for (int n = 0; n < TotalGFXModes; n++) if (GfxModeWidths[n] == bbGfxModeWidth(i) && GfxModeHeights[n] == bbGfxModeHeight(i)) { same = 1; break; }
		if (same) continue;
		if (GraphicWidth == bbGfxModeWidth(i) && GraphicHeight == bbGfxModeHeight(i)) SelectedGFXMode = GFXModes;
		GfxModeWidths[GFXModes] = bbGfxModeWidth(i);
		GfxModeHeights[GFXModes] = bbGfxModeHeight(i);
		GFXModes++;
	}
}

// TODO borderless windowed: blitz made a desktop-sized popup window and upscaled into it.
// here it's a plain window at the chosen res until fresize is ported
static void OpenGame(void) {
	AspectRatioRatio = 1;
	RealGraphicWidth = GraphicWidth; RealGraphicHeight = GraphicHeight;
	if (BorderlessWindowed) Fullscreen = 0;
	Graphics3DExt(GraphicWidth, GraphicHeight, Fullscreen ? 16 * Bit16Mode : 0, Fullscreen ? 1 : 2);
}

static void InitOptions(void) {
	VersionNumber = "1.3.11"; CompatibleNumber = "1.3.11";
	EnableSFXRelease = GetINIInt(OptionFile, "audio", "sfx release", 0);
	EnableSFXRelease_Prev = EnableSFXRelease;
	CanOpenConsole = GetINIInt(OptionFile, "console", "enabled", 0);
	LauncherWidth = Min(GetINIInt(OptionFile, "launcher", "launcher width", 0), 1024);
	LauncherHeight = Min(GetINIInt(OptionFile, "launcher", "launcher height", 0), 768);
	LauncherEnabled = GetINIInt(OptionFile, "launcher", "launcher enabled", 0);
	GraphicWidth = GetINIInt(OptionFile, "options", "width", 0);
	GraphicHeight = GetINIInt(OptionFile, "options", "height", 0);
	Depth = 0; Fullscreen = GetINIInt(OptionFile, "options", "fullscreen", 0);
	SelectedGFXDriver = Max(GetINIInt(OptionFile, "options", "gfx driver", 0), 1);
	ShowFPS = GetINIInt(OptionFile, "options", "show FPS", 0);
	TotalGFXModes = bbCountGfxModes3D();
	GfxModeWidths = calloc(TotalGFXModes + 1, sizeof(int));
	GfxModeHeights = calloc(TotalGFXModes + 1, sizeof(int));
	BorderlessWindowed = GetINIInt(OptionFile, "options", "borderless windowed", 0);
	EnableRoomLights = GetINIInt(OptionFile, "options", "room lights enabled", 0);
	TextureDetails = GetINIInt(OptionFile, "options", "texture details", 0);
	static const float tf[] = {.8f, .4f, 0, -.4f, -.8f};
	if (TextureDetails >= 0 && TextureDetails <= 4) TextureFloat = tf[TextureDetails];
	ConsoleOpening = GetINIInt(OptionFile, "console", "auto opening", 0);
	SFXVolume = GetINIFloat(OptionFile, "audio", "sound volume", 0);
	Bit16Mode = GetINIInt(OptionFile, "options", "16bit", 0);
	AATextEnable = GetINIInt(OptionFile, "options", "antialiased text", 0);
}

static void InitPlayerGlobals(void) {
	viewport_center_x = GraphicWidth / 2; viewport_center_y = GraphicHeight / 2;
	mouselook_x_inc = mouselook_y_inc = .3f;
	mouse_left_limit = 250; mouse_right_limit = bbGraphicsWidth() - 250;
	mouse_top_limit = 150; mouse_bottom_limit = bbGraphicsHeight() - 150;
	KEY_RIGHT = GetINIInt(OptionFile, "binds", "Right key", 0);
	KEY_LEFT = GetINIInt(OptionFile, "binds", "Left key", 0);
	KEY_UP = GetINIInt(OptionFile, "binds", "Up key", 0);
	KEY_DOWN = GetINIInt(OptionFile, "binds", "Down key", 0);
	KEY_BLINK = GetINIInt(OptionFile, "binds", "Blink key", 0);
	KEY_SPRINT = GetINIInt(OptionFile, "binds", "Sprint key", 0);
	KEY_INV = GetINIInt(OptionFile, "binds", "Inventory key", 0);
	KEY_CROUCH = GetINIInt(OptionFile, "binds", "Crouch key", 0);
	KEY_SAVE = GetINIInt(OptionFile, "binds", "Save key", 0);
	KEY_CONSOLE = GetINIInt(OptionFile, "binds", "Console key", 0);
	MouseSmooth = GetINIFloat(OptionFile, "options", "mouse smoothing", 1);
	Playable = 1; BlinkEffect = 1; StaminaEffect = 1;
	InvertMouse = GetINIInt(OptionFile, "options", "invert mouse y", 0);
	NoClipSpeed = 2; CoffinDistance = 100; LightsOn = 1;
	ConsoleR = ConsoleG = ConsoleB = 255;
}

int main(void) {
	// dll checks + FSOUND_Init: nothing to do, raylib audio comes up w/ the window
	InitKeyName();
	OptionFile = "options.ini";
	UpdateCheckEnabled = GetINIInt(OptionFile, "options", "check for updates", 0); // Update.bb
	ErrorFile = "error_log_";
	int ei = 0;
	while (bbFileType(cat(ErrorFile, bbStr(ei), ".txt"))) ei++;
	ErrorFile = cat(ErrorFile, bbStr(ei), ".txt");
	InitOptions();

	if (LauncherEnabled) UpdateLauncher();
	else {
		CollectGfxModes();
		GraphicWidth = GfxModeWidths[SelectedGFXMode];
		GraphicHeight = GfxModeHeights[SelectedGFXMode];
	}
	OpenGame();

	MenuScale = GraphicHeight / 1024.0f;
	bbSetBuffer(bbBackBuffer());
	Framelimit = GetINIInt(OptionFile, "options", "framelimit", 0);
	Vsync = GetINIInt(OptionFile, "options", "vsync", 0);
	Opt_AntiAlias = GetINIInt(OptionFile, "options", "antialias", 0);
	CurrFrameLimit = (Framelimit - 19) / 100.0f;
	ScreenGamma = GetINIFloat(OptionFile, "options", "screengamma", 0);
	bbSeedRnd(bbMilliSecs());
	CanSave = 1;
	bbAppTitle(cat("SCP - Containment Breach v", VersionNumber));
	// TODO PlayStartupVideos: needs a video decoder (BlitzMovie)

	CursorIMG = LoadImage_Strict("GFX\\cursor.png");
	LoadingBack = LoadImage_Strict("Loadingscreens\\loadingback.jpg");
	InitLoadingScreens("Loadingscreens\\loadingscreens.ini");
	InitAAFont();
	float k = GraphicHeight / 1024.0f;
	Font1 = AALoadFont("GFX\\font\\cour\\Courier New.ttf", bbInt(19 * k), 0, 0, 0);
	Font2 = AALoadFont("GFX\\font\\courbd\\Courier New.ttf", bbInt(58 * k), 0, 0, 0);
	Font3 = AALoadFont("GFX\\font\\DS-DIGI\\DS-Digital.ttf", bbInt(22 * k), 0, 0, 0);
	Font4 = AALoadFont("GFX\\font\\DS-DIGI\\DS-Digital.ttf", bbInt(60 * k), 0, 0, 0);
	Font5 = AALoadFont("GFX\\font\\Journal\\Journal.ttf", bbInt(58 * k), 0, 0, 0);
	ConsoleFont = AALoadFont("Blitz", bbInt(20 * k), 0, 0, 0);
	AASetFont(Font2);
	BlinkMeterIMG = LoadImage_Strict("GFX\\blinkmeter.jpg");
	DrawLoading(0, 1);

	InitPlayerGlobals();
	InitAchievements();
	InitDifficulties();

	// TODO: the rest of Main.bb. for now park on a 100% loading screen
	DrawLoading(100, 1);
	return 0;
}
