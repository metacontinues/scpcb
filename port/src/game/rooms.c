// rooms.c: MapSystem.bb's materials, rmesh glue, room templates, CreateRoom, FillRoom (generic part), lights
#include "g.h"
#include "../fmt/rmesh.h"

static void matfree(Materials *m) { free(m->name); }
static void rtfree(RoomTemplates *t) { free(t->objPath); free(t->Name); for (int i = 0; i < 129; i++) free(t->TempTriggerboxName[i]); }
static void roomfree(Rooms *r) { for (int i = 0; i < 129; i++) free(r->TriggerboxName[i]); }
static void scrfree(Screens *s) { free(s->imgpath); }
static void tscrfree(TempScreens *s) { free(s->imgpath); }
TLIST_DTOR(Materials, matfree); TLIST_DTOR(RoomTemplates, rtfree); TLIST_DTOR(Rooms, roomfree); TLIST(LightTemplates);
TLIST(TempWayPoints); TLIST(WayPoints); TLIST_DTOR(Screens, scrfree); TLIST_DTOR(TempScreens, tscrfree); TLIST(MapZones);

// ---- strings
str StripPath(const char *f) {
	int l = s_len(f), i = l;
	while (i > 0 && f[i - 1] != '\\' && f[i - 1] != '/') i--;
	return s_right(f, l - i);
}
str StripFilename(const char *f) {
	int last = 0;
	for (int i = 1; i <= s_len(f); i++) if (f[i - 1] == '\\' || f[i - 1] == '/') last = i;
	return s_left(f, last);
}

float WrapAngle(float a) {
	if (isinf(a)) return 0;
	while (a < 0) a += 360;
	while (a >= 360) a -= 360;
	return a;
}

void GetMeshExtents(int mesh) {
	float lo[3] = {INFINITY, INFINITY, INFINITY}, hi[3] = {-INFINITY, -INFINITY, -INFINITY};
	for (int s = 0; s < count_surfaces(mesh); s++) {
		surf *sf = get_surface(mesh, s);
		for (int v = 0; v < count_vertices(sf); v++) {
			float p[3] = {vertex_x(sf, v), vertex_y(sf, v), vertex_z(sf, v)};
			for (int k = 0; k < 3; k++) { if (p[k] < lo[k]) lo[k] = p[k]; if (p[k] > hi[k]) hi[k] = p[k]; }
		}
	}
	Mesh_MinX = lo[0]; Mesh_MinY = lo[1]; Mesh_MinZ = lo[2];
	Mesh_MaxX = hi[0]; Mesh_MaxY = hi[1]; Mesh_MaxZ = hi[2];
	Mesh_MagX = hi[0] - lo[0]; Mesh_MagY = hi[1] - lo[1]; Mesh_MagZ = hi[2] - lo[2];
}

// ---- materials / texture cache. bump maps are skipped (no dot3 in the renderer yet)
void LoadMaterials_(const char *file) {
	FILE *f = open_file(file);
	while (f && !file_eof(f)) {
		str t = s_trim(read_line(f));
		if (!s_eq(s_left(t, 1), "[")) continue;
		t = s_mid(t, 1, s_len(t) - 2);
		Materials *m = lnew(Materials);
		sset(&m->name, s_lower(t));
		// TODO BumpEnabled: "bump" textures w/ TextureBlend 6 + TextureBumpEnv*
		m->StepSound = GetINIInt(file, t, "stepsound", 0) + 1;
	}
	if (f) close_file(f);
}
Materials *GetCache(const char *name) { each(Materials, tc) if (s_eq(tc->name, name)) return tc; return NULL; }
int GetTextureFromCache(const char *name) { Materials *m = GetCache(name); return m ? m->Diff : 0; }
int GetBumpFromCache(const char *name) { Materials *m = GetCache(name); return m ? m->Bump : 0; }
void AddTextureToCache(int tex) {
	str n = StripPath(texture_name(tex));
	Materials *tc = GetCache(n);
	if (!tc) { tc = lnew(Materials); sset(&tc->name, n); }
	if (!tc->Diff) tc->Diff = tex;
}
void ClearTextureCache(void) {
	each(Materials, tc) { if (tc->Diff) free_texture(tc->Diff); if (tc->Bump) free_texture(tc->Bump); ldel(tc); }
}
void FreeTextureCache(void) {
	each(Materials, tc) { if (tc->Diff) free_texture(tc->Diff); if (tc->Bump) free_texture(tc->Bump); tc->Diff = tc->Bump = 0; }
}

