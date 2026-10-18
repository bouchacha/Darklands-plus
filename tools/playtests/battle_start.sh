#!/bin/bash
# battle_start.sh NAME EXE [NAME2 EXE2]: start lockstep session(s), Quickstart, leave the city, and hunt
# for an overland encounter (tools/playtests/hunt.py). Then look at $LOCKSTEP_DIR/NAME/hunt.png and pick
# the fighting option (see battle.txt). LOCKSTEP_DIR defaults to build/qa2/sess.
cd /home/claude/darklands-re
export LOCKSTEP_DIR=${LOCKSTEP_DIR:-build/qa2/sess}
L="python3 -I tools/playtests/lockstep.py"
$L start $1 $2; N="$1"
if [ -n "$3" ]; then $L start $3 $4; N="$1 $3"; fi
$L do "QUICKSTART; click; wait 3; click; wait 3; click; wait 3; click; wait 3; click; wait 4; click; wait 6; at 300 25; click; wait 8; click; wait 4; shot a" $N
python3 -I tools/playtests/hunt.py $N
