// loop.c: Main.bb's main loop. the menu side works, the in-game side isn't ported yet
#include "g.h"

static void ambience(void) {
	str rn = PlayerRoom->RoomTemplate->Name;
	if (bbSeq(rn, "pocketdimension") || bbSeq(rn, "gatea") || bbSeq(rn, "exit1") || MenuOpen || ConsoleOpen || InvOpen) return;
	if (bbRand(1, 1500) == 1) {
		for (int i = 0; i <= 5; i++) if (AmbientSFX[i][CurrAmbientSFX] && !bbChannelPlaying(AmbientSFXCHN)) { FreeSound_Strict(AmbientSFX[i][CurrAmbientSFX]); AmbientSFX[i][CurrAmbientSFX] = 0; }
		float rx = bbRnd(-1, 1), rz = bbRnd(-1, 1); // x then z (blitz codegen order, the args tie)
		bbPositionEntity(SoundEmitter, bbEntityX(Camera, 0) + rx, 0, bbEntityZ(Camera, 0) + rz, 0);
		if (bbRand(1, 3) == 1) PlayerZone = 3;
		if (bbSeq(rn, "173")) PlayerZone = 4;
		// TODO room860: PlayerZone 5 + emitter at y 30 while its event's EventState = 1
		CurrAmbientSFX = bbRand(0, AmbientSFXAmount[PlayerZone] - 1);
		static const char *dir[] = {"Zone1", "Zone2", "Zone3", "General", "Pre-breach", "Forest"};
		int *a = &AmbientSFX[PlayerZone][CurrAmbientSFX];
		if (!*a) *a = LoadSound_Strict(sf("SFX\\Ambient\\%s\\ambient%d.ogg", PlayerZone <= 2 ? sf("Zone%d", PlayerZone + 1) : dir[PlayerZone], CurrAmbientSFX + 1));
		AmbientSFXCHN = PlaySound2(*a, Camera, SoundEmitter, 10, 1);
	}
	UpdateSoundOrigin(AmbientSFXCHN, Camera, SoundEmitter, 10, 1);
	if (bbRand(1, 50000) == 3 && !bbSeq(rn, "room860") && !bbSeq(rn, "room1123") && !bbSeq(rn, "173") && !bbSeq(rn, "dimension1499")) {
		if (FPSfactor > 0) LightBlink = bbRnd(1, 2);
		PlaySound_Strict(LoadTempSound(sf("SFX\\SCP\\079\\Broadcast%d.ogg", bbRand(1, 7))));
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
			case EASY: BLINKFREQ = bbRnd(490, 700); break;
			case NORMAL: BLINKFREQ = bbRnd(455, 665); break;
			case HARD: BLINKFREQ = bbRnd(420, 630); break;
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
	if (LightBlink > 0) darkA = Min(Max(darkA, LightBlink * bbRnd(.3f, .8f)), 1);
	// TODO Using294: darkA = 1
	if (!WearingNightVision) darkA = Max((1 - SecondaryLightOn) * .9f, darkA);
	if (KillTimer < 0) {
		InvOpen = 0; SelectedItem = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
		BlurTimer = fabsf(KillTimer * 5);
		KillTimer -= FPSfactor * .8f;
		if (KillTimer < -360) { MenuOpen = 1; if (bbLen(SelectedEnding)) EndingTimer = Min(KillTimer, -.1f); }
		darkA = Max(darkA, Min(fabsf(KillTimer / 400), 1));
	}
	if (FallTimer < 0) {
		// TODO drop a half-worn vest/hazmat (DropItem)
		InvOpen = 0; SelectedItem = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
		BlurTimer = fabsf(FallTimer * 10);
		FallTimer -= FPSfactor;
		darkA = Max(darkA, Min(fabsf(FallTimer / 400), 1));
	}
	if (SelectedItem && (bbSeq(SelectedItem->itemtemplate->tempname, "navigator") || bbSeq(SelectedItem->itemtemplate->tempname, "nav"))) darkA = Max(darkA, .5f);
	if (SelectedScreen) darkA = Max(darkA, .5f);
	bbEntityAlpha(Dark, darkA);
}

// TODO DrawMenu (Main.bb 7116): until it's ported, a bare pause screen so you can get out
static void pausestub(void) {
	bbColor(0, 0, 0); bbRect(0, 0, GraphicWidth, GraphicHeight, 1);
	bbColor(255, 255, 255); AASetFont(Font2);
	AAText(GraphicWidth / 2, GraphicHeight / 2 - 40, "PAUSED", 1, 1);
	AASetFont(Font1);
	AAText(GraphicWidth / 2, GraphicHeight / 2 + 20, "(the pause menu isn't ported yet)  esc: resume   q: quit", 1, 1);
	if (bbKeyHit(16)) End();
}

// Main.bb's main loop, in-game side. what isn't ported yet is listed as TODOs in order
void UpdateGame(void) {
	// TODO UpdateStreamSounds
	ShouldPlay = bbInt(Min(PlayerZone, 2));
	DrawHandIcon = 0; RestoreSanity = 1; ShouldEntitiesFall = 1;
	// TODO UpdateSecurityCams
	ambience();
	UpdateCheckpoint1 = UpdateCheckpoint2 = 0;
	if (!MenuOpen && !InvOpen && !SelectedDoor && !ConsoleOpen && !SelectedScreen && EndingTimer >= 0) { // TODO OtherOpen, Using294
		LightVolume = CurveValue(TempLightVolume, LightVolume, 50);
		bbCameraFogRange(Camera, CameraFogNear * LightVolume, CameraFogFar * LightVolume);
		bbCameraFogColor(Camera, 0, 0, 0);
		bbCameraFogMode(Camera, 1);
		bbCameraRange(Camera, .05f, Min(CameraFogFar * LightVolume * 1.5f, 28));
		if (!bbSeq(PlayerRoom->RoomTemplate->Name, "pocketdimension")) bbCameraClsColor(Camera, 0, 0, 0);
		bbAmbientLight(Brightness, Brightness, Brightness);
		PlayerSoundVolume = CurveValue(0, PlayerSoundVolume, 5);
		CanSave = 1;
		// TODO UpdateDeafPlayer, UpdateEmitters
		MouseLook();
		MovePlayer();
		// TODO InFacility = CheckForPlayerInFacility(), dimension1499, UpdateEndings at gate a/b
		UpdateDoors();
		// TODO UpdateEvents, UpdateScreens, TimeCheckpointMonitors, Update294, UpdateRoomLights, UpdateDecals,
		// UpdateMTF, UpdateNPCs, UpdateItems, Use427, UpdateMonitorSaving, devil emitters
		UpdateParticles();
		UpdateRooms();
	}
	if (InfiniteStamina) Stamina = Min(100, Stamina + (100 - Stamina) * .01f * FPSfactor);
	bbUpdateWorld(FPSfactor == 0 ? 0 : 1);
	// TODO ManipulateNPCBones
	RenderWorld2();
	BlurVolume = Min(CurveValue(0, BlurVolume, 20), .95f);
	if (BlurTimer > 0) { BlurVolume = Max(Min(.95f, BlurTimer / 1000), BlurVolume); BlurTimer = Max(BlurTimer - FPSfactor, 0); }
	UpdateBlur(BlurVolume);
	darkness();
	if (LightFlash > 0) {
		bbShowEntity(Light);
		bbEntityAlpha(Light, Max(Min(LightFlash + bbRnd(-.2f, .2f), 1), 0));
		LightFlash = Max(LightFlash - FPSfactor / 70, 0);
	} else bbHideEntity(Light);
	bbEntityColor(Light, 255, 255, 255);
	// TODO inventory key (InvOpen + Pause/ResumeSounds), quick save, console key
	DrawGUI();
	if (bbKeyHit(1) && EndingTimer == 0) { // DrawGUI's tail in blitz. TODO Pause/ResumeSounds, OptionsMenu save, DropItem
		if (MenuOpen || InvOpen) { bbMouseXSpeed(); bbMouseYSpeed(); bbMouseZSpeed(); mouse_x_speed_1 = mouse_y_speed_1 = 0; }
		MenuOpen = !MenuOpen;
		AchievementsMenu = OptionsMenu = QuitMSG = 0;
		SelectedDoor = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
	}
	if (EndingTimer >= 0 && MenuOpen) pausestub(); // TODO DrawEnding when EndingTimer < 0
	// TODO UpdateConsole, the 173 intro document hint
	DrawMessages();
	bbColor(255, 255, 255);
	if (ShowFPS) { AASetFont(ConsoleFont); AAText(20, 20, cat("FPS: ", bbStr(FPS)), 0, 0); AASetFont(Font1); }
	// TODO DrawQuickLoading, UpdateAchievementMsg
}

void MainLoop(void) {
	int CheckFPS = 0, ElapsedLoops = 0;
	for (;;) {
		bbCls();
		CurTime = MilliSecs2();
		float ElapsedTime = (CurTime - PrevTime) / 1000.0f;
		PrevTime = CurTime;
		PrevFPSFactor = FPSfactor;
		FPSfactor = Max(Min(ElapsedTime * 70, 5), .2f);
		FPSfactor2 = FPSfactor;
		// TODO OtherOpen, SelectedDoor, SelectedScreen, Using294 once those exist
		if (MenuOpen || InvOpen || ConsoleOpen) FPSfactor = 0;
		if (Framelimit > 0) {
			bbDelay(bbInt(1000.0f / Framelimit - (MilliSecs2() - LoopDelay)));
			LoopDelay = MilliSecs2();
		}
		if (CheckFPS < MilliSecs2()) { FPS = ElapsedLoops; ElapsedLoops = 0; CheckFPS = MilliSecs2() + 1000; }
		ElapsedLoops++;

		if (Input_ResetTime <= 0) {
			DoubleClick = 0;
			MouseHit1 = bbMouseHit(1);
			if (MouseHit1) {
				if (MilliSecs2() - LastMouseHit1 < 800) DoubleClick = 1;
				LastMouseHit1 = MilliSecs2();
			}
			int prev = MouseDown1;
			MouseDown1 = bbMouseDown(1);
			MouseUp1 = prev && !MouseDown1;
			MouseHit2 = bbMouseHit(2);
			if (!MouseDown1 && !MouseHit1) GrabbedEntity = 0;
		} else Input_ResetTime = Max(Input_ResetTime - FPSfactor, 0);

		UpdateMusic();
		if (EnableSFXRelease) AutoReleaseSounds();

		if (MainMenuOpen) {
			if (ShouldPlay == 21) {
				EndBreathSFX = bbLoadSound("SFX\\Ending\\MenuBreath.ogg");
				EndBreathCHN = bbPlaySound(EndBreathSFX);
				ShouldPlay = 66;
			} else if (ShouldPlay == 66) {
				if (!bbChannelPlaying(EndBreathCHN)) { bbFreeSound(EndBreathSFX); ShouldPlay = 11; }
			} else ShouldPlay = 11;
			UpdateMainMenu();
		} else {
			UpdateGame();
		}
		// TODO borderless upscale + ScreenGamma (fresize_*), CatchErrors
		bbFlip(Vsync != 0);
	}
}
