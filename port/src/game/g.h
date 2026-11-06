// g.h: everything the game shares. blitz has one global namespace, so this does too.
// globals are G(type, names...) so they're declared once: main.c defines G_DEFINE first.
// porting rules: float -> int goes through bbInt (blitz rounds), Min/Max are float like blitz,
// Dim x(n) is x[n+1], True/False are 1/0, strings are str (gc'd, see bb/str.h).
// blitz arrays a[n] (Field/Global/Local) have n+1 slots like Dim, so they're a[n+1] here too.
// And/Or are bitwise in blitz: on raw ints use & and |, && || only where both sides are 0/1.
// game names that clash w/ raylib's exported symbols get a trailing _ (LoadMaterials_).
// str globals/fields start NULL, not "": test them w/ bbLen/bbSeq, never *s.
#pragma once
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../bb/bb.h"

#ifdef G_DEFINE
#define G(T, ...) T __VA_ARGS__
#else
#define G(T, ...) extern T __VA_ARGS__
#endif

static inline float Min(float a, float b) { return a < b ? a : b; }
static inline float Max(float a, float b) { return a > b ? a : b; }
_Noreturn void RuntimeError(const char *msg);
int MilliSecs2(void);
#define End() exit(0)

// ---- Main.bb: setup
enum { HIT_MAP = 1, HIT_PLAYER, HIT_ITEM, HIT_APACHE, HIT_178, HIT_DEAD };
G(str, OptionFile, ErrorFile, VersionNumber, CompatibleNumber);
G(int, UpdaterFont, Font1, Font2, Font3, Font4, Font5, ConsoleFont, CreditsFont, CreditsFont2);
G(int, MenuWhite, MenuBlack, ButtonSFX);
G(int, EnableSFXRelease, EnableSFXRelease_Prev, CanOpenConsole);
G(int, ArrowIMG[5]);
G(int, LauncherWidth, LauncherHeight, LauncherEnabled, LauncherIMG);
G(int, GraphicWidth, GraphicHeight, Depth, Fullscreen, SelectedGFXMode, SelectedGFXDriver);
G(int, fresize_image, fresize_texture, fresize_texture2, fresize_cam);
G(int, ShowFPS, WireframeState, HalloweenTex);
G(int, TotalGFXModes, GFXModes, *GfxModeWidths, *GfxModeHeights);
G(int, BorderlessWindowed, RealGraphicWidth, RealGraphicHeight);
G(float, AspectRatioRatio);
G(int, EnableRoomLights, TextureDetails, ConsoleOpening, Bit16Mode);
G(float, TextureFloat, SFXVolume, MenuScale);
G(int, CurTime, PrevTime, LoopDelay);
G(float, FPSfactor, FPSfactor2, PrevFPSFactor);
G(int, Framelimit, Vsync, Opt_AntiAlias);
G(float, CurrFrameLimit, ScreenGamma);
G(int, GameSaved, CanSave);
G(int, CursorIMG, LoadingScreenAmount, LoadingScreenText, LoadingBack, BlinkMeterIMG);
G(int, viewport_center_x, viewport_center_y);
G(float, mouselook_x_inc, mouselook_y_inc);
G(int, mouse_left_limit, mouse_right_limit, mouse_top_limit, mouse_bottom_limit);
G(float, mouse_x_speed_1, mouse_y_speed_1);
G(int, KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_BLINK, KEY_SPRINT, KEY_INV, KEY_CROUCH, KEY_SAVE, KEY_CONSOLE);
G(float, MouseSmooth);
G(float, Mesh_MinX, Mesh_MinY, Mesh_MinZ, Mesh_MaxX, Mesh_MaxY, Mesh_MaxZ, Mesh_MagX, Mesh_MagY, Mesh_MagZ);
// ---- Main.bb: player state
G(float, KillTimer, FallTimer, DeathTimer, Sanity, ForceMove, ForceAngle);
G(int, KillAnim, RestoreSanity, Playable);
G(float, BLINKFREQ, BlinkTimer, EyeIrritation, EyeStuck, BlinkEffect, BlinkEffectTimer);
G(float, Stamina, StaminaEffect, StaminaEffectTimer, CameraShakeTimer, VomitTimer);
G(int, Vomit, Regurgitate);
G(float, SCP1025state[7], HeartBeatRate, HeartBeatTimer, HeartBeatVolume);
G(int, WearingGasMask, WearingHazmat, WearingVest, Wearing714, WearingNightVision);
G(float, NVTimer, SuperManTimer, Injuries, Bloodloss, Infect, HealTimer);
G(int, SuperMan, RefinedItems);
G(float, DropSpeed, HeadDropSpeed, CurrSpeed, user_camera_pitch, side, CrouchState);
G(int, Crouch, PlayerZone, GrabbedEntity, InvertMouse);
G(int, MouseHit1, MouseDown1, MouseHit2, DoubleClick, LastMouseHit1, MouseUp1);
G(int, GodMode, NoClip);
G(float, NoClipSpeed, CoffinDistance, PlayerSoundVolume, Shake, ExplosionTimer);
G(int, ExplosionSFX, LightsOn, SoundTransmission);
G(int, MainMenuOpen, MenuOpen, InvOpen, EndingScreen);
G(float, StopHidingTimer, EndingTimer, MsgTimer, KeypadTimer);
G(str, SelectedEnding, Msg, DeathMSG, KeypadInput, KeypadMSG);
G(int, AccessCode, DrawHandIcon, DrawArrowIcon[5]);
G(float, MTFtimer, RadioState[11]);
G(int, MTFroomState[11], RadioState3[11], RadioState4[10], RadioCHN[9], OldAiPics[6]);
G(int, PlayTime, ConsoleFlush, ConsoleFlushSnd, ConsoleMusFlush, ConsoleMusPlay);
G(int, InfiniteStamina, NVBlink, IsNVGBlinking);
// later in Main.bb, needed early
G(int, HUDenabled, EnableVRam, BumpEnabled, AATextEnable, UpdateCheckEnabled, AchvMSGenabled);
G(float, MouseSens, MusicVolume);

