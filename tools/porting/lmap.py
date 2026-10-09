import sys; sys.path.insert(0,'/home/claude/darklands-re/tools')
import msc, re
src=sys.argv[1]; funcs=sys.argv[2:] 
fl=['/AM','/G2','/Gs','/Ox','/Zp1']
m0=re.search(r'/\* flags: ([^*]*)\*/', open(src).read())
if m0: fl=m0.group(1).split()
obj,out,asm=msc.compile_c(src,fl,keep_asm=True)
if asm is None: print(out); sys.exit(1)
cur=None
for l in asm.split('\n'):
    m=re.match(r'_(\w+)\s+PROC',l)
    if m: cur=m.group(1); continue
    if cur and (not funcs or any(f in cur for f in funcs)) and (l.startswith(';\t') or 'enter' in l):
        print(cur, l.strip())
