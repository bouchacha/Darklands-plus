#!/usr/bin/env python3
"""soundpack.py - build the sample battle-sound pack from free recordings.

  soundpack.py OUT [--src AUDIO] [--cache DIR]

OUT becomes a mod folder (put it in Darklands+'s mods folder and tick it in F12 > Settings > Mods):

  OUT/mod.txt              the name and description shown in F12 > Settings > Mods
  OUT/sounds/NAME_N.ogg    the battle sounds (miss, parry, hit, knockback, death, bow, gun,
                           burst, magic, ambush, cantgo), several takes of each
  OUT/CREDITS.txt          where every sound comes from, and the licence

The recordings are 0 A.D.'s (Wildfire Games, CC BY-SA 3.0): sword clashes, shields, blows,
death cries, a bow, a musket, the fire and the war horn. AUDIO is the game's
binaries/data/mods/public/audio folder; without --src the needed files are fetched from
0 A.D.'s git repository (github.com/0ad/0ad, only those files) into --cache. They are cut
to start on the blow (the game plays a sound on the frame where the weapon lands), levelled
and in places layered (a hit on mail, a body falling after the cry). The flask breaking and
the magic sparkle are synthesized here. The pack is CC BY-SA 3.0 like its sources and holds
nothing of Darklands, so it can be handed on (with CREDITS.txt).

Needs numpy and ffmpeg (with libvorbis).
"""
import os, sys, argparse, subprocess, shutil
import numpy as np

FS = 44100
REPO = 'https://github.com/0ad/0ad'
REV = '61a3b9507d974084e6badb88a0826bd89a6d5b8b'      # the commit the pack was made from (2026-10)
AUD = 'binaries/data/mods/public/audio'

# ------------------------------------------------------------------ files
def need():
    w, i, d, a = 'attack/weapon/', 'attack/impact/', 'actor/human/death/', 'interface/alarm/'
    f = ['LICENSE.txt']
    f += [w + 'sling_attack_%02d.ogg' % k for k in (1, 3, 4, 6)] + [w + 'jav_attack_%02d.ogg' % k for k in (2, 3)]
    f += [w + 'swordhit_%s.ogg' % k for k in ('11', '12', '15a', '17a', '14b')]
    f += [i + 'shield_metal_%02d.ogg' % k for k in (4, 9, 13, 14)] + [i + 'shield_wood_%02d.ogg' % k for k in (3, 5, 7, 14, 16)]
    f += [i + 'fleshimp_%d.ogg' % k for k in (10, 11, 12)] + [i + 'fleshstab_%02d.ogg' % k for k in (1, 2, 3)]
    f += [d + 'male_death_%02d.ogg' % k for k in DEATHS]
    f += [w + 'bow_attack_%02d.ogg' % k for k in (1, 4, 7, 8)] + [w + 'arrowfly_%02d.ogg' % k for k in (1, 2, 5, 9)]
    f += [w + 'Musket_1.ogg', w + 'Musket_3.ogg']
    f += ['attack/fire/sp_%d.ogg' % k for k in (11, 13, 17)] + ['attack/fire/c_%d.ogg' % k for k in (12, 14, 16)]
    f += [a + 'alarmalert0.ogg', a + 'alarmattackplayer_1.ogg', a + 'alarm_invalid_building_placement_01.ogg']
    return f

DEATHS = (11, 12, 19, 35, 37, 40, 53, 60)

def fetch(cache):
    """only the files the pack uses, from 0 A.D.'s repository (a sparse, blob-less clone)"""
    repo = os.path.join(cache, '0ad')
    if not os.path.isdir(os.path.join(repo, '.git')):
        os.makedirs(cache, exist_ok=True)
        subprocess.run(['git', 'init', '-q', repo], check=True)
        subprocess.run(['git', '-C', repo, 'remote', 'add', 'origin', REPO], check=True)
    subprocess.run(['git', '-C', repo, 'fetch', '-q', '--depth', '1', '--filter=blob:none', 'origin', REV], check=True)
    subprocess.run(['git', '-C', repo, 'sparse-checkout', 'init', '--no-cone'], check=True, capture_output=True)
    with open(os.path.join(repo, '.git', 'info', 'sparse-checkout'), 'w') as fh:
        fh.write(''.join('/%s/%s\n' % (AUD, p) for p in need()))
    subprocess.run(['git', '-C', repo, 'checkout', '-q', REV], check=True)
    return os.path.join(repo, AUD)

