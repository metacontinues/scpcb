// ev1.c: the small UpdateEvents.bb events (173/106 scares, bodies, ambience, fans, levers). fns return 1 for blitz's Exit
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return entity_x(o, 1); }
static float YG(int o) { return entity_y(o, 1); }
static float ZG(int o) { return entity_z(o, 1); }
static int near(Events *e, float d) { return R->dist < d && R->dist > 0; }
static void put173(int o, float y) { position_entity(Curr173->Collider, XG(o), y, ZG(o), 0); reset_entity(Curr173->Collider); }
static NPCs *body(int type, float x, float y, float z) { return CreateNPC(type, x, y, z); }
static void dead(NPCs *n, float frame) { SetNPCFrame(n, frame); n->State = 8; }
static void retex(NPCs *n, const char *f) { n->texture = (str)f; int t = LoadTexture_Strict(f, 1); entity_texture(n->obj, t, 0, 0); free_texture(t); }

static int room1archive(Events *e) { if (e->EventState == 0) e->EventState = irand(1, 3); else R->RoomDoors[0]->KeyCard = iround(e->EventState); return 0; }

static int lockroom096(Events *e) {
	if (!IN) return 0;
	if (!llive(Curr096)) {
		Curr096 = CreateNPC(NPCtype096, XG(R->obj), .3f, ZG(R->obj));
		rotate_entity(Curr096->Collider, 0, R->angle + 45, 0, 1);
	}
	RemoveEvent(e);
	return 0;
}

static int room2offices2(Events *e) { // the duck moves while you blink
	if (!IN || !(BlinkTimer < -8 && BlinkTimer > -12)) return 0;
	int t = R->Objects[irand(1, 4)];
	position_entity(R->Objects[0], XG(t), YG(t), ZG(t), 1);
	rotate_entity(R->Objects[0], 0, frnd(360, 0), 0, 0);
	return 0;
}

static int room2pit(Events *e) {
	if (Curr173->Idle != 0 || !near(e, 8)) return 0;
	if (!entity_visible(Curr173->Collider, Camera) && !entity_visible(R->Objects[6], Camera)) { put173(R->Objects[6], .5f); RemoveEvent(e); }
	return 0;
}

static int pj(Events *e) {
	if (!IN || e->EventState != 0 || entity_distance(Collider, R->obj) >= 2.5f) return 0;
	PlaySound_Strict(RustleSFX[irand(0, 2)]);
	CreateNPC(NPCtype372, 0, 0, 0);
	e->EventState = 1;
	RemoveEvent(e);
	return 0;
}

static int room2doors173(Events *e) {
	if (!IN || e->EventState != 0 || Curr173->Idle != 0 || entity_in_view(Curr173->obj, Camera)) return 0;
	e->EventState = 1;
	put173(R->Objects[0], .5f);
	RemoveEvent(e);
	return 0;
}

static int room3tunnel(Events *e) { // the dead guard
	if (e->EventState != 0) return 0;
	int o = R->Objects[0];
	NPCs *n = R->NPC[0] = body(NPCtypeGuard, XG(o), YG(o) + .5f, ZG(o));
	point_entity(n->Collider, R->obj, 0);
	rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0) + frnd(-20, 20), 0, 1);
	SetNPCFrame(n, 288); n->State = 8;
	e->EventState = 1;
	RemoveEvent(e);
	return 0;
}

static int lockroom173(Events *e) {
	if (!near(e, 6)) return 0;
	if (Curr173->Idle > 1) { RemoveEvent(e); return 0; }
	if (!entity_in_view(Curr173->Collider, Camera) || entity_distance(Curr173->Collider, Collider) > 15) {
		float a = (225 - 90 + R->angle) * DTOR;
		position_entity(Curr173->Collider, R->x + cosf(a) * 2, .6f, R->z + sinf(a) * 2, 0);
		reset_entity(Curr173->Collider);
		RemoveEvent(e);
	}
	return 0;
}

