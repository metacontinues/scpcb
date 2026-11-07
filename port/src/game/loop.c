// loop.c: Main.bb's main loop. the menu side works, the in-game side isn't ported yet
#include "g.h"

static void ambience(void) {
	str rn = PlayerRoom->RoomTemplate->Name;
	if (s_eq(rn, "pocketdimension") || s_eq(rn, "gatea") || s_eq(rn, "exit1") || MenuOpen || ConsoleOpen || InvOpen) return;
	if (irand(1, 1500) == 1) {
		for (int i = 0; i <= 5; i++) if (AmbientSFX[i][CurrAmbientSFX] && !channel_playing(AmbientSFXCHN)) { FreeSound_Strict(AmbientSFX[i][CurrAmbientSFX]); AmbientSFX[i][CurrAmbientSFX] = 0; }
		float rx = frnd(-1, 1), rz = frnd(-1, 1);
		position_entity(SoundEmitter, entity_x(Camera, 0) + rx, 0, entity_z(Camera, 0) + rz, 0);
		if (irand(1, 3) == 1) PlayerZone = 3;
		if (s_eq(rn, "173")) PlayerZone = 4;
		// TODO room860: PlayerZone 5 + emitter at y 30 while its event's EventState = 1
		CurrAmbientSFX = irand(0, AmbientSFXAmount[PlayerZone] - 1);
		static const char *dir[] = {"Zone1", "Zone2", "Zone3", "General", "Pre-breach", "Forest"};
		int *a = &AmbientSFX[PlayerZone][CurrAmbientSFX];
		if (!*a) *a = LoadSound_Strict(sf("SFX\\Ambient\\%s\\ambient%d.ogg", PlayerZone <= 2 ? sf("Zone%d", PlayerZone + 1) : dir[PlayerZone], CurrAmbientSFX + 1));
		AmbientSFXCHN = PlaySound2(*a, Camera, SoundEmitter, 10, 1);
	}
	UpdateSoundOrigin(AmbientSFXCHN, Camera, SoundEmitter, 10, 1);
	if (irand(1, 50000) == 3 && !s_eq(rn, "room860") && !s_eq(rn, "room1123") && !s_eq(rn, "173") && !s_eq(rn, "dimension1499")) {
		if (FPSfactor > 0) LightBlink = frnd(1, 2);
		PlaySound_Strict(LoadTempSound(sf("SFX\\SCP\\079\\Broadcast%d.ogg", irand(1, 7))));
	}
}

