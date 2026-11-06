// strict.c: StrictLoads.bb. loaders that RuntimeError on missing files, Sound/Stream wrappers
#include "g.h"

TLIST(Sound);
typedef struct Stream { lnode n; int sfx, chn; } Stream;
static TLIST(Stream);

int LoadImage_Strict(const char *file) {
	if (bbFileType(file) != 1) RuntimeError(cat("Image \"", file, "\" missing. "));
	return bbLoadImage(file);
}

void AutoReleaseSounds(void) {
	each(Sound, snd) {
		int tryRelease = 1;
		for (int i = 0; i < 32; i++)
			if (snd->channels[i] && bbChannelPlaying(snd->channels[i])) { tryRelease = 0; snd->releaseTime = MilliSecs2() + 5000; break; }
		if (tryRelease && snd->releaseTime < MilliSecs2() && snd->internalHandle) { bbFreeSound(snd->internalHandle); snd->internalHandle = 0; }
	}
}

static void sndmsg(const char *m) { CreateConsoleMsg(m, -1, -1, -1, 0); if (ConsoleOpening) ConsoleOpen = 1; }

// the original has two copies of this per slot (free vs finished channel); they only differ in
// ConsoleFlush vs ConsoleFlushSnd as the flush check, kept that way
int PlaySound_Strict(int h) {
	Sound *snd = bbObject(Sound, h);
	if (!snd) return 0;
	for (int i = 0; i < 32; i++) {
		int busy = snd->channels[i] != 0;
		if (busy && bbChannelPlaying(snd->channels[i])) continue;
		if (!snd->internalHandle) {
			if (bbFileType(snd->name) != 1) sndmsg(cat("Sound \"", snd->name, "\" not found."));
			else if (EnableSFXRelease) snd->internalHandle = bbLoadSound(snd->name);
			if (!snd->internalHandle) sndmsg(cat("Failed to load Sound: \"", snd->name, "\""));
		}
		snd->channels[i] = bbPlaySound((busy ? ConsoleFlush : ConsoleFlushSnd) ? ConsoleFlushSnd : snd->internalHandle);
		bbChannelVolume(snd->channels[i], SFXVolume);
		snd->releaseTime = MilliSecs2() + 5000;
		return snd->channels[i];
	}
	return 0;
}

int LoadSound_Strict(const char *file) {
	Sound *snd = bbNew(Sound);
	snd->name = bbSdup(file);
	if (!EnableSFXRelease) snd->internalHandle = bbLoadSound(snd->name);
	return bbHandle(snd);
}

void FreeSound_Strict(int h) {
	Sound *snd = bbObject(Sound, h);
	if (!snd) return;
	if (snd->internalHandle) bbFreeSound(snd->internalHandle);
	snd->internalHandle = 0;
	bbDelete(snd);
}

// fmod streams -> bbStream*. chn is the stream itself, there are no separate fmod channels
int StreamSound_Strict(const char *file, float volume, int mode) {
	if (bbFileType(file) != 1) { sndmsg(cat("Sound \"", file, "\" not found.")); return 0; }
	Stream *st = bbNew(Stream);
	st->sfx = bbStreamOpen(file, mode == 2);
	if (!st->sfx) { sndmsg(cat("Failed to stream Sound (returned 0): \"", file, "\"")); return 0; }
	bbStreamPlay(st->sfx);
	st->chn = st->sfx;
	bbStreamVolume(st->chn, volume);
	bbStreamPaused(st->chn, 0);
	return bbHandle(st);
}

static Stream *stream(int h, const char *what) {
	Stream *st = bbObject(Stream, h);
	if (!st) { CreateConsoleMsg(cat("Failed to ", what, " stream Sound: Unknown Stream"), -1, -1, -1, 0); return NULL; }
	if (st->chn == 0 || st->chn == -1) { CreateConsoleMsg(cat("Failed to ", what, " stream Sound: Return value ", bbStr(st->chn)), -1, -1, -1, 0); return NULL; }
	return st;
}
void StopStream_Strict(int h) {
	Stream *st = stream(h, "stop");
	if (!st) return;
	bbStreamClose(st->sfx);
	bbDelete(st);
}
void SetStreamVolume_Strict(int h, float v) {
	Stream *st = stream(h, "set stream Sound volume:");
	if (!st) return;
	bbStreamVolume(st->chn, v); bbStreamPaused(st->chn, 0);
}
void SetStreamPaused_Strict(int h, int p) { Stream *st = stream(h, "pause/unpause"); if (st) bbStreamPaused(st->chn, p); }
int IsStreamPlaying_Strict(int h) { Stream *st = stream(h, "find"); return st ? bbStreamPlaying(st->chn) : 0; }
void SetStreamPan_Strict(int h, float pan) { Stream *st = stream(h, "find"); if (st) bbStreamPan(st->chn, pan); }

void UpdateStreamSoundOrigin(int h, ent cam, ent e, float range, float volume) {
	range = Max(range, 1);
	if (volume <= 0) { if (h) SetStreamVolume_Strict(h, 0); return; }
	float dist = bbEntityDistance(cam, e) / range;
	if (1 - dist > 0 && 1 - dist < 1) {
		SetStreamVolume_Strict(h, volume * (1 - dist) * SFXVolume);
		SetStreamPan_Strict(h, sinf(-bbDeltaYaw(cam, e) * DTOR));
	} else SetStreamVolume_Strict(h, 0);
}

ent LoadMesh_Strict(const char *file, ent par) {
	if (bbFileType(file) != 1) RuntimeError(cat("3D Mesh ", file, " not found."));
	ent t = bbLoadMesh(file, par);
	if (!t) RuntimeError(cat("Failed to load 3D Mesh: ", file));
	return t;
}
ent LoadAnimMesh_Strict(const char *file, ent par) {
	if (bbFileType(file) != 1) RuntimeError(cat("3D Animated Mesh ", file, " not found."));
	ent t = bbLoadAnimMesh(file, par);
	if (!t) RuntimeError(cat("Failed to load 3D Animated Mesh: ", file));
	return t;
}
htex LoadTexture_Strict(const char *file, int flags) {
	if (bbFileType(file) != 1) RuntimeError(cat("Texture ", file, " not found."));
	htex t = bbLoadTexture(file, flags + 256 * (EnableVRam != 0));
	if (!t) RuntimeError(cat("Failed to load Texture: ", file));
	return t;
}
int LoadFont_Strict(const char *file, int height, int bold, int italic, int underline) {
	if (bbFileType(file) != 1) RuntimeError(cat("Font ", file, " not found."));
	int t = bbLoadFont(file, height, bold, italic, underline);
	if (!t) RuntimeError(cat("Failed to load Font: ", file));
	return t;
}

int LoadTempSound(const char *file) {
	if (TempSounds[TempSoundIndex]) FreeSound_Strict(TempSounds[TempSoundIndex]);
	int s = TempSounds[TempSoundIndex] = LoadSound_Strict(file);
	TempSoundIndex = (TempSoundIndex + 1) % 10;
	return s;
}
