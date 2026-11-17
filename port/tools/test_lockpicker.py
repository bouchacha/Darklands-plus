"""Exercise the party lock picker in a copied game through Orders and P.

Use a four-member save. The existing dissolve fixture starts a generated mine
beside chest 0. Each case has its own process and disposable copy of the game.
python port/tools/test_lockpicker.py --exe EXE --game GAME --out DIRECTORY
"""
import argparse
import concurrent.futures
from pathlib import Path
import re
import struct

from test_dissolve import Game


CASES = ("percentages", "opt-out", "choose-nonleader", "default-best",
         "shared-picks", "separate-packs", "first-pick", "no-chance", "unavailable", "door")
MEMBER = 0x9c69
SKILL = 0x9c55 + 0x79
FLAGS = 0xd68d
LEADER = 0x907b
MARCH = slice(0xc624, 0xc62a)


def poke(seg, addr, value):
    return f"upoke:{seg:x},{addr:x},{value:x}"


def blob(seg, addr, data):
    return [poke(seg, addr + n, v) for n, v in enumerate(data)]


def mem(game, name):
    path = game.folder / (name + ".bin")
    game.send("umemdump:" + path.as_posix())
    return path.read_bytes()


def segment(image, seg):
    start = (0x200 + seg) * 16
    return image[start:start + 0x10000]


def state(image):
    ds = segment(image, 0x321a)
    return ds[LEADER], ds[MARCH], [ds[FLAGS + 2*i] & 0x60 for i in range(4)]


def packs(image):
    ds = segment(image, 0x321a)
    far = segment(image, 0x2e38)
    return [far[0x3e + 384*i:0x3e + 384*i + ds[MEMBER + 128*i + 0x6a]*6]
            for i in range(4)]


