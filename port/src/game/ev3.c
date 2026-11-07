// ev3.c: endroom106 (the janitor), 008, the checkpoints, the tesla gates (UpdateEvents.bb)
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return bbEntityX(o, 1); }
static float YG(int o) { return bbEntityY(o, 1); }
static float ZG(int o) { return bbEntityZ(o, 1); }
static float X(int o) { return bbEntityX(o, 0); }
static float Z(int o) { return bbEntityZ(o, 0); }
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
		bbPointEntity(n->Collider, R->obj, 0);
		bbRotateEntity(n->Collider, 0, bbEntityYaw(n->Collider, 0), 0, 1);
		bbMoveEntity(n->Collider, 0, 0, .5f);
		d->open = 0;
		PlaySound2(LoadTempSound("SFX\\Door\\EndroomDoor.ogg"), Camera, R->obj, 15, 1);
		e->EventState = 1;
	} else if (e->EventState == 1) {
		if (IN) {
			n->State = 1;
			e->EventState = 2;
			e->Sound = LoadSound_Strict("SFX\\Character\\Janitor\\106Abduct.ogg");
			PlaySound_Strict(e->Sound);
			if (e->SoundCHN) bbStopChannel(e->SoundCHN);
		} else if (R->dist < 8) {
			if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Character\\Janitor\\Idle.ogg");
			e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, n->obj, 15, 1);
		}
	} else if (e->EventState == 2) {
		if (bbEntityDistance(n->Collider, R->obj) < 1.5f) {
			Decals *de = CreateDecal(0, X(R->obj), .01f, Z(R->obj), 90, bbRand(360, 1), 0);
			if (de) { de->Size = .05f; de->SizeChange = .008f; de->Timer = 10000; UpdateDecals(); }
			e->EventState = 3;
		}
	} else {
		float dist = Distance(X(n->Collider), Z(n->Collider), X(R->obj), Z(R->obj));
		NPCs *o = Curr106;
		bbPositionEntity(o->obj, XG(R->obj), 0, ZG(R->obj), 0);
		bbPointEntity(o->obj, n->Collider, 0);
		bbRotateEntity(o->obj, 0, bbEntityYaw(o->obj, 0), 0, 1);
		o->Idle = 1;
		if (dist < .4f) {
			if (n->State == 1) SetNPCFrame(n, 41);
			e->EventState += FPSfactor / 2;
			n->State = 6;
			n->CurrSpeed = CurveValue(0, n->CurrSpeed, 25);
			bbPositionEntity(n->Collider, CurveValue(XG(R->obj), X(n->Collider), 25), .3f - e->EventState / 70, CurveValue(ZG(R->obj), Z(n->Collider), 25), 0);
			bbResetEntity(n->Collider);
			AnimateNPC(n, 41, 58, .1f, 0);
			AnimateNPC(o, 206, 112, -1, 0);
		} else AnimateNPC(o, 112, 206, 1.5f, 0);
		CurrSpeed = Min(CurrSpeed - CurrSpeed * (.15f / bbEntityDistance(n->Collider, Collider)) * FPSfactor, CurrSpeed);
		if (e->EventState > 100) {
			bbPositionEntity(o->obj, X(o->Collider), -100, Z(o->Collider), 1);
			bbPositionEntity(o->Collider, X(o->Collider), -100, Z(o->Collider), 1);
			o->Idle = 0;
			if (bbEntityDistance(Collider, R->obj) < 2.5f) o->State = -.1f;
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
		if (Curr173->Idle < 2 && bbEntityDistance(Curr173->Collider, Collider) > HideDistance) { // 173 far enough to bring it here
			bbPositionEntity(Curr173->Collider, XG(R->Objects[3]), .5f, ZG(R->Objects[3]), 1);
			bbResetEntity(Curr173->Collider);
		}
		e->EventState = 1;
	} else if (e->EventState == 1) {
		e->SoundCHN = LoopSound2(AlarmSFX[0], e->SoundCHN, Camera, o0, 5, 1);
		if (MilliSecs2() % 1000 < 500) bbShowEntity(R->Objects[5]); else bbHideEntity(R->Objects[5]);
		float dist = bbEntityDistance(Collider, o0);
		if (dist < 2) {
			R->RoomDoors[0]->locked = R->RoomDoors[1]->locked = 1;
			if (e->EventState2 == 0) {
				bbShowEntity(R->Objects[2]);
				if (bbEntityDistance(Curr173->Collider, R->Objects[4]) < 3 && (BlinkTimer < -10 || !bbEntityInView(Curr173->obj, Camera)) && Curr173->Idle == 0) {
					bbPositionEntity(Curr173->Collider, XG(R->Objects[4]), .5f, ZG(R->Objects[4]), 1); // 173 smashes the window
					bbResetEntity(Curr173->Collider);
					bbHideEntity(R->Objects[2]);
					if (!WearingHazmat) {
						Injuries += .1f;
						if (Infect == 0) Infect = 1;
						Msg = "The window shattered and a piece of glass cut your arm."; MsgTimer = 70 * 8;
					}
					PlaySound2(LoadTempSound("SFX\\General\\GlassBreak.ogg"), Camera, o0, 10, 1);
					e->EventState2 = 1;
				}
			}
			if (dist < 1 && bbEntityInView(o0, Camera)) {
				DrawHandIcon = 1;
				if (MouseDown1) {
					DrawArrowIcon[2] = 1;
					bbRotateEntity(lev, Max(Min(bbEntityPitch(lev, 0) + Max(Min(-mouse_y_speed_1, 10), -10), 89), 35), bbEntityYaw(lev, 0), 0, 0);
				}
			}
			if (!WearingHazmat && Bloodloss > 0 && Infect == 0) Infect = 1;
		}
		if (bbEntityPitch(lev, 1) < 40) { e->EventState = 2; PlaySound_Strict(LeverSFX); }
		else { // the leak
			Particles *p = CreateParticle(XG(o0), YG(o0), ZG(o0), 6, .02f, -.12f, 200);
			bbRotateEntity(p->pvt, -90, 0, 0, 1);
			float ry = bbRnd(-26, 26), rr = bbRnd(360, 0), rp = bbRnd(-26, 26);
			bbTurnEntity(p->pvt, rp, ry, rr, 0);
			p->SizeChange = .012f; p->Achange = -.015f;
		}
	} else {
		bbHideEntity(R->Objects[5]);
		for (int i = 0; i <= 2; i++) R->RoomDoors[i]->locked = 0;
		bbRotateEntity(lev, CurveAngle(1, bbEntityPitch(lev, 1), 15), bbEntityYaw(lev, 1), 0, 1);
		if (bbEntityPitch(lev, 1) <= 1) RemoveEvent(e);
	}
	return 0;
}

