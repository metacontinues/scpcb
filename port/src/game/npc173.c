// npc173.c: UpdateNPCs' cases for SCP-173 and SCP-106 (NPCtypeOldMan)
#include "g.h"

#define RN (PlayerRoom->RoomTemplate->Name)
static int is(const char *a, const char *b) { return s_eq(a, b); }
static float sinm(float k) { return sinf(MilliSecs2() * k * DTOR); }

// 173 tries buttons of plain unlocked doors next to it
static void opendoors(NPCs *n) {
	each(Doors, d) {
		if (!(!d->locked && !d->open && !s_len(d->Code) && d->KeyCard == 0)) continue;
		for (int i = 0; i <= 1; i++) {
			int b = d->buttons[i];
			if (!b || fabsf(entity_x(n->Collider, 0) - entity_x(b, 0)) >= .5f || fabsf(entity_z(n->Collider, 0) - entity_z(b, 0)) >= .5f) continue;
			if (!(d->openstate >= 180 || d->openstate <= 0)) continue;
			int pvt = create_pivot(0);
			position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0) + .5f, entity_z(n->Collider, 0), 0);
			point_entity(pvt, b, 0);
			move_entity(pvt, 0, 0, n->Speed * .6f);
			if (entity_pick(pvt, .5f) == b) { PlaySound_Strict(LoadTempSound("SFX\\Door\\DoorOpen173.ogg")); UseDoor(d, 0, 1); }
			free_entity(pvt);
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
	PlaySound_Strict(NeckSnapSFX[irand(0, 2)]);
	if (irand(2, 1) == 1) turn_entity(Camera, 0, irand(80, 100), 0, 0);
	else turn_entity(Camera, 0, irand(-100, -80), 0, 0);
	Kill();
}

static void fwd(NPCs *n) { // translate along the collider's yaw
	float y = entity_yaw(n->Collider, 0) + 90;
	translate_entity(n->Collider, cosf(y * DTOR) * n->Speed * FPSfactor, 0, sinf(y * DTOR) * n->Speed * FPSfactor, 0);
}

static void idle2(NPCs *n, float dist) { // 173 in its box, carried by n->Target
	if (n->Target) {
		int tmp = dist > HideDistance * .7f && !entity_visible(n->obj, Collider);
		if (!tmp) {
			point_entity(n->obj, n->Target->Collider, 0);
			rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 10), 0, 1);
			dist = entity_distance(n->Collider, n->Target->Collider);
			move_entity(n->Collider, 0, 0, .016f * FPSfactor * Max(Min((dist * 2 - 1) * .5f, 1), -.5f));
			n->GravityMult = 1;
		} else {
			position_entity(n->Collider, entity_x(n->Target->Collider, 0), entity_y(n->Target->Collider, 0) + .3f, entity_z(n->Target->Collider, 0), 0);
			reset_entity(n->Collider);
			n->DropSpeed = 0; n->GravityMult = 0;
		}
	}
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) + .05f + sinm(.08f) * .02f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0) - 180, 0, 0);
	show_entity(n->obj2);
	position_entity(n->obj2, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .05f + sinm(.08f) * .02f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj2, 0, entity_yaw(n->Collider, 0) - 180, 0, 0);
}