// ---- console (Main.bb)
typedef struct ConsoleMsg { lnode n; str txt; int isCommand, r, g, b; } ConsoleMsg;
TLIST_DECL(ConsoleMsg);
G(int, ConsoleOpen, ConsoleScrollDragging, ConsoleMouseMem, ConsoleR, ConsoleG, ConsoleB);
G(str, ConsoleInput);
G(float, ConsoleScroll);
G(ConsoleMsg *, ConsoleReissue);
void CreateConsoleMsg(const char *txt, int r, int g, int b, int isCommand);

// ---- Main.bb misc fns
int MouseOn(int x, int y, int w, int h);
int ScaledMouseX(void);
int ScaledMouseY(void);
void Graphics3DExt(int w, int h, int depth, int mode);
int ResizeImage2(int img, int w, int h);

// ---- ini.c
str GetINIString(const char *file, const char *section, const char *par, const char *def);
int GetINIInt(const char *file, const char *section, const char *par, int def);
float GetINIFloat(const char *file, const char *section, const char *par, float def);
str GetINIString2(const char *file, int start, const char *par, const char *def);
int GetINIInt2(const char *file, int start, const char *par, const char *def);
int GetINISectionLocation(const char *file, const char *section);
int PutINIValue(const char *file, const char *section, const char *key, const char *value);
void UpdateINIFile(const char *name);

