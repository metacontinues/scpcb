// bb.h: blitz3d-alike runtime on raylib. names mirror blitz w/ a bb prefix
// (same as blitz3d's own runtime), so game code ports ~line-by-line.
// handles are ints, 0 = null, like blitz.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include "m3.h"
#include "str.h"
#include "tl.h"

typedef int ent;
typedef int htex;
typedef int hbrush;
typedef struct surf surf;

// ---- tex flags / blend (blitz values)
enum { TF_COLOR = 1, TF_ALPHA = 2, TF_MASK = 4, TF_MIP = 8, TF_CLAMPU = 16, TF_CLAMPV = 32,
       TF_SPHERE = 64, TF_CUBE = 128, TF_VRAM = 256, TF_HICOLOR = 512 };
enum { TB_NONE, TB_ALPHA, TB_MUL, TB_ADD, TB_DOT3, TB_MUL2 };
enum { FX_FULLBRIGHT = 1, FX_VCOLOR = 2, FX_FLAT = 4, FX_NOFOG = 8, FX_2SIDED = 16, FX_VALPHA = 32 };
enum { BL_ALPHA = 1, BL_MUL = 2, BL_ADD = 3 };
#define BB_NTEX 4 // tex layers per brush

// ---- system
int bbGraphics3D(int w, int h, int depth, int mode);
void bbEndGraphics(void);
int bbGraphicsWidth(void);
int bbGraphicsHeight(void);
void bbFlip(int vsync);
void bbCls(void);
void bbClsColor(int r, int g, int b);
int bbMilliSecs(void);
int bbAppClosing(void);
int bbCountGfxDrivers(void);
const char *bbGfxDriverName(int i);
void bbAntiAlias(int on);
int bbDesktopWidth(void);  // 0 before the first Graphics3D
int bbDesktopHeight(void);

// ---- files: blitz paths (backslashes, any case) -> real paths
const char *bbPath(const char *p); // static buf, NULL if missing
FILE *bbReadFile(const char *p);
int bbReadByte(FILE *f);
int bbReadShort(FILE *f);
int bbReadInt(FILE *f);
float bbReadFloat(FILE *f);
str bbReadString(FILE *f); // int len + bytes
str bbReadLine(FILE *f);   // "" at eof, like blitz

// ---- entities
ent bbCreatePivot(ent par);
ent bbCopyEntity(ent e, ent par);
void bbFreeEntity(ent e);
void bbEntityParent(ent e, ent par, int glob);
ent bbGetParent(ent e);
int bbCountChildren(ent e);
ent bbGetChild(ent e, int n); // 1-based
ent bbFindChild(ent e, const char *name);
void bbNameEntity(ent e, const char *name);
const char *bbEntityName(ent e);
void bbHideEntity(ent e);
void bbShowEntity(ent e);
int bbEntityHidden(ent e);
int bbEntityExists(ent e);

void bbPositionEntity(ent e, float x, float y, float z, int glob);
void bbMoveEntity(ent e, float x, float y, float z);
void bbTranslateEntity(ent e, float x, float y, float z, int glob);
void bbRotateEntity(ent e, float p, float y, float r, int glob);
void bbTurnEntity(ent e, float p, float y, float r, int glob);
void bbScaleEntity(ent e, float x, float y, float z, int glob);
void bbPointEntity(ent e, ent t, float roll);
void bbAlignToVector(ent e, float x, float y, float z, int axis, float rate);
float bbEntityX(ent e, int glob);
float bbEntityY(ent e, int glob);
float bbEntityZ(ent e, int glob);
float bbEntityPitch(ent e, int glob);
float bbEntityYaw(ent e, int glob);
float bbEntityRoll(ent e, int glob);
float bbEntityScaleX(ent e, int glob);
float bbEntityScaleY(ent e, int glob);
float bbEntityScaleZ(ent e, int glob);
float bbEntityDistance(ent a, ent b);
float bbDeltaYaw(ent a, ent b);
float bbDeltaPitch(ent a, ent b);
void bbTFormPoint(float x, float y, float z, ent src, ent dst);
void bbTFormVector(float x, float y, float z, ent src, ent dst);
void bbTFormNormal(float x, float y, float z, ent src, ent dst);
float bbTFormedX(void);
float bbTFormedY(void);
float bbTFormedZ(void);
float bbGetMatElement(ent e, int row, int col);

