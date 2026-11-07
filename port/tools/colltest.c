// colltest: collision + pick checks on primitives. no window needed.
#include <stdio.h>
#include <math.h>
#include "../src/rt/rt.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)
#define NEAR(a, b) (fabsf((a) - (b)) < 0.02f)
enum { T_MAP = 1, T_PLAYER, T_BALL };

static ent box(float x, float y, float z, float sx, float sy, float sz) {
	ent b = create_cube(0);
	scale_entity(b, sx, sy, sz, 0); position_entity(b, x, y, z, 0);
	entity_type(b, T_MAP, 0); entity_pick_mode(b, 2, 1);
	return b;
}
static ent mover(float x, float y, float z, float r) {
	ent p = create_pivot(0);
	position_entity(p, x, y, z, 0); entity_radius(p, r, 0); entity_type(p, T_PLAYER, 0);
	return p;
}

int main(void) {
	coll_rule(T_PLAYER, T_MAP, 2, 2);
	coll_rule(T_PLAYER, T_BALL, 1, 1);
	ent floor = box(0, -1, 0, 10, 1, 10); // top at y=0

	// fall onto the floor
	ent p = mover(0, 2, 0, 0.5f);
	translate_entity(p, 0, -5, 0, 0); update_world(1);
	CHECK(NEAR(entity_y(p, 1), 0.5f), "rest y=%f", entity_y(p, 1));
	CHECK(count_collisions(p) == 1 && collision_entity(p, 1) == floor, "n=%d", count_collisions(p));
	CHECK(NEAR(collision_ny(p, 1), 1), "ny=%f", collision_ny(p, 1));
	CHECK(NEAR(collision_y(p, 1), 0), "cy=%f", collision_y(p, 1));

	// slide along a wall at z=5 (front face at z=4.5)
	ent wall = box(0, 1, 5, 10, 2, 0.5f);
	position_entity(p, 0, 1, 0, 0); reset_entity(p);
	translate_entity(p, 2, 0, 10, 0); update_world(1);
	CHECK(NEAR(entity_z(p, 1), 4.0f), "wall z=%f", entity_z(p, 1));
	CHECK(entity_x(p, 1) > 1.5f, "slid x=%f", entity_x(p, 1));

	// stop response vs a sphere
	ent ball = create_pivot(0);
	position_entity(ball, -5, 1, 0, 0); entity_radius(ball, 0.5f, 0); entity_type(ball, T_BALL, 0);
	position_entity(p, -2, 1, 0, 0); reset_entity(p);
	translate_entity(p, -6, 0, 0, 0); update_world(1);
	CHECK(NEAR(entity_x(p, 1), -4.0f), "ball x=%f", entity_x(p, 1));
	CHECK(entity_collided(p, T_BALL) == ball, "collided");

	// ellipsoid: ry=1 rests at y=1
	ent q = mover(3, 3, 0, 0.3f);
	entity_radius(q, 0.3f, 1);
	translate_entity(q, 0, -10, 0, 0); update_world(1);
	CHECK(NEAR(entity_y(q, 1), 1.0f), "ellipsoid y=%f", entity_y(q, 1));

	// one-sided: moving out of the inside of a box passes through
	ent in = box(20, 0, 0, 2, 2, 2);
	position_entity(q, 20, 0, 0, 0); reset_entity(q);
	translate_entity(q, 5, 0, 0, 0); update_world(1);
	CHECK(NEAR(entity_x(q, 1), 25), "exit x=%f", entity_x(q, 1));
	(void)in;

	// hidden targets don't collide
	hide_entity(wall);
	position_entity(p, 0, 1, 0, 0); reset_entity(p);
	translate_entity(p, 0, 0, 10, 0); update_world(1);
	CHECK(NEAR(entity_z(p, 1), 10), "hidden z=%f", entity_z(p, 1));
	show_entity(wall);

	// picks
	ent h = line_pick(1, 5, 1, 0, -10, 0, 0);
	CHECK(h == floor && NEAR(picked_y(), 0) && NEAR(picked_ny(), 1), "pick e=%d y=%f", h, picked_y());
	h = line_pick(1, 5, 1, 0, -10, 0, 0.25f);
	CHECK(h == floor && NEAR(picked_y(), 0), "sphere pick y=%f", picked_y());
	h = line_pick(1, -0.5f, 1, 0, 10, 0, 0); // from under the floor, inside it: back faces only
	CHECK(h == 0, "inside pick e=%d", h);
	ent eye = create_pivot(0); position_entity(eye, 0, 1, 0, 0);
	rotate_entity(eye, 0, 0, 0, 0);
	CHECK(entity_pick(eye, 10) == wall && NEAR(picked_z(), 4.5f), "entitypick z=%f", picked_z());
	ent tgt = create_pivot(0); position_entity(tgt, 0, 1, 8, 0);
	CHECK(!entity_visible(eye, tgt), "wall blocks sight");
	position_entity(tgt, 0, 1, 3, 0);
	CHECK(entity_visible(eye, tgt), "clear sight");

	// moved + rotated target: cache must refresh
	rotate_entity(wall, 0, 90, 0, 0); // now spans z, thin in x
	h = line_pick(-3, 1, 5, 10, 0, 0, 0);
	CHECK(h == wall && NEAR(picked_x(), -0.5f), "rotated pick x=%f", picked_x());

	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
