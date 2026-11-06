// events.c: Main.bb's CreateEvent/InitEvents/RemoveEvent. TODO UpdateEvents.bb (the per-event logic)
#include "g.h"

TLIST(Events);

static int taken(Rooms *r) { each(Events, e) if (e->room == r) return 1; return 0; }

// id: nth matching room (0 and 1 both mean the first). prob <> 0: every matching room gets it w/ that chance, id ignored
Events *CreateEvent(const char *eventname, const char *roomname, int id, float prob) {
	int i = 0;
	each(Rooms, r) {
		if (!(!*roomname || bbSeq(roomname, r->RoomTemplate->Name))) continue;
		int t = taken(r);
		if (prob == 0) {
			i++;
			if (i >= id && !t) { Events *e = bbNew(Events); e->EventName = bbSdup(eventname); e->room = r; return e; }
		} else if ((bbRnd(0, 1) < prob) & !t) { // blitz And: Rnd rolls for taken rooms too
			Events *e = bbNew(Events); e->EventName = bbSdup(eventname); e->room = r;
		}
	}
	return NULL;
}

Events *FindEvent(const char *name) { each(Events, e) if (bbSeq(e->EventName, name)) return e; return NULL; }

void InitEvents(void) {
	float ag = SelectedDifficulty->aggressiveNPCs;
	CreateEvent("173", "173", 0, 0);
	CreateEvent("alarm", "start", 0, 0);
	CreateEvent("pocketdimension", "pocketdimension", 0, 0);
	CreateEvent("tunnel106", "tunnel", 0, .07f + .1f * ag); // 7% (17% aggressive) of the tunnels
	if (bbRand(3, 1) < 3) CreateEvent("lockroom173", "lockroom", 0, 0);
	CreateEvent("lockroom173", "lockroom", 0, .3f + .5f * ag);
	CreateEvent("room2trick", "room2", 0, .15f);
	CreateEvent("1048a", "room2", 0, 1);
	CreateEvent("room2storage", "room2storage", 0, 0);
	CreateEvent("lockroom096", "lockroom2", 0, 0);
	CreateEvent("endroom106", "endroom", bbRand(0, 1), 0);
	CreateEvent("room2poffices2", "room2poffices2", 0, 0);
	CreateEvent("room2fan", "room2_2", 0, 1);
	CreateEvent("room2elevator2", "room2elevator", 0, 0);
	CreateEvent("room2elevator", "room2elevator", bbRand(1, 2), 0);
	CreateEvent("room3storage", "room3storage", 0, 0);
	CreateEvent("tunnel2smoke", "tunnel2", 0, .2f);
	CreateEvent("tunnel2", "tunnel2", bbRand(0, 2), 0);
	CreateEvent("tunnel2", "tunnel2", 0, .2f * ag);
	CreateEvent("room2doors173", "room2doors", 0, .5f + .4f * ag);
	CreateEvent("room2offices2", "room2offices2", 0, .7f); // the duck
	CreateEvent("room2closets", "room2closets", 0, 0);
	CreateEvent("room2cafeteria", "room2cafeteria", 0, 0);
	CreateEvent("room3pitduck", "room3pit", 0, 0);
	CreateEvent("room3pit1048", "room3pit", 1, 0);
	CreateEvent("room2offices3", "room2offices3", 0, 1);
	CreateEvent("room2servers", "room2servers", 0, 0);
	CreateEvent("room3servers", "room3servers", 0, 0);
	CreateEvent("room3servers", "room3servers2", 0, 0);
	CreateEvent("room3tunnel", "room3tunnel", 0, .08f); // the dead guard
	CreateEvent("room4", "room4", 0, 0);
	if (bbRand(5, 1) < 5) {
		switch (bbRand(3, 1)) {
		case 1: CreateEvent("682roar", "tunnel", bbRand(0, 2), 0); break;
		case 2: CreateEvent("682roar", "room3pit", bbRand(0, 2), 0); break;
		case 3: CreateEvent("682roar", "room2z3", 0, 0); break;
		}
	}
	CreateEvent("testroom173", "room2testroom2", 0, 1);
	CreateEvent("room2tesla", "room2tesla", 0, .9f);
	CreateEvent("room2nuke", "room2nuke", 0, 0);
	if (bbRand(5, 1) < 5) CreateEvent("coffin106", "coffin", 0, 0);
	else CreateEvent("coffin", "coffin", 0, 0);
	CreateEvent("checkpoint", "checkpoint1", 0, 1);
	CreateEvent("checkpoint", "checkpoint2", 0, 1);
	CreateEvent("room3door", "room3", 0, .1f);
	CreateEvent("room3door", "room3tunnel", 0, .1f);
	if (bbRand(2, 1) == 1) {
		CreateEvent("106victim", "room3", bbRand(1, 2), 0);
		CreateEvent("106sinkhole", "room3_2", bbRand(2, 3), 0);
	} else {
		CreateEvent("106victim", "room3_2", bbRand(1, 2), 0);
		CreateEvent("106sinkhole", "room3", bbRand(2, 3), 0);
	}
	CreateEvent("106sinkhole", "room4", bbRand(1, 2), 0);
	static const char *plain[][2] = {{"room079", "room079"}, {"room049", "room049"}, {"room012", "room012"}, {"room035", "room035"},
		{"008", "008"}, {"room106", "room106"}, {"pj", "roompj"}, {"914", "914"}, {"buttghost", "room2toilets"}};
	for (int i = 0; i < 9; i++) CreateEvent(plain[i][0], plain[i][1], 0, 0);
	CreateEvent("toiletguard", "room2toilets", 1, 0);
	CreateEvent("room2pipes106", "room2pipes", bbRand(0, 3), 0);
	CreateEvent("room2pit", "room2pit", 0, .4f + .4f * ag);
	static const char *plain2[][2] = {{"testroom", "testroom"}, {"room2tunnel", "room2tunnel"}, {"room2ccont", "room2ccont"},
		{"gateaentrance", "gateaentrance"}, {"gatea", "gatea"}, {"exit1", "exit1"}, {"room205", "room205"}, {"room860", "room860"},
		{"room966", "room966"}, {"room1123", "room1123"}};
	for (int i = 0; i < 10; i++) CreateEvent(plain2[i][0], plain2[i][1], 0, 0);
	CreateEvent("room2tesla", "room2tesla_lcz", 0, .9f);
	CreateEvent("room2tesla", "room2tesla_hcz", 0, .9f);
	// 1.3's events
	CreateEvent("room4tunnels", "room4tunnels", 0, 0);
	CreateEvent("room_gw", "room2gw", 0, 1);
	CreateEvent("dimension1499", "dimension1499", 0, 0);
	CreateEvent("room1162", "room1162", 0, 0);
	CreateEvent("room2scps2", "room2scps2", 0, 0);
	CreateEvent("room_gw", "room3gw", 0, 1);
	CreateEvent("room2sl", "room2sl", 0, 0);
	CreateEvent("medibay", "medibay", 0, 0);
	CreateEvent("room2shaft", "room2shaft", 0, 0);
	CreateEvent("room1lifts", "room1lifts", 0, 0);
	CreateEvent("room2gw_b", "room2gw_b", bbRand(0, 1), 0);
	static const struct { const char *room; float p; } s096[] = {{"room4pit", .6f}, {"room3pit", .6f}, {"room2pipes", .4f}, {"room2pit", .5f},
		{"room3tunnel", .6f}, {"room4tunnels", .7f}, {"tunnel", .6f}, {"tunnel2", .4f}, {"room3z2", .7f}};
	for (int i = 0; i < 9; i++) CreateEvent("096spawn", s096[i].room, 0, s096[i].p + .2f * ag);
	CreateEvent("room2pit", "room2_4", 0, .4f + .4f * ag);
	CreateEvent("room2offices035", "room2offices", 0, 0);
	CreateEvent("room2pit106", "room2pit", 0, .07f + .1f * ag);
	CreateEvent("room1archive", "room1archive", 0, 1);
}

void RemoveEvent(Events *e) {
	if (e->Sound) FreeSound_Strict(e->Sound);
	if (e->Sound2) FreeSound_Strict(e->Sound2);
	if (e->img) bbFreeImage(e->img);
	bbDelete(e);
}
