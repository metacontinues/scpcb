// npcguard.c: UpdateNPCs' guard + class d/clerk cases, Shoot, PlayMTFSound, ChangeNPCTextureID (NPCs.bb)
#include "g.h"

#define RN (PlayerRoom->RoomTemplate->Name)
static int st(float s) { return s == (int)s ? (int)s : -999; } // blitz Select on a float: no Case matches 1.5

static void gstep(NPCs *n) { int s = StepSFX[2][0][bbRand(0, 2)]; PlaySound2(s, Camera, n->Collider, 8, bbRnd(.5f, .7f)); }

static void aimframes(NPCs *n) { // raise the gun if it's in some other anim
	float f = n->Frame;
	if (f < 39 || (f > 76 && f < 245) || (f > 248 && f < 302) || f > 344) {
		AnimateNPC(n, 345, 357, .2f, 0);
		if (n->Frame >= 356) SetNPCFrame(n, 302);
	}
}

// turn to the player, shoot when the line's clear. returns 1 if it had a shot to take
static void aimshoot(NPCs *n, float dist, float acc, int insta, const char *msg) {
	int pvt = bbCreatePivot(0);
	bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0), bbEntityZ(n->Collider, 0), 0);
	bbPointEntity(pvt, Collider, 0);
	bbRotateEntity(pvt, Min(bbEntityPitch(pvt, 0), 20), bbEntityYaw(pvt, 0), 0, 0);
	bbRotateEntity(n->Collider, CurveAngle(bbEntityPitch(pvt, 0), bbEntityPitch(n->Collider, 0), 10), CurveAngle(bbEntityYaw(pvt, 0), bbEntityYaw(n->Collider, 0), 10), 0, 1);
	bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) + .8f, bbEntityZ(n->Collider, 0), 0);
	bbPointEntity(pvt, Collider, 0);
	bbRotateEntity(pvt, Min(bbEntityPitch(pvt, 0), 40), bbEntityYaw(n->Collider, 0), 0, 0);
	if (n->Reload == 0) {
		bbEntityPick(pvt, dist);
		if (bbPickedEntity() == Collider || n->State3 == 1) {
			if (msg) DeathMSG = (str)msg;
			PlaySound2(GunshotSFX, Camera, n->Collider, 35, 1);
			bbRotateEntity(pvt, bbEntityPitch(n->Collider, 0), bbEntityYaw(n->Collider, 0), 0, 1);
			bbPositionEntity(pvt, bbEntityX(n->obj, 0), bbEntityY(n->obj, 0), bbEntityZ(n->obj, 0), 0);
			bbMoveEntity(pvt, .8f * .079f, 10.75f * .079f, 6.9f * .079f);
			bbPointEntity(pvt, Collider, 0);
			Shoot(bbEntityX(pvt, 0), bbEntityY(pvt, 0), bbEntityZ(pvt, 0), acc, 0, insta);
			n->Reload = 7;
		} else n->CurrSpeed = n->Speed;
	}
	if (n->Reload > 0 && n->Reload <= 7) AnimateNPC(n, 245, 248, .35f, 1);
	else if (n->Frame < 302) AnimateNPC(n, 302, 344, .35f, 1);
	bbFreeEntity(pvt);
}

// walk n->Path, 0 when there's no path. returns 0 if PathStatus says stop
static void followpath(NPCs *n, int a0, int a1, int movefirst) {
	if (!n->Path[n->PathLocation]) {
		if (n->PathLocation > 19) { n->PathLocation = 0; n->PathStatus = 0; }
		else n->PathLocation++;
		return;
	}
	WayPoints *w = n->Path[n->PathLocation];
	if (movefirst) {
		AnimateNPC(n, a0, a1, n->CurrSpeed * 40, 1);
		n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
		bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		bbPointEntity(n->obj, w->obj, 0);
		bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 20), 0, 0);
	} else {
		bbPointEntity(n->obj, w->obj, 0);
		bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 20), 0, 0);
		AnimateNPC(n, a0, a1, n->CurrSpeed * 40, 1);
		n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
		bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	}
	if (bbEntityDistance(n->Collider, w->obj) < .2f) n->PathLocation++;
}

static void walksteps(NPCs *n, float pf) {
	if (pf < 43 && n->Frame >= 43) gstep(n);
	else if (pf < 61 && n->Frame >= 61) gstep(n);
}

