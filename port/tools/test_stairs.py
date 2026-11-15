#!/usr/bin/env python3
"""Check stairs in a generated mine using disposable game copies and SDL dummy video.

python port/tools/test_stairs.py --exe EXE --game GAME --save dksave0.sav --out build/stairs-tests

A four-member save supplies the party. The copied fixture sets the party seed and inn
card, then ufight builds a mine. Diagnostic calls exercise the real take_exit routine
and its normal Darklands+ hooks. No licensed files are bundled with this test.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import time


class Game:
    def __init__(self, args, case, enabled):
        self.root = Path(tempfile.mkdtemp(prefix=case + "-", dir=args.out))
        self.folder = self.root / "game"
        shutil.copytree(args.game, self.folder, ignore=shutil.ignore_patterns(
            "Darklands+ save backups", "darklands_port*.log", "darklands*.txt"))
        saves = self.folder / "SAVES"
        source = next(path for path in saves.iterdir() if path.name.lower() == args.save.lower())
        sav = bytearray(source.read_bytes())
        companion = source.with_suffix(".bsv")
        bsv = companion.read_bytes() if companion.exists() else None
        for path in saves.iterdir():
            if path.suffix.lower() in (".sav", ".bsv"):
                path.unlink()
        struct.pack_into("<H", sav, 0x64, 368)
        struct.pack_into("<H", sav, 0x82, 0x1d)
        (saves / "dksave0.sav").write_bytes(sav)
        if bsv is not None:
            (saves / "dksave0.bsv").write_bytes(bsv)
        (self.folder / "darklands_port.ini").write_text(
            "display.opening = 0\ndisplay.opening_seen = 1\nsaves.restart_on_end = 0\n"
            f"fix.stairsreturn = {int(enabled)}\n")
        self.control = self.root / "ctl.txt"
        self.control.touch()
        self.logfile = self.folder / "darklands_port.log"
        env = dict(os.environ, DL_CTL=str(self.control), SDL_AUDIODRIVER="dummy",
                   SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
        env.pop("DL_HEADLESS", None)
        env.pop("DL_SCRIPT", None)
        self.process = subprocess.Popen([str(args.exe), str(self.folder)], env=env,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))

    def log(self):
        return self.logfile.read_text(encoding="latin1") if self.logfile.exists() else ""

    def send(self, command, delay=0.2):
        assert self.process.poll() is None, self.log()[-1200:]
        with self.control.open("ab") as stream:
            stream.write((command + "\n").encode("ascii"))
        time.sleep(delay)

    def batch(self, commands):
        # run_ctl inserts new commands at the same position, reversing a received batch.
        with self.control.open("ab") as stream:
            stream.write("".join(command + "\n" for command in reversed(commands)).encode("ascii"))
        time.sleep(0.3)

    def wait(self, text, timeout=12):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline and self.process.poll() is None:
            if text in self.log():
                return
            time.sleep(0.1)
        raise AssertionError(f"Did not reach {text!r}\n{self.log()[-1200:]}")

    def dump(self, name, whole=False):
        path = self.root / (name + ".bin")
        self.send(("umemdump:" if whole else "udsdump:") + path.as_posix())
        return path.read_bytes()

    def start(self):
        self.wait("title: open", 20)
        self.send("ufight:15,4885,8,3,8,2,2,4,-1,0,0")
        self.send("key:Return", 0.5)
        self.wait("title: closed")
        for _ in range(10):
            self.send("key:Return", 0.35)
            self.send("umode:")
            if "mode 4" in self.log():
                break
        self.wait("mode 4")
        self.send("upoke:321a,a6c1,1")

    def cell(self, x, y):
        at = len(self.log())
        self.send(f"ufar:{(x // 32 * 40 + y // 16) * 4:x},4")
        line = next(line for line in self.log()[at:].splitlines() if line.startswith("far 20a5:"))
        return list(map(int, line.split(":")[-1].split()))

    def close(self):
        if self.process.poll() is None:
            self.send("quit:", 0.1)
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.terminate()
                self.process.wait(timeout=3)


def member(data, who=1):
    x, y = struct.unpack_from("<hh", data, 0x9c55 + who * 128 + 4)
    return dict(x=x, y=y, floor=data[0xa6b4 + who], shown=data[0x11ca],
                party=data[0xc623], back=data[0xc8b2 + who])


def split(game, enabled, result):
    game.send("ucall:23,1ef8,1618,0,1", 0.4)
    down = game.dump("down")
    assert down[0xa6b5] == 1
    result["down"] = member(down)
    game.send(f"ucall:23,1ef8,1618,{down[0xc8b3]},1", 0.4)
    up = game.dump("return-with-other-floor-shown")
    result["return"] = member(up)
    if not enabled:
        assert up[0xa6b5] == 1, "failure control unexpectedly returned upstairs"
        game.send("ucall:24,1ef8,059a,1")
        game.send(f"ucall:23,1ef8,1618,{up[0xc8b3]},1", 0.4)
        up = game.dump("return-with-own-floor-shown")
        result["return_after_view_switch"] = member(up)
    assert up[0xa6b5] == 0
    position = member(up)
    cell = game.cell(position["x"], position["y"])
    assert bool(cell[3] & 128) == (not enabled), "unexpected landing cell"
    result["landing_cell"] = cell


def together(game, case, result):
    game.batch([f"upoke:321a,{0x9c69 + i * 128:x},0" for i in range(5, 13)] +
               ["upoke:321a,a6c1,0", f"upoke:321a,c102,{2 if case == 'singlefile' else 1}"])
    if case == "unconscious":
        game.send("upoke:321a,9de9,2")
    if case == "singlefile":
        before = game.dump("follower-before")
        game.send("ucall:23,1ef8,1618,0,1", 0.4)
        after = game.dump("follower-after")
        assert [member(before, i) for i in range(4)] == [member(after, i) for i in range(4)]
        result["follower_stopped"] = True
    game.send("ucall:23,1ef8,1618,0,0", 0.4)
    down = game.dump("down")
    expected = [1, 1, 1, 0] if case == "unconscious" else [1] * 4
    assert list(down[0xa6b4:0xa6b8]) == expected
    result["down"] = [member(down, i) for i in range(4)]
    game.send(f"ucall:23,1ef8,1618,{down[0xc8b2]},0", 0.4)
    up = game.dump("up")
    assert list(up[0xa6b4:0xa6b8]) == [0] * 4
    result["up"] = [member(up, i) for i in range(4)]
    for who in range(3 if case == "unconscious" else 4):
        position = member(up, who)
        assert not game.cell(position["x"], position["y"])[3] & 128


def refusal(game, case, result):
    if case == "invalid-other-view":
        game.send("ucall:23,1ef8,1618,0,1", 0.4)
    if case == "blocked":
        # Keep the entire arrival region in the mine's unused corner, on this floor.
        record = struct.pack("<hhBBbbhhbb", 32, 16, 1, 1, 0, -1, 32, 16, 0, 0)
        game.batch([f"upoke:20a5,{0x2fb8 + i:x},{byte:x}" for i, byte in enumerate(record)])
    game.send("urand:1234567")
    before = game.dump("before")
    memory_before = game.dump("memory-before", whole=True)
    game.send(f"ucall:23,1ef8,1618,{0 if case == 'blocked' else 6},1", 0.6)
    after = game.dump("after")
    memory_after = game.dump("memory-after", whole=True)
    assert member(before) == member(after)
    assert before[0x7b20:0x7b24] == after[0x7b20:0x7b24], "query advanced the RNG"
    assert before[0x9ed5:0xa2d5] == after[0x9ed5:0xa2d5], "enemy records changed"
    floor = (0x200 + 0x20a5) * 16  # the runtime's fixed LOADSEG and floor-image segment
    assert memory_before[floor:floor + 0x33fc] == memory_after[floor:floor + 0x33fc]
    result.update(refused=True, rng_preserved=True, enemies_preserved=True, floor_preserved=True)


def run(args, case):
    enabled = case != "control"
    game = Game(args, case, enabled)
    result = dict(case=case, folder=str(game.root))
    try:
        game.start()
        if case in ("control", "split"):
            split(game, enabled, result)
        elif case in ("group", "singlefile", "unconscious"):
            together(game, case, result)
        else:
            refusal(game, case, result)
        assert not re.search(r"hook:.*(?:stack wrong|no function)|FATAL", game.log())
        result["passed"] = True
    except Exception as error:
        result["error"] = repr(error)
    finally:
        game.close()
        result["exit_code"] = game.process.returncode
    print(json.dumps(result), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--game", type=Path, required=True)
    parser.add_argument("--save", default="dksave0.sav")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    args.exe = args.exe.resolve()
    args.game = args.game.resolve()
    args.out = args.out.resolve()
    if args.out == args.game or args.game in args.out.parents:
        parser.error("The output folder must be outside the copied source game folder.")
    args.out.mkdir(parents=True, exist_ok=True)
    cases = ("control", "split", "group", "singlefile", "unconscious", "invalid", "invalid-other-view", "blocked")
    with ThreadPoolExecutor(max_workers=3) as pool:
        results = list(pool.map(lambda case: run(args, case), cases))
    (args.out / "results.json").write_text(json.dumps(results, indent=2))
    raise SystemExit(0 if all(result.get("passed") and result["exit_code"] == 0 for result in results) else 1)


if __name__ == "__main__":
    main()
