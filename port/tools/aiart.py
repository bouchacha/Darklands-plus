#!/usr/bin/env python3
"""aiart.py - repaint the game's pictures with an image model, as a replacement-art mod.

  aiart.py ORIGINALS OUT --backend a1111 [--url http://127.0.0.1:7860] [options]
  aiart.py ORIGINALS OUT --backend openai            (OPENAI_API_KEY in the environment)
  aiart.py ORIGINALS OUT --backend command --cmd "python my_sd.py {in} {out} {prompt_file} {strength} {seed}"
  aiart.py ORIGINALS OUT --backend test              (no model: checks the pipeline)

ORIGINALS is art/original as F12 > Display > "Export the original pictures" writes it. OUT
becomes a mod folder (mod.txt, art/NAME.png or .jpg) to put into the mods folder and tick in
F12 > Mods; it also gets review.html (each original beside its new picture, with the prompt) and
aiart.json (what was asked for each picture, so a run can be repeated or continued).

How a picture is made:
  1. its kind is told from its name and contents: a scene (the 320x200 card and screen pictures),
     an item (armour and shields on the character sheet, with transparency), a weapon (the same,
     but drawn with a piece of the sheet's pillar behind it, so left alone unless --kinds says)
     or interface (frames, buttons, letters, icons: left alone unless --kinds says)
  2. the prompt: the style (--style, a description of the whole pack's look), the kind's own
     words, and the picture's caption from captions.txt if there is one ("XCAVE: a hunchbacked
     man in a cave ..."; --write-captions writes a file to fill in, --caption-with openai asks a
     vision model to fill it)
  3. the original, scaled up (--scale, 4 by default) and softened, goes to the model as the
     starting image (image-to-image): --strength says how far it may move from it (0.3 keeps the
     composition and colours, 0.6 repaints freely). The seed comes from the name, so a run
     can be repeated; --seed-offset gives other variants.
  4. the result is fitted back: scaled to exactly SCALE times the original, the parchment border
     of the cards and the transparency of items kept from the original, and (--match-colours)
     its colours drawn towards the original's so the game's palette and mood stay.

Backends:
  a1111    Stable Diffusion WebUI (AUTOMATIC1111, Forge, SD.Next ...) started with --api:
           POST /sdapi/v1/img2img. --steps, --cfg, --sampler, --negative; the checkpoint is
           whatever the WebUI has loaded.
  openai   OpenAI's image edits (POST /v1/images/edits, --model gpt-image-1 by default); the
           picture is padded to the nearest size it accepts and cut back. Costs money per image.
  command  any program: {in} {out} {prompt} {prompt_file} {negative} {strength} {seed} {w} {h}
           are filled in (ComfyUI or a diffusers script go here).
  test     a filter instead of a model, to try the whole pipeline.

Nothing of the game is sent anywhere by the local backends; openai and --caption-with openai
send the pictures to OpenAI. A pack made from the game's pictures is for your own use.
"""
import os, sys, io, json, re, base64, hashlib, argparse, subprocess, shlex, tempfile, time, html
import urllib.request, urllib.error
import numpy as np
from PIL import Image, ImageFilter

STYLE = ('a hand-painted illustration in the manner of late medieval German painting and woodcuts, '
         'Holy Roman Empire around 1400, muted earthy colours, detailed, atmospheric lighting')
NEGATIVE = 'text, letters, watermark, signature, frame, border, modern, photograph, 3d render, cartoon, anime, blurry, deformed'
KIND_WORDS = {
    'scene': 'a scene from a story of adventure',
    'item': 'a single piece of armour or a shield shown on its own, centred, plain background',
    'weapon': 'a single medieval weapon shown upright in front of a stone pillar',
    'interface': 'an ornament of a game interface',
}

# ------------------------------------------------------------------ kinds
ITEM_RE = re.compile(r'^(SHIELD[A-Z]|(SMALL|MEDIUM|LARGE)SH|[A-Z-]+(VIT|LIM|LIMS)|BRIGSTUD|MAILCUIR)$')
WEAPON_RE = re.compile(r'^WEAPON\d+$')        # drawn with a piece of the sheet's pillar behind them
UI_RE = re.compile(r'^(BUTT|RP|HORIZ|VERT|NEWHORIZ|NEWVERT|SIDEBAR|TEXTBACK|MAP|COPY\d|BUBBLE|BUBLBACK|TIMER|HSTAT|'
                   r'CURS|HANDIC|MOUSEPTR|CHARICON|[A-F]\d\d(SHORT|SMALL|STAT)|PTYSTATS|ARMBRS|FAME|HOURGLAS|'
                   r'ILLMCAPS|DIN|SID$|SMALLGUY|BUYSELL|COMMON|HALLFAME|ARMBACK|ARMSBACK|SNTBACK|WEAPONS|ARMR3)')