// ---- LoadRMesh: geometry comes from fmt/rmesh.c, the point entities turn into the game's templates here
int LoadRMesh(const char *file, RoomTemplates *rt) {
	rmesh rm;
	if (!rmesh_load(file, AmbientLightRoomTex, &rm)) return 0;
	// the room's textures go in the cache, like blitz's GetTextureFromCache/AddTextureToCache did
	ent op = get_child(rm.obj, 1);
	for (int s = 0; s < count_surfaces(op); s++) {
		hbrush b = get_surface_brush(get_surface(op, s));
		for (int l = 0; l < 2; l++) { htex t = get_brush_texture(b, l); if (t && t != AmbientLightRoomTex) AddTextureToCache(t); }
		free_brush(b);
	}
	for (int i = 0; i < rm.npts; i++) {
		rpoint *p = &rm.pts[i];
		int nz = p->x != 0 || p->y != 0 || p->z != 0;
		switch (p->kind) {
		case RP_SCREEN:
			if (!nz || !rt) break;
			{ TempScreens *ts = lnew(TempScreens); ts->x = p->x; ts->y = p->y; ts->z = p->z; sset(&ts->imgpath, s_dup(p->str)); ts->roomtemplate = rt; }
			break;
		case RP_WAYPOINT:
			{ TempWayPoints *w = lnew(TempWayPoints); w->roomtemplate = rt; w->x = p->x; w->y = p->y; w->z = p->z; }
			break;
		case RP_LIGHT: case RP_SPOT:
			if (!nz) break;
			{
				LightTemplates *lt = AddTempLight(rt, p->x, p->y, p->z, 2, p->range, p->r, p->g, p->b);
				if (p->kind == RP_SPOT) { lt->pitch = p->pitch; lt->yaw = p->yaw; lt->innerconeangle = p->inner; lt->outerconeangle = p->outer; }
			}
			break;
		case RP_SOUND:
			if (!rt) break;
			for (int j = 0; j < MaxRoomEmitters; j++) if (!rt->TempSoundEmitter[j]) {
				rt->TempSoundEmitterX[j] = p->x; rt->TempSoundEmitterY[j] = p->y; rt->TempSoundEmitterZ[j] = p->z;
				rt->TempSoundEmitter[j] = p->snd; rt->TempSoundEmitterRange[j] = p->range;
				break;
			}
			break;
		default: break; // playerstart only moved a camera that's never set; models are already parented
		}
	}
	// blitz made the trigger boxes as root meshes (rt\obj was still 0 then), so not part of the room copy.
	// CheckTriggers zeroes their alpha in game; here they start invisible
	if (rt) {
		rt->TempTriggerboxAmount = rm.ntrig;
		for (int t = 0; t < rm.ntrig; t++) {
			entity_parent(rm.trig[t], 0, 1);
			entity_alpha(rm.trig[t], 0);
			rt->TempTriggerbox[t] = rm.trig[t];
			sset(&rt->TempTriggerboxName[t], s_dup(rm.trigname[t]));
		}
	}
	int obj = rm.obj;
	free(rm.pts);
	return obj;
}

// ---- room templates
RoomTemplates *CreateRoomTemplate(const char *meshpath) {
	RoomTemplates *rt = lnew(RoomTemplates);
	sset(&rt->objPath, s_dup(meshpath));
	rt->id = RoomTempID++;
	rt->DisableOverlapCheck = 1;
	return rt;
}

