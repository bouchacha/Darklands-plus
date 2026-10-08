#!/usr/bin/env python3
"""
Run Microsoft C 6.00A (DOS) under the emu2 emulator and inspect the resulting OMF objects.

  msc.py compile <file.c> [flags...]      -> <file>.obj next to the source (flags default: see DEFAULT)
  msc.py dis <file.obj>                    -> disassembly of every code segment, fixups marked

Environment:
  MSC_DOS   DOS tree containing MSC\\BIN, MSC\\INCLUDE, MSC\\LIB (default /home/claude/dos)
  EMU2      emu2 binary (default /home/claude/tools_ext/emu2/emu2)
"""
import sys, os, subprocess, shutil, tempfile, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omflink

DOS = os.environ.get('MSC_DOS', '/home/claude/dos')
EMU2 = os.environ.get('EMU2', '/home/claude/tools_ext/emu2/emu2')
DEFAULT = ['/AM', '/G2', '/Gs', '/Ox']


def compile_c(src, flags=None, extra_files=(), keep_asm=False):
    """Compile src (a path) in a scratch DOS dir. Returns (obj_bytes, compiler_output, asm_text)."""
    flags = list(flags or DEFAULT)
    work = tempfile.mkdtemp(prefix='w', dir=os.path.join(DOS, 'TMP') if os.path.isdir(os.path.join(DOS, 'TMP')) else None)
    if not os.path.isdir(os.path.join(DOS, 'TMP')):
        os.makedirs(os.path.join(DOS, 'TMP'), exist_ok=True)
        shutil.rmtree(work, ignore_errors=True)
        work = tempfile.mkdtemp(prefix='w', dir=os.path.join(DOS, 'TMP'))
    name = 'X.C'
    shutil.copy(src, os.path.join(work, name))
    for f in extra_files:
        shutil.copy(f, os.path.join(work, os.path.basename(f).upper()))
    dosdir = 'C:\\TMP\\' + os.path.basename(work).upper()
    # emu2 maps file names case-insensitively onto the unix tree
    env = dict(os.environ, EMU2_DRIVE_C=DOS, EMU2_CWD=dosdir)
    args = [EMU2, os.path.join(DOS, 'MSC', 'BIN', 'CL.EXE'), '/c', '/nologo'] + flags
    if keep_asm:
        args.append('/Fc')
    args += [name, '--', 'PATH=C:\\MSC\\BIN', 'INCLUDE=C:\\MSC\\INCLUDE', 'TMP=' + dosdir]
    r = subprocess.run(args, env=env, capture_output=True, timeout=300)
    out = (r.stdout + r.stderr).decode('latin-1')
    obj = None
    asm = None
    for f in os.listdir(work):
        if f.lower() == 'x.obj':
            obj = open(os.path.join(work, f), 'rb').read()
        if f.lower() == 'x.cod':
            asm = open(os.path.join(work, f), encoding='latin-1').read()
    shutil.rmtree(work, ignore_errors=True)
    return obj, out, asm


def obj_code(obj_bytes):
    """Return {segment name: (bytes, fixups)} for an OMF object."""
    with tempfile.NamedTemporaryFile(suffix='.obj', delete=False) as t:
        t.write(obj_bytes)
        path = t.name
    m = omflink.Module(path)
    os.unlink(path)
    segs = {}
    for si in range(1, len(m.segs)):
        name, length = m.segs[si]
        buf = bytearray(length)
        for s, off, blob in m.data:
            if s == si:
                buf[off:off + len(blob)] = blob
        fx = [(f[1], f[2], f[3], f[5], m.exts[f[5][1]] if f[5][0] == 2 else None) for f in m.fixups if f[0] == si]
        segs[name] = (bytes(buf), fx)
    return segs, m


def disasm(code, fixups=()):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
    fxat = {f[0]: f for f in fixups}
    lines = []
    for i in md.disasm(code, 0):
        note = ''
        for k in range(i.address, i.address + i.size):
            if k in fxat:
                note = '  <fixup %s>' % (fxat[k][4] or str(fxat[k][3]))
        lines.append('%04x  %-16s %s %s%s' % (i.address, i.bytes.hex(), i.mnemonic, i.op_str, note))
    return '\n'.join(lines)


if __name__ == '__main__':
    cmd = sys.argv[1]
    if cmd == 'compile':
        src = sys.argv[2]
        flags = sys.argv[3:] or None
        obj, out, asm = compile_c(src, flags)
        print(out.strip())
        if obj:
            open(os.path.splitext(src)[0] + '.obj', 'wb').write(obj)
            segs, _ = obj_code(obj)
            for n, (b, fx) in segs.items():
                if b:
                    print('== segment', n, len(b), 'bytes')
                    print(disasm(b, fx))
    elif cmd == 'dis':
        segs, _ = obj_code(open(sys.argv[2], 'rb').read())
        for n, (b, fx) in segs.items():
            print('== segment', n, len(b), 'bytes')
            print(disasm(b, fx))
