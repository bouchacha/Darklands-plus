"""CAT archives (``*.CAT``, ``MSGFILES``, ``BC``, ``LCASTLE``).

Layout (all little-endian)::

    u16 count
    count x { char name[12]; u32 dos_datetime; u32 length; u32 offset }
    file data ...

* ``name`` is an 8.3 name padded with NULs (no garbage observed).
* ``dos_datetime`` is a DOS packed date/time (low word = time, high word =
  date).  Earlier community docs treated it as "seconds since an epoch";
  decoding it as DOS date/time gives valid dates for all 1050 entries
  (1991-01-24 .. 1993-02-25), so that is what it is.
* In every shipped archive the data blocks are stored contiguously, in index
  order, starting right after the index.  :func:`build` reproduces that.

Entries inside archives are often compressed (see :mod:`dlformats.lzss`); this
module does not decompress anything.
"""
from __future__ import annotations

import dataclasses
import os
import struct

from .util import FormatError, dos_datetime, read_file

#: archive files shipped with the game (top-level directory)
ARCHIVES = ["A00C.CAT", "C00C.CAT", "E00C.CAT", "F01C.CAT", "F60C.CAT", "M00C.CAT",
            "EINFO.CAT", "IMAPS.CAT", "MSGFILES", "BC", "LCASTLE"]

ENTRY = struct.Struct("<12sIII")


@dataclasses.dataclass
class CatEntry:
    name: str             # decoded name (up to first NUL)
    timestamp: int        # raw DOS date/time dword
    data: bytes = dataclasses.field(repr=False)
    raw_name: bytes = b""  # the 12 raw name bytes (kept for exact rebuild)
    offset: int = 0       # offset in the source archive (informational)

    @property
    def date(self) -> str | None:
        return dos_datetime(self.timestamp)


@dataclasses.dataclass
class Catalog:
    entries: list[CatEntry]

    def names(self) -> list[str]:
        return [e.name for e in self.entries]

    def get(self, name: str) -> bytes:
        for e in self.entries:
            if e.name.upper() == name.upper():
                return e.data
        raise KeyError(name)

    def __iter__(self):
        return iter(self.entries)


def parse(data: bytes) -> Catalog:
    if len(data) < 2:
        raise FormatError("CAT: too short")
    n, = struct.unpack_from("<H", data, 0)
    if 2 + n * ENTRY.size > len(data):
        raise FormatError("CAT: index exceeds file")
    entries = []
    for i in range(n):
        raw, ts, ln, off = ENTRY.unpack_from(data, 2 + i * ENTRY.size)
        if off + ln > len(data):
            raise FormatError(f"CAT: entry {i} outside file")
        name = raw.split(b"\0", 1)[0].decode("ascii")
        entries.append(CatEntry(name, ts, data[off:off + ln], raw, off))
    return Catalog(entries)


def read(path) -> Catalog:
    return parse(read_file(path))


def build(cat: Catalog) -> bytes:
    """Serialise: index followed by contiguous data in index order."""
    n = len(cat.entries)
    pos = 2 + n * ENTRY.size
    index = [struct.pack("<H", n)]
    blobs = []
    for e in cat.entries:
        raw = e.raw_name or e.name.encode("ascii")
        if len(raw) > 12:
            raise FormatError(f"CAT: name too long: {e.name}")
        raw = raw + b"\0" * (12 - len(raw))
        index.append(ENTRY.pack(raw, e.timestamp, len(e.data), pos))
        blobs.append(e.data)
        pos += len(e.data)
    return b"".join(index + blobs)


def extract(cat: Catalog, outdir) -> list[str]:
    os.makedirs(outdir, exist_ok=True)
    out = []
    for e in cat.entries:
        p = os.path.join(outdir, e.name)
        with open(p, "wb") as f:
            f.write(e.data)
        out.append(p)
    return out


def from_directory(srcdir, order: list[str] | None = None, timestamp: int = 0) -> Catalog:
    """Create a catalog from loose files (e.g. for mods).

    ``order`` gives the entry order (defaults to sorted file names).  The
    game looks entries up by name, but keeping the original order is the
    safe choice when replacing files in an existing archive -- prefer
    :func:`replace` for that.
    """
    names = order or sorted(os.listdir(srcdir))
    return Catalog([CatEntry(n, timestamp, read_file(os.path.join(srcdir, n))) for n in names])


def replace(cat: Catalog, name: str, data: bytes) -> None:
    for e in cat.entries:
        if e.name.upper() == name.upper():
            e.data = data
            return
    raise KeyError(name)