void LoadRoomTemplates(const char *file) {
	FILE *f = open_file(file);
	while (f && !file_eof(f)) {
		str t = s_trim(read_line(f));
		if (!s_eq(s_left(t, 1), "[")) continue;
		t = s_mid(t, 1, s_len(t) - 2);
		RoomTemplates *rt = CreateRoomTemplate(GetINIString(file, t, "mesh path", ""));
		sset(&rt->Name, s_lower(t));
		str s = s_lower(GetINIString(file, t, "shape", ""));
		static const char *sh[][2] = {{"room1", "1"}, {"room2", "2"}, {"room2c", "2c"}, {"room3", "3"}, {"room4", "4"}};
		for (int i = 0; i < 5; i++) if (s_eq(s, sh[i][0]) || s_eq(s, sh[i][1])) rt->Shape = ROOM1 + i;
		for (int i = 0; i <= 4; i++) rt->zone[i] = GetINIInt(file, t, cat("zone", s_str(i + 1)), 0);
		rt->Commonness = Max(Min(GetINIInt(file, t, "commonness", 0), 100), 0);
		rt->Large = GetINIInt(file, t, "large", 0);
		rt->DisableDecals = GetINIInt(file, t, "disabledecals", 0);
		rt->UseLightCones = GetINIInt(file, t, "usevolumelighting", 0);
		rt->DisableOverlapCheck = GetINIInt(file, t, "disableoverlapcheck", 0);
	}
	for (int i = 1;; i++) {
		str s = GetINIString(file, "room ambience", cat("ambience", s_str(i)), "");
		if (!s_len(s)) break;
		RoomAmbience[i] = LoadSound_Strict(s);
	}
	if (f) close_file(f);
}

// .b3d rooms (LoadWorld) don't show up in rooms.ini, so only rmesh + empty paths are handled
void LoadRoomMesh(RoomTemplates *rt) {
	if (s_has(rt->objPath, ".rmesh")) rt->obj = LoadRMesh(rt->objPath, rt);
	else if (s_len(rt->objPath)) RuntimeError(cat("LoadWorld isn't ported: ", rt->objPath));
	else rt->obj = create_pivot(0);
	if (!rt->obj) RuntimeError(cat("Failed to load map file \"", rt->objPath, "\"."));
	CalculateRoomTemplateExtents(rt);
	hide_entity(rt->obj);
}

static Rooms *place(Rooms *r, RoomTemplates *rt, float x, float y, float z) {
	r->RoomTemplate = rt;
	if (!rt->obj) LoadRoomMesh(rt);
	r->obj = copy_entity(rt->obj, 0);
	scale_entity(r->obj, RoomScale, RoomScale, RoomScale, 0);
	entity_type(r->obj, HIT_MAP, 0);
	entity_pick_mode(r->obj, 2, 1);
	position_entity(r->obj, x, y, z, 0);
	FillRoom(r);
	// TODO AddLightCones when rt->UseLightCones
	CalculateRoomExtents(r);
	return r;
}

Rooms *CreateRoom(int zone, int shape, float x, float y, float z, const char *name) {
	Rooms *r = lnew(Rooms);
	r->zone = zone; r->x = x; r->y = y; r->z = z;
	if (s_len(name)) {
		str n = s_lower(name);
		each(RoomTemplates, rt) if (s_eq(rt->Name, n)) return place(r, rt, x, y, z);
	}
	int temp = 0;
	each(RoomTemplates, rt) for (int i = 0; i <= 4; i++) if (rt->zone[i] == zone && rt->Shape == shape) { temp += rt->Commonness; break; }
	int pick = irand(1, temp);
	temp = 0;
	each(RoomTemplates, rt) for (int i = 0; i <= 4; i++) if (rt->zone[i] == zone && rt->Shape == shape) {
		temp += rt->Commonness;
		if (pick > temp - rt->Commonness && pick <= temp) return place(r, rt, x, y, z);
	}
	return r; // blitz falls off the end here too, w/ a template-less room
}

