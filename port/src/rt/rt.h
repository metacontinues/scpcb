// rt.h: the game's runtime on raylib: an entity tree (pivots, meshes, cameras, lights, sprites), collisions + picks,
// animation, textures/brushes, 2d, sound, input, files. handles are ints, 0 = null. angles are degrees, y is up,
// +z is forward (left-handed). indices (children, surfaces, frames) count from 0.
// irand(lo, hi) is inclusive, frnd(lo, hi) is lo + u*(hi-lo).
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

// ---- tex flags / blend
enum { TF_COLOR = 1, TF_ALPHA = 2, TF_MASK = 4, TF_MIP = 8, TF_CLAMPU = 16, TF_CLAMPV = 32,
       TF_SPHERE = 64, TF_CUBE = 128, TF_VRAM = 256, TF_HICOLOR = 512 };
enum { TB_NONE, TB_ALPHA, TB_MUL, TB_ADD, TB_DOT3, TB_MUL2 };
enum { FX_FULLBRIGHT = 1, FX_VCOLOR = 2, FX_FLAT = 4, FX_NOFOG = 8, FX_2SIDED = 16, FX_VALPHA = 32 };
enum { BL_ALPHA = 1, BL_MUL = 2, BL_ADD = 3 };
#define RT_NTEX 4 // tex layers per brush

// ---- system
int graphics_3d(int w, int h, int depth, int mode);
void end_graphics(void);
int graphics_width(void);
int graphics_height(void);
void flip_frame(int vsync);
void clear_screen(void);
void clear_color(int r, int g, int b);
int milli_secs(void);
int app_closing(void);
int count_gfx_drivers(void);
const char *gfx_driver_name(int i);
void anti_alias(int on);
int desktop_width(void);  // 0 before the first Graphics3D
int desktop_height(void);

// ---- files: game paths (backslashes, any case) -> real paths
const char *res_path(const char *p); // static buf, NULL if missing
FILE *read_file(const char *p);
int read_byte(FILE *f);
int read_short(FILE *f);
int read_int(FILE *f);
float read_float(FILE *f);
str read_string(FILE *f); // int len + bytes
str read_line(FILE *f);   // "" at eof

// ---- entities
ent create_pivot(ent par);
ent copy_entity(ent e, ent par);
void free_entity(ent e);
void entity_parent(ent e, ent par, int glob);
ent get_parent(ent e);
int count_children(ent e);
ent get_child(ent e, int n); // 1-based
ent find_child(ent e, const char *name);
void name_entity(ent e, const char *name);
const char *entity_name(ent e);
void hide_entity(ent e);
void show_entity(ent e);
int entity_hidden(ent e);
int entity_exists(ent e);

void position_entity(ent e, float x, float y, float z, int glob);
void move_entity(ent e, float x, float y, float z);
void translate_entity(ent e, float x, float y, float z, int glob);
void rotate_entity(ent e, float p, float y, float r, int glob);
void turn_entity(ent e, float p, float y, float r, int glob);
void scale_entity(ent e, float x, float y, float z, int glob);
void point_entity(ent e, ent t, float roll);
void align_to_vector(ent e, float x, float y, float z, int axis, float rate);
float entity_x(ent e, int glob);
float entity_y(ent e, int glob);
float entity_z(ent e, int glob);
float entity_pitch(ent e, int glob);
float entity_yaw(ent e, int glob);
float entity_roll(ent e, int glob);
float entity_scale_x(ent e, int glob);
float entity_scale_y(ent e, int glob);
float entity_scale_z(ent e, int glob);
float entity_distance(ent a, ent b);
float delta_yaw(ent a, ent b);
float vector_yaw(float x, float y, float z);
float vector_pitch(float x, float y, float z);
float delta_pitch(ent a, ent b);
void t_form_point(float x, float y, float z, ent src, ent dst);
void t_form_vector(float x, float y, float z, ent src, ent dst);
void t_form_normal(float x, float y, float z, ent src, ent dst);
float t_formed_x(void);
float t_formed_y(void);
float t_formed_z(void);
float get_mat_element(ent e, int row, int col);

