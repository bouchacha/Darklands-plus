#!/usr/bin/env python3
"""
nctl.py - interactive control of a headless native session (for automated play that needs to
look at the screen between steps, e.g. hunting for a battle).

  nctl.py start NAME [--exe EXE] [--game DIR] [--x11]
          --x11: a window on a private virtual X display, so the Darklands+ interface is drawn
          (steps wshot NAME, utab N, ucmd CMD:ARG work)
  nctl.py do NAME "at 120 30; click; wait 2; shot a"    -> prints the PNG paths
  nctl.py stop NAME

Steps: as tools/play.py (wait, key, type, at, click, rclick, shot, QUICKSTART).
"""
import sys, os, time, subprocess, shutil, signal

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.abspath(os.path.join(HERE, '..', '..'))
BASE = os.environ.get('NCTL_DIR', '/tmp/nctl')
QUICKSTART = 'wait 10; key space; click; wait 3; key space; wait 6; at 160 62; click; wait 12'
KEYMAP = {'space': 'Space', 'plus': 'Keypad +', 'minus': 'Keypad -', 'BackSpace': 'Backspace'}


def sdir(name):
    return os.path.join(BASE, name)


def start(name, exe, game, x11=False):
    d = sdir(name)
    if os.path.exists(os.path.join(d, 'pid')):
        stop(name)
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d)
    shutil.copytree(game, os.path.join(d, 'game'))
    open(os.path.join(d, 'ctl'), 'w').close()
    env = dict(os.environ, DL_HEADLESS='1', SDL_AUDIODRIVER='dummy', DL_CTL=os.path.join(d, 'ctl'))
    if x11:
        disp = 100 + (sum(map(ord, name)) % 400)
        xv = subprocess.Popen(['Xvfb', ':%d' % disp, '-screen', '0', '1280x800x24', '-nolisten', 'tcp'],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, start_new_session=True)
        open(os.path.join(d, 'xpid'), 'w').write(str(xv.pid))
        time.sleep(1.0)
        del env['DL_HEADLESS']
        env['DISPLAY'] = ':%d' % disp
    if exe.lower().endswith('.exe'):        # a Windows build under Wine: its output goes to darklands_port.log
        env['WINEDEBUG'] = '-all'
        os.symlink(os.path.join(d, 'game', 'darklands_port.log'), os.path.join(d, 'out.txt'))
        p = subprocess.Popen(['wine', exe, os.path.join(d, 'game')], env=env, stdout=subprocess.DEVNULL,
                             stderr=subprocess.DEVNULL, start_new_session=True)
    else:
        p = subprocess.Popen([exe, os.path.join(d, 'game')], env=env, stdout=open(os.path.join(d, 'out.txt'), 'w'),
                             stderr=subprocess.STDOUT, start_new_session=True)
    open(os.path.join(d, 'pid'), 'w').write(str(p.pid))
    open(os.path.join(d, 'pos'), 'w').write('160 100')
    print('started', name, p.pid)


def send(d, line):
    with open(os.path.join(d, 'ctl'), 'a') as f:
        f.write(line + '\n')


def do(name, script):
    d = sdir(name)
    x, y = map(int, open(os.path.join(d, 'pos')).read().split())
    shots = []
    script = script.replace('QUICKSTART', QUICKSTART)
    for raw in script.replace('\n', ';').split(';'):
        f = raw.split()
        if not f:
            continue
        c = f[0]
        if c == 'wait':
            time.sleep(float(f[1]))
        elif c == 'key':
            for k in f[1:]:
                send(d, 'key:%s' % KEYMAP.get(k, k)); time.sleep(0.15)
        elif c == 'type':
            for ch in ' '.join(f[1:]):
                send(d, 'key:%s' % ('Space' if ch == ' ' else ch)); time.sleep(0.12)
        elif c == 'at':
            x, y = int(f[1]), int(f[2])
            send(d, 'move:%d,%d' % (x, y)); time.sleep(0.1)
        elif c in ('click', 'rclick'):
            send(d, '%s:%d,%d' % (c, x, y)); time.sleep(0.3)
        elif c == 'wshot':
            p = os.path.join(d, f[1] + '.bmp')
            if os.path.exists(p):
                os.remove(p)
            send(d, 'wshot:%s' % p)
            last = -1
            for _ in range(100):
                if os.path.exists(p) and os.path.getsize(p) == last and last > 0:
                    break
                last = os.path.getsize(p) if os.path.exists(p) else -1
                time.sleep(0.1)
            try:
                from PIL import Image
                png = p[:-4] + '.png'
                Image.open(p).save(png)
                shots.append(png)
            except Exception as e:
                shots.append(p + ' (%s)' % e)
        elif c in ('utab', 'ugive'):
            send(d, '%s:%s' % (c, ' '.join(f[1:]))); time.sleep(0.1)
        elif c == 'ucmd':
            send(d, ' '.join(f[1:])); time.sleep(0.1)
        elif c == 'shot':
            p = os.path.join(d, f[1] + '.bmp')
            if os.path.exists(p):
                os.remove(p)
            send(d, 'shot:%s' % p)
            for _ in range(100):
                if os.path.exists(p) and os.path.getsize(p) >= 192054:
                    break
                time.sleep(0.05)
            time.sleep(0.1)
            try:
                from PIL import Image
                png = p[:-4] + '.png'
                Image.open(p).save(png)
                shots.append(png)
            except Exception as e:
                shots.append(p + ' (%s)' % e)
        else:
            raise SystemExit('unknown step ' + raw)
    open(os.path.join(d, 'pos'), 'w').write('%d %d' % (x, y))
    alive = os.path.exists('/proc/' + open(os.path.join(d, 'pid')).read().strip())
    for s in shots:
        print(s)
    if not alive:
        print('SESSION ENDED; output tail:')
        print(open(os.path.join(d, 'out.txt'), encoding='latin1').read()[-1500:])


def stop(name):
    d = sdir(name)
    for f in ('pid', 'xpid'):
        try:
            pid = int(open(os.path.join(d, f)).read())
            os.kill(pid, signal.SIGKILL)
        except Exception:
            pass
    for f in ('pid', 'xpid'):
        try:
            os.remove(os.path.join(d, f))
        except Exception:
            pass
    print('stopped', name)


if __name__ == '__main__':
    a = sys.argv[1:]
    exe = os.path.join(TOP, 'build', 'port', 'darklands')
    game = '/home/claude/dl_raw/extracted/DARKLAND'
    if '--exe' in a:
        i = a.index('--exe'); exe = a[i + 1]; del a[i:i + 2]
    if '--game' in a:
        i = a.index('--game'); game = a[i + 1]; del a[i:i + 2]
    x11 = '--x11' in a
    if x11:
        a.remove('--x11')
    if a[0] == 'start':
        start(a[1], exe, game, x11)
    elif a[0] == 'do':
        do(a[1], a[2])
    elif a[0] == 'stop':
        stop(a[1])