// the per-room Select lives in fillroom1-3.c, the generic part that follows it is here
void FillRoom(Rooms *r) {
	const FillEntry *tabs[] = {Fills1, Fills2, Fills3};
	for (int k = 0; k < 3; k++) for (const FillEntry *f = tabs[k]; f->name; f++)
		if (s_eq(r->RoomTemplate->Name, f->name)) { f->fn(r); k = 3; break; }
	each(LightTemplates, lt) if (lt->roomtemplate == r->RoomTemplate) {
		int l = AddLight(r, r->x + lt->x, r->y + lt->y, r->z + lt->z, lt->ltype, lt->range, lt->r, lt->g, lt->b);
		if (l && lt->ltype == 3) { light_cone_angles(l, lt->innerconeangle, lt->outerconeangle); rotate_entity(l, lt->pitch, lt->yaw, 0, 0); }
	}
	each(TempScreens, ts) if (ts->roomtemplate == r->RoomTemplate) CreateScreen(r->x + ts->x, r->y + ts->y, r->z + ts->z, ts->imgpath, r);
	each(TempWayPoints, tw) if (tw->roomtemplate == r->RoomTemplate) CreateWaypoint(r->x + tw->x, r->y + tw->y, r->z + tw->z, NULL, r);
	RoomTemplates *rt = r->RoomTemplate;
	if (rt->TempTriggerboxAmount > 0) {
		r->TriggerboxAmount = rt->TempTriggerboxAmount;
		for (int i = 0; i < r->TriggerboxAmount; i++) {
			r->Triggerbox[i] = copy_entity(rt->TempTriggerbox[i], r->obj);
			sset(&r->TriggerboxName[i], rt->TempTriggerboxName[i]);
		}
	}
	for (int i = 0; i < MaxRoomEmitters; i++) if (rt->TempSoundEmitter[i]) {
		r->SoundEmitterObj[i] = create_pivot(r->obj);
		position_entity(r->SoundEmitterObj[i], r->x + rt->TempSoundEmitterX[i], r->y + rt->TempSoundEmitterY[i], r->z + rt->TempSoundEmitterZ[i], 1);
		entity_parent(r->SoundEmitterObj[i], r->obj, 1);
		r->SoundEmitter[i] = rt->TempSoundEmitter[i];
		r->SoundEmitterRange[i] = rt->TempSoundEmitterRange[i];
	}
}

// ---- lights, screens, waypoints
static int sprite(float x, float y, float z, float sc, int tex) {
	int s = create_sprite(0);
	position_entity(s, x, y, z, 0);
	scale_sprite(s, sc, sc);
	entity_texture(s, tex, 0, 0);
	entity_blend(s, 3);
	return s;
}

int AddLight(Rooms *room, float x, float y, float z, int ltype, float range, int r, int g, int b) {
	if (!room) {
		int l = create_light(ltype, 0);
		light_range(l, range); light_color(l, r, g, b);
		position_entity(l, x, y, z, 1);
		sprite(x, y, z, .13f, LightSpriteTex[0]);
		return l;
	}
	for (int i = 0; i < MaxRoomLights; i++) {
		if (room->Lights[i]) continue;
		int l = room->Lights[i] = create_light(ltype, 0);
		light_range(l, range); light_color(l, r, g, b);
		position_entity(l, x, y, z, 1);
		entity_parent(l, room->obj, 1);
		room->LightIntensity[i] = (r + g + b) / 255.0f / 3.0f;
		room->LightSprites[i] = sprite(x, y, z, .13f, LightSpriteTex[0]);
		entity_parent(room->LightSprites[i], room->obj, 1);
		int pv = room->LightSpritesPivot[i] = create_pivot(0);
		entity_radius(pv, .05f, .05f);
		position_entity(pv, x, y, z, 0);
		entity_parent(pv, room->obj, 1);
		int s2 = room->LightSprites2[i] = sprite(x, y, z, .6f, LightSpriteTex[2]);
		entity_order(s2, -1);
		entity_color(s2, r, g, b);
		entity_parent(s2, room->obj, 1);
		entity_fx(s2, 1);
		rotate_entity(s2, 0, 0, irand(1, 360), 0);
		sprite_view_mode(s2, 1);
		room->LightSpriteHidden[i] = 1;
		hide_entity(s2);
		room->LightFlicker[i] = irand(1, 10);
		room->LightR[i] = r; room->LightG[i] = g; room->LightB[i] = b;
		hide_entity(l);
		room->MaxLights++;
		return l;
	}
	return 0;
}

LightTemplates *AddTempLight(RoomTemplates *rt, float x, float y, float z, int ltype, float range, int r, int g, int b) {
	LightTemplates *lt = lnew(LightTemplates);
	lt->roomtemplate = rt; lt->x = x; lt->y = y; lt->z = z; lt->ltype = ltype; lt->range = range;
	lt->r = r; lt->g = g; lt->b = b;
	return lt;
}

