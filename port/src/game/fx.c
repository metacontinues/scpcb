// fx.c: particles, emitters, decals, security cams, elevator objs (the Update* fns come w/ the game loop)
#include "g.h"

TLIST(Particles); TLIST(Emitters); TLIST(DevilEmitters); TLIST(Decals); TLIST(SecurityCams); TLIST(ElevatorObj);

void LoadParticleTextures(void) {
	static const char *f[] = {"GFX\\smoke.png", "GFX\\flash.jpg", "GFX\\dust.jpg", "GFX\\npcs\\hg.pt", "GFX\\map\\sun.jpg",
		"GFX\\bloodsprite.png", "GFX\\smoke2.png", "GFX\\spark.jpg", "GFX\\particle.png"};
	for (int i = 0; i < 9; i++) ParticleTextures[i] = LoadTexture_Strict(f[i], 1 + 2);
}

Particles *CreateParticle(float x, float y, float z, int image, float size, float gravity, int lifetime) {
	Particles *p = lnew(Particles);
	p->lifetime = lifetime;
	p->obj = create_sprite(0);
	position_entity(p->obj, x, y, z, 1);
	entity_texture(p->obj, ParticleTextures[image], 0, 0);
	rotate_entity(p->obj, 0, 0, frnd(0, 360), 0); // blitz Rnd(x) is Rnd(x, 0)
	entity_fx(p->obj, 1 + 8);
	sprite_view_mode(p->obj, 3);
	if (image == 0 || image == 5 || image == 6) entity_blend(p->obj, 1);
	else if (image >= 1 && image <= 7) entity_blend(p->obj, BL_ADD);
	p->pvt = create_pivot(0);
	position_entity(p->pvt, x, y, z, 1);
	p->image = image;
	p->gravity = gravity * .004f;
	p->R = p->G = p->B = 255; p->A = 1;
	p->size = size;
	scale_sprite(p->obj, size, size);
	return p;
}

void UpdateParticles(void) {
	each(Particles, p) {
		move_entity(p->pvt, 0, 0, p->speed * FPSfactor);
		if (p->gravity != 0) p->yspeed -= p->gravity * FPSfactor;
		translate_entity(p->pvt, 0, p->yspeed * FPSfactor, 0, 1);
		position_entity(p->obj, entity_x(p->pvt, 1), entity_y(p->pvt, 1), entity_z(p->pvt, 1), 1);
		if (p->Achange != 0) { p->A = Min(Max(p->A + p->Achange * FPSfactor, 0), 1); entity_alpha(p->obj, p->A); }
		if (p->SizeChange != 0) { p->size += p->SizeChange * FPSfactor; scale_sprite(p->obj, p->size, p->size); }
		p->lifetime -= FPSfactor;
		if (p->lifetime <= 0 || p->size < .00001f || p->A <= 0) RemoveParticle(p);
	}
}

void RemoveParticle(Particles *p) { free_entity(p->obj); free_entity(p->pvt); ldel(p); }

Emitters *CreateEmitter(float x, float y, float z, int type) {
	Emitters *e = lnew(Emitters);
	e->Obj = create_pivot(0);
	name_entity(e->Obj, "Emitter1");
	position_entity(e->Obj, x, y, z, 1);
	if (type == 0) { e->Size = .03f; e->Gravity = -.2f; e->LifeTime = 200; e->SizeChange = .005f; e->Speed = .004f; e->RandAngle = 20; e->Achange = -.008f; }
	else if (type == 1) { e->Size = .03f; e->Gravity = -.2f; e->LifeTime = 200; e->SizeChange = .008f; e->Speed = .004f; e->RandAngle = 40; e->Achange = -.01f; e->MinImage = e->MaxImage = 6; }
	each(Rooms, r) if (fabsf(entity_x(e->Obj, 0) - entity_x(r->obj, 0)) < 4 && fabsf(entity_z(e->Obj, 0) - entity_z(r->obj, 0)) < 4) e->Room = r;
	return e;
}

DevilEmitters *CreateDevilEmitter(float x, float y, float z, Rooms *room, int particleID, float maxTime) {
	DevilEmitters *d = lnew(DevilEmitters);
	d->obj = create_pivot(0);
	name_entity(d->obj, "Emitter2");
	position_entity(d->obj, x, y, z, 1);
	entity_parent(d->obj, room->obj, 1);
	d->room = room; d->x = x; d->y = y; d->z = z;
	d->particleID = particleID; d->maxtimer = maxTime;
	return d;
}

