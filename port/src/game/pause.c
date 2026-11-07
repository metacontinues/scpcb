// pause.c: Main.bb's DrawMenu (pause / death screen, in-game options, achievements, quit) + NullGame, KillSounds
#include "g.h"
#include "../fmt/rmesh.h"

#define M MenuScale
#define R(v) iround(v)
#define ON(ox, oy, w, h) MouseOn(R(x + (ox) * M), R(y + (oy) * M), R(w), R(h))

static float tx, ty, tw, th;
#define TIP(o, v) DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th), o, v, 0)

// label + tick at x+270. locked ones (no runtime switch for them) are grey
static int tick(int x, int y, const char *l, int v, int lock, const char *tip, float tiph) {
	if (lock) set_color(100, 100, 100); else set_color(255, 255, 255);
	AAText(x, y, l, 0, 0);
	v = DrawTick(R(x + 270 * M), R(y + M), v, lock);
	if (ON(270, 1, 20 * M, 20 * M) && !OnSliderID) DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th + tiph), tip, 0, 0);
	return v;
}
// label + 100px slidebar at x+ox (0..100 in and out)
static float slide(int x, int y, int ox, int oy, const char *l, float v) {
	v = SlideBar(R(x + ox * M), R(y + oy * M), R(100 * M), v);
	set_color(255, 255, 255);
	AAText(x, y, l, 0, 0);
	return v;
}
#define SLIDETIP(ox, oy) (ON(ox, oy, 100 * M + 14, 20) && !OnSliderID)

static void graphics(int x, int y) {
	BumpEnabled = tick(x, y, "Enable bump mapping:", BumpEnabled, 1, "bump", 0); y = R(y + 30 * M);
	Vsync = tick(x, y, "VSync:", Vsync, 0, "vsync", 0); y = R(y + 30 * M);
	Opt_AntiAlias = tick(x, y, "Anti-aliasing:", Opt_AntiAlias, 0, "antialias", 0); y = R(y + 30 * M);
	EnableRoomLights = tick(x, y, "Enable room lights:", EnableRoomLights, 0, "roomlights", 0); y = R(y + 30 * M);
	ScreenGamma = slide(x, y, 270, 6, "Screen gamma", ScreenGamma * 50) / 50;
	if (SLIDETIP(270, 6)) TIP("gamma", ScreenGamma);
	y = R(y + 50 * M);
	set_color(255, 255, 255); AAText(x, y, "Particle amount:", 0, 0);
	ParticleAmount = Slider3(R(x + 270 * M), R(y + 6 * M), R(100 * M), ParticleAmount, 2, "MINIMAL", "REDUCED", "FULL");
	if (SLIDETIP(270, -6) || OnSliderID == 2) TIP("particleamount", ParticleAmount);
	y = R(y + 50 * M);
	set_color(255, 255, 255); AAText(x, y, "Texture LOD Bias:", 0, 0);
	TextureDetails = Slider5(R(x + 270 * M), R(y + 6 * M), R(100 * M), TextureDetails, 3, "0.8", "0.4", "0.0", "-0.4", "-0.8");
	static const float tf[] = {.8f, .4f, 0, -.4f, -.8f};
	if (TextureDetails >= 0 && TextureDetails <= 4) TextureFloat = tf[TextureDetails];
	texture_lod_bias(TextureFloat);
	if (SLIDETIP(270, -6) || OnSliderID == 3) DrawOptionsTooltip(R(tx), R(ty), R(tw), R(th + 100 * M), "texquality", 0, 0);
	y = R(y + 50 * M);
	EnableVRam = tick(x, y, "Save textures in the VRAM:", EnableVRam, 1, "vram", 0);
}

