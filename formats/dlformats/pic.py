"""PIC images (``PICS/*.PIC``, ``EINFO.CAT`` entries).

A PIC file is a sequence of chunks ``{u16 tag; u16 length; u8 data[length]}``:

* ``M0`` (tag 0x304D) palette: ``u8 first, u8 last, (last-first+1) x RGB``
  (6-bit VGA DAC values).  The chunk length is padded to an even number of
  bytes (12 files have one trailing pad byte, e.g. 17..255 -> 720 bytes).
* ``X0`` (tag 0x3058) image: ``u16 width, u16 height, u8 max_bits`` followed
  by the compressed pixel stream.  ``max_bits`` is 11 in every shipped file.
* ``X1`` (0x3058+1) exists in the display code according to Quadko but is not
  used by any shipped file; it is not supported here.

Pixel stream = LZW( RLE( pixels ) ), the same scheme as Civilization-era
MicroProse PICs:

* LZW: codes are packed LSB-first; the width starts at 9 bits and grows by one
  after 256, 512, 1024 ... codes have been *read* (purely count based); when
  ``9 + k > max_bits`` the width returns to 9 and the dictionary is reset.
  The dictionary holds 256 single bytes, entry 256 is never used, new entries
  start at 257 and the dictionary is reset after it reaches ``1 << max_bits``
  entries (1792 codes per cycle for max_bits = 11).  The final code is padded
  with zero bits to a byte boundary.
* RLE: ``0x90 n`` repeats the previous byte so that it appears ``n`` times in
  total; ``0x90 0x00`` is a literal 0x90.  The encoder emits runs of 4..255
  over the whole image (runs cross scanlines) and never run-encodes 0x90.

Both LZW and RLE encoders here reproduce all 607 shipped images byte-exactly.

Two files in PICS/ are not images: ``PICSHOW.EXE``/``PICSHOW.OVL`` (a DOS
viewer).  ``CHARPALT.PIC`` has a 0x0 image (one stray pixel in the stream)
and is effectively palette-only.
"""
from __future__ import annotations

import dataclasses
import struct

from .util import FormatError, read_file, vga_default_palette

TAG_PAL = 0x304D   # 'M0'
TAG_IMG = 0x3058   # 'X0'


# --------------------------------------------------------------------------
# LZW + RLE
# --------------------------------------------------------------------------

def lzw_unpack_codes(data: bytes, max_bits: int) -> list[int]:
    codes = []
    bitbuf = nb = 0
    width, cnt, thr = 9, 0, 256
    i, n = 0, len(data)
    while True:
        while nb < width and i < n:
            bitbuf |= data[i] << nb
            nb += 8
            i += 1
        if nb < width:
            break
        codes.append(bitbuf & ((1 << width) - 1))
        bitbuf >>= width
        nb -= width
        cnt += 1
        if cnt == thr:
            cnt = 0
            width += 1
            thr <<= 1
            if width > max_bits:
                width, thr = 9, 256
    return codes


def lzw_pack_codes(codes: list[int], max_bits: int) -> bytes:
    out = bytearray()
    bitbuf = nb = 0
    width, cnt, thr = 9, 0, 256
    for c in codes:
        bitbuf |= c << nb
        nb += width
        while nb >= 8:
            out.append(bitbuf & 0xFF)
            bitbuf >>= 8
            nb -= 8
        cnt += 1
        if cnt == thr:
            cnt = 0
            width += 1
            thr <<= 1
            if width > max_bits:
                width, thr = 9, 256
    if nb:
        out.append(bitbuf & 0xFF)
    return bytes(out)


def lzw_decode(codes: list[int], max_bits: int) -> bytes:
    out = bytearray()
    i, n = 0, len(codes)
    size = 1 << max_bits
    while i < n:
        table = [bytes((k,)) for k in range(256)] + [b""]
        c = codes[i]
        if c > 255:
            raise FormatError("LZW: first code of a cycle must be a literal")
        w = table[c]
        out += w
        i += 1
        while len(table) < size and i < n:
            k = codes[i]
            i += 1
            if k < len(table):
                e = table[k]
            elif k == len(table):
                e = w + w[:1]
            else:
                raise FormatError(f"LZW: bad code {k}")
            out += e
            table.append(w + e[:1])
            w = e
    return bytes(out)


