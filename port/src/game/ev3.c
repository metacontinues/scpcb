// ev3.c: endroom106 (the janitor), 008, the checkpoints, the tesla gates (UpdateEvents.bb)
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return entity_x(o, 1); }
static float YG(int o) { return entity_y(o, 1); }
static float ZG(int o) { return entity_z(o, 1); }
static float X(int o) { return entity_x(o, 0); }
static float Z(int o) { return entity_z(o, 0); }
static float flat(int a, int b) { return Distance(X(a), Z(a), XG(b), ZG(b)); } // a's local x/z vs b's global, like the blitz lines

static int endroom106(Events *e) { // 106 takes the janitor
	if (Contained106) return 0;
	NPCs *n = R->NPC[0];
	if (e->EventState == 0) {
		if (!(R->dist < 8 && R->dist > 0)) return 0;
		if (Curr106->State < 0) { RemoveEvent(e); return 0; }
		Doors *d = R->RoomDoors[0];
		d->open = 1;
		n = R->NPC[0] = CreateNPC(NPCtypeD, XG(d->obj), .5f, ZG(d->obj));
		ChangeNPCTextureID(n, 4);
		point_entity(n->Collider, R->obj, 0);
		rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), 0, 1);
		move_entity(n->Collider, 0, 0, .5f);
		d->open = 0;
		PlaySound2(LoadTempSound("SFX\\Door\\EndroomDoor.ogg"), Camera, R->obj, 15, 1);
		e->EventState = 1;
	} else if (e->EventState == 1) {
		if (IN) {
			n->State = 1;
			e->EventState = 2;
			e->Sound = LoadSound_Strict("SFX\\Character\\Janitor\\106Abduct.ogg");
			PlaySound_Strict(e->Sound);
			if (e->SoundCHN) stop_channel(e->SoundCHN);
		} else if (R->dist < 8) {
			if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Character\\Janitor\\Idle.ogg");
			e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, n->obj, 15, 1);
		}
	} else if (e->EventState == 2) {
		if (entity_distance(n->Collider, R->obj) < 1.5f) {
			Decals *de = CreateDecal(0, X(R->obj), .01f, Z(R->obj), 90, irand(360, 1), 0);
			if (de) { de->Size = .05f; de->SizeChange = .008f; de->Timer = 10000; UpdateDecals(); }
			e->EventState = 3;
		}
	} else {
		float dist = Distance(X(n->Collider), Z(n->Collider), X(R->obj), Z(R->obj));
		NPCs *o = Curr106;
		position_entity(o->obj, XG(R->obj), 0, ZG(R->obj), 0);
		point_entity(o->obj, n->Collider, 0);
		rotate_entity(o->obj, 0, entity_yaw(o->obj, 0), 0, 1);
		o->Idle = 1;
		if (dist < .4f) {
			if (n->State == 1) SetNPCFrame(n, 41);
			e->EventState += FPSfactor / 2;
			n->State = 6;
			n->CurrSpeed = CurveValue(0, n->CurrSpeed, 25);
			position_entity(n->Collider, CurveValue(XG(R->obj), X(n->Collider), 25), .3f - e->EventState / 70, CurveValue(ZG(R->obj), Z(n->Collider), 25), 0);
			reset_entity(n->Collider);
			AnimateNPC(n, 41, 58, .1f, 0);
			AnimateNPC(o, 206, 112, -1, 0);
		} else AnimateNPC(o, 112, 206, 1.5f, 0);
		CurrSpeed = Min(CurrSpeed - CurrSpeed * (.15f / entity_distance(n->Collider, Collider)) * FPSfactor, CurrSpeed);
		if (e->EventState > 100) {
			position_entity(o->obj, X(o->Collider), -100, Z(o->Collider), 1);
			position_entity(o->Collider, X(o->Collider), -100, Z(o->Collider), 1);
			o->Idle = 0;
			if (entity_distance(Collider, R->obj) < 2.5f) o->State = -.1f;
			RemoveNPC(n);
			RemoveEvent(e);
		}
	}
	return 0;
}

