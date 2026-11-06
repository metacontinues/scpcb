// npc173.c: UpdateNPCs' cases for SCP-173 and SCP-106 (NPCtypeOldMan)
#include "g.h"

#define RN (PlayerRoom->RoomTemplate->Name)
static int is(const char *a, const char *b) { return bbSeq(a, b); }
static float sinm(float k) { return sinf(MilliSecs2() * k * DTOR); }

// 173 tries buttons of plain unlocked doors next to it
static void opendoors(NPCs *n) {
	each(Doors, d) {
		if (!(!d->locked && !d->open && !bbLen(d->Code) && d->KeyCard == 0)) continue;
		for (int i = 0; i <= 1; i++) {
			int b = d->buttons[i];
			if (!b || fabsf(bbEntityX(n->Collider, 0) - bbEntityX(b, 0)) >= .5f || fabsf(bbEntityZ(n->Collider, 0) - bbEntityZ(b, 0)) >= .5f) continue;
			if (!(d->openstate >= 180 || d->openstate <= 0)) continue;
			int pvt = bbCreatePivot(0);
			bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) + .5f, bbEntityZ(n->Collider, 0), 0);
			bbPointEntity(pvt, b, 0);
			bbMoveEntity(pvt, 0, 0, n->Speed * .6f);
			if (bbEntityPick(pvt, .5f) == b) { PlaySound_Strict(LoadTempSound("SFX\\Door\\DoorOpen173.ogg")); UseDoor(d, 0, 1); }
			bbFreeEntity(pvt);
		}
	}
}

static void snapneck(NPCs *n) {
	if (is(RN, "lockroom") || is(RN, "room2closets") || is(RN, "coffin"))
		DeathMSG = "Subject D-9341. Cause of death: Fatal cervical fracture. The surveillance tapes confirm that the subject was killed by SCP-173.";
	else if (is(RN, "173"))
		DeathMSG = "Subject D-9341. Cause of death: Fatal cervical fracture. According to Security Chief Franklin who was present at SCP-173's containment "
			"chamber during the breach, the subject was killed by SCP-173 as soon as the disruptions in the electrical network started.";
	else if (is(RN, "room2doors"))
		DeathMSG = "\"If I'm not mistaken, one of the main purposes of these rooms was to stop SCP-173 from moving further in the event of a containment breach. "
			"So, who's brilliant idea was it to put A GODDAMN MAN-SIZED VENTILATION DUCT in there?\"";
	else DeathMSG = "Subject D-9341. Cause of death: Fatal cervical fracture. Assumed to be attacked by SCP-173.";
	if (!GodMode) n->Idle = 1;
	PlaySound_Strict(NeckSnapSFX[bbRand(0, 2)]);
	if (bbRand(2, 1) == 1) bbTurnEntity(Camera, 0, bbRand(80, 100), 0, 0);
	else bbTurnEntity(Camera, 0, bbRand(-100, -80), 0, 0);
	Kill();
}

static void fwd(NPCs *n) { // translate along the collider's yaw
	float y = bbEntityYaw(n->Collider, 0) + 90;
	bbTranslateEntity(n->Collider, cosf(y * DTOR) * n->Speed * FPSfactor, 0, sinf(y * DTOR) * n->Speed * FPSfactor, 0);
}

static void idle2(NPCs *n, float dist) { // 173 in its box, carried by n->Target
	if (n->Target) {
		int tmp = dist > HideDistance * .7f && !bbEntityVisible(n->obj, Collider);
		if (!tmp) {
			bbPointEntity(n->obj, n->Target->Collider, 0);
			bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 10), 0, 1);
			dist = bbEntityDistance(n->Collider, n->Target->Collider);
			bbMoveEntity(n->Collider, 0, 0, .016f * FPSfactor * Max(Min((dist * 2 - 1) * .5f, 1), -.5f));
			n->GravityMult = 1;
		} else {
			bbPositionEntity(n->Collider, bbEntityX(n->Target->Collider, 0), bbEntityY(n->Target->Collider, 0) + .3f, bbEntityZ(n->Target->Collider, 0), 0);
			bbResetEntity(n->Collider);
			n->DropSpeed = 0; n->GravityMult = 0;
		}
	}
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) + .05f + sinm(.08f) * .02f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0) - 180, 0, 0);
	bbShowEntity(n->obj2);
	bbPositionEntity(n->obj2, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .05f + sinm(.08f) * .02f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj2, 0, bbEntityYaw(n->Collider, 0) - 180, 0, 0);
}

