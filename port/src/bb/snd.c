// snd.c: blitz sounds/channels + the fmod stream bits scpcb uses, on raylib audio.
// a channel is a raylib sound alias (its own voice). handles carry a generation so a
// stale channel never steers a recycled voice. the game does its own 3d (vol + pan).
#include <stdlib.h>
#include <string.h>
#include "in.h"

typedef struct { Sound s; int used, loop; float vol, pan, hz, rate; } snd;
typedef struct { Sound a; int used, gen, snd, loop, paused; } chan;
typedef struct { Music m; int used, gen, paused; } strm;

static snd *ss; static int nss, css;
#define NCH 256
static chan ch[NCH];
#define NST 32
static strm st[NST];
static int audio; // -1 failed, 1 up

static int up(void) {
	if (!audio) { InitAudioDevice(); audio = IsAudioDeviceReady() ? 1 : -1; if (audio < 0) fprintf(stderr, "bb: no audio device, sounds are silent\n"); }
	return audio > 0;
}
void snd_quit(void) {
	for (int i = 0; i < NCH; i++) if (ch[i].used) { UnloadSoundAlias(ch[i].a); ch[i].used = 0; }
	for (int i = 0; i < NST; i++) if (st[i].used) { UnloadMusicStream(st[i].m); st[i].used = 0; }
	for (int i = 1; i < nss; i++) if (ss[i].used) { if (ss[i].s.stream.buffer) UnloadSound(ss[i].s); ss[i].used = 0; }
	if (audio > 0) CloseAudioDevice();
	audio = 0;
}

// ---- sounds
static snd *gsnd(int h) { return h > 0 && h < nss && ss[h].used ? &ss[h] : NULL; }
// no audio device: still hand out a handle (silent), so game code doesn't see load errors
int bbLoadSound(const char *p) {
	const char *r = bbPath(p);
	if (!r || DirectoryExists(r)) return 0;
	Sound so = {0}; int rate = 44100;
	if (up()) {
		Wave w = LoadWave(r);
		if (!w.data) return 0;
		so = LoadSoundFromWave(w); rate = w.sampleRate;
		UnloadWave(w);
	}
	if (!nss) nss = 1;
	int h = 0;
	for (int i = 1; i < nss; i++) if (!ss[i].used) { h = i; break; }
	if (!h) { grow(ss, nss, css); h = nss++; }
	ss[h] = (snd){so, 1, 0, 1, 0, 0, rate};
	return h;
}
static void stopall(int h) {
	for (int i = 0; i < NCH; i++) if (ch[i].used && ch[i].snd == h) { StopSound(ch[i].a); UnloadSoundAlias(ch[i].a); ch[i].used = 0; }
}
void bbFreeSound(int h) {
	snd *s = gsnd(h);
	if (!s) return;
	stopall(h); // aliases share its buffer
	if (s->s.stream.buffer) UnloadSound(s->s);
	s->used = 0;
}
void bbLoopSound(int h) { snd *s = gsnd(h); if (s) s->loop = 1; }
void bbSoundVolume(int h, float v) { snd *s = gsnd(h); if (s) s->vol = v; }
void bbSoundPan(int h, float p) { snd *s = gsnd(h); if (s) s->pan = p; }
void bbSoundPitch(int h, float hz) { snd *s = gsnd(h); if (s) s->hz = hz; }

// ---- channels
static chan *gch(int h) {
	int i = (h & 0xfff) - 1;
	if (h <= 0 || i >= NCH || !ch[i].used || ch[i].gen != h >> 12) return NULL;
	return &ch[i];
}
static float rlpan(float p) { return (1 - fmaxf(-1, fminf(1, p))) * 0.5f; } // blitz -1 left .. 1 right; raylib 1 = left

