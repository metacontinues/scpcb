// evpd.c: the "pocketdimension" event (UpdateEvents.bb) + the faces DrawGUI flashes at you in there (Main.bb 4629)
// EventState: timer for the breathing tunnels. EventState2: 0 starting room, 1 throne/plane/trench rooms, 12-15 the
// pillar room (106 hops pillar to pillar). EventState3: 1 the hcz-looking trick tunnel, 2 its door opened.
// EventVal: how long 106's eyes have stared at you (1000 = kneel)
#include "g.h"

#define R (e->room)
#define RS RoomScale
static float XG(int o) { return entity_x(o, 1); }
static float YG(int o) { return entity_y(o, 1); }
static float ZG(int o) { return entity_z(o, 1); }
static float X(int o) { return entity_x(o, 0); }
static float Y(int o) { return entity_y(o, 0); }
static float Z(int o) { return entity_z(o, 0); }
static float sind(float a) { return sinf(a * DTOR); }
static float cosd(float a) { return cosf(a * DTOR); }

static void fog(int r, int g, int b) { camera_fog_color(Camera, r, g, b); camera_cls_color(Camera, r, g, b); }
static void refresh(void) { UpdateDoorsTimer = 0; UpdateDoors(); UpdateRooms(); }

// look at t (the player can't look away from the plane / 106's eyes)
static void stare(int t, float smooth) {
	int pvt = create_pivot(0);
	position_entity(pvt, X(Camera), Y(Camera), Z(Camera), 0);
	point_entity(pvt, t, 0);
	turn_entity(pvt, 90, 0, 0, 0);
	user_camera_pitch = CurveAngle(entity_pitch(pvt, 0), user_camera_pitch + 90, smooth);
	user_camera_pitch -= 90;
	rotate_entity(Collider, entity_pitch(Collider, 0), CurveAngle(entity_yaw(pvt, 0), entity_yaw(Collider, 0), smooth), 0, 0);
	free_entity(pvt);
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
		rotate_entity(sl->room->Levers[0], 0, entity_yaw(sl->room->Levers[0], 0), 0, 0);
		TurnCheckpointMonitorsOff(0);
	}
}

static void the106(int ox, float y) { // 106 stands at a spot, swaying toward you
	NPCs *n = Curr106;
	n->Idle = 1;
	position_entity(n->Collider, XG(ox), y, ZG(ox), 0);
	point_entity(n->Collider, Camera, 0);
	turn_entity(n->Collider, 0, sind(MilliSecs2() / 20) * 6, 0, 1);
	move_entity(n->Collider, 0, 0, sind(MilliSecs2() / 15) * .06f);
}
static void show106(void) {
	NPCs *n = Curr106;
	show_entity(n->obj); show_entity(n->Collider);
	reset_entity(n->Collider);
	n->GravityMult = 0; n->DropSpeed = 0;
	position_entity(n->obj, X(n->Collider), Y(n->Collider) - .15f, Z(n->Collider), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0), 0, 0);
}

