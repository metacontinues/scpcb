// npcguard.c: UpdateNPCs' guard + class d/clerk cases, Shoot, PlayMTFSound, ChangeNPCTextureID (NPCs.bb)
#include "g.h"

#define RN (PlayerRoom->RoomTemplate->Name)
static int st(float s) { return s == (int)s ? (int)s : -999; } // blitz Select on a float: no Case matches 1.5

static void gstep(NPCs *n) { int s = StepSFX[2][0][irand(0, 2)]; PlaySound2(s, Camera, n->Collider, 8, frnd(.5f, .7f)); }

static void aimframes(NPCs *n) { // raise the gun if it's in some other anim
	float f = n->Frame;
	if (f < 39 || (f > 76 && f < 245) || (f > 248 && f < 302) || f > 344) {
		AnimateNPC(n, 345, 357, .2f, 0);
		if (n->Frame >= 356) SetNPCFrame(n, 302);
	}
}

// turn to the player, shoot when the line's clear. returns 1 if it had a shot to take
static void aimshoot(NPCs *n, float dist, float acc, int insta, const char *msg) {
	int pvt = create_pivot(0);
	position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0), entity_z(n->Collider, 0), 0);
	point_entity(pvt, Collider, 0);
	rotate_entity(pvt, Min(entity_pitch(pvt, 0), 20), entity_yaw(pvt, 0), 0, 0);
	rotate_entity(n->Collider, CurveAngle(entity_pitch(pvt, 0), entity_pitch(n->Collider, 0), 10), CurveAngle(entity_yaw(pvt, 0), entity_yaw(n->Collider, 0), 10), 0, 1);
	position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0) + .8f, entity_z(n->Collider, 0), 0);
	point_entity(pvt, Collider, 0);
	rotate_entity(pvt, Min(entity_pitch(pvt, 0), 40), entity_yaw(n->Collider, 0), 0, 0);
	if (n->Reload == 0) {
		entity_pick(pvt, dist);
		if (picked_entity() == Collider || n->State3 == 1) {
			if (msg) DeathMSG = (str)msg;
			PlaySound2(GunshotSFX, Camera, n->Collider, 35, 1);
			rotate_entity(pvt, entity_pitch(n->Collider, 0), entity_yaw(n->Collider, 0), 0, 1);
			position_entity(pvt, entity_x(n->obj, 0), entity_y(n->obj, 0), entity_z(n->obj, 0), 0);
			move_entity(pvt, .8f * .079f, 10.75f * .079f, 6.9f * .079f);
			point_entity(pvt, Collider, 0);
			Shoot(entity_x(pvt, 0), entity_y(pvt, 0), entity_z(pvt, 0), acc, 0, insta);
			n->Reload = 7;
		} else n->CurrSpeed = n->Speed;
	}
	if (n->Reload > 0 && n->Reload <= 7) AnimateNPC(n, 245, 248, .35f, 1);
	else if (n->Frame < 302) AnimateNPC(n, 302, 344, .35f, 1);
	free_entity(pvt);
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
		move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		point_entity(n->obj, w->obj, 0);
		rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 20), 0, 0);
	} else {
		point_entity(n->obj, w->obj, 0);
		rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 20), 0, 0);
		AnimateNPC(n, a0, a1, n->CurrSpeed * 40, 1);
		n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
		move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	}
	if (entity_distance(n->Collider, w->obj) < .2f) n->PathLocation++;
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
			dist = entity_distance(n->Collider, Collider);
			float acc = .4f + .5f * SelectedDifficulty->aggressiveNPCs, detect = 11;
			if (s_eq(RN, "exit1")) { // gate b: shoots from further, worse aim unless you stand still
				detect = 21; acc = 0;
				if (irand(1, 8 - SelectedDifficulty->aggressiveNPCs * 4) < 2) acc = .03f;
				acc += .5f - CurrSpeed * 20;
			}
			if (dist < detect) {
				int insta = 0; const char *msg = "";
				if (s_eq(RN, "start")) {
					msg = "Subject D-9341. Cause of death: Gunshot wound to the head. The surveillance tapes confirm that the subject was terminated by Agent Ulgrin shortly after the site lockdown was initiated.";
					insta = 1;
				} else if (s_eq(RN, "exit1")) msg = "\"Agent G. to control. Eliminated a Class D escapee in Gate B's courtyard.\"";
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
			Particles *p = CreateParticle(entity_x(n->obj, 1), entity_y(n->obj, 1), entity_z(n->obj, 1), 1, .2f, 0, 5);
			position_entity(p->pvt, entity_x(n->obj, 0), entity_y(n->obj, 0), entity_z(n->obj, 0), 0);
			rotate_entity(p->pvt, entity_pitch(n->Collider, 0), entity_yaw(n->Collider, 0), 0, 1);
			move_entity(p->pvt, .8f * .079f, 10.75f * .079f, 6.9f * .079f);
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
		if (irand(400, 1) == 1) n->Angle = frnd(-180, 180);
		rotate_entity(n->Collider, 0, CurveAngle(n->Angle + sinf(MilliSecs2() / 50 * DTOR) * 2, entity_yaw(n->Collider, 0), 150), 0, 1);
		dist = entity_distance(n->Collider, Collider);
		if (dist < 15 && WrapAngle(entity_yaw(n->Collider, 0) - delta_yaw(n->Collider, Collider)) < 90 && entity_visible(n->Collider, Collider)) n->State = 1; // spots you
		break;
	case 5: { // following a target
		rotate_entity(n->Collider, 0, CurveAngle(vector_yaw(n->EnemyX - entity_x(n->Collider, 0), 0, n->EnemyZ - entity_z(n->Collider, 0)) + n->Angle, entity_yaw(n->Collider, 0), 20), 0, 0);
		dist = Distance(entity_x(n->Collider, 0), entity_z(n->Collider, 0), n->EnemyX, n->EnemyZ);
		AnimateNPC(n, 1, 38, n->CurrSpeed * 40, 1);
		float sg = dist - 1.5f > 0 ? 1 : dist - 1.5f < 0 ? -1 : 0;
		n->CurrSpeed = dist > 2 || dist < 1 ? CurveValue(n->Speed * sg * .75f, n->CurrSpeed, 10) : CurveValue(0, n->CurrSpeed, 10);
		move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		break;
	}
	case 7: AnimateNPC(n, 77, 201, .2f, 1); break;
	case 8: break;
	case 9:
		AnimateNPC(n, 77, 201, .2f, 1);
		n->BoneToManipulate = "head"; n->ManipulateBone = 1; n->ManipulationType = 0;
		n->Angle = entity_yaw(n->Collider, 0);
		break;
	case 10:
		AnimateNPC(n, 1, 38, n->CurrSpeed * 40, 1);
		n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
		move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
		break;
	case 11: // hunts the player down the waypoints
		aimframes(n);
		if (KillTimer < 0) { n->State = 0; break; }
		dist = entity_distance(n->Collider, Collider);
		if (dist < 11 && entity_visible(n->Collider, Collider)) { aimshoot(n, dist, 1, 0, ""); break; }
		if (n->PathStatus == 1) followpath(n, 39, 76, 1);
		else {
			if (n->PathTimer == 0) n->PathStatus = FindPath(n, entity_x(Collider, 0), entity_y(Collider, 0) + .5f, entity_z(Collider, 0));
			WayPoints *wc = NULL;
			each(WayPoints, wp) if (entity_distance(wp->obj, Collider) < 2) { wc = wp; break; }
			if (wc) {
				n->PathTimer = 1;
				if (entity_visible(wc->obj, n->Collider) && fabsf(delta_yaw(n->Collider, wc->obj)) > 0) {
					point_entity(n->obj, wc->obj, 0);
					rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 20), 0, 0);
				}
			} else n->PathTimer = 0;
			if (n->PathTimer == 1) {
				AnimateNPC(n, 39, 76, n->CurrSpeed * 40, 1);
				n->CurrSpeed = CurveValue(n->Speed * .7f, n->CurrSpeed, 20);
				move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
			}
		}
		walksteps(n, pf);
		break;
	case 12: { // stands aiming (at the player if State2 = 1, else along Angle)
		aimframes(n);
		if (n->Frame < 345) AnimateNPC(n, 302, 344, .35f, 1);
		int pvt = create_pivot(0);
		position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0), entity_z(n->Collider, 0), 0);
		if (n->State2 == 1) point_entity(pvt, Collider, 0); else rotate_entity(pvt, 0, n->Angle, 0, 0);
		rotate_entity(pvt, Min(entity_pitch(pvt, 0), 20), entity_yaw(pvt, 0), 0, 0);
		rotate_entity(n->Collider, CurveAngle(entity_pitch(pvt, 0), entity_pitch(n->Collider, 0), 10), CurveAngle(entity_yaw(pvt, 0), entity_yaw(n->Collider, 0), 10), 0, 1);
		position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0) + .8f, entity_z(n->Collider, 0), 0);
		if (n->State2 == 1) { point_entity(pvt, Collider, 0); n->ManipulateBone = 1; n->BoneToManipulate = "Chest"; n->ManipulationType = 0; }
		else rotate_entity(pvt, 0, n->Angle, 0, 0);
		rotate_entity(pvt, Min(entity_pitch(pvt, 0), 40), entity_yaw(n->Collider, 0), 0, 0);
		free_entity(pvt);
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
		if (irand(400, 1) == 1) n->PrevState = iround(frnd(-30, 30)); // PrevState is an int field
		n->PathStatus = 0;
		AnimateNPC(n, 77, 201, .2f, 1);
		rotate_entity(n->Collider, 0, CurveAngle(n->Angle + n->PrevState + sinf(MilliSecs2() / 50 * DTOR) * 2, entity_yaw(n->Collider, 0), 50), 0, 1);
	}
	if (n->CurrSpeed > .01f) {
		if (pf < 5 && n->Frame >= 5) gstep(n);
		else if (pf < 23 && n->Frame >= 23) gstep(n);
	}
	if (n->Frame > 286.5f && n->Frame < 288.5f) n->IsDead = 1;
	n->Reload = Max(0, n->Reload - FPSfactor);
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .2f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0) + 180, 0, 0);
}

