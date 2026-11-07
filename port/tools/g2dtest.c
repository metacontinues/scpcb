// g2dtest: draws every 2d path once + checks pixel io. run from repo root:
//   port/build/g2dtest out.png
#include <stdio.h>
#include "../src/rt/rt.h"
#include "../src/rt/in.h"
#include "rlgl.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)

int main(int argc, char **argv) {
	if (!graphics_3d(1024, 600, 0, 2)) return 1;
	app_title("g2dtest");
	int fnt = load_font("GFX\\font\\cour\\Courier New.ttf", 22, 0, 0, 0), big = load_font("GFX\\font\\courbd\\Courier New.ttf", 48, 0, 0, 0);
	CHECK(fnt && big, "fonts");
	CHECK(load_font("Blitz", 16, 0, 0, 0), "non-file font name falls back");

	// texture drawn into via TextureBuffer, then shown on a cube: text must read, red box top-left
	htex t = create_texture(256, 256, TF_COLOR, 1);
	set_buffer(texture_buffer(t, 0));
	clear_color(40, 40, 120); clear_screen();
	set_color(255, 0, 0); draw_rect(0, 0, 64, 64, 1);
	set_font(big); set_color(255, 255, 255); draw_text(128, 128, "TEX", 1, 1);
	set_buffer(back_buffer());
	ent cam = create_camera(0); camera_viewport(cam, 0, 0, 1024, 600);
	camera_cls_color(cam, 30, 30, 30);
	ent cube = create_cube(0); position_entity(cube, -1.4f, 0, 4, 0); entity_texture(cube, t, 0, 0); entity_fx(cube, 1);

	// image drawn into via ImageBuffer
	int im = create_image(160, 60, 1);
	set_buffer(image_buffer(im, 0));
	clear_color(0, 0, 0); clear_screen(); // black = masked
	set_color(0, 200, 0); draw_oval(0, 0, 160, 60, 1);
	set_font(fnt); set_color(0, 0, 0); set_color(10, 10, 10); draw_text(80, 30, "IMAGE", 1, 1);
	set_buffer(back_buffer());
	CHECK(image_width(im) == 160 && image_height(im) == 60, "image size");
	int px = read_pixel(80, 2, image_buffer(im, 0)), corner = read_pixel(0, 0, image_buffer(im, 0));
	CHECK((px & 0xffffff) == 0x00c800, "readpixel oval top-center = %06x", px & 0xffffff);
	CHECK((corner & 0xffffff) == 0, "readpixel corner black = %06x", corner & 0xffffff);
	write_pixel(1, 1, 0xff123456, image_buffer(im, 0));
	CHECK((read_pixel(1, 1, image_buffer(im, 0)) & 0xffffff) == 0x123456, "write/read pixel");
	int icon = load_image("GFX\\BlinkIcon.png");
	CHECK(icon, "load image");
	mid_handle(icon);

	htex cap = create_texture(256, 256, TF_COLOR, 1);
	ent cube2 = create_cube(0); position_entity(cube2, 1.4f, 0, 4, 0); entity_texture(cube2, cap, 0, 0); entity_fx(cube2, 1);
	for (int f = 0; f < 4; f++) {
		turn_entity(cube, 0, 0, 0, 0);
		render_world(1);
		set_font(fnt); set_color(255, 255, 0);
		draw_text(10, 10, "Text top-left (Courier 22)", 0, 0);
		draw_text_a(512, 560, "centered, alpha .5", 1, 1, 0.5f);
		set_color(255, 255, 255); draw_rect(10, 40, 100, 50, 0); set_color(0, 120, 255); draw_rect(120, 40, 100, 50, 1);
		set_color(255, 0, 255); draw_line(230, 40, 330, 90); draw_oval(340, 40, 100, 50, 0);
		draw_image(im, 460, 35, 0);
		draw_image(icon, 700, 65, 0);
		set_font(big); set_color(255, 255, 255); draw_text(10, 480, "2D OK?", 0, 0);
		// grab the top-left of the screen into a texture, show it on cube2 (must be upright)
		copy_rect(0, 0, 256, 256, 0, 0, back_buffer(), texture_buffer(cap, 0));
		flip_frame(1);
	}
	render_world(1);
	set_font(fnt); set_color(255, 255, 0); draw_text(10, 10, "Text top-left (Courier 22)", 0, 0);
	set_color(255, 255, 255); draw_rect(10, 40, 100, 50, 0); set_color(0, 120, 255); draw_rect(120, 40, 100, 50, 1);
	set_color(255, 0, 255); draw_line(230, 40, 330, 90); draw_oval(340, 40, 100, 50, 0);
	draw_image(im, 460, 35, 0); draw_image(icon, 700, 65, 0);
	set_font(big); set_color(255, 255, 255); draw_text(10, 480, "2D OK?", 0, 0);
	int w = GetRenderWidth(), h = GetRenderHeight();
	g2d_end(); rlDrawRenderBatchActive();
	Image s = {rlReadScreenPixels(w, h), w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	if (argc > 1) save_png(argv[1], w, h, s.data);
	UnloadImage(s);
	flip_frame(1);
	end_graphics();
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