static int room2offices3(Events *e) { // the door shuts behind you
	if (!IN) return 0;
	e->EventState += FPSfactor;
	Doors *d = R->RoomDoors[0];
	if (e->EventState > 700 && entity_distance(d->obj, Collider) > .5f && !entity_in_view(d->obj, Camera)) { d->open = 0; RemoveEvent(e); }
	return 0;
}

static int buttghost(Events *e) {
	if (!IN || entity_distance(Collider, R->Objects[0]) >= 1.8f) return 0;
	if (e->EventState == 0) { GiveAchievement(Achv789, 1); e->SoundCHN = PlaySound2(ButtGhostSFX, Camera, R->Objects[0], 10, 1); e->EventState = 1; }
	else if (!channel_playing(e->SoundCHN)) RemoveEvent(e);
	return 0;
}

static int room2elevator2(Events *e) { // a body by the elevator
	if (!near(e, 8)) return 0;
	int o = R->Objects[0];
	CreateDecal(3, XG(o), .0005f, ZG(o), 90, frnd(360, 0), 0);
	NPCs *n = R->NPC[0] = body(NPCtypeD, XG(o), .5f, ZG(o));
	ChangeNPCTextureID(n, 0);
	rotate_entity(n->Collider, 0, entity_yaw(R->obj, 0) - 80, 0, 1);
	dead(n, 19);
	RemoveEvent(e);
	return 0;
}

static int roar682(Events *e) {
	str rn = PlayerRoom->RoomTemplate->Name;
	if (e->EventState == 0) { if (IN) e->EventState = 70 * irand(300, 1000); return 0; }
	if (s_eq(rn, "pocketdimension") || s_eq(rn, "room860") || s_eq(rn, "room1123") || s_eq(rn, "dimension1499")) return 0;
	e->EventState -= FPSfactor;
	if (e->EventState >= 17 * 70) return 0;
	if (e->EventState + FPSfactor >= 17 * 70) { LoadEventSound(e, "SFX\\SCP\\682\\Roar.ogg", 0); e->SoundCHN = PlaySound_Strict(e->Sound); }
	if (e->EventState > 17 * 70 - 3 * 70) CameraShake = .5f;
	if (e->EventState < 17 * 70 - 7.5f * 70 && e->EventState > 17 * 70 - 11 * 70) CameraShake = 2;
	if (e->EventState < 70) { if (e->Sound) FreeSound_Strict(e->Sound); RemoveEvent(e); }
	return 0;
}

static int room4tunnels(Events *e) {
	if (!near(e, 10)) return 0;
	NPCs *n = R->NPC[0] = body(NPCtypeD, XG(R->obj) + 1, .5f, ZG(R->obj) + 1);
	retex(n, "GFX\\npcs\\body1.jpg");
	rotate_entity(n->Collider, 0, entity_yaw(R->obj, 0) - irand(20, 60), 0, 1);
	dead(n, 19);
	RemoveEvent(e);
	return 0;
}

static int room2nuke(Events *e) {
	if (IN) {
		e->EventState2 = UpdateElevators(e->EventState2, R->RoomDoors[0], R->RoomDoors[1], R->Objects[4], R->Objects[5], e, 1);
		e->EventState = UpdateLever(R->Objects[1], 0);
		UpdateLever(R->Objects[3], 0);
	}
	if (e->EventState3 == 0) {
		NPCs *n = body(NPCtypeD, XG(R->Objects[6]), .5f, ZG(R->Objects[6]));
		rotate_entity(n->Collider, 0, R->angle + 90, 0, 0);
		n->State = 3;
		SetNPCFrame(n, 40);
		n->IsDead = 1;
		retex(n, "GFX\\npcs\\body2.jpg");
		e->EventState3 = 1;
	}
	return 0;
}

