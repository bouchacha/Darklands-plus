#!/bin/sh
# gen.sh - regenerate build/port_gen from the original game files (needs build/analysis/analysis.pickle)
HERE=$(cd "$(dirname "$0")/../.." && pwd)
G=${GAME:-/home/claude/dl_raw/extracted/DARKLAND}
cd "$HERE" || exit 1
python3 -I port/recomp/recomp16.py "$G/DARKLAND.EXE" build/analysis/analysis.pickle build/port_gen || exit 1
# sound drivers are recognised by content at run time (GOG ships Roland copies under the FM
# drivers' names, and the FM drivers renamed to .DC/.DB): c = cards/strategic, b = battle
python3 -I port/recomp/recomp_drv.py build/port_gen mgraphic="$G/MGRAPHIC.EXE" misc="$G/MISC.EXE" \
    none_c="$G/NSOUND.DLC" roland_c="$G/RSOUND.DLC" roland_b="$G/RSOUND.DLB" \
    opl3_c="$G/PSOUND.DC" opl3_b="$G/PSOUND.DB" adlib_c="$G/ASOUND.DC" adlib_b="$G/ASOUND.DB" \
    spk_c="$G/ISOUND.DLC" spk_b="$G/ISOUND.DLB"
