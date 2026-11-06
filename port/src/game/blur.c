// blur.c: Dreamfilter.bb (the motion blur overlay) + Main.bb's RenderWorld2
#include "g.h"

void CreateBlurImage(void) {
	int cam = ark_blur_cam = bbCreateCamera(0);
	bbCameraProjMode(cam, 2);
	bbCameraZoom(cam, .1f);
	bbCameraClsMode(cam, 0, 0);
	bbCameraRange(cam, .1f, 1.5f);
	bbMoveEntity(cam, 0, 0, 10000);
	ark_sw = GraphicWidth; ark_sh = GraphicHeight;
	bbCameraViewport(cam, 0, 0, ark_sw, ark_sh);
	int spr = bbCreateMesh(cam);
	surf *sf = bbCreateSurface(spr, 0);
	bbAddVertex(sf, -1, 1, 0, 0, 0, 0); bbAddVertex(sf, 1, 1, 0, 1, 0, 0);
	bbAddVertex(sf, -1, -1, 0, 0, 1, 0); bbAddVertex(sf, 1, -1, 0, 1, 1, 0);
	bbAddTriangle(sf, 0, 1, 2); bbAddTriangle(sf, 3, 2, 1);
	bbEntityFX(spr, 17);
	bbScaleEntity(spr, 2048.0f / ark_sw, 2048.0f / ark_sw, 1, 0);
	bbPositionEntity(spr, 0, 0, 1.0001f, 0);
	bbEntityOrder(spr, -100000);
	bbEntityBlend(spr, 1);
	ark_blur_image = spr;
	ark_blur_texture = bbCreateTexture(2048, 2048, 256, 1);
	bbEntityTexture(spr, ark_blur_texture, 0, 0);
}

// last frame stays on the overlay at `power` alpha. TODO the copy is a cpu readback, glCopyTexSubImage2D would do
void UpdateBlur(float power) {
	bbEntityAlpha(ark_blur_image, power);
	bbCopyRect(0, 0, GraphicWidth, GraphicHeight, bbInt(1024.0f - GraphicWidth / 2), bbInt(1024.0f - GraphicHeight / 2), bbBackBuffer(), bbTextureBuffer(ark_blur_texture, 0));
}

void RenderWorld2(void) {
	bbCameraProjMode(ark_blur_cam, 0);
	bbCameraProjMode(Camera, 1);
	if (WearingNightVision > 0 && WearingNightVision < 3) { float b = Min(Brightness * 2, 255); bbAmbientLight(b, b, b); }
	else if (WearingNightVision == 3) bbAmbientLight(255, 255, 255);
	else if (PlayerRoom) {
		str n = PlayerRoom->RoomTemplate->Name;
		if (!bbSeq(n, "173") && !bbSeq(n, "exit1") && !bbSeq(n, "gatea")) bbAmbientLight(Brightness, Brightness, Brightness);
	}
	IsNVGBlinking = 0;
	bbHideEntity(NVBlink);
	bbCameraViewport(Camera, 0, 0, GraphicWidth, GraphicHeight);
	int hasBattery = 2, power = 0;
	if (WearingNightVision == 1 || WearingNightVision == 2) {
		for (int i = 0; i < MaxItemAmount; i++) {
			Items *it = Inventory[i];
			if (!it) continue;
			str t = it->itemtemplate->tempname;
			if (!((WearingNightVision == 1 && bbSeq(t, "nvgoggles")) || (WearingNightVision == 2 && bbSeq(t, "supernv")))) continue;
			it->state -= FPSfactor * (.02f * WearingNightVision);
			power = bbInt(it->state);
			if (it->state <= 0) {
				hasBattery = 0;
				Msg = "The batteries in these night vision goggles died.";
				BlinkTimer = -1; MsgTimer = 350;
				break;
			} else if (it->state <= 100) hasBattery = 1;
		}
		if (hasBattery) bbRenderWorld(1);
	} else bbRenderWorld(1);
	CurrTrisAmount = bbTrisRendered();
	if (hasBattery == 0 && WearingNightVision != 3) { IsNVGBlinking = 1; bbShowEntity(NVBlink); }
	if (BlinkTimer < -16 || BlinkTimer > -6) {
		// TODO WearingNightVision == 2: the "REFRESHING DATA" npc readout (needs NPCs)
		if ((WearingNightVision == 1 || WearingNightVision == 2) && hasBattery) {
			if (WearingNightVision == 2) bbColor(0, 0, 55); else bbColor(0, 55, 0);
			for (int k = 0; k <= 10; k++) bbRect(45, bbInt(GraphicHeight * .5f - k * 20), 54, 10, 1);
			if (WearingNightVision == 2) bbColor(0, 0, 255); else bbColor(0, 255, 0);
			for (int l = 0; l <= (int)floorf((power + 50) * .01f); l++) bbRect(45, bbInt(GraphicHeight * .5f - l * 20), 54, 10, 1);
			bbDrawImage(NVGImages, 40, bbInt(GraphicHeight * .5f + 30), WearingNightVision == 2);
			bbColor(255, 255, 255);
		}
	}
	bbCameraProjMode(ark_blur_cam, 2);
	bbCameraProjMode(Camera, 0);
	bbRenderWorld(1);
	bbCameraProjMode(ark_blur_cam, 0);
	if ((BlinkTimer < -16 || BlinkTimer > -6) && (WearingNightVision == 1 || WearingNightVision == 2) && hasBattery == 1 && MilliSecs2() % 800 < 400) {
		bbColor(255, 0, 0);
		AASetFont(Font3);
		AAText(GraphicWidth / 2, bbInt(20 * MenuScale), "WARNING: LOW BATTERY", 1, 0);
		bbColor(255, 255, 255);
	}
}