static void audio(int x, int y) {
	MusicVolume = slide(x, y, 250, -4, "Music volume:", MusicVolume * 100) / 100;
	if (SLIDETIP(250, -4)) TIP("musicvol", MusicVolume);
	y = R(y + 30 * M);
	PrevSFXVolume = slide(x, y, 250, -4, "Sound volume:", SFXVolume * 100) / 100;
	if (!DeafPlayer) SFXVolume = PrevSFXVolume;
	if (SLIDETIP(250, -4)) TIP("soundvol", PrevSFXVolume);
	y = R(y + 30 * M);
	EnableSFXRelease = tick(x, y, "Sound auto-release:", EnableSFXRelease, 1, "sfxautorelease", 220 * M); y = R(y + 30 * M);
	EnableUserTracks = tick(x, y, "Enable user tracks:", EnableUserTracks, 1, "usertrack", 0);
	if (!EnableUserTracks) return;
	y = R(y + 30 * M);
	UserTrackMode = tick(x, y, "User track mode:", UserTrackMode, 0, "usertrackmode", 0);
	AAText(x, R(y + 20 * M), UserTrackMode ? "Repeat" : "Random", 0, 0);
}

static void controls(int x, int y) {
	MouseSens = slide(x, y, 270, -4, "Mouse sensitivity:", (MouseSens + .5f) * 100) / 100 - .5f;
	if (SLIDETIP(270, -4)) TIP("mousesensitivity", MouseSens);
	y = R(y + 30 * M);
	InvertMouse = tick(x, y, "Invert mouse Y-axis:", InvertMouse, 0, "mouseinvert", 0);
	y = R(y + 40 * M);
	MouseSmooth = slide(x, y, 270, -4, "Mouse smoothing:", MouseSmooth * 50) / 50;
	if (SLIDETIP(270, -4)) TIP("mousesmoothing", MouseSmooth);
	set_color(255, 255, 255);
	y = R(y + 30 * M);
	AAText(x, y, "Control configuration:", 0, 0);
	y = R(y + 10 * M);
	struct { const char *l; int *k, id; } b[] = {
		{"Move Forward", &KEY_UP, 5}, {"Strafe Left", &KEY_LEFT, 3}, {"Move Backward", &KEY_DOWN, 6}, {"Strafe Right", &KEY_RIGHT, 4},
		{"Manual Blink", &KEY_BLINK, 7}, {"Sprint", &KEY_SPRINT, 8}, {"Open/Close Inventory", &KEY_INV, 9}, {"Crouch", &KEY_CROUCH, 10},
		{"Quick Save", &KEY_SAVE, 11}, {"Open/Close Console", &KEY_CONSOLE, 12},
	};
	for (int i = 0; i < 10; i++) {
		int ry = R(y + 20 * (i + 1) * M);
		AAText(x, ry, b[i].l, 0, 0);
		InputBox(R(x + 200 * M), ry, R(100 * M), R(20 * M), KeyName[(int)Min(*b[i].k, 210)], b[i].id);
	}
	if (MouseOn(x, y, R(300 * M), R(220 * M))) TIP("controls", 0);
	int key = 0;
	for (int i = 0; i <= 227; i++) if (key_hit(i)) { key = i; break; }
	if (!key) return;
	for (int i = 0; i < 10; i++) if (SelectedInputBox == b[i].id) *b[i].k = key;
	SelectedInputBox = 0;
}

static void advanced(int x, int y) {
	HUDenabled = tick(x, y, "Show HUD:", HUDenabled, 0, "hud", 0); y = R(y + 30 * M);
	CanOpenConsole = tick(x, y, "Enable console:", CanOpenConsole, 0, "consoleenable", 0); y = R(y + 30 * M);
	ConsoleOpening = tick(x, y, "Open console on error:", ConsoleOpening, 0, "consoleerror", 0); y = R(y + 50 * M);
	AchvMSGenabled = tick(x, y, "Achievement popups:", AchvMSGenabled, 0, "achpopup", 0); y = R(y + 50 * M);
	ShowFPS = tick(x, y, "Show FPS:", ShowFPS, 0, "showfps", 0); y = R(y + 30 * M);
	if (tick(x, y, "Framelimit:", CurrFrameLimit > 0, 0, "framelimit", 0)) {
		CurrFrameLimit = Max(SlideBar(R(x + 150 * M), R(y + 30 * M), R(100 * M), CurrFrameLimit * 99) / 99, .01f);
		Framelimit = R(19 + CurrFrameLimit * 100);
		set_color(255, 255, 0);
		AAText(R(x + 5 * M), R(y + 25 * M), cat(s_str(Framelimit), " FPS"), 0, 0);
	} else { CurrFrameLimit = 0; Framelimit = 0; }
	if (ON(150, 30, 100 * M + 14, 20)) TIP("framelimit", Framelimit);
	y = R(y + 80 * M);
	// raylib text is smooth either way (see AAText), so this only flips the ini value
	AATextEnable = tick(x, y, "Antialiased text:", AATextEnable, 0, "antialiastext", 0);
}

