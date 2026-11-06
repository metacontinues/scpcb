// colltest: collision + pick checks on primitives. no window needed.
#include <stdio.h>
#include <math.h>
#include "../src/bb/bb.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)
#define NEAR(a, b) (fabsf((a) - (b)) < 0.02f)
enum { T_MAP = 1, T_PLAYER, T_BALL };

static ent box(float x, float y, float z, float sx, float sy, float sz) {
	ent b = bbCreateCube(0);
	bbScaleEntity(b, sx, sy, sz, 0); bbPositionEntity(b, x, y, z, 0);
	bbEntityType(b, T_MAP, 0); bbEntityPickMode(b, 2, 1);
	return b;
}
static ent mover(float x, float y, float z, float r) {
	ent p = bbCreatePivot(0);
	bbPositionEntity(p, x, y, z, 0); bbEntityRadius(p, r, 0); bbEntityType(p, T_PLAYER, 0);
	return p;
}

int main(void) {
	bbCollisions(T_PLAYER, T_MAP, 2, 2);
	bbCollisions(T_PLAYER, T_BALL, 1, 1);
	ent floor = box(0, -1, 0, 10, 1, 10); // top at y=0

	// fall onto the floor
	ent p = mover(0, 2, 0, 0.5f);
	bbTranslateEntity(p, 0, -5, 0, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityY(p, 1), 0.5f), "rest y=%f", bbEntityY(p, 1));
	CHECK(bbCountCollisions(p) == 1 && bbCollisionEntity(p, 1) == floor, "n=%d", bbCountCollisions(p));
	CHECK(NEAR(bbCollisionNY(p, 1), 1), "ny=%f", bbCollisionNY(p, 1));
	CHECK(NEAR(bbCollisionY(p, 1), 0), "cy=%f", bbCollisionY(p, 1));

	// slide along a wall at z=5 (front face at z=4.5)
	ent wall = box(0, 1, 5, 10, 2, 0.5f);
	bbPositionEntity(p, 0, 1, 0, 0); bbResetEntity(p);
	bbTranslateEntity(p, 2, 0, 10, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityZ(p, 1), 4.0f), "wall z=%f", bbEntityZ(p, 1));
	CHECK(bbEntityX(p, 1) > 1.5f, "slid x=%f", bbEntityX(p, 1));

	// stop response vs a sphere
	ent ball = bbCreatePivot(0);
	bbPositionEntity(ball, -5, 1, 0, 0); bbEntityRadius(ball, 0.5f, 0); bbEntityType(ball, T_BALL, 0);
	bbPositionEntity(p, -2, 1, 0, 0); bbResetEntity(p);
	bbTranslateEntity(p, -6, 0, 0, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityX(p, 1), -4.0f), "ball x=%f", bbEntityX(p, 1));
	CHECK(bbEntityCollided(p, T_BALL) == ball, "collided");

	// ellipsoid: ry=1 rests at y=1
	ent q = mover(3, 3, 0, 0.3f);
	bbEntityRadius(q, 0.3f, 1);
	bbTranslateEntity(q, 0, -10, 0, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityY(q, 1), 1.0f), "ellipsoid y=%f", bbEntityY(q, 1));

	// one-sided: moving out of the inside of a box passes through
	ent in = box(20, 0, 0, 2, 2, 2);
	bbPositionEntity(q, 20, 0, 0, 0); bbResetEntity(q);
	bbTranslateEntity(q, 5, 0, 0, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityX(q, 1), 25), "exit x=%f", bbEntityX(q, 1));
	(void)in;

	// hidden targets don't collide
	bbHideEntity(wall);
	bbPositionEntity(p, 0, 1, 0, 0); bbResetEntity(p);
	bbTranslateEntity(p, 0, 0, 10, 0); bbUpdateWorld(1);
	CHECK(NEAR(bbEntityZ(p, 1), 10), "hidden z=%f", bbEntityZ(p, 1));
	bbShowEntity(wall);

	// picks
	ent h = bbLinePick(1, 5, 1, 0, -10, 0, 0);
	CHECK(h == floor && NEAR(bbPickedY(), 0) && NEAR(bbPickedNY(), 1), "pick e=%d y=%f", h, bbPickedY());
	h = bbLinePick(1, 5, 1, 0, -10, 0, 0.25f);
	CHECK(h == floor && NEAR(bbPickedY(), 0), "sphere pick y=%f", bbPickedY());
	h = bbLinePick(1, -0.5f, 1, 0, 10, 0, 0); // from under the floor, inside it: back faces only
	CHECK(h == 0, "inside pick e=%d", h);
	ent eye = bbCreatePivot(0); bbPositionEntity(eye, 0, 1, 0, 0);
	bbRotateEntity(eye, 0, 0, 0, 0);
	CHECK(bbEntityPick(eye, 10) == wall && NEAR(bbPickedZ(), 4.5f), "entitypick z=%f", bbPickedZ());
	ent tgt = bbCreatePivot(0); bbPositionEntity(tgt, 0, 1, 8, 0);
	CHECK(!bbEntityVisible(eye, tgt), "wall blocks sight");
	bbPositionEntity(tgt, 0, 1, 3, 0);
	CHECK(bbEntityVisible(eye, tgt), "clear sight");

	// moved + rotated target: cache must refresh
	bbRotateEntity(wall, 0, 90, 0, 0); // now spans z, thin in x
	h = bbLinePick(-3, 1, 5, 10, 0, 0, 0);
	CHECK(h == wall && NEAR(bbPickedX(), -0.5f), "rotated pick x=%f", bbPickedX());

	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