void UpdateGuard(NPCs *n) {
	float pf = n->Frame, dist;
	n->BoneToManipulate = ""; n->ManipulateBone = 0; n->ManipulationType = 0; n->NPCNameInSection = "Guard";
	switch (st(n->State)) {
	case 1: // aims and shoots at the player
		aimframes(n);
		if (KillTimer < 0) { n->State = 0; break; }
		{
			dist = bbEntityDistance(n->Collider, Collider);
			float acc = .4f + .5f * SelectedDifficulty->aggressiveNPCs, detect = 11;
			if (bbSeq(RN, "exit1")) { // gate b: shoots from further, worse aim unless you stand still
				detect = 21; acc = 0;
				if (bbRand(1, 8 - SelectedDifficulty->aggressiveNPCs * 4) < 2) acc = .03f;
				acc += .5f - CurrSpeed * 20;
			}
			if (dist < detect) {
				int insta = 0; const char *msg = "";
				if (bbSeq(RN, "start")) {
					msg = "Subject D-9341. Cause of death: Gunshot wound to the head. The surveillance tapes confirm that the subject was terminated by Agent Ulgrin shortly after the site lockdown was initiated.";
					insta = 1;
				} else if (bbSeq(RN, "exit1")) msg = "\"Agent G. to control. Eliminated a Class D escapee in Gate B's courtyard.\"";
				aimshoot(n, dist, acc, insta, msg);
			} else AnimateNPC(n, 302, 344, .35f, 1);
			n->ManipulateBone = 1;
			n->BoneToManipulate = "Chest";
			n->ManipulationType = n->State2 == 10 ? 3 : 0;
		}
		break;
	case 2: // shoots at nothing
		AnimateNPC(n, 245, 248, .35f, 1);
		if (n->Reload == 0) {
			PlaySound2(GunshotSFX, Camera, n->Collider, 20, 1);
			Particles *p = CreateParticle(bbEntityX(n->obj, 1), bbEntityY(n->obj, 1), bbEntityZ(n->obj, 1), 1, .2f, 0, 5);
			bbPositionEntity(p->pvt, bbEntityX(n->obj, 0), bbEntityY(n->obj, 0), bbEntityZ(n->obj, 0), 0);
			bbRotateEntity(p->pvt, bbEntityPitch(n->Collider, 0), bbEntityYaw(n->Collider, 0), 0, 1);
			bbMoveEntity(p->pvt, .8f * .079f, 10.75f * .079f, 6.9f * .079f);
			n->Reload = 7;
		}
		break;
	case 3: // follows a path
		if (n->PathStatus == 2) { n->State = 0; n->CurrSpeed = 0; }
		else if (n->PathStatus == 1) followpath(n, 1, 38, 0);
		else { n->CurrSpeed = 0; n->State = 4; }
		break;
	case 4:
		AnimateNPC(n, 77, 201, .2f, 1);
		if (bbRand(400, 1) == 1) n->Angle = bbRnd(-180, 180);
		bbRotateEntity(n->Collider, 0, CurveAngle(n->Angle + sinf(MilliSecs2() / 50 * DTOR) * 2, bbEntityYaw(n->Collider, 0), 150), 0, 1);
		dist = bbEntityDistance(n->Collider, Collider);
		if (dist < 15 && WrapAngle(bbEntityYaw(n->Collider, 0) - bbDeltaYaw(n->Collider, Collider)) < 90 && bbEntityVisible(n->Collider, Collider)) n->State = 1; // spots you
		break;
	case 5: { // following a target
		bbRotateEntity(n->Collider, 0, CurveAngle(bbVectorYaw(n->EnemyX - bbEntityX(n->Collider, 0), 0, n->EnemyZ - bbEntityZ(n->Collider, 0)) + n->Angle, bbEntityYaw(n->Collider, 0), 20), 0, 0);
		dist = Distance(bbEntityX(n->Collider, 0), bbEntityZ(n->Collider, 0), n->EnemyX, n->EnemyZ);
		AnimateNPC(n, 1, 38, n->CurrSpeed * 40, 1);
		float sg = dist - 1.5f > 0 ? 1 : dist - 1.5f < 0 ? -1 : 0;
		n->CurrSpeed = dist > 2 || dist < 1 ? CurveValue(n->Speed * sg * .75f, n->CurrSpeed, 10) : CurveValue(0, n->CurrSpeed, 10);
		bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		break;
	}
	case 7: AnimateNPC(n, 77, 201, .2f, 1); break;
	case 8: break;
	case 9:
		AnimateNPC(n, 77, 201, .2f, 1);
		n->BoneToManipulate = "head"; n->ManipulateBone = 1; n->ManipulationType = 0;
		n->Angle = bbEntityYaw(n->Collider, 0);
		break;
	case 10:
		AnimateNPC(n, 1, 38, n->CurrSpeed * 40, 1);
		n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
		bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		break;
	case 11: // hunts the player down the waypoints
		aimframes(n);
		if (KillTimer < 0) { n->State = 0; break; }
		dist = bbEntityDistance(n->Collider, Collider);
		if (dist < 11 && bbEntityVisible(n->Collider, Collider)) { aimshoot(n, dist, 1, 0, ""); break; }
		if (n->PathStatus == 1) followpath(n, 39, 76, 1);
		else {
			if (n->PathTimer == 0) n->PathStatus = FindPath(n, bbEntityX(Collider, 0), bbEntityY(Collider, 0) + .5f, bbEntityZ(Collider, 0));
			WayPoints *wc = NULL;
			each(WayPoints, wp) if (bbEntityDistance(wp->obj, Collider) < 2) { wc = wp; break; }
			if (wc) {
				n->PathTimer = 1;
				if (bbEntityVisible(wc->obj, n->Collider) && fabsf(bbDeltaYaw(n->Collider, wc->obj)) > 0) {
					bbPointEntity(n->obj, wc->obj, 0);
					bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 20), 0, 0);
				}
			} else n->PathTimer = 0;
			if (n->PathTimer == 1) {
				AnimateNPC(n, 39, 76, n->CurrSpeed * 40, 1);
				n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
				bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
			}
		}
		walksteps(n, pf);
		break;
	case 12: { // stands aiming (at the player if State2 = 1, else along Angle)
		aimframes(n);
		if (n->Frame < 345) AnimateNPC(n, 302, 344, .35f, 1);
		int pvt = bbCreatePivot(0);
		bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0), bbEntityZ(n->Collider, 0), 0);
		if (n->State2 == 1) bbPointEntity(pvt, Collider, 0); else bbRotateEntity(pvt, 0, n->Angle, 0, 0);
		bbRotateEntity(pvt, Min(bbEntityPitch(pvt, 0), 20), bbEntityYaw(pvt, 0), 0, 0);
		bbRotateEntity(n->Collider, CurveAngle(bbEntityPitch(pvt, 0), bbEntityPitch(n->Collider, 0), 10), CurveAngle(bbEntityYaw(pvt, 0), bbEntityYaw(n->Collider, 0), 10), 0, 1);
		bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) + .8f, bbEntityZ(n->Collider, 0), 0);
		if (n->State2 == 1) { bbPointEntity(pvt, Collider, 0); n->ManipulateBone = 1; n->BoneToManipulate = "Chest"; n->ManipulationType = 0; }
		else bbRotateEntity(pvt, 0, n->Angle, 0, 0);
		bbRotateEntity(pvt, Min(bbEntityPitch(pvt, 0), 40), bbEntityYaw(n->Collider, 0), 0, 0);
		bbFreeEntity(pvt);
		UpdateSoundOrigin(n->SoundChn, Camera, n->Collider, 20, 1);
		break;
	}
	case 13: AnimateNPC(n, 202, 244, .35f, 1); break;
	case 14:
		if (n->PathStatus == 2) { n->State = 13; n->CurrSpeed = 0; }
		else if (n->PathStatus == 1) followpath(n, 39, 76, 0);
		else { n->CurrSpeed = 0; n->State = 13; }
		walksteps(n, pf);
		break;
	default:
		if (bbRand(400, 1) == 1) n->PrevState = bbInt(bbRnd(-30, 30)); // PrevState is an int field
		n->PathStatus = 0;
		AnimateNPC(n, 77, 201, .2f, 1);
		bbRotateEntity(n->Collider, 0, CurveAngle(n->Angle + n->PrevState + sinf(MilliSecs2() / 50 * DTOR) * 2, bbEntityYaw(n->Collider, 0), 50), 0, 1);
	}
	if (n->CurrSpeed > .01f) {
		if (pf < 5 && n->Frame >= 5) gstep(n);
		else if (pf < 23 && n->Frame >= 23) gstep(n);
	}
	if (n->Frame > 286.5f && n->Frame < 288.5f) n->IsDead = 1;
	n->Reload = Max(0, n->Reload - FPSfactor);
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .2f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0) + 180, 0, 0);
}

