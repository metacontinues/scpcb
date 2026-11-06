// entities.c: Main.bb's LoadEntities. TODO: the ark_blur_cam overlays (fog, gas mask, infect, nv),
// particles + templates, SetChunkDataValues, user tracks
#include "g.h"

static int hid(int e) { bbHideEntity(e); return e; }
static int filled(int w, int h, int flags, int r, int g, int b) {
	int t = bbCreateTexture(w, h, flags, 1);
	bbTextureFill(t, r, g, b, 255);
	return t;
}
// monitor_checkpoint: every surface but the screen gets the "off" texture
static void monitoroff(int m) {
	for (int i = 2; i <= bbCountSurfaces(m); i++) {
		surf *sf = bbGetSurface(m, i);
		hbrush b = bbGetSurfaceBrush(sf);
		if (!b) continue;
		htex t1 = bbGetBrushTexture(b, 0);
		if (t1) {
			if (!bbSeq(bbLower(StripPath(bbTextureName(t1))), "monitortexture.jpg")) { bbBrushTexture(b, MonitorTextureOff, 0, 0); bbPaintSurface(sf, b); }
		}
		bbFreeBrush(b);
	}
}

void LoadEntities(void) {
	DrawLoading(0, 0);
	for (int i = 0; i <= 9; i++) TempSounds[i] = 0;
	PauseMenuIMG = LoadImage_Strict("GFX\\menu\\pausemenu.jpg");
	bbMaskImage(PauseMenuIMG, 255, 255, 0);
	bbScaleImage(PauseMenuIMG, MenuScale, MenuScale);
	SprintIcon = LoadImage_Strict("GFX\\sprinticon.png");
	BlinkIcon = LoadImage_Strict("GFX\\blinkicon.png");
	CrouchIcon = LoadImage_Strict("GFX\\sneakicon.png");
	HandIcon = LoadImage_Strict("GFX\\handsymbol.png");
	HandIcon2 = LoadImage_Strict("GFX\\handsymbol2.png");
	StaminaMeterIMG = LoadImage_Strict("GFX\\staminameter.jpg");
	KeypadHUD = LoadImage_Strict("GFX\\keypadhud.jpg"); bbMaskImage(KeypadHUD, 255, 0, 255);
	Panel294 = LoadImage_Strict("GFX\\294panel.jpg"); bbMaskImage(Panel294, 255, 0, 255);
	Brightness = bbInt(GetINIFloat("options.ini", "options", "brightness", 0));
	CameraFogNear = GetINIFloat("options.ini", "options", "camera fog near", 0);
	CameraFogFar = GetINIFloat("options.ini", "options", "camera fog far", 0);
	StoredCameraFogFar = CameraFogFar;

	AmbientLightRoomTex = filled(2, 2, 257, 0, 0, 0);
	bbTextureBlend(AmbientLightRoomTex, 5);
	AmbientLightRoomVal = 0;
	SoundEmitter = bbCreatePivot(0);
	Camera = bbCreateCamera(0);
	bbCameraViewport(Camera, 0, 0, GraphicWidth, GraphicHeight);
	bbCameraRange(Camera, .05f, CameraFogFar);
	bbCameraFogMode(Camera, 1);
	bbCameraFogRange(Camera, CameraFogNear, CameraFogFar);
	bbCameraFogColor(Camera, GetINIInt("options.ini", "options", "fog r", 0), GetINIInt("options.ini", "options", "fog g", 0), GetINIInt("options.ini", "options", "fog b", 0));
	bbAmbientLight(Brightness, Brightness, Brightness);
	ScreenTexs[0] = bbCreateTexture(512, 512, 1 + 256, 1);
	ScreenTexs[1] = bbCreateTexture(512, 512, 1 + 256, 1);
	FogTexture = LoadTexture_Strict("GFX\\fog.jpg", 1);
	GasMaskTexture = LoadTexture_Strict("GFX\\GasmaskOverlay.jpg", 1);
	InfectTexture = LoadTexture_Strict("GFX\\InfectOverlay.jpg", 1);
	NVTexture = LoadTexture_Strict("GFX\\NightVisionOverlay.jpg", 1);
	FogNVTexture = LoadTexture_Strict("GFX\\fogNV.jpg", 1);
	DrawLoading(5, 0);

	DarkTexture = filled(1024, 1024, 1 + 2, 0, 0, 0);
	Dark = bbCreateSprite(Camera);
	bbScaleSprite(Dark, Max(GraphicWidth / 1240.0f, 1), Max(GraphicHeight / 960.0f * .8f, .8f));
	bbEntityTexture(Dark, DarkTexture, 0, 0);
	bbEntityBlend(Dark, 1); bbEntityOrder(Dark, -1002);
	bbMoveEntity(Dark, 0, 0, 1);
	bbEntityAlpha(Dark, 0);
	LightTexture = filled(1024, 1024, 1 + 2 + 256, 255, 255, 255);
	TeslaTexture = LoadTexture_Strict("GFX\\map\\tesla.jpg", 1 + 2);
	Light = bbCreateSprite(Camera);
	bbScaleSprite(Light, Max(GraphicWidth / 1240.0f, 1), Max(GraphicHeight / 960.0f * .8f, .8f));
	bbEntityTexture(Light, LightTexture, 0, 0);
	bbEntityBlend(Light, 1); bbEntityOrder(Light, -1002);
	bbMoveEntity(Light, 0, 0, 1);
	bbHideEntity(Light);

	Collider = bbCreatePivot(0);
	bbEntityRadius(Collider, .15f, .3f);
	bbEntityPickMode(Collider, 1, 1);
	bbEntityType(Collider, HIT_PLAYER, 0);
	Head = bbCreatePivot(0);
	bbEntityRadius(Head, .15f, .15f);
	bbEntityType(Head, HIT_PLAYER, 0);

	LiquidObj = hid(LoadMesh_Strict("GFX\\items\\cupliquid.x", 0));
	MTFObj = hid(LoadAnimMesh_Strict("GFX\\npcs\\MTF2.b3d", 0));
	GuardObj = hid(LoadAnimMesh_Strict("GFX\\npcs\\guard.b3d", 0));
	ClassDObj = hid(LoadAnimMesh_Strict("GFX\\npcs\\classd.b3d", 0));
	ApacheObj = hid(LoadAnimMesh_Strict("GFX\\apache.b3d", 0));
	ApacheRotorObj = hid(LoadAnimMesh_Strict("GFX\\apacherotor.b3d", 0));
	NPC049OBJ = hid(LoadAnimMesh_Strict("GFX\\npcs\\scp-049.b3d", 0));
	NPC0492OBJ = hid(LoadAnimMesh_Strict("GFX\\npcs\\zombie1.b3d", 0));
	ClerkOBJ = hid(LoadAnimMesh_Strict("GFX\\npcs\\clerk.b3d", 0));
	LightSpriteTex[0] = LoadTexture_Strict("GFX\\light1.jpg", 1);
	LightSpriteTex[1] = LoadTexture_Strict("GFX\\light2.jpg", 1);
	LightSpriteTex[2] = LoadTexture_Strict("GFX\\lightsprite.jpg", 1);
	DrawLoading(10, 0);

	DoorOBJ = hid(LoadMesh_Strict("GFX\\map\\door01.x", 0));
	DoorFrameOBJ = hid(LoadMesh_Strict("GFX\\map\\doorframe.x", 0));
	HeavyDoorObj[0] = hid(LoadMesh_Strict("GFX\\map\\heavydoor1.x", 0));
	HeavyDoorObj[1] = hid(LoadMesh_Strict("GFX\\map\\heavydoor2.x", 0));
	DoorColl = hid(LoadMesh_Strict("GFX\\map\\doorcoll.x", 0));
	ButtonOBJ = hid(LoadMesh_Strict("GFX\\map\\Button.x", 0));
	ButtonKeyOBJ = hid(LoadMesh_Strict("GFX\\map\\ButtonKeycard.x", 0));
	ButtonCodeOBJ = hid(LoadMesh_Strict("GFX\\map\\ButtonCode.x", 0));
	ButtonScannerOBJ = hid(LoadMesh_Strict("GFX\\map\\ButtonScanner.x", 0));
	BigDoorOBJ[0] = hid(LoadMesh_Strict("GFX\\map\\ContDoorLeft.x", 0));
	BigDoorOBJ[1] = hid(LoadMesh_Strict("GFX\\map\\ContDoorRight.x", 0));
	LeverBaseOBJ = hid(LoadMesh_Strict("GFX\\map\\leverbase.x", 0));
	LeverOBJ = hid(LoadMesh_Strict("GFX\\map\\leverhandle.x", 0));
	DrawLoading(15, 0);

	for (int i = 0; i <= 5; i++) GorePics[i] = LoadTexture_Strict(cat("GFX\\895pics\\pic", bbStr(i + 1), ".jpg"), 1);
	OldAiPics[0] = LoadTexture_Strict("GFX\\AIface.jpg", 1);
	OldAiPics[1] = LoadTexture_Strict("GFX\\AIface2.jpg", 1);
	DrawLoading(20, 0);
	for (int i = 0; i <= 6; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\decal", bbStr(i + 1), ".png"), 3);
	DecalTextures[7] = LoadTexture_Strict("GFX\\items\\INVpaperstrips.jpg", 3);
	for (int i = 8; i <= 12; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\decalpd", bbStr(i - 7), ".jpg"), 3);
	for (int i = 13; i <= 14; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\bullethole", bbStr(i - 12), ".jpg"), 3);
	for (int i = 15; i <= 16; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\blooddrop", bbStr(i - 14), ".png"), 3);
	DecalTextures[17] = LoadTexture_Strict("GFX\\decal8.png", 3);
	DecalTextures[18] = LoadTexture_Strict("GFX\\decalpd6.dc", 3);
	DecalTextures[19] = LoadTexture_Strict("GFX\\decal19.png", 3);
	DecalTextures[20] = LoadTexture_Strict("GFX\\decal427.png", 3);
	DrawLoading(25, 0);

	Monitor = hid(LoadMesh_Strict("GFX\\map\\monitor.b3d", 0));
	MonitorTexture = LoadTexture_Strict("GFX\\monitortexture.jpg", 1);
	CamBaseOBJ = hid(LoadMesh_Strict("GFX\\map\\cambase.x", 0));
	CamOBJ = hid(LoadMesh_Strict("GFX\\map\\CamHead.b3d", 0));
	Monitor2 = hid(LoadMesh_Strict("GFX\\map\\monitor_checkpoint.b3d", 0));
	Monitor3 = hid(LoadMesh_Strict("GFX\\map\\monitor_checkpoint.b3d", 0));
	MonitorTexture2 = LoadTexture_Strict("GFX\\map\\LockdownScreen2.jpg", 1);
	MonitorTexture3 = LoadTexture_Strict("GFX\\map\\LockdownScreen.jpg", 1);
	MonitorTexture4 = LoadTexture_Strict("GFX\\map\\LockdownScreen3.jpg", 1);
	MonitorTextureOff = filled(1, 1, 1, 0, 0, 0);
	LightConeModel = hid(LoadMesh_Strict("GFX\\lightcone.b3d", 0));
	monitoroff(Monitor2);
	monitoroff(Monitor3);

	static const char *dt[] = {0, "gonzales", "corpse", "scientist", "scientist2", "janitor", "106victim", "classd2", "035victim"};
	for (int i = 1; i <= 8; i++) {
		DTextures[i] = hid(bbCopyEntity(ClassDObj, 0));
		int tex = LoadTexture_Strict(cat("GFX\\npcs\\", dt[i], ".jpg"), 1);
		bbEntityTexture(DTextures[i], tex, 0, 0);
		bbFreeTexture(tex);
	}
	InitItemTemplates();
	LoadParticleTextures();
	// TODO SetChunkDataValues (1499 chunks)
	LoadMaterials_("DATA\\materials.ini");
	static const char *mt[] = {"mt1", "mt2", "mt2c", "mt3", "mt4", "mt_elevator", "mt_generator"};
	for (int i = 0; i < 7; i++) OBJTunnel[i] = hid(LoadRMesh(cat("GFX\\map\\", mt[i], ".rmesh"), NULL));
	bbTextureLodBias(TextureFloat);
	Room2slCam = bbCreateCamera(0);
	bbCameraViewport(Room2slCam, 0, 0, 128, 128);
	bbCameraRange(Room2slCam, .05f, 6);
	bbCameraZoom(Room2slCam, .8f);
	bbHideEntity(Room2slCam);
	DrawLoading(30, 0);
}