static void back(int x, int y, int oy) {
	if (!DrawButton(R(x + 101 * M), R(y + oy * M), R(230 * M), R(60 * M), "Back", 1, 0, 1)) return;
	AchievementsMenu = OptionsMenu = QuitMSG = 0;
	MouseHit1 = 0;
}

static void options(int x, int y, int w) {
	if (DrawButton(R(x + 101 * M), R(y + 390 * M), R(230 * M), R(60 * M), "Back", 1, 0, 1)) {
		AchievementsMenu = OptionsMenu = QuitMSG = 0;
		MouseHit1 = 0;
		SaveOptionsINI();
		anti_alias(Opt_AntiAlias);
		texture_lod_bias(TextureFloat);
	}
	static const char *names[] = {"GRAPHICS", "AUDIO", "CONTROLS", "ADVANCED"};
	set_color(0, 255, 0);
	if (OptionsMenu >= 1 && OptionsMenu <= 4) draw_rect(R(x + (110 * (OptionsMenu - 1) - 10) * M), R(y - 5 * M), R(110 * M), R(40 * M), 1);
	for (int i = 0; i < 4; i++)
		if (DrawButton(R(x + (110 * i - 5) * M), y, R(100 * M), R(30 * M), names[i], 0, 0, 1)) OptionsMenu = i + 1;
	tx = GraphicWidth / 2 + w / 2; ty = y; tw = 400 * M; th = 150 * M;
	set_color(255, 255, 255);
	AASetFont(Font1);
	y = R(y + 50 * M);
	switch (OptionsMenu) {
	case 1: graphics(x, y); break;
	case 2: audio(x, y); break;
	case 3: controls(x, y); break;
	case 4: advanced(x, y); break;
	}
}

static void tomenu(void) {
	NullGame(1);
	MenuOpen = 0; MainMenuOpen = 1; MainMenuTab = 0;
	sset(&CurrSave, "");
	flush_keys();
}

static void quit(int x, int y) {
	// TODO "Save & Quit" (SAVEONQUIT/SAVEANYWHERE, not in 173/exit1/gatea, CanSave) once Save.bb's SaveGame is ported
	if (DrawButton(x, R(y + 60 * M), R(390 * M), R(60 * M), "Quit", 1, 0, 1)) { tomenu(); return; }
	back(x, y, 344);
}

static void achievements(int x, int y) {
	back(x, y, 344);
	if (AchievementsMenu <= 0) return;
	float sc = GraphicHeight / 768.0f;
	int ax = R(x + 22 * M), sep = R(76 * sc), s = R(64 * sc), page = (AchievementsMenu - 1) * 12;
	if (AchievementsMenu <= (MAXACHIEVEMENTS - 1) / 12 && DrawButton(R(x + 341 * M), R(y + 344 * M), R(50 * M), R(60 * M), ">", 1, 0, 1)) AchievementsMenu++;
	if (AchievementsMenu > 1 && DrawButton(R(x + 41 * M), R(y + 344 * M), R(50 * M), R(60 * M), "<", 1, 0, 1)) AchievementsMenu--;
	for (int i = 0; i < 12 && page + i < MAXACHIEVEMENTS; i++) DrawAchvIMG(ax, R(y + i / 4 * 120 * M), page + i);
	for (int i = 0; i < 12 && page + i < MAXACHIEVEMENTS; i++)
		if (MouseOn(ax + i % 4 * sep, R(y + i / 4 * 120 * M), s, s)) { AchievementTooltip(page + i); break; }
}