# ------------------------------------------------------------------ sound helpers
SRC = ''
def load(rel):
    raw = subprocess.run(['ffmpeg', '-v', 'quiet', '-i', os.path.join(SRC, rel), '-ac', '1', '-ar', str(FS),
                          '-f', 'f32le', '-'], capture_output=True, check=True).stdout
    x = np.frombuffer(raw, np.float32).astype(np.float64)
    if not len(x): sys.exit('cannot read ' + rel)
    return x

def env(x, ms=10):
    w = max(1, int(FS * ms / 1000))
    return np.sqrt(np.convolve(x * x, np.ones(w) / w, 'same'))

def onset(x, db=-30):
    e = env(x, 5)
    return int(np.nonzero(e > e.max() * 10 ** (db / 20))[0][0])

def peak_at(x):
    return int(np.argmax(env(x, 20)))

def cut(x, a, b=None):
    a = max(0, int(a)); b = len(x) if b is None else min(len(x), int(b))
    return x[a:b].copy()

def fade(x, fin=0.004, fout=0.08):
    y = x.copy()
    ni, no = min(len(y), int(fin * FS)), min(len(y), int(fout * FS))
    if ni: y[:ni] *= np.linspace(0, 1, ni)
    if no: y[-no:] *= np.linspace(1, 0, no) ** 2
    return y

def tail(x, db=-45):
    """drop what follows once the sound has died away"""
    e = env(x, 20)
    k = np.nonzero(e > e.max() * 10 ** (db / 20))[0]
    return x[:k[-1] + int(0.02 * FS)] if len(k) else x

def speed(x, r):
    """play r times as fast (pitch up by r)"""
    n = int(len(x) / r)
    return np.interp(np.arange(n) * r, np.arange(len(x)), x)

def level(x, peak):
    return x * (peak / (np.abs(x).max() + 1e-12))

def mix(*parts):
    """(signal, start seconds, gain) ..."""
    n = max(int(t * FS) + len(s) for s, t, g in parts)
    out = np.zeros(n)
    for s, t, g in parts:
        i = int(t * FS); out[i:i + len(s)] += g * s
    return out

def hp(x, fc):
    """a gentle one-pole high-pass (rumble out)"""
    a = np.exp(-2 * np.pi * fc / FS)
    y = np.zeros_like(x); prev_x = prev_y = 0.0
    for i, v in enumerate(x):
        prev_y = a * (prev_y + v - prev_x); prev_x = v; y[i] = prev_y
    return y

def from_onset(x, pre=0.004, length=None, fout=0.08):
    a = onset(x) - pre * FS
    y = cut(x, a, None if length is None else a + length * FS)
    return fade(y, 0.002, fout)

# ------------------------------------------------------------------ synthesized parts (made here)
rng = np.random.default_rng(1492)

def glass(dur=0.6):
    """a flask shattering: a crack and a shower of small, bright, quickly dying pings"""
    n = int(dur * FS); t = np.arange(n) / FS; y = np.zeros(n)
    crack = rng.standard_normal(n) * np.exp(-t / 0.006)
    y += 0.9 * np.diff(np.concatenate([[0], crack]))          # bright click
    for _ in range(46):
        t0 = 0.22 * rng.random() ** 2; f = 2400 + 7000 * rng.random(); tau = 0.008 + 0.05 * rng.random() ** 2
        a = (0.06 + 0.35 * rng.random() ** 3)
        k = t >= t0; tt = t[k] - t0
        y[k] += a * np.exp(-tt / tau) * (np.sin(2 * np.pi * f * tt + rng.random() * 6.28) +
                                         0.5 * np.sin(2 * np.pi * f * 2.41 * tt))
    return y

