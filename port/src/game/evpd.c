// evpd.c: the "pocketdimension" event (UpdateEvents.bb) + the faces DrawGUI flashes at you in there (Main.bb 4629)
// EventState: timer for the breathing tunnels. EventState2: 0 starting room, 1 throne/plane/trench rooms, 12-15 the
// pillar room (106 hops pillar to pillar). EventState3: 1 the hcz-looking trick tunnel, 2 its door opened.
// EventStr holds a float as text, like blitz (it does Float(e\EventStr) + ..., so the Str() rounding is kept)
#include "g.h"

#define R (e->room)
#define RS RoomScale
static float XG(int o) { return bbEntityX(o, 1); }
static float YG(int o) { return bbEntityY(o, 1); }
static float ZG(int o) { return bbEntityZ(o, 1); }
static float X(int o) { return bbEntityX(o, 0); }
static float Y(int o) { return bbEntityY(o, 0); }
static float Z(int o) { return bbEntityZ(o, 0); }
static float sind(float a) { return sinf(a * DTOR); }
static float cosd(float a) { return cosf(a * DTOR); }
static float strf(Events *e) { return bbLen(e->EventStr) ? bbFloat(e->EventStr) : 0; }

static void fog(int r, int g, int b) { bbCameraFogColor(Camera, r, g, b); bbCameraClsColor(Camera, r, g, b); }
static void refresh(void) { UpdateDoorsTimer = 0; UpdateDoors(); UpdateRooms(); }

// look at t (the player can't look away from the plane / 106's eyes)
static void stare(int t, float smooth) {
	int pvt = bbCreatePivot(0);
	bbPositionEntity(pvt, X(Camera), Y(Camera), Z(Camera), 0);
	bbPointEntity(pvt, t, 0);
	bbTurnEntity(pvt, 90, 0, 0, 0);
	user_camera_pitch = CurveAngle(bbEntityPitch(pvt, 0), user_camera_pitch + 90, smooth);
	user_camera_pitch -= 90;
	bbRotateEntity(Collider, bbEntityPitch(Collider, 0), CurveAngle(bbEntityYaw(pvt, 0), bbEntityYaw(Collider, 0), smooth), 0, 0);
	bbFreeEntity(pvt);
}

// out of the dimension: room2sl's monitor/lever get reset
static void left(Events *e) {
	GiveAchievement(AchvPD, 1);
	e->EventState = 0; e->EventState2 = 0;
	SecondaryLightOn = PrevSecondaryLightOn; PrevSecondaryLightOn = 0;
	Events *sl = FindEvent("room2sl");
	if (sl) {
		sl->EventState3 = 0;
		UpdateLever(sl->room->Levers[0], 0);
		bbRotateEntity(sl->room->Levers[0], 0, bbEntityYaw(sl->room->Levers[0], 0), 0, 0);
		TurnCheckpointMonitorsOff(0);
	}
}

static void the106(int ox, float y) { // 106 stands at a spot, swaying toward you
	NPCs *n = Curr106;
	n->Idle = 1;
	bbPositionEntity(n->Collider, XG(ox), y, ZG(ox), 0);
	bbPointEntity(n->Collider, Camera, 0);
	bbTurnEntity(n->Collider, 0, sind(MilliSecs2() / 20) * 6, 0, 1);
	bbMoveEntity(n->Collider, 0, 0, sind(MilliSecs2() / 15) * .06f);
}
static void show106(void) {
	NPCs *n = Curr106;
	bbShowEntity(n->obj); bbShowEntity(n->Collider);
	bbResetEntity(n->Collider);
	n->GravityMult = 0; n->DropSpeed = 0;
	bbPositionEntity(n->obj, X(n->Collider), Y(n->Collider) - .15f, Z(n->Collider), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0), 0, 0);
}

