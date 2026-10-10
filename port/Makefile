# Native Darklands. Generated code comes from port/recomp (see port/README.md).
#   make -C port                 Linux build  -> build/port/darklands
#   make -C port TARGET=win64    Windows build (mingw-w64) -> build/port_win64/darklands.exe
TOP      := ..
GEN      := $(TOP)/build/port_gen
TARGET   ?= linux
OPT      ?= -O1
ifeq ($(TARGET),win64)
  CC     := x86_64-w64-mingw32-gcc
  OUT    := $(TOP)/build/port_win64
  SDLDIR ?= $(TOP)/build/sdl2-win64
  SDL_CFLAGS := -I$(SDLDIR)/include/SDL2 -Dmain=SDL_main
  SDL_LIBS   := -L$(SDLDIR)/lib -static -lmingw32 -lSDL2main -lSDL2 -mwindows \
                -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
                -lshell32 -lsetupapi -lversion -luuid -static-libgcc
  BIN    := $(OUT)/darklands.exe
  LDEXTRA := -s
else
  CC     ?= gcc
  OUT    := $(TOP)/build/port
  SDL_CFLAGS := $(shell sdl2-config --cflags)
  SDL_LIBS   := $(shell sdl2-config --libs) -lm
  BIN    := $(OUT)/darklands
endif
ifdef FTRACE
  OUT    := $(OUT)_ft
  BIN    := $(OUT)/$(notdir $(BIN))
  EXTRA  := -DFTRACE
endif
CFLAGS   := $(EXTRA) $(OPT) -g0 -fwrapv -fno-strict-aliasing -w -Iruntime -I$(GEN) $(SDL_CFLAGS)
RTFLAGS  := $(EXTRA) -O2 -g -fwrapv -fno-strict-aliasing -Wall -Wno-unused-function -Wno-misleading-indentation \
            -Wno-unused-variable -Wno-unused-result -Wno-format-truncation -Wno-unused-but-set-variable -Iruntime -I$(GEN) $(SDL_CFLAGS)
RT_SRC   := $(wildcard runtime/*.c) third_party/nuked-opl3/opl3.c
GEN_SRC  := $(wildcard $(GEN)/*.c)
OBJS     := $(patsubst runtime/%.c,$(OUT)/rt_%.o,$(wildcard runtime/*.c)) $(OUT)/opl3.o $(patsubst $(GEN)/%.c,$(OUT)/gen_%.o,$(GEN_SRC))

all: $(BIN)
$(BIN): $(OBJS)
	$(CC) $(LDEXTRA) -o $@ $^ $(SDL_LIBS)
$(OUT)/rt_%.o: runtime/%.c runtime/*.h | $(OUT)
	$(CC) $(RTFLAGS) -c -o $@ $<
$(OUT)/opl3.o: third_party/nuked-opl3/opl3.c | $(OUT)
	$(CC) -O2 -c -o $@ $<
$(OUT)/gen_%.o: $(GEN)/%.c runtime/cpu.h runtime/strops.h | $(OUT)
	$(CC) $(CFLAGS) -c -o $@ $<
$(OUT):
	mkdir -p $@
clean:
	rm -rf $(OUT)
.PHONY: all clean
