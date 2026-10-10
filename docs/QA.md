# QA round 1 (2026-10-09)
OK in both builds: tavern intro, inn (after fix 1), market/barter, churches, side streets, character sheets, equipment, formulae, saints, overland travel and encounters, save game, Create a New World (first steps).
Bugs:
1. FIXED: 32 card scripts called sub_s014_0e76_199c instead of sub_s029_0e76_199c (sibling overlays share address 0e76:199c); identical bytes in the original layout, wrong target once s014 moved. dec16 now prefers names reachable from the caller.
2. OPEN: battle starts garbled (sidebar garbage, view off-party, not paused) when battlefield drafts (s002, s004-s013) and combat sub-module drafts (s015-s025) are both in. Candidates s020/s025. Untested split builds: build/qa/one_s2c15.EXE, one_b4c15.EXE.
3. OPEN: with the s014 drafts, loot after a battle lists garbage items: s014_0e76 end-of-battle or s014_18bf attack code.
Same-second runs share the RNG seed: start original and test build together for lockstep comparisons (tools/playtests/lockstep.py).
