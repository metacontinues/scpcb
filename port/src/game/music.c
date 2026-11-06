// music.c: Main.bb's music list + UpdateMusic
#include "g.h"

void InitMusic(void) {
	static const char *m[] = {"The Dread", "HeavyContainment", "EntranceZone", "PD", "079", "GateB1", "GateB2", "Room3Storage",
		"Room049", "8601", "106", "Menu", "8601Cancer", "Intro", "178", "PDTrench", "205", "GateA", "1499", "1499Danger",
		"049Chase", "..\\Ending\\MenuBreath", "914", "Ending", "Credits", "SaveMeFrom"};
	for (int i = 0; i < 26; i++) Music[i] = (str)m[i];
	MusicVolume = GetINIFloat(OptionFile, "audio", "music volume", 0);
	MusicCHN = StreamSound_Strict(cat("SFX\\Music\\", Music[2], ".ogg"), MusicVolume, FMOD_MODE);
	CurrMusicVolume = 1; NowPlaying = 2; ShouldPlay = 11; CurrMusic = 1;
}

void UpdateMusic(void) {
	if (ConsoleFlush) {
		if (!bbChannelPlaying(ConsoleMusPlay)) ConsoleMusPlay = bbPlaySound(ConsoleMusFlush);
		return;
	}
	if (PlayCustomMusic) {
		if (FPSfactor > 0 || OptionsMenu == 2) {
			if (!bbChannelPlaying(MusicCHN)) MusicCHN = PlaySound_Strict(CustomMusic);
			bbChannelVolume(MusicCHN, MusicVolume);
		}
		return;
	}
	if (NowPlaying != ShouldPlay) { // wrong clip, fade out
		CurrMusicVolume = Max(CurrMusicVolume - FPSfactor / 250.0f, 0);
		if (CurrMusicVolume == 0) {
			if (NowPlaying < 66) StopStream_Strict(MusicCHN);
			NowPlaying = ShouldPlay;
			MusicCHN = 0; CurrMusic = 0;
		}
	} else CurrMusicVolume += (MusicVolume - CurrMusicVolume) * (.1f * FPSfactor);
	if (NowPlaying < 66) {
		if (CurrMusic == 0) { MusicCHN = StreamSound_Strict(cat("SFX\\Music\\", Music[NowPlaying], ".ogg"), 0, FMOD_MODE); CurrMusic = 1; }
		SetStreamVolume_Strict(MusicCHN, CurrMusicVolume);
	}
}