static int room2shaft(Events *e) {
	if (e->EventState == 0) {
		int o = R->Objects[1];
		NPCs *n = R->NPC[0] = body(NPCtypeGuard, XG(o), YG(o) + .5f, ZG(o));
		rotate_entity(n->Collider, 0, R->angle + 180, 0, 1);
		dead(n, 286);
		e->EventState = 1;
	}
	if (IN) {
		UpdateButton(R->Objects[2]);
		if (ClosestButton == R->Objects[2] && MouseHit1) {
			Msg = "The elevator appears to be broken.";
			PlaySound2(ButtonSFX2, Camera, R->Objects[2], 10, 1);
			MsgTimer = 5 * 70;
			MouseHit1 = 0;
		}
	}
	return 0;
}

static int room2gw_b(Events *e) {
	if (R->dist >= 8) return 0;
	if (e->EventState == 0) {
		int o = R->Objects[2];
		NPCs *n = R->NPC[0] = body(NPCtypeGuard, XG(o), YG(o) + .5f, ZG(o));
		point_entity(n->Collider, R->obj, 0);
		rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), 0, 1);
		dead(n, 288);
		e->EventState = 1;
	}
	int o = R->Objects[0];
	Particles *p = CreateParticle(XG(o), YG(o), ZG(o), 6, .2f, 0, 10);
	p->speed = .01f;
	rotate_entity(p->pvt, -60, R->angle - 90, 0, 0);
	p->Achange = -.02f;
	e->SoundCHN = LoopSound2(AlarmSFX[3], e->SoundCHN, Camera, R->Objects[3], 5, 1);
	return 0;
}

static int room2pit106(Events *e) { // 106 stares at you from the pit
	if (Contained106 || Curr106->State <= 0) return 0;
	if (e->EventState == 0) { if (IN) e->EventState = 1; return 0; }
	e->EventState++;
	NPCs *n = Curr106;
	int o = R->Objects[7];
	position_entity(n->Collider, XG(o), YG(o), ZG(o), 0);
	reset_entity(n->Collider);
	point_entity(n->Collider, Camera, 0);
	turn_entity(n->Collider, 0, sinf(MilliSecs2() / 20 * DTOR) * 6, 0, 1);
	move_entity(n->Collider, 0, 0, sinf(MilliSecs2() / 15 * DTOR) * .06f);
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .15f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0), 0, 0);
	n->Idle = 1;
	AnimateNPC(n, 334, 494, .3f, 1);
	if (e->EventState > 800 && BlinkTimer < -5) { n->Idle = 0; RemoveEvent(e); }
	return 0;
}

static int room2poffices2(Events *e) {
	if (!IN) return 0;
	Doors *d = R->RoomDoors[0];
	if (e->EventState == 0) {
		if (d->open) { if (d->openstate == 180) { e->EventState = 1; PlaySound_Strict(HorrorSFX[5]); } }
		else if (entity_distance(Collider, d->obj) < 1.5f && RemoteDoorOn) d->open = 1;
	} else if (entity_distance(R->Objects[0], Collider) < 2) {
		HeartBeatVolume = CurveValue(.5f, HeartBeatVolume, 5);
		HeartBeatRate = CurveValue(120, HeartBeatRate, 150);
		e->SoundCHN = LoopSound2(OldManSFX[4], e->SoundCHN, Camera, R->obj, 5, .3f);
		Curr106->State -= FPSfactor * 3;
	}
	return 0;
}

static int room3door(Events *e) { // a door slams shut out of view
	if (!IN || entity_distance(R->obj, Collider) >= 2.5f) return 0;
	each(Doors, d) {
		if (fabsf(entity_x(d->obj, 1) - entity_x(Collider, 0)) >= 2 || fabsf(entity_z(d->obj, 1) - entity_z(Collider, 0)) >= 2) continue;
		if (!entity_in_view(d->obj, Camera) && d->open) { d->open = 0; d->openstate = 0; BlurTimer = 100; CameraShake = 3; }
		break;
	}
	RemoveEvent(e);
	return 0;
}