void Update173(NPCs *n) {
	static float SoundVol; // a blitz Local declared mid-function: keeps its value between NPCs/branches that skip it
	if (Curr173->Idle == 3) return;
	float dist = entity_distance(n->Collider, Collider);
	n->State3 = 1;
	if (n->Idle >= 2) { idle2(n, dist); return; }
	if (n->IdleTimer > .1f) { n->Idle = 1; n->IdleTimer = Max(n->IdleTimer - FPSfactor, .1f); }
	else if (n->IdleTimer == .1f) { n->Idle = 0; n->IdleTimer = 0; }
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .32f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0) - 180, 0, 0);
	if (n->Idle == 0) {
		int temp = 0, move = 1;
		if (dist < 15) {
			if (dist < 10 && entity_visible(n->Collider, Collider)) {
				temp = 1;
				n->EnemyX = entity_x(Collider, 1); n->EnemyY = entity_y(Collider, 1); n->EnemyZ = entity_z(Collider, 1);
			}
			SoundVol = Max(Min(Distance(entity_x(n->Collider, 0), entity_z(n->Collider, 0), n->PrevX, n->PrevZ) * 2.5f, 1), 0);
			n->SoundChn = LoopSound2(StoneDragSFX, n->SoundChn, Camera, n->Collider, 10, n->State);
			n->PrevX = entity_x(n->Collider, 0); n->PrevZ = entity_z(n->Collider, 0);
			if ((BlinkTimer < -16 || BlinkTimer > -6) && !IsNVGBlinking && entity_in_view(n->obj, Camera)) move = 0;
		}
		if (NoTarget) move = 1;
		if (!move) { // being looked at: freeze
			BlurVolume = Max(Max(Min((4 - dist) / 6, .9f), .1f), BlurVolume);
			CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 15 * Max((3.5f - dist) / 3.5f, 0));
			if (dist < 3.5f && MilliSecs2() - n->LastSeen > 60000 && temp) { PlaySound_Strict(HorrorSFX[irand(3, 4)]); n->LastSeen = MilliSecs2(); }
			if (dist < 1.5f && irand(700, 1) == 1) PlaySound2(Scp173SFX[irand(0, 2)], Camera, n->obj, 10, 1);
			if (dist < 1.5f && n->LastDist > 2 && temp) {
				CurrCameraZoom = 40;
				HeartBeatRate = Max(HeartBeatRate, 140);
				HeartBeatVolume = .5f;
				static const int h[] = {1, 2, 9, 10, 14};
				PlaySound_Strict(HorrorSFX[h[irand(5, 1) - 1]]);
			}
			n->LastDist = dist;
			n->State = Max(0, n->State - FPSfactor / 20);
		} else if (dist > 50) { // 6+ rooms away: teleport somewhere 15-25 units from the player
			if (irand(70, 1) == 1 && !is(RN, "exit1") && !is(RN, "gatea") && !is(RN, "pocketdimension")) {
				each(WayPoints, w) {
					if (w->door || irand(5, 1) != 1) continue;
					float x = fabsf(entity_x(Collider, 0) - entity_x(w->obj, 1));
					if (!(x < 25 && x > 15)) continue;
					float z = fabsf(entity_z(Collider, 0) - entity_z(w->obj, 1));
					if (!(z < 25 && z > 15)) continue;
					position_entity(n->Collider, entity_x(w->obj, 1), entity_y(w->obj, 1) + .25f, entity_z(w->obj, 1), 0);
					reset_entity(n->Collider);
					break;
				}
			}
		} else if (dist > HideDistance * .8f) { // 3-6 rooms away: hop between waypoints
			if (irand(70, 1) == 1) TeleportCloser(n);
		} else { // close: hunt
			n->State = CurveValue(SoundVol, n->State, 3);
			if (irand(20, 1) == 1) opendoors(n);
			if (NoTarget) { temp = 0; n->EnemyX = n->EnemyY = n->EnemyZ = 0; }
			if (temp) { // not looked at + sees the player
				if (dist < .65f) { if (KillTimer >= 0 && !GodMode) snapneck(n); }
				else {
					point_entity(n->Collider, Collider, 0);
					rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), entity_roll(n->Collider, 0), 0);
					fwd(n);
				}
			} else if (n->EnemyX != 0) { // go where the player was last seen
				if (Distance(entity_x(n->Collider, 0), entity_z(n->Collider, 0), n->EnemyX, n->EnemyZ) > .5f) {
					align_to_vector(n->Collider, n->EnemyX - entity_x(n->Collider, 0), 0, n->EnemyZ - entity_z(n->Collider, 0), 3, 1);
					move_entity(n->Collider, 0, 0, n->Speed * FPSfactor);
					if (irand(500, 1) == 1) n->EnemyX = n->EnemyY = n->EnemyZ = 0;
				} else n->EnemyX = n->EnemyY = n->EnemyZ = 0;
			} else {
				if (irand(400, 1) == 1) rotate_entity(n->Collider, 0, frnd(360, 0), 10, 0);
				fwd(n);
			}
		}
	}
	position_entity(n->Collider, entity_x(n->Collider, 0), Min(entity_y(n->Collider, 0), .35f), entity_z(n->Collider, 0), 0);
}

// ---- 106

static void step106(NPCs *n, float prev, float cur) {
	if ((prev <= 286 && cur > 286) || (prev <= 311 && cur > 311)) {
		int s = Step2SFX[irand(0, 2)];
		PlaySound2(s, Camera, n->Collider, 6, frnd(.8f, 1));
	}
}

