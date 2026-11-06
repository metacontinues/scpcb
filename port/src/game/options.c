// options.c: the options tabs of UpdateMainMenu (MainMenuTab 3 graphics, 5 audio, 6 controls, 7 advanced)
#include "g.h"

#define M MenuScale
#define R(v) bbInt(v)

static float tx, ty, tw, th;
#define TIP(o, v) DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th), o, v, 0)
#define TICKTIP(x, y) MouseOn(R(x + 310 * M), R(y + M), R(20 * M), R(20 * M))
#define LABEL(x, y, s) (bbColor(255, 255, 255), AAText(R(x + 20 * M), y, s, 0, 0))
#define TICK(x, y, v) DrawTick(R(x + 310 * M), R(y + M), v, 0)

static void graphics(int x, int y, int w) {
	DrawFrame(x, y, w, R(330 * M), 0, 0);
	y = R(y + 20 * M);
	LABEL(x, y, "Enable bump mapping:"); BumpEnabled = TICK(x, y, BumpEnabled);
	if (TICKTIP(x, y) && OnSliderID == 0) TIP("bump", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "VSync:"); Vsync = TICK(x, y, Vsync);
	if (TICKTIP(x, y) && OnSliderID == 0) TIP("vsync", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Anti-aliasing:"); Opt_AntiAlias = TICK(x, y, Opt_AntiAlias);
	if (TICKTIP(x, y) && OnSliderID == 0) TIP("antialias", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Enable room lights:"); EnableRoomLights = TICK(x, y, EnableRoomLights);
	if (TICKTIP(x, y) && OnSliderID == 0) TIP("roomlights", 0);
	y = R(y + 30 * M);
	ScreenGamma = SlideBar(R(x + 310 * M), R(y + 6 * M), R(150 * M), ScreenGamma * 50) / 50;
	LABEL(x, y, "Screen gamma");
	if (MouseOn(R(x + 310 * M), R(y + 6 * M), R(150 * M + 14), 20) && OnSliderID == 0) TIP("gamma", ScreenGamma);
	y = R(y + 50 * M);
	LABEL(x, y, "Particle amount:");
	ParticleAmount = Slider3(R(x + 310 * M), R(y + 6 * M), R(150 * M), ParticleAmount, 2, "MINIMAL", "REDUCED", "FULL");
	if ((MouseOn(R(x + 310 * M), R(y - 6 * M), R(150 * M + 14), 20) && OnSliderID == 0) || OnSliderID == 2) TIP("particleamount", ParticleAmount);
	y = R(y + 50 * M);
	LABEL(x, y, "Texture LOD Bias:");
	TextureDetails = Slider5(R(x + 310 * M), R(y + 6 * M), R(150 * M), TextureDetails, 3, "0.8", "0.4", "0.0", "-0.4", "-0.8");
	static const float tf[] = {.8f, .4f, 0, -.4f, -.8f};
	if (TextureDetails >= 0 && TextureDetails <= 4) TextureFloat = tf[TextureDetails];
	bbTextureLodBias(TextureFloat);
	if ((MouseOn(R(x + 310 * M), R(y - 6 * M), R(150 * M + 14), 20) && OnSliderID == 0) || OnSliderID == 3)
		DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th + 100 * M), "texquality", 0, 0);
	y = R(y + 50 * M);
	LABEL(x, y, "Save textures in the VRAM:"); EnableVRam = TICK(x, y, EnableVRam);
	if (TICKTIP(x, y) && OnSliderID == 0) TIP("vram", 0);
}

static void scantracks(void) {
	UserTrackCheck = UserTrackCheck2 = 0;
	void *d = bbReadDir("SFX\\Radio\\UserTracks\\");
	for (const char *f; *(f = bbNextFile(d));) {
		str p = cat("SFX\\Radio\\UserTracks\\", f);
		if (bbFileType(p) != 1) continue;
		UserTrackCheck++;
		int t = bbLoadSound(p);
		if (t) UserTrackCheck2++;
		bbFreeSound(t);
	}
	bbCloseDir(d);
}

