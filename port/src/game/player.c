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
// blitz's Int() of NaN/out of range floats is INT_MIN (x87 fistp), the game compares against that
static int badf(float v) { return isnan(v) || fabsf(v) >= 2147483648.0f; }

void Kill(void) {
	if (GodMode) return;
	if (BreathCHN && bbChannelPlaying(BreathCHN)) bbStopChannel(BreathCHN);
	if (KillTimer < 0) return;
	KillAnim = bbRand(0, 1);
	PlaySound_Strict(DamageSFX[0]);
	if (SelectedDifficulty->permaDeath) {
		bbDeleteFile(cat(bbCurrentDir(), SavePath, CurrSave, "\\save.txt"));
		bbDeleteDir(cat(SavePath, CurrSave));
		LoadSaveGames();
	}
	KillTimer = Min(-1, KillTimer);
	bbShowEntity(Head);
	bbPositionEntity(Head, bbEntityX(Camera, 1), bbEntityY(Camera, 1), bbEntityZ(Camera, 1), 1);
	bbResetEntity(Head);
	bbRotateEntity(Head, 0, bbEntityYaw(Camera, 0), 0, 0);
}

// TODO UpdateInfect (SCP-008), comes w/ the events
static void UpdateInfect(void) {}

static int steps(int set, int run) { // one footstep, blitz's CurrStepSFX cases
	switch (CurrStepSFX) {
	case 0: return PlaySound_Strict(StepSFX[set][run][bbRand(0, 7)]);
	case 1: return PlaySound_Strict(Step2SFX[bbRand(0, 2)]);
	case 2: return PlaySound_Strict(Step2SFX[bbRand(3, 5)]);
	case 3: return PlaySound_Strict(StepSFX[0][run][bbRand(0, 7)]);
	}
	return 0;
}
static int is(const char *a, const char *b) { return bbSeq(a, b); }

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
			bbShowEntity(Fog);
		} else { BlurTimer = 500; bbHideEntity(Fog); }
	}
	if (DeathTimer > 0) { DeathTimer -= FPSfactor; if (DeathTimer < 1) DeathTimer = -1; }
	else if (DeathTimer < 0) Kill();

	Stamina = Min(Stamina + .15f * (CurrSpeed > 0 ? FPSfactor / 1.25f : FPSfactor * 1.25f), 100);
	if (StaminaEffectTimer > 0) StaminaEffectTimer -= FPSfactor / 70;
	else if (StaminaEffect != 1) StaminaEffect = 1;

	if (!is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && bbKeyDown(KEY_SPRINT)) {
		int t = WearingGasMask > 0 || Wearing1499 > 0;
		if (Stamina < 5) { if (!bbChannelPlaying(BreathCHN)) BreathCHN = PlaySound_Strict(BreathSFX[t][0]); }
		else if (Stamina < 50 && (BreathCHN == 0 || !bbChannelPlaying(BreathCHN))) {
			BreathCHN = PlaySound_Strict(BreathSFX[t][bbRand(1, 3)]);
			bbChannelVolume(BreathCHN, Min((70 - Stamina) / 70, 1) * SFXVolume);
		}
	}
	for (int i = 0; i < MaxItemAmount; i++) if (Inventory[i] && is(Inventory[i]->itemtemplate->tempname, "finevest")) Stamina = Min(Stamina, 60);
	if (Wearing714) { Stamina = Min(Stamina, 10); Sanity = Max(-850, Sanity); }
	if (IsZombie) Crouch = 0;
	if (fabsf(CrouchState - Crouch) < .001f) CrouchState = Crouch;
	else CrouchState = CurveValue(Crouch, CrouchState, 10);

	if (!NoClip) {
		// blitz And/Or share one precedence level, left to right: ((down Or up) Or (right Or left)) And Playable
		int moving = ((bbKeyDown(KEY_DOWN) | bbKeyDown(KEY_UP)) | (bbKeyDown(KEY_RIGHT) | bbKeyDown(KEY_LEFT))) & Playable;
		if (moving || ForceMove > 0) {
			if (Crouch == 0 && bbKeyDown(KEY_SPRINT) && Stamina > 0 && !IsZombie) {
				Sprint = 2.5f;
				Stamina -= FPSfactor * .4f * StaminaEffect;
				if (Stamina <= 0) Stamina = -20;
			}
			if (is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && (bbEntityY(Collider, 0) < 2000 * RoomScale || bbEntityY(Collider, 0) > 2608 * RoomScale)) {
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
				ch = steps(bbInt(temp), run);
				if (ch) bbChannelVolume(ch, (1 - Crouch * (CurrStepSFX == 1 || CurrStepSFX == 2 ? .4f : .6f)) * SFXVolume);
			}
		}
	} else if (bbKeyDown(KEY_SPRINT)) Sprint = 2.5f;
	else if (bbKeyDown(KEY_CROUCH)) Sprint = .5f;

	if (bbKeyHit(KEY_CROUCH) && Playable) Crouch = !Crouch;
	float temp2 = Speed * Sprint / (1 + CrouchState);

	if (NoClip) {
		Shake = 0; CurrSpeed = 0; CrouchState = 0; Crouch = 0;
		bbRotateEntity(Collider, WrapAngle(bbEntityPitch(Camera, 0)), WrapAngle(bbEntityYaw(Camera, 0)), 0, 0);
		temp2 *= NoClipSpeed;
		if (bbKeyDown(KEY_DOWN)) bbMoveEntity(Collider, 0, 0, -temp2 * FPSfactor);
		if (bbKeyDown(KEY_UP)) bbMoveEntity(Collider, 0, 0, temp2 * FPSfactor);
		if (bbKeyDown(KEY_LEFT)) bbMoveEntity(Collider, -temp2 * FPSfactor, 0, 0);
		if (bbKeyDown(KEY_RIGHT)) bbMoveEntity(Collider, temp2 * FPSfactor, 0, 0);
		bbResetEntity(Collider);
	} else {
		temp2 /= Max((Injuries + 3) / 3, 1);
		if (Injuries > .5f) temp2 *= Min(sinf(Shake / 2 * DTOR) + 1.2f, 1);
		int go = 0;
		if (!IsZombie) {
			if (bbKeyDown(KEY_DOWN) && Playable) {
				go = 1; angle = 180;
				if (bbKeyDown(KEY_LEFT)) angle = 135;
				if (bbKeyDown(KEY_RIGHT)) angle = -135;
			} else if (bbKeyDown(KEY_UP) && Playable) {
				go = 1; angle = 0;
				if (bbKeyDown(KEY_LEFT)) angle = 45;
				if (bbKeyDown(KEY_RIGHT)) angle = -45;
			} else if (ForceMove > 0) { go = 1; angle = ForceAngle; }
			else if (Playable) {
				if (bbKeyDown(KEY_LEFT)) { angle = 90; go = 1; }
				if (bbKeyDown(KEY_RIGHT)) { angle = -90; go = 1; }
			}
		} else { go = 1; angle = ForceAngle; }
		angle = WrapAngle(bbEntityYaw(Collider, 1) + angle + 90);
		if (go) CurrSpeed = CurveValue(temp2, CurrSpeed, 20);
		else CurrSpeed = Max(CurveValue(0, CurrSpeed - .1f, 1), 0);
		if (!UnableToMove) bbTranslateEntity(Collider, cosf(angle * DTOR) * CurrSpeed * FPSfactor, 0, sinf(angle * DTOR) * CurrSpeed * FPSfactor, 1);

		int floor = 0;
		for (int i = 1; i <= bbCountCollisions(Collider); i++) if (bbCollisionY(Collider, i) < bbEntityY(Collider, 0) - .25f) floor = 1;
		if (floor) {
			if (DropSpeed < -.07f) {
				if (CurrStepSFX == 0) PlaySound_Strict(StepSFX[GetStepSound(Collider)][0][bbRand(0, 7)]);
				else steps(0, 0);
				PlayerSoundVolume = Max(3, PlayerSoundVolume);
			}
			DropSpeed = 0;
		} else if (PlayerFallingPickDistance != 0) {
			int pick = bbLinePick(bbEntityX(Collider, 0), bbEntityY(Collider, 0), bbEntityZ(Collider, 0), 0, -PlayerFallingPickDistance, 0, 0);
			DropSpeed = pick ? Min(Max(DropSpeed - .006f * FPSfactor, -2), 0) : 0;
		} else DropSpeed = Min(Max(DropSpeed - .006f * FPSfactor, -2), 0);
		PlayerFallingPickDistance = 10;
		if (!UnableToMove && ShouldEntitiesFall) bbTranslateEntity(Collider, 0, DropSpeed * FPSfactor, 0, 0);
	}
	ForceMove = 0;

	if (Injuries > 1) {
		temp2 = Bloodloss;
		BlurTimer = Max(Max(sinf(MilliSecs2() / 100.0f * DTOR) * Bloodloss * 30, Bloodloss * 2 * (2 - CrouchState)), BlurTimer);
		if (!(I_427->Using & (I_427->Timer < 70 * 360)) /* blitz Not covers the whole And */) Bloodloss = Min(Bloodloss + Min(Injuries, 3.5f) / 300 * FPSfactor, 100);
		if (temp2 <= 60 && Bloodloss > 60) { Msg = "You are feeling faint from the amount of blood you have lost."; MsgTimer = 70 * 4; }
	}
	UpdateInfect();
	if (Bloodloss > 0) {
		if (bbRnd(200, 0) < Min(Injuries, 4)) {
			int pvt = bbCreatePivot(0);
			float rx = bbRnd(-.05f, .05f), rz = bbRnd(-.05f, .05f); // x arg first, then z: blitz codegen order
			bbPositionEntity(pvt, bbEntityX(Collider, 0) + rx, bbEntityY(Collider, 0) - .05f, bbEntityZ(Collider, 0) + rz, 0);
			bbTurnEntity(pvt, 90, 0, 0, 0);
			bbEntityPick(pvt, .3f);
			int a = bbRand(360, 1), id = bbRand(15, 16); // Rand(360) runs before Rand(15,16) in blitz
			Decals *de = CreateDecal(id, bbPickedX(), bbPickedY() + .005f, bbPickedZ(), 90, a, 0);
			de->Size = bbRnd(.03f, .08f) * Min(Injuries, 3);
			bbEntityAlpha(de->obj, 1); bbScaleSprite(de->obj, de->Size, de->Size);
			int ch = PlaySound_Strict(DripSFX[bbRand(0, 2)]);
			bbChannelVolume(ch, bbRnd(0, .8f) * SFXVolume);
			bbChannelPitch(ch, bbRand(20000, 30000));
			bbFreeEntity(pvt);
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
		if (bbKeyHit(KEY_BLINK)) BlinkTimer = 0;
		if (bbKeyDown(KEY_BLINK) && BlinkTimer < -10) BlinkTimer = -10;
	}
	if (HeartBeatVolume > 0) {
		if (HeartBeatTimer <= 0) {
			int ch = PlaySound_Strict(HeartBeatSFX);
			bbChannelVolume(ch, HeartBeatVolume * SFXVolume);
			HeartBeatTimer = 70 * (60 / Max(HeartBeatRate, 1));
		} else HeartBeatTimer -= FPSfactor;
		HeartBeatVolume = Max(HeartBeatVolume - FPSfactor * .05f, 0);
	}
}

