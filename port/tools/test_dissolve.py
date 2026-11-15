#!/usr/bin/env python3
"""Exercise Dissolve Lock through a copied Windows game with SDL's dummy video driver.

Requires a four-member save. A generated mine and controlled inventory changes reproduce
the stale-slot and wrong-member conditions, then normal D input or the combat action loop
executes the order. The source installation is only read.

python port/tools/test_dissolve.py --exe build/port-review/darklands.exe --game GAME --out build/dissolve-test
Add --baseline EXE to repeat the failure controls on an earlier executable.
"""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import time


class Game:
    def __init__(self, folder, executable, source, save_name, enabled):
        self.folder = folder
        folder.mkdir(parents=True, exist_ok=False)
        self.game = folder / "game"
        shutil.copytree(source, self.game, ignore=shutil.ignore_patterns(
            "Darklands+ save backups", "darklands_port*.log", "darklands*.txt"))
        saves = self.game / "SAVES"
        # This is a disposable copy. Give the title screen exactly one starting save.
        start = (saves / save_name).read_bytes()
        companion = saves / Path(save_name).with_suffix(".bsv")
        battle = companion.read_bytes() if companion.exists() else None
        for path in saves.iterdir():
            if path.suffix.lower() in (".sav", ".bsv"):
                path.unlink()
        start = bytearray(start)
        struct.pack_into("<H", start, 0x82, 0x1d)  # enter the inn card, where ufight runs
        (saves / "dksave0.sav").write_bytes(start)
        if battle is not None:
            (saves / "dksave0.bsv").write_bytes(battle)
        settings = ["saves.restart_on_end = 0", "display.maximized = 0",
                    "display.opening = 0", "display.opening_seen = 1",
                    "battle.controls = 1", "battle.pauses = 0", f"fix.dissolve = {int(enabled)}"]
        (self.game / "darklands_port.ini").write_text("\n".join(settings) + "\n")
        self.control = folder / "ctl.txt"
        self.control.touch()
        self.log_path = self.game / "darklands_port.log"
        env = dict(os.environ, DL_CTL=str(self.control), SDL_AUDIODRIVER="dummy",
                   SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software", DL_FIX_LOG="1")
        env.pop("DL_HEADLESS", None)
        env.pop("DL_SCRIPT", None)
        self.process = subprocess.Popen([str(executable), str(self.game)], env=env,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))

    def log(self):
        return self.log_path.read_text(encoding="latin1") if self.log_path.exists() else ""

    def send(self, command, delay=0.18):
        with self.control.open("ab") as stream:
            stream.write((command + "\n").encode("ascii"))
        time.sleep(delay)

    def batch(self, commands, delay=0.35):
        # run_ctl inserts each newly read command at the same position, reversing a batch.
        with self.control.open("ab") as stream:
            stream.write("".join(command + "\n" for command in reversed(commands)).encode("ascii"))
        time.sleep(delay)

    def wait(self, text, timeout=8, after=0):
        end = time.monotonic() + timeout
        while time.monotonic() < end and self.process.poll() is None:
            if text in self.log()[after:]:
                return
            time.sleep(0.08)
        raise AssertionError(f"did not reach {text!r}; process exit {self.process.poll()}")

    def dump(self, name):
        path = self.folder / (name + ".bin")
        self.send("udsdump:" + path.as_posix())
        return path.read_bytes()

    def inventory(self, member):
        before = len(self.log())
        self.send(f"uinv:{member}")
        self.wait(f"inv {member} ", after=before)
        line = next(line for line in self.log()[before:].splitlines() if line.startswith(f"inv {member} "))
        return [tuple(map(int, values)) for values in re.findall(r"\[\d+\] (\d+)/\d+ q(\d+) x(\d+)", line)]

    def set_inventory(self, member, items):
        commands = []
        for slot, (code, kind, quality, count) in enumerate(items):
            record = struct.pack("<HBBBB", code, kind, quality, count, 0)
            address = 0x3e + member * 0x180 + slot * 6
            for offset, byte in enumerate(record):
                commands.append(f"upoke:2e38,{address + offset:x},{byte:x}")
        commands.append(f"upoke:321a,{0x9c69 + member * 0x80 + 0x6a:x},{len(items):x}")
        self.batch(commands)

    def start_battle(self):
        self.wait("title: open", 15)
        self.send("ufight:15,4885,8,3,8,2,2,4,-1,0,0")
        self.send("key:Return", 0.6)
        self.wait("title: closed")
        for _ in range(10):
            self.send("key:Return", 0.35)
            self.send("umode:")
            if "mode 4" in self.log():
                break
        self.wait("mode 4")
        for member in range(4):
            self.send(f"ustyle:{member},0", 0.03)
        self.send("ufloor:")
        chest = re.search(r"chest 0 at (\d+),(\d+) (\d+)x(\d+) lock (\d+)", self.log())
        assert chest and int(chest[5]) > 127, "fixture needs a locked mine chest"
        x, y, width, height = map(int, chest.groups()[:4])
        self.set_inventory(1, [(42, 0x49, 25, 3), (48, 0x4f, 25, 1), (5, 3, 25, 1)])
        self.set_inventory(3, [(5, 3, 25, 1), (96, 0x85, 25, 1), (99, 0x88, 25, 1)])
        self.send("uleader:3")
        self.send(f"uplace:3,{x + min(12, width - 1)},{y + min(10, height - 1)}")
        self.send(f"uplace:1,{x + width + 9},{y + 20}")
        self.send("ucall:17,1ef8,0000,3", 0.25)
        # ufight enters from a card menu that already holds one pause. Release this
        # test-only inherited pause so subsequent input and actions run normally.
        self.batch(["upoke:321a,c7cd,0", "upoke:321a,a6b3,0",
                    "upoke:321a,a695,0", "upoke:321a,d6f1,0", "upoke:321a,a6c1,0",
                    "upoke:321a,c102,0", "upoke:321a,07d3,0"], 0.5)
        self.send("ucall:14,14f7,0008")  # refresh mouse targets after controlled placement

    def close(self):
        if self.process.poll() is None:
            self.send("quit:", 0.1)
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.terminate()
                self.process.wait(timeout=3)