// the blink/darkness overlay part of the loop
static void darkness(void) {
	float darkA = 0;
	if (MenuOpen) return;
	if (Sanity < 0) {
		if (RestoreSanity) Sanity = Min(Sanity + FPSfactor, 0);
		if (Sanity < -200) {
			darkA = Max(Min((-Sanity - 200) / 700, .6f), darkA);
			if (KillTimer >= 0) { HeartBeatVolume = Min(fabsf(Sanity + 200) / 500, 1); HeartBeatRate = Max(70 + fabsf(Sanity + 200) / 6, HeartBeatRate); }
		}
	}
	if (EyeStuck > 0) {
		BlinkTimer = BLINKFREQ;
		EyeStuck = Max(EyeStuck - FPSfactor, 0);
		if (EyeStuck < 9000) BlurTimer = Max(BlurTimer, (9000 - EyeStuck) * .5f);
		if (EyeStuck < 6000) darkA = Min(Max(darkA, (6000 - EyeStuck) / 5000), 1);
		if (EyeStuck < 9000 && EyeStuck + FPSfactor >= 9000) { Msg = "The eyedrops are causing your eyes to tear up."; MsgTimer = 70 * 6; }
	}
	if (BlinkTimer < 0) {
		if (BlinkTimer > -5) darkA = Max(darkA, sinf(fabsf(BlinkTimer * 18) * DTOR));
		else if (BlinkTimer > -15) darkA = 1;
		else darkA = Max(darkA, fabsf(sinf(BlinkTimer * 18 * DTOR)));
		if (BlinkTimer <= -20) {
			switch (SelectedDifficulty->otherFactors) {
			case EASY: BLINKFREQ = frnd(490, 700); break;
			case NORMAL: BLINKFREQ = frnd(455, 665); break;
			case HARD: BLINKFREQ = frnd(420, 630); break;
			}
			BlinkTimer = BLINKFREQ;
		}
		BlinkTimer -= FPSfactor;
	} else {
		BlinkTimer -= FPSfactor * .6f * BlinkEffect;
		if (EyeIrritation > 0) BlinkTimer -= Min(EyeIrritation / 100 + 1, 4) * FPSfactor;
		darkA = Max(darkA, 0);
	}
	EyeIrritation = Max(0, EyeIrritation - FPSfactor);
	if (BlinkEffectTimer > 0) BlinkEffectTimer -= FPSfactor / 70;
	else if (BlinkEffect != 1) BlinkEffect = 1;
	LightBlink = Max(LightBlink - FPSfactor / 35, 0);
	if (LightBlink > 0) darkA = Min(Max(darkA, LightBlink * frnd(.3f, .8f)), 1);
	// TODO Using294: darkA = 1
	if (!WearingNightVision) darkA = Max((1 - SecondaryLightOn) * .9f, darkA);
	if (KillTimer < 0) {
		InvOpen = 0; SelectedItem = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
		BlurTimer = fabsf(KillTimer * 5);
		KillTimer -= FPSfactor * .8f;
		if (KillTimer < -360) { MenuOpen = 1; if (s_len(SelectedEnding)) EndingTimer = Min(KillTimer, -.1f); }
		darkA = Max(darkA, Min(fabsf(KillTimer / 400), 1));
	}
	if (FallTimer < 0) {
		// TODO drop a half-worn vest/hazmat (DropItem)
		InvOpen = 0; SelectedItem = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
		BlurTimer = fabsf(FallTimer * 10);
		FallTimer -= FPSfactor;
		darkA = Max(darkA, Min(fabsf(FallTimer / 400), 1));
	}
	if (SelectedItem && (s_eq(SelectedItem->itemtemplate->tempname, "navigator") || s_eq(SelectedItem->itemtemplate->tempname, "nav"))) darkA = Max(darkA, .5f);
	if (SelectedScreen) darkA = Max(darkA, .5f);
	entity_alpha(Dark, darkA);
}

// TODO DrawMenu (Main.bb 7116): until it's ported, a bare pause screen so you can get out
static void pausestub(void) {
	set_color(0, 0, 0); draw_rect(0, 0, GraphicWidth, GraphicHeight, 1);
	set_color(255, 255, 255); AASetFont(Font2);
	AAText(GraphicWidth / 2, GraphicHeight / 2 - 40, "PAUSED", 1, 1);
	AASetFont(Font1);
	AAText(GraphicWidth / 2, GraphicHeight / 2 + 20, "(the pause menu isn't ported yet)  esc: resume   q: quit", 1, 1);
	if (key_hit(16)) End();
}