static int scp008(Events *e) {
	if (!IN) return 0;
	GiveAchievement(Achv008, 1);
	int o0 = R->Objects[0], lev = R->Levers[0];
	if (e->EventState == 0) { // container open
		if (Curr173->Idle < 2 && entity_distance(Curr173->Collider, Collider) > HideDistance) { // 173 far enough to bring it here
			position_entity(Curr173->Collider, XG(R->Objects[3]), .5f, ZG(R->Objects[3]), 1);
			reset_entity(Curr173->Collider);
		}
		e->EventState = 1;
	} else if (e->EventState == 1) {
		e->SoundCHN = LoopSound2(AlarmSFX[0], e->SoundCHN, Camera, o0, 5, 1);
		if (MilliSecs2() % 1000 < 500) show_entity(R->Objects[5]); else hide_entity(R->Objects[5]);
		float dist = entity_distance(Collider, o0);
		if (dist < 2) {
			R->RoomDoors[0]->locked = R->RoomDoors[1]->locked = 1;
			if (e->EventState2 == 0) {
				show_entity(R->Objects[2]);
				if (entity_distance(Curr173->Collider, R->Objects[4]) < 3 && (BlinkTimer < -10 || !entity_in_view(Curr173->obj, Camera)) && Curr173->Idle == 0) {
					position_entity(Curr173->Collider, XG(R->Objects[4]), .5f, ZG(R->Objects[4]), 1); // 173 smashes the window
					reset_entity(Curr173->Collider);
					hide_entity(R->Objects[2]);
					if (!WearingHazmat) {
						Injuries += .1f;
						if (Infect == 0) Infect = 1;
						Msg = "The window shattered and a piece of glass cut your arm."; MsgTimer = 70 * 8;
					}
					PlaySound2(LoadTempSound("SFX\\General\\GlassBreak.ogg"), Camera, o0, 10, 1);
					e->EventState2 = 1;
				}
			}
			if (dist < 1 && entity_in_view(o0, Camera)) {
				DrawHandIcon = 1;
				if (MouseDown1) {
					DrawArrowIcon[2] = 1;
					rotate_entity(lev, Max(Min(entity_pitch(lev, 0) + Max(Min(-mouse_y_speed_1, 10), -10), 89), 35), entity_yaw(lev, 0), 0, 0);
				}
			}
			if (!WearingHazmat && Bloodloss > 0 && Infect == 0) Infect = 1;
		}
		if (entity_pitch(lev, 1) < 40) { e->EventState = 2; PlaySound_Strict(LeverSFX); }
		else { // the leak
			Particles *p = CreateParticle(XG(o0), YG(o0), ZG(o0), 6, .02f, -.12f, 200);
			rotate_entity(p->pvt, -90, 0, 0, 1);
			float ry = frnd(-26, 26), rr = frnd(360, 0), rp = frnd(-26, 26);
			turn_entity(p->pvt, rp, ry, rr, 0);
			p->SizeChange = .012f; p->Achange = -.015f;
		}
	} else {
		hide_entity(R->Objects[5]);
		for (int i = 0; i <= 2; i++) R->RoomDoors[i]->locked = 0;
		rotate_entity(lev, CurveAngle(1, entity_pitch(lev, 1), 15), entity_yaw(lev, 1), 0, 1);
		if (entity_pitch(lev, 1) <= 1) RemoveEvent(e);
	}
	return 0;
}

static void lock(Events *e, int on) { R->RoomDoors[0]->locked = R->RoomDoors[1]->locked = on; }