static void trenches(Events *e) { // the plane hunts you over the trenches
	int o8 = R->Objects[8], *pl = &R->Objects[20];
	ShouldPlay = 15;
	fog(38, 55, 47);
	if (XG(*pl) < XG(o8) - 4000 * RS) { e->SoundCHN2 = PlaySound_Strict(e->Sound2); bbPositionEntity(*pl, bbEntityX(Collider, 1) + 4000 * RS, 12, bbEntityZ(Collider, 1), 0); }
	bbMoveEntity(Collider, 0, Min(12 - Y(Collider), 0) * FPSfactor, 0);
	float x = -FPSfactor * RS * 4, y = (17 - fabsf(X(Collider) - X(*pl)) * .5f) - Y(*pl), z = bbEntityZ(Collider, 1) - Z(*pl);
	bbTranslateEntity(*pl, x, y, z, 1);
	bbRotateEntity(*pl, -90 - (X(Collider) - X(*pl)) * 1.5f, -90, 0, 1);
	static const float spots[3][2] = {{-1452, -37}, {-121, 188}, {1223, -196}}; // covered spots the plane can't see
	int safe = 0;
	for (int i = 0; i <= 2; i++) if (Distance(X(Collider), Z(Collider), spots[i][0] * RS + XG(o8), spots[i][1] * RS + ZG(o8)) < 200 * RS) { safe = 1; break; }
	float dist = bbEntityDistance(Collider, *pl);
	if (e->SoundCHN2 && bbChannelPlaying(e->SoundCHN2)) e->SoundCHN2 = LoopSound2(e->Sound2, e->SoundCHN2, Camera, Camera, 10, .3f + !safe * .6f);
	if (safe) bbEntityTexture(*pl, R->Objects[18], 0, 0);
	else if (dist < 8) {
		e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, *pl, 8, 1);
		bbEntityTexture(*pl, R->Objects[19], 0, 0);
		Injuries += (8 - dist) * FPSfactor * .0003f;
		if (dist < 7) stare(*pl, 10);
	}
	CameraShake = Max(4 + !safe * 4 - dist, 0);
	if (Y(Collider) < 8.5f) { // the sinkhole out of the trenches -> the exit room
		LoadEventSound(e, "SFX\\Room\\PocketDimension\\Rumble.ogg", 0);
		LoadEventSound(e, "SFX\\Room\\PocketDimension\\PrisonVoices.ogg", 1);
		BlurTimer = 1500;
		e->EventState2 = 1;
		BlinkTimer = -10;
		bbPositionEntity(Collider, XG(o8) - 400 * RS, -304 * RS, ZG(o8), 0);
		bbResetEntity(Collider);
		fog(0, 0, 0);
	}
}