def run_case(root, executable, source, save_name, case, enabled, expected_fixed):
    game = Game(root / case, executable, source, save_name, enabled)
    result = {"case": case, "fixed": expected_fixed}
    try:
        game.start_battle()
        queued = "queued" in case
        missing = "missing" in case
        if queued:
            # Pause a queued order, then move its potion away from cached slot 2.
            game.send("uleader:0")
            game.batch(["upoke:321a,a695,1", "upoke:321a,a6b3,0", "upoke:321a,c7d0,2",
                        "upoke:321a,ca99,0", "upoke:2e38,04c4,63", "upoke:2e38,04c5,0",
                        "upoke:2e38,04c6,88", "upoke:2e38,04c8,2",
                        "upoke:2e38,04ca,60", "upoke:2e38,04cb,0",
                        "upoke:2e38,04cc,85", "upoke:2e38,04ce,3"])
            if missing:
                game.send("upoke:2e38,04c4,60")
        else:
            game.send("move:0,199")
            if "held" in case:
                boundary = len(game.log())
                game.send("uhot:")
                hotspot = re.search(r"hot \d+ cbt 1 x (\d+)\.\.(\d+) y (\d+)\.\.(\d+)", game.log()[boundary:])
                assert hotspot, "the second member must have a visible mouse target"
                left, right, top, bottom = map(int, hotspot.groups())
                game.send(f"move:{60 + (left + right) // 2},{(top + bottom) // 2}")
                game.send("press:1")  # the running key sees the member under the held pointer
        before = [game.inventory(member) for member in range(4)]
        state = game.dump("before")
        assert queued or state[0x96f9] == 1, "Dissolve Lock must be offered"
        if "held" in case:
            assert state[0xa6b2] == 1 and state[0xa695] == 0
        boundary = len(game.log())
        if queued:
            game.batch(["upoke:321a,a6c7,7", "upoke:321a,a695,0"], 0.1)
        else:
            game.send("key:D", 0.1)
        if missing and expected_fixed:
            game.wait("Dissolve Lock needs Eater-Water", after=boundary)
            time.sleep(3.5)  # longer than the original timed message before opening the chest
        else:
            game.wait("loot screen: open", after=boundary)
        after = [game.inventory(member) for member in range(4)]
        if expected_fixed and missing:
            assert before == after, "a missing potion must leave every inventory intact"
            assert "loot screen: open" not in game.log()[boundary:]
        elif expected_fixed:
            expected = list(before[3])
            at = next(i for i, item in enumerate(expected) if item[0] == 99)
            code, quality, count = expected[at]
            if count == 1:
                expected.pop(at)
            else:
                expected[at] = (code, quality, count - 1)
            assert after[3] == expected, (before[3], after[3])
            assert after[:3] == before[:3]
        elif queued:
            expected = list(before[3])
            code, quality, count = expected[2]
            expected[2] = (code, quality, count - 1)
            assert code != 99 and after[3] == expected, "failure control must consume the stale non-potion"
        else:
            assert after[3] == before[3], "failure control must leave the acting member's potion unused"
            if "held" in case:
                expected = list(before[1])
                code, quality, count = expected[0]
                expected[0] = (code, quality, count - 1)
                assert code != 99 and after[1] == expected, "failure control must charge the other member's item"
        if "held" in case:
            game.send("move:0,199")
            game.send("release:1")
        game.send("key:Escape", 0.3)
        assert "left the stack wrong" not in game.log()
        result.update(before=before, after=after, passed=True)
    finally:
        game.close()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--game", required=True, type=Path)
    parser.add_argument("--save", default="dksave0.sav", help="a four-member save inside the game folder")
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--only", help="comma-separated scenario names")
    args = parser.parse_args()
    root = args.out.resolve()
    root.mkdir(parents=True, exist_ok=True)
    selected = set(args.only.split(",")) if args.only else None
    cases = [("key-fixed", True), ("key-held-fixed", True), ("queued-moved-fixed", True), ("queued-missing-fixed", True),
             ("key-disabled", False), ("key-held-disabled", False), ("queued-moved-disabled", False), ("queued-missing-disabled", False)]
    results = []
    for case, enabled in cases:
        if selected and case not in selected:
            continue
        result = run_case(root, args.exe.resolve(), args.game.resolve(), args.save, case, enabled, enabled)
        results.append(result)
        print(case, "PASS", flush=True)
        (root / "results.json").write_text(json.dumps(results, indent=2))
    if args.baseline:
        for case in ("key-baseline", "key-held-baseline", "queued-moved-baseline", "queued-missing-baseline"):
            if selected and case not in selected:
                continue
            result = run_case(root, args.baseline.resolve(), args.game.resolve(), args.save, case, True, False)
            results.append(result)
            print(case, "PASS", flush=True)
            (root / "results.json").write_text(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()