def sparkle(seed, dur=1.3):
    """a magical effect: bell-like partials rising in a quick arpeggio over a soft airy swell"""
    r = np.random.default_rng(seed)
    n = int(dur * FS); t = np.arange(n) / FS; y = np.zeros(n)
    base = 520 * 2 ** (r.integers(0, 5) / 12)
    steps = [0, 4, 7, 12, 16, 19, 24, 28]                     # a major arpeggio upwards
    for k, st in enumerate(steps):
        f = base * 2 ** (st / 12) * (1 + 0.003 * r.standard_normal())
        t0 = 0.045 * k + 0.01 * r.random(); tau = 0.35 - 0.025 * k
        kk = t >= t0; tt = t[kk] - t0
        e = np.exp(-tt / tau) * np.minimum(1, tt / 0.004)
        trem = 1 + 0.25 * np.sin(2 * np.pi * (5 + k) * tt)
        y[kk] += 0.5 * e * trem * (np.sin(2 * np.pi * f * tt) + 0.3 * np.sin(2 * np.pi * f * 2.76 * tt) +
                                   0.12 * np.sin(2 * np.pi * f * 5.4 * tt))
    # air: noise through a resonant band that sweeps up, swelling and fading
    nz = r.standard_normal(n); out = np.zeros(n); ic1 = ic2 = 0.0
    for i in range(n):
        fc = 1500 + 4000 * min(1, t[i] / 0.6)
        g = np.tan(np.pi * fc / FS); kq = 1 / 4.0
        a1 = 1 / (1 + g * (g + kq)); a2 = g * a1; a3 = g * a2
        v3 = nz[i] - ic2; v1 = a1 * ic1 + a2 * v3; v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2; out[i] = v1
    sw = (t / 0.25) ** 1.5 * np.exp(1.5 * (1 - t / 0.25))
    y += 0.35 * out * sw
    return fade(y, 0.002, 0.25)