static void dstep(NPCs *n, int run) {
	int g = GetStepSound(n->Collider), s = StepSFX[g][run][bbRand(0, 2)];
	PlaySound2(s, Camera, n->Collider, 8, bbRnd(.3f, .5f));
}

void UpdateClassD(NPCs *n) { // NPCtypeD + NPCtypeClerk
	bbRotateEntity(n->Collider, 0, bbEntityYaw(n->Collider, 0), bbEntityRoll(n->Collider, 0), 1);
	float pf = bbAnimTime(n->obj);
	switch (st(n->State)) {
	case 0: n->CurrSpeed = CurveValue(0, n->CurrSpeed, 5); Animate2(n->obj, bbAnimTime(n->obj), 210, 235, .1f, 1); break;
	case 1:
		n->CurrSpeed = n->State2 == 1 ? CurveValue(n->Speed * .7f, n->CurrSpeed, 20) : CurveValue(.015f, n->CurrSpeed, 5);
		Animate2(n->obj, bbAnimTime(n->obj), 236, 260, n->CurrSpeed * 18, 1);
		break;
	case 2: n->CurrSpeed = CurveValue(.03f, n->CurrSpeed, 5); Animate2(n->obj, bbAnimTime(n->obj), 301, 319, n->CurrSpeed * 18, 1); break;
	}
	if (n->State2 != 2 && n->CurrSpeed > .01f) {
		float at = bbAnimTime(n->obj);
		if (n->State == 1) {
			if (pf < 244 && at >= 244) dstep(n, 0);
			else if (pf < 256 && at >= 256) dstep(n, 0);
		} else if (n->State == 2) {
			if (pf < 309 && at >= 309) dstep(n, 1);
			else if (pf <= 319 && at <= 301) dstep(n, 1);
		}
	}
	if (n->Frame == 19 || n->Frame == 60) n->IsDead = 1;
	if (bbAnimTime(n->obj) == 19 || bbAnimTime(n->obj) == 60) n->IsDead = 1;
	bbMoveEntity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .32f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, bbEntityPitch(n->Collider, 0), bbEntityYaw(n->Collider, 0) - 180, 0, 0);
}