static void trenches(Events *e) { // the plane hunts you over the trenches
	int o8 = R->Objects[8], *pl = &R->Objects[20];
	ShouldPlay = 15;
	fog(38, 55, 47);
	if (XG(*pl) < XG(o8) - 4000 * RS) { e->SoundCHN2 = PlaySound_Strict(e->Sound2); position_entity(*pl, entity_x(Collider, 1) + 4000 * RS, 12, entity_z(Collider, 1), 0); }
	move_entity(Collider, 0, Min(12 - Y(Collider), 0) * FPSfactor, 0);
	float x = -FPSfactor * RS * 4, y = (17 - fabsf(X(Collider) - X(*pl)) * .5f) - Y(*pl), z = entity_z(Collider, 1) - Z(*pl);
	translate_entity(*pl, x, y, z, 1);
	rotate_entity(*pl, -90 - (X(Collider) - X(*pl)) * 1.5f, -90, 0, 1);
	static const float spots[3][2] = {{-1452, -37}, {-121, 188}, {1223, -196}}; // covered spots the plane can't see
	int safe = 0;
	for (int i = 0; i <= 2; i++) if (Distance(X(Collider), Z(Collider), spots[i][0] * RS + XG(o8), spots[i][1] * RS + ZG(o8)) < 200 * RS) { safe = 1; break; }
	float dist = entity_distance(Collider, *pl);
	if (e->SoundCHN2 && channel_playing(e->SoundCHN2)) e->SoundCHN2 = LoopSound2(e->Sound2, e->SoundCHN2, Camera, Camera, 10, .3f + !safe * .6f);
	if (safe) entity_texture(*pl, R->Objects[18], 0, 0);
	else if (dist < 8) {
		e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, *pl, 8, 1);
		entity_texture(*pl, R->Objects[19], 0, 0);
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
		position_entity(Collider, XG(o8) - 400 * RS, -304 * RS, ZG(o8), 0);
		reset_entity(Collider);
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
			int pvt = create_pivot(0);
			position_entity(pvt, XG(oi), Y(Collider), ZG(oi), 0);
			point_entity(pvt, Collider, 0);
			rotate_entity(pvt, 0, iround(entity_yaw(pvt, 0) / 90) * 90, 0, 1);
			move_entity(pvt, 0, 0, 100 * RS);
			position_entity(Collider, X(pvt), Y(Collider), Z(pvt), 0);
			free_entity(pvt);
			if (KillTimer == 0) {
				sset(&DeathMSG, "In addition to the decomposed appearance typical of SCP-106's victims, the body exhibits injuries that have not been observed before: "
					"massive skull fracture, three broken ribs, fractured shoulder and multiple heavy lacerations.");
				PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Impact.ogg"));
				KillTimer = -1;
			}
		}
		if (e->EventVal < 1000) e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, oi, 6, 1);
	}
	int pvt = create_pivot(0);
	position_entity(pvt, XG(o8) - 1536 * RS, 500 * RS, ZG(o8) + 608 * RS, 0);
	if (entity_distance(pvt, Collider) < 5) e->SoundCHN2 = LoopSound2(e->Sound2, e->SoundCHN2, Camera, pvt, 3, 1);
	free_entity(pvt);
	int eyes = R->Objects[17]; // 106's eyes
	show_entity(eyes);
	position_entity(eyes, XG(o8), 1376 * RS, ZG(o8) - 2848 * RS, 0);
	point_entity(eyes, Collider, 0);
	turn_entity(eyes, 0, 180, 0, 0);
	int temp = iround(entity_distance(Collider, eyes)); // blitz's temp is an int
	if (temp < 2000 * RS) {
		Injuries += FPSfactor / 4000;
		e->EventVal += FPSfactor / 1000;
		if (e->EventVal > 1 && e->EventVal < 1000) {
			PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Kneel.ogg"));
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\Screech.ogg", 0);
			e->EventVal = 1000;
		}
		Sanity = Max(Sanity - FPSfactor / temp / 8, -1000);
		CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 15 * Max((6.0f - temp) / 6, 0));
		stare(eyes, Min(Max(15000 / -Sanity, 15), 500));
		if (Crouch) { // kneel: off to the trenches
			BlinkTimer = -10;
			position_entity(Collider, XG(o8) - 1344 * RS, 2944 * RS, ZG(o8) - 1184 * RS, 0);
			reset_entity(Collider);
			Crouch = 0;
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\Explosion.ogg", 0);
			LoadEventSound(e, "SFX\\Room\\PocketDimension\\TrenchPlane.ogg", 1);
			position_entity(R->Objects[20], XG(o8) - 1000, 0, 0, 1);
			e->EventVal = 0;
		}
	} else if (Y(Collider) < -180 * RS) { // the exit room
		float t = Distance(X(Collider), Z(Collider), XG(o8) + 1024 * RS, ZG(o8));
		if (t < 640 * RS) {
			BlurTimer = (640 * RS - t) * 3000;
			e->SoundCHN2 = LoopSound2(DecaySFX[irand(1, 3)], e->SoundCHN2, Camera, Collider, 2, (640 * RS - t) * fabsf(CurrSpeed) * 100);
			CurrSpeed = CurveValue(0, CurrSpeed, t * 10);
			if (t < 130 * RS) each(Rooms, r) if (s_eq(r->RoomTemplate->Name, "room2shaft")) { // out, into room2shaft
				left(e);
				BlinkTimer = -10; LightBlink = 5; BlurTimer = 1500;
				PlayerRoom = r;
				PlaySound_Strict(LoadTempSound("SFX\\Room\\PocketDimension\\Exit.ogg"));
				TeleportEntity(Collider, XG(r->Objects[0]), .4f, ZG(r->Objects[0]), .3f, 1, 2, 0);
				UpdateRooms(); UpdateDoors();
				Curr106->State = 10000; Curr106->Idle = 0;
				int o = r->Objects[0];
				Decals *de = CreateDecal(0, XG(o), YG(o), ZG(o), 270, irand(1, 360), 0);
				if (de) TeleportEntity(de->obj, XG(o), YG(o) + .6f, ZG(o), 0, 1, 4, 1);
				break;
			}
		}
	}
}

