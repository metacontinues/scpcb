// sfx.h: the game's own sounds: footsteps (by floor material), landing,
// breathing when tired, the zone's random ambience + music
#pragma once
#include "player.h"
#include "world.h"

void sfx_init(void);         // reads Data/materials.ini's step sounds
int world_zone(Vector3 pos); // 0 light, 1 heavy, 2 entrance containment
void sfx_tick(World *w, const Player *p, int sprintheld);
