// strict.c: StrictLoads.bb. loaders that RuntimeError on missing files, Sound/Stream wrappers
#include "g.h"

TLIST(Sound);
typedef struct Stream { lnode n; int sfx, chn; } Stream;
static TLIST(Stream);

int LoadImage_Strict(const char *file) {
	if (file_type(file) != 1) RuntimeError(cat("Image \"", file, "\" missing. "));
	return load_image(file);
}

void AutoReleaseSounds(void) {
	each(Sound, snd) {
		int tryRelease = 1;
		for (int i = 0; i < 32; i++)
			if (snd->channels[i] && channel_playing(snd->channels[i])) { tryRelease = 0; snd->releaseTime = MilliSecs2() + 5000; break; }
		if (tryRelease && snd->releaseTime < MilliSecs2() && snd->internalHandle) { free_sound(snd->internalHandle); snd->internalHandle = 0; }
	}
}

static void sndmsg(const char *m) { CreateConsoleMsg(m, -1, -1, -1, 0); if (ConsoleOpening) ConsoleOpen = 1; }

// the original has two copies of this per slot (free vs finished channel); they only differ in
// ConsoleFlush vs ConsoleFlushSnd as the flush check, kept that way
int PlaySound_Strict(int h) {
	Sound *snd = lobject(Sound, h);
	if (!snd) return 0;
	for (int i = 0; i < 32; i++) {
		int busy = snd->channels[i] != 0;
		if (busy && channel_playing(snd->channels[i])) continue;
		if (!snd->internalHandle) {
			if (file_type(snd->name) != 1) sndmsg(cat("Sound \"", snd->name, "\" not found."));
			else if (EnableSFXRelease) snd->internalHandle = load_sound(snd->name);
			if (!snd->internalHandle) sndmsg(cat("Failed to load Sound: \"", snd->name, "\""));
		}
		snd->channels[i] = play_sound((busy ? ConsoleFlush : ConsoleFlushSnd) ? ConsoleFlushSnd : snd->internalHandle);
		channel_volume(snd->channels[i], SFXVolume);
		snd->releaseTime = MilliSecs2() + 5000;
		return snd->channels[i];
	}
	return 0;
}

int LoadSound_Strict(const char *file) {
	Sound *snd = lnew(Sound);
	sset(&snd->name, s_dup(file));
	if (!EnableSFXRelease) snd->internalHandle = load_sound(snd->name);
	return lhandle(snd);
}

void FreeSound_Strict(int h) {
	Sound *snd = lobject(Sound, h);
	if (!snd) return;
	if (snd->internalHandle) free_sound(snd->internalHandle);
	snd->internalHandle = 0;
	ldel(snd);
}

// fmod streams -> stream*. chn is the stream itself, there are no separate fmod channels
int StreamSound_Strict(const char *file, float volume, int mode) {
	if (file_type(file) != 1) { sndmsg(cat("Sound \"", file, "\" not found.")); return 0; }
	Stream *st = lnew(Stream);
	st->sfx = stream_open(file, mode == 2);
	if (!st->sfx) { sndmsg(cat("Failed to stream Sound (returned 0): \"", file, "\"")); return 0; }
	stream_play(st->sfx);
	st->chn = st->sfx;
	stream_volume(st->chn, volume);
	stream_paused(st->chn, 0);
	return lhandle(st);
}

static Stream *stream(int h, const char *what) {
	Stream *st = lobject(Stream, h);
	if (!st) { CreateConsoleMsg(cat("Failed to ", what, " stream Sound: Unknown Stream"), -1, -1, -1, 0); return NULL; }
	if (st->chn == 0 || st->chn == -1) { CreateConsoleMsg(cat("Failed to ", what, " stream Sound: Return value ", s_str(st->chn)), -1, -1, -1, 0); return NULL; }
	return st;
}
void StopStream_Strict(int h) {
	Stream *st = stream(h, "stop");
	if (!st) return;
	stream_close(st->sfx);
	ldel(st);
}
void SetStreamVolume_Strict(int h, float v) {
	Stream *st = stream(h, "set stream Sound volume:");
	if (!st) return;
	stream_volume(st->chn, v); stream_paused(st->chn, 0);
}
void SetStreamPaused_Strict(int h, int p) { Stream *st = stream(h, "pause/unpause"); if (st) stream_paused(st->chn, p); }
int IsStreamPlaying_Strict(int h) { Stream *st = stream(h, "find"); return st ? stream_playing(st->chn) : 0; }
void SetStreamPan_Strict(int h, float pan) { Stream *st = stream(h, "find"); if (st) stream_pan(st->chn, pan); }

void UpdateStreamSoundOrigin(int h, ent cam, ent e, float range, float volume) {
	range = Max(range, 1);
	if (volume <= 0) { if (h) SetStreamVolume_Strict(h, 0); return; }
	float dist = entity_distance(cam, e) / range;
	if (1 - dist > 0 && 1 - dist < 1) {
		SetStreamVolume_Strict(h, volume * (1 - dist) * SFXVolume);
		SetStreamPan_Strict(h, sinf(-delta_yaw(cam, e) * DTOR));
	} else SetStreamVolume_Strict(h, 0);
}

ent LoadMesh_Strict(const char *file, ent par) {
	if (file_type(file) != 1) RuntimeError(cat("3D Mesh ", file, " not found."));
	ent t = load_mesh(file, par);
	if (!t) RuntimeError(cat("Failed to load 3D Mesh: ", file));
	return t;
}
ent LoadAnimMesh_Strict(const char *file, ent par) {
	if (file_type(file) != 1) RuntimeError(cat("3D Animated Mesh ", file, " not found."));
	ent t = load_anim_mesh(file, par);
	if (!t) RuntimeError(cat("Failed to load 3D Animated Mesh: ", file));
	return t;
}
htex LoadTexture_Strict(const char *file, int flags) {
	if (file_type(file) != 1) RuntimeError(cat("Texture ", file, " not found."));
	htex t = load_texture(file, flags + 256 * (EnableVRam != 0));
	if (!t) RuntimeError(cat("Failed to load Texture: ", file));
	return t;
}
int LoadFont_Strict(const char *file, int height, int bold, int italic, int underline) {
	if (file_type(file) != 1) RuntimeError(cat("Font ", file, " not found."));
	int t = load_font(file, height, bold, italic, underline);
	if (!t) RuntimeError(cat("Failed to load Font: ", file));
	return t;
}

int LoadTempSound(const char *file) {
	if (TempSounds[TempSoundIndex]) FreeSound_Strict(TempSounds[TempSoundIndex]);
	int s = TempSounds[TempSoundIndex] = LoadSound_Strict(file);
	TempSoundIndex = (TempSoundIndex + 1) % 10;
	return s;
}