// entity brush stuff
void entity_color(ent e, float r, float g, float b);
void entity_alpha(ent e, float a);
void entity_shininess(ent e, float s);
void entity_fx(ent e, int fx);
void entity_blend(ent e, int bl);
void entity_texture(ent e, htex t, int frame, int idx);
void entity_order(ent e, int ord);
void paint_entity(ent e, hbrush b);

// ---- brushes
hbrush create_brush(float r, float g, float b);
void free_brush(hbrush b);
void brush_color(hbrush b, float r, float g, float bl);
void brush_alpha(hbrush b, float a);
void brush_fx(hbrush b, int fx);
void brush_blend(hbrush b, int bl);
void brush_texture(hbrush b, htex t, int frame, int idx);
htex get_brush_texture(hbrush b, int idx); // +1 ref, free_texture it
hbrush get_surface_brush(surf *s);
hbrush get_entity_brush(ent e);

// ---- meshes / surfaces
ent create_mesh(ent par);
ent copy_mesh(ent e, ent par);
ent load_mesh(const char *p, ent par);
ent load_anim_mesh(const char *p, ent par);
ent create_cube(ent par);
ent create_sphere(int segs, ent par);
ent create_plane(int segs, ent par);
void add_mesh(ent src, ent dst);
void flip_mesh(ent e);
void scale_mesh(ent e, float x, float y, float z);
void position_mesh(ent e, float x, float y, float z);
void rotate_mesh(ent e, float p, float y, float r);
void fit_mesh(ent e, float x, float y, float z, float w, float h, float d, int uniform);
void update_normals(ent e);
void paint_mesh(ent e, hbrush b);
float mesh_width(ent e);
float mesh_height(ent e);
float mesh_depth(ent e);
int count_surfaces(ent e);
surf *get_surface(ent e, int n); // 1-based
surf *create_surface(ent e, hbrush b);
void paint_surface(surf *s, hbrush b);
void clear_surface(surf *s, int verts, int tris);
int add_vertex(surf *s, float x, float y, float z, float u, float v, float w);
int add_triangle(surf *s, int a, int b, int c);
void vertex_coords(surf *s, int i, float x, float y, float z);
void vertex_normal(surf *s, int i, float x, float y, float z);
void vertex_color(surf *s, int i, float r, float g, float b, float a);
void vertex_tex_coords(surf *s, int i, float u, float v, float w, int set);
int count_vertices(surf *s);
int count_triangles(surf *s);
float vertex_x(surf *s, int i);
float vertex_y(surf *s, int i);
float vertex_z(surf *s, int i);
int triangle_vertex(surf *s, int t, int c);

// ---- textures
htex load_texture(const char *p, int flags);
htex load_anim_texture(const char *p, int flags, int fw, int fh, int first, int count);
htex create_texture(int w, int h, int flags, int frames);
void free_texture(htex t);
void texture_blend(htex t, int bl);
void texture_coords(htex t, int set);
void scale_texture(htex t, float u, float v);
void position_texture(htex t, float u, float v);
int texture_width(htex t);
int texture_height(htex t);
const char *texture_name(htex t);
void texture_lod_bias(float b); // no-op: es2 has no lod bias (would need a shader bias)
void texture_fill(htex t, int r, int g, int b, int a); // stand-in for SetBuffer+Cls

// ---- cameras / lights / sprites / render
ent create_camera(ent par);
void camera_range(ent c, float n, float f);
void camera_zoom(ent c, float z);
void camera_viewport(ent c, int x, int y, int w, int h);
void camera_cls_color(ent c, int r, int g, int b);
void camera_cls_mode(ent c, int col, int z);
void camera_proj_mode(ent c, int m);
void camera_fog_mode(ent c, int m);
void camera_fog_range(ent c, float n, float f);
void camera_fog_color(ent c, int r, int g, int b);
void camera_project(ent c, float x, float y, float z);
float projected_x(void);
float projected_y(void);
float projected_z(void);
int entity_in_view(ent e, ent c);

