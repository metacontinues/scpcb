// loop.c: Main.bb's main loop. the menu side works, the in-game side isn't ported yet
#include "g.h"

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
			MainMenuOpen = 1;
		}
		// TODO borderless upscale + ScreenGamma (fresize_*), CatchErrors
		bbFlip(Vsync != 0);
	}
}