WayPoints *CreateWaypoint(float x, float y, float z, Doors *door, Rooms *room) {
	WayPoints *w = lnew(WayPoints);
	w->obj = create_pivot(0);
	position_entity(w->obj, x, y, z, 0);
	entity_parent(w->obj, room->obj, 1);
	w->room = room; w->door = door;
	return w;
}

Screens *CreateScreen(float x, float y, float z, const char *imgpath, Rooms *r) {
	Screens *s = lnew(Screens);
	s->obj = create_pivot(0);
	entity_pick_mode(s->obj, 1, 1);
	entity_radius(s->obj, .1f, .1f);
	position_entity(s->obj, x, y, z, 0);
	sset(&s->imgpath, s_dup(imgpath));
	s->room = r;
	entity_parent(s->obj, r->obj, 1);
	return s;
}

// ---- extents + overlap
void CalculateRoomTemplateExtents(RoomTemplates *r) {
	if (r->DisableOverlapCheck) return;
	GetMeshExtents(get_child(r->obj, 1));
	r->MinX = Mesh_MinX; r->MinY = Mesh_MinY; r->MinZ = Mesh_MinZ;
	r->MaxX = Mesh_MaxX; r->MaxY = Mesh_MaxY; r->MaxZ = Mesh_MaxZ;
}

void CalculateRoomExtents(Rooms *r) {
	if (r->RoomTemplate->DisableOverlapCheck) return;
	float sh = .05f; // shrink a bit, overlaps thinner than the walls don't matter
	RoomTemplates *t = r->RoomTemplate;
	t_form_vector(t->MinX, t->MinY, t->MinZ, r->obj, 0);
	r->MinX = t_formed_x() + sh + r->x; r->MinY = t_formed_y() + sh; r->MinZ = t_formed_z() + sh + r->z;
	t_form_vector(t->MaxX, t->MaxY, t->MaxZ, r->obj, 0);
	r->MaxX = t_formed_x() - sh + r->x; r->MaxY = t_formed_y() - sh; r->MaxZ = t_formed_z() - sh + r->z;
	if (r->MinX > r->MaxX) { float t2 = r->MaxX; r->MaxX = r->MinX; r->MinX = t2; }
	if (r->MinZ > r->MaxZ) { float t2 = r->MaxZ; r->MaxZ = r->MinZ; r->MinZ = t2; }
}

int CheckRoomOverlap(Rooms *a, Rooms *b) {
	if (a->MaxX <= b->MinX || a->MaxY <= b->MinY || a->MaxZ <= b->MinZ) return 0;
	if (a->MinX >= b->MaxX || a->MinY >= b->MaxY || a->MinZ >= b->MaxZ) return 0;
	return 1;
}

static int overlapsany(Rooms *r) {
	each(Rooms, r2) if (r2 != r && !r2->RoomTemplate->DisableOverlapCheck && CheckRoomOverlap(r, r2)) return 1;
	return 0;
}
static void moveroom(Rooms *r, int x, int y, int rot) {
	r->x = x * 8.0f; r->z = y * 8.0f; r->angle = rot;
	position_entity(r->obj, r->x, r->y, r->z, 0);
	rotate_entity(r->obj, 0, r->angle, 0, 0);
	CalculateRoomExtents(r);
}

