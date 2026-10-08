"""Battle sprites: ``*.IMC`` (inside A00C/C00C/E00C/F01C/F60C/M00C.CAT) and
the uncompressed sprite banks ``BATTLEGR.IMG`` / ``COMMONSP.IMG``.

IMC
---
Each IMC catalog entry is an LZ stream (:mod:`dlformats.lzss`).  File names:
``<group><kind><weapon>.IMC`` with group = ENM image group (A00, E03, M21...),
kind = ``WK`` (walk), ``CB`` (combat) or ``DY`` (dying, no weapon).

Decompressed layout (verified on all 333 IMC files: ``data_length`` always
equals the bytes after the offset table, every frame decodes inside its
16-byte-aligned slot, every frame height equals ``height``)::

    0x00  u16 unknown_00[12]    small values (e.g. 5,2,1,1,0,1,1,1,1,4,0x17,0x10)
    0x18  u16 motion_flag        0xFFFF in DY files; otherwise 0..4 followed by
          u8  motion[20]         (non-DY only) signed per-direction offsets?
    +0    u16 height
    +2    u16 widths[8]          per-direction width (does not always equal the
                                 frame widths -- meaning unclear)
    +18   u16 unknown[8]
    +34   u16 frame_count        frames per direction
    +36   u16 data_length        bytes after the offset table
    +38   u16 frame_offset[8 * frame_count]   in 16-byte paragraphs, relative
                                 to the end of this table; not sorted, may repeat
    data: frames, each 16-byte aligned:
          u8 width, u8 height,
          height x { u8 count; u8 skip; u8 pixel[count] }   (0 = transparent)

The header block starts at 0x1A for DY files and 0x2E otherwise.  Frames of
direction ``j`` (index ``j*frame_count ...``) correspond to header width
``widths[(8-j) % 8]`` in many but not all files -- unknown.  The bytes between
frames are not always zero (stale memory), and frames are not stored in index
order, so exact re-creation keeps the raw data region.  Pixel values index the
battle palette, with ENEMYPAL.DAT recolouring a 16-colour range.

IMG sprite banks
----------------
::

    u32 data_length
    u16 frame_offset[n]       n = (filesize - data_length - 4) / 2;
                              16-byte paragraphs relative to the data start
    data (same frame encoding as IMC)

BATTLEGR.IMG: 200 frames; COMMONSP.IMG: 13 frames.  The bytes in front of the
first frame offset (208 bytes in BATTLEGR) are unknown.
"""
from __future__ import annotations

import dataclasses
import struct

from . import lzss
from .util import FormatError, read_file


@dataclasses.dataclass
class Frame:
    width: int
    height: int
    rows: list[tuple[int, bytes]]    # (skip, pixels) per row

    def to_pixels(self) -> bytes:
        out = bytearray(self.width * self.height)
        for y, (skip, px) in enumerate(self.rows):
            out[y * self.width + skip:y * self.width + skip + len(px)] = px
        return bytes(out)


def decode_frame(data: bytes, pos: int) -> tuple[Frame, int]:
    w, h = data[pos], data[pos + 1]
    pos += 2
    rows = []
    for _ in range(h):
        cnt, skip = data[pos], data[pos + 1]
        if skip + cnt > w:
            raise FormatError("IMC: row wider than frame")
        rows.append((skip, data[pos + 2:pos + 2 + cnt]))
        pos += 2 + cnt
    return Frame(w, h, rows), pos


def encode_frame(f: Frame) -> bytes:
    out = bytearray((f.width, f.height))
    for skip, px in f.rows:
        out += bytes((len(px), skip)) + px
    return bytes(out)


def frame_from_pixels(width: int, height: int, pixels: bytes) -> Frame:
    rows = []
    for y in range(height):
        r = pixels[y * width:(y + 1) * width]
        nz = [i for i, v in enumerate(r) if v]
        if not nz:
            rows.append((0, b""))
        else:
            rows.append((nz[0], bytes(r[nz[0]:nz[-1] + 1])))
    return Frame(width, height, rows)