static void throne(Events *e) { // the throne room w/ the moving pillars, 106's eyes, the exit room below
	int o8 = R->Objects[8];
	e->EventState3 = 0;
	for (int i = 9; i <= 10; i++) { // the pillars crush you
		int oi = R->Objects[i];
		float dist = Distance(X(Collider), Z(Collider), XG(oi), ZG(oi));
		if (dist >= 6) continue;
		if (dist < 100 * RS) {
			int pvt = bbCreatePivot(0);
			bbPositionEntity(pvt, XG(oi), Y(Collider), ZG(oi), 0);
			bbPointEntity(pvt, Collider, 0);
			bbRotateEntity(pvt, 0, bbInt(bbEntityYaw(pvt, 0) / 90) * 90, 0, 1);
			bbMoveEntity(pvt, 0, 0, 100 * RS);
			bbPositionEntity(Collider, X(pvt), Y(Collider), Z(pvt), 0);
			bbFreeEntity(pvt);
			if (KillTimer == 0) {
				DeathMSG = "In addition to the decomposed appearance typical of SCP-106's victims, the body exhibits injuries that have not been observed before: "
					"massive skull fracture, three broken ribs, fractured shoulder and multiple heavy lacerations.";
				PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Impact.ogg"));
				KillTimer = -1;
			}
		}
		if (strf(e) < 1000) e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, oi, 6, 1);
	}
	int pvt = bbCreatePivot(0);
	bbPositionEntity(pvt, XG(o8) - 1536 * RS, 500 * RS, ZG(o8) + 608 * RS, 0);
	if (bbEntityDistance(pvt, Collider) < 5) e->SoundCHN2 = LoopSound2(e->Sound2, e->SoundCHN2, Camera, pvt, 3, 1);
	bbFreeEntity(pvt);
	int eyes = R->Objects[17]; // 106's eyes
	bbShowEntity(eyes);
	bbPositionEntity(eyes, XG(o8), 1376 * RS, ZG(o8) - 2848 * RS, 0);
	bbPointEntity(eyes, Collider, 0);
	bbTurnEntity(eyes, 0, 180, 0, 0);
	int temp = bbInt(bbEntityDistance(Collider, eyes)); // blitz's temp is an int
	if (temp < 2000 * RS) {
		Injuries += FPSfactor / 4000;
		e->EventStr = bbStr(strf(e) + FPSfactor / 1000);
		if (strf(e) > 1 && strf(e) < 1000) {
			PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Kneel.ogg"));
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\Screech.ogg", 0);
			e->EventStr = bbStr(1000.0f);
		}
		Sanity = Max(Sanity - FPSfactor / temp / 8, -1000);
		CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 15 * Max((6.0f - temp) / 6, 0));
		stare(eyes, Min(Max(15000 / -Sanity, 15), 500));
		if (Crouch) { // kneel: off to the trenches
			BlinkTimer = -10;
			bbPositionEntity(Collider, XG(o8) - 1344 * RS, 2944 * RS, ZG(o8) - 1184 * RS, 0);
			bbResetEntity(Collider);
			Crouch = 0;
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\Explosion.ogg", 0);
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\TrenchPlane.ogg", 1);
			bbPositionEntity(R->Objects[20], XG(o8) - 1000, 0, 0, 1);
			e->EventStr = bbStr(0.0f);
		}
	} else if (Y(Collider) < -180 * RS) { // the exit room
		float t = Distance(X(Collider), Z(Collider), XG(o8) + 1024 * RS, ZG(o8));
		if (t < 640 * RS) {
			BlurTimer = (640 * RS - t) * 3000;
			e->SoundCHN2 = LoopSound2(DecaySFX[bbRand(1, 3)], e->SoundCHN2, Camera, Collider, 2, (640 * RS - t) * fabsf(CurrSpeed) * 100);
			CurrSpeed = CurveValue(0, CurrSpeed, t * 10);
			if (t < 130 * RS) each(Rooms, r) if (bbSeq(r->RoomTemplate->Name, "room2shaft")) { // out, into room2shaft
				left(e);
				BlinkTimer = -10; LightBlink = 5; BlurTimer = 1500;
				PlayerRoom = r;
				PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Exit.ogg"));
				TeleportEntity(Collider, XG(r->Objects[0]), .4f, ZG(r->Objects[0]), .3f, 1, 2, 0);
				UpdateRooms(); UpdateDoors();
				Curr106->State = 10000; Curr106->Idle = 0;
				int o = r->Objects[0];
				Decals *de = CreateDecal(0, XG(o), YG(o), ZG(o), 270, bbRand(360, 1), 0);
				if (de) TeleportEntity(de->obj, XG(o), YG(o) + .6f, ZG(o), 0, 1, 4, 1);
				break; // blitz: Exit (the Return after it never runs)
			}
		}
	}
}