static int checkpoint(Events *e) {
	if (IN) {
		if (e->EventState2 == 0 && entity_z(Collider, 0) < R->z) { // through the gate
			PlaySound_Strict(LoadTempSound(PlayerZone == 1 ? "SFX\\Ambient\\ToZone2.ogg" : "SFX\\Ambient\\ToZone3.ogg"));
			e->EventState2 = 1;
		}
		int *o1 = &R->Objects[1];
		if (e->EventState3 == 0) { // half the time 1048 waves at you
			if (irand(2, 1) == 1) {
				GiveAchievement(Achv1048, 1);
				*o1 = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048.b3d", 0);
				scale_entity(*o1, .05f, .05f, .05f, 0);
				position_entity(*o1, XG(R->Objects[0]), YG(R->Objects[0]), ZG(R->Objects[0]), 0);
				set_anim_time(*o1, 267, 0);
			}
			e->EventState3 = 1;
		} else if (*o1) {
			float angle;
			if (e->EventState3 == 1 || e->EventState3 == 2) {
				point_entity(*o1, Collider, 0);
				rotate_entity(*o1, -90, entity_yaw(*o1, 0), 0, 0);
			}
			if (e->EventState3 == 1) {
				angle = WrapAngle(delta_yaw(Collider, *o1));
				if (angle < 40 || angle > 320) e->EventState3 = 2;
			} else if (e->EventState3 == 2) {
				Animate2(*o1, anim_time(*o1), 267, 283, .3f, 0);
				if (anim_time(*o1) == 283) e->EventState3 = 3;
			} else if (e->EventState3 == 3) {
				Animate2(*o1, anim_time(*o1), 283, 267, -.2f, 0);
				if (anim_time(*o1) == 267) e->EventState3 = 4;
			} else if (e->EventState3 == 4) {
				angle = WrapAngle(delta_yaw(Collider, *o1));
				if (angle > 90 && angle < 270) { free_entity(*o1); *o1 = 0; e->EventState3 = 5; }
			}
		}
	}
	if (s_eq(R->RoomTemplate->Name, "checkpoint2")) { // locked while 008's leaking
		each(Events, e2) {
			if (!s_eq(e2->EventName, "008")) continue;
			if (e2->EventState == 2) { if (R->RoomDoors[0]->locked) { TurnCheckpointMonitorsOff(1); lock(e, 0); } }
			else if (R->dist < 12) { UpdateCheckpointMonitors(1); lock(e, 1); }
		}
	} else { // locked while room2sl's lockdown's on
		each(Events, e2) {
			if (!s_eq(e2->EventName, "room2sl") || R->dist >= 12) continue;
			if (e2->EventState3 == 0) { TurnCheckpointMonitorsOff(0); lock(e, 0); }
			else { UpdateCheckpointMonitors(0); lock(e, 1); }
		}
	}
	if (R->RoomDoors[0]->open != e->EventState) {
		if (!e->Sound) LoadEventSound(e, "SFX\\Door\\DoorCheckpoint.ogg", 0);
		e->SoundCHN = PlaySound2(e->Sound, Camera, R->RoomDoors[0]->obj, 10, 1);
		e->SoundCHN2 = PlaySound2(e->Sound, Camera, R->RoomDoors[1]->obj, 10, 1);
	}
	e->EventState = R->RoomDoors[0]->open;
	if (channel_playing(e->SoundCHN)) UpdateSoundOrigin(e->SoundCHN, Camera, R->RoomDoors[0]->obj, 10, 1);
	if (channel_playing(e->SoundCHN2)) UpdateSoundOrigin(e->SoundCHN2, Camera, R->RoomDoors[1]->obj, 10, 1);
	return 0;
}

// ---- tesla gates. EventState: shock timer, EventState2: mtf announcement timer, EventStr: the clerk's script, EventVal: his body's smoke timer

static void activate(Events *e) {
	stop_channel(e->SoundCHN);
	e->SoundCHN = PlaySound2(TeslaActivateSFX, Camera, R->Objects[3], 4, .5f);
}

