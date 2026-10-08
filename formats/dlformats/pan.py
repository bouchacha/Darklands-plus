"""``*.PAN`` -- intro / ending / death / credits animations (graphics).

New finding: every PAN file is a single LZ stream (:mod:`dlformats.lzss`,
the same compressor as IMC/IMAPS), sometimes followed by a few trailing bytes
after the end marker (FIN1A: 13 zero bytes, FIN2A: 15, FIN4A: 13) which are
kept so the files round-trip.  All 15 files re-compress byte-exactly.

The decompressed data (106 KB .. 700 KB) starts with ``u16 0x0A5A`` (magic,
all 15 files) and ``u16 n`` (55..358, probably a frame/chunk count); the rest
(frame encoding, palette) is NOT decoded yet.
"""
from __future__ import annotations

import dataclasses
import struct

from . import lzss
from .util import FormatError, read_file

MAGIC = 0x0A5A


@dataclasses.dataclass
class Pan:
    data: bytes = dataclasses.field(repr=False)   # decompressed payload
    trailer: bytes = b""                         # bytes after the LZ end marker

    @property
    def magic(self) -> int:
        return struct.unpack_from("<H", self.data, 0)[0]

    @property
    def count(self) -> int:
        return struct.unpack_from("<H", self.data, 2)[0]


def parse(raw: bytes) -> Pan:
    data, used = lzss.decompress(raw, return_consumed=True)
    if len(data) < 4 or struct.unpack_from("<H", data, 0)[0] != MAGIC:
        raise FormatError("PAN: bad magic")
    return Pan(data, raw[used:])


def read(path) -> Pan:
    return parse(read_file(path))


def build(p: Pan) -> bytes:
    return lzss.compress(p.data) + p.trailer