// greyed out until Save.bb's LoadGameQuick is ported (GameSaved never gets set yet)
static void loadgame(int x, int y, int dead) {
	if (dead) { DrawButton(x, y, R(390 * M), R(60 * M), "", 1, 0, 1); set_color(50, 50, 50); AAText(R(x + 185 * M), R(y + 30 * M), "Load Game", 1, 1); return; }
	DrawFrame(x, y, R(390 * M), R(60 * M), 0, 0);
	set_color(100, 100, 100);
	AASetFont(Font2);
	AAText(R(x + 390 * M / 2), R(y + 60 * M / 2), "Load Game", 1, 1);
}

// 173 or 106 standing next to you while you sit in the menu: after 40 ticks the menu shuts on you
static int stophiding(void) {
	str rn = PlayerRoom->RoomTemplate->Name;
	if (s_eq(rn, "exit1") || s_eq(rn, "gatea")) return 0;
	if (StopHidingTimer == 0) {
		if ((Curr173 && entity_distance(Curr173->Collider, Collider) < 4) || (Curr106 && entity_distance(Curr106->Collider, Collider) < 4)) StopHidingTimer = 1;
		return 0;
	}
	if (StopHidingTimer >= 40 || KillTimer < 0) return 0;
	StopHidingTimer += FPSfactor2; // FPSfactor is 0 while the menu's open
	if (StopHidingTimer < 40) return 0;
	PlaySound_Strict(HorrorSFX[15]);
	sset(&Msg, "STOP HIDING"); MsgTimer = 6 * 70;
	MenuOpen = 0;
	return 1;
}

void DrawMenu(void) {
	if (!app_focused()) { // out of focus: pause, and go easy on the cpu
		if (!MenuOpen) { MenuOpen = 1; PauseSounds(); } // TODO not while Using294
		delay_ms(1000);
	}
	if (!MenuOpen || stophiding()) { AASetFont(Font1); return; }
	InvOpen = 0;
	int w = image_width(PauseMenuIMG), h = image_height(PauseMenuIMG);
	int x = GraphicWidth / 2 - w / 2, y = GraphicHeight / 2 - h / 2;
	draw_image(PauseMenuIMG, x, y, 0);
	set_color(255, 255, 255);
	x = R(x + 132 * M); y = R(y + 122 * M);
	if (!MouseDown1) OnSliderID = 0;

	int sub = AchievementsMenu > 0 ? 1 : OptionsMenu > 0 ? 2 : QuitMSG > 0 ? 3 : 0;
	static const char *titles[] = {"PAUSED", "ACHIEVEMENTS", "OPTIONS", "QUIT?"};
	AASetFont(Font2);
	AAText(x, R(y - (122 - 45) * M), !sub && KillTimer < 0 ? "YOU DIED" : titles[sub], 0, 1);
	AASetFont(Font1);
	if (!sub) {
		AAText(x, y, cat("Difficulty: ", SelectedDifficulty->name), 0, 0);
		AAText(x, R(y + 20 * M), cat("Save: ", CurrSave), 0, 0);
		AAText(x, R(y + 40 * M), cat("Map seed: ", RandomSeed), 0, 0);
	} else if (sub == 2 && KillTimer >= 0) options(x, y, w);
	else if (sub == 3 && KillTimer >= 0) { quit(x, y); if (MainMenuOpen) return; }
	else achievements(x, y);

	y += 10;
	if (!sub) {
		if (KillTimer >= 0) {
			y = R(y + 72 * M);
			if (DrawButton(x, y, R(390 * M), R(60 * M), "Resume", 1, 1, 1)) {
				MenuOpen = 0;
				ResumeSounds();
				mouse_x_speed(); mouse_y_speed(); mouse_z_speed(); mouse_x_speed_1 = mouse_y_speed_1 = 0;
			}
			y = R(y + 75 * M);
			if (!SelectedDifficulty->permaDeath) { loadgame(x, y, 0); y = R(y + 75 * M); }
			if (DrawButton(x, y, R(390 * M), R(60 * M), "Achievements", 1, 0, 1)) AchievementsMenu = 1;
			y = R(y + 75 * M);
			if (DrawButton(x, y, R(390 * M), R(60 * M), "Options", 1, 0, 1)) OptionsMenu = 1;
			y = R(y + 75 * M);
		} else {
			y = R(y + 104 * M);
			loadgame(x, y, 1);
			if (DrawButton(x, R(y + 80 * M), R(390 * M), R(60 * M), "Quit to Menu", 1, 0, 1)) { tomenu(); return; }
			y = R(y + 80 * M);
		}
		if (KillTimer >= 0 && !MainMenuOpen && DrawButton(x, y, R(390 * M), R(60 * M), "Quit", 1, 0, 1)) QuitMSG = 1;
		AASetFont(Font1);
		if (KillTimer < 0) RowText(DeathMSG, x, R(y + 80 * M), R(390 * M), R(600 * M), 0, 1);
	}
	if (Fullscreen) draw_image(CursorIMG, ScaledMouseX(), ScaledMouseY(), 0);
	AASetFont(Font1);
}