static void walk106(NPCs *n, float dist, int visible) {
	if (dist > .8f) {
		if ((dist > 25 || is(RN, "pocketdimension") || visible || n->PathStatus != 1) && !is(RN, "gatea")) {
			if (dist > 40 || is(RN, "pocketdimension")) translate_entity(n->Collider, 0, (entity_y(Collider, 0) - .14f - entity_y(n->Collider, 0)) / 50, 0, 0);
			n->CurrSpeed = CurveValue(n->Speed, n->CurrSpeed, 10);
			point_entity(n->obj, Collider, 0);
			rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 10), 0, 0);
			if (KillTimer >= 0) {
				float pf = n->Frame;
				AnimateNPC(n, 284, 333, n->CurrSpeed * 43, 1);
				step106(n, pf, n->Frame);
			} else n->CurrSpeed = 0;
			n->PathTimer = Max(n->PathTimer - FPSfactor, 0);
			if (n->PathTimer <= 0) { n->PathStatus = FindPath(n, entity_x(Collider, 1), entity_y(Collider, 1), entity_z(Collider, 1)); n->PathTimer = 70 * 10; }
		} else if (n->PathTimer <= 0) {
			n->PathStatus = FindPath(n, entity_x(Collider, 1), entity_y(Collider, 1), entity_z(Collider, 1));
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
					translate_entity(n->Collider, 0, (entity_y(w->obj, 1) - .11f - entity_y(n->Collider, 0)) / 50, 0, 0);
					point_entity(n->obj, w->obj, 0);
					float d2 = entity_distance(n->Collider, w->obj);
					rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), Min(20, d2 * 10)), 0, 0);
					n->CurrSpeed = CurveValue(n->Speed, n->CurrSpeed, 10);
					float pf = anim_time(n->obj);
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
			point_entity(n->obj, Collider, 0);
			rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 10), 0, 0);
			if (ceilf(n->Frame) == 110 && !GodMode) {
				PlaySound_Strict(DamageSFX[1]);
				PlaySound_Strict(HorrorSFX[5]);
				if (is(RN, "pocketdimension")) {
					DeathMSG = "Subject D-9341. Body partially decomposed by what is assumed to be SCP-106's \"corrosion\" effect. Body disposed of via incineration.";
					Kill();
				} else {
					PlaySound_Strict(OldManSFX[3]);
					FallTimer = Min(-1, FallTimer);
					position_entity(Head, entity_x(Camera, 1), entity_y(Camera, 1), entity_z(Camera, 1), 1);
					reset_entity(Head);
					rotate_entity(Head, 0, entity_yaw(Camera, 0) + irand(-45, 45), 0, 0);
				}
			}
		}
	}
}