static void cough(void) { if (!CoughCHN || !bbChannelPlaying(CoughCHN)) CoughCHN = PlaySound_Strict(CoughSFX[bbRand(0, 2)]); }

void MouseLook(void) {
	CameraShake = Max(CameraShake - FPSfactor / 10, 0);
	bbCameraZoom(Camera, Min(1 + CurrCameraZoom / 400, 1.1f));
	CurrCameraZoom = Max(CurrCameraZoom - FPSfactor, 0);
	if (KillTimer >= 0 && FallTimer >= 0) {
		HeadDropSpeed = 0;
		if (badf(bbEntityX(Collider, 0))) {
			bbPositionEntity(Collider, bbEntityX(Camera, 1), bbEntityY(Camera, 1) - .5f, bbEntityZ(Camera, 1), 1);
			Msg = cat("EntityX(Collider) = NaN, RESETTING COORDINATES    -    New coordinates: ", bbStr(bbEntityX(Collider, 0)));
			MsgTimer = 300;
		}
		float up = sinf(Shake * DTOR) / (20 + CrouchState * 20) * .6f;
		float roll = Max(Min(sinf(Shake / 2 * DTOR) * 2.5f * Min(Injuries + .25f, 3), 8), -8);
		bbPositionEntity(Camera, bbEntityX(Collider, 0), bbEntityY(Collider, 0), bbEntityZ(Collider, 0), 0);
		bbRotateEntity(Camera, 0, bbEntityYaw(Collider, 0), roll * .5f, 0);
		bbMoveEntity(Camera, side, up + .6f + CrouchState * -.3f, 0);
		mouse_x_speed_1 = CurveValue(bbMouseXSpeed() * (MouseSens + .6f), mouse_x_speed_1, 6 / (MouseSens + 1) * MouseSmooth);
		if (badf(mouse_x_speed_1)) mouse_x_speed_1 = 0;
		if (PrevFPSFactor > 0 && fabsf(FPSfactor / PrevFPSFactor - 1) > 1) mouse_x_speed_1 = mouse_y_speed_1 = 0;
		mouse_y_speed_1 = CurveValue((InvertMouse ? -bbMouseYSpeed() : bbMouseYSpeed()) * (MouseSens + .6f), mouse_y_speed_1, 6 / (MouseSens + 1) * MouseSmooth);
		if (badf(mouse_y_speed_1)) mouse_y_speed_1 = 0;
		float yaw = mouse_x_speed_1 * mouselook_x_inc / (1 + WearingVest), pitch = mouse_y_speed_1 * mouselook_y_inc / (1 + WearingVest);
		bbTurnEntity(Collider, 0, -yaw, 0, 0);
		user_camera_pitch += pitch;
		if (user_camera_pitch > 70) user_camera_pitch = 70;
		if (user_camera_pitch < -70) user_camera_pitch = -70;
		// the yaw arg is needier, so blitz rolls its Rnd before the pitch's (both roll every frame)
		float sy = bbRnd(-CameraShake, CameraShake), sp = bbRnd(-CameraShake, CameraShake);
		bbRotateEntity(Camera, WrapAngle(user_camera_pitch + sp), WrapAngle(bbEntityYaw(Collider, 0) + sy), roll, 0);
		if (is(PlayerRoom->RoomTemplate->Name, "pocketdimension") && (bbEntityY(Collider, 0) < 2000 * RoomScale || bbEntityY(Collider, 0) > 2608 * RoomScale))
			bbRotateEntity(Camera, WrapAngle(bbEntityPitch(Camera, 0)), WrapAngle(bbEntityYaw(Camera, 0)), roll + WrapAngle(sinf(MilliSecs2() / 150.0f * DTOR) * 30), 0);
	} else {
		bbHideEntity(Collider);
		bbPositionEntity(Camera, bbEntityX(Head, 0), bbEntityY(Head, 0), bbEntityZ(Head, 0), 0);
		int floor = 0;
		for (int i = 1; i <= bbCountCollisions(Head); i++) if (bbCollisionY(Head, i) < bbEntityY(Head, 0) - .01f) floor = 1;
		if (floor) HeadDropSpeed = 0;
		else {
			float s = KillAnim == 0 ? 1 : -1;
			bbMoveEntity(Head, 0, 0, s * HeadDropSpeed);
			bbRotateEntity(Head, CurveAngle(-90 * s, bbEntityPitch(Head, 0), 20), bbEntityYaw(Head, 0), bbEntityRoll(Head, 0), 0);
			bbRotateEntity(Camera, CurveAngle(bbEntityPitch(Head, 0) - 40 * s, bbEntityPitch(Camera, 0), 40), bbEntityYaw(Camera, 0), bbEntityRoll(Camera, 0), 0);
			HeadDropSpeed -= .002f * FPSfactor;
		}
		float my = bbMouseYSpeed() * .05f * FPSfactor, mx = -bbMouseXSpeed() * .15f * FPSfactor;
		bbTurnEntity(Camera, InvertMouse ? -my : my, mx, 0, 0);
	}
	if (ParticleAmount == 2 && bbRand(1, 35) == 1) {
		int pvt = bbCreatePivot(0);
		bbPositionEntity(pvt, bbEntityX(Camera, 1), bbEntityY(Camera, 1), bbEntityZ(Camera, 1), 0);
		bbRotateEntity(pvt, 0, bbRnd(360, 0), 0, 0);
		bbRand(1, 2); // both branches are the same in blitz
		float dy = bbRnd(-.5f, .5f), dz = bbRnd(.5f, 1);
		bbMoveEntity(pvt, 0, dy, dz);
		Particles *p = CreateParticle(bbEntityX(pvt, 0), bbEntityY(pvt, 0), bbEntityZ(pvt, 0), 2, .002f, 0, 300);
		p->speed = .001f;
		float ry = bbRnd(360, 0), rp = bbRnd(-20, 20); // Rnd(360) first, blitz codegen order
		bbRotateEntity(p->pvt, rp, ry, 0, 0);
		p->SizeChange = -.00001f;
		bbFreeEntity(pvt);
	}
	if (bbMouseX() > mouse_right_limit || bbMouseX() < mouse_left_limit || bbMouseY() > mouse_bottom_limit || bbMouseY() < mouse_top_limit)
		bbMoveMouse(viewport_center_x, viewport_center_y);
	if (WearingGasMask || WearingHazmat || Wearing1499) {
		if (!Wearing714 && (WearingGasMask == 2 || Wearing1499 == 2 || WearingHazmat == 2)) Stamina = Min(100, Stamina + (100 - Stamina) * .01f * FPSfactor);
		if (WearingHazmat == 1) Stamina = Min(60, Stamina);
		bbShowEntity(GasMaskOverlay);
	} else bbHideEntity(GasMaskOverlay);
	if (WearingNightVision != 0) {
		bbShowEntity(NVOverlay);
		if (WearingNightVision == 2) bbEntityColor(NVOverlay, 0, 100, 255);
		else if (WearingNightVision == 3) bbEntityColor(NVOverlay, 255, 0, 0);
		else bbEntityColor(NVOverlay, 0, 255, 0);
		AmbientLightRooms(15);
		bbEntityTexture(Fog, FogNVTexture, 0, 0);
	} else {
		AmbientLightRooms(0);
		bbHideEntity(NVOverlay);
		bbEntityTexture(Fog, FogTexture, 0, 0);
	}
	for (int i = 0; i <= 5; i++) {
		if (SCP1025state[i] <= 0) continue;
		switch (i) {
		case 0: if (FPSfactor > 0 && bbRand(1, 1000) == 1) cough(); Stamina -= FPSfactor * .3f; break; // common cold
		case 1: if (bbRand(1, 9000) == 1 && !bbLen(Msg)) { Msg = "Your skin is feeling itchy."; MsgTimer = 70 * 4; } break;
		case 2: if (FPSfactor > 0 && bbRand(1, 800) == 1) cough(); Stamina -= FPSfactor * .1f; break; // lung cancer
		case 3: // appendicitis
			if (!(I_427->Using & (I_427->Timer < 70 * 360)) /* blitz Not covers the whole And */) SCP1025state[i] += FPSfactor * .0005f;
			if (SCP1025state[i] > 20) { if (SCP1025state[i] - FPSfactor <= 20) Msg = "The pain in your stomach is becoming unbearable."; Stamina -= FPSfactor * .3f; }
			else if (SCP1025state[i] > 10 && SCP1025state[i] - FPSfactor <= 10) Msg = "Your stomach is aching.";
			break;
		case 4: // asthma
			if (Stamina < 35) {
				if (bbRand(bbInt(140 + Stamina * 8), 1) == 1) cough();
				CurrSpeed = CurveValue(0, CurrSpeed, 10 + Stamina * 15);
			}
			break;
		case 5: // cardiac arrest
			if (!(I_427->Using & (I_427->Timer < 70 * 360)) /* blitz Not covers the whole And */) SCP1025state[i] += FPSfactor * .35f;
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
