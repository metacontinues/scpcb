// fx.c: particles, emitters, decals, security cams, elevator objs (the Update* fns come w/ the game loop)
#include "g.h"

TLIST(Particles); TLIST(Emitters); TLIST(DevilEmitters); TLIST(Decals); TLIST(SecurityCams); TLIST(ElevatorObj);

void LoadParticleTextures(void) {
	static const char *f[] = {"GFX\\smoke.png", "GFX\\flash.jpg", "GFX\\dust.jpg", "GFX\\npcs\\hg.pt", "GFX\\map\\sun.jpg",
		"GFX\\bloodsprite.png", "GFX\\smoke2.png", "GFX\\spark.jpg", "GFX\\particle.png"};
	for (int i = 0; i < 9; i++) ParticleTextures[i] = LoadTexture_Strict(f[i], 1 + 2);
}

Particles *CreateParticle(float x, float y, float z, int image, float size, float gravity, int lifetime) {
	Particles *p = bbNew(Particles);
	p->lifetime = lifetime;
	p->obj = bbCreateSprite(0);
	bbPositionEntity(p->obj, x, y, z, 1);
	bbEntityTexture(p->obj, ParticleTextures[image], 0, 0);
	bbRotateEntity(p->obj, 0, 0, bbRnd(360, 0), 0); // blitz Rnd(x) is Rnd(x, 0)
	bbEntityFX(p->obj, 1 + 8);
	bbSpriteViewMode(p->obj, 3);
	if (image == 0 || image == 5 || image == 6) bbEntityBlend(p->obj, 1);
	else if (image >= 1 && image <= 7) bbEntityBlend(p->obj, BL_ADD);
	p->pvt = bbCreatePivot(0);
	bbPositionEntity(p->pvt, x, y, z, 1);
	p->image = image;
	p->gravity = gravity * .004f;
	p->R = p->G = p->B = 255; p->A = 1;
	p->size = size;
	bbScaleSprite(p->obj, size, size);
	return p;
}

void UpdateParticles(void) {
	each(Particles, p) {
		bbMoveEntity(p->pvt, 0, 0, p->speed * FPSfactor);
		if (p->gravity != 0) p->yspeed -= p->gravity * FPSfactor;
		bbTranslateEntity(p->pvt, 0, p->yspeed * FPSfactor, 0, 1);
		bbPositionEntity(p->obj, bbEntityX(p->pvt, 1), bbEntityY(p->pvt, 1), bbEntityZ(p->pvt, 1), 1);
		if (p->Achange != 0) { p->A = Min(Max(p->A + p->Achange * FPSfactor, 0), 1); bbEntityAlpha(p->obj, p->A); }
		if (p->SizeChange != 0) { p->size += p->SizeChange * FPSfactor; bbScaleSprite(p->obj, p->size, p->size); }
		p->lifetime -= FPSfactor;
		if (p->lifetime <= 0 || p->size < .00001f || p->A <= 0) RemoveParticle(p);
	}
}

void RemoveParticle(Particles *p) { bbFreeEntity(p->obj); bbFreeEntity(p->pvt); bbDelete(p); }

Emitters *CreateEmitter(float x, float y, float z, int type) {
	Emitters *e = bbNew(Emitters);
	e->Obj = bbCreatePivot(0);
	bbNameEntity(e->Obj, "Emitter1");
	bbPositionEntity(e->Obj, x, y, z, 1);
	if (type == 0) { e->Size = .03f; e->Gravity = -.2f; e->LifeTime = 200; e->SizeChange = .005f; e->Speed = .004f; e->RandAngle = 20; e->Achange = -.008f; }
	else if (type == 1) { e->Size = .03f; e->Gravity = -.2f; e->LifeTime = 200; e->SizeChange = .008f; e->Speed = .004f; e->RandAngle = 40; e->Achange = -.01f; e->MinImage = e->MaxImage = 6; }
	each(Rooms, r) if (fabsf(bbEntityX(e->Obj, 0) - bbEntityX(r->obj, 0)) < 4 && fabsf(bbEntityZ(e->Obj, 0) - bbEntityZ(r->obj, 0)) < 4) e->Room = r;
	return e;
}

