// sounds.c: LoadAllSounds.bb + Main.bb's 3d-ish sound helpers
#include "g.h"

#define L(p) LoadSound_Strict(p)
#define LN(fmt, i) LoadSound_Strict(sf(fmt, (int)(i)))

void LoadAllSounds(void) {
	for (int i = 0; i <= 2; i++) {
		OpenDoorSFX[0][i] = LN("SFX\\Door\\DoorOpen%d.ogg", i + 1);
		CloseDoorSFX[0][i] = LN("SFX\\Door\\DoorClose%d.ogg", i + 1);
		OpenDoorSFX[2][i] = LN("SFX\\Door\\Door2Open%d.ogg", i + 1);
		CloseDoorSFX[2][i] = LN("SFX\\Door\\Door2Close%d.ogg", i + 1);
		OpenDoorSFX[3][i] = LN("SFX\\Door\\ElevatorOpen%d.ogg", i + 1);
		CloseDoorSFX[3][i] = LN("SFX\\Door\\ElevatorClose%d.ogg", i + 1);
	}
	for (int i = 0; i <= 1; i++) {
		OpenDoorSFX[1][i] = LN("SFX\\Door\\BigDoorOpen%d.ogg", i + 1);
		CloseDoorSFX[1][i] = LN("SFX\\Door\\BigDoorClose%d.ogg", i + 1);
	}
	KeyCardSFX1 = L("SFX\\Interact\\KeyCardUse1.ogg"); KeyCardSFX2 = L("SFX\\Interact\\KeyCardUse2.ogg");
	ButtonSFX2 = L("SFX\\Interact\\Button2.ogg");
	ScannerSFX1 = L("SFX\\Interact\\ScannerUse1.ogg"); ScannerSFX2 = L("SFX\\Interact\\ScannerUse2.ogg");
	OpenDoorFastSFX = L("SFX\\Door\\DoorOpenFast.ogg");
	CautionSFX = L("SFX\\Room\\LockroomSiren.ogg");
	CameraSFX = L("SFX\\General\\Camera.ogg");
	StoneDragSFX = L("SFX\\SCP\\173\\StoneDrag.ogg");
	GunshotSFX = L("SFX\\General\\Gunshot.ogg"); Gunshot2SFX = L("SFX\\General\\Gunshot2.ogg");
	Gunshot3SFX = L("SFX\\General\\BulletMiss.ogg"); BullethitSFX = L("SFX\\General\\BulletHit.ogg");
	TeslaIdleSFX = L("SFX\\Room\\Tesla\\Idle.ogg"); TeslaActivateSFX = L("SFX\\Room\\Tesla\\WindUp.ogg");
	TeslaPowerUpSFX = L("SFX\\Room\\Tesla\\PowerUp.ogg");
	MagnetUpSFX = L("SFX\\Room\\106Chamber\\MagnetUp.ogg"); MagnetDownSFX = L("SFX\\Room\\106Chamber\\MagnetDown.ogg");
	for (int i = 0; i <= 3; i++) DecaySFX[i] = LN("SFX\\SCP\\106\\Decay%d.ogg", i);
	BurstSFX = L("SFX\\Room\\TunnelBurst.ogg");
	for (int i = 0; i <= 2; i++) RustleSFX[i] = LN("SFX\\SCP\\372\\Rustle%d.ogg", i);
	Death914SFX = L("SFX\\SCP\\914\\PlayerDeath.ogg"); Use914SFX = L("SFX\\SCP\\914\\PlayerUse.ogg");
	for (int i = 0; i <= 3; i++) DripSFX[i] = LN("SFX\\Character\\D9341\\BloodDrip%d.ogg", i);
	LeverSFX = L("SFX\\Interact\\LeverFlip.ogg"); LightSFX = L("SFX\\General\\LightSwitch.ogg");
	ButtGhostSFX = L("SFX\\SCP\\Joke\\789J.ogg");
	RadioSFX[1][0] = L("SFX\\Radio\\RadioAlarm.ogg"); RadioSFX[1][1] = L("SFX\\Radio\\RadioAlarm2.ogg");
	for (int i = 0; i <= 8; i++) RadioSFX[2][i] = LN("SFX\\Radio\\scpradio%d.ogg", i);
	RadioSquelch = L("SFX\\Radio\\squelch.ogg"); RadioStatic = L("SFX\\Radio\\static.ogg"); RadioBuzz = L("SFX\\Radio\\buzz.ogg");
	ElevatorBeepSFX = L("SFX\\General\\Elevator\\Beep.ogg"); ElevatorMoveSFX = L("SFX\\General\\Elevator\\Moving.ogg");
	for (int i = 0; i <= 3; i++) PickSFX[i] = LN("SFX\\Interact\\PickItem%d.ogg", i);
	static const int amb[] = {11, 11, 12, 15, 5, 10};
	for (int i = 0; i < 6; i++) AmbientSFXAmount[i] = amb[i];
	for (int i = 0; i <= 2; i++) OldManSFX[i] = LN("SFX\\SCP\\106\\Corrosion%d.ogg", i + 1);
	OldManSFX[3] = L("SFX\\SCP\\106\\Laugh.ogg"); OldManSFX[4] = L("SFX\\SCP\\106\\Breathing.ogg");
	OldManSFX[5] = L("SFX\\Room\\PocketDimension\\Enter.ogg");
	for (int i = 0; i <= 2; i++) OldManSFX[6 + i] = LN("SFX\\SCP\\106\\WallDecay%d.ogg", i + 1);
	for (int i = 0; i <= 2; i++) Scp173SFX[i] = LN("SFX\\SCP\\173\\Rattle%d.ogg", i + 1);
	for (int i = 0; i <= 11; i++) HorrorSFX[i] = LN("SFX\\Horror\\Horror%d.ogg", i);
	for (int i = 14; i <= 15; i++) HorrorSFX[i] = LN("SFX\\Horror\\Horror%d.ogg", i);
	for (int i = 7; i <= 9; i++) IntroSFX[i] = LN("SFX\\Room\\Intro\\Bang%d.ogg", i - 6);
	for (int i = 10; i <= 12; i++) IntroSFX[i] = LN("SFX\\Room\\Intro\\Light%d.ogg", i - 9);
	IntroSFX[15] = L("SFX\\Room\\Intro\\173Vent.ogg");
	AlarmSFX[0] = L("SFX\\Alarm\\Alarm.ogg"); AlarmSFX[2] = L("SFX\\Alarm\\Alarm3.ogg");
	AlarmSFX[3] = L("SFX\\Alarm\\Alarm4.ogg"); AlarmSFX[4] = L("SFX\\Alarm\\Alarm5.ogg");
	HeartBeatSFX = L("SFX\\Character\\D9341\\Heartbeat.ogg");
	for (int i = 0; i <= 4; i++) {
		BreathSFX[0][i] = LN("SFX\\Character\\D9341\\breath%d.ogg", i);
		BreathSFX[1][i] = LN("SFX\\Character\\D9341\\breath%dgas.ogg", i);
	}
	for (int i = 0; i <= 2; i++) NeckSnapSFX[i] = LN("SFX\\SCP\\173\\NeckSnap%d.ogg", i + 1);
	for (int i = 0; i <= 8; i++) DamageSFX[i] = LN("SFX\\Character\\D9341\\Damage%d.ogg", i + 1);
	for (int i = 0; i <= 2; i++) CoughSFX[i] = LN("SFX\\Character\\D9341\\Cough%d.ogg", i + 1);
	MachineSFX = L("SFX\\SCP\\914\\Refining.ogg");
	ApacheSFX = L("SFX\\Character\\Apache\\Propeller.ogg");
	for (int i = 0; i <= 7; i++) {
		StepSFX[0][0][i] = LN("SFX\\Step\\Step%d.ogg", i + 1);
		StepSFX[1][0][i] = LN("SFX\\Step\\StepMetal%d.ogg", i + 1);
		StepSFX[0][1][i] = LN("SFX\\Step\\Run%d.ogg", i + 1);
		StepSFX[1][1][i] = LN("SFX\\Step\\RunMetal%d.ogg", i + 1);
		if (i < 3) {
			StepSFX[2][0][i] = LN("SFX\\Character\\MTF\\Step%d.ogg", i + 1);
			StepSFX[3][0][i] = LN("SFX\\SCP\\049\\Step%d.ogg", i + 1);
		}
		if (i < 4) StepSFX[4][0][i] = LN("SFX\\Step\\SCP\\StepSCP%d.ogg", i + 1);
	}
	for (int i = 0; i <= 2; i++) {
		Step2SFX[i] = LN("SFX\\Step\\StepPD%d.ogg", i + 1);
		Step2SFX[i + 3] = LN("SFX\\Step\\StepForest%d.ogg", i + 1);
	}
}

