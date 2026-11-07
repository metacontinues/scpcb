// ev4.c: room012 (the composition) and coffin/coffin106 (SCP-895) (UpdateEvents.bb)
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return entity_x(o, 1); }
static float YG(int o) { return entity_y(o, 1); }
static float ZG(int o) { return entity_z(o, 1); }
static float X(int o) { return entity_x(o, 0); }
static float Z(int o) { return entity_z(o, 0); }
static float sind(float a) { return sinf(a * DTOR); }

static void retex(int o, const char *f) { int t = LoadTexture_Strict(f, 1); entity_texture(o, t, 0, 1); free_texture(t); }

static void push(float angle, float k) { // ForceMove toward where you're facing, like blitz's 012 pull
	if (angle < 40) ForceMove = (40 - angle) * k;
	else if (angle > 310) ForceMove = (40 - fabsf(360 - angle)) * k;
}

// 012 has you: walk to it, then write w/ your own blood
static void composition(Events *e) {
	int o2 = R->Objects[2], o3 = R->Objects[3];
	e->SoundCHN2 = LoopSound2(e->Sound2, e->SoundCHN2, Camera, o3, 10, e->EventState3 / (86.0f * 70));
	int pvt = create_pivot(0);
	position_entity(pvt, X(Camera), YG(o2) - .05f, Z(Camera), 0);
	point_entity(pvt, o2, 0);
	rotate_entity(Collider, entity_pitch(Collider, 0), CurveAngle(entity_yaw(pvt, 0), entity_yaw(Collider, 0), 80 - e->EventState3 / 200), 0, 0);
	turn_entity(pvt, 90, 0, 0, 0);
	user_camera_pitch = CurveAngle(entity_pitch(pvt, 0) + 25, user_camera_pitch + 90, 80 - e->EventState3 / 200);
	user_camera_pitch -= 90;
	float dist = Distance(X(Collider), Z(Collider), XG(o2), ZG(o2));
	HeartBeatRate = 150;
	HeartBeatVolume = Max(3 - dist, 0) / 3;
	BlurVolume = Max((2 - dist) * (e->EventState3 / 800) * sind((float)MilliSecs2() / 20 + 1), BlurVolume);
	CurrCameraZoom = Max(CurrCameraZoom, (sind((float)MilliSecs2() / 20) + 1) * 8 * Max(3 - dist, 0));
	if (BreathCHN && channel_playing(BreathCHN)) stop_channel(BreathCHN);
	if (dist < .6f) {
		e->EventState3 = Min(e->EventState3 + FPSfactor, 86 * 70);
		float s = e->EventState3, p = s - FPSfactor;
#define AT(t) (s > (t) * 70 && p <= (t) * 70)
		if (AT(1)) PlaySound_Strict(LoadTempSound("SFX\\SCP\\012\\Speech1.ogg"));
		else if (AT(13)) {
			sset(&Msg, "You start pushing your nails into your wrist, drawing blood."); MsgTimer = 7 * 70;
			Injuries += .5f;
			PlaySound_Strict(LoadTempSound("SFX\\SCP\\012\\Speech2.ogg"));
		} else if (AT(31)) {
			retex(R->Objects[4], "GFX\\map\\scp-012_1.jpg");
			sset(&Msg, "You tear open your left wrist and start writing on the composition with your blood."); MsgTimer = 7 * 70;
			Injuries = Max(Injuries, 1.5f);
			PlaySound_Strict(LoadTempSound(sf("SFX\\SCP\\012\\Speech%d.ogg", irand(3, 4))));
		} else if (AT(49)) {
			sset(&Msg, "You push your fingers deeper into the wound."); MsgTimer = 8 * 70;
			Injuries += .3f;
			PlaySound_Strict(LoadTempSound("SFX\\SCP\\012\\Speech5.ogg"));
		} else if (AT(63)) {
			retex(R->Objects[4], "GFX\\map\\scp-012_2.jpg");
			Injuries += .5f;
			PlaySound_Strict(LoadTempSound("SFX\\SCP\\012\\Speech6.ogg"));
		} else if (AT(74)) {
			retex(R->Objects[4], "GFX\\map\\scp-012_3.jpg");
			sset(&Msg, "You rip the wound wide open. Grabbing scoops of blood pouring out."); MsgTimer = 7 * 70;
			Injuries += .8f;
			PlaySound_Strict(LoadTempSound("SFX\\SCP\\012\\Speech7.ogg"));
			Crouch = 1;
			Decals *de = CreateDecal(17, X(Collider), -768 * RS + .01f, Z(Collider), 90, frnd(0, 360), 0);
			if (de) { de->Size = .1f; de->MaxSize = .45f; de->SizeChange = .0002f; UpdateDecals(); }
		} else if (AT(85)) {
			sset(&DeathMSG, "Subject D-9341 found in a pool of blood next to SCP-012. Subject seems to have ripped open his wrists and written three extra "
				"lines to the composition before dying of blood loss.");
			Kill();
		}
#undef AT
		rotate_entity(Collider, entity_pitch(Collider, 0), CurveAngle(entity_yaw(Collider, 0) + sind(s * (s / 2000)) * (s / 300), entity_yaw(Collider, 0), 80), 0, 0);
	} else push(WrapAngle(entity_yaw(pvt, 0) - entity_yaw(Collider, 0)), .02f);
	free_entity(pvt);
}