ent create_light(int type, ent par);
void light_color(ent l, float r, float g, float b);
void light_range(ent l, float r);
void light_cone_angles(ent l, float in, float out);
void ambient_light(float r, float g, float b);

ent create_sprite(ent par);
ent load_sprite(const char *p, int flags, ent par);
void scale_sprite(ent s, float x, float y);
void handle_sprite(ent s, float x, float y);
void rotate_sprite(ent s, float a);
void sprite_view_mode(ent s, int m);

void render_world(float tween);
int tris_rendered(void);

// ---- collision / pick fields (logic lands in the collision phase)
void entity_type(ent e, int type, int rec);
int get_entity_type(ent e);
void entity_radius(ent e, float rx, float ry);
void entity_box(ent e, float x, float y, float z, float w, float h, float d);
void entity_pick_mode(ent e, int mode, int obscurer);

// ---- collisions (coll.c). methods: 1 sphere, 2 polygon, 3 box. responses: 1 stop, 2 slide, 3 slide w/o going down slopes
void coll_rule(int src, int dst, int method, int response);
void clear_collisions(void);
void update_world(float anim);
void reset_entity(ent e);
int count_collisions(ent e);
float collision_x(ent e, int i);
float collision_y(ent e, int i);
float collision_z(ent e, int i);
float collision_nx(ent e, int i);
float collision_ny(ent e, int i);
float collision_nz(ent e, int i);
float collision_time(ent e, int i);
ent collision_entity(ent e, int i);
surf *collision_surface(ent e, int i);
int collision_triangle(ent e, int i);
ent entity_collided(ent e, int type);

// ---- picking. pick modes: 1 sphere, 2 polygon, 3 box
ent line_pick(float x, float y, float z, float dx, float dy, float dz, float radius);
ent entity_pick(ent e, float range);
ent camera_pick(ent cam, float x, float y);
int entity_visible(ent a, ent b);
float picked_x(void);
float picked_y(void);
float picked_z(void);
float picked_nx(void);
float picked_ny(void);
float picked_nz(void);
float picked_time(void);
ent picked_entity(void);
surf *picked_surface(void);
int picked_triangle(void);

// ---- animation (anim.c). times are in frames
void set_anim_time(ent e, float t, int seq);
float anim_time(ent e);
int anim_seq(ent e);
int anim_length(ent e);
int animating(ent e);
void animate(ent e, int mode, float speed, int seq, float trans); // mode 0 stop 1 loop 2 pingpong 3 once
int extract_anim_seq(ent e, int first, int last, int seq);
const char *entity_class(ent e);

// ---- sound (snd.c). channels from PlaySound are independent voices; pan -1 left .. 1 right, pitch in hz
int load_sound(const char *p);
void free_sound(int s);
void loop_sound(int s);
void sound_volume(int s, float v);
void sound_pan(int s, float p);
void sound_pitch(int s, float hz);
int play_sound(int s);
void stop_channel(int c);
void pause_channel(int c);
void resume_channel(int c);
void channel_volume(int c, float v);
void channel_pan(int c, float p);
void channel_pitch(int c, float hz);
int channel_playing(int c);
// music streams, standing in for the FSOUND_Stream_* calls (StreamSound_Strict & co)
int stream_open(const char *p, int loop); // fmod Mode 2 = loop
void stream_play(int s);
void stream_paused(int s, int paused);
void stream_volume(int s, float v);
void stream_pan(int s, float p);
int stream_playing(int s);
void stream_close(int s);

