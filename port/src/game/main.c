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
	InitLateGlobals();
	MainLoop();
	return 0;
}

int GenerateSeedNumber(const char *seed) {
	int t = 0, sh = 0;
	for (int i = 1; i <= bbLen(seed); i++) { t ^= (unsigned)bbAsc(bbMid(seed, i, 1)) << sh; sh = (sh + 1) % 24; }
	return t;
}

void SaveOptionsINI(void) {
#define P(sec, k, v) PutINIValue(OptionFile, sec, k, bbStr(v))
	P("options", "mouse sensitivity", MouseSens); P("options", "invert mouse y", InvertMouse);
	P("options", "bump mapping enabled", BumpEnabled); P("options", "HUD enabled", HUDenabled);
	P("options", "screengamma", ScreenGamma); P("options", "antialias", Opt_AntiAlias);
	P("options", "vsync", Vsync); P("options", "show FPS", ShowFPS); P("options", "framelimit", Framelimit);
	P("options", "achievement popup enabled", AchvMSGenabled); P("options", "room lights enabled", EnableRoomLights);
	P("options", "texture details", TextureDetails); P("console", "enabled", CanOpenConsole);
	P("console", "auto opening", ConsoleOpening); P("options", "antialiased text", AATextEnable);
	P("options", "particle amount", ParticleAmount); P("options", "enable vram", EnableVRam);
	P("options", "mouse smoothing", MouseSmooth);
	P("audio", "music volume", MusicVolume); P("audio", "sound volume", PrevSFXVolume);
	P("audio", "sfx release", EnableSFXRelease); P("audio", "enable user tracks", EnableUserTracks);
	P("audio", "user track setting", UserTrackMode);
	P("binds", "Right key", KEY_RIGHT); P("binds", "Left key", KEY_LEFT); P("binds", "Up key", KEY_UP);
	P("binds", "Down key", KEY_DOWN); P("binds", "Blink key", KEY_BLINK); P("binds", "Sprint key", KEY_SPRINT);
	P("binds", "Inventory key", KEY_INV); P("binds", "Crouch key", KEY_CROUCH); P("binds", "Save key", KEY_SAVE);
	P("binds", "Console key", KEY_CONSOLE);
#undef P
}

// Main.bb 1525-2809: what runs between the first loading screen and the main loop
void InitLateGlobals(void) {
	BumpEnabled = GetINIInt("options.ini", "options", "bump mapping enabled", 0);
	HUDenabled = GetINIInt("options.ini", "options", "HUD enabled", 0);
	Brightness = bbInt(GetINIFloat("options.ini", "options", "brightness", 0));
	CameraFogNear = GetINIFloat("options.ini", "options", "camera fog near", 0);
	CameraFogFar = GetINIFloat("options.ini", "options", "camera fog far", 0);
	StoredCameraFogFar = CameraFogFar;
	MouseSens = GetINIFloat("options.ini", "options", "mouse sensitivity", 0);
	EnableVRam = GetINIInt("options.ini", "options", "enable vram", 0);
	// TODO Dreamfilter.bb
	InitMusic();
	DrawLoading(10, 1);
	DrawLoading(20, 1);
	DrawLoading(25, 1);
	DrawLoading(30, 1);
	NVGImages = bbLoadAnimImage("GFX\\battery.png", 64, 64, 0, 2);
	bbMaskImage(NVGImages, 255, 0, 255);
	EnableUserTracks = GetINIInt(OptionFile, "audio", "enable user tracks", 0);
	UserTrackMode = GetINIInt(OptionFile, "audio", "user track setting", 0);
	InFacility = 1;
	PrevMusicVolume = MusicVolume; PrevSFXVolume = SFXVolume;
	menuroomscale = 8.0f / 2048.0f;
	CurrMenu_TestIMG = "";
	ParticleAmount = GetINIInt(OptionFile, "options", "particle amount", 0);
	for (int i = 0; i < 4; i++) {
		NavImages[i] = LoadImage_Strict(cat("GFX\\navigator\\roomborder", bbStr(i), ".png"));
		bbMaskImage(NavImages[i], 255, 0, 255);
	}
	NavImages[4] = LoadImage_Strict("GFX\\navigator\\batterymeter.png");
	NavBG = bbCreateImage(GraphicWidth, GraphicHeight, 1);
	DrawLoading(35, 1);
	// TODO Items.bb, Particles.bb (HissSFX), doors, MapSystem.bb (LoadRoomTemplates), NPCs.bb, UpdateEvents.bb
	DrawLoading(40, 1);
	DrawLoading(80, 1);
	bbCollisions(HIT_PLAYER, HIT_MAP, 2, 2);
	bbCollisions(HIT_PLAYER, HIT_PLAYER, 1, 3);
	bbCollisions(HIT_ITEM, HIT_MAP, 2, 2);
	bbCollisions(HIT_APACHE, HIT_APACHE, 1, 2);
	bbCollisions(HIT_178, HIT_MAP, 2, 2);
	bbCollisions(HIT_178, HIT_178, 1, 3);
	bbCollisions(HIT_DEAD, HIT_MAP, 2, 2);
	DrawLoading(90, 1);
	ShouldEntitiesFall = 1; PlayerFallingPickDistance = 10;
	Save_MSG = "";
	InitMenu();
	MainMenuOpen = 1;
	bbFlushKeys(); bbFlushMouse();
	DrawLoading(100, 1);
	LoopDelay = bbMilliSecs();
}

// TODO LoadEntities/LoadAllSounds/InitNewGame (new) and LoadGame/InitLoadGame (load)
int StartGame(const char *how) {
	fprintf(stderr, "scpcb: can't start a game yet (%s), the game itself isn't ported\n", how);
	return 0;
}