static void dstep(NPCs *n, int run) {
	int g = GetStepSound(n->Collider), s = StepSFX[g][run][irand(0, 2)];
	PlaySound2(s, Camera, n->Collider, 8, frnd(.3f, .5f));
}

void UpdateClassD(NPCs *n) { // NPCtypeD + NPCtypeClerk
	rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), entity_roll(n->Collider, 0), 1);
	float pf = anim_time(n->obj);
	switch (st(n->State)) {
	case 0: n->CurrSpeed = CurveValue(0, n->CurrSpeed, 5); Animate2(n->obj, anim_time(n->obj), 210, 235, .1f, 1); break;
	case 1:
		n->CurrSpeed = n->State2 == 1 ? CurveValue(n->Speed * .7f, n->CurrSpeed, 20) : CurveValue(.015f, n->CurrSpeed, 5);
		Animate2(n->obj, anim_time(n->obj), 236, 260, n->CurrSpeed * 18, 1);
		break;
	case 2: n->CurrSpeed = CurveValue(.03f, n->CurrSpeed, 5); Animate2(n->obj, anim_time(n->obj), 301, 319, n->CurrSpeed * 18, 1); break;
	}
	if (n->State2 != 2 && n->CurrSpeed > .01f) {
		float at = anim_time(n->obj);
		if (n->State == 1) {
			if (pf < 244 && at >= 244) dstep(n, 0);
			else if (pf < 256 && at >= 256) dstep(n, 0);
		} else if (n->State == 2) {
			if (pf < 309 && at >= 309) dstep(n, 1);
			else if (pf <= 319 && at <= 301) dstep(n, 1);
		}
	}
	if (n->Frame == 19 || n->Frame == 60) n->IsDead = 1;
	if (anim_time(n->obj) == 19 || anim_time(n->obj) == 60) n->IsDead = 1;
	move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .32f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, entity_pitch(n->Collider, 0), entity_yaw(n->Collider, 0) - 180, 0, 0);
}

