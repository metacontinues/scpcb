// entities.c: Main.bb's LoadEntities. TODO: particle templates + templates, SetChunkDataValues, user tracks
#include "g.h"

static int hid(int e) { hide_entity(e); return e; }
static int filled(int w, int h, int flags, int r, int g, int b) {
	int t = create_texture(w, h, flags, 1);
	texture_fill(t, r, g, b, 255);
	return t;
}
// full screen sprite on the blur cam (fog vignette, gas mask, infect, nv)
static int overlay(int tex, float w, float h, int blend, int order, int fx) {
	int s = create_sprite(ark_blur_cam);
	scale_sprite(s, Max(GraphicWidth / w, 1), Max(GraphicHeight / h * .8f, .8f));
	entity_texture(s, tex, 0, 0);
	entity_blend(s, blend);
	if (fx) entity_fx(s, 1);
	entity_order(s, order);
	move_entity(s, 0, 0, 1);
	return s;
}
// monitor_checkpoint: every surface but the screen gets the "off" texture
static void monitoroff(int m) {
	for (int i = 2; i <= count_surfaces(m); i++) {
		surf *sf = get_surface(m, i);
		hbrush b = get_surface_brush(sf);
		if (!b) continue;
		htex t1 = get_brush_texture(b, 0);
		if (t1) {
			str name = StripPath(texture_name(t1));
			if (!s_eq(s_lower(name), "monitortexture.jpg")) { brush_texture(b, MonitorTextureOff, 0, 0); paint_surface(sf, b); }
			if (s_len(name)) free_texture(t1);
		}
		free_brush(b);
	}
}

