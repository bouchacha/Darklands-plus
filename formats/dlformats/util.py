"""Shared helpers for the Darklands data-format readers/writers.

Everything here is game-agnostic plumbing:

* a tiny declarative struct layer (``Spec``) used by the record-based formats,
  so that the same field list drives parsing, writing and the docs;
* fixed-size string fields that remember the bytes after the NUL terminator
  (several Darklands files carry editor memory garbage there, and we must keep
  it to round-trip byte-exactly);
* the Darklands character set (a German ISO-646 variant);
* DOS packed date/time decoding (used by the CAT archive index);
* a JSON encoder that copes with ``bytes`` and dataclasses.

Conventions used by all modules:

* Strings are decoded as latin-1 so that decoding is lossless; use
  :func:`dl_to_unicode` to obtain human-readable text.
* Unknown fields are named ``unknown_XX`` (XX = hex offset inside the record)
  and are preserved verbatim.
* ``<field>__pad`` holds the raw bytes after the NUL of a fixed-size string
  when they are not all zero.
"""
from __future__ import annotations

import dataclasses
import datetime as _dt
import json
import struct
from typing import Any, Iterable

# ---------------------------------------------------------------------------
# Character set
# ---------------------------------------------------------------------------

#: Darklands text uses a German ISO-646-like charset.  The three lower-case
#: mappings are verified from text context (e.g. ``J\x1fger``, ``L|beck``,
#: ``L{we``) *and* from the glyph shapes in FONTS.FNT; the upper-case umlauts
#: and sharp s are inferred from FONTS.FNT glyph shapes only (they do not
#: occur in game text in a way that would confirm them).
DL_CHARSET = {
    0x1F: "ä",  # ä  (verified)
    0x7B: "ö",  # {  -> ö (verified)
    0x7C: "ü",  # |  -> ü (verified)
    0x5B: "Ä",  # [  -> Ä (glyph shape only)
    0x5C: "Ö",  # \\ -> Ö (glyph shape only)
    0x5D: "Ü",  # ]  -> Ü (glyph shape only)
    0x5F: "ß",  # _  -> ß (glyph shape only)
}
_DL_REVERSE = {v: k for k, v in DL_CHARSET.items()}


def dl_to_unicode(s: str | bytes, umlauts_only: bool = True) -> str:
    """Translate raw Darklands text to Unicode.

    ``umlauts_only=True`` only applies the verified lower-case mappings
    (0x1F, ``{``, ``|``) so that brackets/underscores in non-text data are
    left alone.
    """
    if isinstance(s, str):
        s = s.encode("latin-1")
    keys = (0x1F, 0x7B, 0x7C) if umlauts_only else tuple(DL_CHARSET)
    return "".join(DL_CHARSET[b] if b in keys else chr(b) for b in s)


def unicode_to_dl(s: str) -> str:
    """Inverse of :func:`dl_to_unicode` (returns a latin-1 'raw' string)."""
    return "".join(chr(_DL_REVERSE[c]) if c in _DL_REVERSE else c for c in s)


# ---------------------------------------------------------------------------
# Misc helpers
# ---------------------------------------------------------------------------

def dos_datetime(value: int) -> str | None:
    """Decode a DOS packed date/time dword (low word = time, high word = date).

    Returns an ISO string, or None if the value is not a valid DOS date.
    """
    t = value & 0xFFFF
    d = value >> 16
    try:
        return _dt.datetime((d >> 9) + 1980, (d >> 5) & 15, d & 31,
                            t >> 11, (t >> 5) & 63, (t & 31) * 2).isoformat()
    except ValueError:
        return None