void Update173(NPCs *n) {
	static float SoundVol; // a blitz Local declared mid-function: keeps its value between NPCs/branches that skip it
	if (Curr173->Idle == 3) return;
	float dist = bbEntityDistance(n->Collider, Collider);
	n->State3 = 1;
	if (n->Idle >= 2) { idle2(n, dist); return; }
	if (n->IdleTimer > .1f) { n->Idle = 1; n->IdleTimer = Max(n->IdleTimer - FPSfactor, .1f); }
	else if (n->IdleTimer == .1f) { n->Idle = 0; n->IdleTimer = 0; }
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .32f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0) - 180, 0, 0);
	if (n->Idle == 0) {
		int temp = 0, move = 1;
		if (dist < 15) {
			if (dist < 10 && bbEntityVisible(n->Collider, Collider)) {
				temp = 1;
				n->EnemyX = bbEntityX(Collider, 1); n->EnemyY = bbEntityY(Collider, 1); n->EnemyZ = bbEntityZ(Collider, 1);
			}
			SoundVol = Max(Min(Distance(bbEntityX(n->Collider, 0), bbEntityZ(n->Collider, 0), n->PrevX, n->PrevZ) * 2.5f, 1), 0);
			n->SoundChn = LoopSound2(StoneDragSFX, n->SoundChn, Camera, n->Collider, 10, n->State);
			n->PrevX = bbEntityX(n->Collider, 0); n->PrevZ = bbEntityZ(n->Collider, 0);
			if ((BlinkTimer < -16 || BlinkTimer > -6) && !IsNVGBlinking && bbEntityInView(n->obj, Camera)) move = 0;
		}
		if (NoTarget) move = 1;
		if (!move) { // being looked at: freeze
			BlurVolume = Max(Max(Min((4 - dist) / 6, .9f), .1f), BlurVolume);
			CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 15 * Max((3.5f - dist) / 3.5f, 0));
			if (dist < 3.5f && MilliSecs2() - n->LastSeen > 60000 && temp) { PlaySound_Strict(HorrorSFX[bbRand(3, 4)]); n->LastSeen = MilliSecs2(); }
			if ((dist < 1.5f) & (bbRand(700, 1) == 1)) PlaySound2(Scp173SFX[bbRand(0, 2)], Camera, n->obj, 10, 1); // blitz And: Rand rolls every time
			if (dist < 1.5f && n->LastDist > 2 && temp) {
				CurrCameraZoom = 40;
				HeartBeatRate = Max(HeartBeatRate, 140);
				HeartBeatVolume = .5f;
				static const int h[] = {1, 2, 9, 10, 14};
				PlaySound_Strict(HorrorSFX[h[bbRand(5, 1) - 1]]);
			}
			n->LastDist = dist;
			n->State = Max(0, n->State - FPSfactor / 20);
		} else if (dist > 50) { // 6+ rooms away: teleport somewhere 15-25 units from the player
			if (bbRand(70, 1) == 1 && !is(RN, "exit1") && !is(RN, "gatea") && !is(RN, "pocketdimension")) {
				each(WayPoints, w) {
					if (!((w->door == NULL) & (bbRand(5, 1) == 1))) continue; // Rand rolls for door waypoints too
					float x = fabsf(bbEntityX(Collider, 0) - bbEntityX(w->obj, 1));
					if (!(x < 25 && x > 15)) continue;
					float z = fabsf(bbEntityZ(Collider, 0) - bbEntityZ(w->obj, 1));
					if (!(z < 25 && z > 15)) continue;
					bbPositionEntity(n->Collider, bbEntityX(w->obj, 1), bbEntityY(w->obj, 1) + .25f, bbEntityZ(w->obj, 1), 0);
					bbResetEntity(n->Collider);
					break;
				}
			}
		} else if (dist > HideDistance * .8f) { // 3-6 rooms away: hop between waypoints
			if (bbRand(70, 1) == 1) TeleportCloser(n);
		} else { // close: hunt
			n->State = CurveValue(SoundVol, n->State, 3);
			if (bbRand(20, 1) == 1) opendoors(n);
			if (NoTarget) { temp = 0; n->EnemyX = n->EnemyY = n->EnemyZ = 0; }
			if (temp) { // not looked at + sees the player
				if (dist < .65f) { if (KillTimer >= 0 && !GodMode) snapneck(n); }
				else {
					bbPointEntity(n->Collider, Collider, 0);
					bbRotateEntity(n->Collider, 0, bbEntityYaw(n->Collider, 0), bbEntityRoll(n->Collider, 0), 0);
					fwd(n);
				}
			} else if (n->EnemyX != 0) { // go where the player was last seen
				if (Distance(bbEntityX(n->Collider, 0), bbEntityZ(n->Collider, 0), n->EnemyX, n->EnemyZ) > .5f) {
					bbAlignToVector(n->Collider, n->EnemyX - bbEntityX(n->Collider, 0), 0, n->EnemyZ - bbEntityZ(n->Collider, 0), 3, 1);
					bbMoveEntity(n->Collider, 0, 0, n->Speed * FPSfactor);
					if (bbRand(500, 1) == 1) n->EnemyX = n->EnemyY = n->EnemyZ = 0;
				} else n->EnemyX = n->EnemyY = n->EnemyZ = 0;
			} else {
				if (bbRand(400, 1) == 1) bbRotateEntity(n->Collider, 0, bbRnd(360, 0), 10, 0);
				fwd(n);
			}
		}
	}
	bbPositionEntity(n->Collider, bbEntityX(n->Collider, 0), Min(bbEntityY(n->Collider, 0), .35f), bbEntityZ(n->Collider, 0), 0);
}

