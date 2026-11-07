// player.c: Main.bb's MovePlayer, MouseLook, Kill, CurveValue/CurveAngle
#include "g.h"

TLIST(SCP427);

float CurveValue(float number, float old, float smooth) {
	if (FPSfactor == 0) return old;
	float v = old + (number - old) * (1.0f / smooth * FPSfactor);
	return number < old ? Max(v, number) : Min(v, number);
}
float CurveAngle(float val, float old, float smooth) {
	if (FPSfactor == 0) return old;
	float diff = WrapAngle(val) - WrapAngle(old);
	if (diff > 180) diff -= 360;
	if (diff < -180) diff += 360;
	return WrapAngle(old + diff * (1.0f / smooth * FPSfactor));
}
float Distance(float x1, float y1, float x2, float y2) { float x = x2 - x1, y = y2 - y1; return sqrtf(x * x + y * y); }
// NaN/huge floats: bail out instead of carrying them around
static int badf(float v) { return isnan(v) || fabsf(v) >= 2147483648.0f; }

void Kill(void) {
	if (GodMode) return;
	if (BreathCHN && channel_playing(BreathCHN)) stop_channel(BreathCHN);
	if (KillTimer < 0) return;
	KillAnim = irand(0, 1);
	PlaySound_Strict(DamageSFX[0]);
	if (SelectedDifficulty->permaDeath) {
		delete_file(cat(current_dir(), SavePath, CurrSave, "\\save.txt"));
		delete_dir(cat(SavePath, CurrSave));
		LoadSaveGames();
	}
	KillTimer = Min(-1, KillTimer);
	show_entity(Head);
	position_entity(Head, entity_x(Camera, 1), entity_y(Camera, 1), entity_z(Camera, 1), 1);
	reset_entity(Head);
	rotate_entity(Head, 0, entity_yaw(Camera, 0), 0, 0);
}

// TODO UpdateInfect (SCP-008), comes w/ the events
static void UpdateInfect(void) {}

static int steps(int set, int run) { // one footstep, blitz's CurrStepSFX cases
	switch (CurrStepSFX) {
	case 0: return PlaySound_Strict(StepSFX[set][run][irand(0, 7)]);
	case 1: return PlaySound_Strict(Step2SFX[irand(0, 2)]);
	case 2: return PlaySound_Strict(Step2SFX[irand(3, 5)]);
	case 3: return PlaySound_Strict(StepSFX[0][run][irand(0, 7)]);
	}
	return 0;
}
static int is(const char *a, const char *b) { return s_eq(a, b); }