static void hit(const char **m, const char *txt, float lo, float hi) { BlurTimer = 500; *m = txt; Injuries += bbRnd(lo, hi); }

void Shoot(float x, float y, float z, float hitProb, int particles, int instaKill) {
	Particles *p = CreateParticle(x, y, z, 1, bbRnd(.08f, .1f), 0, 5); // muzzle flash
	bbTurnEntity(p->obj, 0, 0, bbRnd(360, 0), 0);
	p->Achange = -.15f;
	LightVolume = TempLightVolume * 1.2f;
	if (GodMode) return;
	if (instaKill) { Kill(); PlaySound_Strict(BullethitSFX); return; }
	if (bbRnd(1, 0) <= hitProb) {
		float ry = bbRnd(-3, 3), rp = bbRnd(-3, 3); // yaw's Rnd first: TurnEntity's defaulted global arg makes the tail needier
		bbTurnEntity(Camera, rp, ry, 0, 0);
		const char *m = "";
		if (WearingVest > 0) {
			if (WearingVest == 1) {
				int r = bbRand(8, 1);
				if (r <= 5) { Stamina = 0; hit(&m, "A bullet penetrated your vest, making you gasp.", .1f, .5f); }
				else if (r == 6) hit(&m, "A bullet hit your left leg.", .8f, 1.2f);
				else if (r == 7) hit(&m, "A bullet hit your right leg.", .8f, 1.2f);
				else { Stamina = 0; hit(&m, "A bullet struck your neck, making you gasp.", 1.2f, 1.6f); }
			} else if (bbRand(10, 1) == 1) { Stamina -= 1; hit(&m, "A bullet hit your chest. The vest absorbed some of the damage.", .8f, 1.1f); }
			else { m = "A bullet hit your chest. The vest absorbed most of the damage."; Injuries += bbRnd(.1f, .5f); }
			if (Injuries >= 5 && bbRand(3, 1) == 1) Kill();
		} else {
			switch (bbRand(6, 1)) {
			case 1: Kill(); break;
			case 2: hit(&m, "A bullet hit your left leg.", .8f, 1.2f); break;
			case 3: hit(&m, "A bullet hit your right leg.", .8f, 1.2f); break;
			case 4: hit(&m, "A bullet hit your right shoulder.", .8f, 1.2f); break;
			case 5: hit(&m, "A bullet hit your left shoulder.", .8f, 1.2f); break;
			case 6: hit(&m, "A bullet hit your right shoulder.", 2.5f, 4); break;
			}
		}
		if (MsgTimer < 64 * 4) { Msg = (str)m; MsgTimer = 70 * 6; } // only if the last one's 2s old
		Injuries = Min(Injuries, 4);
		PlaySound_Strict(BullethitSFX);
	} else if (particles && ParticleAmount > 0) { // missed: dust + a bullet hole behind you
		int pvt = bbCreatePivot(0);
		bbPositionEntity(pvt, bbEntityX(Collider, 0), (bbEntityY(Collider, 0) + bbEntityY(Camera, 0)) / 2, bbEntityZ(Collider, 0), 0);
		bbPointEntity(pvt, p->obj, 0);
		bbTurnEntity(pvt, 0, 180, 0, 0);
		bbEntityPick(pvt, 2.5f);
		if (bbPickedEntity()) {
			PlaySound2(Gunshot3SFX, Camera, pvt, .4f, bbRnd(.8f, 1));
			p = CreateParticle(bbPickedX(), bbPickedY(), bbPickedZ(), 0, .03f, 0, 80);
			p->speed = .001f; p->SizeChange = .003f; p->A = .8f; p->Achange = -.01f;
			bbRotateEntity(p->pvt, bbEntityPitch(pvt, 0) - 180, bbEntityYaw(pvt, 0), 0, 0);
			for (int i = 0, k = bbRand(2, 3); i <= k; i++) {
				p = CreateParticle(bbPickedX(), bbPickedY(), bbPickedZ(), 0, .006f, .003f, 80);
				p->speed = .02f; p->A = .8f; p->Achange = -.01f;
				float rp = bbRnd(170, 190), ry = bbRnd(-10, 10);
				bbRotateEntity(p->pvt, bbEntityPitch(pvt, 0) + rp, bbEntityYaw(pvt, 0) + ry, 0, 0);
			}
			Decals *de = CreateDecal(bbRand(13, 14), bbPickedX(), bbPickedY(), bbPickedZ(), 0, 0, 0);
			if (de) {
				bbAlignToVector(de->obj, -bbPickedNX(), -bbPickedNY(), -bbPickedNZ(), 3, 1);
				bbMoveEntity(de->obj, 0, 0, -.001f);
				bbEntityFX(de->obj, 1);
				de->lifetime = 70 * 20;
				bbEntityBlend(de->obj, 2);
				de->Size = bbRnd(.028f, .034f);
				bbScaleSprite(de->obj, de->Size, de->Size);
			}
		}
		bbFreeEntity(pvt);
	}
}