// ---- 106

static void step106(NPCs *n, float prev, float cur) {
	if ((prev <= 286 && cur > 286) || (prev <= 311 && cur > 311)) {
		int s = Step2SFX[bbRand(0, 2)]; // Rand then Rnd: blitz codegen order, the args tie
		PlaySound2(s, Camera, n->Collider, 6, bbRnd(.8f, 1));
	}
}

static void walk106(NPCs *n, float dist, int visible) {
	if (dist > .8f) {
		if ((dist > 25 || is(RN, "pocketdimension") || visible || n->PathStatus != 1) && !is(RN, "gatea")) {
			if (dist > 40 || is(RN, "pocketdimension")) bbTranslateEntity(n->Collider, 0, (bbEntityY(Collider, 0) - .14f - bbEntityY(n->Collider, 0)) / 50, 0, 0);
			n->CurrSpeed = CurveValue(n->Speed, n->CurrSpeed, 10);
			bbPointEntity(n->obj, Collider, 0);
			bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 10), 0, 0);
			if (KillTimer >= 0) {
				float pf = n->Frame;
				AnimateNPC(n, 284, 333, n->CurrSpeed * 43, 1);
				step106(n, pf, n->Frame);
			} else n->CurrSpeed = 0;
			n->PathTimer = Max(n->PathTimer - FPSfactor, 0);
			if (n->PathTimer <= 0) { n->PathStatus = FindPath(n, bbEntityX(Collider, 1), bbEntityY(Collider, 1), bbEntityZ(Collider, 1)); n->PathTimer = 70 * 10; }
		} else if (n->PathTimer <= 0) {
			n->PathStatus = FindPath(n, bbEntityX(Collider, 1), bbEntityY(Collider, 1), bbEntityZ(Collider, 1));
			n->PathTimer = 70 * 10;
			n->CurrSpeed = 0;
		} else {
			n->PathTimer = Max(n->PathTimer - FPSfactor, 0);
			if (n->PathStatus == 2) n->CurrSpeed = 0;
			else if (n->PathStatus == 1) {
				while (!n->Path[n->PathLocation]) {
					if (n->PathLocation > 19) { n->PathLocation = 0; n->PathStatus = 0; break; }
					n->PathLocation++;
				}
				WayPoints *w = n->Path[n->PathLocation];
				if (w) {
					bbTranslateEntity(n->Collider, 0, (bbEntityY(w->obj, 1) - .11f - bbEntityY(n->Collider, 0)) / 50, 0, 0);
					bbPointEntity(n->obj, w->obj, 0);
					float d2 = bbEntityDistance(n->Collider, w->obj);
					bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), Min(20, d2 * 10)), 0, 0);
					n->CurrSpeed = CurveValue(n->Speed, n->CurrSpeed, 10);
					float pf = bbAnimTime(n->obj);
					AnimateNPC(n, 284, 333, n->CurrSpeed * 43, 1);
					step106(n, pf, n->Frame);
					if (d2 < .2f) n->PathLocation++;
				}
			} else if (n->PathStatus == 0) {
				if (n->State3 == 0) AnimateNPC(n, 334, 494, .3f, 1);
				n->CurrSpeed = CurveValue(0, n->CurrSpeed, 10);
			}
		}
	} else if (!is(RN, "gatea") && !NoTarget) { // in reach: grab
		n->CurrSpeed = dist > .5f ? CurveValue(n->Speed * 2.5f, n->CurrSpeed, 10) : 0;
		AnimateNPC(n, 105, 110, .15f, 0);
		if (KillTimer >= 0 && FallTimer >= 0) {
			bbPointEntity(n->obj, Collider, 0);
			bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 10), 0, 0);
			if (ceilf(n->Frame) == 110 && !GodMode) {
				PlaySound_Strict(DamageSFX[1]);
				PlaySound_Strict(HorrorSFX[5]);
				if (is(RN, "pocketdimension")) {
					DeathMSG = "Subject D-9341. Body partially decomposed by what is assumed to be SCP-106's \"corrosion\" effect. Body disposed of via incineration.";
					Kill();
				} else {
					PlaySound_Strict(OldManSFX[3]);
					FallTimer = Min(-1, FallTimer);
					bbPositionEntity(Head, bbEntityX(Camera, 1), bbEntityY(Camera, 1), bbEntityZ(Camera, 1), 1);
					bbResetEntity(Head);
					bbRotateEntity(Head, 0, bbEntityYaw(Camera, 0) + bbRand(-45, 45), 0, 0);
				}
			}
		}
	}
}