def check(args, case):
    g = Game(args.out / case, args.exe, args.game, args.save, True)
    try:
        g.start_battle()
        # Use a deterministic difficulty, with four distinct percentages and
        # an able leader (Marta, slot 3) already standing beside the chest.
        skills = [25, 26, 34, 35]
        qualities = [[25] for _ in range(4)]
        difficulty = 35
        selected = None
        if case == "choose-nonleader":
            skills = [100, 100, 10, 0]
            selected = 1
        elif case == "default-best":
            skills = [5, 15, 99, 0]
            selected = 2
        elif case == "shared-picks":
            qualities = [[], [], [45], []]
            skills = [0, 15, 0, 0]
            selected = 1
        elif case == "separate-packs":
            qualities = [[25], [], [35], [26]]
            g.send("uoption:inventory.shared=0")
        elif case == "first-pick":
            qualities = [[20, 35], [25], [25], [25]]
            g.send("uoption:inventory.shared=0")
            g.send("uoption:fix.bestpick=0")
        elif case == "opt-out":
            g.send("uoption:battle.choose_lockpicker=0")
        elif case == "no-chance":
            difficulty = 95
        elif case == "door":
            skills[1] = 100
            selected = 1

        for i, qs in enumerate(qualities):
            g.set_inventory(i, [(64, 0x70, q, 2) for q in qs])
        setup = [poke(0x321a, SKILL + 128*i, s) for i, s in enumerate(skills)]
        setup.extend(poke(0x321a, FLAGS + 2*i, 0) for i in range(4))
        lock = 0x3016
        if case == "door":
            # Place a locked door and its activation rectangle at 135,460 in
            # the mine. Move the leader beside it so the game builds Door orders.
            rec = struct.pack("<hhBBBBBBBBBB", 135, 460, 48, 24, 255, 128 + difficulty,
                              0x44, 29, 0, 0, 255, 0)
            setup.extend(blob(0x20a5, 0x1f54, rec))
            # A closed east-facing door must also have the closed wall nibble.
            # This generated mine originally has an open wall at this cell.
            setup.extend([poke(0x20a5, (4*40 + 29)*4, 2),
                          poke(0x20a5, (5*40 + 29)*4 + 1, 2),
                          poke(0x20a5, 0x1f52, 1), "uplace:3,144,470"])
            lock = 0x1f5b
        setup.append(poke(0x20a5, lock, difficulty | 128))
        if case == "unavailable":
            setup.extend([poke(0x321a, MEMBER + 128, 2), poke(0x321a, 0xa6b4 + 2, 1)])
        g.batch(setup)
        before = mem(g, "prepared")
        ds = segment(before, 0x321a)
        assert ds[0x96e5] == 1 and ds[0xca99] == (case == "door"), "fixture has wrong lock target"
        if case == "door":
            assert ds[0xc678 + ds[LEADER]] == 1, "leader must face a closed door"
        begin = len(g.log())
        if case == "opt-out":
            g.send("key:P")
            g.wait("loot screen: open", after=begin)
            after = mem(g, "finished")
            assert "lock picker:" not in g.log()[begin:]
            assert state(before)[:2] == state(after)[:2]
            assert state(after)[2] == [0, 0, 0, 0x20]
            assert segment(after, 0x20a5)[lock] == 0
            return case
        from_menu = case in ("choose-nonleader", "door")
        g.send("umenu:1,11" if from_menu else "key:P")
        g.wait("lock picker:", after=begin)
        view = g.log()[begin:]
        if from_menu:
            assert "menu: Orders > Pick Lock..." in view, view
        rows = {int(i): (int(s), int(q), int(chance)) for i, s, q, chance in re.findall(
            r"lock picker: member (\d+), Artifice (\d+), picks (-?\d+), chance (\d+)%", view)}
        assert rows, view[-900:]
        for i in range(4):
            if case == "unavailable" and i in (1, 2):
                assert i not in rows or rows[i][2] == 0, rows
                continue
            if case not in ("separate-packs", "first-pick"):
                quality = max(q for qlist in qualities for q in qlist)
            else:
                quality = qualities[i][0] if qualities[i] else -1
            # Exhaustively count successful die faces for the original strict comparison.
            chance = sum(difficulty < skills[i] + quality - 25 + roll for roll in range(1, 11))*10 if quality >= 0 else 0
            assert rows[i] == (skills[i], quality, chance), (i, rows, quality, chance)
        if case == "percentages":
            g.send("wshot:" + (g.folder / "chooser.bmp").as_posix())
        if selected is None:
            if case in ("percentages", "no-chance"):
                g.send("key:1")              # impossible choice must keep the picker open
            if case == "unavailable":
                g.send("key:2")
            g.send("key:Escape")
            g.wait("-> -1", after=begin)
        else:
            command = "key:Return" if case == "default-best" else f"key:{selected+1}"
            if case == "choose-nonleader":
                # Center of the second roster row, using real SDL pointer input.
                command = "ugclick:160,94"
            g.send(command)
            g.wait(f"-> {selected}", after=begin)
            if case == "door":
                for _ in range(30):
                    g.send("ufloor:", 0.2)
                    if re.search(r"door 0 at .* open 1", g.log()[begin:]):
                        break
                else:
                    raise AssertionError("door remained shut after a successful roll")
            else:
                g.wait("loot screen: open", after=begin, timeout=10)
        after = mem(g, "finished")
        assert state(before)[:2] == state(after)[:2], "changed leader or marching order"
        flags = state(after)[2]
        actual_lock = segment(after, 0x20a5)[lock]
        if selected is None:
            assert state(before) == state(after) and actual_lock == (difficulty | 128)
            assert packs(before) == packs(after), "opening and canceling moved items"
        else:
            assert flags[selected] == 0x20 and sum(flags) == 0x20, (selected, flags)
            assert actual_lock == 0, actual_lock
            if case == "shared-picks":
                after_packs = packs(after)
                assert after_packs[1][0:5] == bytes((64, 0, 0x70, 45, 1))
                assert after_packs[2][4] == 1
            if case == "door":
                assert segment(after, 0x20a5)[0x33d3] == 1, "door stayed closed"
        assert "left the stack wrong" not in g.log()
        return case
    finally:
        g.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("exe", "game", "out"):
        parser.add_argument("--"+name, required=True, type=Path)
    parser.add_argument("--save", default="dksave0.sav")
    parser.add_argument("--only", choices=CASES, action="append")
    parser.add_argument("--jobs", default=1, type=int)
    args = parser.parse_args()
    args.exe, args.game, args.out = [v.resolve() for v in (args.exe, args.game, args.out)]
    args.out.mkdir(parents=True, exist_ok=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        cases = {pool.submit(check, args, c): c for c in args.only or CASES}
        failed = []
        for future in concurrent.futures.as_completed(cases):
            try:
                print(f"{future.result()} PASS", flush=True)
            except Exception as exc:
                print(f"{cases[future]} FAIL {exc!r}", flush=True)
                failed.append(cases[future])
        if failed:
            raise SystemExit(1)


if __name__ == "__main__":
    main()
