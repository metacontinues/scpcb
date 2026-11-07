// sndtest: sound/channel/stream behaviour. needs an audio device, no window.
#include <stdio.h>
#include <time.h>
#include "../src/rt/rt.h"
#include "../src/rt/in.h" // snd_update/snd_quit + raylib

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)
static void wait(double s) { // pump like flip_frame would
	for (double t = 0; t < s; t += 0.01) { snd_update(); nanosleep(&(struct timespec){0, 10000000}, NULL); }
}

int main(void) {
	int s = load_sound("SFX\\Interact\\Button.ogg");
	SetMasterVolume(0); // exercise everything, hear nothing
	CHECK(s, "load");
	CHECK(!load_sound("SFX\\nope.ogg"), "missing file -> 0");
	if (!IsAudioDeviceReady()) { // headless/ci: only silent mode is checkable
		CHECK(!play_sound(s) && !channel_playing(0), "silent: play -> 0 channel");
		CHECK(!stream_open("SFX\\Music\\079.ogg", 1), "silent: no stream");
		free_sound(s);
		printf("no audio device: checked silent mode only. %s (%d failed)\n", fails ? "FAILED" : "all good", fails);
		return fails != 0;
	}
	int a = play_sound(s), b = play_sound(s);
	CHECK(a && b && a != b, "two voices a=%d b=%d", a, b);
	CHECK(channel_playing(a) && channel_playing(b), "both playing");
	channel_volume(a, 0.5f); channel_pan(a, -1); channel_pitch(a, 22050);
	pause_channel(b);
	CHECK(channel_playing(b), "paused counts as playing");
	stop_channel(a);
	CHECK(!channel_playing(a), "stopped");
	resume_channel(b);
	wait(3);
	CHECK(!channel_playing(b), "finished after 3s");
	// stale handles must not steer recycled voices
	int c = 0;
	for (int i = 0; i < 300; i++) c = play_sound(s);
	CHECK(c && !channel_playing(a), "stale handle a stays dead");
	free_sound(s);
	CHECK(!channel_playing(c), "FreeSound kills its voices");
	CHECK(!play_sound(s), "play freed sound -> 0");

	int m = stream_open("SFX\\Music\\079.ogg", 1);
	CHECK(m, "stream open");
	stream_play(m); stream_volume(m, 0.2f);
	wait(0.3);
	CHECK(stream_playing(m), "stream playing");
	stream_paused(m, 1);
	CHECK(stream_playing(m), "paused stream counts");
	stream_close(m);
	CHECK(!stream_playing(m), "closed");
	snd_quit();
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