int bbPlaySound(int h) {
	snd *s = gsnd(h);
	if (!s || !s->s.stream.buffer) return 0; // silent
	int i = -1;
	for (int k = 0; k < NCH && i < 0; k++) if (!ch[k].used) i = k;
	for (int k = 0; k < NCH && i < 0; k++) // recycle a finished voice
		if (!ch[k].paused && !ch[k].loop && !IsSoundPlaying(ch[k].a)) { UnloadSoundAlias(ch[k].a); ch[k].used = 0; i = k; }
	if (i < 0) return 0;
	chan *c = &ch[i];
	c->a = LoadSoundAlias(s->s);
	c->used = 1; c->gen = (c->gen + 1) & 0x7ffff; c->snd = h; c->loop = s->loop; c->paused = 0;
	SetSoundVolume(c->a, s->vol);
	SetSoundPan(c->a, rlpan(s->pan));
	if (s->hz > 0) SetSoundPitch(c->a, s->hz / s->rate);
	PlaySound(c->a);
	return (c->gen << 12) | (i + 1);
}
void bbStopChannel(int h) { chan *c = gch(h); if (c) { StopSound(c->a); c->loop = 0; c->paused = 0; } }
void bbPauseChannel(int h) { chan *c = gch(h); if (c && IsSoundPlaying(c->a)) { PauseSound(c->a); c->paused = 1; } }
void bbResumeChannel(int h) { chan *c = gch(h); if (c && c->paused) { ResumeSound(c->a); c->paused = 0; } }
void bbChannelVolume(int h, float v) { chan *c = gch(h); if (c) SetSoundVolume(c->a, fmaxf(0, v)); }
void bbChannelPan(int h, float p) { chan *c = gch(h); if (c) SetSoundPan(c->a, rlpan(p)); }
void bbChannelPitch(int h, float hz) { chan *c = gch(h); if (c && gsnd(c->snd)) SetSoundPitch(c->a, hz / ss[c->snd].rate); }
// paused counts as playing, like blitz
int bbChannelPlaying(int h) { chan *c = gch(h); return c && (c->paused || c->loop || IsSoundPlaying(c->a)); }

// ---- streams (what StreamSound_Strict & co need from fmod)
static strm *gst(int h) {
	int i = (h & 0xfff) - 1;
	if (h <= 0 || i >= NST || !st[i].used || st[i].gen != h >> 12) return NULL;
	return &st[i];
}
int bbStreamOpen(const char *p, int loop) {
	const char *r = bbPath(p);
	if (!r || !up()) return 0;
	int i = -1;
	for (int k = 0; k < NST && i < 0; k++) if (!st[k].used) i = k;
	if (i < 0) return 0;
	Music m = LoadMusicStream(r);
	if (!m.stream.buffer) return 0;
	m.looping = loop;
	st[i] = (strm){m, 1, (st[i].gen + 1) & 0x7ffff, 1};
	return (st[i].gen << 12) | (i + 1);
}
void bbStreamPlay(int h) { strm *s = gst(h); if (s) { PlayMusicStream(s->m); s->paused = 0; } }
void bbStreamPaused(int h, int p) {
	strm *s = gst(h);
	if (!s) return;
	if (p) PauseMusicStream(s->m);
	else if (!IsMusicStreamPlaying(s->m)) { if (s->paused) ResumeMusicStream(s->m); else PlayMusicStream(s->m); }
	s->paused = p;
}
void bbStreamVolume(int h, float v) { strm *s = gst(h); if (s) SetMusicVolume(s->m, fmaxf(0, v)); }
void bbStreamPan(int h, float p) { strm *s = gst(h); if (s) SetMusicPan(s->m, rlpan(p)); }
int bbStreamPlaying(int h) { strm *s = gst(h); return s && (s->paused || IsMusicStreamPlaying(s->m)); }
void bbStreamClose(int h) { strm *s = gst(h); if (s) { StopMusicStream(s->m); UnloadMusicStream(s->m); s->used = 0; } }

// per frame (bbFlip): feed streams, restart LoopSound voices
void snd_update(void) {
	if (audio <= 0) return;
	for (int i = 0; i < NST; i++) if (st[i].used && !st[i].paused) UpdateMusicStream(st[i].m);
	for (int i = 0; i < NCH; i++) if (ch[i].used && ch[i].loop && !ch[i].paused && !IsSoundPlaying(ch[i].a)) PlaySound(ch[i].a);
}
