#!/usr/bin/env python3
"""artpack.py - make a replacement-art mod from the game's own pictures.

  artpack.py ORIGINALS OUT [--style clown] [--scale 4] [--only NAME,NAME...]

ORIGINALS is the folder Darklands+ writes with F12 > Display > "Export the original
pictures" (art/original: every picture of the game as PNG, the same names, colour 0
transparent where the game draws it as a sprite). OUT becomes a mod folder:

  OUT/mod.txt          the name and description shown in F12 > Mods
  OUT/art/NAME.png     a replacement for PICS/NAME.PIC, SCALE times the original size

A style is a function that turns one picture (RGBA, already scaled up) into its replacement.
'clown' is the sample: every piece of armour on the character sheet becomes a clown costume
(each kind of armour its own loud pattern, the picture's shading kept: harlequin
diamonds, polka dots, rainbow stripes, striped tights, a ruff and pompom buttons) and
shields become bullseyes. (The weapon pictures carry a piece of the sheet's pillar behind
them, so they are left alone.) Other styles (an image model run on each picture,
say) plug in the same way: see STYLES at the end.

The pack is made from your own game files; do not hand it on with the game's pictures in it.
"""
import os, sys, argparse, math
import numpy as np
from PIL import Image, ImageFilter

# ------------------------------------------------------------------ helpers
def load(path, scale):
    im = Image.open(path).convert('RGBA')
    big = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    # soften the stair-steps of the scaled-up outline: blur the alpha and cut it again
    a = big.getchannel('A').filter(ImageFilter.GaussianBlur(scale * 0.45))
    a = a.point(lambda v: 255 if v >= 128 else 0)
    big.putalpha(a)
    return big

def shade_of(rgba, blur=3.0):
    """the picture's light and dark (0..1), smoothed (the dithering of mail and the like would
    show as a checkerboard), to keep its modelling"""
    rgb = rgba[..., :3].astype(np.float32)
    lum = (0.30 * rgb[..., 0] + 0.59 * rgb[..., 1] + 0.11 * rgb[..., 2])
    lum = np.array(Image.fromarray(np.clip(lum, 0, 255).astype(np.uint8), 'L').filter(ImageFilter.GaussianBlur(blur)), np.float32) / 255.0
    m = rgba[..., 3] > 0
    if m.any():
        lo, hi = np.percentile(lum[m], 5), np.percentile(lum[m], 95)
        lum = np.clip((lum - lo) / max(hi - lo, 1e-3), 0, 1)
    return lum

def paint(rgba, colour, shade, strength=0.75):
    """colour (H, W, 3, 0..255) shaded like the original where the picture is"""
    k = (1.0 - strength) + strength * (0.35 + 0.85 * shade)
    out = rgba.copy()
    m = rgba[..., 3] > 0
    c = np.clip(colour * k[..., None], 0, 255)
    out[..., :3][m] = c[m].astype(np.uint8)
    return out

RAINBOW = np.array([(230, 40, 40), (250, 150, 30), (250, 230, 40), (60, 190, 70), (40, 120, 230), (150, 60, 200)], np.float32)

