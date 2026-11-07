// waypoints.c: MapSystem.bb's InitWayPoints + FindPath (a*)
#include "g.h"

void InitWayPoints(int loadingstart) {
	each(Doors, d) {
		if (d->obj) hide_entity(d->obj);
		if (d->obj2) hide_entity(d->obj2);
		if (d->frameobj) hide_entity(d->frameobj);
		Rooms *cr = d->room;
		if (!cr) {
			float dist = 30;
			each(Rooms, r) {
				float x = fabsf(entity_x(r->obj, 1) - entity_x(d->frameobj, 1));
				if (x >= 20) continue;
				float z = fabsf(entity_z(r->obj, 1) - entity_z(d->frameobj, 1));
				if (z >= 20) continue;
				float d2 = x * x + z * z;
				if (d2 < dist) { cr = r; dist = d2; }
			}
		}
		if (!d->DisableWaypoint) CreateWaypoint(entity_x(d->frameobj, 1), entity_y(d->frameobj, 1) + .18f, entity_z(d->frameobj, 1), d, cr);
	}
	float amount = 0;
	each(WayPoints, w) { entity_pick_mode(w->obj, 1, 1); entity_radius(w->obj, .2f, .2f); amount++; }
	int number = 0, iter = 0;
	each(WayPoints, w) {
		number++; iter++;
		if (iter == 20) { DrawLoading(loadingstart + (int)floorf(35.0f / amount * number), 0); iter = 0; }
		for (WayPoints *w2 = lafter(w); w2; w2 = lafter(w2)) {
			if (!(w->room == w2->room || w->door || w2->door)) continue;
			float dist = entity_distance(w->obj, w2->obj);
			int can = w->room->MaxWayPointY == 0 || w2->room->MaxWayPointY == 0 || fabsf(entity_y(w->obj, 0) - entity_y(w2->obj, 0)) <= w->room->MaxWayPointY;
			if (dist < 7 && can && entity_visible(w->obj, w2->obj)) {
				for (int i = 0; i <= 4; i++) if (!w->connected[i]) { w->connected[i] = w2; w->dist[i] = dist; break; }
				for (int i = 0; i <= 4; i++) if (!w2->connected[i]) { w2->connected[i] = w; w2->dist[i] = dist; break; }
			}
		}
	}
	each(Doors, d) {
		if (d->obj) show_entity(d->obj);
		if (d->obj2) show_entity(d->obj2);
		if (d->frameobj) show_entity(d->frameobj);
	}
	// blitz also hangs a CreateLine debug mesh per connection off each waypoint: 2-vertex degenerate
	// triangles, never visible, so they're skipped
	each(WayPoints, w) { entity_pick_mode(w->obj, 0, 0); entity_radius(w->obj, 0, 0); }
}

void RemoveWaypoint(WayPoints *w) { free_entity(w->obj); ldel(w); }

// 0: not searched / already there, 1: found (n->Path), 2: unreachable
int FindPath(NPCs *n, float x, float y, float z) {
	each(WayPoints, w) { w->state = 0; w->Fcost = w->Gcost = w->Hcost = 0; }
	n->PathStatus = 0; n->PathLocation = 0;
	for (int i = 0; i <= 19; i++) n->Path[i] = NULL;

	int pvt = create_pivot(0);
	position_entity(pvt, x, y, z, 1);
	int tmp = create_pivot(0);
	position_entity(tmp, entity_x(n->Collider, 1), entity_y(n->Collider, 1) + .15f, entity_z(n->Collider, 1), 0);
	WayPoints *sp = NULL, *ep = NULL;
	float dist = 350;
	each(WayPoints, w) {
		float xt = entity_x(w->obj, 1) - entity_x(tmp, 1), zt = entity_z(w->obj, 1) - entity_z(tmp, 1), yt = entity_y(w->obj, 1) - entity_y(tmp, 1);
		float d2 = xt * xt + yt * yt + zt * zt;
		if (d2 >= dist) continue;
		if (!entity_visible(w->obj, tmp)) d2 *= 3; // prefer visible ones
		if (d2 < dist) { dist = d2; sp = w; }
	}
	free_entity(tmp);
	if (!sp) { free_entity(pvt); return 2; } // blitz leaks the pivot here
	sp->state = 1;
	dist = 400;
	each(WayPoints, w) {
		float xt = entity_x(pvt, 1) - entity_x(w->obj, 1), zt = entity_z(pvt, 1) - entity_z(w->obj, 1), yt = entity_y(pvt, 1) - entity_y(w->obj, 1);
		float d2 = xt * xt + yt * yt + zt * zt;
		if (d2 < dist) { dist = d2; ep = w; }
	}
	free_entity(pvt);
	if (ep == sp) {
		if (dist < .4f) return 0;
		n->Path[0] = ep;
		return 1;
	}
	if (!ep) return 2;

	int open;
	do {
		open = 0;
		WayPoints *sm = NULL;
		dist = 10000;
		each(WayPoints, w) if (w->state == 1) { open = 1; if (w->Fcost < dist) { dist = w->Fcost; sm = w; } }
		if (sm) {
			WayPoints *w = sm;
			w->state = 2;
			for (int i = 0; i <= 4; i++) {
				WayPoints *c = llive(w->connected[i]); // links to deleted waypoints (the intro's) don't count
				if (!c || c->state >= 2) continue;
				float g = w->Gcost + w->dist[i];
				if (n->NPCtype == NPCtypeMTF && !c->door) g += .5f;
				if (c->state == 1) {
					if (g < c->Gcost) { c->Gcost = g; c->Fcost = c->Gcost + c->Hcost; c->parent = w; }
				} else {
					c->Hcost = fabsf(entity_x(c->obj, 1) - entity_x(ep->obj, 1)) + fabsf(entity_z(c->obj, 1) - entity_z(ep->obj, 1));
					c->Gcost = g;
					c->Fcost = c->Gcost + c->Hcost;
					c->parent = w;
					c->state = 1;
				}
			}
		} else if (ep->state > 0) { sp->parent = NULL; ep->state = 2; break; }
		if (ep->state > 0) { sp->parent = NULL; ep->state = 2; break; }
	} while (open);

	if (ep->state <= 0) return 2;
	WayPoints *cur = ep, *tw = ep;
	int len = 0;
	do {
		len++;
		cur = cur->parent;
		if (len > 20) tw = tw->parent;
	} while (cur);
	while (tw) {
		len = iround(Min(len - 1, 19));
		tw = tw->parent;
		if (len >= 0) n->Path[len] = tw;
	}
	return 1;
}