int PreventRoomOverlap(Rooms *r) {
	RoomTemplates *t = r->RoomTemplate;
	if (t->DisableOverlapCheck) return 0;
	if (s_eq(t->Name, "checkpoint1") || s_eq(t->Name, "checkpoint2") || s_eq(t->Name, "start")) return 1;
	if (!overlapsany(r)) return 1;
	// a ROOM2 might just fit turned around
	if (t->Shape == ROOM2) {
		r->angle += 180;
		rotate_entity(r->obj, 0, r->angle, 0, 0);
		CalculateRoomExtents(r);
		if (!overlapsany(r)) return 1;
		r->angle -= 180;
		rotate_entity(r->obj, 0, r->angle, 0, 0);
		CalculateRoomExtents(r);
	}
	// else swap places w/ another room of the same shape + zone. blitz doesn't stop at the first
	// swap that works, it keeps going through the rest (a later good swap moves r again). kept
	int any = 0;
	each(Rooms, r2) {
		if (r2 == r || r2->RoomTemplate->DisableOverlapCheck) continue;
		RoomTemplates *t2 = r2->RoomTemplate;
		if (t->Shape != t2->Shape || r->zone != r2->zone) continue;
		if (s_eq(t2->Name, "checkpoint1") || s_eq(t2->Name, "checkpoint2") || s_eq(t2->Name, "start")) continue;
		any = 1;
		int x = iround(r->x / 8.0f), y = iround(r->z / 8.0f), rot = r->angle;
		int x2 = iround(r2->x / 8.0f), y2 = iround(r2->z / 8.0f), rot2 = r2->angle;
		moveroom(r, x2, y2, rot2);
		moveroom(r2, x, y, rot);
		int bad = 0;
		each(Rooms, r3) {
			if (r3->RoomTemplate->DisableOverlapCheck) continue;
			if ((r3 != r && CheckRoomOverlap(r, r3)) || (r3 != r2 && CheckRoomOverlap(r2, r3))) { bad = 1; break; }
		}
		if (bad) { moveroom(r, x, y, rot); moveroom(r2, x2, y2, rot2); }
	}
	return any;
}

// ---- MapSystem.bb's top-level globals
void InitMapGlobals(void) {
	RoomScale = 8.0f / 2048.0f;
	MapWidth = GetINIInt("options.ini", "options", "map size", 0);
	MapHeight = GetINIInt("options.ini", "options", "map size", 0);
	MapTemp = calloc((MapWidth + 2) * (MapHeight + 2), sizeof(int));
	MapFound = calloc((MapWidth + 2) * (MapHeight + 2), sizeof(int));
	HideDistance = 15; SecondaryLightOn = PrevSecondaryLightOn = 1; RemoteDoorOn = 1;
	I_Zone = lnew(MapZones);
	LoadRoomTemplates("Data\\rooms.ini");
}

int *MT(int x, int y) {
	if (x < 0 || y < 0 || x > MapWidth + 1 || y > MapHeight + 1) RuntimeError(cat("MapTemp index out of bounds: ", s_str(x), ", ", s_str(y)));
	return &MapTemp[x * (MapHeight + 2) + y];
}
str *MN(int x, int y) {
	if (x < 0 || y < 0 || x > MapWidth || y > MapHeight) RuntimeError(cat("MapName index out of bounds: ", s_str(x), ", ", s_str(y)));
	return &MapName[x * (MapHeight + 1) + y];
}
str *MR(int t, int i) {
	if (t < 0 || i < 0 || t > ROOM4 + 1 || i > MaxRooms_) RuntimeError(cat("MapRoom index out of bounds: ", s_str(t), ", ", s_str(i)));
	return &MapRoom[t * (MaxRooms_ + 1) + i];
}

// ---- per frame (MapSystem.bb's UpdateRooms)
int IsRoomAdjacent(Rooms *a, Rooms *b) {
	if (!a) return 0;
	if (a == b) return 1;
	for (int i = 0; i <= 3; i++) if (b == a->Adjacent[i]) return 1;
	return 0;
}

static int near4(Rooms *r) { return fabsf(r->x - entity_x(Collider, 1)) < 4 && fabsf(r->z - entity_z(Collider, 1)) < 4; }

