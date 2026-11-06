// waypoints.c: MapSystem.bb's InitWayPoints + FindPath (a*)
#include "g.h"

void InitWayPoints(int loadingstart) {
	each(Doors, d) {
		if (d->obj) bbHideEntity(d->obj);
		if (d->obj2) bbHideEntity(d->obj2);
		if (d->frameobj) bbHideEntity(d->frameobj);
		Rooms *cr = d->room;
		if (!cr) {
			float dist = 30;
			each(Rooms, r) {
				float x = fabsf(bbEntityX(r->obj, 1) - bbEntityX(d->frameobj, 1));
				if (x >= 20) continue;
				float z = fabsf(bbEntityZ(r->obj, 1) - bbEntityZ(d->frameobj, 1));
				if (z >= 20) continue;
				float d2 = x * x + z * z;
				if (d2 < dist) { cr = r; dist = d2; }
			}
		}
		if (!d->DisableWaypoint) CreateWaypoint(bbEntityX(d->frameobj, 1), bbEntityY(d->frameobj, 1) + .18f, bbEntityZ(d->frameobj, 1), d, cr);
	}
	float amount = 0;
	each(WayPoints, w) { bbEntityPickMode(w->obj, 1, 1); bbEntityRadius(w->obj, .2f, .2f); amount++; }
	int number = 0, iter = 0;
	each(WayPoints, w) {
		number++; iter++;
		if (iter == 20) { DrawLoading(loadingstart + (int)floorf(35.0f / amount * number), 0); iter = 0; }
		int can = 0; // blitz's canCreateWayPoint is never reset, so once true it stays true for this w
		for (WayPoints *w2 = bbAfter(w); w2; w2 = bbAfter(w2)) {
			if (!(w->room == w2->room || w->door || w2->door)) continue;
			float dist = bbEntityDistance(w->obj, w2->obj);
			if (w->room->MaxWayPointY == 0 || w2->room->MaxWayPointY == 0) can = 1;
			else if (fabsf(bbEntityY(w->obj, 0) - bbEntityY(w2->obj, 0)) <= w->room->MaxWayPointY) can = 1;
			if (dist < 7 && can && bbEntityVisible(w->obj, w2->obj)) {
				for (int i = 0; i <= 4; i++) if (!w->connected[i]) { w->connected[i] = w2; w->dist[i] = dist; break; }
				for (int i = 0; i <= 4; i++) if (!w2->connected[i]) { w2->connected[i] = w; w2->dist[i] = dist; break; }
			}
		}
	}
	each(Doors, d) {
		if (d->obj) bbShowEntity(d->obj);
		if (d->obj2) bbShowEntity(d->obj2);
		if (d->frameobj) bbShowEntity(d->frameobj);
	}
	// blitz also hangs a CreateLine debug mesh per connection off each waypoint: 2-vertex degenerate
	// triangles, never visible, so they're skipped
	each(WayPoints, w) { bbEntityPickMode(w->obj, 0, 0); bbEntityRadius(w->obj, 0, 0); }
}

void RemoveWaypoint(WayPoints *w) { bbFreeEntity(w->obj); bbDelete(w); }

// 0: not searched / already there, 1: found (n->Path), 2: unreachable
int FindPath(NPCs *n, float x, float y, float z) {
	each(WayPoints, w) { w->state = 0; w->Fcost = w->Gcost = w->Hcost = 0; }
	n->PathStatus = 0; n->PathLocation = 0;
	for (int i = 0; i <= 19; i++) n->Path[i] = NULL;

	int pvt = bbCreatePivot(0);
	bbPositionEntity(pvt, x, y, z, 1);
	int tmp = bbCreatePivot(0);
	bbPositionEntity(tmp, bbEntityX(n->Collider, 1), bbEntityY(n->Collider, 1) + .15f, bbEntityZ(n->Collider, 1), 0);
	WayPoints *sp = NULL, *ep = NULL;
	float dist = 350;
	each(WayPoints, w) {
		float xt = bbEntityX(w->obj, 1) - bbEntityX(tmp, 1), zt = bbEntityZ(w->obj, 1) - bbEntityZ(tmp, 1), yt = bbEntityY(w->obj, 1) - bbEntityY(tmp, 1);
		float d2 = xt * xt + yt * yt + zt * zt;
		if (d2 >= dist) continue;
		if (!bbEntityVisible(w->obj, tmp)) d2 *= 3; // prefer visible ones
		if (d2 < dist) { dist = d2; sp = w; }
	}
	bbFreeEntity(tmp);
	if (!sp) { bbFreeEntity(pvt); return 2; } // blitz leaks the pivot here
	sp->state = 1;
	dist = 400;
	each(WayPoints, w) {
		float xt = bbEntityX(pvt, 1) - bbEntityX(w->obj, 1), zt = bbEntityZ(pvt, 1) - bbEntityZ(w->obj, 1), yt = bbEntityY(pvt, 1) - bbEntityY(w->obj, 1);
		float d2 = xt * xt + yt * yt + zt * zt;
		if (d2 < dist) { dist = d2; ep = w; }
	}
	bbFreeEntity(pvt);
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
				WayPoints *c = w->connected[i];
				if (!c || c->state >= 2) continue;
				float g = w->Gcost + w->dist[i];
				if (n->NPCtype == NPCtypeMTF && !c->door) g += .5f;
				if (c->state == 1) {
					if (g < c->Gcost) { c->Gcost = g; c->Fcost = c->Gcost + c->Hcost; c->parent = w; }
				} else {
					c->Hcost = fabsf(bbEntityX(c->obj, 1) - bbEntityX(ep->obj, 1)) + fabsf(bbEntityZ(c->obj, 1) - bbEntityZ(ep->obj, 1));
					c->Gcost = g;
					c->Fcost = w->Gcost + w->Hcost; // sic: blitz uses the parent's costs here
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
		len = bbInt(Min(len - 1, 19));
		tw = tw->parent;
		if (len >= 0) n->Path[len] = tw;
	}
	return 1;
}
