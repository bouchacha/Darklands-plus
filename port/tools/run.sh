#!/bin/sh
# run.sh [ft] [args] - run the native build headless against a scratch copy of the game.
#   GAME=<dir with DARKLAND.EXE> (default: ~/dl_raw/extracted/DARKLAND), WORK=<scratch dir>
# Other env vars pass through (DL_SCRIPT, DL_TRACE_DISPATCH, ...).
HERE=$(cd "$(dirname "$0")/../.." && pwd)
GAME=${GAME:-/home/claude/dl_raw/extracted/DARKLAND}
WORK=${WORK:-/tmp/dlrun}
BIN=$HERE/build/port/darklands
if [ "$1" = ft ]; then BIN=$HERE/build/port_ft/darklands; shift; fi
if [ ! -d "$WORK/game" ]; then mkdir -p "$WORK"; cp -r "$GAME" "$WORK/game"; fi
cd "$WORK" && DL_HEADLESS=${DL_HEADLESS-1} exec timeout ${TIMEOUT:-30} "$BIN" "$WORK/game" "$@"