static void secondroom(Events *e) { // EventState2 = 1
	int o8 = R->Objects[8], o9 = R->Objects[9], o10 = R->Objects[10];
	float s = e->EventState;
	position_entity(o9, XG(o8) + 3384 * RS, 0, ZG(o8), 0);
	translate_entity(o9, cosd(s * .8f) * 5, 0, sind(s * 1.6f) * 4, 1);
	rotate_entity(o9, 0, s * 2, 0, 0);
	position_entity(o10, XG(o8), 0, ZG(o8) + 3384 * RS, 0);
	translate_entity(o10, sind(s * 1.6f) * 4, 0, cosd(s * .8f) * 5, 1);
	rotate_entity(o10, 0, s * 2, 0, 0);
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
		if (entity_distance(Collider, o8) > 4750 * RS) { // the far exit: room106
			fog(0, 0, 0);
			DropSpeed = 0;
			BlurTimer = 1500;
			position_entity(Collider, XG(R->obj), .4f, ZG(R->obj), 0);
			int out = 0;
			each(Rooms, r) if (s_eq(r->RoomTemplate->Name, "room106")) {
				e->EventState = 0; e->EventState2 = 0;
				TeleportEntity(Collider, XG(r->Objects[10]), .4f, ZG(r->Objects[10]), .3f, 1, 2, 0);
				left(e);
				Curr106->State = 10000; Curr106->Idle = 0;
				out = 1;
				break;
			}
			(void)out;
			reset_entity(Collider);
			e->EventState2 = 0;
			refresh();
		} else { // fell
			if (KillTimer >= 0) {
				PlaySound_Strict(HorrorSFX[8]);
				sset(&DeathMSG, "In addition to the decomposed appearance typical of the victims of SCP-106, the subject seems to have suffered multiple heavy fractures to both of his legs.");
			}
			KillTimer = Min(-1, KillTimer);
			BlurTimer = 3000;
		}
	}
	refresh();
}

