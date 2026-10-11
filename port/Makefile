# Native Darklands. Generated code comes from port/recomp (see port/README.md).
#   make -C port                 Linux build  -> build/port/darklands
#   make -C port TARGET=win64    Windows build (mingw-w64) -> build/port_win64/darklands.exe
TOP      := ..
GEN      := $(TOP)/build/port_gen
TARGET   ?= linux
OPT      ?= -O1
ifeq ($(TARGET),win64)
  CC     := x86_64-w64-mingw32-gcc
  CXX    := x86_64-w64-mingw32-g++
  OUT    := $(TOP)/build/port_win64
  SDLDIR ?= $(TOP)/build/sdl2-win64
  SDL_CFLAGS := -I$(SDLDIR)/include/SDL2 -Dmain=SDL_main
  SDL_LIBS   := -L$(SDLDIR)/lib -static -lmingw32 -lSDL2main -lSDL2 -mwindows \
                -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
                -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++
  BIN    := $(OUT)/darklands.exe
  LDEXTRA := -s
else
  CC     ?= gcc
  CXX    ?= g++
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
IMGUI    := third_party/imgui
UI_OBJS  := $(patsubst ui/%.cpp,$(OUT)/ui_%.o,$(wildcard ui/*.cpp)) $(OUT)/fonts.o \
            $(patsubst mods/%.c,$(OUT)/mods_%.o,$(wildcard mods/*.c)) \
            $(patsubst $(IMGUI)/%.cpp,$(OUT)/imgui_%.o,$(wildcard $(IMGUI)/*.cpp)) \
            $(OUT)/imgui_impl_sdl2.o $(OUT)/imgui_impl_sdlrenderer2.o
CXXFLAGS := -O2 -I$(IMGUI) -Iruntime -I$(GEN) $(SDL_CFLAGS)
OBJS     := $(patsubst runtime/%.c,$(OUT)/rt_%.o,$(wildcard runtime/*.c)) $(OUT)/opl3.o $(UI_OBJS) $(patsubst $(GEN)/%.c,$(OUT)/gen_%.o,$(GEN_SRC))

all: $(BIN)
$(BIN): $(OBJS)
	$(CXX) $(LDEXTRA) -o $@ $^ $(SDL_LIBS)
$(OUT)/ui_%.o: ui/%.cpp ui/*.h runtime/*.h mods/*.h | $(OUT)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OUT)/imgui_%.o: $(IMGUI)/%.cpp | $(OUT)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OUT)/imgui_impl_%.o: $(IMGUI)/backends/imgui_impl_%.cpp | $(OUT)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(OUT)/fonts.c: third_party/fonts/*.ttf tools/bin2c.py | $(OUT)
	python3 -I tools/bin2c.py $@ FONT_MEDIUM=third_party/fonts/EBGaramond-Medium.ttf FONT_SEMIBOLD=third_party/fonts/EBGaramond-SemiBold.ttf \
	    FONT_ALEGREYA=third_party/fonts/Alegreya-Medium.ttf FONT_FELL=third_party/fonts/IMFellEnglish-Regular.ttf \
	    FONT_MEDIEVAL=third_party/fonts/MedievalSharp-Regular.ttf FONT_ATKINSON=third_party/fonts/AtkinsonHyperlegible-Regular.ttf \
	    FONT_FRAKTUR=third_party/fonts/DLFraktur-Regular.ttf
$(OUT)/fonts.o: $(OUT)/fonts.c
	$(CC) -c -o $@ $<
$(OUT)/mods_%.o: mods/%.c mods/*.h runtime/*.h | $(OUT)
	$(CC) $(RTFLAGS) -c -o $@ $<
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