static int room4(Events *e) { // 049 gets moved closer while you're near
	if (e->EventState >= MilliSecs2()) return 0;
	if (!IN && Distance(entity_x(Collider, 0), entity_z(Collider, 0), entity_x(R->obj, 0), entity_z(R->obj, 0)) < 16) {
		each(NPCs, n) {
			if (n->NPCtype != NPCtype049) continue;
			if (n->State == 2 && entity_distance(Collider, n->Collider) > 16) {
				t_form_vector(368, 528, 176, R->obj, 0);
				position_entity(n->Collider, entity_x(R->obj, 0) + t_formed_x(), t_formed_y(), entity_z(R->obj, 0) + t_formed_z(), 0);
				reset_entity(n->Collider);
				n->PathStatus = 0; n->State = 4; n->State2 = 0; n->State3 = 0;
				RemoveEvent(e);
				return 0; // blitz: `If e<>Null`, Null once removed
			}
			break;
		}
	}
	e->EventState = MilliSecs2() + 5000;
	return 0;
}

static int room3pitduck(Events *e) {
	if (!IN) return 0;
	int *o = &R->Objects[2];
	if (!*o) {
		*o = LoadMesh_Strict("GFX\\npcs\\duck_low_res.b3d", 0);
		scale_entity(*o, .07f, .07f, .07f, 0);
		int t = LoadTexture_Strict("GFX\\npcs\\duck1.png", 1);
		entity_texture(*o, t, 0, 0); free_texture(t);
		position_entity(*o, XG(R->Objects[0]), YG(R->Objects[0]), ZG(R->Objects[0]), 0);
		point_entity(*o, R->obj, 0);
		rotate_entity(*o, 0, entity_yaw(*o, 1), 0, 1);
		LoadEventSound(e, "SFX\\SCP\\Joke\\Saxophone.ogg", 0);
	} else if (!entity_in_view(*o, Camera)) {
		e->EventState += FPSfactor;
		if (e->EventState > 300 && irand(200, 1) == 1) { e->EventState = 0; e->SoundCHN = PlaySound2(e->Sound, Camera, *o, 6, 1); }
	} else if (e->SoundCHN && channel_playing(e->SoundCHN)) stop_channel(e->SoundCHN);
	return 0;
}

static int tunnel2smoke(Events *e) {
	if (!IN || R->dist >= 3.5f) return 0;
	PlaySound2(BurstSFX, Camera, R->obj, 10, 1);
	for (int i = 0; i <= 1; i++) {
		int o = R->Objects[i];
		Emitters *em = CreateEmitter(XG(o), YG(o), ZG(o), 0);
		turn_entity(em->Obj, 90, 0, 0, 1);
		entity_parent(em->Obj, R->obj, 1);
		em->Size = .05f; em->RandAngle = 10; em->Speed = .06f; em->SizeChange = .007f;
		for (int z = 0, k = (int)ceilf(3.3333f * (ParticleAmount + 1)); z <= k; z++) {
			Particles *p = CreateParticle(entity_x(em->Obj, 1), 448 * RS, entity_z(em->Obj, 1), irand(em->MinImage, em->MaxImage), em->Size, em->Gravity, em->LifeTime);
			p->speed = em->Speed;
			float ry = frnd(360, 0), rp = frnd(360, 0); // yaw's Rnd first: RotateEntity's trailing args make that tail needier
			rotate_entity(p->pvt, rp, ry, 0, 1);
			p->size = .05f; p->SizeChange = .008f;
		}
	}
	RemoveEvent(e);
	return 0;
}

static int room3servers(Events *e) {
	if (!IN) return 0;
	if (e->EventState3 == 0 && Curr173->Idle == 0 && BlinkTimer < -10) {
		int o = R->Objects[irand(0, 2)];
		position_entity(Curr173->Collider, XG(o), YG(o), ZG(o), 0);
		reset_entity(Curr173->Collider);
		e->EventState3 = 1;
	}
	int o = R->Objects[3];
	if (o <= 0) return 0;
	if (BlinkTimer < -8 && BlinkTimer > -12) { point_entity(o, Camera, 0); rotate_entity(o, 0, entity_yaw(o, 1), 0, 1); }
	if (e->EventState2 == 0) { e->EventState = CurveValue(0, e->EventState, 15); if (irand(800, 1) == 1) e->EventState2 = 1; }
	else {
		e->EventState += FPSfactor * .5f;
		if (e->EventState > 360) e->EventState = 0;
		if (irand(1200, 1) == 1) e->EventState2 = 0;
	}
	position_entity(o, XG(o), -608 * RS + .05f + sinf((e->EventState + 270) * DTOR) * .05f, ZG(o), 1);
	return 0;
}