// ---- the game's own "3d" sound: volume by distance, pan by DeltaYaw
static int audible(ent cam, ent e, float range, float *dist, float *pan) {
	*dist = bbEntityDistance(cam, e) / range;
	*pan = sinf(-bbDeltaYaw(cam, e) * DTOR);
	return 1 - *dist > 0 && 1 - *dist < 1;
}

int PlaySound2(int snd, ent cam, ent e, float range, float volume) {
	range = Max(range, 1);
	float dist, pan;
	if (volume <= 0 || !audible(cam, e, range, &dist, &pan)) return 0;
	int ch = PlaySound_Strict(snd);
	bbChannelVolume(ch, volume * (1 - dist) * SFXVolume);
	bbChannelPan(ch, pan);
	return ch;
}

int LoopSound2(int snd, int chn, ent cam, ent e, float range, float volume) {
	range = Max(range, 1);
	if (volume <= 0) { if (chn) bbChannelVolume(chn, 0); return chn; }
	float dist, pan;
	audible(cam, e, range, &dist, &pan); // blitz commented the range check out here
	if (!chn || !bbChannelPlaying(chn)) chn = PlaySound_Strict(snd);
	bbChannelVolume(chn, volume * (1 - dist) * SFXVolume);
	bbChannelPan(chn, pan);
	return chn;
}