# ------------------------------------------------------------------ the sounds
def build():
    S = {}
    # miss: the swish of a swing that finds nothing (sling and javelin whooshes), the swish's
    # loudest point a little after the start
    S['miss'] = []
    for f in ['attack/weapon/sling_attack_01.ogg', 'attack/weapon/sling_attack_03.ogg', 'attack/weapon/sling_attack_04.ogg',
              'attack/weapon/sling_attack_06.ogg', 'attack/weapon/jav_attack_02.ogg', 'attack/weapon/jav_attack_03.ogg']:
        x = load(f); p = peak_at(x)
        y = fade(cut(x, p - 0.09 * FS, p + 0.30 * FS), 0.03, 0.15)
        S['miss'].append(level(hp(speed(y, 1.12), 120), 0.45))
    # parry: blade on blade, on a metal and on a wooden shield
    S['parry'] = []
    for f in ['attack/weapon/swordhit_11.ogg', 'attack/weapon/swordhit_12.ogg', 'attack/weapon/swordhit_15a.ogg',
              'attack/weapon/swordhit_17a.ogg', 'attack/impact/shield_metal_04.ogg', 'attack/impact/shield_metal_13.ogg']:
        S['parry'].append(level(from_onset(tail(load(f)), length=0.55, fout=0.15), 0.55))
    for f in ['attack/impact/shield_wood_03.ogg', 'attack/impact/shield_wood_14.ogg']:
        x = from_onset(tail(load(f)), length=0.35)
        S['parry'].append(level(mix((x, 0, 1.0), (from_onset(load('attack/weapon/swordhit_14b.ogg'), length=0.3), 0.004, 0.25)), 0.55))
    # hit: a blow that lands, some on mail (a chink of metal over the thud)
    S['hit'] = []
    for f in ['attack/impact/fleshimp_10.ogg', 'attack/impact/fleshimp_11.ogg', 'attack/impact/fleshimp_12.ogg',
              'attack/impact/fleshstab_01.ogg', 'attack/impact/fleshstab_02.ogg']:
        S['hit'].append(level(from_onset(tail(load(f)), length=0.45), 0.6))
    for body, metal in [('fleshimp_11', 'shield_metal_09'), ('fleshimp_12', 'shield_metal_14'), ('fleshimp_10', 'shield_metal_09')]:
        b = from_onset(tail(load('attack/impact/%s.ogg' % body)), length=0.4)
        m = from_onset(load('attack/impact/%s.ogg' % metal), length=0.35, fout=0.15)
        S['hit'].append(level(mix((b, 0, 1.0), (level(m, np.abs(b).max()), 0.006, 0.35)), 0.6))
    # knockback: a heavy blow on a shield, deeper, with a rattle
    S['knockback'] = []
    for body, wood, metal in [('fleshimp_12', 'shield_wood_16', 'shield_metal_09'), ('fleshimp_11', 'shield_wood_05', 'shield_metal_14'),
                              ('fleshimp_10', 'shield_wood_07', 'shield_metal_09'), ('fleshimp_12', 'shield_wood_05', 'shield_metal_14')]:
        b = speed(from_onset(tail(load('attack/impact/%s.ogg' % body)), length=0.45), 0.8)
        w = speed(from_onset(tail(load('attack/impact/%s.ogg' % wood)), length=0.3), 0.85)
        m = from_onset(load('attack/impact/%s.ogg' % metal), length=0.35, fout=0.15)
        S['knockback'].append(level(mix((level(b, 1), 0, 1.0), (level(w, 1), 0.002, 0.8), (level(m, 1), 0.03, 0.18)), 0.7))
    # death: the cry, then the body and its armour hitting the ground
    S['death'] = []
    for k, d in enumerate(DEATHS):
        cry = tail(cut(load('actor/human/death/male_death_%02d.ogg' % d), onset(load('actor/human/death/male_death_%02d.ogg' % d)) - 0.02 * FS))
        cry = fade(cry, 0.01, 0.12)
        e = env(cry, 20); end = np.nonzero(e > e.max() * 0.1)[0][-1] / FS
        body = speed(from_onset(tail(load('attack/impact/fleshimp_12.ogg')), length=0.4), 0.65)
        wood = speed(from_onset(tail(load('attack/impact/shield_wood_16.ogg')), length=0.3), 0.75)
        metal = from_onset(load('attack/impact/shield_metal_%02d.ogg' % (14 if k % 2 else 9)), length=0.35, fout=0.15)
        fall = mix((level(body, 1), 0, 1.0), (level(wood, 1), 0.01, 0.6), (level(metal, 1), 0.05, 0.2))
        S['death'].append(level(mix((level(cry, 1), 0, 1.0), (fall, max(0.25, end - 0.12), 0.45)), 0.6))
    # bow: the string's release and the arrow going
    S['bow'] = []
    for b, a in [(1, 1), (4, 5), (7, 9), (8, 2)]:
        x = load('attack/weapon/bow_attack_%02d.ogg' % b); p = peak_at(x)
        rel = fade(cut(x, p - 0.04 * FS, p + 0.35 * FS), 0.01, 0.12)
        fly = from_onset(load('attack/weapon/arrowfly_%02d.ogg' % a), length=0.4, fout=0.15)
        S['bow'].append(level(mix((level(rel, 1), 0, 1.0), (level(fly, 1), 0.035, 0.55)), 0.5))
    # gun: a musket (an arquebus is near enough)
    S['gun'] = []
    for f, r in [('Musket_1', 1.0), ('Musket_3', 1.0), ('Musket_1', 0.92)]:
        x = load('attack/weapon/%s.ogg' % f)
        S['gun'].append(level(fade(speed(cut(x, onset(x) - 0.003 * FS), r), 0.002, 0.4), 0.85))
    # burst: the flask breaks and its contents flare up (the fire's roar and crackle)
    S['burst'] = []
    for k, (sp, c) in enumerate([(11, 12), (13, 14), (17, 16)]):
        roar = load('attack/fire/sp_%d.ogg' % sp); cr = load('attack/fire/c_%d.ogg' % c)
        n = int(1.1 * FS); t = np.arange(n) / FS
        sw = (t / 0.2) ** 1.4 * np.exp(1.4 * (1 - t / 0.2))
        roar = cut(roar, FS * 0.5, FS * 0.5 + n) * sw[:min(n, len(roar) - int(FS * 0.5))]
        cr = cut(cr, FS * 1.0, FS * 1.0 + n) * np.exp(-t / 0.4)[:min(n, len(cr) - int(FS * 1.0))]
        S['burst'].append(level(fade(mix((level(glass(), 1), 0, 0.8), (level(roar, 1), 0.02, 0.9), (level(cr, 1), 0.05, 0.3)), 0.002, 0.2), 0.6))
    # magic: a sparkle (made here; 0 A.D. has no magic)
    S['magic'] = [level(sparkle(s), 0.45) for s in (3, 5, 8)]
    # ambush: a war horn sounds
    S['ambush'] = []
    for f, length in [('alarmalert0', 1.5), ('alarmattackplayer_1', 1.9)]:
        x = load('interface/alarm/%s.ogg' % f)
        S['ambush'].append(level(fade(cut(x, onset(x) - 0.01 * FS, onset(x) + length * FS), 0.02, 0.45), 0.55))
    # cantgo: a short dull knock
    x = load('interface/alarm/alarm_invalid_building_placement_01.ogg')
    S['cantgo'] = [level(from_onset(tail(x), length=0.3, fout=0.1), 0.4)]
    return S

