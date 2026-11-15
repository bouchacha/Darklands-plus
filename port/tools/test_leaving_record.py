#!/usr/bin/env python3
"""Check departure, rejoin and save/load with the fix on and off, in copied games.

Requires a prepared native game folder and an out-of-battle save with two able members.
python port/tools/test_leaving_record.py --exe EXE --game GAME --save SAVE --out build/leaving-record
SDL's dummy video driver runs the native UI without a desktop window. Each scenario
constructs an inn-card fixture from the supplied save, changes knowledge and inventory,
retires a member through the card, then calls the game's party_add and save/load paths.
The inn roster's separate rule that clears equipment on A/add is outside this check.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import time

DS = (0x0200 + 0x321A) * 16
FAR = (0x0200 + 0x2E38) * 16
HEADER = 0x189
RECORD = 0x22A


def read_word(data, offset):
    return struct.unpack_from("<h", data, offset)[0]


def tail(data, member):
    return (data[FAR + 0x082C + member * 20:FAR + 0x082C + (member + 1) * 20]
            + data[FAR + 0x07BE + member * 22:FAR + 0x07BE + (member + 1) * 22]
            + data[FAR + 0x003E + member * 0x180:FAR + 0x003E + (member + 1) * 0x180])


def quantity(data, code, quality):
    total = 0
    for member in range(5):
        if data[DS + 0x9C69 + member * 0x80] == 0:
            continue
        count = data[DS + 0x9CD3 + member * 0x80]
        for item in range(min(count, 64)):
            offset = FAR + 0x003E + member * 0x180 + item * 6
            if read_word(data, offset) & 0xFFF == code and data[offset + 3] == quality:
                total += data[offset + 4]
    return total


class Session:
    def __init__(self, args, name, fixture, battle, settings):
        self.root = Path(tempfile.mkdtemp(prefix=name + "-", dir=args.out))
        self.game = self.root / "game"
        shutil.copytree(args.game, self.game, ignore=shutil.ignore_patterns(
            "SAVES", "Darklands+ save backups", "darklands_port*.log",
            "darklands*.txt", "darklands_levels*.bin"))
        saves = self.game / "SAVES"
        saves.mkdir()
        shutil.copy(args.game / "SAVES" / "DEFAULT", saves)
        (saves / "dksave0.sav").write_bytes(fixture)
        if battle is not None:
            (saves / "dksave0.bsv").write_bytes(battle)
        (self.game / "darklands_port.ini").write_text(
            "saves.restart_on_end = 0\ndisplay.maximized = 0\n"
            "display.opening = 0\ndisplay.opening_seen = 1\n" + settings)
        self.control = self.root / "ctl.txt"
        self.control.touch()
        self.logfile = self.game / "darklands_port.log"
        env = dict(os.environ, DL_CTL=str(self.control), SDL_AUDIODRIVER="dummy",
                   SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
        env.pop("DL_SCRIPT", None)
        env.pop("DL_HEADLESS", None)  # Keep native UI active on SDL's hidden dummy surface.
        self.stderr = (self.root / "stderr.txt").open("wb")
        self.process = subprocess.Popen(
            [str(args.exe), str(self.game)], env=env, stdout=self.stderr, stderr=self.stderr,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)

    def log(self):
        if self.logfile.exists():
            return self.logfile.read_text(encoding="latin1")
        return (self.root / "stderr.txt").read_text(encoding="latin1")

    def command(self, command, delay=0.22):
        if self.process.poll() is not None:
            raise RuntimeError(f"game ended with {self.process.returncode}")
        with self.control.open("ab") as stream:
            stream.write((command + "\n").encode("utf-8"))
        time.sleep(delay)

    def wait(self, text, timeout=8, since=0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline and self.process.poll() is None:
            if text in self.log()[since:]:
                return
            time.sleep(0.1)
        raise AssertionError(f"did not reach {text!r}\n{self.log()[-1500:]}")

    def memory(self, name):
        path = self.root / (name + ".bin")
        self.command("umemdump:" + path.as_posix())
        self.wait("memory dumped to " + path.as_posix())
        return path.read_bytes()

    def close(self):
        if self.process.poll() is None:
            self.command("quit:")
        try:
            self.process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            self.process.terminate()
            self.process.wait(timeout=3)
        self.stderr.close()
        return self.process.returncode


def scenario(args, fixture, battle, member, record_id, fix, shared):
    name = f"fix-{fix}-shared-{shared}"
    settings = f"fix.leavingrecord = {fix}\ninventory.shared = {shared}\n"
    session = Session(args, name, fixture, battle, settings)
    result = {"case": name, "folder": str(session.root), "fix": fix, "shared": shared}
    try:
        session.wait("opening: skipped", timeout=20)
        for _ in range(2):
            session.command("key:Space")
            if "title: open" in session.log():
                break
        session.wait("title: open")
        session.command("key:Return", 0.6)
        session.wait("title: closed")
        time.sleep(0.6)
        before = session.memory("before")
        saint = next((i, bit) for i in range(17) for bit in (1, 2, 4, 8, 16, 32, 64, 128)
                     if not before[FAR + 0x082C + member * 20 + i] & bit)
        saint_offset = 0x082C + member * 20 + saint[0]
        formula = next(i for i in range(22) if not before[FAR + 0x07BE + member * 22 + i] & 1)
        formula_offset = 0x07BE + member * 22 + formula
        session.command(f"upoke:2e38,{saint_offset:x},{before[FAR + saint_offset] | saint[1]:x}")
        session.command(f"upoke:2e38,{formula_offset:x},{before[FAR + formula_offset] | 1:x}")
        session.command(f"uitem:{member},44,35,2")
        changed = session.memory("changed")
        assert quantity(changed, 44, 35) == quantity(before, 44, 35) + 2
        roster = session.game / "charactr.tmp"
        at = 32 + record_id * RECORD
        old_record = roster.read_bytes()[at:at + RECORD]

        session.command(f"uchoose:{member + 1}")
        for step in range(12):
            session.command("key:Return", 0.3)
            departed = session.memory(f"retire-{step}")
            if departed[DS + 0x9C69 + member * 0x80] == 0:
                break
        assert departed[DS + 0x9C69 + member * 0x80] == 0, "retirement did not occur"
        stored = roster.read_bytes()[at:at + RECORD]
        result["stored_tail_matches_departing_member"] = stored[0x80:] == tail(departed, member)
        assert result["stored_tail_matches_departing_member"] == bool(fix)
        if not fix:
            assert stored[0x80:] == old_record[0x80:]
        if shared:
            assert quantity(departed, 44, 35) == quantity(changed, 44, 35)
            result["shared_spares_retained"] = True

        session.command(f"ucall:37,1462,3cd6,{record_id}", 0.5)
        rejoined = session.memory("rejoined")
        ids = roster.read_bytes()[:12]
        returned = next(i for i in range(5) if read_word(ids, 2 + 2 * i) == record_id)
        assert rejoined[DS + 0x9C69 + returned * 0x80] == 1
        assert tail(rejoined, returned) == stored[0x80:]
        if fix:
            assert tail(rejoined, returned)[:42] == tail(changed, member)[:42]
            assert quantity(rejoined, 44, 35) == quantity(changed, 44, 35)
        session.command("uquicksave:", 0.7)
        session.wait("Game saved")
        session.command("uload:dksave1.sav", 0.7)
        session.wait("Loaded dksave1.sav")
        time.sleep(0.5)
        reloaded = session.memory("reloaded")
        assert tail(reloaded, returned) == tail(rejoined, returned)
        assert quantity(reloaded, 44, 35) == quantity(rejoined, 44, 35)
        assert not any(text in session.log().lower() for text in
                       ("no function at", "left the stack wrong", "too many hooks"))
        result["rejoin_and_reload_passed"] = True
        result["passed"] = True
    except Exception as error:
        result["passed"] = False
        result["error"] = repr(error)
    finally:
        result["exit_code"] = session.close()
    print(json.dumps(result), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--game", required=True, type=Path)
    parser.add_argument("--save", required=True, type=Path)
    parser.add_argument("--out", default=Path("build/leaving-record"), type=Path)
    args = parser.parse_args()
    args.exe, args.game, args.save, args.out = (path.resolve() for path in
                                               (args.exe, args.game, args.save, args.out))
    args.out.mkdir(parents=True, exist_ok=True)
    fixture = bytearray(args.save.read_bytes())
    assert len(fixture) >= HEADER and not fixture[0x96] & 0x80, "use an out-of-battle save"
    count = read_word(fixture, 0xF1)
    assert len(fixture) >= HEADER + count * RECORD
    members = [i for i in range(4) if 0 <= read_word(fixture, 0xF3 + 2 * i) < count
               and fixture[HEADER + read_word(fixture, 0xF3 + 2 * i) * RECORD + 0x14] == 1]
    assert len(members) >= 2, "the source save needs at least two able members"
    member = members[-1]
    record_id = read_word(fixture, 0xF3 + 2 * member)
    struct.pack_into("<H", fixture, 0x82, 0xAA)
    if not 0 <= read_word(fixture, 0x7C) < 92:
        struct.pack_into("<h", fixture, 0x7C, 86)
    companion = next((p for p in args.save.parent.iterdir()
                      if p.name.lower() == args.save.with_suffix(".bsv").name.lower()), None)
    battle = companion.read_bytes() if companion else None
    with ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(lambda settings: scenario(args, fixture, battle, member, record_id, *settings),
                                ((0, 0), (1, 0), (0, 1), (1, 1))))
    (args.out / "results.json").write_text(json.dumps(results, indent=2))
    return 0 if all(result["passed"] for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