def kind_of(name, im):
    if ITEM_RE.match(name): return 'item'
    if WEAPON_RE.match(name): return 'weapon'
    if UI_RE.match(name): return 'interface'
    if im.size == (320, 200): return 'scene'
    return 'interface'

# ------------------------------------------------------------------ captions
def read_captions(path):
    caps = {}
    if path and os.path.exists(path):
        for line in open(path, encoding='utf-8'):
            line = line.strip()
            if not line or line.startswith('#') or ':' not in line: continue
            k, v = line.split(':', 1)
            if v.strip(): caps[k.strip().upper()] = v.strip()
    return caps

def write_captions(path, names, kinds, old):
    with open(path, 'w', encoding='utf-8') as f:
        f.write('# One line a picture: NAME: what it shows (the prompt adds the style). Empty lines are left out.\n')
        for n in names:
            if kinds[n] == 'interface': continue
            f.write('%s: %s\n' % (n, old.get(n, '')))

def caption_openai(im, a):
    buf = io.BytesIO(); im.convert('RGB').resize((im.width * 2, im.height * 2), Image.NEAREST).save(buf, 'PNG')
    body = {'model': a.caption_model, 'max_tokens': 120, 'messages': [{'role': 'user', 'content': [
        {'type': 'text', 'text': 'This is a picture from Darklands (1992), a role-playing game set in the Holy Roman Empire '
                                 'around 1400. Describe what it shows in one sentence for an image generator: the people, '
                                 'creatures and things, the place, the light. No opinions, no mention of the game or pixels.'},
        {'type': 'image_url', 'image_url': {'url': 'data:image/png;base64,' + base64.b64encode(buf.getvalue()).decode()}}]}]}
    r = http_json(a.openai_url.rstrip('/') + '/chat/completions', body, {'Authorization': 'Bearer ' + api_key()})
    return r['choices'][0]['message']['content'].strip().replace('\n', ' ')

# ------------------------------------------------------------------ http
def http_json(url, body, headers=None, timeout=600):
    req = urllib.request.Request(url, data=json.dumps(body).encode(), method='POST',
                                 headers=dict({'Content-Type': 'application/json'}, **(headers or {})))
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        raise SystemExit('%s: HTTP %d %s' % (url, e.code, e.read()[:400].decode('utf-8', 'replace')))
    except urllib.error.URLError as e:
        raise SystemExit('%s: %s (is the server running?)' % (url, e.reason))

def http_multipart(url, fields, files, headers, timeout=600):
    bnd = '----aiart' + hashlib.md5(str(time.time()).encode()).hexdigest()
    out = io.BytesIO()
    for k, v in fields.items():
        out.write(('--%s\r\nContent-Disposition: form-data; name="%s"\r\n\r\n%s\r\n' % (bnd, k, v)).encode())
    for k, (fn, data, ct) in files.items():
        out.write(('--%s\r\nContent-Disposition: form-data; name="%s"; filename="%s"\r\nContent-Type: %s\r\n\r\n' % (bnd, k, fn, ct)).encode())
        out.write(data); out.write(b'\r\n')
    out.write(('--%s--\r\n' % bnd).encode())
    req = urllib.request.Request(url, data=out.getvalue(), method='POST',
                                 headers=dict({'Content-Type': 'multipart/form-data; boundary=' + bnd}, **headers))
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        raise SystemExit('%s: HTTP %d %s' % (url, e.code, e.read()[:400].decode('utf-8', 'replace')))
    except urllib.error.URLError as e:
        raise SystemExit('%s: %s' % (url, e.reason))

def api_key():
    k = os.environ.get('OPENAI_API_KEY')
    if not k: raise SystemExit('OPENAI_API_KEY is not set')
    return k

def b64png(im):
    buf = io.BytesIO(); im.save(buf, 'PNG'); return base64.b64encode(buf.getvalue()).decode()

def from_b64(s):
    return Image.open(io.BytesIO(base64.b64decode(s.split(',', 1)[-1]))).convert('RGB')

