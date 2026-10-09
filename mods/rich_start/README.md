# Example mod: rich start

Changes one line in the opening card script (`$Initi00`, overlay s044): the party starts
with 5000 florins instead of 200. Build it with

    python3 -I tools/build.py /path/to/DARKLAND.EXE --skip-analysis --mod --overlay mods/rich_start --out build/DARKLAND_rich.EXE

and copy the result over DARKLAND.EXE in a copy of the game directory.