// entity brush stuff
void bbEntityColor(ent e, float r, float g, float b);
void bbEntityAlpha(ent e, float a);
void bbEntityShininess(ent e, float s);
void bbEntityFX(ent e, int fx);
void bbEntityBlend(ent e, int bl);
void bbEntityTexture(ent e, htex t, int frame, int idx);
void bbEntityOrder(ent e, int ord);
void bbPaintEntity(ent e, hbrush b);

// ---- brushes
hbrush bbCreateBrush(float r, float g, float b);
void bbFreeBrush(hbrush b);
void bbBrushColor(hbrush b, float r, float g, float bl);
void bbBrushAlpha(hbrush b, float a);
void bbBrushFX(hbrush b, int fx);
void bbBrushBlend(hbrush b, int bl);
void bbBrushTexture(hbrush b, htex t, int frame, int idx);
htex bbGetBrushTexture(hbrush b, int idx);
hbrush bbGetSurfaceBrush(surf *s);
hbrush bbGetEntityBrush(ent e);

// ---- meshes / surfaces
ent bbCreateMesh(ent par);
ent bbCopyMesh(ent e, ent par);
ent bbLoadMesh(const char *p, ent par);
ent bbLoadAnimMesh(const char *p, ent par);
ent bbCreateCube(ent par);
ent bbCreateSphere(int segs, ent par);
ent bbCreatePlane(int segs, ent par);
void bbAddMesh(ent src, ent dst);
void bbFlipMesh(ent e);
void bbScaleMesh(ent e, float x, float y, float z);
void bbPositionMesh(ent e, float x, float y, float z);
void bbRotateMesh(ent e, float p, float y, float r);
void bbFitMesh(ent e, float x, float y, float z, float w, float h, float d, int uniform);
void bbUpdateNormals(ent e);
void bbPaintMesh(ent e, hbrush b);
float bbMeshWidth(ent e);
float bbMeshHeight(ent e);
float bbMeshDepth(ent e);
int bbCountSurfaces(ent e);
surf *bbGetSurface(ent e, int n); // 1-based
surf *bbCreateSurface(ent e, hbrush b);
void bbPaintSurface(surf *s, hbrush b);
void bbClearSurface(surf *s, int verts, int tris);
int bbAddVertex(surf *s, float x, float y, float z, float u, float v, float w);
int bbAddTriangle(surf *s, int a, int b, int c);
void bbVertexCoords(surf *s, int i, float x, float y, float z);
void bbVertexNormal(surf *s, int i, float x, float y, float z);
void bbVertexColor(surf *s, int i, float r, float g, float b, float a);
void bbVertexTexCoords(surf *s, int i, float u, float v, float w, int set);
int bbCountVertices(surf *s);
int bbCountTriangles(surf *s);
float bbVertexX(surf *s, int i);
float bbVertexY(surf *s, int i);
float bbVertexZ(surf *s, int i);
int bbTriangleVertex(surf *s, int t, int c);

// ---- textures
htex bbLoadTexture(const char *p, int flags);
htex bbLoadAnimTexture(const char *p, int flags, int fw, int fh, int first, int count);
htex bbCreateTexture(int w, int h, int flags, int frames);
void bbFreeTexture(htex t);
void bbTextureBlend(htex t, int bl);
void bbTextureCoords(htex t, int set);
void bbScaleTexture(htex t, float u, float v);
void bbPositionTexture(htex t, float u, float v);
int bbTextureWidth(htex t);
int bbTextureHeight(htex t);
const char *bbTextureName(htex t);
void bbTextureLodBias(float b); // no-op: es2 has no lod bias (would need a shader bias)
void bbTextureFill(htex t, int r, int g, int b, int a); // stand-in for SetBuffer+Cls

// ---- cameras / lights / sprites / render
ent bbCreateCamera(ent par);
void bbCameraRange(ent c, float n, float f);
void bbCameraZoom(ent c, float z);
void bbCameraViewport(ent c, int x, int y, int w, int h);
void bbCameraClsColor(ent c, int r, int g, int b);
void bbCameraClsMode(ent c, int col, int z);
void bbCameraProjMode(ent c, int m);
void bbCameraFogMode(ent c, int m);
void bbCameraFogRange(ent c, float n, float f);
void bbCameraFogColor(ent c, int r, int g, int b);
void bbCameraProject(ent c, float x, float y, float z);
float bbProjectedX(void);
float bbProjectedY(void);
float bbProjectedZ(void);
int bbEntityInView(ent e, ent c);