# ------------------------------------------------------------------ backends: (init RGB image, prompt, negative, strength, seed, a) -> RGB image
def be_a1111(init, prompt, neg, strength, seed, a):
    w, h = (init.width + 7) // 8 * 8, (init.height + 7) // 8 * 8
    body = {'init_images': [b64png(init.resize((w, h), Image.LANCZOS))], 'prompt': prompt, 'negative_prompt': neg,
            'denoising_strength': strength, 'seed': seed, 'steps': a.steps, 'cfg_scale': a.cfg, 'width': w, 'height': h,
            'sampler_name': a.sampler, 'batch_size': 1, 'n_iter': 1}
    r = http_json(a.url.rstrip('/') + '/sdapi/v1/img2img', body)
    if not r.get('images'): raise SystemExit('the WebUI sent no image: %s' % str(r)[:300])
    return from_b64(r['images'][0])

OPENAI_SIZES = [(1024, 1024), (1536, 1024), (1024, 1536)]
def be_openai(init, prompt, neg, strength, seed, a):
    ar = init.width / init.height
    W, H = min(OPENAI_SIZES, key=lambda s: abs(s[0] / s[1] - ar))
    # fit the picture inside, padding with its own edge colours, and cut the same box out again
    sc = min(W / init.width, H / init.height); iw, ih = int(init.width * sc), int(init.height * sc)
    canvas = init.resize((W, H), Image.BILINEAR).filter(ImageFilter.GaussianBlur(30))
    x0, y0 = (W - iw) // 2, (H - ih) // 2
    canvas.paste(init.resize((iw, ih), Image.LANCZOS), (x0, y0))
    buf = io.BytesIO(); canvas.save(buf, 'PNG')
    text = prompt + '. Keep the composition, the figures and their poses exactly as in the image; repaint it in this style. Avoid: ' + neg
    r = http_multipart(a.openai_url.rstrip('/') + '/images/edits', {'model': a.model, 'prompt': text, 'size': '%dx%d' % (W, H), 'n': '1'},
                       {'image': ('image.png', buf.getvalue(), 'image/png')}, {'Authorization': 'Bearer ' + api_key()})
    out = from_b64(r['data'][0]['b64_json'])
    if out.size != (W, H): out = out.resize((W, H), Image.LANCZOS)
    return out.crop((x0, y0, x0 + iw, y0 + ih))

def be_command(init, prompt, neg, strength, seed, a):
    with tempfile.TemporaryDirectory() as td:
        pin, pout, pp = os.path.join(td, 'in.png'), os.path.join(td, 'out.png'), os.path.join(td, 'prompt.txt')
        init.save(pin); open(pp, 'w', encoding='utf-8').write(prompt)
        vals = {'in': pin, 'out': pout, 'prompt': prompt, 'prompt_file': pp, 'negative': neg, 'strength': strength,
                'seed': seed, 'w': init.width, 'h': init.height}
        cmd = a.cmd.format(**{k: shlex.quote(str(v)) for k, v in vals.items()})
        r = subprocess.run(cmd, shell=True)
        if r.returncode or not os.path.exists(pout): raise SystemExit('the command failed: ' + cmd)
        return Image.open(pout).convert('RGB')

def be_test(init, prompt, neg, strength, seed, a):
    """a stand-in for a model: smooth and posterise, and tint by the seed (shows that the pipeline works)"""
    im = init.filter(ImageFilter.ModeFilter(7)).filter(ImageFilter.SMOOTH_MORE)
    arr = np.asarray(im).astype(np.float32)
    arr = np.round(arr / 32) * 32
    rng = np.random.default_rng(seed); tint = rng.uniform(0.9, 1.1, 3)
    return Image.fromarray(np.clip(arr * tint, 0, 255).astype(np.uint8), 'RGB')

BACKENDS = {'a1111': be_a1111, 'openai': be_openai, 'command': be_command, 'test': be_test}

# ------------------------------------------------------------------ fitting the result back
def keep_mask(orig, kind, scale):
    """1 where the original is kept: transparency of items; for scenes the plain border that runs
    in from the picture's edges (the parchment around the cards)"""
    a = np.asarray(orig.getchannel('A')) == 0
    if kind == 'scene' and not a.any():
        rgb = np.asarray(orig.convert('RGB')).astype(int)
        edge = np.concatenate([rgb[0], rgb[-1], rgb[:, 0], rgb[:, -1]])
        vals, counts = np.unique(edge, axis=0, return_counts=True)
        bg = vals[np.argmax(counts)]
        near = np.abs(rgb - bg).sum(axis=2) < 40
        if near[0].mean() > 0.5 or near[:, 0].mean() > 0.5:
            # only what is connected to the edge (a flood fill over 'near')
            keep = np.zeros_like(near); stack = [(y, x) for y in range(near.shape[0]) for x in (0, near.shape[1] - 1) if near[y, x]]
            stack += [(y, x) for x in range(near.shape[1]) for y in (0, near.shape[0] - 1) if near[y, x]]
            while stack:
                y, x = stack.pop()
                if keep[y, x] or not near[y, x]: continue
                keep[y, x] = True
                for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    yy, xx = y + dy, x + dx
                    if 0 <= yy < near.shape[0] and 0 <= xx < near.shape[1] and not keep[yy, xx] and near[yy, xx]: stack.append((yy, xx))
            a = keep
    m = Image.fromarray((a * 255).astype(np.uint8), 'L').resize((orig.width * scale, orig.height * scale), Image.BILINEAR)
    return m.filter(ImageFilter.GaussianBlur(scale * 0.8))