Decals *CreateDecal(int id, float x, float y, float z, float pitch, float yaw, float roll) {
	Decals *d = lnew(Decals);
	d->x = x; d->y = y; d->z = z; d->pitch = pitch; d->yaw = yaw; d->roll = roll;
	d->MaxSize = 1; d->Alpha = 1; d->Size = 1;
	d->obj = create_sprite(0);
	d->blendmode = 1;
	entity_texture(d->obj, DecalTextures[id], 0, 0);
	entity_fx(d->obj, 0);
	sprite_view_mode(d->obj, 2);
	position_entity(d->obj, x, y, z, 0);
	rotate_entity(d->obj, pitch, yaw, roll, 0);
	d->ID = id;
	return DecalTextures[id] && d->obj ? d : NULL; // blitz returns Null but keeps the decal in the list
}

SecurityCams *CreateSecurityCam(float x, float y, float z, Rooms *r, int screen) {
	SecurityCams *sc = lnew(SecurityCams);
	sc->obj = copy_entity(CamBaseOBJ, 0);
	scale_entity(sc->obj, .0015f, .0015f, .0015f, 0);
	sc->CameraObj = copy_entity(CamOBJ, 0);
	scale_entity(sc->CameraObj, .01f, .01f, .01f, 0);
	sc->room = r;
	sc->Screen = screen;
	if (screen) {
		sc->AllowSaving = 1;
		sc->RenderInterval = 12;
		float s = RoomScale * 4.5f * .4f, w = mesh_width(Monitor) * s * .95f * .5f, h = mesh_height(Monitor) * s * .95f * .5f;
		sc->ScrObj = create_sprite(0);
		entity_fx(sc->ScrObj, 17);
		sprite_view_mode(sc->ScrObj, 2);
		sc->ScrTexture = 0;
		entity_texture(sc->ScrObj, ScreenTexs[0], 0, 0);
		scale_sprite(sc->ScrObj, w, h);
		sc->ScrOverlay = create_sprite(sc->ScrObj);
		scale_sprite(sc->ScrOverlay, w, h);
		move_entity(sc->ScrOverlay, 0, 0, -.0005f);
		entity_texture(sc->ScrOverlay, MonitorTexture, 0, 0);
		sprite_view_mode(sc->ScrOverlay, 2);
		entity_blend(sc->ScrOverlay, 3);
		sc->MonitorObj = copy_entity(Monitor, sc->ScrObj);
		scale_entity(sc->MonitorObj, s, s, s, 0);
		sc->Cam = create_camera(0);
		camera_viewport(sc->Cam, 0, 0, 512, 512);
		camera_range(sc->Cam, .05f, 8);
		camera_zoom(sc->Cam, .8f);
		hide_entity(sc->Cam);
	}
	position_entity(sc->obj, x, y, z, 0);
	if (r) entity_parent(sc->obj, r->obj, 1);
	return sc;
}

ElevatorObj *AssignElevatorObj(int obj, Doors *door, int in_facility) {
	ElevatorObj *eo = lnew(ElevatorObj);
	eo->obj = obj; eo->door = door; eo->InFacility = in_facility;
	return eo;
}

// Main.bb's UpdateDecals: growing decals (106's corrosion spawns more around itself), fades, lifetimes
void UpdateDecals(void) {
	each(Decals, d) {
		if (d->SizeChange != 0) {
			d->Size += d->SizeChange * FPSfactor;
			scale_sprite(d->obj, d->Size, d->Size);
			if (d->ID == 0) {
				if (d->Timer <= 0) {
					float a = irand(1, 360), t = frnd(0, d->Size);
					float yaw = frnd(0, 360); // the decal's only rng arg
					Decals *d2 = CreateDecal(1, entity_x(d->obj, 0) + cosf(a * DTOR) * t, entity_y(d->obj, 0) - .0005f, entity_z(d->obj, 0) + sinf(a * DTOR) * t, entity_pitch(d->obj, 0), yaw, entity_roll(d->obj, 0));
					d2 = d2 ? d2 : llast(Decals); // blitz gets the decal back even when its texture is missing
					d2->Size = frnd(.1f, .5f); scale_sprite(d2->obj, d2->Size, d2->Size);
					int s = DecaySFX[irand(1, 3)];
					PlaySound2(s, Camera, d2->obj, 10, frnd(.1f, .5f));
					d->Timer = irand(50, 100);
				} else d->Timer -= FPSfactor;
			}
			if (d->Size >= d->MaxSize) { d->SizeChange = 0; d->Size = d->MaxSize; }
		}
		if (d->AlphaChange != 0) { d->Alpha = Min(d->Alpha + FPSfactor * d->AlphaChange, 1); entity_alpha(d->obj, d->Alpha); }
		if (d->lifetime > 0) d->lifetime = Max(d->lifetime - FPSfactor, 5);
		if (d->Size <= 0 || d->Alpha <= 0 || d->lifetime == 5) { free_entity(d->obj); ldel(d); }
	}
}

void RemoveSecurityCam(SecurityCams *sc) {
	if (SelectedMonitor == sc) SelectedMonitor = NULL;
	if (CoffinCam == sc) CoffinCam = NULL;
	ldel(sc);
}