def lzw_encode(data: bytes, max_bits: int) -> list[int]:
    size = 1 << max_bits
    codes = []
    i, n = 0, len(data)
    while i < n:
        d = {bytes((k,)): k for k in range(256)}
        nxt = 257
        w = data[i:i + 1]
        i += 1
        full = False
        while i < n:
            wc = w + data[i:i + 1]
            if wc in d:
                w = wc
                i += 1
                continue
            codes.append(d[w])
            if nxt >= size:
                full = True
                break
            d[wc] = nxt
            nxt += 1
            w = data[i:i + 1]
            i += 1
        if not full:
            codes.append(d[w])
    return codes


def rle_decode(d: bytes) -> bytes:
    out = bytearray()
    i, n = 0, len(d)
    while i < n:
        b = d[i]
        if b == 0x90:
            if i + 1 >= n:
                raise FormatError("RLE: dangling 0x90")
            c = d[i + 1]
            if c == 0:
                out.append(0x90)
            else:
                if not out:
                    raise FormatError("RLE: run without previous byte")
                out += bytes((out[-1],)) * (c - 1)
            i += 2
        else:
            out.append(b)
            i += 1
    return bytes(out)


def rle_encode(px: bytes, minrun: int = 4, maxrun: int = 255) -> bytes:
    out = bytearray()
    i, n = 0, len(px)
    while i < n:
        v = px[i]
        j = i + 1
        while j < n and px[j] == v and j - i < maxrun:
            j += 1
        L = j - i
        if L >= minrun and v != 0x90:
            out += bytes((v, 0x90, L))
        elif v == 0x90:
            out += b"\x90\x00" * L
        else:
            out += bytes((v,)) * L
        i = j
    return bytes(out)


# --------------------------------------------------------------------------
# File model
# --------------------------------------------------------------------------

@dataclasses.dataclass
class Palette:
    first: int
    colors: list[tuple[int, int, int]]   # 6-bit VGA values (0..63)
    pad: bytes = b""                     # trailing chunk padding

    @property
    def last(self) -> int:
        return self.first + len(self.colors) - 1


@dataclasses.dataclass
class Image:
    width: int
    height: int
    max_bits: int
    pixels: bytes = dataclasses.field(repr=False)   # width*height indices (may be longer, see CHARPALT)


@dataclasses.dataclass
class Pic:
    chunks: list   # list of Palette | Image | (tag, bytes) for unknown chunks

    @property
    def palette(self) -> Palette | None:
        return next((c for c in self.chunks if isinstance(c, Palette)), None)

    @property
    def image(self) -> Image | None:
        return next((c for c in self.chunks if isinstance(c, Image)), None)


def parse(data: bytes) -> Pic:
    if data[:2] == b"MZ":
        raise FormatError("PIC: this is a DOS executable, not a PIC")
    chunks = []
    pos = 0
    while pos < len(data):
        if pos + 4 > len(data):
            raise FormatError("PIC: truncated chunk header")
        tag, ln = struct.unpack_from("<HH", data, pos)
        body = data[pos + 4:pos + 4 + ln]
        if len(body) != ln:
            raise FormatError("PIC: truncated chunk")
        pos += 4 + ln
        if tag == TAG_PAL:
            first, last = body[0], body[1]
            cnt = last - first + 1
            if 2 + 3 * cnt > ln:
                raise FormatError("PIC: palette chunk too short")
            cols = [tuple(body[2 + 3 * k:5 + 3 * k]) for k in range(cnt)]
            chunks.append(Palette(first, cols, body[2 + 3 * cnt:]))
        elif tag == TAG_IMG:
            w, h, mb = struct.unpack_from("<HHB", body, 0)
            px = rle_decode(lzw_decode(lzw_unpack_codes(body[5:], mb), mb))
            if len(px) < w * h:
                raise FormatError(f"PIC: image data short ({len(px)} < {w * h})")
            chunks.append(Image(w, h, mb, px))
        else:
            chunks.append((tag, body))
    return Pic(chunks)