def pattern(kind, h, w, s):
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    if kind == 'rainbow':       # diagonal rainbow stripes
        band = ((xx + yy) / (9 * s)).astype(int) % len(RAINBOW)
        return RAINBOW[band]
    if kind == 'dots':          # big polka dots
        base = np.zeros((h, w, 3), np.float32) + (220, 30, 40)
        cx = (xx % (14 * s)) - 7 * s; cy = (yy % (14 * s)) - 7 * s
        dot = cx * cx + cy * cy < (4.2 * s) ** 2
        base[dot] = (255, 235, 60)
        return base
    if kind == 'harlequin':     # diamonds
        u = ((xx + yy) / (10 * s)).astype(int); v = ((xx - yy) / (10 * s)).astype(int)
        base = np.zeros((h, w, 3), np.float32) + (40, 90, 220)
        base[(u + v) % 2 == 0] = (250, 240, 230)
        base[(u % 3 == 0) & ((u + v) % 2 == 1)] = (230, 40, 120)
        return base
    if kind == 'checks':        # big checks
        base = np.zeros((h, w, 3), np.float32) + (90, 200, 60)
        base[((xx // (8 * s)) + (yy // (8 * s))) % 2 == 0] = (160, 60, 210)
        return base
    if kind == 'stripes':       # horizontal stripes (tights)
        base = np.zeros((h, w, 3), np.float32) + (250, 250, 245)
        base[(yy // (4 * s)) % 2 == 0] = (225, 35, 45)
        return base
    if kind == 'candy':         # barber's pole
        base = np.zeros((h, w, 3), np.float32) + (250, 250, 245)
        base[((xx - yy) // (3 * s)) % 2 == 0] = (225, 30, 40)
        return base
    if kind == 'confetti':      # pastel with confetti
        rng = np.random.default_rng(7)
        base = np.zeros((h, w, 3), np.float32) + (255, 190, 220)
        n = int(h * w / (60 * s * s))
        for _ in range(n):
            y, x = rng.integers(0, h), rng.integers(0, w)
            r = int(1.4 * s); c = RAINBOW[rng.integers(0, len(RAINBOW))]
            base[max(0, y - r):y + r, max(0, x - r):x + r] = c
        return base
    raise ValueError(kind)

def pompoms(rgba, s):
    """three fluffy buttons down the middle of the body"""
    a = rgba[..., 3] > 0
    ys, xs = np.nonzero(a)
    if len(xs) == 0: return rgba
    x0, x1, y0, y1 = xs.min(), xs.max(), ys.min(), ys.max()
    cx = (x0 + x1) // 2
    out = rgba.copy()
    hh = y1 - y0
    yy, xx = np.mgrid[0:rgba.shape[0], 0:rgba.shape[1]]
    for i, col in enumerate([(255, 255, 255), (60, 120, 240), (255, 230, 40)]):
        cy = y0 + int(hh * (0.38 + 0.17 * i))
        r = 3.2 * s
        d = (xx - cx) ** 2 + (yy - cy) ** 2
        ring = (d < (r + s) ** 2) & a
        out[ring, :3] = (30, 20, 20)
        dot = (d < r * r) & a
        out[dot, :3] = col
    return out

def ruff(rgba, s):
    """a white ruffled collar where the neck meets the body (the narrowest row near the top)"""
    a = rgba[..., 3] > 0
    rows = a.sum(axis=1)
    ys = np.nonzero(rows)[0]
    if len(ys) < 10: return rgba
    top = ys[0]; span = ys[-1] - top
    seg = rows[top + span // 8: top + span // 2]
    if len(seg) == 0: return rgba
    neck = top + span // 8 + int(np.argmin(np.where(seg > 0, seg, 10 ** 6)))
    xs = np.nonzero(a[neck])[0]
    if len(xs) == 0: return rgba
    cx = (xs.min() + xs.max()) / 2; half = max(6 * s, (xs.max() - xs.min()) * 1.6)
    out = rgba.copy()
    H, W = a.shape
    for k in range(int(-half), int(half), int(2 * s)):
        x = cx + k
        for dy in range(int(-3 * s), int(3 * s)):
            y = neck + dy + int(1.5 * s * math.sin(k / (2 * s) * math.pi))
            xi = int(x)
            if 0 <= y < H:
                for dx in range(int(-1.6 * s), int(1.6 * s)):
                    if 0 <= xi + dx < W:
                        out[y, xi + dx] = (250, 250, 250, 255) if abs(dy) < 2.6 * s else (40, 30, 30, 255)
    return out

def bullseye(rgba, s):
    a = rgba[..., 3] > 0
    ys, xs = np.nonzero(a)
    if len(xs) == 0: return rgba
    cy, cx = (ys.min() + ys.max()) / 2, (xs.min() + xs.max()) / 2
    yy, xx = np.mgrid[0:rgba.shape[0], 0:rgba.shape[1]].astype(np.float32)
    ring = (np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / (4 * s)).astype(int) % len(RAINBOW)
    return paint(rgba, RAINBOW[ring], shade_of(rgba), 0.5)

# ------------------------------------------------------------------ the clown style
ARMOUR = {   # picture -> pattern (each kind of armour its own costume)
    'PLATEVIT': 'rainbow', 'PLATELIM': 'stripes', 'CHAINVIT': 'dots', 'CHAINLIM': 'stripes',
    'BRIGVIT': 'harlequin', 'BRIGLIM': 'harlequin', 'SCALEVIT': 'confetti', 'SCALELIM': 'stripes',
    'CUIRBVIT': 'checks', 'CUIRBLIM': 'checks', 'LEAT-VIT': 'dots', 'LEAT-LIM': 'stripes',
    'STUDVIT': 'rainbow', 'STUDLIM': 'candy', 'PAD-VIT': 'confetti', 'PAD-LIMS': 'checks',
}

def clown(name, rgba, s):
    if name in ARMOUR:
        out = paint(rgba, pattern(ARMOUR[name], rgba.shape[0], rgba.shape[1], s), shade_of(rgba))
        if name.endswith('VIT'):
            out = pompoms(out, s)
            out = ruff(out, s)
        return out
    if name in ('SMALLSH', 'MEDIUMSH', 'LARGESH'):
        return bullseye(rgba, s)
    return None                 # not part of the costume: left as the game draws it

STYLES = {'clown': (clown, 'Clown Company (sample art)',
                    'A sample of replacement art: every piece of armour on the character sheet becomes a\n'
                    'clown costume and shields become bullseyes. Made by\n'
                    'port/tools/artpack.py from the exported original pictures; switch it off to go back.')}

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('originals'); ap.add_argument('out')
    ap.add_argument('--style', default='clown', choices=sorted(STYLES))
    ap.add_argument('--scale', type=int, default=4)
    ap.add_argument('--only', default='')
    a = ap.parse_args()
    fn, title, desc = STYLES[a.style]
    only = set(x.strip().upper() for x in a.only.split(',') if x.strip())
    os.makedirs(os.path.join(a.out, 'art'), exist_ok=True)
    done = 0
    for f in sorted(os.listdir(a.originals)):
        if not f.lower().endswith('.png'): continue
        name = f[:-4].upper()
        if only and name not in only: continue
        big = load(os.path.join(a.originals, f), a.scale)
        res = fn(name, np.array(big), a.scale)
        if res is None: continue
        Image.fromarray(res, 'RGBA').save(os.path.join(a.out, 'art', name + '.png'))
        done += 1
    with open(os.path.join(a.out, 'mod.txt'), 'w') as f:
        f.write(title + '\n' + desc + '\n')
    print('%d pictures written to %s' % (done, os.path.join(a.out, 'art')))

if __name__ == '__main__':
    main()
