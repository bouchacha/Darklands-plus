#!/bin/bash
# mkbuild.sh <workdir> <out.EXE> <wip-relative draft files...>
# Build a mod EXE with ONLY the given src/wip drafts overlaid (for bisecting play-test bugs).
# The work dir gets a copy of build/qa2/analysis + force_db.json, so several builds can run in parallel.
cd /home/claude/darklands-re
W=$1; OUT=$2; shift 2
mkdir -p $W; [ -d $W/analysis ] || cp -r build/qa2/analysis $W/; [ -f $W/force_db.json ] || cp build/qa2/force_db.json $W/
D=$W/ovl; rm -rf $D; mkdir -p $D
for f in "$@"; do mkdir -p $D/$(dirname $f); cp src/wip/$f $D/$f; done
python3 -I tools/build.py /home/claude/dl_raw/extracted/DARKLAND/DARKLAND.EXE --skip-analysis --mod --overlay $D --work $W --out $OUT > $W/build.log 2>&1
tail -1 $W/build.log
