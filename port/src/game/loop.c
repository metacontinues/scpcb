// loop.c: Main.bb's main loop. the menu side works, the in-game side isn't ported yet
#include "g.h"

// stand-in for the game until MovePlayer & co are ported: fly around the generated map.
// wasd + mouse, shift fast, esc back to the menu
static void FlyCam(void) {
	static int init;
	if (!init) {
		init = 1;
		each(Rooms, r) bbShowEntity(r->obj);
		each(Doors, d) { bbShowEntity(d->obj); if (d->obj2) bbShowEntity(d->obj2); bbShowEntity(d->frameobj); }
		Rooms *st = bbFirst(Rooms);
		each(Rooms, r) if (r->RoomTemplate && bbSeq(r->RoomTemplate->Name, "start")) st = r;
		if (st) bbPositionEntity(Camera, st->x, 1.2f, st->z, 0);
		bbCameraRange(Camera, .05f, 400);
		if (getenv("SCPCB_DUMP")) each(Rooms, r) fprintf(stderr, "%s %g %g %g / %g %g %g a%d\n", r->RoomTemplate ? r->RoomTemplate->Name : "?", r->x, r->y, r->z, bbEntityX(r->obj, 1), bbEntityY(r->obj, 1), bbEntityZ(r->obj, 1), r->angle);
		if (getenv("SCPCB_TOPDOWN")) { // dev: whole map from above (ceilings are back-face culled)
			bbPositionEntity(Camera, MapWidth * 4, 170, MapHeight * 4, 0);
			bbRotateEntity(Camera, atof(getenv("SCPCB_TOPDOWN")) ? atof(getenv("SCPCB_TOPDOWN")) : 90, 0, 0, 0);
		}
		bbCameraFogMode(Camera, 0);
		bbHidePointer();
	}
	float sp = (bbKeyDown(42) ? .3f : .08f) * FPSfactor;
	bbTurnEntity(Camera, 0, -bbMouseXSpeed() * .3f, 0, 1);
	bbTurnEntity(Camera, bbMouseYSpeed() * .3f, 0, 0, 0);
	bbMoveEntity(Camera, (bbKeyDown(KEY_RIGHT) - bbKeyDown(KEY_LEFT)) * sp, 0, (bbKeyDown(KEY_UP) - bbKeyDown(KEY_DOWN)) * sp);
	bbMoveMouse(GraphicWidth / 2, GraphicHeight / 2);
	bbMouseXSpeed(); bbMouseYSpeed();
	bbRenderWorld(1);
	AASetFont(ConsoleFont);
	bbColor(255, 255, 255);
	int n = 0; each(Rooms, r) n++;
	AAText(10, 10, cat("map viewer, the game isn't ported yet. seed ", RandomSeed, ", ", bbStr(n), " rooms. esc = menu"), 0, 0);
	if (bbKeyHit(1)) { MainMenuOpen = 1; bbShowPointer(); init = 0; }
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
			// TODO the game: UpdateStreamSounds .. DrawGUI/DrawMenu/UpdateConsole/messages/achievements
			FlyCam();
		}
		// TODO borderless upscale + ScreenGamma (fresize_*), CatchErrors
		bbFlip(Vsync != 0);
	}
}