static void hit(const char **m, const char *txt, float lo, float hi) { BlurTimer = 500; *m = txt; Injuries += frnd(lo, hi); }

void Shoot(float x, float y, float z, float hitProb, int particles, int instaKill) {
	Particles *p = CreateParticle(x, y, z, 1, frnd(.08f, .1f), 0, 5); // muzzle flash
	turn_entity(p->obj, 0, 0, frnd(360, 0), 0);
	p->Achange = -.15f;
	LightVolume = TempLightVolume * 1.2f;
	if (GodMode) return;
	if (instaKill) { Kill(); PlaySound_Strict(BullethitSFX); return; }
	if (frnd(1, 0) <= hitProb) {
		float ry = frnd(-3, 3), rp = frnd(-3, 3); // yaw's Rnd first: TurnEntity's defaulted global arg makes the tail needier
		turn_entity(Camera, rp, ry, 0, 0);
		const char *m = "";
		if (WearingVest > 0) {
			if (WearingVest == 1) {
				int r = irand(8, 1);
				if (r <= 5) { Stamina = 0; hit(&m, "A bullet penetrated your vest, making you gasp.", .1f, .5f); }
				else if (r == 6) hit(&m, "A bullet hit your left leg.", .8f, 1.2f);
				else if (r == 7) hit(&m, "A bullet hit your right leg.", .8f, 1.2f);
				else { Stamina = 0; hit(&m, "A bullet struck your neck, making you gasp.", 1.2f, 1.6f); }
			} else if (irand(10, 1) == 1) { Stamina -= 1; hit(&m, "A bullet hit your chest. The vest absorbed some of the damage.", .8f, 1.1f); }
			else { m = "A bullet hit your chest. The vest absorbed most of the damage."; Injuries += frnd(.1f, .5f); }
			if (Injuries >= 5 && irand(3, 1) == 1) Kill();
		} else {
			switch (irand(6, 1)) {
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
		int pvt = create_pivot(0);
		position_entity(pvt, entity_x(Collider, 0), (entity_y(Collider, 0) + entity_y(Camera, 0)) / 2, entity_z(Collider, 0), 0);
		point_entity(pvt, p->obj, 0);
		turn_entity(pvt, 0, 180, 0, 0);
		entity_pick(pvt, 2.5f);
		if (picked_entity()) {
			PlaySound2(Gunshot3SFX, Camera, pvt, .4f, frnd(.8f, 1));
			p = CreateParticle(picked_x(), picked_y(), picked_z(), 0, .03f, 0, 80);
			p->speed = .001f; p->SizeChange = .003f; p->A = .8f; p->Achange = -.01f;
			rotate_entity(p->pvt, entity_pitch(pvt, 0) - 180, entity_yaw(pvt, 0), 0, 0);
			for (int i = 0, k = irand(2, 3); i <= k; i++) {
				p = CreateParticle(picked_x(), picked_y(), picked_z(), 0, .006f, .003f, 80);
				p->speed = .02f; p->A = .8f; p->Achange = -.01f;
				float rp = frnd(170, 190), ry = frnd(-10, 10);
				rotate_entity(p->pvt, entity_pitch(pvt, 0) + rp, entity_yaw(pvt, 0) + ry, 0, 0);
			}
			Decals *de = CreateDecal(irand(13, 14), picked_x(), picked_y(), picked_z(), 0, 0, 0);
			if (de) {
				align_to_vector(de->obj, -picked_nx(), -picked_ny(), -picked_nz(), 3, 1);
				move_entity(de->obj, 0, 0, -.001f);
				entity_fx(de->obj, 1);
				de->lifetime = 70 * 20;
				entity_blend(de->obj, 2);
				de->Size = frnd(.028f, .034f);
				scale_sprite(de->obj, de->Size, de->Size);
			}
		}
		free_entity(pvt);
	}
}

void PlayMTFSound(int sound, NPCs *n) {
	if (n) n->SoundChn = PlaySound2(sound, Camera, n->Collider, 8, 1);
	if (!SelectedItem || !(SelectedItem->state2 == 3 && SelectedItem->state > 0)) return;
	str t = SelectedItem->itemtemplate->tempname;
	if (!(s_eq(t, "radio") || s_eq(t, "fineradio") || s_eq(t, "18vradio"))) return;
	if (sound != MTFSFX[5] || !channel_playing(RadioCHN[3])) {
		if (RadioCHN[3]) stop_channel(RadioCHN[3]);
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
	free_entity(n->obj);
	n->obj = copy_entity(DTextures[id + 1], 0);
	float t = .5f / mesh_width(n->obj);
	scale_entity(n->obj, t, t, t, 0);
	set_anim_time(n->obj, n->Frame, 0); // pose the new copy like the old one
}
