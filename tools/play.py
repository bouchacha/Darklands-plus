#!/usr/bin/env python3
"""
Headless game runner for smoke tests: runs a Darklands install in DOSBox on a virtual X
display, sends a scripted key sequence and saves screenshots.

  play.py <game_dir> <out_dir> [script]

script: semicolon-separated steps, e.g. "wait 20; shot intro; key space; wait 5; shot menu"
  wait N        sleep N seconds
  key K [K..]   xdotool key names (space, Return, Escape, a, F1 ...)
  type TEXT     type literal text
  shot NAME     save <out_dir>/NAME.png
  click         left mouse click (hold 0.2 s)
  move DX DY    move the mouse by DX, DY screen pixels (relative, which is what DOSBox reads)
  at X Y        put the game's mouse cursor at game pixel X, Y (320x200) and leave it there
Keys only reach the game at some points (the intro accepts a key only while the MPS logo is
showing, about 9-12 s in); the menus are mouse driven, so prefer 'at X Y; click'.
The game directory is copied first, so the original install is never modified.
"""
import sys, os, subprocess, time, shutil, tempfile



def _free_display():
    """A display number nobody uses, so several runs can go in parallel."""
    for n in range(77, 200):
        if not os.path.exists('/tmp/.X%d-lock' % n) and not os.path.exists('/tmp/.X11-unix/X%d' % n):
            return ':%d' % n
    raise SystemExit('no free X display')


# screen pixels of relative motion per game pixel (calibrated with the main menu)
MOUSE_SCALE = (2, 2)


def _rel(env, dx, dy):
    """Relative mouse motion in small steps (big jumps get clipped by the game)."""
    steps = max(1, (max(abs(dx), abs(dy)) + 7) // 8)
    sx = sy = 0
    for k in range(1, steps + 1):
        tx, ty = dx * k // steps, dy * k // steps
        subprocess.run(['xdotool', 'mousemove_relative', '--', str(tx - sx), str(ty - sy)], env=env,
                       capture_output=True)
        sx, sy = tx, ty
        time.sleep(0.02)


def run(game_dir, out_dir, script, exe='DARKLAND.EXE', cycles='12000', keep=False, use_exe=None):
    os.makedirs(out_dir, exist_ok=True)
    work = tempfile.mkdtemp(prefix='dlrun')
    gd = os.path.join(work, 'DARKLAND')
    shutil.copytree(game_dir, gd)
    if use_exe:                      # e.g. a rebuilt or modded executable
        shutil.copy(use_exe, os.path.join(gd, exe))
    conf = os.path.join(work, 'dosbox.conf')
    open(conf, 'w').write('''[sdl]
fullscreen=false
output=surface
[dosbox]
memsize=16
machine=svga_s3
[render]
frameskip=0
aspect=false
scaler=none
[cpu]
core=auto
cputype=auto
cycles=%s
[mixer]
nosound=true
[sblaster]
sbtype=none
[speaker]
pcspeaker=false
[dos]
ems=true
xms=true
[autoexec]
mount c %s
c:
cd \\DARKLAND
%s
exit
''' % (cycles, work, exe))
    DISPLAY = _free_display()
    xvfb = subprocess.Popen(['Xvfb', DISPLAY, '-screen', '0', '800x600x24'], stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    time.sleep(1.5)
    env = dict(os.environ, DISPLAY=DISPLAY, SDL_AUDIODRIVER='dummy')
    log = open(os.path.join(out_dir, 'dosbox.log'), 'w')
    db = subprocess.Popen(['dosbox', '-conf', conf, '-noconsole'], env=env, stdout=log, stderr=log)
    shots = []
    run._inside = False
    try:
        for step in [s.strip() for s in script.split(';') if s.strip()]:
            cmd, _, arg = step.partition(' ')
            if db.poll() is not None:
                break
            if cmd == 'wait':
                time.sleep(float(arg))
            elif cmd == 'key':
                # no window manager on the virtual display: give the DOSBox window the input focus
                # explicitly and keep the pointer over it
                wins = subprocess.run(['xdotool', 'search', '--class', 'dosbox'], env=env,
                                      capture_output=True, text=True).stdout.split()
                subprocess.run(['xdotool', 'mousemove', '320', '200'], env=env, capture_output=True)
                for w in wins:
                    subprocess.run(['xdotool', 'windowfocus', w], env=env, capture_output=True)
                for k in arg.split():
                    subprocess.run(['xdotool', 'keydown', k], env=env)
                    time.sleep(0.12)
                    subprocess.run(['xdotool', 'keyup', k], env=env)
                    time.sleep(0.3)
            elif cmd in ('move', 'at', 'click'):
                subprocess.run(['xdotool', 'mousemove', '240', '200'], env=env, capture_output=True) \
                    if not getattr(run, '_inside', False) else None
                run._inside = True
                if cmd == 'click':
                    subprocess.run(['xdotool', 'mousedown', '1'], env=env)
                    time.sleep(0.2)
                    subprocess.run(['xdotool', 'mouseup', '1'], env=env)
                    time.sleep(0.3)
                    continue
                dx, dy = (int(v) for v in arg.split())
                if cmd == 'at':
                    _rel(env, -700, -500)
                    time.sleep(0.2)
                    dx, dy = dx * MOUSE_SCALE[0], dy * MOUSE_SCALE[1]
                _rel(env, dx, dy)
                time.sleep(0.3)
            elif cmd == 'type':
                subprocess.run(['xdotool', 'type', '--delay', '150', arg], env=env)
            elif cmd == 'shot':
                xwd = os.path.join(work, arg + '.xwd')
                subprocess.run(['xwd', '-root', '-silent', '-out', xwd], env=env)
                png = os.path.join(out_dir, arg + '.png')
                subprocess.run(['convert', xwd, '-crop', '640x400+0+0', '+repage', png])
                shots.append(png)
        alive = db.poll() is None
    finally:
        db.kill()
        xvfb.kill()
        log.close()
        if not keep:
            shutil.rmtree(work, ignore_errors=True)
    return alive, shots


# start a Quickstart game: skip the intro, lock the mouse, click 'Quickstart'
QUICKSTART = 'wait 10; key space; click; wait 3; key space; wait 6; at 160 62; click; wait 12'

if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('game_dir')
    ap.add_argument('out_dir')
    ap.add_argument('script', nargs='?', default='wait 25; shot s1')
    ap.add_argument('--exe', help='run this executable instead of the installed DARKLAND.EXE')
    a = ap.parse_args()
    script = a.script.replace('QUICKSTART', QUICKSTART)
    alive, shots = run(a.game_dir, a.out_dir, script, use_exe=a.exe)
    print('still running at end of script:', alive)
    print('\n'.join(shots))
