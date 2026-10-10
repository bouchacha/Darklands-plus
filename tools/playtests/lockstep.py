#!/usr/bin/env python3
"""Interactive lock-step play-testing: several DOSBox sessions (original + builds under test) that stay
running between commands, so a human/agent can look at each screenshot and decide the next step
(needed for battles: encounters are random). Start the sessions in the same second so they share the
RNG seed; then every `do` sends the same steps to all of them in parallel.
Output dir: $LOCKSTEP_DIR (default build/qa/sess). Kill with `stop` (it also removes the X lock files).
  lockstep.py start NAME EXE|orig     start DOSBox+Xvfb, kept running
  lockstep.py do "script" NAME [NAME2] run play.py-style steps on both sessions in parallel; shots go to
                                   build/qa/sess/NAME/<shot>.png and a side-by-side build/qa/sess/last.png
  lockstep.py stop NAME
"""
import sys, os, json, time, subprocess, shutil, tempfile, threading
sys.path.insert(0, '/home/claude/darklands-re/tools')
import play
GAME = '/home/claude/dl_raw/extracted/DARKLAND'
D = os.environ.get('LOCKSTEP_DIR', '/home/claude/darklands-re/build/qa/sess')
def st(name): return os.path.join(D, name + '.json')

def start(name, exe):
    work = tempfile.mkdtemp(prefix='dlsess', dir=os.environ.get('TMPDIR'))
    gd = os.path.join(work, 'DARKLAND'); shutil.copytree(GAME, gd)
    if exe != 'orig': shutil.copy(exe, os.path.join(gd, 'DARKLAND.EXE'))
    conf = os.path.join(work, 'dosbox.conf')
    open(conf, 'w').write(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lockstep_dosbox.conf.tmpl')).read() % work)
    disp = play._free_display()
    xv = subprocess.Popen(['Xvfb', disp, '-screen', '0', '800x600x24'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(1.5)
    env = dict(os.environ, DISPLAY=disp, SDL_AUDIODRIVER='dummy')
    os.makedirs(os.path.join(D, name), exist_ok=True)
    log = open(os.path.join(D, name, 'dosbox.log'), 'w')   # kept after stop: why DOSBox exited
    db = subprocess.Popen(['dosbox', '-conf', conf, '-noconsole'], env=env, stdout=log, stderr=log)
    os.makedirs(D, exist_ok=True)
    json.dump(dict(work=work, disp=disp, xv=xv.pid, db=db.pid, inside=False), open(st(name), 'w'))

def do(name, script):
    s = json.load(open(st(name)))
    env = dict(os.environ, DISPLAY=s['disp'])
    out = os.path.join(D, name)
    for step in [x.strip() for x in script.split(';') if x.strip()]:
        cmd, _, arg = step.partition(' ')
        if cmd == 'wait': time.sleep(float(arg))
        elif cmd == 'key':
            wins = subprocess.run(['xdotool', 'search', '--class', 'dosbox'], env=env, capture_output=True, text=True).stdout.split()
            for w in wins: subprocess.run(['xdotool', 'windowfocus', w], env=env, capture_output=True)
            for k in arg.split():
                subprocess.run(['xdotool', 'keydown', k], env=env); time.sleep(0.12)
                subprocess.run(['xdotool', 'keyup', k], env=env); time.sleep(0.3)
        elif cmd in ('move', 'at', 'click', 'rclick'):
            if not s['inside']:
                subprocess.run(['xdotool', 'mousemove', '240', '200'], env=env, capture_output=True); s['inside'] = True
            if cmd in ('click', 'rclick'):
                b = '1' if cmd == 'click' else '3'
                subprocess.run(['xdotool', 'mousedown', b], env=env); time.sleep(0.2)
                subprocess.run(['xdotool', 'mouseup', b], env=env); time.sleep(0.3); continue
            dx, dy = (int(v) for v in arg.split())
            if cmd == 'at':
                play._rel(env, -700, -500); time.sleep(0.2); dx, dy = dx * 2, dy * 2
            play._rel(env, dx, dy); time.sleep(0.3)
        elif cmd == 'type':
            subprocess.run(['xdotool', 'type', '--delay', '150', arg], env=env)
        elif cmd == 'shot':
            xwd = os.path.join(s['work'], 'x.xwd')
            subprocess.run(['xwd', '-root', '-silent', '-out', xwd], env=env)
            subprocess.run(['convert', xwd, '-crop', '320x200+80+100', '+repage', os.path.join(out, arg + '.png')])
    json.dump(s, open(st(name), 'w'))

def stop(name):
    s = json.load(open(st(name)))
    for p, sig in ((s['db'], 9), (s['xv'], 15)):
        try: os.kill(p, sig)
        except OSError: pass
    time.sleep(0.5)
    n = s['disp'][1:]
    for f in ('/tmp/.X%s-lock' % n, '/tmp/.X11-unix/X%s' % n):
        try: os.remove(f)
        except OSError: pass
    shutil.rmtree(s['work'], ignore_errors=True)

if __name__ == '__main__':
    a = sys.argv[1:]
    if a[0] == 'start': start(a[1], a[2])
    elif a[0] == 'stop': [stop(n) for n in a[1:]]
    elif a[0] == 'do':
        script = a[1].replace('QUICKSTART', play.QUICKSTART); names = a[2:]
        ts = [threading.Thread(target=do, args=(n, script)) for n in names]
        [t.start() for t in ts]; [t.join() for t in ts]
        shots = [x.strip().split(' ', 1)[1].strip() for x in script.split(';') if x.strip().startswith('shot ')]
        if shots:
            rows = []
            for sh in shots:
                rows.append(['(', *[os.path.join(D, n, sh + '.png') for n in names], '+append', ')'])
            args = ['convert'] + [x for r in rows for x in r] + ['-append', '-scale', '200%', os.path.join(D, 'last.png')]
            subprocess.run(args)
