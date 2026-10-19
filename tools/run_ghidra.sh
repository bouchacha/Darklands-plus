#!/bin/sh
# Run the Ghidra headless pseudo-C export for every code image.
# Inputs come from tools/ghidra_prep.py (build/ghidra/in); output goes to build/ghidra/out/c/<image>.c
# Usage: tools/run_ghidra.sh <ghidra install dir>
REPO=$(dirname "$(dirname "$(readlink -f "$0")")")
GHIDRA=${1:-/home/claude/tools_ext/ghidra_11.4_PUBLIC}
B=$REPO/build/ghidra
mkdir -p "$B/proj" "$B/out" "$B/bins_core" "$B/bins_events"
for f in "$B"/in/*.bin; do
  n=$(basename "$f" .bin)
  case "$n" in
    root|s00*|s01*|s02*|s03*|s040|s041|s043) cp "$f" "$B/bins_core/" ;;
    *) cp "$f" "$B/bins_events/" ;;
  esac
done
for set in core events; do
  "$GHIDRA/support/analyzeHeadless" "$B/proj" "darklands_$set" -import "$B/bins_$set" \
    -processor "x86:LE:16:Real Mode" -loader BinaryLoader -loader-baseAddr 0 -noanalysis \
    -scriptPath "$REPO/tools/ghidra" -postScript DarklandsExport.java "$B/in" "$B/out" \
    -max-cpu 2 -overwrite > "$B/run_$set.log" 2>&1
done
echo ALLDONE > "$B/done.flag"