static int room012(Events *e) {
	if (!IN) return 0;
	Doors *d = R->RoomDoors[0];
	if (e->EventState == 0) {
		if (entity_distance(Collider, d->obj) < 2.5f && RemoteDoorOn) {
			GiveAchievement(Achv012, 1);
			PlaySound_Strict(HorrorSFX[7]);
			PlaySound2(LeverSFX, Camera, d->obj, 10, 1);
			e->EventState = 1;
			d->locked = 0; UseDoor(d, 0, 1); d->locked = 1;
		}
		return 0;
	}
	if (!e->Sound) LoadEventSound(e, "SFX\\Music\\012Golgotha.ogg", 0);
	e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, R->Objects[3], 5, 1);
	if (!e->Sound2) LoadEventSound(e, "SFX\\Music\\012.ogg", 1);
	if (e->EventState < 90) e->EventState = CurveValue(90, e->EventState, 500);
	int o2 = R->Objects[2];
	position_entity(o2, XG(o2), (-130 - 448 * sind(e->EventState)) * RS, ZG(o2), 1);
	if (e->EventState2 > 0 && e->EventState2 < 200) {
		e->EventState2 += FPSfactor;
		rotate_entity(R->Objects[1], CurveValue(85, entity_pitch(R->Objects[1], 0), 5), entity_yaw(R->Objects[1], 0), 0, 0);
	} else {
		e->EventState2 += FPSfactor;
		if (e->EventState2 < 250) show_entity(R->Objects[3]);
		else { hide_entity(R->Objects[3]); if (e->EventState2 > 300) e->EventState2 = 200; }
	}
	if (Wearing714 || WearingGasMask >= 3 || WearingHazmat >= 3) return 0;
	if (entity_visible(o2, Camera)) composition(e);
	else if (Distance(X(Collider), Z(Collider), X(d->frameobj), Z(d->frameobj)) < 4.5f && entity_y(Collider, 0) < -2.5f) { // walk to the door
		int pvt = create_pivot(0);
		position_entity(pvt, X(Camera), entity_y(Collider, 0), Z(Camera), 0);
		point_entity(pvt, d->frameobj, 0);
		user_camera_pitch = CurveAngle(90, user_camera_pitch + 90, 100);
		user_camera_pitch -= 90;
		rotate_entity(Collider, entity_pitch(Collider, 0), CurveAngle(entity_yaw(pvt, 0), entity_yaw(Collider, 0), 150), 0, 0);
		push(WrapAngle(entity_yaw(pvt, 0) - entity_yaw(Collider, 0)), .008f);
		free_entity(pvt);
	}
	return 0;
}

// ---- 895

static int worn(Items *it) { // the kind of nv goggles you have on
	static const char *t[] = {"", "nvgoggles", "supernv", "finenvgoggles"};
	return it && WearingNightVision >= 1 && WearingNightVision <= 3 && s_eq(it->itemtemplate->tempname, t[WearingNightVision]);
}
static Items *goggles(void) { for (int i = 0; i < MaxItemAmount; i++) if (worn(Inventory[i])) return Inventory[i]; return NULL; }
static int battery(void) { // any worn-kind goggles w/ charge (the fine ones need none)
	for (int i = 0; i < MaxItemAmount; i++) if (worn(Inventory[i]) && (Inventory[i]->state > 0 || WearingNightVision == 3)) return 1;
	return 0;
}