static int room2fan(Events *e) { // EventState: on/off timer, 2: on/off, 3: fan speed
	if (IN) {
		turn_entity(R->Objects[0], e->EventState3 * FPSfactor, 0, 0, 0);
		if (e->EventState3 > .01f) R->SoundCHN = LoopSound2(RoomAmbience[9], R->SoundCHN, Camera, R->Objects[0], 5, e->EventState3 / 4);
		e->EventState3 = CurveValue(e->EventState2 * 5, e->EventState3, 150);
	}
	if (R->dist >= 16) return 0;
	if (e->EventState >= 0) { e->EventState -= FPSfactor; return 0; }
	e->EventState = irand(15, 30) * 70;
	int prev = iround(e->EventState2);
	e->EventState2 = irand(0, 1);
	if (!IN) e->EventState3 = e->EventState2 * 5;
	else if (prev == 0 && e->EventState2 == 1) PlaySound2(LoadTempSound("SFX\\ambient\\Room ambience\\FanOn.ogg"), Camera, R->Objects[0], 8, 1);
	else if (prev == 1 && e->EventState2 == 0) PlaySound2(LoadTempSound("SFX\\ambient\\Room ambience\\FanOff.ogg"), Camera, R->Objects[0], 8, 1);
	return 0;
}

static void blood(Events *e) { // the decal room2scps2 leaves either way
	int o = R->Objects[0];
	Decals *de = CreateDecal(0, XG(o), R->y + 2 * RS, ZG(o), 90, irand(360, 1), 0);
	if (!de) return;
	de->Size = .5f; entity_alpha(de->obj, .8f);
	entity_fx(de->obj, 1);
}

static int room2scps2(Events *e) { // emily's scream
	if (R->dist >= 15) return 0;
	if (Contained106 || Curr106->State < 0) e->EventState = 2;
	if (e->EventState >= 2) { R->RoomDoors[0]->locked = 0; blood(e); RemoveEvent(e); return 0; }
	if (e->EventState == 0) {
		LoadEventSound(e, "SFX\\Character\\Scientist\\EmilyScream.ogg", 0);
		e->SoundCHN = PlaySound2(e->Sound, Camera, R->Objects[0], 100, 1);
		blood(e);
		e->EventState = 1;
	} else if (e->EventState == 1) {
		if (!channel_playing(e->SoundCHN)) { e->EventState = 2; R->RoomDoors[0]->locked = 0; }
		else UpdateSoundOrigin(e->SoundCHN, Camera, R->Objects[0], 100, 1);
	}
	return 0;
}

static int room2trick(Events *e) { // you're turned around without noticing
	if (!IN || entity_distance(R->obj, Collider) >= 2) return 0;
	if (entity_distance(Collider, Curr173->obj) < 6 || entity_distance(Collider, Curr106->obj) < 6) { RemoveEvent(e); return 0; }
	int pvt = create_pivot(0);
	position_entity(pvt, entity_x(Collider, 0), entity_y(Collider, 0), entity_z(Collider, 0), 0);
	point_entity(pvt, R->obj, 0);
	rotate_entity(pvt, 0, entity_yaw(pvt, 0), 0, 1);
	move_entity(pvt, 0, 0, entity_distance(pvt, R->obj) * 2);
	BlinkTimer = -10;
	PlaySound_Strict(HorrorSFX[11]);
	position_entity(Collider, entity_x(pvt, 0), entity_y(pvt, 0) + .05f, entity_z(pvt, 0), 0);
	update_world(1);
	turn_entity(Collider, 0, 180, 0, 0);
	free_entity(pvt);
	RemoveEvent(e);
	return 0;
}

