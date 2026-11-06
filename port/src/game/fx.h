// fx.h: Particles.bb (particles, emitters), Main.bb decals, MapSystem.bb security cams + elevator objs
#pragma once

typedef struct Particles { lnode n; int obj, pvt, image; float R, G, B, A, size, speed, yspeed, gravity, Rchange, Gchange, Bchange, Achange, SizeChange, lifetime; } Particles;
typedef struct Emitters { lnode n; int Obj; float Size; int MinImage, MaxImage; float Gravity; int LifeTime, Disable; Rooms *Room; int SoundCHN; float Speed, RandAngle, SizeChange, Achange; } Emitters;
typedef struct DevilEmitters { lnode n; int obj; float x, y, z; int particleID; Rooms *room; float timer, maxtimer; int SoundCHN, isDeconGas; } DevilEmitters;
typedef struct Decals { lnode n; int obj; float SizeChange, Size, MaxSize, AlphaChange, Alpha; int blendmode, fx, ID; float Timer, lifetime, x, y, z, pitch, yaw, roll; } Decals;
typedef struct SecurityCams {
	lnode n;
	int obj, MonitorObj, BaseObj, CameraObj, ScrObj; float ScrWidth, ScrHeight;
	int Screen, Cam, ScrTexture, ScrOverlay; float angle, turn, CurrAngle, State; int PlayerState, soundCHN, InSight;
	float RenderInterval; Rooms *room; int FollowPlayer, CoffinEffect, AllowSaving; float MinAngle, MaxAngle; int dir;
} SecurityCams;
typedef struct ElevatorObj { lnode n; int obj, InFacility; Doors *door; } ElevatorObj;
TLIST_DECL(Particles); TLIST_DECL(Emitters); TLIST_DECL(DevilEmitters); TLIST_DECL(Decals); TLIST_DECL(SecurityCams); TLIST_DECL(ElevatorObj);

G(int, ParticleTextures[11], InSmoke, HissSFX);
G(float, SmokeDelay);
G(SecurityCams *, SelectedMonitor);
G(SecurityCams *, CoffinCam);
G(int, CurrRoom2slRenderCam);

Particles *CreateParticle(float x, float y, float z, int image, float size, float gravity, int lifetime);
void UpdateParticles(void);
void RemoveParticle(Particles *p);
Emitters *CreateEmitter(float x, float y, float z, int type);
DevilEmitters *CreateDevilEmitter(float x, float y, float z, Rooms *room, int particleID, float maxTime);
Decals *CreateDecal(int id, float x, float y, float z, float pitch, float yaw, float roll);
SecurityCams *CreateSecurityCam(float x, float y, float z, Rooms *r, int screen);
ElevatorObj *AssignElevatorObj(int obj, Doors *door, int in_facility);
void LoadParticleTextures(void);