static void cams(Events *e, int on, int blend) { // 079 shows the coffin on every monitor
	each(SecurityCams, sc) {
		if (on) { if (sc->CoffinEffect == 0 && !s_eq(sc->room->RoomTemplate->Name, "room106") && (blend || !s_eq(sc->room->RoomTemplate->Name, "room205"))) sc->CoffinEffect = 2; }
		else if (sc->CoffinEffect != 1) sc->CoffinEffect = 0;
		if (blend && sc->CoffinEffect == 1) entity_blend(sc->ScrOverlay, on ? 3 : 0);
		if (sc->room == R) sc->Screen = on;
	}
}

static void guard(Events *e) { // the guard who falls into the shaft
	NPCs *n = llive(R->NPC[0]);
	if (!n) return;
	UpdateSoundOrigin(n->SoundChn, Camera, n->Collider, 100, 1);
	if (n->PrevState == 0) n->GravityMult = 0;
	else if (n->PrevState == 1) {
		if (n->State2 < 70) { n->State2 += FPSfactor; n->GravityMult = 0; }
		else n->GravityMult = 1;
		if (entity_y(n->Collider, 0) > -1531 * RS + .35f) {
			float dist = entity_distance(Collider, n->Collider);
			if (dist < .8f) { // out of its way
				float d = point_direction(entity_x(Collider, 1), entity_z(Collider, 1), entity_x(n->Collider, 1), entity_z(n->Collider, 1));
				translate_entity(Collider, cosf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0, sinf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0);
			}
			if (entity_y(n->Collider, 0) > .6f) entity_type(n->Collider, 0, 0);
		} else {
			e->EventState += FPSfactor;
			AnimateNPC(n, 270, 286, .4f, 0);
			if (!e->Sound) {
				LoadEventSound(e, "SFX\\General\\BodyFall.ogg", 0);
				e->SoundCHN = PlaySound_Strict(e->Sound);
				Decals *de = CreateDecal(3, X(R->obj), -1531 * RS, Z(R->obj), 90, irand(1, 360), 0);
				if (de) { de->Size = .4f; scale_sprite(de->obj, de->Size, de->Size); UpdateDecals(); }
			}
			if (n->Frame == 286) n->PrevState = 2;
		}
		if (!n->SoundChn2) {
			n->Sound2 = LoadSound_Strict("SFX\\Room\\895Chamber\\GuardRadio.ogg");
			n->SoundChn2 = LoopSound2(n->Sound2, n->SoundChn2, Camera, n->Collider, 5, 1);
		}
	} else if (n->PrevState == 2) {
		if (!channel_playing(e->SoundCHN) && e->Sound) { FreeSound_Strict(e->Sound); e->Sound = 0; e->SoundCHN = 0; }
		if (!channel_playing(n->SoundChn) && n->Sound) { FreeSound_Strict(n->Sound); n->Sound = 0; n->SoundChn = 0; }
		if (!n->Sound2) n->Sound2 = LoadSound_Strict("SFX\\Room\\895Chamber\\GuardRadio.ogg");
		n->SoundChn2 = LoopSound2(n->Sound2, n->SoundChn2, Camera, n->Collider, 5, 1);
	}
}

static void gore(int state2, int sfx) { // a gore pic on the goggles. sfx plays once per goggle state
	entity_texture(NVOverlay, GorePics[irand(0, 5)], 0, 0);
	Items *g = goggles();
	if (!g) return;
	if (g->state2 == state2 - 1) PlaySound_Strict(HorrorSFX[sfx]);
	g->state2 = state2;
}