def to_dos_datetime(iso: str) -> int:
    x = _dt.datetime.fromisoformat(iso)
    d = ((x.year - 1980) << 9) | (x.month << 5) | x.day
    t = (x.hour << 11) | (x.minute << 5) | (x.second // 2)
    return (d << 16) | t


def read_file(path) -> bytes:
    with open(path, "rb") as f:
        return f.read()


def write_file(path, data: bytes) -> None:
    with open(path, "wb") as f:
        f.write(data)


def split_cstrings(data: bytes, count: int, pos: int = 0) -> tuple[list[str], int]:
    """Read ``count`` consecutive NUL-terminated strings."""
    out = []
    for _ in range(count):
        z = data.index(b"\0", pos)
        out.append(data[pos:z].decode("latin-1"))
        pos = z + 1
    return out, pos


def join_cstrings(strings: Iterable[str]) -> bytes:
    return b"".join(s.encode("latin-1") + b"\0" for s in strings)


class FormatError(ValueError):
    """Raised when a file does not match the expected structure."""


# ---------------------------------------------------------------------------
# Declarative record layer
# ---------------------------------------------------------------------------

_PRIM = {
    "u8": "<B", "s8": "<b", "u16": "<H", "s16": "<h", "u32": "<I", "s32": "<i",
    "u16be": ">H",
}


@dataclasses.dataclass
class F:
    """One field of a fixed-size record.

    kind: one of the primitive names in ``_PRIM``, ``"str"`` (fixed-size,
    NUL-padded string, ``size`` bytes), ``"bytes"`` (``size`` raw bytes) or
    ``"spec"`` (nested :class:`Spec` in ``sub``).  ``count`` > 1 makes the
    field a list of that many elements (not allowed for str/bytes).
    ``doc`` is a one-line description used in docs; ``verified`` records how
    sure we are about the *meaning* (offsets/sizes are always verified by the
    round-trip tests).
    """
    name: str
    kind: str
    size: int = 0
    count: int = 1
    sub: "Spec | None" = None
    doc: str = ""
    verified: str = ""   # "", "structural", "cross-ref", "doc" (from community docs, unverified)

    def byte_size(self) -> int:
        if self.kind in _PRIM:
            n = struct.calcsize(_PRIM[self.kind])
        elif self.kind in ("str", "bytes"):
            n = self.size
        elif self.kind == "spec":
            n = self.sub.size
        else:
            raise ValueError(self.kind)
        return n * self.count


class Spec:
    """An ordered list of :class:`F` describing a fixed-size record."""

    def __init__(self, name: str, fields: list[F]):
        self.name = name
        self.fields = fields
        self.size = sum(f.byte_size() for f in fields)

    # -- parsing -----------------------------------------------------------
    def _one(self, f: F, data: bytes, pos: int, out: dict, key: str):
        if f.kind in _PRIM:
            fmt = _PRIM[f.kind]
            return struct.unpack_from(fmt, data, pos)[0], pos + struct.calcsize(fmt)
        if f.kind == "spec":
            return f.sub.parse(data, pos), pos + f.sub.size
        raise AssertionError

    def parse(self, data: bytes, pos: int = 0) -> dict:
        if pos + self.size > len(data):
            raise FormatError(f"{self.name}: need {self.size} bytes at {pos}, have {len(data) - pos}")
        out: dict[str, Any] = {}
        for f in self.fields:
            if f.kind == "str":
                raw = data[pos:pos + f.size]
                z = raw.find(b"\0")
                if z < 0:
                    out[f.name] = raw.decode("latin-1")
                else:
                    out[f.name] = raw[:z].decode("latin-1")
                    pad = raw[z:]
                    if pad.count(0) != len(pad):
                        out[f.name + "__pad"] = pad
                pos += f.size
            elif f.kind == "bytes":
                out[f.name] = data[pos:pos + f.size]
                pos += f.size
            elif f.count == 1:
                out[f.name], pos = self._one(f, data, pos, out, f.name)
            else:
                lst = []
                for _ in range(f.count):
                    v, pos = self._one(f, data, pos, out, f.name)
                    lst.append(v)
                out[f.name] = lst
        return out

    # -- writing -----------------------------------------------------------
    def build(self, rec: dict) -> bytes:
        parts = []
        for f in self.fields:
            v = rec[f.name]
            if f.kind == "str":
                b = v.encode("latin-1") if isinstance(v, str) else bytes(v)
                pad = rec.get(f.name + "__pad")
                if pad is not None:
                    pad = _as_bytes(pad)
                    b = b + pad
                if len(b) > f.size:
                    raise FormatError(f"{self.name}.{f.name}: string too long ({len(b)} > {f.size})")
                parts.append(b + b"\0" * (f.size - len(b)))
            elif f.kind == "bytes":
                b = _as_bytes(v)
                if len(b) != f.size:
                    raise FormatError(f"{self.name}.{f.name}: need {f.size} bytes, got {len(b)}")
                parts.append(b)
            else:
                vals = [v] if f.count == 1 else list(v)
                if len(vals) != f.count:
                    raise FormatError(f"{self.name}.{f.name}: need {f.count} values")
                for x in vals:
                    if f.kind == "spec":
                        parts.append(f.sub.build(x))
                    else:
                        parts.append(struct.pack(_PRIM[f.kind], x))
        out = b"".join(parts)
        assert len(out) == self.size, (self.name, len(out), self.size)
        return out

    # -- docs --------------------------------------------------------------
    def table(self, base: int = 0) -> list[tuple[str, str, str, str, str]]:
        """Rows (offset, type, name, meaning, confidence) for documentation."""
        rows = []
        off = base
        for f in self.fields:
            if f.kind == "str":
                t = f"char[{f.size}]"
            elif f.kind == "bytes":
                t = f"u8[{f.size}]"
            elif f.kind == "spec":
                t = f.sub.name + (f"[{f.count}]" if f.count > 1 else "")
            else:
                t = f.kind + (f"[{f.count}]" if f.count > 1 else "")
            rows.append((f"0x{off:02X}", t, f.name, f.doc, f.verified))
            off += f.byte_size()
        return rows

    def markdown(self, base: int = 0) -> str:
        lines = [f"Record `{self.name}`, size 0x{self.size:X} ({self.size}) bytes:", "",
                 "| off | type | name | meaning | confidence |", "|---|---|---|---|---|"]
        for r in self.table(base):
            lines.append("| " + " | ".join(x.replace("|", "\\|") for x in r) + " |")
        return "\n".join(lines)


def _as_bytes(v) -> bytes:
    if isinstance(v, (bytes, bytearray)):
        return bytes(v)
    if isinstance(v, str):          # hex string coming back from JSON
        return bytes.fromhex(v)
    if isinstance(v, list):
        return bytes(v)
    raise TypeError(type(v))


# ---------------------------------------------------------------------------
# JSON
# ---------------------------------------------------------------------------

class DLJSONEncoder(json.JSONEncoder):
    def default(self, o):
        if isinstance(o, (bytes, bytearray)):
            return o.hex()
        if dataclasses.is_dataclass(o):
            return {f.name: getattr(o, f.name) for f in dataclasses.fields(o)
                    if not f.name.startswith("_")}
        return super().default(o)


def to_json(obj, **kw) -> str:
    kw.setdefault("indent", 1)
    kw.setdefault("ensure_ascii", False)
    return json.dumps(obj, cls=DLJSONEncoder, **kw)


# Standard VGA BIOS default 256-colour palette (mode 13h), returned as 8-bit
# RGB.  This is a hardware default, not game data; it is used only as a
# fallback when a PIC file does not define a colour.
_VGA_STEPS = [(0, 16, 31, 47, 63), (31, 39, 47, 55, 63), (45, 49, 54, 58, 63),
              (0, 7, 14, 21, 28), (14, 17, 21, 24, 28), (20, 22, 24, 26, 28),
              (0, 4, 8, 12, 16), (8, 10, 12, 14, 16), (11, 12, 13, 15, 16)]


def vga_default_palette() -> list[tuple[int, int, int]]:
    pal = [(0, 0, 0), (0, 0, 42), (0, 42, 0), (0, 42, 42), (42, 0, 0), (42, 0, 42), (42, 21, 0), (42, 42, 42),
           (21, 21, 21), (21, 21, 63), (21, 63, 21), (21, 63, 63), (63, 21, 21), (63, 21, 63), (63, 63, 21),
           (63, 63, 63)]
    pal += [(g, g, g) for g in (0, 5, 8, 11, 14, 17, 20, 24, 28, 32, 36, 40, 45, 50, 56, 63)]
    for s0, s1, s2, s3, s4 in _VGA_STEPS:
        pal += [(s0, s0, s4), (s1, s0, s4), (s2, s0, s4), (s3, s0, s4),     # blue -> magenta
                (s4, s0, s4), (s4, s0, s3), (s4, s0, s2), (s4, s0, s1),     # magenta -> red
                (s4, s0, s0), (s4, s1, s0), (s4, s2, s0), (s4, s3, s0),     # red -> yellow
                (s4, s4, s0), (s3, s4, s0), (s2, s4, s0), (s1, s4, s0),     # yellow -> green
                (s0, s4, s0), (s0, s4, s1), (s0, s4, s2), (s0, s4, s3),     # green -> cyan
                (s0, s4, s4), (s0, s3, s4), (s0, s2, s4), (s0, s1, s4)]     # cyan -> blue
    pal += [(0, 0, 0)] * 8
    assert len(pal) == 256
    return [(r * 255 // 63, g * 255 // 63, b * 255 // 63) for r, g, b in pal]