def read(path) -> Pic:
    return parse(read_file(path))


def build(pic: Pic) -> bytes:
    out = []
    for c in pic.chunks:
        if isinstance(c, Palette):
            body = bytes((c.first, c.last)) + b"".join(bytes(col) for col in c.colors) + c.pad
            tag = TAG_PAL
        elif isinstance(c, Image):
            codes = lzw_encode(rle_encode(c.pixels), c.max_bits)
            body = struct.pack("<HHB", c.width, c.height, c.max_bits) + lzw_pack_codes(codes, c.max_bits)
            tag = TAG_IMG
        else:
            tag, body = c
        if len(body) > 0xFFFF:
            raise FormatError("PIC: chunk too large")
        out.append(struct.pack("<HH", tag, len(body)) + body)
    return b"".join(out)


def make(pixels: bytes, width: int, height: int, palette: Palette | None = None) -> Pic:
    """Create a new PIC (palette chunk first, as in the shipped files).

    The game normally uses palettes 16..255 (``first=16``); entries 0..15 are
    the fixed UI colours.
    """
    if len(pixels) != width * height:
        raise ValueError("pixel count mismatch")
    chunks = []
    if palette is not None:
        if (2 + 3 * len(palette.colors) + len(palette.pad)) % 2:
            palette = dataclasses.replace(palette, pad=palette.pad + b"\0")
        chunks.append(palette)
    chunks.append(Image(width, height, 11, bytes(pixels)))
    return Pic(chunks)


# --------------------------------------------------------------------------
# PNG conversion (requires Pillow)
# --------------------------------------------------------------------------

def rgb_palette(pal: Palette | None, base=None) -> list[tuple[int, int, int]]:
    """256 8-bit RGB colours: ``base`` (default: VGA BIOS palette) overlaid
    with the colours defined in ``pal``."""
    out = list(base or vga_default_palette())
    if pal is not None:
        for i, (r, g, b) in enumerate(pal.colors):
            if pal.first + i < 256:
                out[pal.first + i] = ((r & 63) * 255 // 63, (g & 63) * 255 // 63, (b & 63) * 255 // 63)
    return out


def to_png(pic: Pic, path, extra_palette: Palette | None = None, transparent0: bool = False) -> bool:
    """Write the image as an indexed PNG.  Returns False if there is no image.

    Colours not defined by the file (or ``extra_palette``) come from the VGA
    default palette and may not match what the game shows.
    """
    from PIL import Image as PILImage
    img = pic.image
    if img is None or img.width == 0 or img.height == 0:
        return False
    cols = rgb_palette(extra_palette)
    if pic.palette is not None:
        cols = rgb_palette(pic.palette, cols)
    im = PILImage.frombytes("P", (img.width, img.height), img.pixels[:img.width * img.height])
    im.putpalette([v for c in cols for v in c])
    kw = {"transparency": 0} if transparent0 else {}
    im.save(path, **kw)
    return True


def from_png(path, with_palette: bool = True, first: int = 16) -> Pic:
    """Build a PIC from an indexed (mode "P") PNG.  Palette entries
    ``first..255`` are stored (converted to 6-bit)."""
    from PIL import Image as PILImage
    im = PILImage.open(path)
    if im.mode != "P":
        raise ValueError("PNG must be palette-indexed (mode P)")
    pal = None
    if with_palette:
        raw = im.getpalette() or []
        raw = raw + [0] * (768 - len(raw))
        cols = [(raw[3 * i] >> 2, raw[3 * i + 1] >> 2, raw[3 * i + 2] >> 2) for i in range(first, 256)]
        pal = Palette(first, cols)
    return make(im.tobytes(), im.width, im.height, pal)
