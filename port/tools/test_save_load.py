#!/usr/bin/env python3
"""Test load refusal and recovery using a copied licensed game and a copied save.

python port/tools/test_save_load.py --exe EXE --game DIR --save dksave0.sav --out build/load-check
Use --expect-unfixed with the old executable for the empty-BSV failure control.
SDL's dummy driver runs the native screens without opening a desktop window.
"""
import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import time


class Session:
    def __init__(self, args, case, sav, bsv, settings=""):
        self.root = args.out / case
        self.root.mkdir(parents=True)
        self.game = self.root / "game"
        shutil.copytree(args.game, self.game, ignore=shutil.ignore_patterns(
            "SAVES", "Darklands+ save backups", "darklands_port*.log", "darklands*.txt"))
        saves = self.game / "SAVES"
        saves.mkdir()
        for file in (args.game / "SAVES").iterdir():
            if file.name.upper() == "DEFAULT":
                shutil.copy(file, saves / file.name)
        self.put_save(0, sav, bsv)
        (self.game / "darklands_port.ini").write_text(
            "saves.restart_on_end = 0\ndisplay.opening = 0\ndisplay.opening_seen = 1\n"
            "saves.autosave_arrive = 0\nsaves.autosave_battle = 0\n" + settings)
        self.control = self.root / "control.txt"
        self.control.touch()
        self.logfile = self.game / "darklands_port.log"
        env = dict(os.environ, DL_CTL=str(self.control), SDL_AUDIODRIVER="dummy",
                   SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
        env.pop("DL_HEADLESS", None)
        env.pop("DL_SCRIPT", None)
        flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0
        self.process = subprocess.Popen([str(args.exe), str(self.game)], env=env,
                                        creationflags=flags, stdout=subprocess.DEVNULL,
                                        stderr=subprocess.DEVNULL)

    def put_save(self, number, sav, bsv):
        file = self.game / "SAVES" / f"dksave{number}.sav"
        file.write_bytes(sav)
        if bsv is not None:
            file.with_suffix(".bsv").write_bytes(bsv)

    def log(self):
        return self.logfile.read_text(encoding="latin1") if self.logfile.exists() else ""

    def command(self, command, delay=0.25):
        assert self.process.poll() is None, self.log()[-1800:]
        with self.control.open("ab") as file:
            file.write((command + "\n").encode("ascii"))
        time.sleep(delay)

    def wait(self, text, start=0, seconds=12):
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            if text in self.log()[start:]:
                return
            if self.process.poll() is not None:
                break
            time.sleep(0.1)
        raise AssertionError(f"Expected {text!r}\n{self.log()[-2400:]}")

    def start(self):
        self.wait("title: open")
        self.command("key:Return")
        self.wait("title: closed (the game begins)")
        time.sleep(0.5)

    def dump(self, name):
        file = self.root / (name + ".bin")
        self.command("udsdump:" + file.as_posix())
        end = time.monotonic() + 5
        while not file.exists() and time.monotonic() < end:
            time.sleep(0.1)
        return file.read_bytes()

    def close(self):
        if self.process.poll() is None:
            self.command("quit:")
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.terminate()
                self.process.wait(timeout=3)


def changed(sav, kind):
    data = bytearray(sav)
    if kind == "short-sav":
        return data[:512]
    if kind == "month":
        struct.pack_into("<h", data, 0x6a, 12)
    if kind == "quick":
        data[0x15:0x15 + 23] = b"Quicksave".ljust(23, b"\0")
    return data


def running_case(args, sav, bsv, kind):
    settings = "fix.savecheck = 0\n" if args.disable_fix else ""
    session = Session(args, kind, sav, bsv, settings)
    try:
        session.start()
        before = session.dump("before")
        bad_bsv = b"" if kind in ("empty-bsv", "menu", "quick") else bsv
        if kind == "corrupt-bsv":
            bad_bsv = bytes(len(bsv))
        if kind == "missing-bsv":
            bad_bsv = None
        session.put_save(1, changed(sav, kind), bad_bsv)
        at = len(session.log())
        if kind == "menu":
            session.command("down:Left Alt")
            session.command("key:L")
            session.command("up:Left Alt")
            session.wait("saves: the Load page", start=at)
            session.command("key:Up") # newest file, above the current save
            session.command("key:Return")
        elif kind == "quick":
            session.command("usdlkey:F9")
        else:
            session.command("uload:dksave1.sav")
        if args.expect_unfixed:
            end = time.monotonic() + 8
            while session.process.poll() is None and time.monotonic() < end:
                time.sleep(0.1)
            assert session.process.poll() is not None, "The old empty-BSV control did not exit"
        elif kind in ("valid", "missing-bsv"):
            session.wait("Loaded dksave1.sav", start=at)
        else:
            session.wait("Cannot load", start=at)
            after = session.dump("after")
            assert before[0x9c69:0x9c69 + 5 * 0x80] == after[0x9c69:0x9c69 + 5 * 0x80], "Party changed on refusal"
            assert before[0x907e:0x9080] == after[0x907e:0x9080], "Location changed on refusal"
            session.command("uquicksave:")
            session.wait("Game saved", start=at)
        print(kind, "PASS", flush=True)
    finally:
        session.close()


def title_case(args, sav, kind):
    original = kind == "original-title"
    settings = "display.native_screens = 0\n" if original else ""
    session = Session(args, kind, sav, b"", settings)
    try:
        if original:
            session.wait("opening: skipped")
            time.sleep(1)
            session.command("key:T", 0.8)
            session.command("key:L")
        else:
            session.wait("title: open")
            session.command("key:Return")
        session.wait("Cannot load")
        assert "the game begins" not in session.log()
        assert session.process.poll() is None
        print(kind, "PASS", flush=True)
    finally:
        session.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--game", type=Path, required=True)
    parser.add_argument("--save", default="dksave0.sav")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--expect-unfixed", action="store_true")
    parser.add_argument("--disable-fix", action="store_true", help="exercise the setting with --expect-unfixed")
    args = parser.parse_args()
    args.exe, args.game, args.out = args.exe.resolve(), args.game.resolve(), args.out.resolve()
    saves = list((args.game / "SAVES").iterdir())
    file = next(file for file in saves if file.name.lower() == args.save.lower())
    companion = next(file for file in saves if file.name.lower() == Path(args.save).with_suffix(".bsv").name.lower())
    sav, bsv = bytearray(file.read_bytes()), companion.read_bytes()
    # Begin at the normal daytime inn menu, to keep idle comparisons free of the opening card's time reset.
    struct.pack_into("<h", sav, 0x82, 0x1d)
    cases = ["empty-bsv"] if args.expect_unfixed else ["valid", "short-sav", "empty-bsv", "corrupt-bsv", "month", "missing-bsv", "menu", "quick"]
    for kind in cases:
        running_case(args, sav, bsv, kind)
    if not args.expect_unfixed:
        title_case(args, sav, "native-title")
        title_case(args, sav, "original-title")


if __name__ == "__main__":
    main()
