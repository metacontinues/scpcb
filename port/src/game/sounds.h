// sounds.h: Main.bb's sound globals + LoadAllSounds.bb. included from g.h
#pragma once

G(int, OpenDoorSFX[4][4], CloseDoorSFX[4][4], KeyCardSFX1, KeyCardSFX2, ButtonSFX2, ScannerSFX1, ScannerSFX2, OpenDoorFastSFX);
G(int, CautionSFX, NuclearSirenSFX, CameraSFX, StoneDragSFX, GunshotSFX, Gunshot2SFX, Gunshot3SFX, BullethitSFX);
G(int, TeslaIdleSFX, TeslaActivateSFX, TeslaPowerUpSFX, MagnetUpSFX, MagnetDownSFX, FemurBreakerSFX);
G(int, DecaySFX[6], BurstSFX, RustleSFX[4], Use914SFX, Death914SFX, DripSFX[5], LeverSFX, LightSFX, ButtGhostSFX);
G(int, RadioSFX[6][11], RadioSquelch, RadioStatic, RadioBuzz, ElevatorBeepSFX, ElevatorMoveSFX, PickSFX[11]);
G(int, AmbientSFXCHN, CurrAmbientSFX, AmbientSFXAmount[7], AmbientSFX[7][16], OldManSFX[9], Scp173SFX[4], HorrorSFX[21]);
G(int, IntroSFX[21], AlarmSFX[6], CommotionState[26], HeartBeatSFX, VomitSFX, BreathSFX[3][6], BreathCHN, NeckSnapSFX[4]);
G(int, DamageSFX[10], MTFSFX[9], CoughSFX[4], CoughCHN, VomitCHN, MachineSFX, ApacheSFX, CurrStepSFX, StepSFX[6][3][9], Step2SFX[7]);

void LoadAllSounds(void);
int PlaySound2(int snd, ent cam, ent e, float range, float volume);
int LoopSound2(int snd, int chn, ent cam, ent e, float range, float volume);
void UpdateSoundOrigin(int chn, ent cam, ent e, float range, float volume);
void UpdateSoundOrigin2(int chn, ent cam, ent e, float range, float volume);
int GetStepSound(ent e);
