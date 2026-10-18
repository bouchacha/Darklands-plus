#!/bin/sh
# batch.sh OUT [exe] - run the standard play tests against the native build in parallel and summarise
OUT=$1; EXE=${2:-$(dirname "$0")/../../build/port/darklands}
T="new_world charsheet market save_game side_street new_world_full inn overland tavern_menu"
mkdir -p "$OUT"
for f in $T; do
  python3 -I "$(dirname "$0")/nplay.py" --exe "$EXE" "$OUT/$f" "@$(dirname "$0")/../../tools/playtests/$f.txt" > "$OUT/$f.log" 2>&1 &
done
wait
for f in $T; do
  echo "== $f: $(grep -c '^shot' $OUT/$f.log) shots, $(grep -c missing $OUT/$f.log) missing $(grep -a 'FATAL\|program\|run-time' $OUT/$f/stderr.txt 2>/dev/null | head -2 | tr '\n' ' ')"
done