// ---- back to the main menu

static void stopchn(int c, int stream) { if (!c) return; if (stream) StopStream_Strict(c); else if (channel_playing(c)) stop_channel(c); }

void KillSounds(void) {
	for (int i = 0; i <= 9; i++) if (TempSounds[i]) { FreeSound_Strict(TempSounds[i]); TempSounds[i] = 0; }
	each(Events, e) { stopchn(e->SoundCHN, e->SoundCHN_isStream); stopchn(e->SoundCHN2, e->SoundCHN2_isStream); }
	each(NPCs, n) { stopchn(n->SoundChn, n->SoundChn_IsStream); stopchn(n->SoundChn2, n->SoundChn2_IsStream); }
	each(Doors, d) stopchn(d->SoundCHN, 0);
	each(DevilEmitters, dem) stopchn(dem->SoundCHN, 0);
	stopchn(AmbientSFXCHN, 0);
	stopchn(BreathCHN, 0);
	if (IntercomStreamCHN) { StopStream_Strict(IntercomStreamCHN); IntercomStreamCHN = 0; }
	each(Sound, snd) {
		if (EnableSFXRelease && snd->internalHandle) { free_sound(snd->internalHandle); snd->internalHandle = 0; snd->releaseTime = 0; }
		for (int i = 0; i < 32; i++) if (snd->channels[i]) stop_channel(snd->channels[i]);
	}
}