static int room2ccont(Events *e) { // the electrical center's levers
	if (!IN) return 0;
	entity_pick(Camera, 1.5f);
	if (picked_entity() == R->Objects[3] && e->EventState == 0) {
		e->EventState = Max(e->EventState, 1);
		PlaySound_Strict(HorrorSFX[7]);
		PlaySound_Strict(LeverSFX);
	}
	UpdateLever(R->Objects[1], 0); // primary lighting
	int prev = iround(e->EventState2);
	e->EventState2 = UpdateLever(R->Objects[3], 0); // secondary lighting
	if (prev != e->EventState2 && e->EventState > 0) PlaySound2(LightSFX, Camera, R->Objects[3], 10, 1);
	SecondaryLightOn = CurveValue(e->EventState2 ? 1 : 0, SecondaryLightOn, 10);
	RemoteDoorOn = UpdateLever(R->Objects[5], 0);
	if (e->EventState > 0 && e->EventState < 200) {
		e->EventState += FPSfactor;
		rotate_entity(R->Objects[3], CurveValue(-85, entity_pitch(R->Objects[3], 0), 5), entity_yaw(R->Objects[3], 0), 0, 0);
	}
	return 0;
}

static int toiletguard(Events *e) {
	if (e->EventState == 0) { if (near(e, 8)) e->EventState = 1; return 0; }
	if (e->EventState == 1) {
		int o = R->Objects[1];
		NPCs *n = R->NPC[0] = body(NPCtypeGuard, XG(o), YG(o) + .5f, ZG(o));
		point_entity(n->Collider, R->obj, 0);
		rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0) - 20, 0, 1);
		dead(n, 287);
		e->EventState = 2;
		return 0;
	}
	NPCs *n = R->NPC[0];
	if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Character\\Guard\\SuicideGuard1.ogg");
	if (R->dist < 15 && R->dist >= 4) e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, n->Collider, 15, 1);
	else if (R->dist < 4 && PlayerSoundVolume > 1) {
		if (e->EventState2 == 0) {
			int o = R->Objects[2];
			Decals *de = CreateDecal(3, XG(o), YG(o), ZG(o), 0, R->angle + 270, 0);
			if (de) { de->Size = .3f; scale_sprite(de->obj, de->Size, de->Size); }
			e->EventState2 = 1;
		}
		if (!e->SoundCHN2) {
			stop_channel(e->SoundCHN);
			FreeSound_Strict(e->Sound);
			n->Sound = LoadSound_Strict("SFX\\Character\\Guard\\SuicideGuard2.ogg");
			e->SoundCHN2 = PlaySound2(n->Sound, Camera, n->Collider, 15, 1);
		}
		UpdateSoundOrigin(e->SoundCHN2, Camera, n->Collider, 15, 1);
		if (!channel_playing(e->SoundCHN2)) RemoveEvent(e);
	}
	return 0;
}

static int tunnel2(Events *e) { // the lights go out and 173's there
	if (IN) {
		if (Curr173->Idle > 1) { RemoveEvent(e); return 1; }
		if (e->EventState == 0 && Distance(entity_x(Collider, 0), entity_z(Collider, 0), entity_x(R->obj, 0), entity_z(R->obj, 0)) < 3.5f) {
			PlaySound_Strict(LightSFX);
			LightBlink = frnd(0, 1) * (e->EventState / 200);
			e->EventState = 1;
		}
	}
	if (e->EventState > 0 && e->EventState < 200) {
		BlinkTimer = -10;
		if (e->EventState > 30) {
			LightBlink = 1;
			if (e->EventState - FPSfactor <= 30) PlaySound_Strict(LoadTempSound("SFX\\ambient\\general\\ambient3.ogg"));
		}
		if (e->EventState - FPSfactor <= 100 && e->EventState > 100) {
			PlaySound_Strict(LoadTempSound("SFX\\ambient\\general\\ambient6.ogg"));
			position_entity(Curr173->Collider, entity_x(R->obj, 0), .6f, entity_z(R->obj, 0), 0);
			reset_entity(Curr173->Collider);
			Curr173->Idle = 1;
		}
		LightBlink = 1;
		e->EventState += FPSfactor;
	} else if (e->EventState != 0) {
		BlinkTimer = BLINKFREQ;
		Curr173->Idle = 0;
		RemoveEvent(e);
	}
	return 0;
}