static void secondroom(Events *e) { // EventState2 = 1
	int o8 = R->Objects[8], o9 = R->Objects[9], o10 = R->Objects[10];
	float s = e->EventState;
	bbPositionEntity(o9, XG(o8) + 3384 * RS, 0, ZG(o8), 0);
	bbTranslateEntity(o9, cosd(s * .8f) * 5, 0, sind(s * 1.6f) * 4, 1);
	bbRotateEntity(o9, 0, s * 2, 0, 0);
	bbPositionEntity(o10, XG(o8), 0, ZG(o8) + 3384 * RS, 0);
	bbTranslateEntity(o10, sind(s * 1.6f) * 4, 0, cosd(s * .8f) * 5, 1);
	bbRotateEntity(o10, 0, s * 2, 0, 0);
	if (e->EventState3 == 1 || e->EventState3 == 2) { // the trick room
		if (e->EventState3 == 1 && (R->RoomDoors[0]->openstate > 150 || R->RoomDoors[1]->openstate > 150)) {
			PlaySound_Strict(LoadTempSound("SFX\\Horror\\Horror16.ogg"));
			BlurTimer = 800;
			e->EventState3 = 2;
		}
		if (Y(Collider) < 5) e->EventState3 = 0;
	} else if (Y(Collider) > 6) trenches(e);
	else throne(e);
	if (Y(Collider) < -1600 * RS) {
		if (bbEntityDistance(Collider, o8) > 4750 * RS) { // the far exit: room106
			fog(0, 0, 0);
			DropSpeed = 0;
			BlurTimer = 1500;
			bbPositionEntity(Collider, XG(R->obj), .4f, XG(R->obj), 0); // sic: x for z, like blitz
			int out = 0;
			each(Rooms, r) if (bbSeq(r->RoomTemplate->Name, "room106")) {
				e->EventState = 0; e->EventState2 = 0;
				TeleportEntity(Collider, XG(r->Objects[10]), .4f, ZG(r->Objects[10]), .3f, 1, 2, 0);
				left(e);
				Curr106->State = 10000; Curr106->Idle = 0;
				out = 1;
				break;
			}
			(void)out;
			bbResetEntity(Collider);
			e->EventState2 = 0;
			refresh();
		} else { // fell
			if (KillTimer >= 0) {
				PlaySound_Strict(HorrorSFX[8]);
				DeathMSG = "In addition to the decomposed appearance typical of the victims of SCP-106, the subject seems to have suffered multiple heavy fractures to both of his legs.";
			}
			KillTimer = Min(-1, KillTimer);
			BlurTimer = 3000;
		}
	}
	refresh();
}

static void startroom(Events *e) { // EventState2 = 0: walking too far out gets you thrown somewhere
	float dist = bbEntityDistance(Collider, R->obj);
	if (dist <= 1700 * RS) return;
	BlinkTimer = -10;
	int o8 = R->Objects[8];
	int r25 = bbRand(25, 1);
	if (r25 <= 4) { // back through the middle, 106's doorway in front of you
		PlaySound_Strict(OldManSFX[3]);
		int pvt = bbCreatePivot(0);
		bbPositionEntity(pvt, X(Collider), Y(Collider), Z(Collider), 0);
		bbPointEntity(pvt, R->obj, 0);
		bbMoveEntity(pvt, 0, 0, dist * 1.9f);
		bbPositionEntity(Collider, X(pvt), Y(Collider), Z(pvt), 0);
		bbResetEntity(Collider);
		bbMoveEntity(pvt, 0, 0, .8f);
		bbPositionEntity(R->Objects[10], X(pvt), 0, Z(pvt), 0);
		bbRotateEntity(R->Objects[10], 0, bbEntityYaw(pvt, 0), 0, 1);
		bbFreeEntity(pvt);
	} else if (r25 <= 10) {
		e->EventState2 = 1; BlinkTimer = -10;
		PlaySound_Strict(OldManSFX[3]);
		bbPositionEntity(Collider, XG(o8), .5f, ZG(o8), 0);
		bbResetEntity(Collider);
	} else if (r25 <= 12) { BlurTimer = 500; bbPositionEntity(Collider, X(R->obj), .5f, Z(R->obj), 0); } // the middle
	else if (r25 <= 15) { // the exit room
		BlurTimer = 1500; e->EventState2 = 1; BlinkTimer = -10;
		bbPositionEntity(Collider, XG(o8) - 400 * RS, -304 * RS, ZG(o8), 0);
		bbResetEntity(Collider);
	} else if (r25 <= 19) { // out, into a tunnel
		BlurTimer = 1500;
		each(Rooms, r) if (bbSeq(r->RoomTemplate->Name, "tunnel")) {
			left(e);
			TeleportEntity(Collider, XG(r->obj), .4f, ZG(r->obj), .3f, 1, 2, 0);
			Curr106->State = 250; Curr106->Idle = 0;
			break;
		}
	} else if (r25 <= 22) { // the pillar room
		BlinkTimer = -10;
		bbPositionEntity(Collider, XG(R->Objects[12]), .6f, ZG(R->Objects[12]), 0);
		bbResetEntity(Collider);
		e->EventState2 = 15;
	} else { // the trick tunnel
		BlurTimer = 1500; e->EventState2 = 1; e->EventState3 = 1; BlinkTimer = -10;
		PlaySound_Strict(OldManSFX[3]);
		bbPositionEntity(Collider, XG(o8), 2288 * RS, ZG(o8), 0);
		bbResetEntity(Collider);
	}
	refresh();
}