void LoadEntities(void) {
	DrawLoading(0, 0);
	for (int i = 0; i <= 9; i++) TempSounds[i] = 0;
	PauseMenuIMG = LoadImage_Strict("GFX\\menu\\pausemenu.jpg");
	mask_image(PauseMenuIMG, 255, 255, 0);
	scale_image(PauseMenuIMG, MenuScale, MenuScale);
	SprintIcon = LoadImage_Strict("GFX\\sprinticon.png");
	BlinkIcon = LoadImage_Strict("GFX\\blinkicon.png");
	CrouchIcon = LoadImage_Strict("GFX\\sneakicon.png");
	HandIcon = LoadImage_Strict("GFX\\handsymbol.png");
	HandIcon2 = LoadImage_Strict("GFX\\handsymbol2.png");
	StaminaMeterIMG = LoadImage_Strict("GFX\\staminameter.jpg");
	KeypadHUD = LoadImage_Strict("GFX\\keypadhud.jpg"); mask_image(KeypadHUD, 255, 0, 255);
	Panel294 = LoadImage_Strict("GFX\\294panel.jpg"); mask_image(Panel294, 255, 0, 255);
	Brightness = iround(GetINIFloat("options.ini", "options", "brightness", 0));
	CameraFogNear = GetINIFloat("options.ini", "options", "camera fog near", 0);
	CameraFogFar = GetINIFloat("options.ini", "options", "camera fog far", 0);
	StoredCameraFogFar = CameraFogFar;

	AmbientLightRoomTex = filled(2, 2, 257, 0, 0, 0);
	texture_blend(AmbientLightRoomTex, 5);
	AmbientLightRoomVal = 0;
	SoundEmitter = create_pivot(0);
	Camera = create_camera(0);
	camera_viewport(Camera, 0, 0, GraphicWidth, GraphicHeight);
	camera_range(Camera, .05f, CameraFogFar);
	camera_fog_mode(Camera, 1);
	camera_fog_range(Camera, CameraFogNear, CameraFogFar);
	camera_fog_color(Camera, GetINIInt("options.ini", "options", "fog r", 0), GetINIInt("options.ini", "options", "fog g", 0), GetINIInt("options.ini", "options", "fog b", 0));
	ambient_light(Brightness, Brightness, Brightness);
	ScreenTexs[0] = create_texture(512, 512, 1 + 256, 1);
	ScreenTexs[1] = create_texture(512, 512, 1 + 256, 1);
	CreateBlurImage();
	camera_proj_mode(ark_blur_cam, 0);
	FogTexture = LoadTexture_Strict("GFX\\fog.jpg", 1);
	Fog = overlay(FogTexture, 1240.0f, 960.0f, 2, -1000, 0);
	GasMaskTexture = LoadTexture_Strict("GFX\\GasmaskOverlay.jpg", 1);
	GasMaskOverlay = hid(overlay(GasMaskTexture, 1024.0f, 1024.0f, 2, -1003, 1));
	InfectTexture = LoadTexture_Strict("GFX\\InfectOverlay.jpg", 1);
	InfectOverlay = hid(overlay(InfectTexture, 1024.0f, 1024.0f, 3, -1003, 1));
	NVTexture = LoadTexture_Strict("GFX\\NightVisionOverlay.jpg", 1);
	NVOverlay = hid(overlay(NVTexture, 1024.0f, 1024.0f, 2, -1003, 1));
	NVBlink = create_sprite(ark_blur_cam);
	scale_sprite(NVBlink, Max(GraphicWidth / 1024.0f, 1), Max(GraphicHeight / 1024.0f * .8f, .8f));
	entity_color(NVBlink, 0, 0, 0);
	entity_fx(NVBlink, 1);
	entity_order(NVBlink, -1005);
	move_entity(NVBlink, 0, 0, 1);
	hide_entity(NVBlink);
	FogNVTexture = LoadTexture_Strict("GFX\\fogNV.jpg", 1);
	DrawLoading(5, 0);

	DarkTexture = filled(1024, 1024, 1 + 2, 0, 0, 0);
	Dark = create_sprite(Camera);
	scale_sprite(Dark, Max(GraphicWidth / 1240.0f, 1), Max(GraphicHeight / 960.0f * .8f, .8f));
	entity_texture(Dark, DarkTexture, 0, 0);
	entity_blend(Dark, 1); entity_order(Dark, -1002);
	move_entity(Dark, 0, 0, 1);
	entity_alpha(Dark, 0);
	LightTexture = filled(1024, 1024, 1 + 2 + 256, 255, 255, 255);
	TeslaTexture = LoadTexture_Strict("GFX\\map\\tesla.jpg", 1 + 2);
	Light = create_sprite(Camera);
	scale_sprite(Light, Max(GraphicWidth / 1240.0f, 1), Max(GraphicHeight / 960.0f * .8f, .8f));
	entity_texture(Light, LightTexture, 0, 0);
	entity_blend(Light, 1); entity_order(Light, -1002);
	move_entity(Light, 0, 0, 1);
	hide_entity(Light);

	Collider = create_pivot(0);
	entity_radius(Collider, .15f, .3f);
	entity_pick_mode(Collider, 1, 1);
	entity_type(Collider, HIT_PLAYER, 0);
	Head = create_pivot(0);
	entity_radius(Head, .15f, .15f);
	entity_type(Head, HIT_PLAYER, 0);

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

	for (int i = 0; i <= 5; i++) GorePics[i] = LoadTexture_Strict(cat("GFX\\895pics\\pic", s_str(i + 1), ".jpg"), 1);
	OldAiPics[0] = LoadTexture_Strict("GFX\\AIface.jpg", 1);
	OldAiPics[1] = LoadTexture_Strict("GFX\\AIface2.jpg", 1);
	DrawLoading(20, 0);
	for (int i = 0; i <= 6; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\decal", s_str(i + 1), ".png"), 3);
	DecalTextures[7] = LoadTexture_Strict("GFX\\items\\INVpaperstrips.jpg", 3);
	for (int i = 8; i <= 12; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\decalpd", s_str(i - 7), ".jpg"), 3);
	for (int i = 13; i <= 14; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\bullethole", s_str(i - 12), ".jpg"), 3);
	for (int i = 15; i <= 16; i++) DecalTextures[i] = LoadTexture_Strict(cat("GFX\\blooddrop", s_str(i - 14), ".png"), 3);
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
		DTextures[i] = hid(copy_entity(ClassDObj, 0));
		int tex = LoadTexture_Strict(cat("GFX\\npcs\\", dt[i], ".jpg"), 1);
		entity_texture(DTextures[i], tex, 0, 0);
		free_texture(tex);
	}
	InitItemTemplates();
	LoadParticleTextures();
	// TODO SetChunkDataValues (1499 chunks)
	LoadMaterials_("DATA\\materials.ini");
	static const char *mt[] = {"mt1", "mt2", "mt2c", "mt3", "mt4", "mt_elevator", "mt_generator"};
	for (int i = 0; i < 7; i++) OBJTunnel[i] = hid(LoadRMesh(cat("GFX\\map\\", mt[i], ".rmesh"), NULL));
	texture_lod_bias(TextureFloat);
	Room2slCam = create_camera(0);
	camera_viewport(Room2slCam, 0, 0, 128, 128);
	camera_range(Room2slCam, .05f, 6);
	camera_zoom(Room2slCam, .8f);
	hide_entity(Room2slCam);
	DrawLoading(30, 0);
}
