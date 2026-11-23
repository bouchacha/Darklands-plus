#!/usr/bin/env python3
"""run_tests.py - build and run every C test under port/tools (test_*.c), each by the gcc command its own
header gives, from the repository root. They use synthetic data only (no game files). release_check.py runs
this first: a test nobody runs stops building within a few changes of the code it includes.

  python3 -I port/tools/run_tests.py [--out DIR]     (DIR for the binaries; a temporary one by default)

Exit status 1 if any test fails to build or to pass. The Python tests beside them (test_*.py) need a game
folder and a save, and are run by hand.
"""
import glob, os, re, subprocess, sys, tempfile

def command(path):
    """the gcc command in the file's opening comment, its lines joined"""
    head = open(path, encoding='utf-8', errors='replace').read().split('*/', 1)[0]
    lines = [re.sub(r'^\s*/?\*+\s?', '', l).rstrip() for l in head.split('\n')]
    for i, l in enumerate(lines):
        if not l.startswith('gcc '): continue
        cmd = l
        while cmd.endswith('\\') and i + 1 < len(lines):
            i += 1; cmd = cmd[:-1] + ' ' + lines[i].strip()
        return cmd.split('&&')[0].strip()
    return None

def main():
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    out = sys.argv[sys.argv.index('--out') + 1] if '--out' in sys.argv else tempfile.mkdtemp(prefix='dltests')
    os.makedirs(out, exist_ok=True)
    failed = 0
    for path in sorted(glob.glob(os.path.join(root, 'port', 'tools', 'test_*.c'))):
        name = os.path.basename(path)[:-2]
        cmd = command(path)
        if not cmd:
            print('%-24s FAILED: no gcc command in its header' % name); failed += 1; continue
        exe = os.path.join(out, name)
        cmd = re.sub(r'-o\s+\S+', '-o ' + exe, cmd, count=1)
        b = subprocess.run(cmd, shell=True, cwd=root, capture_output=True, text=True)
        if b.returncode:
            print('%-24s FAILED to build\n%s' % (name, '\n'.join((b.stderr or b.stdout).strip().split('\n')[-12:]))); failed += 1; continue
        r = subprocess.run([exe], cwd=root, capture_output=True, text=True, timeout=300)
        if r.returncode:
            print('%-24s FAILED (%d)\n%s' % (name, r.returncode, '\n'.join((r.stdout + r.stderr).strip().split('\n')[-12:]))); failed += 1
        else:
            print('%-24s OK' % name)
    print('tests: %s' % ('all passed' if not failed else '%d FAILED' % failed))
    return 1 if failed else 0

if __name__ == '__main__':
    sys.exit(main())