void NullGame(int buttonsfx) {
	KillSounds();
	if (buttonsfx) PlaySound_Strict(ButtonSFX);
	ClearTextureCache();
	UnableToMove = 0;
	QuickLoadPercent = -1; QuickLoadPercent_DisplayTimer = 0; QuickLoad_CurrEvent = NULL;
	sset(&DeathMSG, ""); sset(&SelectedMap, "");
	UsedConsole = 0;
	DoorTempID = RoomTempID = 0;
	GameSaved = 0;
	HideDistance = 15;
	memset(MapTemp, 0, sizeof(int) * (MapWidth + 2) * (MapHeight + 2));
	memset(MapFound, 0, sizeof(int) * (MapWidth + 2) * (MapHeight + 2));
	DropSpeed = Shake = CurrSpeed = DeathTimer = HeartBeatVolume = 0;
	StaminaEffect = BlinkEffect = 1; StaminaEffectTimer = BlinkEffectTimer = 0;
	Bloodloss = Injuries = Infect = 0;
	for (int i = 0; i <= 5; i++) SCP1025state[i] = 0;
	sset(&SelectedEnding, "");
	EndingTimer = ExplosionTimer = CameraShake = LightFlash = 0;
	GodMode = NoClip = WireframeState = 0;
	WearingGasMask = WearingHazmat = WearingVest = Wearing714 = 0;
	if (WearingNightVision) { CameraFogFar = StoredCameraFogFar; WearingNightVision = 0; }
	I_427->Using = 0; I_427->Timer = 0;
	ForceMove = ForceAngle = 0;
	user_camera_pitch = mouse_x_speed_1 = mouse_y_speed_1 = 0; // upstream carried the look pitch into the next game
	Playable = 1;
	CoffinDistance = 100;
	Contained106 = 0;
	MTFtimer = 0;
	for (int i = 0; i <= 9; i++) { MTFrooms[i] = NULL; MTFroomState[i] = 0; }
	each(Screens, s) { if (s->img) { free_image(s->img); s->img = 0; } ldel(s); }
	for (int i = 0; i < MAXACHIEVEMENTS; i++) Achievements[i] = 0;
	RefinedItems = 0;
	sset(&ConsoleInput, ""); ConsoleOpen = 0;
	EyeIrritation = EyeStuck = 0;
	ShouldPlay = 0;
	KillTimer = FallTimer = 0; Stamina = 100; BlurTimer = 0;
	SuperMan = 0; SuperManTimer = 0; Sanity = 0; RestoreSanity = 1;
	Crouch = 0; CrouchState = 0; LightVolume = 0;
	Vomit = 0; VomitTimer = 0;
	SecondaryLightOn = PrevSecondaryLightOn = 1; RemoteDoorOn = 1; SoundTransmission = 0;
	InfiniteStamina = 0;
	sset(&Msg, ""); MsgTimer = 0;
	for (int i = 0; i < MaxItemAmount; i++) Inventory[i] = NULL;
	SelectedItem = NULL; ClosestItem = NULL; OtherOpen = NULL; ClosestButton = 0; GrabbedEntity = 0;
	SelectedDoor = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
	each(Events, e) { if (e->Sound) FreeSound_Strict(e->Sound); if (e->Sound2) FreeSound_Strict(e->Sound2); }
	// the objects go wholesale: ldel_each skips the Remove* fns since every handle they'd clear dies too
	ldel_each(Doors); ldel_each(LightTemplates); ldel_each(Materials); ldel_each(WayPoints); ldel_each(TempWayPoints);
	ldel_each(TempScreens); // upstream kept these, so each new game added another copy of every screen
	ldel_each(Rooms); ldel_each(ItemTemplates); ldel_each(Items); ldel_each(Decals); ldel_each(NPCs);
	ldel_each(Events); ldel_each(SecurityCams); ldel_each(Emitters); ldel_each(Particles);
	ldel_each(DevilEmitters); ldel_each(ElevatorObj); ldel_each(Forest);
	Curr173 = Curr106 = Curr096 = Curr5131 = NULL;
	PlayerRoom = NULL;
	each(RoomTemplates, rt) rt->obj = 0;
	for (int i = 0; i <= 5; i++) if (RadioCHN[i] && channel_playing(RadioCHN[i])) stop_channel(RadioCHN[i]);
	Wearing1499 = 0; // TODO the NTF_1499* globals + DeleteChunks once 1499 is in
	NoTarget = 0;
	OptionsMenu = QuitMSG = AchievementsMenu = -1;
	MusicVolume = PrevMusicVolume; SFXVolume = PrevSFXVolume;
	DeafPlayer = 0; DeafTimer = 0;
	IsZombie = 0;
	ldel_each(AchievementMsg); CurrAchvMSGID = 0;
	// LoadEntities loads these again
	int *imgs[] = {&PauseMenuIMG, &SprintIcon, &BlinkIcon, &CrouchIcon, &HandIcon, &HandIcon2, &StaminaMeterIMG, &KeypadHUD, &Panel294};
	for (int i = 0; i < 9; i++) { free_image(*imgs[i]); *imgs[i] = 0; }
	clear_world();
	rmesh_clear_cache();
	Camera = ark_blur_cam = Collider = Sky = 0;
}
