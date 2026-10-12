#!/usr/bin/env python3
"""make_icon.py OUT.ico [OUT.png] - the Darklands+ icon: a red shield with a gold rim and a
Fraktur "D+" (DL Fraktur, SIL OFL). Our own design; run again to change it."""
import sys, os
from PIL import Image, ImageDraw, ImageFont
HERE = os.path.dirname(os.path.abspath(__file__))
FONT = os.path.join(HERE, '..', 'third_party', 'fonts', 'DLFraktur-Regular.ttf')


def shield(S):
    im = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    def pts(inset):
        k = S / 256.0
        l, r, t = (20 + inset) * k, (236 - inset) * k, (14 + inset) * k
        mid, bot = S / 2, (246 - inset * 1.4) * k
        p = [(l, t), (r, t)]
        for i in range(1, 21):            # the right side curving down to the point
            f = i / 20.0
            p.append((r - (r - mid) * f ** 2.2, t + (bot - t) * (0.45 + 0.55 * f) if f > 0 else t))
        p2 = [(S - x, y) for (x, y) in reversed(p[2:])]
        return p + p2
    d.polygon(pts(0), fill=(196, 152, 64, 255))           # gold rim
    d.polygon(pts(9), fill=(122, 26, 18, 255))            # red field
    d.polygon(pts(15), outline=(232, 196, 110, 255), width=max(1, S // 128))
    f = ImageFont.truetype(FONT, int(S * 0.62))
    tw = d.textbbox((0, 0), 'D', font=f)
    x = (S - (tw[2] - tw[0])) / 2 - tw[0] - S * 0.05
    y = S * 0.44 - (tw[3] - tw[1]) / 2 - tw[1]
    d.text((x, y), 'D', font=f, fill=(246, 236, 214, 255))
    g = ImageFont.truetype(FONT, int(S * 0.36))
    d.text((S * 0.66, S * 0.14), '+', font=g, fill=(232, 196, 110, 255))
    return im


if __name__ == '__main__':
    big = shield(256)
    big.save(sys.argv[1], sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    if len(sys.argv) > 2:
        big.save(sys.argv[2])