// ---- strict.c (StrictLoads.bb). Sound handles are Handle(Sound), like blitz
typedef struct Sound { lnode n; int internalHandle; str name; int channels[32], releaseTime; } Sound;
TLIST_DECL(Sound);
enum { FMOD_MODE = 2 }; // FMod.bb's Mode const
int LoadImage_Strict(const char *file);
int LoadSound_Strict(const char *file);
int PlaySound_Strict(int h);
void FreeSound_Strict(int h);
void AutoReleaseSounds(void);
int StreamSound_Strict(const char *file, float volume, int mode);
void StopStream_Strict(int h);
void SetStreamVolume_Strict(int h, float v);
void SetStreamPaused_Strict(int h, int p);
int IsStreamPlaying_Strict(int h);
void SetStreamPan_Strict(int h, float pan);
void UpdateStreamSoundOrigin(int h, ent cam, ent e, float range, float volume);
ent LoadMesh_Strict(const char *file, ent par);
ent LoadAnimMesh_Strict(const char *file, ent par);
htex LoadTexture_Strict(const char *file, int flags);
int LoadFont_Strict(const char *file, int height, int bold, int italic, int underline);
G(int, TempSounds[11], TempSoundIndex);
int LoadTempSound(const char *file); // Main.bb, ring of 10 one-shot sounds
void UpdateMusic(void); // music.c
void InitMusic(void);
void CollectGfxModes(void);

// ---- AAText.bb: raylib text is already smooth, so these are thin
static inline int AALoadFont(const char *f, int h, int b, int i, int u) { return bbLoadFont(f, h, b, i, u); }
static inline void AASetFont(int f) { bbSetFont(f); }
static inline int AAStringWidth(const char *s) { return bbStringWidth(s); }
static inline int AAStringHeight(const char *s) { return bbStringHeight(s); }
static inline void AAText(int x, int y, const char *s, int cx, int cy) { bbTextA(x, y, s, cx, cy, 1); }
static inline void AATextA(int x, int y, const char *s, int cx, int cy, float a) { bbTextA(x, y, s, cx, cy, a); }
static inline void InitAAFont(void) {}

// ---- KeyName.bb
G(str, KeyName[212]);
void InitKeyName(void);

// ---- Difficulty.bb
enum { SAFE, EUCLID, KETER, CUSTOM };
enum { SAVEANYWHERE, SAVEONQUIT, SAVEONSCREENS };
enum { EASY, NORMAL, HARD };
typedef struct Difficulty { lnode n; str name, description; int permaDeath, aggressiveNPCs, saveType, otherFactors, r, g, b, customizable; } Difficulty;
TLIST_DECL(Difficulty);
G(Difficulty *, difficulties[5]);
G(Difficulty *, SelectedDifficulty);
void InitDifficulties(void);

// ---- Achievements.bb
enum { MAXACHIEVEMENTS = 37 };
G(int, Achievements[MAXACHIEVEMENTS + 1], AchvIMG[MAXACHIEVEMENTS + 1], AchvLocked, UsedConsole, AchievementsMenu, CurrAchvMSGID);
G(str, AchievementStrings[MAXACHIEVEMENTS + 1], AchievementDescs[MAXACHIEVEMENTS + 1]);
void InitAchievements(void);

