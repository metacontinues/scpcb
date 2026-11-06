# scpcb c port. plain make, no cmake.
#   make                 build build/rmview against the system gles/egl (mesa)
#   make ANGLE=/path     also rpath angle's libEGL.so/libGLESv2.so so they win at runtime
#   make clean

CC ?= cc
RL := vendor/raylib/src
GC := vendor/bdwgc
B := build
CFLAGS ?= -O2 -g
CFLAGS += -std=gnu11 -Wall -Wextra -Wno-unused-parameter -I$(RL) -I$(GC)/include
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
BBOBJ := $(BBSRC:%.c=$(B)/%.o) $(B)/gc.o

all: $(B)/rmview $(B)/colltest $(B)/sndtest $(B)/g2dtest $(B)/misctest $(B)/strtest

$(B)/libraylib.a: $(RLOBJ)
	ar rcs $@ $^
$(B)/raylib/%.o: $(RL)/%.c
	@mkdir -p $(@D)
	$(CC) -c -O2 -w $(RLDEFS) -I$(RL)/external/glfw/include $< -o $@

# bdwgc as one translation unit, single threaded (only the main thread touches gc memory)
$(B)/gc.o: $(GC)/extra/gc.c
	@mkdir -p $(@D)
	$(CC) -c -O2 -w -I$(GC)/include -DNO_EXECUTE_PERMISSION $< -o $@

$(B)/%.o: %.c $(wildcard src/*/*.h)
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $< -o $@

$(B)/rmview: $(B)/tools/rmview.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/colltest: $(B)/tools/colltest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/sndtest: $(B)/tools/sndtest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/g2dtest: $(B)/tools/g2dtest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/misctest: $(B)/tools/misctest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(B)/strtest: $(B)/tools/strtest.o $(BBOBJ) $(B)/libraylib.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# run from anywhere; sndtest needs the repo's SFX + an audio device
test: $(B)/colltest $(B)/sndtest $(B)/misctest $(B)/strtest
	$(B)/colltest
	$(B)/misctest
	$(B)/strtest
	cd .. && port/$(B)/sndtest

clean:
	rm -rf $(B)

.PHONY: all clean test