static void attack106(NPCs *n, float dist) {
	if (bbEntityY(n->Collider, 0) < bbEntityY(Collider, 0) - 20 - .55f) { // rise out of the floor under the player
		if (!PlayerRoom->RoomTemplate->DisableDecals) {
			Decals *de = CreateDecal(0, bbEntityX(Collider, 0), .01f, bbEntityZ(Collider, 0), 90, bbRand(360, 1), 0);
			if (de) { de->Size = .05f; de->SizeChange = .001f; bbEntityAlpha(de->obj, .8f); }
			UpdateDecals();
		}
		n->PrevY = bbEntityY(Collider, 0);
		bbSetAnimTime(n->obj, 110, 0);
		if (!is(RN, "coffin")) bbPositionEntity(n->Collider, bbEntityX(Collider, 0), bbEntityY(Collider, 0) - 15, bbEntityZ(Collider, 0), 0);
		PlaySound_Strict(DecaySFX[0]);
	}
	if (bbRand(500, 1) == 1) PlaySound2(OldManSFX[bbRand(0, 2)], Camera, n->Collider, 10, 1);
	n->SoundChn = LoopSound2(OldManSFX[4], n->SoundChn, Camera, n->Collider, 8, .8f);
	if (n->State > -10) {
		ShouldPlay = 66;
		if (n->Frame < 259) {
			bbPositionEntity(n->Collider, bbEntityX(n->Collider, 0), n->PrevY - .15f, bbEntityZ(n->Collider, 0), 0);
			bbPointEntity(n->obj, Collider, 0);
			bbRotateEntity(n->Collider, 0, CurveValue(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 100), 0, 1);
			AnimateNPC(n, 110, 259, .15f, 0);
		} else n->State = -10;
	} else {
		if (!is(RN, "gatea")) ShouldPlay = 10;
		int visible = dist < 8 ? bbEntityVisible(n->Collider, Collider) : 0;
		if (NoTarget) visible = 0;
		if (visible) {
			if (!is(RN, "gatea")) n->PathTimer = 0;
			if (bbEntityInView(n->Collider, Camera)) {
				GiveAchievement(Achv106, 1);
				BlurVolume = Max(Max(Min((4 - dist) / 6, .9f), .1f), BlurVolume);
				CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 20 * Max((4 - dist) / 4, 0));
				if (MilliSecs2() - n->LastSeen > 60000) { CurrCameraZoom = 40; PlaySound_Strict(HorrorSFX[6]); n->LastSeen = MilliSecs2(); }
			}
		} else n->State -= FPSfactor;
		walk106(n, dist, visible);
	}
	bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	if (n->State <= bbRand(-3500, -3000) && !(bbEntityInView(n->obj, Camera) & (dist > 5))) { // blitz: Not (InView And dist > 5)
		n->State = bbRand(22000, 27000);
		bbPositionEntity(n->Collider, 0, 500, 0, 0);
	}
	if (FallTimer < -250) { MoveToPocketDimension(); n->State = 250; }
	if (n->Reload == 0 && dist > 10 && !is(RN, "pocketdimension") && !is(RN, "gatea") && n->State < -5 && !bbEntityInView(n->obj, Camera)) {
		// pop up behind the player
		bbTurnEntity(Collider, 0, 180, 0, 0);
		int pick = bbEntityPick(Collider, 5);
		bbTurnEntity(Collider, 0, 180, 0, 0);
		if (pick) {
			TeleportEntity(n->Collider, bbPickedX(), bbPickedY(), bbPickedZ(), n->CollRadius, 0, 2, 0);
			bbPointEntity(n->Collider, Collider, 0);
			bbRotateEntity(n->Collider, 0, bbEntityYaw(n->Collider, 0), 0, 0);
			bbMoveEntity(n->Collider, 0, 0, -2);
			PlaySound2(OldManSFX[3], Camera, n->Collider, 10, 1);
			n->SoundChn2 = PlaySound2(OldManSFX[6 + bbRand(0, 2)], Camera, n->Collider, 10, 1);
			n->PathTimer = 0;
			n->Reload = 70 * 10.0f / (SelectedDifficulty->otherFactors + 1);
		}
	}
	n->Reload = Max(0, n->Reload - FPSfactor);
	UpdateSoundOrigin(n->SoundChn2, Camera, n->Collider, 10, 1);
}

