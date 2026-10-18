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
                -lshell32 -lsetupapi -lversion -luuid -static-libgcc -static-libstdc++ \
                -Wl,--stack,16777216   # the game's calls nest as native calls: 16 MB as on Linux (8), not 2
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
# SPLIT=1: the program without the game's translated code, which it loads at run time from a
# module made on the player's machine (runtime/split.h; "make module" builds one here with TinyCC)
RT_C     := $(filter-out runtime/gensplit.c,$(wildcard runtime/*.c))
GEN_SRC  := $(wildcard $(GEN)/*.c)
ifdef SPLIT
  OUT    := $(OUT)_split
  BIN    := $(OUT)/$(notdir $(BIN))
  EXTRA  += -DDL_RTSPLIT
  RT_C   := $(filter-out runtime/overrides.c,$(RT_C))
  GEN_OBJS :=
  ifneq ($(TARGET),win64)
    SDL_LIBS += -ldl
  endif
else
  GEN_OBJS := $(patsubst $(GEN)/%.c,$(OUT)/gen_%.o,$(GEN_SRC))
endif
ifdef ASAN
  # the port's own code (runtime, interface, mods) checked by AddressSanitizer; the generated game code as usual
  OUT    := $(OUT)_asan
  BIN    := $(OUT)/$(notdir $(BIN))
  SANFL  := -fsanitize=address -fno-omit-frame-pointer -g
  SDL_LIBS += -fsanitize=address
endif
CFLAGS   := $(EXTRA) $(OPT) -g0 -fwrapv -fno-strict-aliasing -w -Iruntime -I$(GEN) $(SDL_CFLAGS)
RTFLAGS  := $(EXTRA) $(SANFL) -O2 -g -fwrapv -fno-strict-aliasing -Wall -Wno-unused-function -Wno-misleading-indentation \
            -Wno-unused-variable -Wno-unused-result -Wno-format-truncation -Wno-unused-but-set-variable -Iruntime -I$(GEN) $(SDL_CFLAGS)
IMGUI    := third_party/imgui
UI_OBJS  := $(patsubst ui/%.cpp,$(OUT)/ui_%.o,$(wildcard ui/*.cpp)) $(OUT)/fonts.o \
            $(patsubst mods/%.c,$(OUT)/mods_%.o,$(wildcard mods/*.c)) \
            $(patsubst $(IMGUI)/%.cpp,$(OUT)/imgui_%.o,$(wildcard $(IMGUI)/*.cpp)) \
            $(OUT)/imgui_impl_sdl2.o $(OUT)/imgui_impl_sdlrenderer2.o
CXXFLAGS := $(EXTRA) $(SANFL) -O2 -I$(IMGUI) -Iruntime -I$(GEN) $(SDL_CFLAGS)
# Munt's MT-32 emulator (runtime/mt32.c)
MT32     := third_party/mt32emu
MT32_SRC := $(wildcard $(MT32)/*.cpp) $(wildcard $(MT32)/c_interface/*.cpp) $(MT32)/sha1/sha1.cpp \
            $(MT32)/srchelper/InternalResampler.cpp $(wildcard $(MT32)/srchelper/srctools/src/*.cpp)
MT32_OBJS := $(patsubst $(MT32)/%.cpp,$(OUT)/mt32emu/%.o,$(MT32_SRC))
OBJS     := $(patsubst runtime/%.c,$(OUT)/rt_%.o,$(RT_C)) $(OUT)/opl3.o $(MT32_OBJS) $(UI_OBJS) $(GEN_OBJS) \
            $(OUT)/version.o $(if $(filter win64,$(TARGET)),$(OUT)/res.o)
# the build's name: commits so far and the commit ("152-3815c0f"), shown in F12 > Settings > About and the log
RELEASE  ?=
VERSION  := $(shell git -C $(TOP) rev-list --count HEAD 2>/dev/null || echo 0)-$(shell git -C $(TOP) rev-parse --short HEAD 2>/dev/null || echo local)
# the translator's id: a game module made by another one does not fit (split.h, tools/prepare.py)
TRANSLATOR_ID := $(shell python3 -I tools/prepare.py --id 2>/dev/null || echo unknown)

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
	    FONT_FRAKTUR=third_party/fonts/DLFraktur-Regular.ttf FONT_GRENZE=third_party/fonts/GrenzeGotisch-Medium.ttf
$(OUT)/fonts.o: $(OUT)/fonts.c
	$(CC) -c -o $@ $<
$(OUT)/mods_%.o: mods/%.c mods/*.h runtime/*.h ui/*.h | $(OUT)
	$(CC) $(RTFLAGS) -c -o $@ $<
$(OUT)/rt_%.o: runtime/%.c runtime/*.h mods/*.h ui/*.h | $(OUT)
	$(CC) $(RTFLAGS) -c -o $@ $<
$(OUT)/opl3.o: third_party/nuked-opl3/opl3.c | $(OUT)
	$(CC) -O2 -c -o $@ $<
$(OUT)/mt32emu/%.o: $(MT32)/%.cpp | $(OUT)
	@mkdir -p $(dir $@)
	$(CXX) $(SANFL) -O2 -DMT32EMU_WITH_INTERNAL_RESAMPLER -DMT32EMU_WITH_STD_SNPRINTF -I$(MT32) -c -o $@ $<
$(OUT)/gen_%.o: $(GEN)/%.c runtime/cpu.h runtime/strops.h | $(OUT)
	$(CC) $(CFLAGS) -c -o $@ $<
$(OUT)/res.o: res/darklands.rc res/darklands.ico $(OUT)/version.c | $(OUT)
	x86_64-w64-mingw32-windres -DDLP_VERSION='\"$(VERSION)\"' -I res -i res/darklands.rc -o $@
$(OUT):
	mkdir -p $@
$(OUT)/version.c: FORCE | $(OUT)
	@echo 'const char dlp_version[] = "$(VERSION)"; const char dlp_release[] = "$(RELEASE)"; const char dlp_translator[] = "$(TRANSLATOR_ID)";' > $@.tmp; cmp -s $@.tmp $@ || mv $@.tmp $@; rm -f $@.tmp
$(OUT)/version.o: $(OUT)/version.c
	$(CC) -c -o $@ $<
.PHONY: FORCE
FORCE:
clean:
	rm -rf $(OUT)
.PHONY: all clean
# the game module (split.h) from the generated code, as the player's machine makes it
TCC      ?= tcc
MODULE   := $(if $(filter win64,$(TARGET)),darklands_game.dll,libdarklands_game.so)
module: | $(OUT)
	$(TCC) -shared -w -DDL_GENSPLIT -DGEN_TRANSLATOR=\"$(TRANSLATOR_ID)\" -Iruntime -I$(GEN) -o $(OUT)/$(MODULE) $(GEN_SRC) runtime/cpu.c runtime/overrides.c runtime/gensplit.c
.PHONY: all module