static void idle(Events *e) { // EventState 0: humming, waiting for someone to step in
	if (R->dist < 8) {
		hide_entity(R->Objects[3]);
		if (MilliSecs2() % 1500 < 800) show_entity(R->Objects[4]); else hide_entity(R->Objects[4]);
		if (!e->SoundCHN || !channel_playing(e->SoundCHN)) e->SoundCHN = PlaySound2(TeslaIdleSFX, Camera, R->Objects[3], 4, .5f);
		for (int i = 0; i <= 2; i++) if (flat(Collider, R->Objects[i]) < 300 * RS && KillTimer >= 0) {
			PlayerSoundVolume = Max(8, PlayerSoundVolume);
			activate(e);
			e->EventState = 1;
			break;
		}
		int first = 1; // only one tesla gate gets the clerk
		each(Events, e2) if (s_eq(e2->EventName, e->EventName) && e2 != e && s_len(e2->EventStr)) { first = 0; e->EventStr = "done"; break; }
		if (first && !s_len(e->EventStr) && IN) {
			int o = entity_distance(R->Objects[5], Collider) < entity_distance(R->Objects[6], Collider) ? R->Objects[6] : R->Objects[5];
			NPCs *n = R->NPC[0] = CreateNPC(NPCtypeClerk, XG(o), .5f, ZG(o));
			point_entity(n->Collider, R->Objects[2], 0);
			n->State = 2;
			e->EventStr = "step1";
			e->EventState = e->EventState2 = e->EventState3 = 0;
		}
	} else hide_entity(R->Objects[4]);
	if (Curr106->State < -10 && e->EventState == 0) {
		for (int i = 0; i <= 2; i++) if (flat(Curr106->Collider, R->Objects[i]) < 300 * RS && KillTimer >= 0) {
			activate(e);
			hide_entity(R->Objects[4]);
			e->EventState = 1;
			Curr106->State = 70 * 60 * irand(10, 13);
			GiveAchievement(AchvTesla, 1);
			break;
		}
	}
}

static void zap106(void) {
	show_entity(Light);
	LightFlash = .3f;
	if (ParticleAmount > 0) for (int i = 0; i <= 5 + 5 * (ParticleAmount - 1); i++) {
		Particles *p = CreateParticle(entity_x(Curr106->Collider, 1), entity_y(Curr106->Collider, 1), entity_z(Curr106->Collider, 1), 0, .015f, -.2f, 250);
		p->size = .03f; p->gravity = -.2f; p->lifetime = 200; p->SizeChange = .005f; p->speed = .001f;
		float ry = frnd(360, 0), rp = frnd(360, 0);
		rotate_entity(p->pvt, rp, ry, 0, 1);
	}
	Curr106->State = -20000;
	translate_entity(Curr106->Collider, 0, -50, 0, 1);
}

static void shock(Events *e) { // EventState counting up: charge, zap, power back up
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s <= 40) {
		hide_entity(R->Objects[3]);
		if (MilliSecs2() % 100 < 50) show_entity(R->Objects[4]); else hide_entity(R->Objects[4]);
		return;
	}
	if (s - FPSfactor <= 40) { if (R->dist < 2) PlaySound_Strict(e->Sound); else PlaySound2(e->Sound, Camera, R->Objects[2], 10, 1); }
	if (s >= 70) {
		if (s - FPSfactor < 70) { stop_channel(e->SoundCHN); e->SoundCHN = PlaySound2(TeslaPowerUpSFX, Camera, R->Objects[3], 4, .5f); }
		hide_entity(R->Objects[3]);
		if (s > 150) e->EventState = 0;
		return;
	}
	if (KillTimer >= 0) for (int i = 0; i <= 2; i++) if (flat(Collider, R->Objects[i]) < 250 * RS) {
		show_entity(Light); LightFlash = .4f; CameraShake = 1;
		Kill();
		DeathMSG = "Subject D-9341 killed by the Tesla gate at [REDACTED].";
	}
	if (s_eq(e->EventStr, "step1")) R->NPC[0]->State = 3;
	if (Curr106->State < -10) {
		for (int i = 0; i <= 2; i++) if (flat(Curr106->Collider, R->Objects[i]) < 250 * RS) {
			zap106();
			break;
		}
	}
	hide_entity(R->Objects[3]); hide_entity(R->Objects[4]);
	if (irand(5, 1) < 5) { position_texture(TeslaTexture, 0, frnd(0, 1)); show_entity(R->Objects[3]); }
}