void UpdateRooms(void) {
	float cz = entity_z(Collider, 0) / 8.0f;
	int sm = !s_len(SelectedMap); // blitz subtracts the boolean (sset(&SelectedMap, ""))
	PlayerZone = cz < I_Zone->Transition[1] - sm ? 2 : cz < I_Zone->Transition[0] - sm ? 1 : 0;
	TempLightVolume = 0;
	int found = 0;
	if (PlayerRoom) {
		if (fabsf(entity_y(Collider, 0) - entity_y(PlayerRoom->obj, 0)) < 1.5f) {
			if (near4(PlayerRoom)) found = 1;
			else for (int i = 0; i <= 3; i++) if (PlayerRoom->Adjacent[i] && near4(PlayerRoom->Adjacent[i])) {
				found = 1; PlayerRoom = PlayerRoom->Adjacent[i];
				break;
			}
		} else found = 1; // high up or deep down: PlayerRoom stays
	}
	each(Rooms, r) {
		float x = fabsf(r->x - entity_x(Collider, 1)), z = fabsf(r->z - entity_z(Collider, 1));
		r->dist = Max(x, z);
		if (x < 16 && z < 16) {
			for (int i = 0; i < MaxRoomEmitters; i++) {
				if (!r->SoundEmitter[i]) continue;
				if (entity_distance(r->SoundEmitterObj[i], Collider) < r->SoundEmitterRange[i])
					r->SoundEmitterCHN[i] = LoopSound2(RoomAmbience[r->SoundEmitter[i]], r->SoundEmitterCHN[i], Camera, r->SoundEmitterObj[i], r->SoundEmitterRange[i], 1);
			}
			if (!found && PlayerRoom != r && x < 4 && z < 4) {
				if (fabsf(entity_y(Collider, 0) - entity_y(r->obj, 0)) < 1.5f) PlayerRoom = r;
				found = 1;
			}
		}
		int hide = r != PlayerRoom && !IsRoomAdjacent(PlayerRoom, r);
		if (hide) for (int i = 0; i <= 3; i++) if (IsRoomAdjacent(PlayerRoom->Adjacent[i], r)) { hide = 0; break; }
		if (hide) { hide_entity(r->obj); continue; }
		show_entity(r->obj);
		for (int i = 0; i < MaxRoomLights && r->Lights[i]; i++) {
			float d = entity_distance(Collider, r->Lights[i]);
			if (d < HideDistance) TempLightVolume += r->LightIntensity[i] * r->LightIntensity[i] * ((HideDistance - d) / HideDistance);
		}
		for (int i = 0; i < r->TriggerboxAmount; i++) { entity_color(r->Triggerbox[i], 255, 255, 255); entity_alpha(r->Triggerbox[i], 0); } // TODO DebugHUD: yellow, .2
	}
	MapFound[(int)floorf(entity_x(PlayerRoom->obj, 0) / 8.0f) * (MapHeight + 2) + (int)floorf(entity_z(PlayerRoom->obj, 0) / 8.0f)] = 1;
	PlayerRoom->found = 1;
	TempLightVolume = Max(TempLightVolume / 4.5f, 1);
	// only draw the next room's opaque mesh through doors that are open + on screen
	entity_alpha(get_child(PlayerRoom->obj, 1), 1);
	for (int i = 0; i <= 3; i++) {
		Rooms *a = PlayerRoom->Adjacent[i];
		if (!a) continue;
		Doors *d = PlayerRoom->AdjDoor[i];
		if (d) entity_alpha(get_child(a->obj, 1), d->openstate == 0 || !entity_in_view(d->frameobj, Camera) ? 0 : 1);
		for (int j = 0; j <= 3; j++) if (a->Adjacent[j] && a->Adjacent[j] != PlayerRoom) entity_alpha(get_child(a->Adjacent[j]->obj, 1), 0);
	}
}

void AmbientLightRooms(int value) {
	if (value == AmbientLightRoomVal) return;
	AmbientLightRoomVal = value;
	texture_fill(AmbientLightRoomTex, value, value, value, 255); // blitz: SetBuffer + ClsColor + Cls
}

// frees a room + clears everything that points at it (only the intro's room ever goes away)
void RemoveRoom(Rooms *r) {
	if (!r->n.ls) return;
	if (PlayerRoom == r) PlayerRoom = NULL;
	for (int i = 0; i < 11; i++) if (MTFrooms[i] == r) MTFrooms[i] = NULL;
	each(Rooms, o) for (int i = 0; i < 5; i++) if (o->Adjacent[i] == r) o->Adjacent[i] = NULL;
	each(WayPoints, w) if (w->room == r) w->room = NULL;
	each(Doors, d) if (d->room == r) d->room = NULL;
	each(Events, e) if (e->room == r) e->room = NULL;
	each(Screens, s) if (s->room == r) s->room = NULL;
	each(Emitters, em) if (em->Room == r) em->Room = NULL;
	each(DevilEmitters, de) if (de->room == r) de->room = NULL;
	each(SecurityCams, sc) if (sc->room == r) sc->room = NULL;
	free_entity(r->obj);
	ldel(r);
}
