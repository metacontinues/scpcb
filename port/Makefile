# scpcb c port. plain make, no cmake.
#   make                 build build/rmview against the system gles/egl (mesa)
#   make ANGLE=/path     also rpath angle's libEGL.so/libGLESv2.so so they win at runtime
#   make clean

CC ?= cc
RL := vendor/raylib/src
B := build
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu11 -Wall -Wextra -Wno-unused-parameter -I$(RL)
RLDEFS := -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_ES2 -D_GLFW_X11 \
	-DSUPPORT_FILEFORMAT_JPG -DSUPPORT_FILEFORMAT_BMP
CFLAGS += $(RLDEFS) # so rlgl.h/config.h match the lib
LDLIBS := -lm -lpthread -ldl -lrt -lX11

ifdef ANGLE
LDFLAGS += -L$(ANGLE) -Wl,-rpath,$(abspath $(ANGLE))
endif

RLSRC := rcore rshapes rtextures rtext rmodels raudio utils rglfw
RLOBJ := $(RLSRC:%=$(B)/raylib/%.o)
BBSRC := $(wildcard src/bb/*.c) $(wildcard src/fmt/*.c)
BBOBJ := $(BBSRC:%.c=$(B)/%.o)

all: $(B)/rmview $(B)/colltest $(B)/sndtest

$(B)/libraylib.a: $(RLOBJ)
	ar rcs $@ $^
$(B)/raylib/%.o: $(RL)/%.c
	@mkdir -p $(@D)
	$(CC) -c -O2 -w $(RLDEFS) -I$(RL)/external/glfw/include $< -o $@

$(B)/%.o: %.c $(wildcard src/*/*.h)
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

$(B)/rmview: $(B)/tools/rmview.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/colltest: $(B)/tools/colltest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/sndtest: $(B)/tools/sndtest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# run from anywhere; sndtest needs the repo's SFX + an audio device
test: $(B)/colltest $(B)/sndtest
	$(B)/colltest
	cd .. && port/$(B)/sndtest

clean:
	rm -rf $(B)

.PHONY: all clean test