static void origin(int chn, ent cam, ent e, float range, float volume, float mul) {
	range = Max(range, 1);
	if (volume <= 0) { if (chn) bbChannelVolume(chn, 0); return; }
	float dist, pan;
	if (!audible(cam, e, range, &dist, &pan)) return;
	bbChannelVolume(chn, volume * (1 - dist) * mul);
	bbChannelPan(chn, pan);
}
void UpdateSoundOrigin(int chn, ent cam, ent e, float range, float volume) { origin(chn, cam, e, range, volume, SFXVolume); }
void UpdateSoundOrigin2(int chn, ent cam, ent e, float range, float volume) { origin(chn, cam, e, range, volume, 1); }

// floor material under e -> step sound set (Data/materials.ini "stepsound"), checks texture layers 3, 2, 1
int GetStepSound(ent e) {
	ent pk = bbLinePick(bbEntityX(e, 0), bbEntityY(e, 0), bbEntityZ(e, 0), 0, -1, 0, 0);
	if (!pk || bbGetEntityType(pk) != HIT_MAP) return 0;
	hbrush b = bbGetSurfaceBrush(bbGetSurface(pk, bbCountSurfaces(pk)));
	if (!b) return 0;
	for (int l = 3; l >= 1; l--) {
		htex t = bbGetBrushTexture(b, l);
		if (!t) continue;
		str name = StripPath(bbTextureName(t));
		if (bbLen(name)) bbFreeTexture(t);
		if (l == 1) bbFreeBrush(b);
		each(Materials, m) if (bbSeq(m->name, name)) {
			if (m->StepSound > 0) { if (l != 1) bbFreeBrush(b); return m->StepSound - 1; }
			break;
		}
		if (l == 1) return 0;
	}
	return 0; // blitz leaks the brush when no layer has a texture; so do we
}