ent bbCreateLight(int type, ent par);
void bbLightColor(ent l, float r, float g, float b);
void bbLightRange(ent l, float r);
void bbLightConeAngles(ent l, float in, float out);
void bbAmbientLight(float r, float g, float b);

ent bbCreateSprite(ent par);
ent bbLoadSprite(const char *p, int flags, ent par);
void bbScaleSprite(ent s, float x, float y);
void bbHandleSprite(ent s, float x, float y);
void bbRotateSprite(ent s, float a);
void bbSpriteViewMode(ent s, int m);

void bbRenderWorld(float tween);
int bbTrisRendered(void);

// ---- collision / pick fields (logic lands in the collision phase)
void bbEntityType(ent e, int type, int rec);
int bbGetEntityType(ent e);
void bbEntityRadius(ent e, float rx, float ry);
void bbEntityBox(ent e, float x, float y, float z, float w, float h, float d);
void bbEntityPickMode(ent e, int mode, int obscurer);

// ---- collisions (coll.c). methods: 1 sphere, 2 polygon, 3 box. responses: 1 stop, 2 slide, 3 slide w/o going down slopes
void bbCollisions(int src, int dst, int method, int response);
void bbClearCollisions(void);
void bbUpdateWorld(float anim);
void bbResetEntity(ent e);
int bbCountCollisions(ent e);
float bbCollisionX(ent e, int i); // i is 1-based like blitz
float bbCollisionY(ent e, int i);
float bbCollisionZ(ent e, int i);
float bbCollisionNX(ent e, int i);
float bbCollisionNY(ent e, int i);
float bbCollisionNZ(ent e, int i);
float bbCollisionTime(ent e, int i);
ent bbCollisionEntity(ent e, int i);
surf *bbCollisionSurface(ent e, int i);
int bbCollisionTriangle(ent e, int i);
ent bbEntityCollided(ent e, int type);

// ---- picking. pick modes: 1 sphere, 2 polygon, 3 box
ent bbLinePick(float x, float y, float z, float dx, float dy, float dz, float radius);
ent bbEntityPick(ent e, float range);
ent bbCameraPick(ent cam, float x, float y);
int bbEntityVisible(ent a, ent b);
float bbPickedX(void);
float bbPickedY(void);
float bbPickedZ(void);
float bbPickedNX(void);
float bbPickedNY(void);
float bbPickedNZ(void);
float bbPickedTime(void);
ent bbPickedEntity(void);
surf *bbPickedSurface(void);
int bbPickedTriangle(void);

// ---- animation (anim.c). times are in frames, like blitz
void bbSetAnimTime(ent e, float t, int seq);
float bbAnimTime(ent e);
int bbAnimSeq(ent e);
int bbAnimLength(ent e);
int bbAnimating(ent e);
void bbAnimate(ent e, int mode, float speed, int seq, float trans); // mode 0 stop 1 loop 2 pingpong 3 once
int bbExtractAnimSeq(ent e, int first, int last, int seq);
const char *bbEntityClass(ent e);

// ---- sound (snd.c). channels from PlaySound are independent voices; pan -1 left .. 1 right, pitch in hz
int bbLoadSound(const char *p);
void bbFreeSound(int s);
void bbLoopSound(int s);
void bbSoundVolume(int s, float v);
void bbSoundPan(int s, float p);
void bbSoundPitch(int s, float hz);
int bbPlaySound(int s);
void bbStopChannel(int c);
void bbPauseChannel(int c);
void bbResumeChannel(int c);
void bbChannelVolume(int c, float v);
void bbChannelPan(int c, float p);
void bbChannelPitch(int c, float hz);
int bbChannelPlaying(int c);
// music streams, standing in for the FSOUND_Stream_* calls (StreamSound_Strict & co)
int bbStreamOpen(const char *p, int loop); // fmod Mode 2 = loop
void bbStreamPlay(int s);
void bbStreamPaused(int s, int paused);
void bbStreamVolume(int s, float v);
void bbStreamPan(int s, float p);
int bbStreamPlaying(int s);
void bbStreamClose(int s);