DevilEmitters *CreateDevilEmitter(float x, float y, float z, Rooms *room, int particleID, float maxTime) {
	DevilEmitters *d = bbNew(DevilEmitters);
	d->obj = bbCreatePivot(0);
	bbNameEntity(d->obj, "Emitter2");
	bbPositionEntity(d->obj, x, y, z, 1);
	bbEntityParent(d->obj, room->obj, 1);
	d->room = room; d->x = x; d->y = y; d->z = z;
	d->particleID = particleID; d->maxtimer = maxTime;
	return d;
}

Decals *CreateDecal(int id, float x, float y, float z, float pitch, float yaw, float roll) {
	Decals *d = bbNew(Decals);
	d->x = x; d->y = y; d->z = z; d->pitch = pitch; d->yaw = yaw; d->roll = roll;
	d->MaxSize = 1; d->Alpha = 1; d->Size = 1;
	d->obj = bbCreateSprite(0);
	d->blendmode = 1;
	bbEntityTexture(d->obj, DecalTextures[id], 0, 0);
	bbEntityFX(d->obj, 0);
	bbSpriteViewMode(d->obj, 2);
	bbPositionEntity(d->obj, x, y, z, 0);
	bbRotateEntity(d->obj, pitch, yaw, roll, 0);
	d->ID = id;
	return DecalTextures[id] && d->obj ? d : NULL; // blitz returns Null but keeps the decal in the list
}

SecurityCams *CreateSecurityCam(float x, float y, float z, Rooms *r, int screen) {
	SecurityCams *sc = bbNew(SecurityCams);
	sc->obj = bbCopyEntity(CamBaseOBJ, 0);
	bbScaleEntity(sc->obj, .0015f, .0015f, .0015f, 0);
	sc->CameraObj = bbCopyEntity(CamOBJ, 0);
	bbScaleEntity(sc->CameraObj, .01f, .01f, .01f, 0);
	sc->room = r;
	sc->Screen = screen;
	if (screen) {
		sc->AllowSaving = 1;
		sc->RenderInterval = 12;
		float s = RoomScale * 4.5f * .4f, w = bbMeshWidth(Monitor) * s * .95f * .5f, h = bbMeshHeight(Monitor) * s * .95f * .5f;
		sc->ScrObj = bbCreateSprite(0);
		bbEntityFX(sc->ScrObj, 17);
		bbSpriteViewMode(sc->ScrObj, 2);
		sc->ScrTexture = 0;
		bbEntityTexture(sc->ScrObj, ScreenTexs[0], 0, 0);
		bbScaleSprite(sc->ScrObj, w, h);
		sc->ScrOverlay = bbCreateSprite(sc->ScrObj);
		bbScaleSprite(sc->ScrOverlay, w, h);
		bbMoveEntity(sc->ScrOverlay, 0, 0, -.0005f);
		bbEntityTexture(sc->ScrOverlay, MonitorTexture, 0, 0);
		bbSpriteViewMode(sc->ScrOverlay, 2);
		bbEntityBlend(sc->ScrOverlay, 3);
		sc->MonitorObj = bbCopyEntity(Monitor, sc->ScrObj);
		bbScaleEntity(sc->MonitorObj, s, s, s, 0);
		sc->Cam = bbCreateCamera(0);
		bbCameraViewport(sc->Cam, 0, 0, 512, 512);
		bbCameraRange(sc->Cam, .05f, 8);
		bbCameraZoom(sc->Cam, .8f);
		bbHideEntity(sc->Cam);
	}
	bbPositionEntity(sc->obj, x, y, z, 0);
	if (r) bbEntityParent(sc->obj, r->obj, 1);
	return sc;
}

ElevatorObj *AssignElevatorObj(int obj, Doors *door, int in_facility) {
	ElevatorObj *eo = bbNew(ElevatorObj);
	eo->obj = obj; eo->door = door; eo->InFacility = in_facility;
	return eo;
}
