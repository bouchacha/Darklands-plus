#!/bin/sh
# build_tcc_win.sh OUTDIR - TinyCC for 64-bit Windows, as a release ships it (darklands_plus/tcc):
# tcc.exe and libtcc.dll cross-compiled with mingw-w64, libtcc1.a from a Linux-hosted cross tcc,
# the Windows headers and .def files of TinyCC's win32 folder. Needs git, gcc, make and
# x86_64-w64-mingw32-gcc. TinyCC is LGPL 2.1 (COPYING is copied along).
set -e
OUT=${1:?usage: build_tcc_win.sh OUTDIR}
REPO=${TCC_REPO:-https://github.com/TinyCC/tinycc.git}
REV=${TCC_REV:-43c7708}                 # the revision the split build was tested with
W=$(mktemp -d)
git clone -q "$REPO" "$W/src"
git -C "$W/src" checkout -q "$REV"
cp -r "$W/src" "$W/cross"
(cd "$W/cross" && ./configure >/dev/null && make -s cross-x86_64-win32)
cp -r "$W/src" "$W/win"
(cd "$W/win" && ./configure --targetos=WIN32 --cpu=x86_64 --cross-prefix=x86_64-w64-mingw32- --config-predefs=no >/dev/null \
    && make -s tcc.exe libtcc.dll 2>/dev/null || make -s tcc.exe)
mkdir -p "$OUT/include" "$OUT/lib"
cp "$W/win/tcc.exe" "$OUT/"
[ -f "$W/win/libtcc.dll" ] && cp "$W/win/libtcc.dll" "$OUT/"
cp -r "$W/win/win32/include/." "$OUT/include/"
cp "$W/win/include/"*.h "$OUT/include/"
cp "$W/cross/x86_64-win32-libtcc1.a" "$OUT/lib/libtcc1.a"
cp "$W/win/win32/lib/"*.def "$OUT/lib/"
cp "$W/src/COPYING" "$OUT/COPYING"
echo "TinyCC $REV from $REPO (LGPL 2.1, see COPYING; source: that repository at that revision)" > "$OUT/SOURCE.txt"
rm -rf "$W"
echo "TinyCC for Windows in $OUT"