static void audio(int x, int y, int w) {
	DrawFrame(x, y, w, R(220 * M), 0, 0);
	y = R(y + 20 * M);
	MusicVolume = SlideBar(R(x + 310 * M), R(y - 4 * M), R(150 * M), MusicVolume * 100) / 100;
	LABEL(x, y, "Music volume:");
	if (MouseOn(R(x + 310 * M), R(y - 4 * M), R(150 * M + 14), 20)) TIP("musicvol", MusicVolume);
	y = R(y + 40 * M);
	PrevSFXVolume = SlideBar(R(x + 310 * M), R(y - 4 * M), R(150 * M), SFXVolume * 100) / 100;
	SFXVolume = PrevSFXVolume;
	LABEL(x, y, "Sound volume:");
	if (MouseOn(R(x + 310 * M), R(y - 4 * M), R(150 * M + 14), 20)) TIP("soundvol", PrevSFXVolume);
	y = R(y + 30 * M);
	LABEL(x, y, "Sound auto-release:"); EnableSFXRelease = TICK(x, y, EnableSFXRelease);
	if (EnableSFXRelease_Prev != EnableSFXRelease) {
		each(Sound, snd) {
			if (EnableSFXRelease) {
				for (int i = 0; i < 32; i++) if (snd->channels[i] && bbChannelPlaying(snd->channels[i])) bbStopChannel(snd->channels[i]);
				if (snd->internalHandle) { bbFreeSound(snd->internalHandle); snd->internalHandle = 0; }
				snd->releaseTime = 0;
			} else if (!snd->internalHandle) snd->internalHandle = bbLoadSound(snd->name);
		}
		EnableSFXRelease_Prev = EnableSFXRelease;
	}
	if (TICKTIP(x, y)) DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th + 220 * M), "sfxautorelease", 0, 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Enable user tracks:"); EnableUserTracks = TICK(x, y, EnableUserTracks);
	if (TICKTIP(x, y)) TIP("usertrack", 0);
	if (!EnableUserTracks) { UserTrackCheck = 0; return; }
	y = R(y + 30 * M);
	LABEL(x, y, "User track mode:"); UserTrackMode = TICK(x, y, UserTrackMode);
	AAText(R(x + 350 * M), R(y + M), UserTrackMode ? "Repeat" : "Random", 0, 0);
	if (TICKTIP(x, y)) TIP("usertrackmode", 0);
	if (DrawButton(R(x + 20 * M), R(y + 30 * M), R(190 * M), R(25 * M), "Scan for User Tracks", 0, 0, 1)) scantracks();
	if (MouseOn(R(x + 20 * M), R(y + 30 * M), R(190 * M), R(25 * M))) TIP("usertrackscan", 0);
	if (UserTrackCheck > 0)
		AAText(R(x + 20 * M), R(y + 100 * M), cat("User tracks found (", bbStr(UserTrackCheck2), "/", bbStr(UserTrackCheck), " successfully loaded)"), 0, 0);
}

static void controls(int x, int y, int w) {
	DrawFrame(x, y, w, R(270 * M), 0, 0);
	y = R(y + 20 * M);
	MouseSens = SlideBar(R(x + 310 * M), R(y - 4 * M), R(150 * M), (MouseSens + .5f) * 100) / 100 - .5f;
	LABEL(x, y, "Mouse sensitivity:");
	if (MouseOn(R(x + 310 * M), R(y - 4 * M), R(150 * M + 14), 20)) TIP("mousesensitivity", MouseSens);
	y = R(y + 40 * M);
	LABEL(x, y, "Invert mouse Y-axis:"); InvertMouse = TICK(x, y, InvertMouse);
	if (TICKTIP(x, y)) TIP("mouseinvert", 0);
	y = R(y + 40 * M);
	MouseSmooth = SlideBar(R(x + 310 * M), R(y - 4 * M), R(150 * M), MouseSmooth * 50) / 50;
	LABEL(x, y, "Mouse smoothing:");
	if (MouseOn(R(x + 310 * M), R(y - 4 * M), R(150 * M + 14), 20)) TIP("mousesmoothing", MouseSmooth);
	bbColor(255, 255, 255);
	y = R(y + 30 * M);
	AAText(R(x + 20 * M), y, "Control configuration:", 0, 0);
	y = R(y + 10 * M);
	// label, column, row, key, input box id
	struct { const char *l; int col, row, *k, id; } b[] = {
		{"Move Forward", 0, 1, &KEY_UP, 5}, {"Strafe Left", 0, 2, &KEY_LEFT, 3}, {"Move Backward", 0, 3, &KEY_DOWN, 6},
		{"Strafe Right", 0, 4, &KEY_RIGHT, 4}, {"Quick Save", 0, 5, &KEY_SAVE, 11},
		{"Manual Blink", 1, 1, &KEY_BLINK, 7}, {"Sprint", 1, 2, &KEY_SPRINT, 8}, {"Open/Close Inventory", 1, 3, &KEY_INV, 9},
		{"Crouch", 1, 4, &KEY_CROUCH, 10}, {"Open/Close Console", 1, 5, &KEY_CONSOLE, 12},
	};
	for (int i = 0; i < 10; i++) {
		int ry = R(y + 20 * b[i].row * M);
		AAText(R(x + (b[i].col ? 280 : 20) * M), ry, b[i].l, 0, 0);
		InputBox(R(x + (b[i].col ? 470 : 160) * M), ry, R(100 * M), R(20 * M), KeyName[(int)Min(*b[i].k, 210)], b[i].id);
	}
	if (MouseOn(R(x + 20 * M), y, R(w - 40 * M), R(120 * M))) TIP("controls", 0);
	int key = 0;
	for (int i = 0; i <= 227; i++) if (bbKeyHit(i)) { key = i; break; }
	if (!key) return;
	for (int i = 0; i < 10; i++) if (SelectedInputBox == b[i].id) *b[i].k = key;
	SelectedInputBox = 0;
}

