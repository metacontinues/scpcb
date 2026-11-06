// sndtest: sound/channel/stream behaviour. needs an audio device, no window.
#include <stdio.h>
#include <time.h>
#include "../src/bb/bb.h"
#include "../src/bb/in.h" // snd_update/snd_quit + raylib

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)
static void wait(double s) { // pump like bbFlip would
	for (double t = 0; t < s; t += 0.01) { snd_update(); nanosleep(&(struct timespec){0, 10000000}, NULL); }
}

int main(void) {
	int s = bbLoadSound("SFX\\Interact\\Button.ogg");
	SetMasterVolume(0); // exercise everything, hear nothing
	CHECK(s, "load");
	CHECK(!bbLoadSound("SFX\\nope.ogg"), "missing file -> 0");
	if (!IsAudioDeviceReady()) { // headless/ci: only silent mode is checkable
		CHECK(!bbPlaySound(s) && !bbChannelPlaying(0), "silent: play -> 0 channel");
		CHECK(!bbStreamOpen("SFX\\Music\\079.ogg", 1), "silent: no stream");
		bbFreeSound(s);
		printf("no audio device: checked silent mode only. %s (%d failed)\n", fails ? "FAILED" : "all good", fails);
		return fails != 0;
	}
	int a = bbPlaySound(s), b = bbPlaySound(s);
	CHECK(a && b && a != b, "two voices a=%d b=%d", a, b);
	CHECK(bbChannelPlaying(a) && bbChannelPlaying(b), "both playing");
	bbChannelVolume(a, 0.5f); bbChannelPan(a, -1); bbChannelPitch(a, 22050);
	bbPauseChannel(b);
	CHECK(bbChannelPlaying(b), "paused counts as playing");
	bbStopChannel(a);
	CHECK(!bbChannelPlaying(a), "stopped");
	bbResumeChannel(b);
	wait(3);
	CHECK(!bbChannelPlaying(b), "finished after 3s");
	// stale handles must not steer recycled voices
	int c = 0;
	for (int i = 0; i < 300; i++) c = bbPlaySound(s);
	CHECK(c && !bbChannelPlaying(a), "stale handle a stays dead");
	bbFreeSound(s);
	CHECK(!bbChannelPlaying(c), "FreeSound kills its voices");
	CHECK(!bbPlaySound(s), "play freed sound -> 0");

	int m = bbStreamOpen("SFX\\Music\\079.ogg", 1);
	CHECK(m, "stream open");
	bbStreamPlay(m); bbStreamVolume(m, 0.2f);
	wait(0.3);
	CHECK(bbStreamPlaying(m), "stream playing");
	bbStreamPaused(m, 1);
	CHECK(bbStreamPlaying(m), "paused stream counts");
	bbStreamClose(m);
	CHECK(!bbStreamPlaying(m), "closed");
	snd_quit();
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