static void clerk(Events *e) { // the clerk who runs into the gate
	NPCs *n = llive(R->NPC[0]);
	if (!n) return;
	str s = e->EventStr;
	if (s_eq(s, "step1") && n->State != 3) {
		if (e->EventState == 0) for (int i = 0; i <= 2; i++) if (flat(n->Collider, R->Objects[i]) < 400 * RS) {
			activate(e);
			hide_entity(R->Objects[4]);
			e->EventState = 1;
			break;
		}
	} else if (s_eq(s, "step1") && n->State == 3) {
		n->CurrSpeed = 0;
		AnimateNPC(n, 41, 60, .5f, 0);
		if (n->Frame == 60) { n->IsDead = 1; e->EventStr = "step2"; SetNPCFrame(n, 57); }
	} else if (s_eq(s, "step2")) {
		AnimateNPC(n, 57, 60, .5f, 0);
		if (n->Frame == 60) { e->EventStr = "smoke"; e->EventVal = 0; }
	} else if (s_len(s) && !s_eq(s, "step1") && !s_eq(s, "done")) { // the body smokes for 10s
		if (e->EventVal < 70 * 10) {
			if (ParticleAmount > 0 && irand(20 - 10 * (ParticleAmount - 1), 1) == 1) {
				Particles *p = CreateParticle(entity_x(n->Collider, 0), entity_y(n->obj, 0) + .05f, entity_z(n->Collider, 0), 0, .05f, 0, 60);
				p->speed = .002f;
				rotate_entity(p->pvt, 0, entity_yaw(n->Collider, 0), 0, 0);
				float mx = frnd(-.1f, .1f), mz = frnd(0, .5f);
				move_entity(p->pvt, mx, 0, .1f + mz);
				rotate_entity(p->pvt, -90, entity_yaw(n->Collider, 0), 0, 0);
				p->Achange = -.02f;
			}
			e->EventVal += FPSfactor;
		} else e->EventStr = "done";
	}
}

static int tesla(Events *e) {
	int temp = !(e->EventState2 > 70 * 3.5f && e->EventState2 < 70 * 90);
	if (temp && entity_y(Collider, 1) > entity_y(R->obj, 1) && entity_y(Collider, 1) < 4) {
		if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Room\\Tesla\\Shock.ogg");
		if (e->EventState == 0) idle(e);
		else shock(e);
	} else hide_entity(R->Objects[4]);
	clerk(e);
	str rn = PlayerRoom->RoomTemplate->Name;
	if (s_eq(rn, "pocketdimension") || s_eq(rn, "room860")) return 0;
	if (e->EventState2 == 0) { // mtf near the gate get warned off
		if (e->EventState3 <= 0) {
			temp = 0;
			each(NPCs, n) {
				if (n->NPCtype != NPCtypeMTF || fabsf(X(n->Collider) - XG(R->obj)) >= 4 || fabsf(Z(n->Collider) - ZG(R->obj)) >= 4) continue;
				temp = 1;
				if (e->EventState2 == 0) {
					n->Sound = LoadSound_Strict("SFX\\Character\\MTF\\Tesla0.ogg");
					PlayMTFSound(n->Sound, n);
					n->Idle = 70 * 10;
					e->EventState2 = 70 * 100;
				}
			}
			if (!temp) e->EventState2 = 70 * 3.5f;
			e->EventState3 += 140;
		} else e->EventState3 -= FPSfactor;
	} else {
		if (e->EventState2 >= 70 * 92 && e->EventState2 - FPSfactor < 70 * 92) PlayAnnouncement(sf("SFX\\Character\\MTF\\Tesla%d.ogg", irand(1, 3)));
		e->EventState2 = Max(e->EventState2 - FPSfactor, 0);
	}
	return 0;
}

const EvEntry Evs3[] = {
	{"endroom106", endroom106}, {"008", scp008}, {"checkpoint", checkpoint}, {"room2tesla", tesla}, {NULL, NULL},
};