void MovePlayer(void) {
	float Sprint = 1, Speed = .018f, angle = 0, temp;
	if (SuperMan) {
		Speed *= 3;
		SuperManTimer += FPSfactor;
		CameraShake = sinf(SuperManTimer / 5 * DTOR) * (SuperManTimer / 1500);
		if (SuperManTimer > 70 * 50) {
			DeathMSG = "A Class D jumpsuit found in [DATA REDACTED]. Upon further examination, the jumpsuit was found to be filled with 12.5 kilograms of blue ash-like substance. "
				"Chemical analysis of the substance remains non-conclusive. Most likely related to SCP-914.";
			Kill();
			show_entity(Fog);
		} else { BlurTimer = 500; hide_entity(Fog); }
	}
	if (DeathTimer > 0) { DeathTimer -= FPSfactor; if (DeathTimer < 1) DeathTimer = -1; }
	else if (DeathTimer < 0) Kill();

	Stamina = Min(Stamina + .15f * (CurrSpeed > 0 ? FPSfactor / 1.25f : FPSfactor * 1.25f), 100);
	if (StaminaEffectTimer > 0) StaminaEffectTimer -= FPSfactor / 70;
	else if (StaminaEffect != 1) StaminaEffect = 1;

	if (!is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && key_down(KEY_SPRINT)) {
		int t = WearingGasMask > 0 || Wearing1499 > 0;
		if (Stamina < 5) { if (!channel_playing(BreathCHN)) BreathCHN = PlaySound_Strict(BreathSFX[t][0]); }
		else if (Stamina < 50 && (BreathCHN == 0 || !channel_playing(BreathCHN))) {
			BreathCHN = PlaySound_Strict(BreathSFX[t][irand(1, 3)]);
			channel_volume(BreathCHN, Min((70 - Stamina) / 70, 1) * SFXVolume);
		}
	}
	for (int i = 0; i < MaxItemAmount; i++) if (Inventory[i] && is(Inventory[i]->itemtemplate->tempname, "finevest")) Stamina = Min(Stamina, 60);
	if (Wearing714) { Stamina = Min(Stamina, 10); Sanity = Max(-850, Sanity); }
	if (IsZombie) Crouch = 0;
	if (fabsf(CrouchState - Crouch) < .001f) CrouchState = Crouch;
	else CrouchState = CurveValue(Crouch, CrouchState, 10);

	if (!NoClip) {
		int moving = (key_down(KEY_DOWN) || key_down(KEY_UP) || key_down(KEY_RIGHT) || key_down(KEY_LEFT)) && Playable;
		if (moving || ForceMove > 0) {
			if (Crouch == 0 && key_down(KEY_SPRINT) && Stamina > 0 && !IsZombie) {
				Sprint = 2.5f;
				Stamina -= FPSfactor * .4f * StaminaEffect;
				if (Stamina <= 0) Stamina = -20;
			}
			if (is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && (entity_y(Collider, 0) < 2000 * RoomScale || entity_y(Collider, 0) > 2608 * RoomScale)) {
				Stamina = 0; Speed = .015f; Sprint = 1;
			}
			if (ForceMove > 0) Speed *= ForceMove;
			if (SelectedItem) {
				str t = SelectedItem->itemtemplate->tempname;
				if (is(t, "firstaid") || is(t, "finefirstaid") || is(t, "firstaid2")) Sprint = 0;
			}
			temp = fmodf(Shake, 360);
			if (!UnableToMove) Shake = fmodf(Shake + FPSfactor * Min(Sprint, 1.5f) * 7, 720);
			if (temp < 180 && fmodf(Shake, 360) >= 180 && KillTimer >= 0) {
				int run = Sprint != 1, ch = 0;
				if (CurrStepSFX == 0) temp = GetStepSound(Collider);
				if (CurrStepSFX == 0 || CurrStepSFX == 3) PlayerSoundVolume = Max(run ? 2.5f - Crouch * .6f : 4, PlayerSoundVolume);
				ch = steps(iround(temp), run);
				if (ch) channel_volume(ch, (1 - Crouch * (CurrStepSFX == 1 || CurrStepSFX == 2 ? .4f : .6f)) * SFXVolume);
			}
		}
	} else if (key_down(KEY_SPRINT)) Sprint = 2.5f;
	else if (key_down(KEY_CROUCH)) Sprint = .5f;

	if (key_hit(KEY_CROUCH) && Playable) Crouch = !Crouch;
	float temp2 = Speed * Sprint / (1 + CrouchState);

	if (NoClip) {
		Shake = 0; CurrSpeed = 0; CrouchState = 0; Crouch = 0;
		rotate_entity(Collider, WrapAngle(entity_pitch(Camera, 0)), WrapAngle(entity_yaw(Camera, 0)), 0, 0);
		temp2 *= NoClipSpeed;
		if (key_down(KEY_DOWN)) move_entity(Collider, 0, 0, -temp2 * FPSfactor);
		if (key_down(KEY_UP)) move_entity(Collider, 0, 0, temp2 * FPSfactor);
		if (key_down(KEY_LEFT)) move_entity(Collider, -temp2 * FPSfactor, 0, 0);
		if (key_down(KEY_RIGHT)) move_entity(Collider, temp2 * FPSfactor, 0, 0);
		reset_entity(Collider);
	} else {
		temp2 /= Max((Injuries + 3) / 3, 1);
		if (Injuries > .5f) temp2 *= Min(sinf(Shake / 2 * DTOR) + 1.2f, 1);
		int go = 0;
		if (!IsZombie) {
			if (key_down(KEY_DOWN) && Playable) {
				go = 1; angle = 180;
				if (key_down(KEY_LEFT)) angle = 135;
				if (key_down(KEY_RIGHT)) angle = -135;
			} else if (key_down(KEY_UP) && Playable) {
				go = 1; angle = 0;
				if (key_down(KEY_LEFT)) angle = 45;
				if (key_down(KEY_RIGHT)) angle = -45;
			} else if (ForceMove > 0) { go = 1; angle = ForceAngle; }
			else if (Playable) {
				if (key_down(KEY_LEFT)) { angle = 90; go = 1; }
				if (key_down(KEY_RIGHT)) { angle = -90; go = 1; }
			}
		} else { go = 1; angle = ForceAngle; }
		angle = WrapAngle(entity_yaw(Collider, 1) + angle + 90);
		if (go) CurrSpeed = CurveValue(temp2, CurrSpeed, 20);
		else CurrSpeed = Max(CurveValue(0, CurrSpeed - .1f, 1), 0);
		if (!UnableToMove) translate_entity(Collider, cosf(angle * DTOR) * CurrSpeed * FPSfactor, 0, sinf(angle * DTOR) * CurrSpeed * FPSfactor, 1);

		int floor = 0;
		for (int i = 1; i <= count_collisions(Collider); i++) if (collision_y(Collider, i) < entity_y(Collider, 0) - .25f) floor = 1;
		if (floor) {
			if (DropSpeed < -.07f) {
				if (CurrStepSFX == 0) PlaySound_Strict(StepSFX[GetStepSound(Collider)][0][irand(0, 7)]);
				else steps(0, 0);
				PlayerSoundVolume = Max(3, PlayerSoundVolume);
			}
			DropSpeed = 0;
		} else if (PlayerFallingPickDistance != 0) {
			int pick = line_pick(entity_x(Collider, 0), entity_y(Collider, 0), entity_z(Collider, 0), 0, -PlayerFallingPickDistance, 0, 0);
			DropSpeed = pick ? Min(Max(DropSpeed - .006f * FPSfactor, -2), 0) : 0;
		} else DropSpeed = Min(Max(DropSpeed - .006f * FPSfactor, -2), 0);
		PlayerFallingPickDistance = 10;
		if (!UnableToMove && ShouldEntitiesFall) translate_entity(Collider, 0, DropSpeed * FPSfactor, 0, 0);
	}
	ForceMove = 0;

	if (Injuries > 1) {
		temp2 = Bloodloss;
		BlurTimer = Max(Max(sinf(MilliSecs2() / 100.0f * DTOR) * Bloodloss * 30, Bloodloss * 2 * (2 - CrouchState)), BlurTimer);
		if (!(I_427->Using && I_427->Timer < 70 * 360)) Bloodloss = Min(Bloodloss + Min(Injuries, 3.5f) / 300 * FPSfactor, 100);
		if (temp2 <= 60 && Bloodloss > 60) { Msg = "You are feeling faint from the amount of blood you have lost."; MsgTimer = 70 * 4; }
	}
	UpdateInfect();
	if (Bloodloss > 0) {
		if (frnd(200, 0) < Min(Injuries, 4)) {
			int pvt = create_pivot(0);
			float rx = frnd(-.05f, .05f), rz = frnd(-.05f, .05f);
			position_entity(pvt, entity_x(Collider, 0) + rx, entity_y(Collider, 0) - .05f, entity_z(Collider, 0) + rz, 0);
			turn_entity(pvt, 90, 0, 0, 0);
			entity_pick(pvt, .3f);
			int a = irand(360, 1), id = irand(15, 16);
			Decals *de = CreateDecal(id, picked_x(), picked_y() + .005f, picked_z(), 90, a, 0);
			de->Size = frnd(.03f, .08f) * Min(Injuries, 3);
			entity_alpha(de->obj, 1); scale_sprite(de->obj, de->Size, de->Size);
			int ch = PlaySound_Strict(DripSFX[irand(0, 2)]);
			channel_volume(ch, frnd(0, .8f) * SFXVolume);
			channel_pitch(ch, irand(20000, 30000));
			free_entity(pvt);
		}
		CurrCameraZoom = Max(CurrCameraZoom, (sinf(MilliSecs2() / 20.0f * DTOR) + 1) * Bloodloss * .2f);
		if (Bloodloss > 60) Crouch = 1;
		if (Bloodloss >= 100) { Kill(); HeartBeatVolume = 0; }
		else if (Bloodloss > 80) { HeartBeatRate = Max(150 - (Bloodloss - 80) * 5, HeartBeatRate); HeartBeatVolume = Max(HeartBeatVolume, .75f + (Bloodloss - 80) * .0125f); }
		else if (Bloodloss > 35) { HeartBeatRate = Max(70 + Bloodloss, HeartBeatRate); HeartBeatVolume = Max(HeartBeatVolume, (Bloodloss - 35) / 60); }
	}
	if (HealTimer > 0) {
		HealTimer -= FPSfactor / 70;
		Bloodloss = Min(Bloodloss + 2 / 400.0f * FPSfactor, 100);
		Injuries = Max(Injuries - FPSfactor / 70 / 30, 0);
	}
	if (Playable) {
		if (key_hit(KEY_BLINK)) BlinkTimer = 0;
		if (key_down(KEY_BLINK) && BlinkTimer < -10) BlinkTimer = -10;
	}
	if (HeartBeatVolume > 0) {
		if (HeartBeatTimer <= 0) {
			int ch = PlaySound_Strict(HeartBeatSFX);
			channel_volume(ch, HeartBeatVolume * SFXVolume);
			HeartBeatTimer = 70 * (60 / Max(HeartBeatRate, 1));
		} else HeartBeatTimer -= FPSfactor;
		HeartBeatVolume = Max(HeartBeatVolume - FPSfactor * .05f, 0);
	}
}