@dataclasses.dataclass
class Imc:
    unknown_00: bytes            # 24 bytes
    motion_flag: int
    motion: bytes                # 20 bytes or b"" (DY)
    height: int
    widths: list[int]
    unknown_widths2: list[int]
    frame_count: int
    frame_offsets: list[int]
    data: bytes = dataclasses.field(repr=False)   # raw frame area (exact)

    @property
    def is_dying(self) -> bool:
        return self.motion_flag == 0xFFFF

    def frames(self) -> list[Frame]:
        return [decode_frame(self.data, o * 16)[0] for o in self.frame_offsets]


def parse_raw(o: bytes) -> Imc:
    if len(o) < 0x1A:
        raise FormatError("IMC: too short")
    flag, = struct.unpack_from("<H", o, 0x18)
    if flag == 0xFFFF:
        motion = b""
        base = 0x1A
    else:
        motion = o[0x1A:0x2E]
        base = 0x2E
    h, = struct.unpack_from("<H", o, base)
    widths = list(struct.unpack_from("<8H", o, base + 2))
    w2 = list(struct.unpack_from("<8H", o, base + 18))
    fc, dl = struct.unpack_from("<HH", o, base + 34)
    offs = list(struct.unpack_from(f"<{8 * fc}H", o, base + 38))
    ds = base + 38 + 16 * fc
    if len(o) - ds != dl:
        raise FormatError(f"IMC: data_length {dl} != {len(o) - ds}")
    return Imc(o[:0x18], flag, motion, h, widths, w2, fc, offs, o[ds:])


def build_raw(m: Imc) -> bytes:
    out = bytearray(m.unknown_00) + struct.pack("<H", m.motion_flag) + m.motion
    out += struct.pack("<H8H8HHH", m.height, *m.widths, *m.unknown_widths2, m.frame_count, len(m.data))
    out += struct.pack(f"<{len(m.frame_offsets)}H", *m.frame_offsets)
    return bytes(out + m.data)


def parse(compressed: bytes) -> Imc:
    return parse_raw(lzss.decompress(compressed))


def build(m: Imc) -> bytes:
    return lzss.compress(build_raw(m))


def read(path) -> Imc:
    return parse(read_file(path))


def with_frames(m: Imc, frames: list[Frame]) -> Imc:
    """Return a copy of ``m`` whose frame area is regenerated from ``frames``
    (index order, zero padding).  Valid for the game's reader as far as the
    format is understood, but not byte-identical to the original layout."""
    if len(frames) != 8 * m.frame_count:
        raise ValueError("need 8 * frame_count frames")
    data = bytearray()
    offs = []
    for f in frames:
        offs.append(len(data) // 16)
        enc = encode_frame(f)
        data += enc + b"\0" * (-len(enc) % 16)
    if len(data) // 16 > 0xFFFF:
        raise FormatError("IMC: too much frame data")
    return dataclasses.replace(m, frame_offsets=offs, data=bytes(data))


# ---------------------------------------------------------------------------
# IMG sprite banks
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class ImgBank:
    frame_offsets: list[int]
    data: bytes = dataclasses.field(repr=False)

    def frames(self) -> list[Frame]:
        return [decode_frame(self.data, o * 16)[0] for o in self.frame_offsets]


def parse_img(data: bytes) -> ImgBank:
    dl, = struct.unpack_from("<I", data, 0)
    hdr = len(data) - dl
    if hdr < 4 or (hdr - 4) % 2:
        raise FormatError("IMG: bad header size")
    n = (hdr - 4) // 2
    return ImgBank(list(struct.unpack_from(f"<{n}H", data, 4)), data[hdr:])


def build_img(b: ImgBank) -> bytes:
    return struct.pack(f"<I{len(b.frame_offsets)}H", len(b.data), *b.frame_offsets) + b.data


def frames_to_png(frames: list[Frame], path, palette=None, columns: int = 8) -> None:
    """Contact sheet of frames (index 0 transparent)."""
    from PIL import Image
    from .util import vga_default_palette
    if not frames:
        return
    cw = max(f.width for f in frames) + 1
    ch = max(f.height for f in frames) + 1
    rows = (len(frames) + columns - 1) // columns
    sheet = Image.new("P", (cw * columns, ch * rows), 0)
    pal = palette or vga_default_palette()
    sheet.putpalette([v for c in pal for v in c])
    for i, f in enumerate(frames):
        if f.width and f.height:
            im = Image.frombytes("P", (f.width, f.height), f.to_pixels())
            sheet.paste(im, ((i % columns) * cw, (i // columns) * ch))
    sheet.save(path, transparency=0)