def write_ogg(path, x):
    pcm = np.clip(x, -1, 1).astype(np.float32).tobytes()
    subprocess.run(['ffmpeg', '-v', 'error', '-y', '-f', 'f32le', '-ar', str(FS), '-ac', '1', '-i', '-',
                    '-c:a', 'libvorbis', '-q:a', '5', path], input=pcm, check=True)

CREDITS = """Darklands+ sample battle sounds
===============================

Licence: Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0),
https://creativecommons.org/licenses/by-sa/3.0/

The recordings are from 0 A.D. (https://play0ad.com), copyright (C) Wildfire Games,
http://www.wildfiregames.com/, licensed CC BY-SA 3.0. They were cut, levelled, pitched
and layered for Darklands+ by port/tools/soundpack.py; the flask breaking (burst) and
the magic sparkle (magic) were synthesized by that script. This pack, being made from
them, is licensed CC BY-SA 3.0 too.

Sources (binaries/data/mods/public/audio/ in 0 A.D.'s repository, github.com/0ad/0ad):

  miss       attack/weapon/sling_attack_01, 03, 04, 06; jav_attack_02, 03
  parry      attack/weapon/swordhit_11, 12, 15a, 17a, 14b; attack/impact/shield_metal_04, 13;
             attack/impact/shield_wood_03, 14
  hit        attack/impact/fleshimp_10, 11, 12; fleshstab_01, 02; shield_metal_09, 14
  knockback  attack/impact/fleshimp_10, 11, 12; shield_wood_05, 07, 16; shield_metal_09, 14
  death      actor/human/death/male_death_%s;
             attack/impact/fleshimp_12, shield_wood_16, shield_metal_09, 14
  bow        attack/weapon/bow_attack_01, 04, 07, 08; arrowfly_01, 02, 05, 09
  gun        attack/weapon/Musket_1, Musket_3
  burst      attack/fire/sp_11, 13, 17; c_12, 14, 16 (and synthesized glass)
  magic      synthesized
  ambush     interface/alarm/alarmalert0, alarmattackplayer_1
  cantgo     interface/alarm/alarm_invalid_building_placement_01
"""

def main():
    global SRC
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('out')
    ap.add_argument('--src', help="0 A.D.'s binaries/data/mods/public/audio folder")
    ap.add_argument('--cache', default=os.path.join(os.path.expanduser('~'), '.cache', 'darklands-soundpack'))
    a = ap.parse_args()
    if not shutil.which('ffmpeg'): sys.exit('ffmpeg is needed')
    SRC = a.src or fetch(a.cache)
    S = build()
    sd = os.path.join(a.out, 'sounds')
    if os.path.isdir(sd): shutil.rmtree(sd)
    os.makedirs(sd)
    n = 0
    for name, takes in S.items():
        for k, x in enumerate(takes[:8]):
            write_ogg(os.path.join(sd, '%s_%d.ogg' % (name, k + 1)), x); n += 1
    with open(os.path.join(a.out, 'mod.txt'), 'w') as f:
        f.write('Battle sounds (0 A.D. recordings)\n'
                'Real sounds for the battles in place of the synthesizer notes: swords clashing,\n'
                'shields, blows landing, cries, the bow, the handgun, a war horn for an ambush.\n'
                "Recordings from 0 A.D. by Wildfire Games, CC BY-SA 3.0 (see CREDITS.txt).\n"
                'F12 > Settings > Display sets their volume; switch the mod off for the game\'s own sounds.\n')
    with open(os.path.join(a.out, 'CREDITS.txt'), 'w') as f:
        f.write(CREDITS % ', '.join('%02d' % d for d in DEATHS))
    print('%d sounds written to %s' % (n, sd))

if __name__ == '__main__':
    main()