static int room2offices035(Events *e) { // 035's victim, once 035 is out
	int out = 0;
	each(Events, e2) if (e2 != e && s_eq(e2->EventName, "room035")) { if (e2->EventState < 0) { out = 1; break; } }
	if (!out) return 0;
	if (R->dist < 8) {
		if (!llive(R->NPC[0])) {
			NPCs *n = R->NPC[0] = body(NPCtypeD, R->x, .5f, R->z);
			rotate_entity(n->Collider, 0, R->angle + 180, 0, 0);
			move_entity(n->Collider, 0, 0, -.5f);
			n->State = 3;
			n->texture = "GFX\\npcs\\035victim.jpg";
			ChangeNPCTextureID(n, 7);
			SetNPCFrame(n, 19);
		}
		if (!llive(R->NPC[1]) && entity_distance(R->NPC[0]->Collider, Collider) < 2.5f) {
			NPCs *t = R->NPC[1] = CreateNPC(NPCtypeTentacle, entity_x(R->NPC[0]->Collider, 0), 0, entity_z(R->NPC[0]->Collider, 0));
			rotate_entity(t->Collider, 0, R->angle, 0, 0);
			move_entity(t->Collider, 0, 0, .6f);
		}
	} else if (R->dist > HideDistance && llive(R->NPC[1])) { RemoveNPC(R->NPC[1]); R->NPC[1] = NULL; }
	return 0;
}

static int medibay(Events *e) { // EventState: entered, 2: the zombie's wake-up timer
	if (!IN) hide_entity(R->Objects[0]);
	else {
		show_entity(R->Objects[0]);
		if (e->EventState == 0) {
			NPCs *n = R->NPC[0] = CreateNPC(NPCtype008, XG(R->Objects[3]), .5f, ZG(R->Objects[3]));
			rotate_entity(n->Collider, 0, R->angle - 90, 0, 0);
			e->EventState = 1;
		}
		if (entity_distance(R->NPC[0]->Collider, Collider) < 1.2f && e->EventState2 == 0) { LightBlink = 12; PlaySound_Strict(LightSFX); e->EventState2 = FPSfactor; }
	}
	if (e->EventState2 > 0 && e->EventState2 < 70 * 4) e->EventState2 += FPSfactor;
	else if (e->EventState2 >= 70 * 4 && R->NPC[0]->State == 0) R->NPC[0]->State = 2;
	return 0;
}

const EvEntry Evs1[] = {
	{"room1archive", room1archive}, {"lockroom096", lockroom096}, {"room2offices2", room2offices2}, {"room2pit", room2pit},
	{"pj", pj}, {"room2doors173", room2doors173}, {"room3tunnel", room3tunnel}, {"lockroom173", lockroom173},
	{"room2offices3", room2offices3}, {"buttghost", buttghost}, {"room2elevator2", room2elevator2}, {"682roar", roar682},
	{"room4tunnels", room4tunnels}, {"room2nuke", room2nuke}, {"room2shaft", room2shaft}, {"room2gw_b", room2gw_b},
	{"room2pit106", room2pit106}, {"room2poffices2", room2poffices2}, {"room3door", room3door}, {"room4", room4},
	{"room3pitduck", room3pitduck}, {"tunnel2smoke", tunnel2smoke}, {"room3servers", room3servers}, {"room2fan", room2fan},
	{"room2scps2", room2scps2}, {"room2trick", room2trick}, {"room2ccont", room2ccont}, {"toiletguard", toiletguard},
	{"tunnel2", tunnel2}, {"room2offices035", room2offices035}, {"medibay", medibay}, {NULL, NULL},
};