static void advanced(int x, int y, int w) {
	DrawFrame(x, y, w, R(320 * M), 0, 0);
	y = R(y + 20 * M);
	LABEL(x, y, "Show HUD:"); HUDenabled = TICK(x, y, HUDenabled);
	if (TICKTIP(x, y)) TIP("hud", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Enable console:"); CanOpenConsole = TICK(x, y, CanOpenConsole);
	if (TICKTIP(x, y)) TIP("consoleenable", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Open console on error:"); ConsoleOpening = TICK(x, y, ConsoleOpening);
	if (TICKTIP(x, y)) TIP("consoleerror", 0);
	y = R(y + 50 * M);
	LABEL(x, y, "Achievement popups:"); AchvMSGenabled = TICK(x, y, AchvMSGenabled);
	if (TICKTIP(x, y)) TIP("achpopup", 0);
	y = R(y + 50 * M);
	LABEL(x, y, "Show FPS:"); ShowFPS = TICK(x, y, ShowFPS);
	if (TICKTIP(x, y)) TIP("showfps", 0);
	y = R(y + 30 * M);
	LABEL(x, y, "Framelimit:");
	bbColor(255, 255, 255);
	if (DrawTick(R(x + 310 * M), y, CurrFrameLimit > 0, 0)) {
		CurrFrameLimit = Max(SlideBar(R(x + 150 * M), R(y + 30 * M), R(100 * M), CurrFrameLimit * 99) / 99, .01f);
		Framelimit = R(19 + CurrFrameLimit * 100);
		bbColor(255, 255, 0);
		AAText(R(x + 25 * M), R(y + 25 * M), cat(bbStr(Framelimit), " FPS"), 0, 0);
	} else { CurrFrameLimit = 0; Framelimit = 0; }
	if (TICKTIP(x, y)) TIP("framelimit", Framelimit);
	if (MouseOn(R(x + 150 * M), R(y + 30 * M), R(100 * M + 14), 20)) TIP("framelimit", Framelimit);
	y = R(y + 80 * M);
	// raylib text is smooth either way (see AAText), so this only flips the ini value
	LABEL(x, y, "Antialiased text:"); AATextEnable = TICK(x, y, AATextEnable);
	if (TICKTIP(x, y)) TIP("antialiastext", 0);
}

void UpdateOptionsTab(int x, int y, int w) {
	int h = R(60 * M);
	DrawFrame(x, y, w, h, 0, 0);
	bbColor(0, 255, 0);
	static const int tabs[] = {3, 5, 6, 7};
	static const char *names[] = {"GRAPHICS", "AUDIO", "CONTROLS", "ADVANCED"};
	for (int i = 0; i < 4; i++)
		if (MainMenuTab == tabs[i]) bbRect(R(x + (15 + 140 * i) * M), R(y + 10 * M), R(w / 5 + 10 * M), R(h / 2 + 10 * M), 1);
	bbColor(255, 255, 255);
	for (int i = 0; i < 4; i++)
		if (DrawButton(R(x + (20 + 140 * i) * M), R(y + 15 * M), w / 5, h / 2, names[i], 0, 0, 1)) MainMenuTab = tabs[i];
	AASetFont(Font1);
	y = R(y + 70 * M);
	if (MainMenuTab != 5) UserTrackCheck = UserTrackCheck2 = 0;
	tx = x + w; ty = y; tw = 400 * M; th = 150 * M;
	switch (MainMenuTab) {
	case 3: graphics(x, y, w); break;
	case 5: audio(x, y, w); break;
	case 6: controls(x, y, w); break;
	case 7: advanced(x, y, w); break;
	}
}