def match_colours(new, ref, amount):
    n = np.asarray(new).astype(np.float32); r = np.asarray(ref).astype(np.float32)
    out = n.copy()
    for c in range(3):
        nm, ns, rm, rsd = n[..., c].mean(), n[..., c].std() + 1e-3, r[..., c].mean(), r[..., c].std() + 1e-3
        out[..., c] = (n[..., c] - nm) / ns * rsd + rm
    return Image.fromarray(np.clip(n + (out - n) * amount, 0, 255).astype(np.uint8), 'RGB')

def init_image(orig, scale):
    rgb = orig.convert('RGB')
    big = rgb.resize((orig.width * scale, orig.height * scale), Image.NEAREST)
    return big.filter(ImageFilter.GaussianBlur(scale * 0.35))

def seed_of(name, off):
    return (int(hashlib.sha1(name.encode()).hexdigest()[:8], 16) + off) % 2147483647

# ------------------------------------------------------------------ review page
def review(out, done, recs):
    rows = []
    for n in done:
        r = recs[n]
        rows.append('<div class="r"><h3>%s <small>%s, strength %.2f, seed %d</small></h3>'
                    '<img class="o" src="review/%s.png"><img src="art/%s"><p>%s</p></div>'
                    % (n, r['kind'], r['strength'], r['seed'], n, r['file'], html.escape(r['prompt'])))
    page = ('<!doctype html><meta charset="utf-8"><title>Art pack review</title><style>body{font-family:sans-serif;background:#222;color:#ddd;margin:16px}'
            '.r{margin-bottom:28px}img{max-width:48%;margin-right:1%;vertical-align:top}.o{image-rendering:pixelated}'
            'p{font-size:13px;color:#aaa}small{color:#888;font-weight:normal}</style><h1>Art pack review</h1>'
            '<p>Left the original, right the new picture. To make one again: aiart.py ... --only NAME --redo '
            '(--seed-offset 1 for another variant, --strength to stay closer or move further).</p>' + ''.join(rows))
    open(os.path.join(out, 'review.html'), 'w', encoding='utf-8').write(page)

# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('originals'); ap.add_argument('out')
    ap.add_argument('--backend', choices=sorted(BACKENDS), default='test')
    ap.add_argument('--url', default='http://127.0.0.1:7860', help='a1111: the WebUI address')
    ap.add_argument('--openai-url', default='https://api.openai.com/v1')
    ap.add_argument('--model', default='gpt-image-1', help='openai: the image model')
    ap.add_argument('--cmd', default='', help='command: the command line, with {in} {out} {prompt} ...')
    ap.add_argument('--scale', type=int, default=4)
    ap.add_argument('--strength', type=float, default=0.45)
    ap.add_argument('--item-strength', type=float, default=0.35)
    ap.add_argument('--steps', type=int, default=30); ap.add_argument('--cfg', type=float, default=6.5)
    ap.add_argument('--sampler', default='DPM++ 2M')
    ap.add_argument('--style', default=STYLE); ap.add_argument('--negative', default=NEGATIVE)
    ap.add_argument('--captions', default='', help='captions file (default OUT/captions.txt)')
    ap.add_argument('--write-captions', action='store_true', help='write the captions file to fill in, then stop')
    ap.add_argument('--caption-with', choices=['openai'], help='fill empty captions with a vision model')
    ap.add_argument('--caption-model', default='gpt-4o-mini')
    ap.add_argument('--kinds', default='scene,item', help='which kinds to repaint: scene, item, weapon, interface')
    ap.add_argument('--only', default='', help='these pictures only (comma separated)')
    ap.add_argument('--limit', type=int, default=0, help='at most this many new pictures this run')
    ap.add_argument('--redo', action='store_true', help='make pictures again that are already done')
    ap.add_argument('--seed-offset', type=int, default=0)
    ap.add_argument('--match-colours', type=float, default=0.5, help='0 none, 1 the original\'s colours entirely')
    ap.add_argument('--jpeg', type=int, default=92, help='quality of scenes without transparency saved as .jpg (0: always .png)')
    ap.add_argument('--name', default='Repainted art')
    a = ap.parse_args()

    os.makedirs(os.path.join(a.out, 'art'), exist_ok=True); os.makedirs(os.path.join(a.out, 'review'), exist_ok=True)
    names = sorted(f[:-4].upper() for f in os.listdir(a.originals) if f.lower().endswith('.png'))
    files = {f[:-4].upper(): os.path.join(a.originals, f) for f in os.listdir(a.originals) if f.lower().endswith('.png')}
    origs = {n: Image.open(files[n]).convert('RGBA') for n in names}
    kinds = {n: kind_of(n, origs[n]) for n in names}
    capfile = a.captions or os.path.join(a.out, 'captions.txt')
    caps = read_captions(capfile)
    want = set(k.strip() for k in a.kinds.split(','))
    only = set(x.strip().upper() for x in a.only.split(',') if x.strip())
    todo = [n for n in names if kinds[n] in want and (not only or n in only)]

    if a.caption_with:
        for n in todo:
            if n in caps: continue
            caps[n] = caption_openai(origs[n], a); print('%s: %s' % (n, caps[n])); write_captions(capfile, names, kinds, caps)
    if a.write_captions or a.caption_with:
        write_captions(capfile, names, kinds, caps)
        print('captions in %s (%d of %d filled)' % (capfile, sum(1 for n in names if caps.get(n)), sum(1 for n in names if kinds[n] != 'interface')))
        if a.write_captions: return

    recf = os.path.join(a.out, 'aiart.json')
    recs = json.load(open(recf)) if os.path.exists(recf) else {}
    be = BACKENDS[a.backend]
    made = 0
    for n in todo:
        if n in recs and not a.redo and os.path.exists(os.path.join(a.out, 'art', recs[n]['file'])): continue
        if a.limit and made >= a.limit: break
        o, k = origs[n], kinds[n]
        strength = a.item_strength if k in ('item', 'weapon') else a.strength
        prompt = ', '.join(x for x in (caps.get(n, ''), KIND_WORDS[k], a.style) if x)
        seed = seed_of(n, a.seed_offset)
        init = init_image(o, a.scale)
        t0 = time.time()
        new = be(init, prompt, a.negative, strength, seed, a)
        W, H = o.width * a.scale, o.height * a.scale
        if new.size != (W, H): new = new.resize((W, H), Image.LANCZOS)
        ref = o.convert('RGB').resize((W, H), Image.BICUBIC)
        if a.match_colours > 0: new = match_colours(new, ref, a.match_colours)
        km = keep_mask(o, k, a.scale)
        transparent = np.asarray(o.getchannel('A')).min() == 0
        if transparent:            # items: the original's shape, the new paint
            alpha = o.getchannel('A').resize((W, H), Image.BILINEAR).point(lambda v: 255 if v >= 128 else 0)
            res = new.convert('RGBA'); res.putalpha(alpha)
            fn = n + '.png'
        else:                      # scenes: the original's border, the new paint inside
            base = o.convert('RGB').resize((W, H), Image.BICUBIC).filter(ImageFilter.GaussianBlur(a.scale * 0.3))
            res = Image.composite(base, new, km)
            fn = n + ('.jpg' if a.jpeg else '.png')
        for ext in ('.png', '.jpg'):
            old = os.path.join(a.out, 'art', n + ext)
            if os.path.exists(old) and n + ext != fn: os.remove(old)
        p = os.path.join(a.out, 'art', fn)
        if fn.endswith('.jpg'): res.convert('RGB').save(p, quality=a.jpeg)
        else: res.save(p)
        o.resize((o.width * 2, o.height * 2), Image.NEAREST).save(os.path.join(a.out, 'review', n + '.png'))
        recs[n] = {'file': fn, 'kind': k, 'prompt': prompt, 'negative': a.negative, 'strength': strength, 'seed': seed,
                   'backend': a.backend, 'scale': a.scale}
        json.dump(recs, open(recf, 'w'), indent=1)
        made += 1
        print('%-10s %-6s %5.1fs  %s' % (n, k, time.time() - t0, fn), flush=True)
    with open(os.path.join(a.out, 'mod.txt'), 'w') as f:
        f.write(a.name + '\nThe game\'s pictures repainted by an image model (port/tools/aiart.py, %s).\n' % a.backend)
    review(a.out, [n for n in names if n in recs], recs)
    print('%d new pictures; %d in the pack; review.html shows them' % (made, len(recs)))

if __name__ == '__main__':
    main()