// Main.bb's main loop, in-game side. what isn't ported yet is listed as TODOs in order
void UpdateGame(void) {
	UpdateStreamSounds();
	ShouldPlay = iround(Min(PlayerZone, 2));
	DrawHandIcon = 0; RestoreSanity = 1; ShouldEntitiesFall = 1;
	// TODO UpdateSecurityCams
	ambience();
	UpdateCheckpoint1 = UpdateCheckpoint2 = 0;
	if (!MenuOpen && !InvOpen && !OtherOpen && !SelectedDoor && !ConsoleOpen && !SelectedScreen && EndingTimer >= 0) { // TODO Using294
		LightVolume = CurveValue(TempLightVolume, LightVolume, 50);
		camera_fog_range(Camera, CameraFogNear * LightVolume, CameraFogFar * LightVolume);
		camera_fog_color(Camera, 0, 0, 0);
		camera_fog_mode(Camera, 1);
		camera_range(Camera, .05f, Min(CameraFogFar * LightVolume * 1.5f, 28));
		if (!s_eq(PlayerRoom->RoomTemplate->Name, "pocketdimension")) camera_cls_color(Camera, 0, 0, 0);
		ambient_light(Brightness, Brightness, Brightness);
		PlayerSoundVolume = CurveValue(0, PlayerSoundVolume, 5);
		CanSave = 1;
		// TODO UpdateDeafPlayer, UpdateEmitters
		MouseLook();
		MovePlayer();
		// TODO InFacility = CheckForPlayerInFacility(), dimension1499, UpdateEndings at gate a/b
		UpdateDoors();
		UpdateEvents();
		// TODO UpdateScreens, TimeCheckpointMonitors, Update294, UpdateRoomLights
		UpdateDecals();
		// TODO UpdateMTF
		UpdateNPCs();
		UpdateItems();
		// TODO Use427, UpdateMonitorSaving, devil emitters
		UpdateParticles();
		UpdateRooms();
	}
	if (InfiniteStamina) Stamina = Min(100, Stamina + (100 - Stamina) * .01f * FPSfactor);
	update_world(FPSfactor == 0 ? 0 : 1);
	// TODO ManipulateNPCBones
	RenderWorld2();
	BlurVolume = Min(CurveValue(0, BlurVolume, 20), .95f);
	if (BlurTimer > 0) { BlurVolume = Max(Min(.95f, BlurTimer / 1000), BlurVolume); BlurTimer = Max(BlurTimer - FPSfactor, 0); }
	UpdateBlur(BlurVolume);
	darkness();
	if (LightFlash > 0) {
		show_entity(Light);
		entity_alpha(Light, Max(Min(LightFlash + frnd(-.2f, .2f), 1), 0));
		LightFlash = Max(LightFlash - FPSfactor / 70, 0);
	} else hide_entity(Light);
	entity_color(Light, 255, 255, 255);
	InventoryKey();
	// TODO quick save, console key
	DrawGUI();
	if (EndingTimer >= 0 && MenuOpen) pausestub(); // TODO DrawEnding when EndingTimer < 0
	// TODO UpdateConsole, the 173 intro document hint
	DrawMessages();
	set_color(255, 255, 255);
	if (ShowFPS) { AASetFont(ConsoleFont); AAText(20, 20, cat("FPS: ", s_str(FPS)), 0, 0); AASetFont(Font1); }
	DrawQuickLoading();
	UpdateAchievementMsg();
}

void MainLoop(void) {
	int CheckFPS = 0, ElapsedLoops = 0;
	for (;;) {
		clear_screen();
		CurTime = MilliSecs2();
		float ElapsedTime = (CurTime - PrevTime) / 1000.0f;
		PrevTime = CurTime;
		PrevFPSFactor = FPSfactor;
		FPSfactor = Max(Min(ElapsedTime * 70, 5), .2f);
		FPSfactor2 = FPSfactor;
		// TODO SelectedDoor, SelectedScreen, Using294 once those exist
		if (MenuOpen || InvOpen || ConsoleOpen || OtherOpen) FPSfactor = 0;
		if (Framelimit > 0) {
			delay_ms(iround(1000.0f / Framelimit - (MilliSecs2() - LoopDelay)));
			LoopDelay = MilliSecs2();
		}
		if (CheckFPS < MilliSecs2()) { FPS = ElapsedLoops; ElapsedLoops = 0; CheckFPS = MilliSecs2() + 1000; }
		ElapsedLoops++;

		if (Input_ResetTime <= 0) {
			DoubleClick = 0;
			MouseHit1 = mouse_hit(1);
			if (MouseHit1) {
				if (MilliSecs2() - LastMouseHit1 < 800) DoubleClick = 1;
				LastMouseHit1 = MilliSecs2();
			}
			int prev = MouseDown1;
			MouseDown1 = mouse_down(1);
			MouseUp1 = prev && !MouseDown1;
			MouseHit2 = mouse_hit(2);
			if (!MouseDown1 && !MouseHit1) GrabbedEntity = 0;
		} else Input_ResetTime = Max(Input_ResetTime - FPSfactor, 0);

		UpdateMusic();
		if (EnableSFXRelease) AutoReleaseSounds();

		if (MainMenuOpen) {
			if (ShouldPlay == 21) {
				EndBreathSFX = load_sound("SFX\\Ending\\MenuBreath.ogg");
				EndBreathCHN = play_sound(EndBreathSFX);
				ShouldPlay = 66;
			} else if (ShouldPlay == 66) {
				if (!channel_playing(EndBreathCHN)) { free_sound(EndBreathSFX); ShouldPlay = 11; }
			} else ShouldPlay = 11;
			UpdateMainMenu();
		} else {
			UpdateGame();
		}
		// TODO borderless upscale + ScreenGamma (fresize_*), CatchErrors
		flip_frame(Vsync != 0);
	}
}
