#!/usr/bin/env python3
"""
nplay.py - run a tools/play.py-style script against the NATIVE build (headless) and save
screenshots as PNG.

  nplay.py [--exe build/port/darklands] [--game DIR] <out_dir> <script or @file>

Script steps (same as tools/play.py): wait N; key K [K..]; type TEXT; shot NAME; click;
rclick; at X Y (game pixels 320x200); QUICKSTART. Times are game time: the native build runs
in real time like DOSBox, so the same waits work.
"""
import sys, os, subprocess, shutil, tempfile, argparse

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.abspath(os.path.join(HERE, '..', '..'))
QUICKSTART = 'wait 10; key space; click; wait 3; key space; wait 6; at 160 62; click; wait 12'
KEYMAP = {'Return': 'Return', 'space': 'Space', 'Escape': 'Escape', 'BackSpace': 'Backspace', 'Tab': 'Tab',
          'plus': 'Keypad +', 'minus': 'Keypad -', 'Up': 'Up', 'Down': 'Down', 'Left': 'Left', 'Right': 'Right'}


def convert(script, out_dir):
    t, x, y = 500, 160, 100
    steps, shots = [], []
    script = script.replace('QUICKSTART', QUICKSTART)
    for raw in script.replace('\n', ';').split(';'):
        f = raw.split()
        if not f or f[0].startswith('#'):
            continue
        c = f[0]
        if c == 'wait':
            t += int(float(f[1]) * 1000)
        elif c == 'key':
            for k in f[1:]:
                steps.append('%d:key:%s' % (t, KEYMAP.get(k, k)))
                t += 150
        elif c == 'type':
            for ch in ' '.join(f[1:]):
                steps.append('%d:key:%s' % (t, 'Space' if ch == ' ' else ch))
                t += 120
        elif c == 'shot':
            p = os.path.join(out_dir, f[1] + '.bmp')
            steps.append('%d:shot:%s' % (t, p))
            shots.append(p)
        elif c == 'at':
            x, y = int(f[1]), int(f[2])
            steps.append('%d:move:%d,%d' % (t, x, y))
            t += 100
        elif c in ('rdown', 'rup', 'ldown', 'lup'):
            steps.append('%d:%s:%d' % (t, 'press' if c.endswith('down') else 'release', 2 if c[0] == 'r' else 1))
            t += 50
        elif c in ('click', 'rclick'):
            steps.append('%d:%s:%d,%d' % (t, c, x, y))
            t += 250
        elif c == 'wshot':
            p = os.path.join(out_dir, f[1] + '.bmp')
            steps.append('%d:wshot:%s' % (t, p))
            shots.append(p)
            t += 100
        elif c in ('artexport', 'artrescan', 'umap'):
            steps.append('%d:%s:' % (t, c))
        elif c in ('utab', 'ugive', 'uclick', 'uhover', 'ucmd'):
            steps.append('%d:%s:%s' % (t, c, ' '.join(f[1:])) if c != 'ucmd' else '%d:%s' % (t, ' '.join(f[1:])))
            t += 100
        elif c == 'move':
            x += int(f[1]) // 2; y += int(f[2]) // 2
            steps.append('%d:move:%d,%d' % (t, x, y))
        else:
            raise SystemExit('unknown step ' + raw)
    steps.append('%d:quit' % (t + 500))
    return ';'.join(steps), shots, t + 500


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.path.join(TOP, 'build', 'port', 'darklands'))
    ap.add_argument('--game', default=os.environ.get('DARKLANDS_DIR', '/home/claude/dl_raw/extracted/DARKLAND'), help='the Darklands folder (copied, not changed; default $DARKLANDS_DIR)')
    ap.add_argument('--keep', action='store_true')
    ap.add_argument('--wine', action='store_true', help='run a Windows build under Wine')
    ap.add_argument('--x11', action='store_true', help='run in a virtual X display (xvfb-run) with the window '
                    'and the Darklands+ interface, so wshot/utab/ugive work')
    ap.add_argument('--ini', default='', help='settings for darklands_port.ini, "key=value;key=value"')
    ap.add_argument('out_dir')
    ap.add_argument('script')
    a = ap.parse_args()
    script = a.script
    if script.startswith('@'):
        script = '\n'.join(l for l in open(script[1:]) if not l.startswith('#'))
    os.makedirs(a.out_dir, exist_ok=True)
    dl, shots, total = convert(script, os.path.abspath(a.out_dir))
    work = tempfile.mkdtemp(prefix='nplay')
    gd = os.path.join(work, 'game')
    shutil.copytree(a.game, gd)
    if a.ini:
        with open(os.path.join(gd, 'darklands_port.ini'), 'w') as f:
            for kv in a.ini.split(';'):
                if '=' in kv:
                    k, v = kv.split('=', 1)
                    f.write('%s = %s\n' % (k.strip(), v.strip()))
    env = dict(os.environ, DL_SCRIPT=dl, SDL_AUDIODRIVER='dummy')
    if not a.x11:
        env['DL_HEADLESS'] = '1'
    cmd = [a.exe, gd]
    if a.wine:
        env.update(WINEDEBUG='-all')
        cmd = ['wine'] + cmd
    if a.x11:
        cmd = ['xvfb-run', '-a', '-s', '-screen 0 1920x1080x24'] + cmd
    r = subprocess.run(cmd, env=env, capture_output=True, timeout=total / 1000 + 120)
    err = r.stderr.decode('latin1') + (r.stdout.decode('latin1') if a.x11 else '')   # xvfb-run sends stderr to stdout
    if a.wine and os.path.exists(os.path.join(gd, 'darklands_port.log')):
        err += open(os.path.join(gd, 'darklands_port.log'), encoding='latin1').read()
    for line in err.splitlines():
        if 'FATAL' in line or 'error' in line.lower() or 'program' in line:
            print(line)
    try:
        from PIL import Image
        for p in shots:
            if os.path.exists(p):
                Image.open(p).save(p[:-4] + '.png')
                os.remove(p)
                print('shot', p[:-4] + '.png')
            else:
                print('missing', p)
    except ImportError:
        pass
    open(os.path.join(a.out_dir, 'stderr.txt'), 'w').write(err)
    if not a.keep:
        shutil.rmtree(work, ignore_errors=True)
    print('exit', r.returncode)


if __name__ == '__main__':
    main()