static void startroom(Events *e) { // EventState2 = 0: walking too far out gets you thrown somewhere
	float dist = entity_distance(Collider, R->obj);
	if (dist <= 1700 * RS) return;
	BlinkTimer = -10;
	int o8 = R->Objects[8];
	int r25 = irand(1, 25);
	if (r25 <= 4) { // back through the middle, 106's doorway in front of you
		PlaySound_Strict(OldManSFX[3]);
		int pvt = create_pivot(0);
		position_entity(pvt, X(Collider), Y(Collider), Z(Collider), 0);
		point_entity(pvt, R->obj, 0);
		move_entity(pvt, 0, 0, dist * 1.9f);
		position_entity(Collider, X(pvt), Y(Collider), Z(pvt), 0);
		reset_entity(Collider);
		move_entity(pvt, 0, 0, .8f);
		position_entity(R->Objects[10], X(pvt), 0, Z(pvt), 0);
		rotate_entity(R->Objects[10], 0, entity_yaw(pvt, 0), 0, 1);
		free_entity(pvt);
	} else if (r25 <= 10) {
		e->EventState2 = 1; BlinkTimer = -10;
		PlaySound_Strict(OldManSFX[3]);
		position_entity(Collider, XG(o8), .5f, ZG(o8), 0);
		reset_entity(Collider);
	} else if (r25 <= 12) { BlurTimer = 500; position_entity(Collider, X(R->obj), .5f, Z(R->obj), 0); } // the middle
	else if (r25 <= 15) { // the exit room
		BlurTimer = 1500; e->EventState2 = 1; BlinkTimer = -10;
		position_entity(Collider, XG(o8) - 400 * RS, -304 * RS, ZG(o8), 0);
		reset_entity(Collider);
	} else if (r25 <= 19) { // out, into a tunnel
		BlurTimer = 1500;
		each(Rooms, r) if (s_eq(r->RoomTemplate->Name, "tunnel")) {
			left(e);
			TeleportEntity(Collider, XG(r->obj), .4f, ZG(r->obj), .3f, 1, 2, 0);
			Curr106->State = 250; Curr106->Idle = 0;
			break;
		}
	} else if (r25 <= 22) { // the pillar room
		BlinkTimer = -10;
		position_entity(Collider, XG(R->Objects[12]), .6f, ZG(R->Objects[12]), 0);
		reset_entity(Collider);
		e->EventState2 = 15;
	} else { // the trick tunnel
		BlurTimer = 1500; e->EventState2 = 1; e->EventState3 = 1; BlinkTimer = -10;
		PlaySound_Strict(OldManSFX[3]);
		position_entity(Collider, XG(o8), 2288 * RS, ZG(o8), 0);
		reset_entity(Collider);
	}
	refresh();
}

static void pillars(Events *e) { // EventState2 12-15
	fog(19, 27, 23); // blitz passes 38*.5, 55*.5, 47*.5 as floats (27.5, 23.5); our fog colors are ints
	if (ParticleAmount > 0 && irand(1, 800) == 1) {
		float angle = entity_yaw(Camera, 1) + frnd(150, 210);
		Particles *p = CreateParticle(X(Collider) + cosd(angle) * 7.5f, 0, Z(Collider) + sind(angle) * 7.5f, 3, 4, 0, 2500);
		entity_blend(p->obj, 2);
		p->speed = .01f; p->SizeChange = 0;
		point_entity(p->pvt, Camera, 0);
		turn_entity(p->pvt, 0, 145, 0, 1);
		turn_entity(p->pvt, irand(10, 20), 0, 0, 1);
	}
	if (e->EventState2 > 12) {
		int o = R->Objects[iround(e->EventState2)];
		the106(o, .27f);
		show106();
		if (irand(1, 750) == 1) { BlinkTimer = -10; e->EventState2--; PlaySound_Strict(HorrorSFX[8]); }
		if (e->EventState2 == 12) {
			CameraShake = 1;
			int o12 = R->Objects[12];
			position_entity(Curr106->Collider, XG(o12), -1, ZG(o12), 0);
			Curr106->State = -10;
			reset_entity(Curr106->Collider);
		}
	} else { Curr106->State = -10; Curr106->Idle = 0; }
	if (Y(Collider) >= -1600 * RS) return;
	int o16 = R->Objects[16];
	if (Distance(XG(o16), ZG(o16), X(Collider), Z(Collider)) < 144 * RS) { // the way out: back to the start
		fog(0, 0, 0);
		DropSpeed = 0; BlurTimer = 500;
		position_entity(Collider, X(R->obj), .5f, Z(R->obj), 0);
		reset_entity(Collider);
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
		hide_entity(R->obj);
		e->EventState = e->EventState2 = e->EventState3 = 0;
		e->EventVal = 0;
		return 0;
	}
	show_entity(R->obj);
	PlayerFallingPickDistance = 0;
	Injuries += FPSfactor * .00005f;
	PrevSecondaryLightOn = SecondaryLightOn; SecondaryLightOn = 1;
	if (Y(Collider) < 2000 * RS || Y(Collider) > 2608 * RS) CurrStepSFX = 1;
	if (!e->Sound) LoadEventSound(e, "SFX\\Room\\PocketDimension\\Rumble.ogg", 0);
	if (!e->Sound2) e->Sound2 = LoadEventSound(e, "SFX\\Room\\PocketDimension\\PrisonVoices.ogg", 1);
	if (e->EventState == 0) { fog(0, 0, 0); e->EventState = .1f; }
	ShouldPlay = Y(Collider) < 2000 * RS || e->EventState3 == 0 || Y(Collider) > 2608 * RS ? 3 : 0;
	float s = e->EventState;
	scale_entity(R->obj, RS, RS * (1 + sind(s / 14) * .2f), RS, 0);
	for (int i = 0; i <= 7; i++) scale_entity(R->Objects[i], RS * (1 + fabsf(sind(s / 21 + i * 45) * .1f)), RS * (1 + sind(s / 14 + i * 20) * .1f), RS, 1);
	scale_entity(R->Objects[9], RS * (1.5f + fabsf(sind(s / 21 + 9 * 45) * .1f)), RS * (1 + sind(s / 14 + 9 * 20) * .1f), RS, 1);
	e->EventState += FPSfactor;
	s = e->EventState;
	if (e->EventState2 == 0) {
		R->RoomDoors[0]->open = 0; R->RoomDoors[1]->open = 0;
		NPCs *n = Curr106;
		if (n->State > 0) { // 106 circles the starting room
			float angle = fmodf(s / 10, 360);
			position_entity(n->Collider, X(R->obj), .2f + .35f + sind(s / 14) * .4f, Z(R->obj), 0);
			rotate_entity(n->Collider, 0, angle, 0, 0);
			move_entity(n->Collider, 0, 0, 6 - sind(s / 10));
			AnimateNPC(n, 55, 104, .5f, 1);
			rotate_entity(n->Collider, 0, angle + 90, 0, 0);
			n->Idle = 1;
			show106();
			if (s > 65 * 70 && irand(1, 800) == 1) {
				PlaySound_Strict(HorrorSFX[8]);
				n->State = -.1f; n->Idle = 0;
				e->EventState = 601;
			}
		}
	}
	if (entity_distance(Collider, Curr106->Collider) < .3f) { Curr106->Idle = 0; Curr106->State = -10; } // close enough to grab you
	if (e->EventState2 == 1) secondroom(e);
	else if (e->EventState2 == 0) startroom(e);
	else pillars(e);
	return 0;
}

