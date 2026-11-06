// bb.h: blitz3d-alike runtime on raylib. names mirror blitz w/ a bb prefix
// (same as blitz3d's own runtime), so game code ports ~line-by-line.
// handles are ints, 0 = null, like blitz.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include "m3.h"

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

// ---- files: blitz paths (backslashes, any case) -> real paths
const char *bbPath(const char *p); // static buf, NULL if missing
FILE *bbReadFile(const char *p);
int bbReadByte(FILE *f);
int bbReadShort(FILE *f);
int bbReadInt(FILE *f);
float bbReadFloat(FILE *f);
char *bbReadString(FILE *f); // malloc'd, int len + bytes
char *bbReadLine(FILE *f);   // malloc'd, NULL at eof

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
htex bbCreateTexture(int w, int h, int flags, int frames);
void bbFreeTexture(htex t);
void bbTextureBlend(htex t, int bl);
void bbTextureCoords(htex t, int set);
void bbScaleTexture(htex t, float u, float v);
void bbPositionTexture(htex t, float u, float v);
int bbTextureWidth(htex t);
int bbTextureHeight(htex t);
const char *bbTextureName(htex t);
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
