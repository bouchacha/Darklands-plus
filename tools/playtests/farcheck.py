#!/usr/bin/env python3
"""Find declarations like 'extern T far A, B;' where B (an F_<seg>_<off> far-segment name) lost the
'far': in MSC 'far' belongs to the declarator, so B is a NEAR (DGROUP) object and every access
to it reads DS:<off> instead of <seg>:<off>.   farcheck.py [FILES]  (default: src/wip and src/c)"""
import re, glob, sys
files = sys.argv[1:] or sorted(glob.glob('/home/claude/darklands-re/src/wip/**/*.c', recursive=True) +
                               glob.glob('/home/claude/darklands-re/src/c/**/*.c', recursive=True))
for p in files:
    t = open(p, encoding='latin-1').read()
    t = re.sub(r'/\*.*?\*/', lambda m: ' ' * len(m.group(0)) if '\n' not in m.group(0) else '\n' * m.group(0).count('\n'), t, flags=re.S)
    for m in re.finditer(r'^(?:extern|static)?[^;(){}]*?(?:\{[^{}]*\})?[^;(){}]*;', t, flags=re.M):
        s = m.group(0)
        if 'far' not in s: continue
        body = re.sub(r'\{[^{}]*\}', '{}', s)
        parts = body.split(',')
        for d in parts[1:]:
            if re.search(r'\bF_[0-9a-f]{4}_', d) and 'far' not in d:
                print('%s:%d: %s' % (p, t[:m.start()].count('\n') + 1, ' '.join(s.split())[:160])); break