// ---- 2d (g2d.c). buffers: BackBuffer/ImageBuffer/TextureBuffer, 0 in CopyRect/pixels = current
int bbBackBuffer(void);
int bbFrontBuffer(void);
int bbImageBuffer(int img, int frame);
int bbTextureBuffer(htex t, int frame);
int bbGraphicsBuffer(void);
void bbSetBuffer(int b);
void bbColor(int r, int g, int b);
int bbColorRed(void);
int bbColorGreen(void);
int bbColorBlue(void);
void bbClsColor(int r, int g, int b);
void bbCls(void);
void bbOrigin(int x, int y);
void bbViewport(int x, int y, int w, int h);
void bbPlot(int x, int y);
void bbLine(int x1, int y1, int x2, int y2);
void bbRect(int x, int y, int w, int h, int solid);
void bbOval(int x, int y, int w, int h, int solid);
int bbLoadFont(const char *p, int height, int bold, int italic, int underline);
void bbFreeFont(int f);
void bbSetFont(int f);
int bbFontHeight(void);
int bbFontWidth(void);
int bbStringWidth(const char *s);
int bbStringHeight(const char *s);
void bbText(int x, int y, const char *s, int cx, int cy);
void bbTextA(int x, int y, const char *s, int cx, int cy, float alpha); // AAText's replacement
int bbLoadImage(const char *p);
int bbLoadAnimImage(const char *p, int fw, int fh, int first, int count);
int bbCreateImage(int w, int h, int frames);
int bbCopyImage(int img);
void bbFreeImage(int img);
int bbImageWidth(int img);
int bbImageHeight(int img);
void bbMaskImage(int img, int r, int g, int b);
void bbHandleImage(int img, float x, float y);
void bbMidHandle(int img);
void bbAutoMidHandle(int on);
void bbResizeImage(int img, float w, float h);
void bbScaleImage(int img, float sx, float sy);
void bbRotateImage(int img, float ang);
void bbDrawImage(int img, int x, int y, int frame);
void bbDrawBlock(int img, int x, int y, int frame);
void bbDrawImageRect(int img, int x, int y, int rx, int ry, int rw, int rh, int frame);
int bbImageRectOverlap(int img, int x, int y, int rx, int ry, int rw, int rh);
void bbCopyRect(int sx, int sy, int w, int h, int dx, int dy, int src, int dst);
void bbLockBuffer(int b);
void bbUnlockBuffer(int b);
int bbReadPixel(int x, int y, int b); // argb; Fast variants are the same thing
void bbWritePixel(int x, int y, int argb, int b);
#define bbReadPixelFast bbReadPixel
#define bbWritePixelFast bbWritePixel
int bbCountGfxModes3D(void);
int bbGfxModeWidth(int i);
int bbGfxModeHeight(int i);
void bbHidePointer(void);
void bbShowPointer(void);
void bbMoveMouse(int x, int y);
void bbAppTitle(const char *t);
int bbTotalVidMem(void);
int bbAvailVidMem(void);

// ---- input (input.c). keys are directinput scancodes like blitz; mouse buttons 1 l, 2 r, 3 m
int bbKeyDown(int sc);
int bbKeyHit(int sc);
void bbFlushKeys(void);
int bbGetKey(void);
int bbWaitKey(void);
int bbMouseDown(int b);
int bbMouseHit(int b);
int bbMouseX(void);
int bbMouseY(void);
int bbMouseXSpeed(void);
int bbMouseYSpeed(void);
int bbMouseZSpeed(void);
void bbFlushMouse(void);

// ---- misc (misc.c). rng is blitz's exact generator, so seeds build the same maps
float bbRnd(float from, float to);
int bbRand(int from, int to);
void bbSeedRnd(int seed);
int bbRndSeed(void);
FILE *bbWriteFile(const char *p);
FILE *bbOpenFile(const char *p);
void bbCloseFile(FILE *f);
int bbEof(FILE *f);
int bbFilePos(FILE *f);
int bbSeekFile(FILE *f, int pos);
void bbWriteByte(FILE *f, int v);
void bbWriteShort(FILE *f, int v);
void bbWriteInt(FILE *f, int v);
void bbWriteFloat(FILE *f, float v);
void bbWriteString(FILE *f, const char *s);
void bbWriteLine(FILE *f, const char *s);
int bbFileType(const char *p);
int bbFileSize(const char *p);
int bbCreateDir(const char *p);
int bbDeleteFile(const char *p);
int bbCopyFile(const char *from, const char *to);
const char *bbCurrentDir(void);
void *bbReadDir(const char *p);
const char *bbNextFile(void *d);
void bbCloseDir(void *d);
int bbDeleteDir(const char *p);
const char *bbCurrentDate(void);
const char *bbCurrentTime(void);
void bbDelay(int ms);