static void attack106(NPCs *n, float dist) {
	if (entity_y(n->Collider, 0) < entity_y(Collider, 0) - 20 - .55f) { // rise out of the floor under the player
		if (!PlayerRoom->RoomTemplate->DisableDecals) {
			Decals *de = CreateDecal(0, entity_x(Collider, 0), .01f, entity_z(Collider, 0), 90, irand(360, 1), 0);
			if (de) { de->Size = .05f; de->SizeChange = .001f; entity_alpha(de->obj, .8f); }
			UpdateDecals();
		}
		n->PrevY = entity_y(Collider, 0);
		set_anim_time(n->obj, 110, 0);
		if (!is(RN, "coffin")) position_entity(n->Collider, entity_x(Collider, 0), entity_y(Collider, 0) - 15, entity_z(Collider, 0), 0);
		PlaySound_Strict(DecaySFX[0]);
	}
	if (irand(500, 1) == 1) PlaySound2(OldManSFX[irand(0, 2)], Camera, n->Collider, 10, 1);
	n->SoundChn = LoopSound2(OldManSFX[4], n->SoundChn, Camera, n->Collider, 8, .8f);
	if (n->State > -10) {
		ShouldPlay = 66;
		if (n->Frame < 259) {
			position_entity(n->Collider, entity_x(n->Collider, 0), n->PrevY - .15f, entity_z(n->Collider, 0), 0);
			point_entity(n->obj, Collider, 0);
			rotate_entity(n->Collider, 0, CurveValue(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 100), 0, 1);
			AnimateNPC(n, 110, 259, .15f, 0);
		} else n->State = -10;
	} else {
		if (!is(RN, "gatea")) ShouldPlay = 10;
		int visible = dist < 8 ? entity_visible(n->Collider, Collider) : 0;
		if (NoTarget) visible = 0;
		if (visible) {
			if (!is(RN, "gatea")) n->PathTimer = 0;
			if (entity_in_view(n->Collider, Camera)) {
				GiveAchievement(Achv106, 1);
				BlurVolume = Max(Max(Min((4 - dist) / 6, .9f), .1f), BlurVolume);
				CurrCameraZoom = Max(CurrCameraZoom, (sinf((float)MilliSecs2() / 20 * DTOR) + 1) * 20 * Max((4 - dist) / 4, 0));
				if (MilliSecs2() - n->LastSeen > 60000) { CurrCameraZoom = 40; PlaySound_Strict(HorrorSFX[6]); n->LastSeen = MilliSecs2(); }
			}
		} else n->State -= FPSfactor;
		walk106(n, dist, visible);
	}
	move_entity(n->Collider, 0, 0, n->CurrSpeed * FPSfactor);
	if (n->State <= irand(-3500, -3000) && !entity_in_view(n->obj, Camera) && dist > 5) { // gives up when out of sight
		n->State = irand(22000, 27000);
		position_entity(n->Collider, 0, 500, 0, 0);
	}
	if (FallTimer < -250) { MoveToPocketDimension(); n->State = 250; }
	if (n->Reload == 0 && dist > 10 && !is(RN, "pocketdimension") && !is(RN, "gatea") && n->State < -5 && !entity_in_view(n->obj, Camera)) {
		// pop up behind the player
		turn_entity(Collider, 0, 180, 0, 0);
		int pick = entity_pick(Collider, 5);
		turn_entity(Collider, 0, 180, 0, 0);
		if (pick) {
			TeleportEntity(n->Collider, picked_x(), picked_y(), picked_z(), n->CollRadius, 0, 2, 0);
			point_entity(n->Collider, Collider, 0);
			rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), 0, 0);
			move_entity(n->Collider, 0, 0, -2);
			PlaySound2(OldManSFX[3], Camera, n->Collider, 10, 1);
			n->SoundChn2 = PlaySound2(OldManSFX[6 + irand(0, 2)], Camera, n->Collider, 10, 1);
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
		hide_entity(n->obj); hide_entity(n->obj2);
		position_entity(n->obj, 0, 500, 0, 1);
		return;
	}
	float dist = entity_distance(n->Collider, Collider);
	int spawn = 1;
	if (is(RN, "dimension1499")) spawn = 0;
	Events *e = FindEvent("room860");
	if (e && e->EventState == 1) spawn = 0;
	if (is(RN, "room049") && entity_y(Collider, 0) <= -2848 * RoomScale) spawn = 0;
	e = FindEvent("gatea"); // gate a's event keeps 106 around, and wins over room860
	if (e && e->EventState != 0) { spawn = 1; n->Idle = is(RN, "dimension1499"); }
	if (!spawn && n->State <= 0) { n->State = irand(22000, 27000); position_entity(n->Collider, 0, 500, 0, 0); }

	if (n->Idle != 0 || !spawn) { hide_entity(n->obj2); return; }
	if (n->State <= 0) attack106(n, dist);
	else { // idling under the map
		n->CurrSpeed = 0;
		move_entity(n->Collider, 0, (entity_y(Collider, 0) - 30 - entity_y(n->Collider, 0)) / 200, 0);
		n->DropSpeed = 0;
		n->Frame = 110;
		if (!PlayerRoom->RoomTemplate->DisableDecals && !is(RN, "gatea")) n->State -= SelectedDifficulty->aggressiveNPCs ? FPSfactor * 2 : FPSfactor;
	}
	reset_entity(n->Collider);
	n->DropSpeed = 0;
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .15f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0), 0, 0);
	position_entity(n->obj2, entity_x(n->obj, 0), entity_y(n->obj, 0), entity_z(n->obj, 0), 0);
	rotate_entity(n->obj2, 0, entity_yaw(n->Collider, 0) - 180, 0, 0);
	move_entity(n->obj2, 0, 8.6f * .11f, -1.5f * .11f);
	float fd = CameraFogFar * LightVolume * .6f;
	if (is(RN, "pocketdimension") || is(RN, "gatea") || dist < fd) hide_entity(n->obj2);
	else { show_entity(n->obj2); entity_alpha(n->obj2, Min(dist - fd, 1)); }
}
