#!/bin/sh
# gen.sh - regenerate build/port_gen from the original game files (needs build/analysis/analysis.pickle)
HERE=$(cd "$(dirname "$0")/../.." && pwd)
G=${GAME:-/home/claude/dl_raw/extracted/DARKLAND}
cd "$HERE" || exit 1
python3 -I port/recomp/recomp16.py "$G/DARKLAND.EXE" build/analysis/analysis.pickle build/port_gen || exit 1
python3 -I port/recomp/recomp_drv.py build/port_gen mgraphic="$G/MGRAPHIC.EXE" misc="$G/MISC.EXE" \
    nsound="$G/NSOUND.DLC" asound="$G/ASOUND.DLC" psound="$G/PSOUND.DLC" rsound="$G/RSOUND.DLC"