static void cough(void) { if (!CoughCHN || !channel_playing(CoughCHN)) CoughCHN = PlaySound_Strict(CoughSFX[irand(0, 2)]); }

void MouseLook(void) {
	CameraShake = Max(CameraShake - FPSfactor / 10, 0);
	camera_zoom(Camera, Min(1 + CurrCameraZoom / 400, 1.1f));
	CurrCameraZoom = Max(CurrCameraZoom - FPSfactor, 0);
	if (KillTimer >= 0 && FallTimer >= 0) {
		HeadDropSpeed = 0;
		if (badf(entity_x(Collider, 0))) {
			position_entity(Collider, entity_x(Camera, 1), entity_y(Camera, 1) - .5f, entity_z(Camera, 1), 1);
			Msg = cat("EntityX(Collider) = NaN, RESETTING COORDINATES    -    New coordinates: ", s_str(entity_x(Collider, 0)));
			MsgTimer = 300;
		}
		float up = sinf(Shake * DTOR) / (20 + CrouchState * 20) * .6f;
		float roll = Max(Min(sinf(Shake / 2 * DTOR) * 2.5f * Min(Injuries + .25f, 3), 8), -8);
		position_entity(Camera, entity_x(Collider, 0), entity_y(Collider, 0), entity_z(Collider, 0), 0);
		rotate_entity(Camera, 0, entity_yaw(Collider, 0), roll * .5f, 0);
		move_entity(Camera, side, up + .6f + CrouchState * -.3f, 0);
		mouse_x_speed_1 = CurveValue(mouse_x_speed() * (MouseSens + .6f), mouse_x_speed_1, 6 / (MouseSens + 1) * MouseSmooth);
		if (badf(mouse_x_speed_1)) mouse_x_speed_1 = 0;
		if (PrevFPSFactor > 0 && fabsf(FPSfactor / PrevFPSFactor - 1) > 1) mouse_x_speed_1 = mouse_y_speed_1 = 0;
		mouse_y_speed_1 = CurveValue((InvertMouse ? -mouse_y_speed() : mouse_y_speed()) * (MouseSens + .6f), mouse_y_speed_1, 6 / (MouseSens + 1) * MouseSmooth);
		if (badf(mouse_y_speed_1)) mouse_y_speed_1 = 0;
		float yaw = mouse_x_speed_1 * mouselook_x_inc / (1 + WearingVest), pitch = mouse_y_speed_1 * mouselook_y_inc / (1 + WearingVest);
		turn_entity(Collider, 0, -yaw, 0, 0);
		user_camera_pitch += pitch;
		if (user_camera_pitch > 70) user_camera_pitch = 70;
		if (user_camera_pitch < -70) user_camera_pitch = -70;
		// the yaw arg is needier, so blitz rolls its Rnd before the pitch's (both roll every frame)
		float sy = frnd(-CameraShake, CameraShake), sp = frnd(-CameraShake, CameraShake);
		rotate_entity(Camera, WrapAngle(user_camera_pitch + sp), WrapAngle(entity_yaw(Collider, 0) + sy), roll, 0);
		if (is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && (entity_y(Collider, 0) < 2000 * RoomScale || entity_y(Collider, 0) > 2608 * RoomScale))
			rotate_entity(Camera, WrapAngle(entity_pitch(Camera, 0)), WrapAngle(entity_yaw(Camera, 0)), roll + WrapAngle(sinf(MilliSecs2() / 150.0f * DTOR) * 30), 0);
	} else {
		hide_entity(Collider);
		position_entity(Camera, entity_x(Head, 0), entity_y(Head, 0), entity_z(Head, 0), 0);
		int floor = 0;
		for (int i = 1; i <= count_collisions(Head); i++) if (collision_y(Head, i) < entity_y(Head, 0) - .01f) floor = 1;
		if (floor) HeadDropSpeed = 0;
		else {
			float s = KillAnim == 0 ? 1 : -1;
			move_entity(Head, 0, 0, s * HeadDropSpeed);
			rotate_entity(Head, CurveAngle(-90 * s, entity_pitch(Head, 0), 20), entity_yaw(Head, 0), entity_roll(Head, 0), 0);
			rotate_entity(Camera, CurveAngle(entity_pitch(Head, 0) - 40 * s, entity_pitch(Camera, 0), 40), entity_yaw(Camera, 0), entity_roll(Camera, 0), 0);
			HeadDropSpeed -= .002f * FPSfactor;
		}
		float my = mouse_y_speed() * .05f * FPSfactor, mx = -mouse_x_speed() * .15f * FPSfactor;
		turn_entity(Camera, InvertMouse ? -my : my, mx, 0, 0);
	}
	if (ParticleAmount == 2 && irand(1, 35) == 1) {
		int pvt = create_pivot(0);
		position_entity(pvt, entity_x(Camera, 1), entity_y(Camera, 1), entity_z(Camera, 1), 0);
		rotate_entity(pvt, 0, frnd(360, 0), 0, 0);
		float dy = frnd(-.5f, .5f), dz = frnd(.5f, 1);
		move_entity(pvt, 0, dy, dz);
		Particles *p = CreateParticle(entity_x(pvt, 0), entity_y(pvt, 0), entity_z(pvt, 0), 2, .002f, 0, 300);
		p->speed = .001f;
		float ry = frnd(360, 0), rp = frnd(-20, 20);
		rotate_entity(p->pvt, rp, ry, 0, 0);
		p->SizeChange = -.00001f;
		free_entity(pvt);
	}
	if (mouse_x() > mouse_right_limit || mouse_x() < mouse_left_limit || mouse_y() > mouse_bottom_limit || mouse_y() < mouse_top_limit)
		move_mouse(viewport_center_x, viewport_center_y);
	if (WearingGasMask || WearingHazmat || Wearing1499) {
		if (!Wearing714 && (WearingGasMask == 2 || Wearing1499 == 2 || WearingHazmat == 2)) Stamina = Min(100, Stamina + (100 - Stamina) * .01f * FPSfactor);
		if (WearingHazmat == 1) Stamina = Min(60, Stamina);
		show_entity(GasMaskOverlay);
	} else hide_entity(GasMaskOverlay);
	if (WearingNightVision != 0) {
		show_entity(NVOverlay);
		if (WearingNightVision == 2) entity_color(NVOverlay, 0, 100, 255);
		else if (WearingNightVision == 3) entity_color(NVOverlay, 255, 0, 0);
		else entity_color(NVOverlay, 0, 255, 0);
		AmbientLightRooms(15);
		entity_texture(Fog, FogNVTexture, 0, 0);
	} else {
		AmbientLightRooms(0);
		hide_entity(NVOverlay);
		entity_texture(Fog, FogTexture, 0, 0);
	}
	for (int i = 0; i <= 5; i++) {
		if (SCP1025state[i] <= 0) continue;
		switch (i) {
		case 0: if (FPSfactor > 0 && irand(1, 1000) == 1) cough(); Stamina -= FPSfactor * .3f; break; // common cold
		case 1: if (irand(1, 9000) == 1 && !s_len(Msg)) { Msg = "Your skin is feeling itchy."; MsgTimer = 70 * 4; } break;
		case 2: if (FPSfactor > 0 && irand(1, 800) == 1) cough(); Stamina -= FPSfactor * .1f; break; // lung cancer
		case 3: // appendicitis
			if (!(I_427->Using && I_427->Timer < 70 * 360)) SCP1025state[i] += FPSfactor * .0005f;
			if (SCP1025state[i] > 20) { if (SCP1025state[i] - FPSfactor <= 20) Msg = "The pain in your stomach is becoming unbearable."; Stamina -= FPSfactor * .3f; }
			else if (SCP1025state[i] > 10 && SCP1025state[i] - FPSfactor <= 10) Msg = "Your stomach is aching.";
			break;
		case 4: // asthma
			if (Stamina < 35) {
				if (irand(iround(140 + Stamina * 8), 1) == 1) cough();
				CurrSpeed = CurveValue(0, CurrSpeed, 10 + Stamina * 15);
			}
			break;
		case 5: // cardiac arrest
			if (!(I_427->Using && I_427->Timer < 70 * 360)) SCP1025state[i] += FPSfactor * .35f;
			if (SCP1025state[i] > 110) {
				HeartBeatRate = 0;
				BlurTimer = Max(BlurTimer, 500);
				if (SCP1025state[i] > 140) {
					DeathMSG = "\"He died of a cardiac arrest after reading SCP-1025, that's for sure. Is there such a thing as psychosomatic cardiac arrest, or does SCP-1025 have some "
						"anomalous properties we are not yet aware of?\"";
					Kill();
				}
			} else { HeartBeatRate = Max(HeartBeatRate, 70 + SCP1025state[i]); HeartBeatVolume = 1; }
			break;
		}
	}
}