static void pillars(Events *e) { // EventState2 12-15
	fog(19, 27, 23); // blitz passes 38*.5, 55*.5, 47*.5 as floats (27.5, 23.5); our fog colors are ints
	if (ParticleAmount > 0 && bbRand(800, 1) == 1) {
		float angle = bbEntityYaw(Camera, 1) + bbRnd(150, 210);
		Particles *p = CreateParticle(X(Collider) + cosd(angle) * 7.5f, 0, Z(Collider) + sind(angle) * 7.5f, 3, 4, 0, 2500);
		bbEntityBlend(p->obj, 2);
		p->speed = .01f; p->SizeChange = 0;
		bbPointEntity(p->pvt, Camera, 0);
		bbTurnEntity(p->pvt, 0, 145, 0, 1);
		bbTurnEntity(p->pvt, bbRand(10, 20), 0, 0, 1);
	}
	if (e->EventState2 > 12) {
		int o = R->Objects[bbInt(e->EventState2)];
		the106(o, .27f);
		show106();
		if ((bbRand(750, 1) == 1) & (e->EventState2 > 12)) { BlinkTimer = -10; e->EventState2--; PlaySound_Strict(HorrorSFX[8]); }
		if (e->EventState2 == 12) {
			CameraShake = 1;
			int o12 = R->Objects[12];
			bbPositionEntity(Curr106->Collider, XG(o12), -1, ZG(o12), 0);
			Curr106->State = -10;
			bbResetEntity(Curr106->Collider);
		}
	} else { Curr106->State = -10; Curr106->Idle = 0; }
	if (Y(Collider) >= -1600 * RS) return;
	int o16 = R->Objects[16];
	if (Distance(XG(o16), ZG(o16), X(Collider), Z(Collider)) < 144 * RS) { // the way out: back to the start
		fog(0, 0, 0);
		DropSpeed = 0; BlurTimer = 500;
		bbPositionEntity(Collider, X(R->obj), .5f, Z(R->obj), 0);
		bbResetEntity(Collider);
		e->EventState2 = 0;
		refresh();
	} else { // fell
		if (KillTimer >= 0) PlaySound_Strict(HorrorSFX[8]);
		KillTimer = Min(-1, KillTimer);
		BlurTimer = 3000;
	}
}