// DrawGUI: 106's face (or "kneel, mortal") flashes up while you blink in the pocket dimension
void PocketDimensionFaces(void) {
	if (!s_eq(PlayerRoom->RoomTemplate->Name, "pocketdimension")) return;
	each(Events, e) {
		if (e->room != PlayerRoom) continue;
		int blink = BlinkTimer < -3 && BlinkTimer > -10;
		if (e->EventVal < 1000) {
			if (e->EventState <= 600) continue;
			if (blink) {
				if (!e->img) {
					if (BlinkTimer > -5 && irand(1, 30) == 1) {
						PlaySound_Strict(DripSFX[0]);
						if (!e->img) e->img = LoadImage_Strict("GFX\\npcs\\106face.jpg");
					}
				} else { int x = irand(310, 390), y = irand(290, 310); draw_image(e->img, GraphicWidth / 2 - x, GraphicHeight / 2 - y, 0); }
			} else if (e->img) { free_image(e->img); e->img = 0; }
			break;
		}
		if (blink) {
			if (!e->img) {
				if (BlinkTimer > -5 && !e->img) {
					e->img = LoadImage_Strict("GFX\\kneelmortal.pd");
					if (channel_playing(e->SoundCHN)) stop_channel(e->SoundCHN);
					e->SoundCHN = PlaySound_Strict(e->Sound);
				}
			} else { int x = irand(310, 390), y = irand(290, 310); draw_image(e->img, GraphicWidth / 2 - x, GraphicHeight / 2 - y, 0); }
		} else {
			if (e->img) { free_image(e->img); e->img = 0; }
			if (BlinkTimer < -3) { if (!channel_playing(e->SoundCHN)) e->SoundCHN = PlaySound_Strict(e->Sound); }
			else if (channel_playing(e->SoundCHN)) stop_channel(e->SoundCHN);
		}
		break;
	}
}