static void nv895(Events *e) { // looking at 895 through nv goggles
	if (!battery() || CoffinDistance >= 4 || Wearing714) return;
	int o1 = R->Objects[1];
	float a = point_direction(entity_x(Collider, 1), entity_z(Collider, 1), XG(o1), ZG(o1)), yaw = entity_yaw(Collider, 0);
	turn_entity(Collider, 0, angleDist(a + 90 + sind(WrapAngle(e->EventState3 / 10)), yaw) / 4, 0, 1);
	float d = fabsf(point_distance(entity_x(Collider, 1), entity_z(Collider, 1), XG(o1), ZG(o1)));
	user_camera_pitch = user_camera_pitch * .8f + -60 * Min(Max((2 - d) / 2, 0), 1) * .2f;
	Sanity -= FPSfactor * 1.1f / WearingNightVision;
	RestoreSanity = 0;
	BlurTimer = sind(MilliSecs2() / 10) * fabsf(Sanity);
	if (VomitTimer < 0) { RestoreSanity = 0; Sanity = -1010; }
	if (Sanity < -1000) {
		sset(&DeathMSG, WearingNightVision > 1
			? "\"Class D viewed SCP-895 through a pair of digital night vision goggles, presumably enhanced by SCP-914. It might be possible that the subject "
			  "was able to resist the memetic effects partially through these goggles. The goggles have been stored for further study.\""
			: "\"Class D viewed SCP-895 through a pair of digital night vision goggles, killing him.\"");
		entity_texture(NVOverlay, NVTexture, 0, 0);
		if (VomitTimer < -10) Kill();
	} else if (Sanity < -800) {
		if (irand(1, 3) == 1) entity_texture(NVOverlay, NVTexture, 0, 0);
		if (irand(1, 6) < 5) gore(2, 1);
		BlurTimer = 1000;
		if (VomitTimer == 0) VomitTimer = 1;
	} else if (Sanity < -500) {
		if (irand(1, 7) == 1) entity_texture(NVOverlay, NVTexture, 0, 0);
		if (irand(1, 50) == 1) gore(1, 0);
	} else {
		entity_texture(NVOverlay, NVTexture, 0, 0);
		for (int i = 0; i < MaxItemAmount; i++) if (worn(Inventory[i])) Inventory[i]->state2 = 0;
	}
}

static int coffin(Events *e) {
	if (e->EventState < MilliSecs2()) { // 079 broadcasts the coffin once you're out of the first zone
		if (PlayerZone > 0) cams(e, entity_pitch(R->Levers[0], 1) > 0, 0);
		e->EventState = MilliSecs2() + 3000;
	}
	if (!IN) { CoffinDistance = R->dist; return 0; }
	int o1 = R->Objects[1];
	CoffinDistance = entity_distance(Collider, o1);
	if (CoffinDistance < 1.5f) {
		GiveAchievement(Achv895, 1);
		if (!Contained106 && s_eq(e->EventName, "coffin106") && e->EventState2 == 0) {
			Decals *de = CreateDecal(0, XG(o1), -1531 * RS, ZG(o1), 90, irand(1, 360), 0);
			if (de) { de->Size = .05f; de->SizeChange = .001f; entity_alpha(de->obj, .8f); UpdateDecals(); }
			if (Curr106->State > 0) {
				position_entity(Curr106->Collider, XG(o1), -10240 * RS, ZG(o1), 0);
				Curr106->State = -.1f;
				show_entity(Curr106->obj);
				e->EventState2 = 1;
			}
		}
	} else if (CoffinDistance < 3) {
		if (!llive(R->NPC[0])) {
			NPCs *n = R->NPC[0] = CreateNPC(NPCtypeGuard, R->x, R->y, R->z);
			rotate_entity(n->Collider, 0, R->angle + 90, 0, 0);
			n->State = 8;
			SetNPCFrame(n, 270);
			n->GravityMult = 0;
			n->Sound = LoadSound_Strict(sf("SFX\\Room\\895Chamber\\GuardIdle%d.ogg", irand(1, 3)));
			n->SoundChn = PlaySound2(n->Sound, Camera, n->Collider, 10, 1);
			n->IsDead = 1;
			n->FallingPickDistance = 0;
		}
	} else if (CoffinDistance > 5) {
		NPCs *n = llive(R->NPC[0]);
		if (n && n->PrevState == 0) { // he falls in
			if (channel_playing(n->SoundChn)) stop_channel(n->SoundChn);
			FreeSound_Strict(n->Sound);
			n->Sound = LoadSound_Strict(sf("SFX\\Room\\895Chamber\\GuardScream%d.ogg", irand(1, 3)));
			n->SoundChn = PlaySound2(n->Sound, Camera, n->Collider, 100, 1);
			n->PrevState = 1;
			n->State2 = 0;
		}
	}
	guard(e);
	if (WearingNightVision > 0) nv895(e);
	if (e->EventState3 > 0) e->EventState3 = Max(e->EventState3 - FPSfactor, 0);
	if (e->EventState3 == 0) {
		e->EventState3 = -1;
		entity_texture(NVOverlay, NVTexture, 0, 0);
		if (WearingNightVision == 1) entity_color(NVOverlay, 0, 255, 0);
		else if (WearingNightVision == 2) entity_color(NVOverlay, 0, 100, 255);
	}
	ShouldPlay = 66;
	cams(e, UpdateLever(R->Levers[0], 0), 1);
	return 0;
}

const EvEntry Evs4[] = {{"room012", room012}, {"coffin", coffin}, {"coffin106", coffin}, {NULL, NULL}};