static void lock(Events *e, int on) { R->RoomDoors[0]->locked = R->RoomDoors[1]->locked = on; }

static int checkpoint(Events *e) {
	if (IN) {
		if (e->EventState2 == 0 && bbEntityZ(Collider, 0) < R->z) { // through the gate
			PlaySound_Strict(LoadTempSound(PlayerZone == 1 ? "SFX\\Ambient\\ToZone2.ogg" : "SFX\\Ambient\\ToZone3.ogg"));
			e->EventState2 = 1;
		}
		int *o1 = &R->Objects[1];
		if (e->EventState3 == 0) { // half the time 1048 waves at you
			if (bbRand(2, 1) == 1) {
				GiveAchievement(Achv1048, 1);
				*o1 = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048.b3d", 0);
				bbScaleEntity(*o1, .05f, .05f, .05f, 0);
				bbPositionEntity(*o1, XG(R->Objects[0]), YG(R->Objects[0]), ZG(R->Objects[0]), 0);
				bbSetAnimTime(*o1, 267, 0);
			}
			e->EventState3 = 1;
		} else if (*o1) {
			float angle;
			if (e->EventState3 == 1 || e->EventState3 == 2) {
				bbPointEntity(*o1, Collider, 0);
				bbRotateEntity(*o1, -90, bbEntityYaw(*o1, 0), 0, 0);
			}
			if (e->EventState3 == 1) {
				angle = WrapAngle(bbDeltaYaw(Collider, *o1));
				if (angle < 40 || angle > 320) e->EventState3 = 2;
			} else if (e->EventState3 == 2) {
				Animate2(*o1, bbAnimTime(*o1), 267, 283, .3f, 0);
				if (bbAnimTime(*o1) == 283) e->EventState3 = 3;
			} else if (e->EventState3 == 3) {
				Animate2(*o1, bbAnimTime(*o1), 283, 267, -.2f, 0);
				if (bbAnimTime(*o1) == 267) e->EventState3 = 4;
			} else if (e->EventState3 == 4) {
				angle = WrapAngle(bbDeltaYaw(Collider, *o1));
				if (angle > 90 && angle < 270) { bbFreeEntity(*o1); *o1 = 0; e->EventState3 = 5; }
			}
		}
	}
	if (bbSeq(R->RoomTemplate->Name, "checkpoint2")) { // locked while 008's leaking
		each(Events, e2) {
			if (!bbSeq(e2->EventName, "008")) continue;
			if (e2->EventState == 2) { if (R->RoomDoors[0]->locked) { TurnCheckpointMonitorsOff(1); lock(e, 0); } }
			else if (R->dist < 12) { UpdateCheckpointMonitors(1); lock(e, 1); }
		}
	} else { // locked while room2sl's lockdown's on
		each(Events, e2) {
			if (!bbSeq(e2->EventName, "room2sl") || R->dist >= 12) continue;
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
	if (bbChannelPlaying(e->SoundCHN)) UpdateSoundOrigin(e->SoundCHN, Camera, R->RoomDoors[0]->obj, 10, 1);
	if (bbChannelPlaying(e->SoundCHN2)) UpdateSoundOrigin(e->SoundCHN2, Camera, R->RoomDoors[1]->obj, 10, 1);
	return 0;
}

// ---- tesla gates. EventState: shock timer, EventState2: mtf announcement timer, EventStr: the clerk's script, EventVal: his body's smoke timer

static void activate(Events *e) {
	bbStopChannel(e->SoundCHN);
	e->SoundCHN = PlaySound2(TeslaActivateSFX, Camera, R->Objects[3], 4, .5f);
}

static void idle(Events *e) { // EventState 0: humming, waiting for someone to step in
	if (R->dist < 8) {
		bbHideEntity(R->Objects[3]);
		if (MilliSecs2() % 1500 < 800) bbShowEntity(R->Objects[4]); else bbHideEntity(R->Objects[4]);
		if (!e->SoundCHN || !bbChannelPlaying(e->SoundCHN)) e->SoundCHN = PlaySound2(TeslaIdleSFX, Camera, R->Objects[3], 4, .5f);
		for (int i = 0; i <= 2; i++) if (flat(Collider, R->Objects[i]) < 300 * RS && KillTimer >= 0) {
			PlayerSoundVolume = Max(8, PlayerSoundVolume);
			activate(e);
			e->EventState = 1;
			break;
		}
		int first = 1; // only one tesla gate gets the clerk
		each(Events, e2) if (bbSeq(e2->EventName, e->EventName) && e2 != e && bbLen(e2->EventStr)) { first = 0; e->EventStr = "done"; break; }
		if (first && !bbLen(e->EventStr) && IN) {
			int o = bbEntityDistance(R->Objects[5], Collider) < bbEntityDistance(R->Objects[6], Collider) ? R->Objects[6] : R->Objects[5];
			NPCs *n = R->NPC[0] = CreateNPC(NPCtypeClerk, XG(o), .5f, ZG(o));
			bbPointEntity(n->Collider, R->Objects[2], 0);
			n->State = 2;
			e->EventStr = "step1";
			e->EventState = e->EventState2 = e->EventState3 = 0;
		}
	} else bbHideEntity(R->Objects[4]);
	if (Curr106->State < -10 && e->EventState == 0) {
		for (int i = 0; i <= 2; i++) if (flat(Curr106->Collider, R->Objects[i]) < 300 * RS && KillTimer >= 0) {
			activate(e);
			bbHideEntity(R->Objects[4]);
			e->EventState = 1;
			Curr106->State = 70 * 60 * bbRand(10, 13);
			GiveAchievement(AchvTesla, 1);
			break;
		}
	}
}

static void zap106(void) {
	bbShowEntity(Light);
	LightFlash = .3f;
	if (ParticleAmount > 0) for (int i = 0; i <= 5 + 5 * (ParticleAmount - 1); i++) {
		Particles *p = CreateParticle(bbEntityX(Curr106->Collider, 1), bbEntityY(Curr106->Collider, 1), bbEntityZ(Curr106->Collider, 1), 0, .015f, -.2f, 250);
		p->size = .03f; p->gravity = -.2f; p->lifetime = 200; p->SizeChange = .005f; p->speed = .001f;
		float ry = bbRnd(360, 0), rp = bbRnd(360, 0);
		bbRotateEntity(p->pvt, rp, ry, 0, 1);
	}
	Curr106->State = -20000;
	bbTranslateEntity(Curr106->Collider, 0, -50, 0, 1);
}

static void shock(Events *e) { // EventState counting up: charge, zap, power back up
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s <= 40) {
		bbHideEntity(R->Objects[3]);
		if (MilliSecs2() % 100 < 50) bbShowEntity(R->Objects[4]); else bbHideEntity(R->Objects[4]);
		return;
	}
	if (s - FPSfactor <= 40) { if (R->dist < 2) PlaySound_Strict(e->Sound); else PlaySound2(e->Sound, Camera, R->Objects[2], 10, 1); }
	if (s >= 70) {
		if (s - FPSfactor < 70) { bbStopChannel(e->SoundCHN); e->SoundCHN = PlaySound2(TeslaPowerUpSFX, Camera, R->Objects[3], 4, .5f); }
		bbHideEntity(R->Objects[3]);
		if (s > 150) e->EventState = 0;
		return;
	}
	if (KillTimer >= 0) for (int i = 0; i <= 2; i++) if (flat(Collider, R->Objects[i]) < 250 * RS) {
		bbShowEntity(Light); LightFlash = .4f; CameraShake = 1;
		Kill();
		DeathMSG = "Subject D-9341 killed by the Tesla gate at [REDACTED].";
	}
	if (bbSeq(e->EventStr, "step1")) R->NPC[0]->State = 3;
	if (Curr106->State < -10) {
		for (int i = 0; i <= 2; i++) if (flat(Curr106->Collider, R->Objects[i]) < 250 * RS) {
			zap106();
			break;
		}
	}
	bbHideEntity(R->Objects[3]); bbHideEntity(R->Objects[4]);
	if (bbRand(5, 1) < 5) { bbPositionTexture(TeslaTexture, 0, bbRnd(0, 1)); bbShowEntity(R->Objects[3]); }
}

static void clerk(Events *e) { // the clerk who runs into the gate
	NPCs *n = bbLive(R->NPC[0]);
	if (!n) return;
	str s = e->EventStr;
	if (bbSeq(s, "step1") && n->State != 3) {
		if (e->EventState == 0) for (int i = 0; i <= 2; i++) if (flat(n->Collider, R->Objects[i]) < 400 * RS) {
			activate(e);
			bbHideEntity(R->Objects[4]);
			e->EventState = 1;
			break;
		}
	} else if (bbSeq(s, "step1") && n->State == 3) {
		n->CurrSpeed = 0;
		AnimateNPC(n, 41, 60, .5f, 0);
		if (n->Frame == 60) { n->IsDead = 1; e->EventStr = "step2"; SetNPCFrame(n, 57); }
	} else if (bbSeq(s, "step2")) {
		AnimateNPC(n, 57, 60, .5f, 0);
		if (n->Frame == 60) { e->EventStr = "smoke"; e->EventVal = 0; }
	} else if (bbLen(s) && !bbSeq(s, "step1") && !bbSeq(s, "done")) { // the body smokes for 10s
		if (e->EventVal < 70 * 10) {
			if (ParticleAmount > 0 && bbRand(20 - 10 * (ParticleAmount - 1), 1) == 1) {
				Particles *p = CreateParticle(bbEntityX(n->Collider, 0), bbEntityY(n->obj, 0) + .05f, bbEntityZ(n->Collider, 0), 0, .05f, 0, 60);
				p->speed = .002f;
				bbRotateEntity(p->pvt, 0, bbEntityYaw(n->Collider, 0), 0, 0);
				float mx = bbRnd(-.1f, .1f), mz = bbRnd(0, .5f);
				bbMoveEntity(p->pvt, mx, 0, .1f + mz);
				bbRotateEntity(p->pvt, -90, bbEntityYaw(n->Collider, 0), 0, 0);
				p->Achange = -.02f;
			}
			e->EventVal += FPSfactor;
		} else e->EventStr = "done";
	}
}

static int tesla(Events *e) {
	int temp = !(e->EventState2 > 70 * 3.5f && e->EventState2 < 70 * 90);
	if (temp && bbEntityY(Collider, 1) > bbEntityY(R->obj, 1) && bbEntityY(Collider, 1) < 4) {
		if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Room\\Tesla\\Shock.ogg");
		if (e->EventState == 0) idle(e);
		else shock(e);
	} else bbHideEntity(R->Objects[4]);
	clerk(e);
	str rn = PlayerRoom->RoomTemplate->Name;
	if (bbSeq(rn, "pocketdimension") || bbSeq(rn, "room860")) return 0;
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
		if (e->EventState2 >= 70 * 92 && e->EventState2 - FPSfactor < 70 * 92) PlayAnnouncement(sf("SFX\\Character\\MTF\\Tesla%d.ogg", bbRand(1, 3)));
		e->EventState2 = Max(e->EventState2 - FPSfactor, 0);
	}
	return 0;
}

const EvEntry Evs3[] = {
	{"endroom106", endroom106}, {"008", scp008}, {"checkpoint", checkpoint}, {"room2tesla", tesla}, {NULL, NULL},
};
