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
The game directory is copied first, so the original install is never modified.
"""
import sys, os, subprocess, time, shutil, tempfile

DISPLAY = ':77'


def run(game_dir, out_dir, script, exe='DARKLAND.EXE', cycles='12000', keep=False):
    os.makedirs(out_dir, exist_ok=True)
    work = tempfile.mkdtemp(prefix='dlrun')
    gd = os.path.join(work, 'DARKLAND')
    shutil.copytree(game_dir, gd)
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
    xvfb = subprocess.Popen(['Xvfb', DISPLAY, '-screen', '0', '800x600x24'], stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    time.sleep(1.5)
    env = dict(os.environ, DISPLAY=DISPLAY, SDL_AUDIODRIVER='dummy')
    log = open(os.path.join(out_dir, 'dosbox.log'), 'w')
    db = subprocess.Popen(['dosbox', '-conf', conf, '-noconsole'], env=env, stdout=log, stderr=log)
    shots = []
    try:
        for step in [s.strip() for s in script.split(';') if s.strip()]:
            cmd, _, arg = step.partition(' ')
            if db.poll() is not None:
                break
            if cmd == 'wait':
                time.sleep(float(arg))
            elif cmd == 'key':
                subprocess.run(['xdotool', 'search', '--class', 'dosbox', 'windowactivate', '--sync'], env=env,
                               capture_output=True)
                for k in arg.split():
                    subprocess.run(['xdotool', 'key', '--delay', '120', k], env=env)
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


if __name__ == '__main__':
    alive, shots = run(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else 'wait 25; shot s1')
    print('still running at end of script:', alive)
    print('\n'.join(shots))