void Update106(NPCs *n) {
	if (Contained106) {
		n->Idle = 1;
		bbHideEntity(n->obj); bbHideEntity(n->obj2);
		bbPositionEntity(n->obj, 0, 500, 0, 1);
		return;
	}
	float dist = bbEntityDistance(n->Collider, Collider);
	int spawn = 1;
	if (is(RN, "dimension1499")) spawn = 0;
	Events *e = FindEvent("room860");
	if (e && e->EventState == 1) spawn = 0;
	if (is(RN, "room049") && bbEntityY(Collider, 0) <= -2848 * RoomScale) spawn = 0;
	e = FindEvent("gatea"); // gate a's event keeps 106 around, and wins over room860
	if (e && e->EventState != 0) { spawn = 1; n->Idle = is(RN, "dimension1499"); }
	if (!spawn && n->State <= 0) { n->State = bbRand(22000, 27000); bbPositionEntity(n->Collider, 0, 500, 0, 0); }

	if (n->Idle != 0 || !spawn) { bbHideEntity(n->obj2); return; }
	if (n->State <= 0) attack106(n, dist);
	else { // idling under the map
		n->CurrSpeed = 0;
		bbMoveEntity(n->Collider, 0, (bbEntityY(Collider, 0) - 30 - bbEntityY(n->Collider, 0)) / 200, 0);
		n->DropSpeed = 0;
		n->Frame = 110;
		if (!PlayerRoom->RoomTemplate->DisableDecals && !is(RN, "gatea")) n->State -= SelectedDifficulty->aggressiveNPCs ? FPSfactor * 2 : FPSfactor;
	}
	bbResetEntity(n->Collider);
	n->DropSpeed = 0;
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .15f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0), 0, 0);
	bbPositionEntity(n->obj2, bbEntityX(n->obj, 0), bbEntityY(n->obj, 0), bbEntityZ(n->obj, 0), 0);
	bbRotateEntity(n->obj2, 0, bbEntityYaw(n->Collider, 0) - 180, 0, 0);
	bbMoveEntity(n->obj2, 0, 8.6f * .11f, -1.5f * .11f);
	float fd = CameraFogFar * LightVolume * .6f;
	if (is(RN, "pocketdimension") || is(RN, "gatea") || dist < fd) bbHideEntity(n->obj2);
	else { bbShowEntity(n->obj2); bbEntityAlpha(n->obj2, Min(dist - fd, 1)); }
}
