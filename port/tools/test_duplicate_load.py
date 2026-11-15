#!/usr/bin/env python3
"""Check duplicate party IDs through headless native/original loads and re-saving.

python port/tools/test_duplicate_load.py --exe EXE --game GAME --save SAVE --out build/duplicate-load
Every scenario copies the licensed game and constructs its fixture from the supplied
out-of-battle save. SDL's dummy video driver keeps the native UI active without a window.
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

HEADER, RECORD = 0x189, 0x22A
DS, FAR = (0x0200 + 0x321A) * 16, (0x0200 + 0x2E38) * 16


def word(data, at):
    return struct.unpack_from("<h", data, at)[0]


def ids(data, at):
    return [word(data, at + 2 * slot) for slot in range(5)]


def record_from_memory(data, slot):
    return (data[DS + 0x9C55 + 128 * slot:DS + 0x9C55 + 128 * (slot + 1)]
            + data[FAR + 0x082C + 20 * slot:FAR + 0x082C + 20 * (slot + 1)]
            + data[FAR + 0x07BE + 22 * slot:FAR + 0x07BE + 22 * (slot + 1)]
            + data[FAR + 0x003E + 0x180 * slot:FAR + 0x003E + 0x180 * (slot + 1)])


class Session:
    def __init__(self, args, case, sav, bsv, original, fix):
        self.root = Path(tempfile.mkdtemp(prefix=case + "-", dir=args.out))
        self.game = self.root / "game"
        shutil.copytree(args.game, self.game, ignore=shutil.ignore_patterns(
            "SAVES", "Darklands+ save backups", "darklands_port*.log",
            "darklands*.txt", "darklands_levels*.bin"))
        (self.game / "SAVES").mkdir()
        shutil.copy(args.game / "SAVES" / "DEFAULT", self.game / "SAVES")
        self.put_save(0, sav, bsv)
        (self.game / "darklands_port.ini").write_text(
            "saves.restart_on_end = 0\ndisplay.opening = 0\ndisplay.opening_seen = 1\n"
            f"fix.duplicateload = {fix}\n" + ("display.native_screens = 0\n" if original else ""))
        self.control = self.root / "control.txt"
        self.control.touch()
        self.logfile = self.game / "darklands_port.log"
        env = dict(os.environ, DL_CTL=str(self.control), DL_SAVES_LOG="1",
                   SDL_AUDIODRIVER="dummy", SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
        env.pop("DL_SCRIPT", None)
        env.pop("DL_HEADLESS", None)
        self.stderr = (self.root / "stderr.txt").open("wb")
        self.process = subprocess.Popen([str(args.exe), str(self.game)], env=env,
            stdout=self.stderr, stderr=self.stderr,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)

    def put_save(self, number, sav, bsv):
        file = self.game / "SAVES" / f"dksave{number}.sav"
        file.write_bytes(sav)
        if bsv is not None:
            file.with_suffix(".bsv").write_bytes(bsv)
        return file

    def log(self):
        return self.logfile.read_text(encoding="latin1") if self.logfile.exists() else ""

    def command(self, command, delay=0.25):
        assert self.process.poll() is None, f"game exited {self.process.returncode}\n{self.log()[-1000:]}"
        with self.control.open("ab") as file:
            file.write((command + "\n").encode("utf-8"))
        time.sleep(delay)

    def wait(self, text, seconds=15, since=0):
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            if text in self.log()[since:]:
                return
            if self.process.poll() is not None:
                break
            time.sleep(0.1)
        raise AssertionError(f"did not reach {text!r}\n{self.log()[-1600:]}")

    def ready(self):
        start = len(self.log())
        for _ in range(18):
            self.command("umode:", 0.25)
            if "mode 1" in self.log()[start:] or "mode 2" in self.log()[start:]:
                return
        raise AssertionError("loaded game did not reach a card or map")

    def load_title(self, original):
        self.wait("opening: skipped", seconds=20)
        if original:
            time.sleep(1)
            self.command("key:T", 0.8)
            self.command("key:L", 0.7)
        else:
            self.wait("title: open")
            self.command("key:Return", 0.7)

    def memory(self, name):
        file = self.root / (name + ".bin")
        self.command("umemdump:" + file.as_posix())
        self.wait("memory dumped to " + file.as_posix())
        return file.read_bytes()

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


def scenario(args, initial, bsv, settings):
    case, original, fix, warm, repeats, repeated_leader = settings
    fixture = bytearray(initial)
    original_ids = ids(fixture, 0xF3)
    for slot in repeats:
        struct.pack_into("<h", fixture, 0xF3 + 2 * slot, original_ids[0])
    if repeated_leader:
        fixture[0xA1] = repeats[0]
    expected_ids = ids(fixture, 0xF3)
    for slot in repeats:
        expected_ids[slot] = -1
    session = Session(args, case, initial if warm else fixture, bsv, original, fix)
    result = {"case": case, "folder": str(session.root)}
    try:
        session.load_title(original)
        if not fix and repeats and not warm:
            session.process.wait(timeout=12)
            assert session.process.returncode == 99, "the missing-shield failure control did not reproduce"
            result.update(passed=True, exit_on_load=99)
            return result
        session.ready()
        source = session.game / "SAVES/dksave0.sav"
        if warm:
            source = session.put_save(1, fixture, bsv)
            before = session.memory("before-warm")
            assert record_from_memory(before, repeats[0])[0x7E] > 0, "the warm-load slot needs a real pack"
            session.command("uload:dksave1.sav", 0.7)
            session.wait("Loaded dksave1.sav")
            session.ready()
        memory = session.memory("loaded")
        roster = (session.game / "charactr.tmp").read_bytes()
        assert ids(roster, 2) == expected_ids, (ids(roster, 2), expected_ids)
        count = word(fixture, 0xF1)
        assert word(roster, 0) == count
        assert roster[32:] == fixture[HEADER:HEADER + count * RECORD], "stored character data changed during load"
        first = fixture[HEADER + original_ids[0] * RECORD:HEADER + (original_ids[0] + 1) * RECORD]
        assert record_from_memory(memory, 0) == first, "first legitimate character changed"
        for slot in repeats:
            assert record_from_memory(memory, slot) == bytes(RECORD), f"slot {slot} retained earlier character data"
        if repeated_leader:
            assert memory[DS + 0x907B] == 0
        assert source.read_bytes() == fixture, "the original save was rewritten"
        before_files = set((session.game / "SAVES").glob("*.sav"))
        session.command("uquicksave:", 0.7)
        session.wait("Game saved")
        new = set((session.game / "SAVES").glob("*.sav")) - before_files
        assert len(new) == 1, "expected one new save"
        saved = new.pop()
        data = saved.read_bytes()
        assert ids(data, 0xF3) == expected_ids
        assert word(data, 0xF1) == count
        start = len(session.log())
        session.command("uload:" + saved.name, 0.7)
        session.wait("Loaded " + saved.name, since=start)
        session.ready()
        reloaded = session.memory("reloaded")
        assert record_from_memory(reloaded, 0) == record_from_memory(memory, 0)
        for slot in repeats:
            assert reloaded[DS + 0x9C69 + slot * 128] == 0
        assert not any(message in session.log() for message in ("left the stack wrong", "no function at", "too many hooks"))
        result.update(passed=True, normalized_ids=expected_ids, source_unchanged=True,
                      first_record_preserved=True, repeated_slots_cleared=True, save_reload=True)
    except Exception as error:
        result.update(passed=False, error=repr(error))
    finally:
        result["exit_code"] = session.close()
        print(json.dumps(result), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--game", required=True, type=Path)
    parser.add_argument("--save", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    args.exe, args.game, args.save, args.out = (path.resolve() for path in
                                               (args.exe, args.game, args.save, args.out))
    args.out.mkdir(parents=True, exist_ok=True)
    sav = bytearray(args.save.read_bytes())
    assert len(sav) >= HEADER and not sav[0x96] & 0x80, "use an out-of-battle save"
    assert 0 <= word(sav, 0xF3) < word(sav, 0xF1) and 0 <= word(sav, 0xF5) < word(sav, 0xF1)
    struct.pack_into("<h", sav, 0x82, 0x1D)
    companion = next((file for file in args.save.parent.iterdir()
                      if file.name.lower() == args.save.with_suffix(".bsv").name.lower()), None)
    bsv = companion.read_bytes() if companion else None
    cases = [("native-disabled", False, 0, False, [1], False),
             ("original-disabled", True, 0, False, [1], False),
             ("native-fixed", False, 1, False, [1], False),
             ("original-fixed", True, 1, False, [1], False),
             ("warm-fixed", False, 1, True, [1], False),
             ("fifth-fixed", False, 1, False, [4], False),
             ("leader-fixed", False, 1, False, [1], True),
             ("all-slots-fixed", False, 1, False, [1, 2, 3, 4], False),
             ("ordinary-fixed", False, 1, False, [], False)]
    with ThreadPoolExecutor(max_workers=3) as pool:
        results = list(pool.map(lambda case: scenario(args, sav, bsv, case), cases))
    (args.out / "results.json").write_text(json.dumps(results, indent=2))
    return 0 if all(result["passed"] for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
