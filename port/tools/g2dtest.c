// g2dtest: draws every 2d path once + checks pixel io. run from repo root:
//   port/build/g2dtest out.png
#include <stdio.h>
#include "../src/bb/bb.h"
#include "../src/bb/in.h"
#include "rlgl.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)

int main(int argc, char **argv) {
	if (!bbGraphics3D(1024, 600, 0, 2)) return 1;
	bbAppTitle("g2dtest");
	int fnt = bbLoadFont("GFX\\font\\cour\\Courier New.ttf", 22, 0, 0, 0), big = bbLoadFont("GFX\\font\\courbd\\Courier New.ttf", 48, 0, 0, 0);
	CHECK(fnt && big, "fonts");
	CHECK(bbLoadFont("Blitz", 16, 0, 0, 0), "non-file font name falls back");

	// texture drawn into via TextureBuffer, then shown on a cube: text must read, red box top-left
	htex t = bbCreateTexture(256, 256, TF_COLOR, 1);
	bbSetBuffer(bbTextureBuffer(t, 0));
	bbClsColor(40, 40, 120); bbCls();
	bbColor(255, 0, 0); bbRect(0, 0, 64, 64, 1);
	bbSetFont(big); bbColor(255, 255, 255); bbText(128, 128, "TEX", 1, 1);
	bbSetBuffer(bbBackBuffer());
	ent cam = bbCreateCamera(0); bbCameraViewport(cam, 0, 0, 1024, 600);
	bbCameraClsColor(cam, 30, 30, 30);
	ent cube = bbCreateCube(0); bbPositionEntity(cube, -1.4f, 0, 4, 0); bbEntityTexture(cube, t, 0, 0); bbEntityFX(cube, 1);

	// image drawn into via ImageBuffer
	int im = bbCreateImage(160, 60, 1);
	bbSetBuffer(bbImageBuffer(im, 0));
	bbClsColor(0, 0, 0); bbCls(); // black = masked
	bbColor(0, 200, 0); bbOval(0, 0, 160, 60, 1);
	bbSetFont(fnt); bbColor(0, 0, 0); bbColor(10, 10, 10); bbText(80, 30, "IMAGE", 1, 1);
	bbSetBuffer(bbBackBuffer());
	CHECK(bbImageWidth(im) == 160 && bbImageHeight(im) == 60, "image size");
	int px = bbReadPixel(80, 2, bbImageBuffer(im, 0)), corner = bbReadPixel(0, 0, bbImageBuffer(im, 0));
	CHECK((px & 0xffffff) == 0x00c800, "readpixel oval top-center = %06x", px & 0xffffff);
	CHECK((corner & 0xffffff) == 0, "readpixel corner black = %06x", corner & 0xffffff);
	bbWritePixel(1, 1, 0xff123456, bbImageBuffer(im, 0));
	CHECK((bbReadPixel(1, 1, bbImageBuffer(im, 0)) & 0xffffff) == 0x123456, "write/read pixel");
	int icon = bbLoadImage("GFX\\BlinkIcon.png");
	CHECK(icon, "load image");
	bbMidHandle(icon);

	htex cap = bbCreateTexture(256, 256, TF_COLOR, 1);
	ent cube2 = bbCreateCube(0); bbPositionEntity(cube2, 1.4f, 0, 4, 0); bbEntityTexture(cube2, cap, 0, 0); bbEntityFX(cube2, 1);
	for (int f = 0; f < 4; f++) {
		bbTurnEntity(cube, 0, 0, 0, 0);
		bbRenderWorld(1);
		bbSetFont(fnt); bbColor(255, 255, 0);
		bbText(10, 10, "Text top-left (Courier 22)", 0, 0);
		bbTextA(512, 560, "centered, alpha .5", 1, 1, 0.5f);
		bbColor(255, 255, 255); bbRect(10, 40, 100, 50, 0); bbColor(0, 120, 255); bbRect(120, 40, 100, 50, 1);
		bbColor(255, 0, 255); bbLine(230, 40, 330, 90); bbOval(340, 40, 100, 50, 0);
		bbDrawImage(im, 460, 35, 0);
		bbDrawImage(icon, 700, 65, 0);
		bbSetFont(big); bbColor(255, 255, 255); bbText(10, 480, "2D OK?", 0, 0);
		// grab the top-left of the screen into a texture, show it on cube2 (must be upright)
		bbCopyRect(0, 0, 256, 256, 0, 0, bbBackBuffer(), bbTextureBuffer(cap, 0));
		bbFlip(1);
	}
	bbRenderWorld(1);
	bbSetFont(fnt); bbColor(255, 255, 0); bbText(10, 10, "Text top-left (Courier 22)", 0, 0);
	bbColor(255, 255, 255); bbRect(10, 40, 100, 50, 0); bbColor(0, 120, 255); bbRect(120, 40, 100, 50, 1);
	bbColor(255, 0, 255); bbLine(230, 40, 330, 90); bbOval(340, 40, 100, 50, 0);
	bbDrawImage(im, 460, 35, 0); bbDrawImage(icon, 700, 65, 0);
	bbSetFont(big); bbColor(255, 255, 255); bbText(10, 480, "2D OK?", 0, 0);
	int w = GetRenderWidth(), h = GetRenderHeight();
	g2d_end(); rlDrawRenderBatchActive();
	Image s = {rlReadScreenPixels(w, h), w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	if (argc > 1) ExportImage(s, argv[1]);
	UnloadImage(s);
	bbFlip(1);
	bbEndGraphics();
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
