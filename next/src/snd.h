// snd.h: sounds + music. sounds are baked qoa by id ("sfx/door/dooropen1"),
// played on a pool of voices (raylib sound aliases) so one sound can overlap
// itself. "3d" is the original's: volume falls off linearly to 0 at `range`,
// pan from the angle to the listener's view
#pragma once
#include "raylib.h"

void snd_init(float sfx, float music); // volumes 0..1
void snd_close(void);
void snd_listener(Vector3 eye, float yaw); // yaw: the player's

// voice handles: 0 = none. a handle goes dead once its voice is reused
int snd_play(const char *id, float vol);
int snd_play3d(const char *id, Vector3 at, float range, float vol);
void snd_place(int v, Vector3 at, float range, float vol); // keep it positioned
int snd_playing(int v);
void snd_stop(int v);

// music: the track that should be playing; the old one fades out first
void snd_music(const char *id);
void snd_tick(void);  // music fades, per tick
void snd_frame(void); // feeds the music stream, per frame
