#!/usr/bin/env python3
"""package.py - the Windows release of Darklands+: only our own code and the tools that make the
game module on the player's machine (runtime/split.h). Nothing made from the game is put in.

  package.py --exe build/port_win64_split/darklands.exe --tcc TCCDIR \\
             --python-zip python-3.12.x-embed-amd64.zip --capstone-wheel capstone-...-win_amd64.whl \\
             --version NAME --out DarklandsPlus-NAME-win64.zip

  TCCDIR          port/tools/build_tcc_win.sh
  --python-zip    the "Windows embeddable package (64-bit)" from python.org
  --capstone-wheel  pip download capstone --only-binary=:all: --platform win_amd64 --python-version 3.12

Layout of the zip (its contents go into the Darklands folder, next to DARKLAND.EXE):
  darklands.exe                   the program (make SPLIT=1 TARGET=win64)
  darklands_plus/src/...          the translator: the files of port/tools/translator_files.txt
  darklands_plus/tcc/             TinyCC
  darklands_plus/python/          embedded Python with capstone
  darklands_plus/licenses/        the notices of everything in the package
  Darklands+ README.txt
"""
import sys, os, shutil, zipfile, argparse, tempfile, glob, re

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
PORT = os.path.join(ROOT, 'port')


def translator_files():
    for line in open(os.path.join(HERE, 'translator_files.txt')):
        p = line.split('#')[0].strip()
        if p:
            yield p


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', required=True)
    ap.add_argument('--tcc', required=True)
    ap.add_argument('--python-zip')
    ap.add_argument('--capstone-wheel')
    ap.add_argument('--version', default='test')
    ap.add_argument('--out', required=True)
    ap.add_argument('--no-python', action='store_true', help='leave Python out (tests only)')
    a = ap.parse_args()
    if not a.no_python and not (a.python_zip and a.capstone_wheel):
        ap.error('--python-zip and --capstone-wheel are needed (or --no-python for a test)')
    st = tempfile.mkdtemp(prefix='dlpkg')
    plus = os.path.join(st, 'darklands_plus')
    lic = os.path.join(plus, 'licenses')
    os.makedirs(lic)
    shutil.copy(a.exe, os.path.join(st, 'darklands.exe'))
    # the translator
    for p in list(translator_files()) + ['port/tools/translator_files.txt']:
        d = os.path.join(plus, 'src', p)
        os.makedirs(os.path.dirname(d), exist_ok=True)
        shutil.copy(os.path.join(ROOT, p), d)
    # TinyCC
    shutil.copytree(a.tcc, os.path.join(plus, 'tcc'))
    shutil.copy(os.path.join(a.tcc, 'COPYING'), os.path.join(lic, 'TinyCC-LGPL-2.1.txt'))
    # Python and capstone
    if not a.no_python:
        py = os.path.join(plus, 'python')
        zipfile.ZipFile(a.python_zip).extractall(py)
        sp = os.path.join(py, 'Lib', 'site-packages')
        zipfile.ZipFile(a.capstone_wheel).extractall(sp)
        pth = glob.glob(os.path.join(py, 'python*._pth'))
        if not pth:
            raise SystemExit('no python*._pth in the embeddable package')
        with open(pth[0], 'a') as f:
            f.write('\nLib\\site-packages\n')
        if os.path.exists(os.path.join(py, 'LICENSE.txt')):
            shutil.copy(os.path.join(py, 'LICENSE.txt'), os.path.join(lic, 'Python-PSF.txt'))
        for l in glob.glob(os.path.join(sp, 'capstone*', 'LICENSE*')) + glob.glob(os.path.join(sp, 'capstone*.dist-info', 'LICENSE*')):
            shutil.copy(l, os.path.join(lic, 'capstone-' + os.path.basename(l)))
    # our third-party notices
    tp = os.path.join(PORT, 'third_party')
    shutil.copy(os.path.join(tp, 'imgui', 'LICENSE.txt'), os.path.join(lic, 'DearImGui-MIT.txt'))
    for f in os.listdir(os.path.join(tp, 'nuked-opl3')):
        if f.upper().startswith('LICENSE'):
            shutil.copy(os.path.join(tp, 'nuked-opl3', f), os.path.join(lic, 'NukedOPL3-LGPL.txt'))
    for f in os.listdir(os.path.join(tp, 'fonts')):
        if f.endswith('.txt'):
            shutil.copy(os.path.join(tp, 'fonts', f), os.path.join(lic, 'fonts-' + f))
    open(os.path.join(lic, 'SDL2-zlib.txt'), 'w').write(SDL_ZLIB)
    for f in ('LICENSE', 'LICENSE.txt', 'LICENSE.md', 'COPYING'):
        if os.path.exists(os.path.join(ROOT, f)):
            shutil.copy(os.path.join(ROOT, f), os.path.join(lic, 'Darklands+-' + f))
    readme = open(os.path.join(HERE, 'release_readme.txt')).read().replace('@VERSION@', a.version)
    open(os.path.join(st, 'Darklands+ README.txt'), 'w', newline='\r\n').write(readme)
    # nothing made from the game may be in it
    bad = []
    for dp, dn, fn in os.walk(st):
        for f in fn:
            if re.match(r'(g_s\d+|g_root|funcs|d_\w+)\.(c|h)$', f) or 'darklands_game' in f or f.upper() in ('DARKLAND.EXE', 'MGRAPHIC.EXE'):
                bad.append(os.path.join(dp, f))
    if bad:
        raise SystemExit('game-derived files in the package: %s' % bad[:5])
    if os.path.exists(a.out):
        os.remove(a.out)
    with zipfile.ZipFile(a.out, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for dp, dn, fn in os.walk(st):
            for f in sorted(fn):
                p = os.path.join(dp, f)
                z.write(p, os.path.relpath(p, st))
    shutil.rmtree(st)
    print('wrote', a.out, os.path.getsize(a.out) // 1024, 'KB')


SDL_ZLIB = '''Simple DirectMedia Layer
Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
'''

if __name__ == '__main__':
    main()