int EvPocketDimension(Events *e) {
	if (PlayerRoom != R) {
		bbHideEntity(R->obj);
		e->EventState = e->EventState2 = e->EventState3 = 0;
		e->EventStr = bbStr(0.0f);
		return 0;
	}
	bbShowEntity(R->obj);
	PlayerFallingPickDistance = 0;
	Injuries += FPSfactor * .00005f;
	PrevSecondaryLightOn = SecondaryLightOn; SecondaryLightOn = 1;
	if (Y(Collider) < 2000 * RS || Y(Collider) > 2608 * RS) CurrStepSFX = 1;
	if (!e->Sound) LoadEventSound(e, "SFX\\Room\\PocketDimension\\Rumble.ogg", 0);
	if (!e->Sound2) e->Sound2 = LoadEventSound(e, "SFX\\Room\\PocketDimension\\PrisonVoices.ogg", 1);
	if (e->EventState == 0) { fog(0, 0, 0); e->EventState = .1f; }
	ShouldPlay = Y(Collider) < 2000 * RS || e->EventState3 == 0 || Y(Collider) > 2608 * RS ? 3 : 0;
	float s = e->EventState;
	bbScaleEntity(R->obj, RS, RS * (1 + sind(s / 14) * .2f), RS, 0);
	int i;
	for (i = 0; i <= 7; i++) bbScaleEntity(R->Objects[i], RS * (1 + fabsf(sind(s / 21 + i * 45) * .1f)), RS * (1 + sind(s / 14 + i * 20) * .1f), RS, 1);
	// i is 8 here, blitz uses the loop's leftover i too
	bbScaleEntity(R->Objects[9], RS * (1.5f + fabsf(sind(s / 21 + i * 45) * .1f)), RS * (1 + sind(s / 14 + i * 20) * .1f), RS, 1);
	e->EventState += FPSfactor;
	s = e->EventState;
	if (e->EventState2 == 0) {
		R->RoomDoors[0]->open = 0; R->RoomDoors[1]->open = 0;
		NPCs *n = Curr106;
		if (n->State > 0) { // 106 circles the starting room
			float angle = fmodf(s / 10, 360);
			bbPositionEntity(n->Collider, X(R->obj), .2f + .35f + sind(s / 14 + i * 20) * .4f, X(R->obj), 0); // sic: x for z, like blitz
			bbRotateEntity(n->Collider, 0, angle, 0, 0);
			bbMoveEntity(n->Collider, 0, 0, 6 - sind(s / 10));
			AnimateNPC(n, 55, 104, .5f, 1);
			bbRotateEntity(n->Collider, 0, angle + 90, 0, 0);
			n->Idle = 1;
			show106();
			if (s > 65 * 70 && bbRand(800, 1) == 1) {
				PlaySound_Strict(HorrorSFX[8]);
				n->State = -.1f; n->Idle = 0;
				e->EventState = 601;
			}
		}
	}
	if (bbEntityDistance(Collider, Curr106->Collider) < .3f) { Curr106->Idle = 0; Curr106->State = -10; } // close enough to grab you
	if (e->EventState2 == 1) secondroom(e);
	else if (e->EventState2 == 0) startroom(e);
	else pillars(e);
	return 0;
}

// DrawGUI: 106's face (or "kneel, mortal") flashes up while you blink in the pocket dimension
void PocketDimensionFaces(void) {
	if (!bbSeq(PlayerRoom->RoomTemplate->Name, "pocketdimension")) return;
	each(Events, e) {
		if (e->room != PlayerRoom) continue;
		int blink = BlinkTimer < -3 && BlinkTimer > -10;
		if (strf(e) < 1000) {
			if (e->EventState <= 600) continue;
			if (blink) {
				if (!e->img) {
					if ((BlinkTimer > -5) & (bbRand(30, 1) == 1)) { // blitz And: Rand always rolls
						PlaySound_Strict(DripSFX[0]);
						if (!e->img) e->img = LoadImage_Strict("GFX\\npcs\\106face.jpg");
					}
				} else { int x = bbRand(390, 310), y = bbRand(290, 310); bbDrawImage(e->img, GraphicWidth / 2 - x, GraphicHeight / 2 - y, 0); }
			} else if (e->img) { bbFreeImage(e->img); e->img = 0; }
			break;
		}
		if (blink) {
			if (!e->img) {
				if (BlinkTimer > -5 && !e->img) {
					e->img = LoadImage_Strict("GFX\\kneelmortal.pd");
					if (bbChannelPlaying(e->SoundCHN)) bbStopChannel(e->SoundCHN);
					e->SoundCHN = PlaySound_Strict(e->Sound);
				}
			} else { int x = bbRand(390, 310), y = bbRand(290, 310); bbDrawImage(e->img, GraphicWidth / 2 - x, GraphicHeight / 2 - y, 0); }
		} else {
			if (e->img) { bbFreeImage(e->img); e->img = 0; }
			if (BlinkTimer < -3) { if (!bbChannelPlaying(e->SoundCHN)) e->SoundCHN = PlaySound_Strict(e->Sound); }
			else if (bbChannelPlaying(e->SoundCHN)) bbStopChannel(e->SoundCHN);
		}
		break;
	}
}