// ---- 2d (g2d.c). buffers: BackBuffer/ImageBuffer/TextureBuffer, 0 in CopyRect/pixels = current
int back_buffer(void);
int front_buffer(void);
int image_buffer(int img, int frame);
int texture_buffer(htex t, int frame);
int graphics_buffer(void);
void set_buffer(int b);
void set_color(int r, int g, int b);
int color_red(void);
int color_green(void);
int color_blue(void);
void clear_color(int r, int g, int b);
void clear_screen(void);
void set_origin(int x, int y);
void set_viewport(int x, int y, int w, int h);
void draw_plot(int x, int y);
void draw_line(int x1, int y1, int x2, int y2);
void draw_rect(int x, int y, int w, int h, int solid);
void draw_oval(int x, int y, int w, int h, int solid);
int load_font(const char *p, int height, int bold, int italic, int underline);
void free_font(int f);
void set_font(int f);
int font_height(void);
int font_width(void);
int string_width(const char *s);
int string_height(const char *s);
void draw_text(int x, int y, const char *s, int cx, int cy);
void draw_text_a(int x, int y, const char *s, int cx, int cy, float alpha); // AAText's replacement
int load_image(const char *p);
int load_anim_image(const char *p, int fw, int fh, int first, int count);
int create_image(int w, int h, int frames);
int copy_image(int img);
void free_image(int img);
int image_width(int img);
int image_height(int img);
void mask_image(int img, int r, int g, int b);
void handle_image(int img, float x, float y);
void mid_handle(int img);
void auto_mid_handle(int on);
void resize_image(int img, float w, float h);
void scale_image(int img, float sx, float sy);
void rotate_image(int img, float ang);
void draw_image(int img, int x, int y, int frame);
void draw_block(int img, int x, int y, int frame);
void draw_image_rect(int img, int x, int y, int rx, int ry, int rw, int rh, int frame);
int image_rect_overlap(int img, int x, int y, int rx, int ry, int rw, int rh);
void copy_rect(int sx, int sy, int w, int h, int dx, int dy, int src, int dst);
void lock_buffer(int b);
void unlock_buffer(int b);
int read_pixel(int x, int y, int b); // argb; Fast variants are the same thing
void write_pixel(int x, int y, int argb, int b);
#define read_pixel_fast read_pixel
#define write_pixel_fast write_pixel
int count_gfx_modes_3d(void);
int gfx_mode_width(int i);
int gfx_mode_height(int i);
void hide_pointer(void);
void show_pointer(void);
void move_mouse(int x, int y);
void app_title(const char *t);
int total_vid_mem(void);
int avail_vid_mem(void);

// ---- input (input.c). keys are directinput scancodes; mouse buttons 1 l, 2 r, 3 m
int key_down(int sc);
int key_hit(int sc);
void flush_keys(void);
int get_key(void);
int wait_key(void);
int mouse_down(int b);
int mouse_hit(int b);
int mouse_x(void);
int mouse_y(void);
int mouse_x_speed(void);
int mouse_y_speed(void);
int mouse_z_speed(void);
void flush_mouse(void);

// ---- misc (misc.c)
float frnd(float lo, float hi);
int irand(int lo, int hi);
void seed_rnd(int seed);
int rnd_seed(void);
FILE *write_file(const char *p);
FILE *open_file(const char *p);
void close_file(FILE *f);
int file_eof(FILE *f);
int file_pos(FILE *f);
int seek_file(FILE *f, int pos);
void write_byte(FILE *f, int v);
void write_short(FILE *f, int v);
void write_int(FILE *f, int v);
void write_float(FILE *f, float v);
void write_string(FILE *f, const char *s);
void write_line(FILE *f, const char *s);
int file_type(const char *p);
int file_size(const char *p);
int create_dir(const char *p);
int delete_file(const char *p);
int copy_file(const char *from, const char *to);
const char *current_dir(void);
void *read_dir(const char *p);
const char *next_file(void *d);
void close_dir(void *d);
int delete_dir(const char *p);
const char *current_date(void);
const char *current_time(void);
void delay_ms(int ms);
int save_png(const char *p, int w, int h, const void *rgba); // rgba8, top row first
