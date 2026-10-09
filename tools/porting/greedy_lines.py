import sys, re, subprocess, os
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0,'/home/claude/darklands-re/build/agent/e1')
from getf import getf
P=os.path.dirname(os.path.abspath(__file__))
src='/home/claude/darklands-re/src/wip/s030/overland.c'; func='sub_s030_1462_02c0'
s=open(src).read(); f=getf(src,func)
lines=f.split('\n')
pat=re.compile(r'^(\s+)((?:case [^:]+: )*case [^:]+:|default:) (return [^;]+;)$')
start=next(i for i,l in enumerate(lines) if 'switch (D_d843)' in l)
pat2=re.compile(r'^(\s+)((?:case [^:]+: )*case [^:]+:|default:)$')
cands=[i for i,l in enumerate(lines) if i>start and (pat.match(l) or (pat2.match(l) and i+1<len(lines) and re.match(r'^\s+return [^;]+;$', lines[i+1])))]
print(len(cands), 'candidates', flush=True)
def build(ls):
    return s.replace(f, '\n'.join(x for x in ls if x is not None))
def score(ls, j):
    t=os.path.join(P,'gr%d.c'%j); open(t,'w').write(build(ls))
    out=subprocess.run(['python3','-I',os.path.join(P,'fdskip.py'),t,'02c0'],capture_output=True,text=True).stdout
    return out.count('!!')
def toggle(ls,i):
    ls=list(ls)
    m=pat.match(ls[i]) if ls[i] else None
    if m:
        ind=m.group(1); ls[i]=ind+m.group(2)+'\n'+ind+'    '+m.group(3); return ls
    m=pat2.match(ls[i]) if ls[i] else None
    if m and ls[i+1] is not None:
        ls[i]=ls[i]+' '+ls[i+1].strip(); ls[i+1]=None; return ls
    return ls
cur=list(lines); best=score(cur,0); print('base',best,flush=True)
B=4
for k in range(0,len(cands),B):
    batch=cands[k:k+B]
    with ThreadPoolExecutor(B) as ex:
        res=list(ex.map(lambda a: (a[1], score(toggle(cur,a[1]), a[0])), enumerate(batch)))
    for i,sc in res:
        if sc < best:
            nxt=toggle(cur,i); nb=score(nxt,0)
            if nb < best:
                cur=nxt; best=nb; print('take',i,sc,'->',best,flush=True)
open(os.path.join(P,'greedy_best.c'),'w').write(build(cur)); print('final',best)