// ---- Menu.bb
typedef struct LoadingScreens { lnode n; str imgpath; int img, ID; str title; int alignx, aligny, disablebackground; str txt[5]; int txtamount; } LoadingScreens;
TLIST_DECL(LoadingScreens);
G(LoadingScreens *, SelectedLoadingScreen);
G(int, SelectedInputBox);
void UpdateLauncher(void);
void InitLoadingScreens(const char *file);
void DrawLoading(int percent, int shortloading);
void DrawTiledImageRect(int img, int sx, int sy, float sw, float sh, int x, int y, int w, int h);
str rInput(const char *s);
str InputBox(int x, int y, int w, int h, const char *txt, int id);
void DrawFrame(int x, int y, int w, int h, int xo, int yo);
int DrawButton(int x, int y, int w, int h, const char *txt, int bigfont, int waitForMouseUp, int usingAA);
int DrawTick(int x, int y, int selected, int locked);
void RowText(const char *a, int x, int y, int w, int h, int align, float leading);
int LimitText(const char *txt, int x, int y, int w, int usingAA);
int DrawButton2(int x, int y, int w, int h, const char *txt, int bigfont);
float SlideBar(int x, int y, int w, float value);
int SliderN(int x, int y, int w, int value, int id, int n, const char **vals);
#define Slider3(x, y, w, v, id, a, b, c) SliderN(x, y, w, v, id, 3, (const char *[]){a, b, c})
#define Slider4(x, y, w, v, id, a, b, c, d) SliderN(x, y, w, v, id, 4, (const char *[]){a, b, c, d})
#define Slider5(x, y, w, v, id, a, b, c, d, e) SliderN(x, y, w, v, id, 5, (const char *[]){a, b, c, d, e})
#define Slider7(x, y, w, v, id, a, b, c, d, e, f, g) SliderN(x, y, w, v, id, 7, (const char *[]){a, b, c, d, e, f, g})
int Button(int x, int y, int w, int h, const char *txt, int disabled);
float DrawScrollBar(int x, int y, int w, int h, int bx, int by, int bw, int bh, float bar, int dir);
int GetLineAmount(const char *a, int w, int h, float leading);
void DrawTooltip(const char *msg);
void DrawOptionsTooltip(int x, int y, int w, int h, const char *option, float value, int ingame);
void DrawMapCreatorTooltip(int x, int y, int w, int h, const char *mapname);
G(int, OnSliderID, OnBar);
G(float, ScrollBarY, ScrollMenuHeight);

// ---- Menu.bb main menu (mainmenu.c, options.c)
G(int, MenuBack, MenuText, Menu173, QuickLoadIcon, MenuBlinkTimer[3], MenuBlinkDuration[3], MenuStrX, MenuStrY, MainMenuTab, IntroEnabled);
G(str, RandomSeed, MenuStr, SavePath, SaveMSG, CurrSave, SelectedMap);
G(int, SaveGameAmount, SavedMapsAmount, CurrLoadGamePage, QuickLoadPercent);
G(float, QuickLoadPercent_DisplayTimer);
G(str, *SaveGames, *SaveGameTime, *SaveGameDate, *SaveGameVersion, *SavedMaps, *SavedMapsAuthor);
void InitMenu(void);
void UpdateMainMenu(void);
void UpdateOptionsTab(int x, int y, int w);
void DrawQuickLoading(void);

// ---- Save.bb (save.c)
void LoadSaveGames(void);
void LoadSavedMaps(void);

// ---- Main.bb: later globals (1525-2809), music (music.c), the loop (loop.c)
G(float, BlurVolume, BlurTimer, LightBlink, LightFlash, CameraShake, CurrCameraZoom, CameraFogNear, CameraFogFar, StoredCameraFogFar);
G(int, Camera, Brightness);
G(str, Music[41]);
G(int, CurrMusicStream, MusicCHN, NowPlaying, ShouldPlay, CurrMusic, PlayCustomMusic, CustomMusic);
G(float, CurrMusicVolume, PrevMusicVolume, PrevSFXVolume);
G(int, NVGImages, NavImages[6], NavBG, UserTrackCheck, UserTrackCheck2, UserTrackMusicAmount, CurrUserTrack, UserTrackFlag);
G(int, EnableUserTracks, UserTrackMode, OptionsMenu, QuitMSG, InFacility, DeafPlayer, IsZombie, ParticleAmount, NoTarget, Wearing1499);
G(float, DeafTimer, menuroomscale, MonitorTimer, MonitorTimer2, PrevInjuries, PrevBloodloss);
G(int, UnableToMove, ShouldEntitiesFall, MTF_CameraCheckDetected);
G(float, PlayerFallingPickDistance, Save_MSG_Timer, Save_MSG_Y, MTF_CameraCheckTimer, UpdateParticles_Time, Input_ResetTime);
G(str, Save_MSG, CurrMenu_TestIMG);
G(int, EndBreathSFX, EndBreathCHN, FPS);
void InitLateGlobals(void);
void MainLoop(void);
int GenerateSeedNumber(const char *seed);
void SaveOptionsINI(void);
// not ported yet: these say so and keep you in the menu
int StartGame(const char *how);

#include "map.h"