void PlayMTFSound(int sound, NPCs *n) {
	if (n) n->SoundChn = PlaySound2(sound, Camera, n->Collider, 8, 1);
	if (!SelectedItem || !(SelectedItem->state2 == 3 && SelectedItem->state > 0)) return;
	str t = SelectedItem->itemtemplate->tempname;
	if (!(bbSeq(t, "radio") || bbSeq(t, "fineradio") || bbSeq(t, "18vradio"))) return;
	if (sound != MTFSFX[5] || !bbChannelPlaying(RadioCHN[3])) {
		if (RadioCHN[3]) bbStopChannel(RadioCHN[3]);
		RadioCHN[3] = PlaySound_Strict(sound);
	}
}

void ChangeNPCTextureID(NPCs *n, int id) {
	if (!n) {
		CreateConsoleMsg("Tried to change the texture of an invalid NPC", 255, 255, 255, 0);
		if (ConsoleOpening) ConsoleOpen = 1;
		return;
	}
	n->TextureID = id + 1;
	bbFreeEntity(n->obj);
	n->obj = bbCopyEntity(DTextures[id + 1], 0);
	float t = .5f / bbMeshWidth(n->obj);
	bbScaleEntity(n->obj, t, t, t, 0);
	bbSetAnimTime(n->obj, n->Frame, 0); // pose the new copy like the old one
}
