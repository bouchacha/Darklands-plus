# Porting notes from the matching agents

Raw notes collected while porting (rounds 3+). The most important points are summarised in PORTING.md; helper scripts are in tools/porting/ (they may need their paths adjusted).

## r3w0

- `push bp; mov bp,sp ... leave` in a function that never touches [bp+x] and has no locals:
  the function still takes a (unused) parameter, e.g. `int far f(int p_6)`; the dec16 stub
  `f(void)` drops the frame.
- Switch/if arms that share a tail (`push 0; push 4; jmp L` back into an earlier arm): write
  every arm out in full with `break;` (no goto, no `default: return;`); MSC cross-jumps the
  later arm into the first one. With a goto to a label in the first arm MSC puts the shared
  code at the end instead (and uses `add sp,N` rather than `mov sp,bp`).
- Sums of several calls (`(f1() + (f2() + (f3() + f4()))) / 5`): MSC reorders the operands of
  the commutative chain by *symbol-table position* of the called functions, the source order of
  the operands does not matter at all. Fix by permuting the prototypes and inserting unused
  `extern int X_n;` lines before them (s089_197f: 4 dummies + order 1f3b,05ee,1a7e,01a0 gave
  the original 05ee,1a7e,1f3b,01a0). Brute-force helper: build/agent/r3w0/multi.py with a
  candidate file (see c6.py): SUBS = [(old_text, [candidates...]), ...], tries the product and
  prints the call order of each candidate.
- UNSOLVED (s109_1a52_0284, also seen in s050/s053/s056_19c4 asm): party loop
  `sub si,si; cmp [a67e],si; jle; mov di,9c69h; L: ...; add di,80h; inc si; cmp si,[a67e]; jl L`
  (bottom test `3B` cmp si,mem / jl). Every for/while/do/goto form tried under 6.00A gives
  `cmp [a67e],si; jg` at the bottom (as in matched s085_18a9). Possibly a 6.00 vs 6.00A difference.
- Entry function with `enter 6,0` but only [bp-6] used (callback result): there is a local
  assigned a constant and passed later, e.g. `int mode; ... mode = 0; ... sub_s043_150b_049a(mode);`
  (the call still shows `push 0`, but the local keeps its 2 stack bytes). An unused local does not.
- `push [bx-11b5h]; mov ax,[ee6c]; mov [ee8c],ax; push ax; call` is
  `f(D_ee8c = D_ee6c, D_ee4b[D_ee6a])` (assignment inside the argument).
- Statement-level `x = 0 or k ...; return 0;` at the end of an `int` function whose last store
  is a constant: MSC hoists the constant store above the `sub ax,ax` group. If the original ends
  with the store and AX is just leftover 0, drop the `return 0;` (fall off the end).
- UNSOLVED (s124_199d_000c tail): `mov dx,14Fh / ... mov dx,[e48a] / ... mov dx,150h / mov ax,dx /
  mov [e48a],dx` = `return D_e48a = c1 ? 0x14f : c2 < 10 ? D_e48a : 0x150;` in 6.00. 6.00A
  turns that into per-arm stores; with `(...) + 0` it keeps the ternary but uses SI and puts the
  0x150 arm first.
- Global kept in a register through a call-free loop and written back once
  (`mov bx,[g]; L: inc bx; ...; je L; ...; mov [g],bx`): plain C on the global
  (`while (!D_9c69[D_ee8c % 5].f_00) D_ee8c++; r.b = D_ee8c %= 5;`) wrapped in
  `#pragma optimize("a", on)` ... `#pragma optimize("", on)` (s133_1838_0424). Without /Oa MSC
  does `inc word ptr [g]`.
- Struct-returning callback whose epilogue is `mov ax,[bp-4]` with DX left from a computation (no
  `mov dx,[bp-2]`): the last statement is `r.b = <expr>` with the value already in DX, e.g.
  `r.b = D_ee8c %= 5;` (idiv leaves the remainder in DX).
- `shl bx,7; [bx-6397h]` for D_9c69 -> declare `struct member { char f_00; char _p01[0x7f]; };
  extern struct member D_9c69[];` (dec16 sometimes emits `extern char D_9c69[]`, which breaks).
- **Compare orientation also depends on symbol-table position**: loop bottom test
  `cmp [a67e],si; jg` vs `cmp si,[a67e]; jl` (and probably other reg/mem compares) flips when
  an unused `extern int X_0;` is inserted right before `extern int D_a67e;` (s051_1967_03dc).
  Source operand order (`v_si < D_a67e` vs `D_a67e > v_si`) and for/while/do shape do NOT matter.
- SOLVED the party-loop `cmp si,[a67e]; jl` (s109_1a52_0284): move `extern int D_a67e;` around with
  build/agent/r3w2/declpos.py (position near `extern char D_ee9c[];` worked). Same scan on a call's
  prototype fixed `f1()+f2()` order (s109_1a52_0a10: inline `g(1,0x63,f1(-1,10)+f2(-1,5))`).
- s074_18f6_0294: memset right after `or ax,ax; jne` (AX known 0, no `sub ax,ax`) is NOT rewritten
  to `rep stosw` by cport.compat600 (it requires a preceding sub ax,ax / mov ax,-1). Tool limitation.
- Struct-return callbacks computing a sum of calls (`r.a = f1()/3 + f2()*3/4 + f3()*3/4; r.b = D_907b;`):
  write r.a FIRST, directly (no v_si temp, no `r.b` before it). Assigning via a temp after `r.b = ...`
  reverses the order the terms are evaluated after the calls (s134_18f9_04f4).
- Arg computations dec16 splits into `v_si = c70(4); v_si += vec()/3; v_si++; v_si = c70(3); ...` are
  inline args of the 2278 call: `sub_s029_0e76_2278(k, D_907e, 0xb, c70(3) + vec_1c1b()/4 + 3, c70(4) + vec_1c1b()/3 + 1, ...)`.
  Also nested: `sub_s029_1367_0066(vec_2161(), 7, c70(4) + vec_1c1b()/3 + 1)` as an arg; `vec_1c1b() > 5 ? 1 : 0x12`.
- `push 0EEA2h / jmp L / push 0EE9Ch / L: push 906Bh; call 02d4` = full if/else arms each with
  `if (sub_02d4(D_906b, D_eea2/D_ee9c) == -1) D_ee7c = 2;` (MSC cross-jumps).
- Far item data (slots 81CA and 81CC both seg 2e38 = two externs): `struct fchar far F_2e38_0000[]`
  (0x180-byte char record, items at 0x3e, 6 bytes each, see src/c/s035/cache.c) and
  `struct itemdef far * far F_2e38_0890[]`; `cmp es:[bx+20h],400h; cmp es:[bx+22h],0` = `->flags == 0x400L` (long).
- **SOLVED `cmp si,1` vs `dec si`** (value dead after the compare): add a redundant test that keeps it
  live: `if (v_si == 1 && v_si) {` -> `cmp si,1; jne` (s053_1838_0874). Probably works for `cmp dx,1` too.
- `mov dx,[g]; or dx,dx; ...; sub ax,dx` / `mov dx,[g]; cmp dx,K; jne; mov ax,dx` (global CSE'd in DX across
  inline strcpy / for a `return g;`): plain C on the global inside `#pragma optimize("a", on)`
  (s048_1a88 x4: `r.b=-2; if (!D_5856) {strcpy..; r.a=-1;} else r.a = 0x64 - D_5856;`; s053_1838_0000 `return D_e7d8;`).
- Party loop bottom `cmp si,[a67e]; jl`: in both s109_1a52 and s075_19be the working position for
  `extern int D_a67e;` was right before `extern char D_ee9c[];` (after the D_ee8x block). Try that first.
- `long` returning helper `sub_s029_1367_00f2` (or dx,dx; jg; jl; cmp ax,K; ja): declare
  `long far sub_s029_1367_00f2();` and write `if (f(D_906b) <= 0L) ... else if (f(D_906b) <= 0xcL)`.
- MSC substitutes constants with a register known to hold that value (`add sp,si` when si==2,
  `mov cx,si; idiv cx` for `% 5` when si==5 in that branch). dec16 then prints `% v_si`, `f(.., v_si, ..)`:
  write the literal constant instead (s063_1838_0500).
- `v_si = D_907b; v_si = f(v_si,5) + g(v_si,0xa)/2 + D_ee74;` reproduces `mov si,ax` deferred after the
  next call's pushes (old si pushed as arg first) (s063/s066 _032c).
- Comparison of call result vs sum of calls (`push 64h; call c70; push 10h; mov si,ax; ...; add di,ax; cmp di,si`):
  `if (c70(0x64) <= f(0x10) + f(0xf))` with no variables.
- Entry function frame 2 bytes too big, orphan slot at [bp-2] never written: a user temp whose value
  stays in AX, e.g. `if (f(-2, i) && (k = F[i]->f26 * F[i]->f27) > bestv) { bestv = k; best = i; }`
  (s107_1838_0000). The `k = 0xfc; if (D_e48a == k)` trick instead emitted a real store here.
- Early-exit `if (!f()) { g(); return 0xc; }` whose block sits at the END of the function in the
  original: write `if (f()) { ...whole body...; return D_e48a; } g(); return 0xc;`.
- `_fstrcpy(F_2e38_1a96, (char far *)"gold coins")` - cast the literal so `push ds` is emitted.
- WARNING for helper scripts: `s.index("void far X")` finds the PROTOTYPE first; always anchor on
  `"X(void)\n{"` or the body, otherwise text gets duplicated/deleted (fmatch can still report FILE MATCH
  with a missing function only if it's the last one, but check function counts).
- UNSOLVED (s047_1852_0476, same family as s124_199d_000c tail): if-chain/ternary whose arms leave the
  value in AX (`call; add ax,32h; jmp L` ... `sub ax,ax; L: push ax; call 000a`). 6.00A always puts the
  ternary temp in SI (or a stack slot with optimize("e",off)); an if-chain into r.a stores per arm.
- Card entry loops where dec16 put the D_e99c[...] stores inside each `switch (D_8dee)` case: write the
  switch with full cases + break and the stores once after it (s047_1852_0006).
- `mov di,[bp+6]` then `cmp si,di` twice: the source compares with the PARAMETER directly
  (`if (v_si > p_6 && ...) ...; if (v_si > p_6)`), MSC enregisters p_6. A `v_di = p_6;` copy flips the compare.
- `mov di,[bp-2]` with [bp-2] never written = the register variable read uninitialised on one path
  (the var's stack home): `if (c) {...} else if (p_6 >= v_si) v_di = 1; else v_di = 0; f(.., v_di, ..);` (s074_1838_0376).
- Local 6-byte buffer passed by address (`mov [bp-6],5; sub ax,ax; mov [bp-4],ax; mov [bp-2],ax; lea ax,[bp-6]`):
  `int l[3]; l[0] = 5; l[1] = 0; l[2] = 0; f(D_906b, l);` (separate ints lose the stores) (s115_1901_0370).
- Wrong far-slot number (`mov es,[81e4]` vs `[81e0]`) in a later function: slots are allocated in order of
  first reference in the file - fix the earlier functions (stub entry function) first.
- Module-private initialised data in this module's _DATA (`push 5EECh`, 6 zero bytes after the literals):
  `static int D_5eec[3] = {0};` at the top of the file (s074_1838_0000). A tentative `static int x[3];`
  before it gives "unsupported non-empty segment _BSS".
- **reg/reg compare orientation (`cmp di,si` vs `cmp si,di`) follows LOCAL declaration order**: swapping
  `int v_di; int v_si;` to `int v_si; int v_di;` flipped it (s090_1a66_0726). Try this before anything else.
- UNSOLVED s082_1845_0152: `mov si,ax; or ax,ax; jne REST; <case 0 body inline>; REST: dec ax ... ranges`
  = switch whose case-0 body 6.00 placed inline after an inverted test; 6.00A always emits the whole
  dispatch first (switch, if/else-switch and case-order variants tried). Same family as r3w1 s115_19bf_0222.
- Party min-search loops `v_si = 0x64; for (i...) { t = 01a0(i,0xf); if (t < v_si) { v_si = t; D_ee8c = i; } v_si += K; }`:
  frame `enter 8` = r + t's home + one more: copy s050_18fc_03e8 (`int t; int u;` + `#pragma optimize("a", on)`).
  Variant where 6.00 spilled the minimum to [bp-2] inside the loop (enter 6): plain `int t;` without u/pragma (s049_19f7_0c82).
- UNSOLVED s049_19f7_0002 (entry): 6.00 keeps two locals in CX with write-through stores
  (`mov [bp-2],cx; cmp cx,4; jge; mov cx,4; mov [bp-2],cx`) and puts one arm of `amt > 0 ? amt : 0x30`
  out of line after the loop. `int l[1]` arrays get the memory homes right but not the CX caching.
- Split identical case bodies (`case 2: X; break; case 3: X; break;`) for dispatch `dec ax; je A; dec ax; je A`
  (vs `case 2: case 3:` which gives `jl/jle` range tests) (s115_1901_025c, s049_19f7_07b0/08f2).
- Repeated `F_2e38_10be[D_907e]->field = K` stores reload es:bx each time in 6.00A; wrap the function in
  `#pragma optimize("a", on)` to keep es:bx (s105_1838_035e).
- Switch on a call result picked by another switch (`switch (p_6) { default: v=f(0x7e,...); break; case 0: v=f(0x7b,...); ...}`
  then `switch (v) {...}`): write each arm in full; MSC cross-jumps the shared call (s105_1838_035e).
- Adding prototypes (e.g. while filling a stub) shifts symbol positions and can re-break call-order fixes
  elsewhere in the file: re-run the declpos scan for those functions at the end (s105_1838_0f3e).
- UNSOLVED s105_1838_096c: 2180 result should be in SI and the saved D_ee8c in DI; 6.00A swaps them
  whatever the declaration order / register keyword / using p_6 as the saved var.
- Operand order of `a + b + c + GLOBAL` (when the global is added before a stack local) also follows the
  global's `extern` declaration position: declpos on `extern int D_ee74;` fixed s056_19c4_0a38.
- `push 0; push 3; push dx; push ax; call _aFldiv` = long division of a `long` call result:
  declare `long far sub_s029_1367_00f2();` and write `v = sub_s029_1367_00f2(D_906b) / 3;`.
- `add si,ax; mov ax,si; mov cx,3; cwd; idiv cx; mov [bp-4],ax` (SI kept, then copied): `v_si = f1(); v_si += f2();
  v_si /= 3; r.a = v_si;` (s084_18db_04e4). `r.a = v_si / 3` gives `add ax,si`.
- `mov di,[e7d8]; <push D_ee4b[D_ee6a]>; mov ax,[ee6c]; mov [ee8c],ax; push ax; call 2180` under optimize("a"):
  `D_ee8c = D_ee6c; v_di = D_e7d8; v_si = f(D_ee8c, D_ee4b[D_ee6a]);` (/Oa forwards the store into the arg).
  The in-arg assignment with `v_di = D_e7d8;` before it flips the push order (s107_1838_0798).
- `mov si,ax (call1); call2; push..; mov di,ax; call3; add si,di; add ax,si` = one flat sum of three calls with NO
  variable: `(f1() + f2() + f3() - 0x46) / 3`; then fix the call order by prototype position (s090_1a66_045a:
  vec_1f45 moved up among the D_4fxx externs). Outcomes of declpos are periodic (~17 lines), so scan every line.
- Random multi-decl position search helper: build/agent/r3w0/dsearch.py (SCAN=1 env = scan positions in order; vtry.py = try function variants separated by ====; dpos.py = parallel declpos) = move K decl lines to random top-level
  positions, check several functions at once (`only=[...]`), stop at match. Check the WHOLE file afterwards.
- Ternary arg kept in SI (`cmp [g],K; jg; mov si,A; jmp; mov si,B; push si` after other pushes): if/else
  statement into the register var before the call, `if (g <= K) v_si = A; else v_si = B; f(.., v_si, ..);`
  (the `>`-form if or a real ternary give AX / inverted branch) (s074_1838_0734).
- When adding externs/prototypes for a newly written stub breaks call/operand order in other functions, put the new
  decl lines at the END of the top-level declarations (just before the first function body): then the earlier
  symbol positions are untouched (s132_186c). Then position-scan only what the new function needs.
- Party loop with the counter in SI and pointer in DI (`sub si,si; ...; mov di,9C69h`): wrap the function in
  `#pragma optimize("a", on)`; without it 6.00A swaps (counter DI, pointer SI) (s132_186c_02fc).
- Entry functions `(*D_ec00[0][D_ee6a])()` = `call [bx-1400h]` (1-d use of the 2-d callback table).
- Entry fn sum kept in DI (`add di,si; push 64h; call c70; cmp ax,di`) but 6.00A spills v_di to [bp-2]: wrap the
  entry function in `#pragma optimize("a", on)` (s108_1a3f_0006).
- Party count loop with counter in DX and D_a67e cached in CX (`sub di,di; mov dx,di; mov cx,[a67e]; or cx,cx; jle;
  mov bx,9C69h ... inc dx; cmp dx,cx; jl`): plain `int i;` loop (3rd local, not a register var) +
  `#pragma optimize("a", on)`; init order `for (i = 0, v_di = 0; ...)` gives `sub di,di; mov dx,di`
  (`v_di = 0; for (i = 0;` gives the reverse) (s108_1a3f_032a).
- `cmp [bp+6],1; sbb ax,ax; add ax,30h` (no `and`) = `p_6 ? 0x30 : 0x2f`.
- s056_19c4_0514 SOLVED: an arm that in the original does `call 03b0; jmp END` instead of being cross-jumped into the
  shared `call 03b0` had its own full copy of the function tail (`...; return D_e48a;`) in the source.
- s056_19c4_0354 PARTIAL (written, not matching): flag in [bp-2] (`int l_2[1]`), switch on 07ee() whose arms
  `cmp di,K; jge CASE2BODY` jump into case 2's `l_2=1` - goto-to-label-in-case-2 gives that layout but then 6.00A
  swaps SI/DI (1a8e result gets SI); the if/else forms keep regs right but cross-jump the other way. 0008 still a stub.
- Param kept in DI and the local in SI (`mov di,[bp+6]` ... `mov di,ax; ...; add di,ax; cmp di,si`): dec16 split
  `p_6 = f(); p_6 += g()/10;` must be ONE statement `p_6 = f() + g() / 10;` - the split form makes MSC give SI to p_6 (s054_19df_05a8/0888).
- `v_si = f(2) * 3 + (unsigned)f(0x21) % 0xb + 0x14;` as ONE statement (dec16 splits it in three, which moves the
  `push 0` arg and keeps the sum in DX) (s054_19df_0f88).
- dec16 `((F << 1) + F << 1) + F` / `(F << 1) + F` with F a call duplicated = `F * 7` / `F * 3` (mov cx,ax; shl; add).
- UNSOLVED s054_19df_0298: arm `if (v_si < 0x32) { v_si = p_6; 00a4(..); 0db4(..); if (v_si) 03b0(v_si,0); ...}`: original
  loads `mov si,[bp+6]` at the top of the arm, 6.00A sinks it below the two calls (pragmas a/g/c/l, register, dummy stmt tried).
- SOLVED s047_1852_0476 (ternary chain left in AX, no SI): `r.a = sub_000a(D_8dee == 0 ? f(D_ee8c, 0xe) :
  D_8dee == 1 ? f(D_ee8c, 0xe) + 0x32 : D_8dee == 3 ? f(D_ee8c, 0xe) - 0x32 : 0);` - the arg must be the GLOBAL
  just assigned (`D_ee8c`, MSC reuses AX), not `(unsigned char)D_991d`; with the cast form 6.00A uses SI.
- SOLVED s105_1838_096c (2180 result in SI, saved D_ee8c in DI were swapped): `#pragma optimize("a", on)` around the
  function + `if (v_si == 1)` instead of dec16's `v_si--; if (!v_si)` (still compiles to `dec si; je`).
  General: when 6.00A swaps SI/DI between two register vars, try optimize("a") first.

## r3w1

- **Zero shared through AX before a 1:0 ternary.** `sub ax,ax; mov [ee8c],ax; cmp byte [9ce9],1; jne L; mov ax,1; L: mov [ee8e],ax`
  comes from an *equal-arm ternary* first: `D_ee8c = D_9c69 == 1 ? 0 : 0; D_ee8e = D_9ce9 == 1 ? 1 : 0;`
  (per-party-member macro `x[i] = member[i].flag == 1 ? i : 0` with i = 0). Plain `D_ee8c = 0;` gives `mov [ee8c],0`.
- **Phantom 2-byte stack slot.** A separate register variable copied from a local (`v_di = l_2;` before a loop) gets its
  own (unused) stack slot -> frame 2 bytes too big. Using the local directly lets MSC hoist the load into DI itself, but 6.00A
  then enregisters the local over a larger region (loads at branch ends, `mov di,0xc` instead of `mov [bp-2],0xc`).
  An array/struct-wrapped local (`int l_2[1]`) is never enregistered nor hoisted. (s081_1962_000c unresolved.)
- **Block placed between `default:` and the first case.** Layout `jmp table; default-body; jmp END; <block X ending in return>; case bodies...; END: cmp [bp-2],1; jne B; jmp X` comes from a goto into the switch body: `switch (f()) { default: v = 6; break; L_X: ...; return; case 0: ... } if (flag == 1) goto L_X;` (s056_1abb_0212). Case bodies are laid out in value order regardless of source order; default goes where it is in the source.
- **`dec dx` vs `cmp dx,1`.** `t = g; if (!t) A else if (t == 1) B else C` gives `dec dx` (t dead after compare); original 6.00 sometimes kept `cmp dx,1`. Unresolved (s056_1abb_03d8).
- **`push bp; mov bp,sp` frame with no locals and no [bp] use** (s106_19bc_049e, `t = D_e7d8; if (!t) .. else if (t == 1)` with `cmp dx,1` not `dec dx`): not reproduced. A jump-table `switch` in a function forces such a frame (removing it from s056_1838_02ba removes the frame); an unused `int p_6` parameter does not.
- **`sub ax,ax; push ax` for a lone constant-0 argument**: `f(0xb, D_e48a ? 0 : 0);` (equal-arm ternary, as in s110_1910_024a). Also check for unreferenced functions hidden in `db` bytes between functions (s110_19fd_0240): add them to the C with their address name.
- **Operand evaluation order `(f(0x10) + f(0xf))`**: both source orders compiled f(0xf) first; ONE extra unused `extern int D_ff00;` right after `struct res` at the top of the file flipped it (s115_19bf_03d6). Externs placed just before the function did nothing.
- **6.00A moves a small case body out of line** (after an unconditional `jmp` in the tail) so the previous case can fall through to the code after the switch; the original keeps `jmp END` + body in place (s115_19bf_0222, unresolved).
- **Split case ranges.** Dispatch `or ax,ax; jl D; dec ax; jle C; dec ax; jl D; dec ax; jg D; C:` = two label groups `case 0: case 1:` and `case 2: case 3:` with *identical bodies* (cross-jumped into one). `case 0: case 1: case 2: case 3:` gives `sub ax,3` instead (s126_1838_04e0).
- **Nested ternary into a global through DX** (`mov dx,K / mov dx,[g] / mov dx,K2; mov ax,dx; mov [g],dx`): 6.00A turns `return g = c ? K : (d ? g : K2);` into per-arm stores; a temp var goes to SI. Unresolved (s126_1838_0000).
- **Struct return order**: `mov ax,0x64; sub ax,[57bc]; mov [bp-2],-2; mov dx,[bp-2]` = `r.a = 0x64 - D_57bc; r.b = -2; return r;` (r.a first). s046_1838.
- **Frame with `mov sp,bp` cleanups but no [bp] access** (s046_1838_057e: `f(0x64); g(0); D_e48a = h() ? 0x1d : 0x1e;`): sibling functions are `if (f(0x64) >= p_6 && D_57c8) {...}`. 6.00A drops dead param refs / dead locals / `&& 0` without keeping the frame. Unresolved.
- **Min-over-party loop with frame 4 bytes too big** (`enter 8,0`, result at [bp-6]; `call f; cmp ax,si; jge; mov si,ax; add si,19h`):
  `t = f(i, 0xf); if (t < best) best = t; best += 0x19;` inside `#pragma optimize("a", on)`. Without /Oa 6.00A splits `best`
  between SI and a stack home. (s050_18fc_03e8 and 6 siblings)
- **`call; add sp; push NEXTARG; mov si,ax`** (result move scheduled after the next call's push) = the calls are all
  operands of ONE expression; MSC evaluates them right-to-left into si/di/[bp-x]. dec16 splits them into statements.
  `(unsigned)(f(0x21)+0x14) % 0x15 + g(4)/0x14 + f(0)/2 + h(5) < 0x64` (s049_1b1a_0374).
- **`mov di,[bp+6]` right after the prologue** (param loaded into DI before anything else): declare the parameter
  `register int p_6` and use it directly (a `v_di = p_6;` copy loads late). (s062_19fe_034c, s065_18fb_0352)
- **Evaluation order of call operands fixed by moving prototypes**: build/agent/r3w2/declpos.py worked for
  `f1a7e(0) + (f01a0(c,10) + f05ee(c,5))` (s062: move 05ee decl; s065: move 05ee then 01a0).
- **Jump-table diffs only (`jmp cs:[bx+T]` / table words off by N)** are just a shift from an earlier function of the
  file with a different length; they go away when the earlier function matches.
- **Min of two calls returned in AX** (`mov si,ax; call; cmp ax,si; jle; mov ax,si; mov [bp-2],-2; mov dx,[bp-2]`):
  `v_si = f(0xf); t = f(0x10); r.a = t > v_si ? v_si : t; r.b = -2; return r;` (s062_19fe_0b78)
- **Shared tail `mov [a],ax; mov [b],ax` reached by `jmp` from an earlier arm with AX=0**: write the zero stores in
  both arms (`if (!f()) { x=0; y=0; a=0; b=0; } else { ...; if (g()==2) { a=0; b=0; } }`) (s062_19fe_0008).
- **`sub di,di; mov si,di`** = `v_di = v_si = 0;` (or `v_si = 0; v_di = 0;`); `v_si = v_di = 0;` gives the reverse (s100_18f6_0000).
- **`or ax,ax; je C0; dec ax; je C1; dec ax; je C2; C0:`** (explicit case 0 that equals default): write `default: v=2; break; case 0: v=2; break; ...` with duplicated bodies (s089_1838_0000).
- **SOLVED the party-loop bottom test `cmp si,[a67e]; jl`** (vs 6.00A's `cmp [a67e],si; jg`): move the line
  `extern int D_a67e;` with build/agent/r3w2/declpos.py FILE _func "extern int D_a67e;" 1 — it flips at specific
  positions (s089_1838: positions 15, 32, 51, 68 = every ~17-19 lines). Check the whole file at those positions.
- **Order of two calls of the SAME function in a sum** (`f(0xe) + f(0x10)`): also moved by the prototype position of f (declpos.py) (s048_1838_08c0).
- **Dead computation kept** (sum computed into si, never used, then `call 1a8e` with result ignored): the source had
  `v += 0x14` under `if (f1a8e(..) & 0x40)` with v otherwise unused; MSC deleted the test/add but kept the computation.
  Write the full sibling-style code: `v = (...)/10 + D_ee74; v += g()*5; if ((char)f(..) & 0x40) v += 0x14;` (s076_1901_03dc).
- **Frameless switch functions called with an argument** (no push bp; `retf` directly in dispatch; dec16 emits
  `int f(void)` + bogus `case -0x40..` labels): `void far f() { switch (g(...)) { case 0: ...; break; case 1: case 2: ...; break; } }`
  (K&R `()` so callers can still pass args), tails written in full in every arm (MSC cross-jumps them). (s070_18c8_0324, s117_18e0_0296)
- **int function ending with constant stores + `return K;`** -> drop the return (fall off end) so stores stay in source order (s117_18e0_038a/0500, s070_18c8_0434).
- **One string literal shared by many calls** (data has "merchant" once, every case pushes the same address; dec16
  repeated the literal -> DATA DIFFERS): use a local pointer `char *m; m = "merchant";` and pass `(char far *)m`
  (MSC propagates the constant address). A `static char[]` gets word-aligned and shifts the data (s109_1838_069e).
- **Missing call arguments**: dec16 sometimes emits `f();` where the asm pushes args (`push 0; push 2; call 03b0`) - add them.
- **`push K` of a later constant argument hoisted before the computation of the first arg** + result kept in SI
  (`push 5ah; ...; add si,ax; push si; call 0084; mov si,ax; push si; call 000a`): two statements
  `v = A + B/2 + D_ee74; v = f0084(v, 0x5a); r.a = f000a(v);` (one expression for the sum; s067_1954_0330, s062_19fe_04b6).
- **`push 0; push 0Ah; ...; mov al,[991d]; sub ah,ah; mov [ee8c],ax; push ax`** = assignment inside the argument:
  `vec_1f63(D_ee8c = (unsigned char)D_991d, 0xe, 0, 0xa, 0)` (s067_1954_059a).
- **Look for twin functions in finished files** (grep a distinctive call like `vec_2067(-1, 0x7f)` in src/c): s064_1838_0508 = s064_1a37_0504 with constants changed (dec16 for(;;)/goto mess replaced by the if/else-if chain).
- **Register var spilled to a stack home under 6.00A** (`mov [bp-2],di; ...; cmp ax,[bp-2]`, frame 2 bytes too big) while the original keeps it in DI: wrap the function in `#pragma optimize("a", on)` (s108_1b3a_0008; also s050_18fc).
- **Orphan frame slot (`enter 2,0`, no [bp-2])** in a switch function: `int k; k = 5;` at the top and use k in a case body (`D_9c69[i][0] == k`) - assignment in a different basic block than the use (s108_1b3a_030c). Short party loop there used DX/CX: needs `#pragma optimize("a", on)` to hoist `mov cx,[a67e]`.
- **Global compared to several constants via DX** (`mov dx,[e7d8]; cmp dx,1 ... cmp dx,2 ... cmp dx,3`, even across calls): `#pragma optimize("a", on)` around the function, plain `if (D_e7d8 == 1) ... else if (D_e7d8 == 2 || D_e7d8 == 3)` (s106_18b5_0828/09fa/0bc6).
- **Far-table pointer stores, original keeps `D_907e*4` in a stack temp but reloads the far pointers each time**
  (`mov [bp-8],bx; les si,es:[bx+10be]; ...; mov es,[8294]; les si,es:[bx+10be]`; address `&tbl[i]` saved as far temp
  `[bp-0c]/[bp-0a]` and re-dereferenced): `#pragma optimize("a", on)` + access through a volatile view of the far struct
  `#define FV (*(struct T volatile far *)&F_2e38_0000)`. Same source with plain /Oa (no volatile) gives the
  CSE'd-pointer-value variant (`[bp-6]` holds the loaded pointer) - s133_1971_030e vs _04dc.
- **`mov [bp-2],si` dead store** of a register var under /Oa = the register var's own stack home (6.00A writes it too); do
  not model it as a struct field.
- **Jump-table group laid out out of value order** (`default, case1, {0,2,4}, case3, case5` in the code): write the
  group as one separate body + a label group: `case 0: X; break; case 2: case 4: X; break;` - MSC cross-jumps the
  case-0 copy into the later `{2,4}` copy, which is placed by its first value (2). (s082_1922_0000)
- **`mov [bp-2],ax; inc ax; jne` then `cmp word [bp-2],-1` / `push [bp-2]`** (6.00A: `cmp ax,0xffff` and keeps AX) for
  `t = f(); if (t == -1) t = g(); if (t == -1) t = g(); h(t);` with `&t` taken elsewhere: `#pragma optimize("a", on)`
  around the function (s082_1922_0484).
- **Ternary argument `push (c ? 5 : 0)` as branches** (`cmp; je; push 5; jmp; push 0; call`): `if (c) f(5); else f(0);`
  (6.00A turns the ternary into `cmp/cmc/sbb/and`). (s048_1838_0000)
- **Order of 3 calls in a sum** (`a + (b + c) + abs(g)`, all orders tried): only the prototype position matters;
  build/agent/r3w2/score.py based tools/porting/declpos.py found it (01a0 right after the struct F line; 1326 at line 11).
  NOTE: the scratchpad dir is shared with other workers - copy files under a unique subdir before running declpos on
  them; `pkill -f declpos` also kills your own shell.
- **dec16 goto/for(;;) mess with a shared tail reached by `jmp` backwards** (s107_191f_04ca): nested if/else with the
  common block written out in full in both arms; MSC cross-jumps the second copy into the first (`jmp 04d9`).
- **Switch nested in `if (v == 1)` with two-group dispatch** (s107_191f_06e0): `if (v == -1) return 0; if (v == 1)
  switch (g) { case 0: case 1: A; break; case 2: case 3: B+tail; break; } else C+tail;` (tails in full).
- **`mov cx,2; cwd; idiv cx` (division by constant 2 via idiv)**: `r.a /= 2;` on a struct field (s044_19dd_05da).
  `r.a = sum / 2; if (c) r.a /= 2; r.a = f(1, 0x63, r.a);` keeps everything in AX.
- **Local buffer passed to _fstrcpy** (`lea ax,[bp-14h]`, frame 0x14): `char buf[20];` not `int l_14;` (s044_19dd_061c).
- SOLVED `mov si,ax; inc si; mov ax,si; imul di; push ax`: `v_si = f()/3 + 1; v_si = v_si * v_di; g(.., v_si);`
  (assign the product back to the register var; same for `v = f()/3 + 1; v = D_a67e * v << 1;` in s073_1a7e_0000).
- Unresolved: s058_1838_0606: loop var gets a stack home `[bp-2]` (frame +2) in a loop that sets a
  flag local; original keeps it in SI and reuses SI for `v = t; if (ok) v = 0;`. s106_18b5_02b6: frame 2 bytes
  short (enter 4 vs 2) and case-0 tail cross-jumped the other way.
- **Found-flag loop with stack homes** (`mov [bp-2],0; cmp [a67e],0; jle X; mov si,[bp-2]` ... `X: mov di,[bp-4]`):
  plain `found = 0; ...; for (i = 0; i < D_a67e; i++) { found = f(i, 0x3b) >= 1 ? 1 : 0; if (found == 1) break; }`
  with `found`/`i` as ordinary locals - 6.00A gives the same homes (s073_1a7e_0000).
- **Struct return, value left in AX after the last store**: write `r.b = ...;` BEFORE `r.a = best;` (s073_1a7e_035e/044a).
- `int f(void) { ... ? 0x64 : 0 ... }` ternary into AX inverted by 6.00A: use if/else with `<= -10` first arm (s073_07da).
- Call result left on the stack before `leave` (no `add sp`): 6.00A also omits it; just write the plain expression.

## r3w2

- **Dead first store into a register var** (`call f; mov di,ax; mov cx,5; div cx` with di never read):
  the variable is reassigned later and that later store IS eliminated. `v = f(A); g(A,0,0,v/5); v = f(B); g(B,0,0,v/5);`
  -> first store kept in di, second dropped. (s086_1916_036c)
- **Frame bigger than used locals** (`enter 6,0`, only [bp-6] referenced; callback result at [bp-6] instead of [bp-4]):
  an int local whose only uses were constant-propagated away still gets a slot. `int l_2; ... l_2 = 0; ... sub_s043_150b_049a(l_2);`
  compiles to `push 0` but keeps the frame at 6. An unused (never-assigned) local is NOT allocated. (s113_1838_0000)
- **Function with no bp frame** (`push si` first, no `push bp/mov bp,sp`): write it as `(void)` with no params; the decompiler's
  `return;`-without-value in an `int` function with gotos produced a frame. Rewriting as structured if/switch fixed it. (s113_1838_02d4)
- **Sum of 4 calls** `f1()+f2()+vec_1f3b()+f4(a,b)`: MSC reassociates and always calls the zero-arg call last in our tries;
  original calls it third. Unsolved (s065_1838_04b4).
- **Evaluation order of two calls in one expression = position of the function's DECLARATION.** Moving the line
  `int far vec_2161();` to different places among the file's top-level declarations flips which call MSC evaluates
  first (`f(x)/25 + vec_2161()*6`). Scan positions with `build/agent/r3w2/declpos.py FILE _func "decl line" [step]`
  (prints diff count per position). Solved s106_1a56_000a (r2w0 had tried 60+ variants of extern/block tricks).
- **`x*6`** compiles to `mov cx,ax; shl ax,1; add ax,cx; shl ax,1` (dec16 decompiles it as two calls `(f()<<1)+f()<<1`).
- **`add ax,si; push ax` vs `add si,ax; push si`** for `a + (unsigned)b/25` passed as an int argument: write
  `a + (int)((unsigned)b / 25)` (the cast makes MSC keep the sum in AX). (s106_1a56_000a)
- **Function ending `mov ax,K; mov [g1],ax; mov [g2],ax; mov [g3],K2; retf`**: int function that falls off the end:
  `g1 = K; g2 = K; g3 = K2;` with no return (dec16's `return K;` moves the g3 store first). (s106_1a56_0414)
  -> SOLVED s065_1838_04b4 with a 2-D scan of the declaration positions of `vec_1f3b` and `sub_s029_0e76_01a0`
     (`build/agent/r3w2/declpos2.py FILE _func "decl1" "decl2" 5,14,22,...`). The effect is periodic in blocks of
     ~8-9 declaration lines, so sampling one position per block is enough.
- **`mov dx,[g]; cmp dx,3; jne; ... cmp dx,2; ... cmp dx,1`** (global loaded once into DX, compared with `cmp` not `dec`):
  plain `if (g == 3) ... else if (g == 2) ... else if (g == 1)` on the global, inside `#pragma optimize("a", on)`
  (no-alias CSE of the global). A local temp `t = g;` gives `dec dx` for the last compare instead. (s065_1838_0000)
- **bp frame with no locals/params used** (`push bp; mov bp,sp ... leave`): give the function an unused parameter
  `int far f(int p_6)` (it is called with one argument anyway). (s120_18cf_0388)
- **Call without stack cleanup before `leave`** (`call far X` directly followed by `mov ax,1`... `leave`): dec16 dropped the
  arguments (`f();`); look at the pushes before the call and write them back. (s120_18cf_05bc)
- **Switch on a call result stored in a register var** (`mov di,ax; or ax,ax; jl; dec ax; jle A; dec ax; jl; dec ax; dec ax; jle B`):
  `v_di = f(...); switch (v_di) { case 0: case 1: A; break; case 2: case 3: case 4: B; break; }` (di store kept even though
  unused afterwards). dec16 turns such range chains into bogus `case -0x40..` labels. (s126_199d_031a)
- **Struct return order**: `mov [bp-2],K` after the computation of r.a means `r.a = ...; r.b = K;` (dec16 sometimes swaps). (s126_199d_0c34)
- **User variable kept in a register still gets a stack home.** `l = (f()*2 + g())/2; if (l < best) ...` with l never
  stored still reserves a slot (e.g. [bp-2] of `enter 0xa,0`). So an unexplained unused word in the frame = a separate
  user variable whose value lives in a register. (s068_19f9_054a, s068_19f9_0004)
- **Party loop `cmp byte ptr [di],1 ... add di,80h`** = `extern char D_9c69[][0x80]; for (i = 0; i < D_a67e; i++) if (D_9c69[i][0] == 1)`.
- **Unsolved: store order of a local in a chained-constant block.** Original `mov ax,1; mov [bp-4],ax; mov [ee76],ax; ...`;
  MSC 6.00A always emits the local store after all the global stores (tried: position, chains, `D_x = l`, struct field,
  pragmas a/c/g/w/z). (s068_19f9_0004)
- **`push args; mov si,ax; call`** (store of the previous call result scheduled after the next call's pushes) = the value is
  a compiler temp of ONE big expression, not a user variable. E.g. `r.a = (f(16)+g(3)+g(4))/3 + g(5) + f(10);`. MSC
  canonicalizes the call order (source order of terms does not matter); fix the order with the declaration-position scan
  of the called function's prototype (declpos.py; now scores with score.py = edit distance). Scan all affected functions
  and pick a position that zeroes all of them. (s119_1988_05a4/0750/08c6)
- Scratchpad is shared with other workers: keep temp files in build/agent/r3w2/tmp/.
- **Param loaded into DI in the prologue (`mov di,[bp+6]`) and a dead sum also stored in DI**: write the param as a plain
  `int p` used directly (MSC enregisters it; no `v = p;` copy) and put the sum in a BLOCK-SCOPE local:
  `if (roll <= p) { int s; s = f() + g()/10; if (s >= roll) ...}`. The block local shares DI with the param and the
  function-level `roll` gets SI. (s050_1a06_026c/05b8)
- **`return;` in an int function** when AX already holds the value just stored (`mov [ee8c],ax; ... retf` without
  reload): `D_ee8c = D_907b; if (!p) { D_e48a = 1; return; }`.
- **Local kept on the stack although MSC would enregister it, with `push ax; push ax` of a known-zero value**:
  `#pragma optimize("e", off)` before the function (no register allocation) reproduces both; the 6.00 "constant in a
  register" pushes appear by themselves then. If a value is then reloaded from the stack after a branch
  (`mov bx,[bp-16h]` where AX still held it), add `#pragma optimize("a", on)` too. Restore with
  `#pragma optimize("", on)` after. (s061_18cf_06e6)
- **Order of two globals in a sum** (`mov ax,[4bb8]; add ax,[ee74]`) is fixed by the extern declaration positions,
  same scan as for calls (declpos.py on the `extern int D_xxxx;` line). (s061_18cf_0008/06b4)
- `char buf[N]` frame size off by 2: try N+2 (`enter 0xc` with buf at [bp-0xc] and one register var).
- **dec16 `goto L_xxx` into the middle of another switch case** (`case 3: ...; goto L;` where L is in case 2) is MSC
  cross-jumping: write each case in full (`f(0); D_e48a = 0x3a; break;`); MSC re-merges the tails. Also fixes a missing
  `push bp` frame in some functions. (s058_191a_01f4/09ba, s090_1838_0222)
- **Two identical early-return blocks** where the second test jumps BACK into the first block's body
  (`... jne L0416; L03fa: body; ret; L0416: test2; jne L03fa`) = two separate `if (c1) {body; return;} if (c2) {body; return;}`
  (not `if (c1 || c2)`). (s048_190e_03de/04ee/000e)
- **Wrong `&&`/`||` precedence from dec16**: `!a && !b || c` should often be `!a && (!b || c)` - check the jump targets.
- **6.00-style register reuse (`push cx; push cx` with cx=4 from a preceding `% 4`, `add sp, si` with si==2) DOES come
  out of 6.00A** when the source writes the plain constants (`f(D_907e, 4, 4)` after `x % 4`, `f(D_907e, 1, 2)` inside
  `if (v_si == 2 && ...)`). Do not try to reproduce it with variables. (s064_1a37_0504)
- **Merged tails of `if/else if` arms** (`jmp` into the middle of another arm's stores, skipping `sub ax,ax`): write all
  arms' assignments in full (`D_ee78 = 0; D_ee7a = 0; D_ee7c = 0;` / `D_ee7a = 0; D_ee7c = 0;`). (s064_1a37_0004)
- `vec_1f63(D_ee8c = (unsigned char)D_991d, ...)`: assignment inside the argument puts the store right before `push ax`.
- **`push bp; mov bp,sp ... mov sp,bp ... leave` around a call whose result is unused, no param reference visible**:
  the param is used in a comparison whose two arms are identical: `if (f(100) <= p) { X; } else { X; }` - MSC merges the
  arms and drops the compare but keeps the frame. An unused param alone does not create the frame. (s104_1838_08fc)
- **Bit-flag set/test on a far byte array** (`p` stored as far ptr at [bp-4], mask at [bp-6], no user vars):
  write it WITHOUT pointer/mask variables: `if (F_2e38_082c[k + v / 8] & 0x80 >> v % 8) ... else F_2e38_082c[k + v / 8] |= 0x80 >> v % 8;`
  - MSC's CSE temps go to the stack. (s114_1838_0566 etc.)
- **Source order of commutative call operands DOES matter** (in addition to declaration positions): for
  `f(x,10)/3 + f(x,9)/3 + f(x,8)/4 + g(x,3)/2` only some permutations give the original call order - brute-force
  all permutations with perm.py (one variant per stdin line, `\n` for newlines). (s114_1838_0356)
- `r.b = D_907b; r.a = h(..., f(r.b, ...) ...)`: the struct field is CSE'd into SI for the call args.
- **6-byte struct passed by value to vec_1f6d** (`push [bp-6]; push [bp-8]; push [bp-0a]` after byte stores):
  `struct s068_arg { int idx; char f1e; char rnd; char x; char f25; } a; a.idx = 0x44; a.f1e = F_2e38_0000.f_09a0[0x1e];
  a.rnd = ...; a.f25 = F_2e38_0000.f_09a0[0x25]; a.x = 1; vec_1f6d(D_907b, a);` (see also src/c/s101). (s068_1838)
- A value computed in SI (temp) and then stored to a stack local that is read back later (`add si,ax; mov [bp-2],si`)
  = one expression assigned to a non-register local: `l_2 = f() + (g() + 0x11) * 2;`.
- **Local forced to the stack while 2 others get SI/DI**: `volatile int l_2;` (s059_1838_02f2 - a switch-assigned value).
- **Frame 4 bytes larger than any visible local in an `int` function that returns a call result**: a `struct res r;`
  whose `.a` receives the result and `return r.a;` (r never touches memory, but reserves 4 bytes). (s048_190e_04ee)
- **MSC splits a register var's live range**: `best = 0x63; for (i...) { t = f(i); if (t < best) { g = i; best = t; } }`
  compiles with best spilled to a stack slot inside the loop and t sharing best's register (dec16 shows an extra `l_2`).
  Write the natural loop; the register choice (which of t/i gets SI vs DI) is weight-based and NOT affected by declaration
  order - unsolved in s074_1995_04b4 (si/di swapped).
- `x*3`, `x*5`, `x*7` come out of dec16 as `(f() << 1) + f()` etc. with duplicated calls; `(c - f()) * 5` shows up as
  `v = c; v -= f(); v <<= 2; v += v;` (wrong). Rewrite as the multiplication.
- **Compare operand order** (`cmp ax,cx` vs `cmp cx,ax`, `cmp si,[g]` vs `cmp [g],si`) also follows the declaration
  positions (source order of the operands does not matter): declpos.py on the called function's prototype or the
  global's extern line. (s048_19e9_0008)
- **Global loaded once into DX** before an unrelated store (`mov dx,[584a]; mov [bp-2],-2; or dx,dx`) and reused after
  a branch: `#pragma optimize("a", on)` around the function with the plain source (`r.b = -2; if (!D_584a) ... else
  r.a = 0x64 - D_584a;`). (s048_19e9_04ec x9)
- **Unsolved (round 3 w2, second session):**
  - s067_1869_04ba/0456, s065_1afe_04b4 (same template): original keeps a single-use register var
    (`mov si,ax; push si` for `v = f(); g(v);`); 6.00A always propagates it. Tried register, assignment-in-arg, pragmas.
  - s067_1869_080a / s065_1afe_0b3e: `r.a = min(f(15), f(16))` kept in AX (`cmp ax,si; jle; mov ax,si`), ours stores r.a.
  - s070_1ac1_0532: `sub si,si; push si` (a variable holding 0 pushed 3x); 6.00A constant-propagates.
  - s080_18a5_055a: `sum / 4` done with `mov cx,4; idiv cx` (no shift) - unknown source form.
  - s108_1925_05f2/086e: `D_907e % 4` remainder in DX reused across an `&&` block (`push dx`); ours recomputes.
  - s110_1838_0bb4/0c14: `if (c) r.a = 0x63; else r.a = f();` - orig stores the constant directly, 6.00A merges via AX.
  - s079_18d8_02f6: dead stores to stack locals kept by the original (l_8 = 0 ...) - looks like no dead-store elim.
  - s077_1838_0626: loop pointer temp gets DI before a user var (orig: user var in DI, pointer on stack).
  - s059_1838_0864: call order of `f1800(5)/10 + (f01c0(-1)/2 + f1326(4)/20)` - no decl position found.
  - s046_194b_0002: final `mov ax,dx; mov [e48a],dx` (ours `mov [e48a],ax`).
- Tools added: score.py (edit distance), perm.py (variants from stdin), declsearch.py (coordinate descent over decl
  positions for several functions; slow: run in background with nohup, logs in tmp/).
- **(session 3) Remainder reused across an `&&` (`push dx` of `D_907e % 4` after `cmp dx,[eeb4]; je`)**: the source passes the
  GLOBAL it was compared with: `if (D_907e % 4 == D_eeb4) { if (v >= 0x5a && !f(0x55, D_eeb4, D_907e)) ...` - MSC knows
  D_eeb4 == DX after the equal-compare. SOLVED s108_1925_05f2/086e.
- `cwd; xor ax,dx; sub ax,dx` = `abs()` intrinsic (declare `int abs(int);`). s070_1ac1_064e:
  `r.a = f0066(0, 0x63, D_9082 / 20 + (abs(D_ee74) / 2 + (f1600(0xf) / 3 + f01c0(D_907b) / 3)));` + declpos.
- Adding a new function shifts the symbol table: re-check ALL functions, re-run declsearch.py over the broken ones
  together (s108_1925: 0438 + 0aee fixed by moving one prototype).
- build/agent/r3w2/trys.py = try.py scored by score.py (edit distance) and with `\n` support in OLD/NEW.
- **Ternary branch sense** (`cmp ax,[tmp]; jge L_B; mov ax,A; jmp; L_B: mov ax,B`, A=0x10 B=0xf, ax = 2nd call):
  `f(i, 0x10) <= f(i, 0xf) ? 0xf : 0x10` gave it; `f(i,0xf) < f(i,0x10) ? 0x10 : 0xf` gives the inverted jump.
  Try the mirrored comparison with swapped arms. (s065_1afe_09cc)
- **s051_19f2 (9 stubs) SOLVED.** File-I/O card functions copy src/c/s082/s082_1922.c (`_fdopen`, `_dos_read`, FILE decls).
  `char far f(void)` for `mov al,1; retf`. Chain of `if (a) return 1; if (b) return 1; ... return 0;` for a shared
  `mov al,1; retf` block jumped to backwards. Error branch laid out at the END = it is the `else` of the source
  (`if (f() >= 0) { main } else { error }`), not an early return.
- `push 1; jmp L / ... push 6; jmp L / push 2 / L: call f` (args pushed in nested branches) = if/else chain of full
  calls `if (..) f(1); else if (..) { if (..) f(4); else if (!(x % 3)) f(6); else f(2); } else f(0);`, not a ternary arg
  (the ternary gives `mov ax,K` + `push ax`).
- `int a[5] = {0};` -> `mov word ptr [bp-12h],0; sub ax,ax; mov cx,8; rep stosb`.
- Loop-temp spill slots ([bp-2]/[bp-4]) and `cmp dx,di` direction depended on an extra block variable (`int r;` instead
  of reusing v_si) and on the position of its declaration among the function's locals (scan positions).
- s110_1838_0000 and _031a written from asm (card main + switch). 031a: first party loop has si/di swapped
  (i in DI, pointer in SI; orig i=SI, ptr=DI); second loop correct. Tried block `int i`, register, user pointer var,
  `char *p = D_9c69[i]` (gets regs right but adds `mov cx,di; mov bx,cx`). Unsolved.
- **s046_1838_057e SOLVED** with the identical-arms trick (`if (f(100) <= p) {X} else {X}` keeps the bp frame).
  Same trick on s073_1a7e_059e (dead `p_6 = 0x64;` store kept by orig) gets the store + frame back, but 6.00A then
  loads 0x64 via AX (`mov ax,0x64; mov [bp+6],ax; push ax`), orig uses immediates. Unsolved (score 3).
- s044_18d1: 000e written from asm; 0348 = switch whose `default:` has nested `if (!v) { if (g()) B0 else B1 } else A`
  (A laid out last); 0772 `case 1: case 2: if (D_ee6a == 1) f(0x36) else f(0x3d)` (dec16 gave goto into case);
  0e80 = `volatile int l_2` + every switch arm written in full. Remaining 000e: `f(D_8de0, l_2 > 0 ? l_2 : 1, 0, 0)` -
  orig puts the `mov cx,[bp-2]; jmp back` arm OUT OF LINE (after a later unconditional jmp, `jle +3; jmp near`);
  6.00A keeps it inline for all ternary/if forms tried.
- **s114_18d3 SOLVED** (from r3w1 copy): card main written from asm; far bit flags
  `if (F_2e38_0000.f_10be[D_e89b][0x14] & 0x80) { ...[0x14] |= 1; } else { ...[0x14] |= 0x81; }` (no pointer var; the
  original's extra 4 frame bytes come from MSC's CSE temp); dec16 `goto`/`for(;;)` tangles rewritten as plain if/else with
  every arm in full (tails get cross-jumped back); `r.a = f(D_ee8c = D_ee6c, ...) * 3`; `register int v_si = p_6;`.
- **s105_1a12**: 4 stubs written from asm and matched (0426 needed declsearch of the 05ee prototype). 0330 = if/else chain
  (dec16's switch with `case -4..4` was wrong); `push 2D1Eh` is an immediate, not D_2d1e. 0006 left: frame 8 with
  `mov di,dx` (dead high word of 2nd `00f2()` call) - probably an unused `long` user var; not solved.
- **s127_18b4 SOLVED** (from r3w1 copy): stub card main (second callback table is `D_ec00[0][D_ee6a]` = `[bx-1400h]`);
  0296 = `if (f()) g(); if (D_e48a != 0xc) switch (h(...)) { case 0: case 1: if (p) X(p,0); break; case 2..4: ... }`;
  08f6: early-return block at the end of the code = `if (v) { switch ... } else { ... }` (not `if (!v) {..; return;}`);
  05d4/08c0/0cb4 call order fixed by declsearch on the 14a4 prototype.
- s057_192f (from r3w1): 4 stubs written; 04d6 SOLVED by keeping dead register stores inside ONE call expression
  (`f(D_ee9c, 0, v_si = g(0xc, 0xb4, v_si = h() + (int)(L() / 0x96)) / 0xc, 0)` gives `mov si,ax; push si`);
  0672 `r.b = v_si >= v_di ? -2 : x` (ternary operand order picks the jump sense). 0a16 left at score 3:
  `volatile int v; volatile int ok;` fixes slots/regs of the found-flag loop, but the final `i = v; if (ok) i = 0;`
  temp lands in CX instead of SI.
- **s067_1a24 SOLVED** (from r3w1): stub 0222 with literal strings ("today", "tomorrow", "two days hence",
  "three days hence" - guessed from movsw counts, data matched); 0558/084c one expression
  `(f14a4(0xf) + (f0690() + f1600(0xf)) + v_si) / 3` + declsearch; 079e `void f(register int p_6)` using p_6
  directly (loads `mov si,[bp+6]` in the prologue); 035a: a case falling through into another (`case 1: case 2: X;
  case 0: Y`) gets Y laid out at the end - write both bodies in full (`X; Y; break; case 0: Y; break;`).
- **Single-use register var kept (`mov si,ax; push si`) - SOLVED pattern**: put an assignment to the SAME var inside
  the expression that computes it: `v_si = f0094(v_si = f05ee(..,4) + f05ee(..,3), v_di);  r.a = f000a(v_si);`
  (s067_1869_0456, s065_1afe_04b4 now match). Only works when the inner value really is built in SI in the original;
  for s067_1869_04ba (`push ax` for the inner value) the same trick adds `mov si,ax` (score 2, unsolved).
- **s065_1afe_0518 / s065_18fb_0522 / s062_19fe_051a SOLVED** (same template): plain nested ifs with early returns,
  `(unsigned)f(0x21) % 0xf` for the `sub dx,dx; div cx`, `switch ((unsigned)(D_9c4a + D_907e) % 3) { default: ...;
  case 0: ...; case 1: ... }` with `if (!f392c(..)) f2c4e(..., 0, 0x32, 0, 0xa8, 0, 0);` (the `push ax` zeros come
  out by themselves). Rewriting these also fixed the jump-table layout diffs of 080c/094a and 0848/0986 (symbol table).
- **Remaining family puzzle** (s065_1afe_0b6a, s065_18fb_0be2, s062_19fe_0ba4): `v_di = f(i,0x10); v_di = v_di <= f(i,0xf)
  ? 0xf : 0x10;` gets registers right (pointer spilled to [bp-2], v_di in DI) but MSC computes the ternary in AX then
  `mov di,ax` (orig stores `mov di,0x10` / `mov di,0xf` directly). Score 10.
- **SOLVED the "loop pointer takes DI before a user var" family** (s065_1afe_0b6a, s065_18fb_0be2, s062_19fe_0ba4;
  orig keeps the party pointer on the stack: `mov word ptr [bp-2],9C69h ... mov bx,[bp-2]; cmp byte ptr [bx],1 ...
  add word ptr [bp-2],80h`): use an explicit `char * volatile p;` and a do-while with the pointer set AFTER the guard
  and bumped BEFORE the index:
  `v_si = 0; if (v_si < D_a67e) { p = D_9c69[0]; do { if (*p == 1) { v_di = f(i,0x10) <= f(i,0xf) ? 0xf : 0x10; ... }
  p += 0x80; v_si++; } while (v_si < D_a67e); }`. -> s065_1afe, s065_18fb, s062_19fe FILE MATCH.
  Probably also applies to s077_1838_0626 / s074_1995_04b4 / s110_1838_031a (si/di swap in party loops) - not tried.
- s077_1838_0626 with the volatile-pointer do-while: only the stack slots of `best` and `p` are swapped
  (orig best=[bp-0Ah], p=[bp-8]); decl order, volatile best, block-scoped p, init order tried - unsolved (score 7).
- s044_18d1, s105_1a12, s057_192f, s073_1a7e, s070_18c8, s106_18b5: copies of r3w1's files now in r3w2/ with my progress.

## r3w3

- **Orphan stack slot (`enter 2,0` with no `[bp-2]` use).** A local assigned a constant whose uses are all
  constant-propagated keeps its frame slot. s071_1a33_033c: `int t; t = 3; ... vec_1f63(0, 9, 7, t, -1);
  ... p_6 / t ... sub_s043_150b_03b0(t, 0);` gives `enter 2,0` and `mov cx,3; idiv cx; ...; push cx`.
  Unused locals / `char x[2]` / temps holding computed values are dropped instead.
  The same trick fixes card-script entry loops whose frame is 2 bytes bigger than `struct res`
  (s115_1838_0000: `enter 6,0`, result at [bp-6]): `k = 0x11c; if (D_e48a == k) { do {...} while (D_e48a == k); }`
  (k assigned right before the `if`, after `D_e48a = 0x11c;`). Assigning k in the same basic block as all its
  uses (local propagation) does NOT keep the slot.
- **Frameless int function** (`push si` only, no bp) with `sub ax,ax` on one path and garbage elsewhere:
  `int f(void) { int r; r = g(D_ee8c = D_ee6c, D_ee4b[D_ee6a]); if (r == -1) return 0; if (r == 1) { switch (D_ee6a) { case 0: case 1: ...; D_e48a = 0xc; break; case 2: case 3: ...} } else D_e48a = 0x11b; }`
  (s115_1838_0278). The `or ax,ax; jl out; dec ax; jle A; dec ax; jl out; dec ax; jle B` chain is a switch with case ranges.
- **`mov sp,bp` instead of `add sp,N`** after calls: normal MSC output for a function with a frame but no
  locals and no saved si/di (s128_1838_034c). Nothing special needed in C.
- **Switch on call result with ranges** `or ax,ax; jl out; dec ax; jle A; dec ax; jl out; dec ax; dec ax; jle B`
  = `switch (f()) { case 0: case 1: A; break; case 2: case 3: case 4: B; break; }` (s128_1838_034c).
- **`call [bx-1400h]` (D_ec00 is declared [5][4])**: write `(*D_ec00[0][D_ee6a])()`.
- **`cmp ax,1; sbb ax,ax; inc ax; jne`** (a 0/1 value tested) = `if ((f() != 0) == 0)` (s047_1a44_0000).
- **`sub si,si; mov di,si`** (two zero inits, di copied from si) = `v_di = 0; v_si = 0;` or `v_si = v_di = 0;`
  (not `v_si = 0; v_di = v_si;`, which gives the reverse) (s047_1a44_0778).
- **Register choice si/di** follows expression shape: `v_si = f() / 2 + 2; v_di = g() + v_si;` gives
  `mov si,ax; inc si; inc si; ...; mov di,ax; add di,si`, while the split form `v_si = f()/2; v_si++; v_si++;
  v_di = g(); v_di += v_si;` swaps si/di (s113_189d_03c6).
- **Local char buffers**: `lea ax,[bp-14h]; push ss; push ax` for _fstrcpy -> `char l_14[20];` (dec16 emits `int l_14; &l_14`).
- **Inline call args** dec16 splits into `v_si = rnd(3); v_si += vec()/4; v_si++; f(.., v_si, ..)`
  must be written inline as `f(.., sub_root_0000_0c70(3) + vec_1c1b() / 4 + 1, ..)` when the original pushes
  other args before computing it (s077_1996_05e2, s084_199a_0206).
- **Else block placed after the function epilogue** (`jne then; jmp ELSE` ... `leave; retf; ELSE: push 0; push 8;
  jmp mid-then`) = if/else whose arms share a long identical tail, written out in full in both arms; MSC
  cross-jumps the tails (s125_19c2_05d0). Writing the shared code once after the if/else gives an inline else.
- **Stores reordered**: a `int` function `{...; D_ee53 = 0xa; return 0;}` lets MSC hoist the constant store;
  `void` (AX garbage = leftover `sub ax,ax`) keeps source order (s125_19c2_0378).
- **First arm cross-jumps INTO the second arm, third arm placed last** (`A; push 0; push 1; jmp L` / `B; push 0;
  push 2; L: call 03b0; COMMON; jmp end` / `C` / `end:`): write
  `if (roll <= p) { A; f(1,0); goto common; } if (p*3 >= roll) { B; f(2,0); common: COMMON } else { C } tail;`
  (s131_1838_01ec/038a). Full duplicate arms make the *second* arm jump into the first instead; a goto to a
  trailing label gets the target block moved inline.
- **`x*3`** shows as `mov cx,ax; shl ax,1; add ax,cx` (or `mov ax,si; shl si,1; add si,ax`): write `x * 3`
  (dec16 prints `(f() << 1) + f()`, calling twice).
- **`push args; mov si,ax; call` everywhere = ONE expression** (confirmed again): s053_1a4f_0530 roll check is
  `if (f1600(0x10) + f1600(0xf) >= rnd(100))` (rnd called first). Order of the two same-function calls was fixed by
  moving `int far sub_s029_0e76_1600();` after `..._19d0();` AND adding `extern int D_ff00;` after `struct res`.
  s065_1a29_033a: `0066(0, 0x5a, 05ee(D_907b,5) + 01a0(D_907b,10)/2 + D_ee74)` (D_907b CSE'd in si; dec16 had
  passed the 05ee result to 01a0 - wrong).
- **`mov cx,si; idiv cx`** where si is known to equal the constant: source wrote `% 5` (constant divisor in cx,
  optimizer replaces `mov cx,5` with `mov cx,si`). Same for `push cx; push cx` after `mov cx,4` divisor.
- **`x*10`** = `mov ax,si`(or ax already = si) `shl si,2; add si,ax; shl si,1` -> write `v * 10`.
- **`vec_1c1b` takes NO arguments.** Pushes around it belong to the enclosing call (e.g. `sub_s029_0e76_2278` with 11 args);
  a missing `add sp` after `call vec_1c1b` is the giveaway. s116_191b_029c.
- **Party count loop with D_ee8c cached in cx** (`mov cx,[ee8c]` before loop, `mov cx,di` inside, `mov [ee8c],cx` after):
  `#pragma optimize("a", on)` around the function; `sub si,si; mov di,si; ... mov dx,si` = `v_si = 0; v_di = v_si; cnt = v_si;`
  (v_si otherwise unused) (s116_191b_029c).
- Sum of two globals `D_e3d0 + D_e3d2` loaded in the wrong order: swap their `extern` declaration lines (symbol order).
- **Local kept in memory and re-read** (`mov ax,[g]; mov [bp-2],ax; cmp ax,0xc; ...; cmp word ptr [bp-2],0x72`, frame 2):
  `volatile int l_2;` (s090_1999_0642). `int l[1]` / one-field struct get optimized away there.
- **Local layout int below a char buffer** (`int` at [bp-16h], buffer at [bp-14h]): `struct { int l; char b[20]; } l;`
  (s060_1a36_0770; plain declarations in either order put the int on top).
- **SOLVED "dx-clobber" division scheduling** (`mov cx,3; mov dx,ax; mov ax,di; mov bx,dx; cwd; idiv cx; mov cx,ax; mov ax,bx; ...`:
  first call's result kept raw in di and divided only after the second call): write the second-evaluated term FIRST in
  the source: `1a7e(0) + (05ee(-1,5) / 2 + 01a0(-1,10) / 3)` (01a0 still called before 05ee). The source order
  `01a0/3 + 05ee/2` divides immediately (s105_1954_0450).
- **`D_907e % 10` chain with D_907e cached in bx** (`mov bx,[907e]; mov ax,bx; ... mov ax,bx; cwd; idiv cx` with cx=10 reused):
  `if (D_907e % 10 < 5) ... else if (D_907e % 10 < 8 || rnd(2)) ... else goto fail;` inside `#pragma optimize("a", on)`.
  Without the pragma MSC reloads `mov ax,[907e]` (that form is right in other functions, e.g. s105_1954_0654).
- dec16 turns `x % 10 < 5` if-chains into bogus `case -0x40 ... case 4:` switches; rewrite as if/else.
- **Compare orientation** (orig `cmp [bp-4],ax; jl`, ours `mov ax,[bp-4]; cmp [g],ax; jg`) for `roll >= (D_582a = expr)`:
  fixed by a declpos.py scan of `extern int D_582a;` (s047_1a44_03f4). Same scan fixed s052_1838_029c
  (`D_ee8c == D_ee8e` load order).
- **`mov [g],ax; imul word ptr [h]`** (global just stored, product computed from AX without reload) where MSC 6.00A reloads
  `mov ax,[h]; imul [g]`: the stored value went through a dead register temp: `D_581c = v_di = f()/3 + 1;
  D_581c = v_di * D_a67e * 14 / 10;` (s047_1974_000c). `v_di = D_581c = ...` does NOT work.
- **Function with a by-value struct parameter** (`push [8df4]; push [8df2]; push [8df0]; push [bp-6]; call`) and
  `mov sp,bp` after every call inside the callee: `void f(int a, struct s6 q) { g(D_906b, &q); ...}` (s047_1974_0396).
- s047_1974_0730 (`ax=[bp-4]; dx=[bp-6]` return): separate `int l_4` best value, `r.b = -2; r.a = l_4; return r;`.
- **`long` function result tested `or dx,dx; jg; jl; or ax,ax; jne`** = `if (f() <= 0)` with `long far f();` (dec16 prints a
  bogus `switch ((int)(f() >> 16)) { case -0x40 ... }`) (s079_1a6e_0000).
- **Far struct pointer CSE'd in es:bx** across 3 stores + global (D_ee6a) CSE'd in dx across far stores: typed
  `struct town far *f_10be[]` with `->f_0c` fields and `#pragma optimize("a", on)` around the function (s079_1a6e_06a8).
- **`push 0; push 0; cmp [g],K; jg; mov si,A; jmp; mov si,B; push si`** (if/else into a register var, then the call with
  the constants pushed first): write `if (g <= K) v_si = A; else v_si = B; f(..., v_si, 0, 0);` - an inline ternary
  arg uses AX instead (s073_199e_07ae).
- **`sub si,15; neg si; mov ax,si; shl si,1; add si,ax; shl si,1`** = `f((0xf - v_si) * 6)` as the call argument;
  every statement-split form gives `mov ax,0xf; sub ax,si` or `neg ax` (s073_199e_0b56).
- **Party "chance" loops** (flag computed in loop 1 via di + `mov [bp-x],di` after the loop; loop 2 with best values):
  write loop 1 dec16-style (`do { t = ...; if (t == 1) break; v_si++; } while (...); flag = t;`) and loop 2 as a
  plain `for` with a single `chance` variable used throughout (`chance = (...)/2; if (chance < best) best = chance;
  chance = best;`), and `if (rnd(100) > p) chance = 0; if (rnd(100) <= chance)` afterwards (s073_199e_01d0/054c).
  A variable set to 0 at the top and then `col = 1` on one path is how `mov si,[bp-0eh]` reloads in the else path appear.
- **`call A; jmp L; call B; L: mov si,K; sub si,ax`** (constant minus a branch-dependent call kept in si, MSC otherwise
  reassociates `K - x` into the final sum): write the subtraction in BOTH arms: `if (!g) v = K - A(); else v = K - B();`
  - MSC cross-jumps the identical tails (s073_1838_046a).
- `abs(x)` is inlined (`cwd; xor ax,dx; sub ax,dx`) with `int abs(int);` declared.
- Sum of calls evaluated in an odd order (`1600, 01a0, 05ee, 1a7e, 1326`): right-nested `a() + (b() + (c() + (d() + e()/10)))`
  plus a declpos.py search for the prototype that comes out of order (s073_1838_0700).
- `if (f(..) + g()/2 >= rnd(100))` - rnd called first (one expression) (s073_1838_0000).
- **Several far CONST slots for the same segment** (`mov es,[80e6h]`, `[80e8h]`, `[80eah]`, ... all 2e38): each is a
  separate extern symbol (`F_2e38_07be[][0x16]`, `F_2e38_082c[]`, `F_2e38_0000` struct, `F_2e38_10be[]`); slots are
  assigned in the order of first use / declaration (s089_18ce).
- **6-byte struct passed by value** (`push [bp-2]; push [bp-4]; push [bp-6]; push D_907b`): `struct cs { int f0; char f2,
  f3, f4, f5; } l; ... vec_1f6d(D_907b, l);` (s089_18ce_04d6).
- **Bit set in a far bitmap row**: `F_2e38_082c[D_ee8c * 0x14 + x / 8] |= 0x80 >> x % 8;` after `D_ee8c = D_ee6c;` (s089_18ce_03be).
- **Global CSE'd in bx across an if/else** (`mov bx,[9082]; cmp bx,64h; ... mov ax,bx`): plain `D_9082` uses inside
  `#pragma optimize("a", on)`; a local temp gets si/di instead (s081_18bb_07fe).
- **`mov si,ax(D_907b); push si; call A; push 5; push si; mov si,ax; call B`**: one variable reused:
  `v_si = D_907b; v_si = B(v_si, 5) / 5 + A(v_si, 0xa) / 0xa;` (A evaluated first) (s081_18bb_04cc).
- **`mov ax,dx; or ax,ax; je` on a global CSE'd in dx** (after `mov dx,[ee6a]; cmp dx,1`): the second test is a
  `switch (D_ee6a) { case 0: ...; break; default: ...; }` inside `#pragma optimize("a", on)` (s060_18cf_06e2).
- **`or ax,ax; jl X; dec ax; dec ax; jg X; jmp B`** with the out-of-range code inline first: `switch (v) { default: A;
  break; case 0: case 1: case 2: B; }` (s060_18cf_0244).
- **char local at [bp-2] in a 2-byte frame** (6.00A puts a lone char at [bp-1]): `struct { char c; char pad; } l;`.
- **Random party member search with D_a67e in dx**: `c = D_9c69[v].f_00; while (c != 1) { if (c == 2) break;
  if (++v >= D_a67e) v = 0; c = D_9c69[v].f_00; }` + pragma "a" (s060_18cf_0244).
- **`max(0, x)` computed in AX before the store** (`add ax,si; jns L; sub ax,ax; L: mov [bp-4],ax`):
  `r.a = x < 0 ? 0 : x;` with x written out twice (CSE'd) (s044_1ac6_0368).
- **`r.a` stored to [bp-4] in both if/else arms** (`call; mov [bp-4],ax; jmp; mov [bp-4],0`, 6.00A keeps it in AX):
  dead register temp again: `if (c) r.a = t = f(...); else r.a = 0;` (s112_1881_0de8).
- **Global stored and the value kept in si** (`mov [ed4c],ax; ... mov si,ax; ... imul si`): `D_ed4c = v_si = expr;`
  (register var assigned first, global from it); `v_si = D_ed4c = expr` reloads from the global (s056_18c0_000c).
  General rule: **"dead/extra register temp" (`g = v = ...`, `r.a = t = f()`) reproduces several 6.00-vs-6.00A
  differences where the original keeps a value in a register or stores a struct field in both arms.**
- **`add al,7; and ax,0fh; add ax,0eh`** = `((f() + 7) & 0xf) + 0xe` (the add is narrowed to AL because of the mask).
- **5-term sums of calls (`(A + B + C + D/10 + E + 0x14) / 4`, calls A..E in order, adds D,C,B,A,E)**:
  right-nested `A + (B + (C + (D / 0xa + (E)))) + 0x14`, then a declpos search on one prototype to get the call
  order (s056_18c0_040c/056e/06e8/0856/0a5a: position of `sub_s029_0e76_05ee`/`16fe` prototypes).
- My scratch helpers (not in tools/): mydp.py = declpos that stops at the first position where a list of functions
  all match; vt.py = parallel variant tester (old-text file + variants separated by `\n====\n`).
- **Compare orientation of two register locals** (`cmp di,si` vs `cmp si,di`): swap the declaration order of the two locals (s080_1929_0a8e).
- **6.00 'constant kept in AX' push** (`mov ax,1; mov [ee82],ax; push ax` for the next call's arg): pass the global: `D_ee82 = 1; f(..., D_ee82);` (s080_1929_07ba).
- **Stack local stored then copied into si** (`mov [bp-2],ax; mov si,ax; cmp [g],0; jge; ...; mov si,ax; push si`,
  frame without a phantom slot): use the stack local directly (`l = ...; l = D_a67e * l * 0x12 / 0xa; if (D_ee74 < 0)
  l = ...; f(.., l);`) - MSC enregisters it in si for that region. A separate `v_si = l;` copy gets a phantom slot
  (s044_1ac6_0008). The si var can still be reused later as the switch variable.
- **Read PORTING_NOTES r3w0..r3w2 first** - pragma "a" for "register var spilled to stack home" (s112 000c/000e) and
  pragma "e" off + "a" on for "local on stack with ax reuse, reload after branch" (s061_1a5f_0790) were already there.
- **vec_1f6d 6-byte struct arg whose idx word is pushed from AX** (`push [bp-4]; push [bp-6]; push ax`, frame has an
  unused word at [bp-8]): `a.idx = l_2[0]; a.f1e = F_2e38_0890[a.idx]->f_1e; a.rnd = v_si; a.f25 = ...; a.x = 1;
  vec_1f6d(-1, a);` - the dead store of a.idx disappears, its slot stays (s109_192e_023c).
- `-(f()/2 + 5)` compiles as `sar; sub ax,-5; neg ax`.
- **A local `l_c = l_6 / 2` at its own stack slot that is reused in a second expression** was a CSE temp: write
  `l_6 / 2` in both expressions (no variable); then the call in the expression is evaluated first and the frame
  layout of the other locals comes out right (s092_197b_0438).
- **Order of `+ l_a + l_8` (two stack locals added) follows the LOCAL declaration order**, not the source order
  (swap `int l_a; int l_8;`) (s092_197b_0438). Same family as the reg/reg compare orientation rule.
- **s063_1a3b is a twin of s053_1924** (constants 0x40->4, 0x1a->0x17, 0x25/0x26->0x4f/0x50, extra fstrcpy blocks);
  port fixes between them function by function and keep the declaration blocks identical line-for-line (add unused
  `extern char D_59fe[];` / `struct blk9` to mirror) so the decl-position-dependent call orders behave the same.
- `switch ((D_9c4a + D_907e) & 3) { case 0: case 2: while (f()); break; case 1: case 3: while (g()); break; }` gives
  the `mov al,[9c4a]; add al,[907e]; and ax,3; je; dec ax; je ...` dispatch with do-while call loops (s107_19ce).
- Card "load city" functions (`_fdopen/fseek/_dos_read/fopen/fgetc/fread` with "darkland.cty"/"darkland.dsc"):
  twin in src/c/s051/s051_19f2.c (0604); s066_19ec 0d56/0e66/0f76/1086 are the same with fields 0x4a/4c/4e/50.

## r3w3 late patterns (s066/s053/s063/s107/s084)
- String literals are DATA: fmatch places the file's _DATA blob by searching DGROUP, so a wrong literal shifts every
  string offset in the file (e.g. 0x5cf5 -> 0x5935). Read the exact bytes from section 136 at DG*16+off
  (RTLinkExe(...).section_memory(136), base 0x321a*16-load_seg*16). s066_19ec_000e needed "$Enter00".
- Far CONST slot that "skips" one (orig 7fee vs ours 7fea) usually means a *preceding stub* function hasn't been
  written yet: write the earlier function and the slots fall into place.
- Chained global assignment: stores come out right-to-left, so `D_ee88 = D_ee86 = ... = D_ee76 = 1;` stores D_ee76
  first (mov ax,1 then ee76, ee78, ...).
- `a[i] = r == v_di` gives `cmp di,dx` vs `cmp dx,di` depending on decl order of r vs v_di (declare r after v_di/v_si).
- Switch on 2278() result with cases {0, 1-4} and no default: negatives go to the end-of-switch `return 0`, values >4 to an
  *inlined copy* of the epilogue (sub ax,ax/pop si/retf) - this is NOT a separate default; drop the fake negative
  case lists (s107_19ce_086c).
- `F.f_10be[D_907e]->f_0c = 0x62; ...->f_0e = 0x62;` with `les si` + reloaded pointer + CSE'd bx index =
  `#pragma optimize("a", on)` (s107 0b78/0cc8/086c). In 06f8 (si not otherwise used) "a" over-CSEs (whole pointer +
  0x62 in ax) - unsolved.
- s053_1924_07b8 / s063_1a3b_0726: code matches exactly with a copy variable `c = D_907b; v_si = 01a0(c,0xa);
  v_si += 05ee(c,5);` (or `e` off) but leaves a phantom 2-byte stack slot (enter 6 vs 4). Not solved (register,
  nested block, pragmas a/e/l/g all keep the slot).
- s084_199a_044e: original is a frameless dx:ax return that saves dx in bx around cdq/idiv; MSC 6.00A here
  clobbers dx (long return) or adds a frame (struct). Only the unsigned-division form preserves dx (in cx).
- volatile cast at the use site (`((volatile struct F_2e38_0000_t far *)&F_2e38_0000)->f_10be[D_907e]->f_0c`) + pragma a: forces the far table pointer reload while the near index (D_907e<<2) stays CSEd in bx -> `les si` form (s107_19ce_06f8).
  Statement order matters too: vec_1f31(1) BEFORE the two stores -> pointer CSE + `mov ax,62h` (s107 02e0, plain pragma a);
  vec_1f31(1) AFTER the stores -> `les si` form (s107 04e8). Do not make the extern itself volatile; that breaks the CSE form.
- `e` off + `a` on reproduces the s053 07b8 si-CSE of D_907b exactly, but every named local then gets a stack home
  (enter grows). Under `e` off `register` is ignored.
- s104_190e_0002: under every variant tried (plain/register/copy var, pragmas a/e), l_2 is never cached in si inside the
  if block; the original does `mov si,[bp-2]; mov ax,si; shl si,2 ... mov si,ax`. Unsolved.

## r5w0

- **Random multi-decl position search beats 1-D scans.** build/agent/r5w0/t/rsearch.py FILE _func N "decl1" "decl2" ...
  puts ALL given prototype lines at random top-level positions at once (+0-3 dummy `extern int D_ffxx;`), scores
  score.py(target) + 50 per other broken function (whole-file fmatch), stops at 0 and keeps FILE.zero.c.
  Solved s059_1838_0864 (call order 1800/01c0/1326, "no decl position found" by 1-D scans) in 2 trials and
  s063_1a3b_0726 in 33 trials. Env: SEED, J (jobs), DUM (max dummies), IGN (functions to ignore).
- s063_1a3b_0726 (phantom slot with `c = D_907b` copy var): no variable at all for the sum, one expression
  `v_si = D_907b; r.a = f0066(1, 0x63, f1a7e(0) + f01a0(v_si, 0xa) + f05ee(v_si, 5));` (1a7e result goes to cx
  then di = MSC temp), then call order via rsearch on the three prototypes.
- **Single-use register var kept (`mov si,ax; push si`) where the inner value goes through `push ax`** (s067_1869_04ba):
  split the inner call out into the SAME var first: `v_si = f086a(0x13); v_si = f0094(v_si / 2, 3); f05c8(v_si);`
  (the first store is dead but makes MSC keep v_si a real register variable for the second one).
- **6.00 "zero kept in a register var" pushes (`sub si,si; push si` x3) SOLVED** (s070_1ac1_0532): keep the 0 store
  live by USING the old value later on one path: `v_si = 0; f(5, v_si); ... g(.., v_si, ..); if (..) {...} else
  { v_si += c70(2); v_si++; }` (instead of `v_si = c70(2); v_si++;`). 6.00A then really keeps 0 in SI and pushes SI
  for every `v_si` argument; the `+=` on a known 0 folds to the original `mov si,ax`.
- s110_1838 FILE MATCH: 031a party loop si/di swap = just `#pragma optimize("a", on)` around the function;
  0bb4/0c14 `if (c) r.a = K; else r.a = f(..);` with `mov [bp-4],K` in the then-arm = dead temp in the else arm
  `r.a = t = f(..);` (int t) + rsearch on the 088e/vec_205d prototypes for the call order.
- **SOLVED the "nested ternary into a global through DX" family** (s124_199d_000c; also listed unsolved for
  s126_1838_0000, s046_194b_0002): `mov dx,K1 / mov dx,[g] / mov dx,K2; mov ax,dx; mov [g],dx` =
  `return t = D_e48a = c1 ? K1 : c2 ? D_e48a : K2;` with a plain `int t;` local and `#pragma optimize("a", on)`
  around the function (without "a", or with `D_e48a = t = ...`, it goes per-arm / through AX).
- **Found-flag loop + `v = t; if (ok) v = 0;` with the final value in the loop counter's register** (s058_1838_0606,
  frame 2 too big): drop the copy and modify t itself: `if (ok) t = 0; r.a = f(0, 0x63, t);` - MSC loads t into the
  freed SI (`mov si,[bp-4]; ...; sub si,si; push si`). Same shape as s057_192f_0a16 (try it there).
- **`mov si,[bp+6]` at the top of an if-arm (6.00A sinks a `v = p_6;` copy below the calls)** (s054_19df_0298):
  no copy - use the parameter directly in that arm (`if (p_6) f(p_6, 0);`); MSC enregisters the param into the
  free SI at the start of the region, like the original.
- **Store order of a local in a chained-constant block SOLVED** (s068_19f9_0004: `mov ax,1; mov [bp-4],ax; mov [ee76],ax..`
  and later `mov di,[bp-4]` when the loop is skipped): dec16's `v_di = l_4;` was wrong; the source is a flag register
  var initialised FIRST: `v_di = 1; D_ee76 = 1; ...; for (..) { if (f(i,0x3b) >= 1) v_di = 1; else v_di = 0; }`.
  6.00A writes the register var's stack home ([bp-4]) and reloads it on the zero-trip path, exactly like 6.00.
- **Dead constant store to a parameter kept with an immediate** (`mov word ptr [bp+6],64h; push 64h; call c70; cmp ax,64h`,
  bp frame with `mov sp,bp` cleanups): declare the parameter `volatile int p_6` and keep `p_6 = 0x64;`
  (s055_1932_0196). Probably also fixes s073_1a7e_059e (score 3 with the identical-arms trick).
- Entry functions with si/di swapped in the party max loop + 2-byte orphan frame slot (s055_1932_0000): plain
  `for (v_di = 0; v_di < D_a67e; v_di++) { v_si = f(v_di, 1); if (D_8dfc < v_si) {..} }` + `#pragma optimize("a", on)`
  fixed both (dec16's guarded do-while did not).
- s079_19a0 FILE MATCH: typed `struct town far *f_10be[]` (from src/c/s079/s079_1a6e.c) instead of dec16's
  `*(int far *)(p + 0xc)`, every function with far-table stores wrapped in `#pragma optimize("a", on)`, and the
  `les si,..; mov es:[si+0Ch],K; mov es,[slot]; les bx,..; or es:[bx+14h],4` form = first store through
  `((volatile struct F_2e38_0000_t far *)&F_2e38_0000)->f_10be[D_907e]->f_0c = K;`, second store plain.
  Wrong push/`add sp` vs `mov sp,bp` (si saved or not) disappears once SI is really used (les si).
- s105_1a12_0006 (frame 8 with `mov di,dx` = dead high word): the 00f2 helper returns a `long`; call it through a
  cast `(unsigned)((long (far *)())sub_s029_1367_00f2)(D_9072) / 0x64` (frame +4 comes from the long temp; no user
  var needed). Left: `add bx,si` vs `add si,bx` for `(int)(X/100 + Y/100) + vec_2161()*0x14` (score 2; rsearch on
  vec_2161/00f2 positions found nothing in ~60 trials).
- s070_19af: 000a written from asm (pragma "a" for the `mov dx,[e3d0]; cmp dx,4..` chain; the `push ax` zero for
  `03b0(6, 0)` after `or ax,ax; jne` comes out by itself), 0abc and 08c8 written from asm (`int k;` uninitialised
  default of a `% 7` jump-table switch = `mov cx,[bp-2]`). 08c8: two jump tables (one per if-arm) pointing at the SAME
  case bodies = the first switch's cases are `goto Lxx;` to labels on the second switch's bodies
  (`case 0: case 2: goto L78;` ... / `L78: vec_2085(-2, 0x78); break;`). Duplicated bodies instead leave a dead
  `jmp` after the first table in 6.00A. -> s070_19af FILE MATCH (055e/071c/0abc call orders by rsearch over the
  01a0/05ee/1326/1a7e prototypes, multi-function scoring `_055e,_071c,_0abc`).
- **Jump-table body order 0x79 before 0x78 for `case 0: case 2:`** (s070_19af 0abc/08c8): split the group,
  `case 0: k = 0x78; break;` ON ONE LINE + `case 1:` + `case 2:` multi-line; MSC keeps the case-2 copy (e1 line-layout rule).
  A jump-table base off by 2 (`jmp cs:[bx+0b3e]` vs 0b3c) is only the length of an EARLIER function.
- s084_199a_044e: as a `long` function `D_ee8c = D_907b; return ((long)D_ee8c << 16) | (unsigned)(v / 3);` gives
  the frameless dx:ax form but 6.00A then forgets to save DX around `cwd; idiv` (picks `bx` as divisor, no
  `mov bx,dx`) - looks like a 6.00A codegen difference; unsolved.
- s115_19bf_0222 / s082_1845_0152 / s044_18d1_000e / s049_19f7_0002: "6.00A moves a small block out of line /
  6.00 keeps it" family; tried one-line layouts, case orders, default/goto, all pragmas: unsolved.

## r5w1

- vt.py FILE FULLFUNCNAME VARIANTS [JOBS]: variants (full function text, may include pragma lines) separated by
  `\n====\n`, scored by r3w2/score.py; `APPLY=k` writes variant k into FILE.
- s074_1995_04b4 (min-over-party loop, best in SI / i in DI swapped): `#pragma optimize("a", on)` around the function. SOLVED.
- s077_1838_0626 (stack slots of `best` and volatile loop pointer `p` swapped; renaming did nothing): make `best` an
  array `int best[1];` (aggregates go below the scalars) -> FILE MATCH.
- s104_190e_0002 (`mov [bp-2],ax; inc ax; je; mov si,[bp-2]` = local stored, compare uses AX, then reloaded into SI
  inside the if): `l_2 = f(); if (l_2 != -1) { v_si = *(volatile int *)&l_2; ... use v_si ... }` inside
  `#pragma optimize("a", on)`. A volatile declaration makes the compare re-read memory too. SOLVED.
- s053_1924_07b8: twin of matched s062_19fe_0764 - one expression `v_di = 1a7e(0) + (01a0(D_907b,10) + 05ee(D_907b,5));`
  (D_907b CSE'd into SI, 1a7e result parked in CX) + declpos on the `sub_s029_0e76_1a7e` prototype. SOLVED.
  Lesson: grep src/c for a sibling with the same call constants before experimenting.
- dput.py FILE DECLLINE POS [OUT]: move a declaration line to a declpos.py position (to check the whole file).
- s057_192f_0a16 still score 3 (final temp in CX, orig SI): tried separate k/register/block vars, ternary forms, volatile-read casts, pragmas a/g/c/l/w/z/t/s, dead later uses. Unsolved.
- s070_18c8_000e: only `cmp cx,ax; jg` vs ours `cmp ax,cx; jl` for `c70(0x64) <= l_2*3/4`. Tried declpos of 0c70/184a/09f0 protos + dummy extern, operand orders, swapped if/else arms, local perms/renames, pragmas, temps. Unsolved (score 2).
- s073_1a7e_059e: dead constant store kept as immediate `mov [bp+6],64h` (6.00A routes it through AX and reuses AX for the push): `*(volatile int *)&p_6 = 0x64;`. SOLVED. General: volatile-cast lvalue stores/loads give 6.00-style memory ops without making the variable volatile.
- s046_194b_0002 tail `mov ax,dx; mov [e48a],dx`: tried return/store splits, volatile casts, type casts, register temps, pragmas a / c-off. Best is `mov ax,dx; mov [e48a],ax` (or swapped order with volatile store). Unsolved.
- **`mov cx,4; cwd; idiv cx` for a signed /4** (6.00A uses the shift idiom for `x / 4` and for `x /= 4` on a plain
  local): the compound division must be on an AGGREGATE member: `int sum[1]; ... sum[0] /= 4; f(sum[0]);`
  (or `struct {int s;} sum; sum.s /= 4;`, or a struct res field `r.a /= 2` as in s044). Plus declpos of
  `extern int D_a67e;` for the loop bottom test. s080_18a5_055a SOLVED.
- s081_1962_000c (l_2 in memory, loaded into DI only for the loop): volatile-cast stores `*(volatile int *)&l_2 = 0xc;`
  + a copy register var for the loop gives exact code but a phantom 2-byte slot (enter 8); using an unused
  PARAMETER as the copy (`int f(int p_6) ... p_6 = l_2; do {... 049a(p_6) ...}`) avoids the slot but 6.00A adds
  `mov [bp+6],di`. Score 2, unsolved.
- s106_19bc_049e SOLVED (bp frame without locals + `cmp dx,1` + else-arm jumping back into the first arm):
  `t = D_e7d8; if (!t) { if (!f()) { A1; D_e48a = 0xc; return; } A2; D_e48a = 0xc; return; } if (t == 1 && t) {...; return;} ...`
  in an `int` function with bare `return;` (no values). The early-return form gave the frame AND the original
  cross-jump direction; the if/else / switch forms gave neither.
- s126_1838_0000 tail is the same `mov ax,dx; mov [e48a],dx` family as s046_194b_0002 - unsolved.
- s118_1838 SOLVED by rewriting dec16's bogus long-switch as `long far 00f2(); if (f() >= 0x3e8L) {..} else {..}`, `(f(1) - 0x23) * 5`, and a goto mess as if + switch with full arms.
- **Single-use register var kept in SI (`mov si,ax; sub si,cx; add si,64h; push si; call f`)**: assign the call
  result back to the var: `v = expr; v = f(v); r.a = v;` (s057_1a02_07b0). The call-arg value then stays in SI.
- s057_1a02 SOLVED: long 00f2 compares, `(unsigned)00f2() * 9 / 10`, one-expression 2278 args written inline, switch(D_a88d) {default; case 6/8 fallthrough into 9/10} + switch(v_si) with full arms (dec16 had gotos and bogus negative cases).
- s057_1838: 03cc fixed (pragma a + one-expression sum + declpos 01a0 pos 56). 0500 (value switch into CX with gotos into case bodies) best score 16-17 with if/else + goto L_0561/L_0577 forms; stubs 0000/0214/067c not written.
- s066_1b35 SOLVED (8 functions): `extern char D_9c69[][0x80]` + `D_9c69[D_ee8c][0] = 0;` (shl bx,7 + push [ee8c]);
  `mov ax,[g]; or ax,ax / dec ax; je` on D_ee53 = `switch (D_ee53)` (not if); struct-res callback switch with
  `r.a = call` per arm; ALL dec16 gotos replaced by full tails per arm (MSC cross-jumps them back exactly);
  stub 0196 = `switch (D_ee53 %= 5) { default: ...; case 0: ... }`; declpos for 05ee/01a0 order in 0e68.
- s064_190c (partial): 04bc SOLVED with `v_si = D_907b; v_si = 0094(v_si = 05ee(v_si,4) + 05ee(v_si,3), v_di); r.a = 000a(v_si);`.
  0774 uses the s062/s053 twin form but declpos of 1a7e alone only reaches score 4 (needs a 2-D scan with 01a0/05ee).
  0350: `register int p_6` used directly fixes the prologue load; left `cmp si,di; jl` vs ours `cmp di,si; jg` (score 2).
  Stubs 0520/0a1a not written; 0008/081e/095c/0bb4 not attempted.

## r6w0

- s105_1a12_0006 (`add bx,si` vs `add si,bx` for `(int)(X/100 + Y/100) + vec()*20`): drop the `(int)` cast, keep
  everything unsigned: `X/0x64 + Y/0x64 + vec_2161()*0x14`. FILE MATCH. Lesson: casts change which operand MSC
  treats as the accumulator; try removing/adding casts before declpos searches.
- s126_1838_0000 / s046_194b_0002 (`mov ax,dx; mov [e48a],dx` tail): the s124 solution applies directly -
  `return t = D_e48a = c1 ? K1 : c2 ? D_e48a : K2;` (plain int t), `#pragma optimize("a", on)` around the function.
  s046 variant (else arm `if (g != 0x3c) { D_e7d8 = g; v = 0x24; }`): NO pragma, ternary with a comma expression:
  `return t = D_e48a = c ? D_e48a : D_e48a == 0x3c ? D_e48a : (D_e7d8 = D_e48a, 0x24);` (with pragma a: score 9).
  Both FILE MATCH.
- s081_1962_000c (l_2 in memory, `mov di,[bp-2]` only before the loop): just `#pragma optimize("a", on)` around the
  unchanged function (`int l_2 = 0xc;`, plain stores). "w" on also works. FILE MATCH. Always do a full pragma sweep.
- s057_192f_0a16 (found-flag loop, final value in SI vs ours CX): no volatile, no copy: plain `int v; int ok;`,
  `v = f()+0x14; for (...) ok = 0 ...; if (ok) v = 0; r.a = g(0, 0x64, v);`. MSC keeps v/ok in memory in the
  loop region and loads v into SI for the tail by itself. FILE MATCH.
- s056_1abb_0212 (case 0/1 `cmp di,8; jmp` INTO case 6's `jl; mov si,1` - 6.00A cross-jumped the other way, insensitive
  to case order/line layout): explicit goto into a label INSIDE case 6's if-body:
  `case 3: if (v_di >= v_si) goto L1; if (v_di > 4) v_si = 0x12; break;`
  `case 6: if (v_di >= 5) { L1: v_si = 1; } break;`  `case 0: case 1: if (v_di >= 8) goto L1; break;`
  MSC then merges `jge L1` into `jl out` + shared tail exactly like 6.00. Loop bottom test `cmp [a67e],si; jg`
  then fixed by rsearch on `extern int D_a67e;` alone (1 trial). FILE MATCH.
- s049_1893 FILE MATCH (13 failing -> 0): (1) module data `D_586c` (an int between the "$Marke00" literal and the
  next function's literals) = a real DEFINITION `int D_586c = 0;` placed between functions 0002 and 045a (keep
  `extern int D_586c;` at the top). `static int D_586c;` + later `static ... = 0` goes to _BSS (fmatch refuses).
  (2) the 10 small callbacks reading it into DX once (`mov dx,[586c]` before the r.b store) = `#pragma optimize("a", on)`.
  (3) dec16's `(f() << 2) + f()` (two calls) = `f() * 5` (`mov cx,ax; shl ax,2; add ax,cx`).
  (4) entry function: dec16 goto/for(;;) mess rewritten as early `if (a && !b) { ...; return D_e48a; }`,
  `t = D_e48a; if (t == 0x91 || t == 0x152 || t == 0x90 || t == 0x151) return t;` (DX chain, no pragma), plain ifs,
  final `return D_e48a;` (MSC cross-jumps the end back to the early return).
- s056_19c4_0008 written (was a stub): standard entry function; module data after "$Cathe00" = definitions placed
  AFTER the function: `char D_5aa1 = 0; int D_5aa2 = 0; char D_5aa4[6] = {0}; char D_5aaa[6] = {0};` (data matches).
  `long l = 00f2(D_906b); if (l < 0x1c20L) ...; q = l / 3;` Left (score ~7): original keeps the long quotient in
  SI:DI (`mov si,ax; mov di,dx`) and computes `mov ax,si; sub ax,[bp-4]; neg ax` for `(int)(l - q)`; 6.00A keeps
  only SI and does `sub si,[bp-4]; neg si`. Tried register/plain long q, int v_si, 6 expression shapes, pragma a.
  Adding declarations near the end of the decl block flipped 0ac2's `cmp si,di` - put new externs earlier/check.
- s049_19f7_0002: `l_2 = X; if (l_2 < 4) l_2 = 4;` (cx stored twice: `mov [bp-2],cx; cmp cx,4; jge; mov cx,4; mov [bp-2],cx`)
  = `l_2 = X; l_2 = l_2 >= 4 ? l_2 : 4;` with plain int locals (not arrays). Left: the 6.00 "ternary then-arm
  moved out of line" family (`X > 0 ? X : K` -> `jle Lk; jmp FAR; Lk: mov cx,K` with `FAR: mov cx,ax; jmp back`
  placed after the next unconditional jmp in the code). Same in s044_18d1_000e (`l_2 > 0 ? l_2 : 1`).
  6.00A never does this; cannot be expressed with goto because the arm value lives in a compiler temp (CX).
- **Compare direction after the signed-/4 or abs idiom** (s070_18c8_000e `c70(100) <= l_2*3/4`, s079_18d8_02f6
  `c70(si) <= abs(D_ee74/5)`): 6.00 `mov cx,ax(call); ...; cmp cx,ax; jg`, 6.00A `cmp ax,cx; jl`. The same shape
  with a plain `idiv` RHS (`D_9082/10`) matches in s079, so it is tied to the cwd/xor/sub idiom. Insensitive to
  operand order, casts, temps, aggregate locals, pragmas, decl positions (rsearch 120 trials). Probably 6.00 vs 6.00A.
- s084_199a_044e (frameless long return `mov dx,ax; mov ax,si; mov bx,dx; cwd; idiv cx; mov dx,bx`): with a `long`
  (or far-pointer) return 6.00A emits `mov bx,3; idiv bx` and loses DX (codegen bug); 12 expression shapes tried.
- s115_19bf_0222: 6.00A moves the 1-statement `D_e48a = 0xd` block (cases 3/4) out of line after the next jmp even
  when it is written after the switch with gotos; per-arm full tails, pragma sweep: no effect.
- s106_18b5_02b6: frame 4 vs 2 (original loads an uninitialised `[bp-2]` into DI before the 2nd strength-reduced loop
  = 6.00 region artefact) + case-0 `push; push; jmp` into case 1/2's call (6.00A merges the other way); case
  permutations, one-line layouts, goto end: no effect.
- s082_1845_0152: 6.00 placed switch case 0 inline after `or ax,ax; jne` (rest of the dec chain after it); if/else +
  switch rewrite reproduces the shape only partially (score 43).
- s057_1838_0500: real source is `int k;` (uninit default) + `switch (07ee()) { case 3..7: k = ...; }`
  `1fb8(3, k, 0);`, but with gotos between case bodies 6.00A re-lays the blocks (score 49, worse than r5w1's 16).

## r7w0

- **s064_190c FILE MATCH** (promoted). s064_190c is a near-exact TWIN of matched s053_1924 (constants 0x40->0x10,
  e7d8 0x1a->0x17, etc.). Copying s053's whole declaration block (with fn names mapped by index, `struct member D_9c69[]`,
  `extern int D_a67e;` right after D_4ec4, 1a7e prototype before 18a8) fixed the call orders (0774/095c) and the
  `cmp si,[a67e]; jl` loop bottoms (0a1a/0bb4) at once. 0bb4 = s053 0bfc (plain for loops + `#pragma optimize("a", on)`).
  0008: `if (3470() != 1) { ee78=0; ee7c=0; ee7e=0; ee80=0; } else {... if (3742(..0x17..)==0x10) {ee7e=0; ee80=0;} }`
  (full arms; dec16's goto into the shared tail gave a jmp 2 bytes off).
  LESSON: run build/agent/r7w0/twins.py (asm mnemonic similarity vs all src/c files) first.
- s057_1838: 0000 (entry), 067c written and MATCH. 0214 (twin of s056_1abb_0212) score ~20: 6.00 compares `cmp di,si`
  where si holds the constant 6 (source probably `v_di >= 6`); 6.00A const-propagates. `default: v_si = 6;` blocks the
  propagation but then MSC swaps SI/DI (v_di gets SI). Tried volatile l_2, decl orders, renames, extra dead stores,
  loop-in-switch label (s056 style), pragma sweep. Unsolved.
- s050_1838: all 5 stubs written; 03b6/0568/0a26 MATCH. Fixes: `for (cnt = v_si = 0; ...)` (orig `sub si,si; mov [bp-x],si`);
  0b64/05ee call order + loop bottom `cmp si,[a67e]; jl` by declpos (tools/porting/declpos.py on `extern int D_a67e;`
  -> right before `struct member`, after rsearch on the 05ee/3a20/0b64 prototypes); `struct member D_9c69[]` instead of
  `char [][0x80]`; 0568 imul operand order fixed by declaring `int gold; int cnt;` (decl order, not expression order).
  Left: 0000 (6.00 out-of-line `if (1a8e(..)>-40) 072a();` block + `mov dx,9c69; mov di,dx` with an extra unused frame
  slot - a 6.00 strength-reduction artefact; explicit pointer var gives the slot but `mov [bp-2],9c69`), 08a0 (same dx/slot
  artefact), 051c (SI/DI swap of 3a20 temp vs sum; all expression shapes/pragmas canonicalise to the same code -> try rsearch).
- s066_190c: 0008/067c written. 058a: one "Alchemist" literal for two calls = `char *m = "Alchemist";` local (const-propagated
  to push immediates; data then matches). 07ec: `2c4e(.., v_si, .., w = v_si / 4 + 0xc, ..); 00a4(.., w)` (second var).
  042c: one expression (MSC hoists all calls to temps) + rsearch on the 05ee/1a7e/01a0/14a4 prototypes -> MATCH.
  067c: separate far symbol `extern char far F_2e38_07be[];` (own CONST slot / SEG load in the loop);
  `v_si = f(); v_si %= 0x42; v_di = v_si;` gives `mov si,dx; mov di,si` (chained form copy-propagates dx);
  decl order `int v_si; int v_di;` fixed `cmp di,si`. 0008: `if ((l_a.b = first++) == 0) 049a(l_a.b); else 049a(1);`
  (dead struct field) reproduces 6.00 `push ax` of known zero. LEFT: the three cross-jumped early-return blocks
  (3878/e896/072a -> 03b0(6/2/3); return D_e48a = 0x1a) are laid out 3,2,6 by 6.00A vs 6,3,2 orig; tried goto labels
  in all orders, nesting, else-if, one-line layouts. Unsolved.
- s081_19bc: 0a88/0c2a written, 0dba (every arm in full -> push bp frame back), 0f30 (`l_2 = v_di = 0; for (v_si = 0..)`)
  all MATCH; only the 2608-byte 0008 stub left. 0a88: `v_di = f(); v_di += g();` as two statements keeps v_di in DI and
  the loop pointer in [bp-2] like 6.00. `struct item far F_2e38_0040[][64]` (6-byte items) for the far inventory.
- s081_19bc_0008 (2608-byte dragon card main) WRITTEN, score 207 -> 37, len 2602/2608. What worked:
  * the 6.00 loops here are UNROTATED (`init; jmp test; inc; test; body; jmp inc`) and re-compute the far index per
    statement: write each filter as its own `if (...) continue;` statement (the `==` chain as one `||` continue) - this
    reproduced case 3 / case 5 exactly (pragmas l/g/c/e had no effect on this function);
  * case 5 tail: `if (c70(100) <= 10) { vec_2003(i, j); continue; } if (c70(100) <= 5 || !flag) vec_2003(i, j); flag = 1;`
  * switch value via if/else into a local that MSC never stores (`k = c70(3)` ...; `switch (k)`) - gives the cross-jumped
    `push 3; jmp L` shape; the dead local also explains one of the orig's 2 unused frame slots;
  * case 6: `if (t < j) f(i, 6, t); else f(i, 6, j / 2);` (calls cross-jumped; the ternary form canonicalises the other way);
  * `extern int D_a67e;` position (tmp/dp2.py checks several declpos positions against the whole file) fixed the
    `mov ax,[i]; cmp [a67e],ax` loop test;
  * frame slots by renaming locals (tmp/nsearch.py: random names, compiles and reads MSC's `; name = -N` slot comments).
  LEFT: case 4 inner loop and the !flag loop in case 5 (bodies `if (t < 0x43) continue; if (t > 0x54) continue; work;`)
  are ROTATED by 6.00A (inc+test after the body) - tried 10 body shapes, goto loops, for(;;)+break: no change;
  frame 0x10 vs 0x12 (orig has unused slots at -2 and -18; a second dead temp moves the other slots).
- s057_1838_0214 / s056_19c4_0354 (same twin, `cmp di,si` with si==6): block-scope loop temps, register keyword, dummy
  `v_si = v_si;` refs: SI/DI stay swapped. Unsolved.
- s050_051c: rsearch (05ee/3a20 prototypes) fixes the call order but the sum still lands in SI (orig DI). Unsolved.

## e0

- **Loading a data file into a fresh buffer** (`call alloc_buffer; sub cx,cx; mov [bp-x],ax;
  mov [g],cx; mov [g+2],ax; push ax; push cx; push "file"; mov si,ax; mov [bp-y],cx;
  mov [bp-y+2],si; call load; les bx,[bp-y]`): one far pointer built from the call, then
  assigned to the global, not a separate `seg` variable:
  `p = (unsigned char far *)((unsigned long)alloc_buffer(ft.size, "x") << 16); D_a889 = p;
  load("darkland.lst", p);` (s040_1462_0000, s033_15d1_0106).
- **Header bytes skipped with separate `inc di; inc di`** after `lea ax,[bx+1]; mov di,ax`:
  dead reads `n = *p++; t = *p++; t = *p++;` (the loads vanish, the increments stay);
  `p += 2` gets folded into `+3`. (s040_1462_0000)
- **Repeated far-pointer-table lookups CSE'd into one `les bx`** with the index still copied into
  si (`mov si,[bp+6]; mov es,[slot]; mov bx,si; shl bx,2; les bx,es:[bx+T]; cmp es:[bx+..]`):
  write `F_2e38_0890[k]->a == .. || F_2e38_0890[k]->a == ..` every time (no local pointer); a
  local `d = F_2e38_0890[k]` puts the pointer in si instead. A declared-but-unused far pointer
  local still gives `enter 4,0`. (s032_1462_0efa)
- **Conditional store with a 0 arm kept as one store** (`... jmp L; nop; Z: sub ax,ax; L: mov [g],ax`):
  `g = !f(a) && f(b) ? h(b) : 0;` makes 6.00A sink the store into both arms (`mov [g],0` immediate).
  Nest the ternary instead: `g = f(a) ? 0 : f(b) ? h(b) : 0;` (s036_1462_0000).
- **Frame slot for a variable that generates no code** (`enter 2,0` but [bp-2] never used): a local
  assigned a constant *before an `if`* and used after it keeps its slot after constant propagation:
  `x = 0xf5; if (D_89fc) free_buffer(D_89fc, "b"); ... f(x, ...)` -> `enter 2,0`, `push 0F5h`.
  The same assignment placed after the `if` gets no slot; unused or never-read locals get none
  either, and constant locals across a `while` loop get none (s036_1462_1c3c). 6.00 apparently
  also allocated never-used arrays (s032_1462_0000 has `enter 8Ah` for an unused buf[136]);
  6.00A drops them.

## Session 2 (2026-10-09)

- **Hex escapes eat following letters**: `"\x8c R\xffegain"` is `\xffe` (C4139 warning, wrong
  byte). Split the literal: `"\x8c R\xff" "egain"`. Check with grep for `xff[0-9a-fA-F]`.
- **CSE temp of a far-table element address lands at the bottom of the frame** (below all
  arrays): `F_2e38_1b28[k + 1]->loc == x ... F_2e38_1b28[k + 1]->name` written out each time
  (no local pointer) gives `mov [bp-84h],bx; mov [bp-82h],es` with the 120-byte buffer
  above it (s036_1d24). A named `struct x far * far *pp` local instead takes a slot next to
  the ints and makes the frame 4 bytes larger.
- **Strength-reduced store pointer initialised after the loop guard** (`mov [bp-4],8A38h`
  after `or ax,ax; jg`) comes from an indexed store `D_8a38[j] = ..` with j the loop counter;
  an explicit `int *p = D_8a38` is initialised before the guard instead (s036_1d24).
- **Unoptimised loops inside an optimised file** (`jmp test` at loop entry, no strength
  reduction, `mov bx,di; shl bx,7` recomputed per access): `#pragma optimize("l", off)`
  around the function (s036_1462_0082).
- **8-bit signed division** (`mov cl,0Fh; cbw; idiv cl; cbw`): `char_field / (char)15`.
- **Ternary only as the last (first-pushed) argument goes straight to push** (`push 15h; jmp L
  / push 2; call; or ax,ax; je; push 9; jmp L; push 0Ah; L: call f`): write the arms as an
  if/else chain of whole calls `if (m == 1) f(0x15); else if (g(2)) f(9); else f(10);` - MSC
  cross-jumps the identical call tails. A nested `?:` in the argument becomes `cmp ax,1; sbb`.
- **Prototype types decide constant-register reuse**: with `int far _dos_findfirst();` 6.00A
  pushed DI (which held a 0 for `i = 0`) for the 0 argument; with the real prototype
  `unsigned _dos_findfirst(const char *, unsigned, struct find_t *)` it pushes the immediate.
- **Unresolved**: byte compare `mov al,[bx+4Fh]; cmp [bx+56h],al; jg` (original) vs 6.00A's
  `mov al,[bx+56h]; cmp [bx+4Fh],al; jl` - operand order, casts, if/else, separate fields:
  no change. 6.00A also hoists the DS=far-segment load of a nested loop to the outer loop
  when the outer loop's near accesses can use SS (s033_15d1_0106); 6.00 only did it for the
  inner loop. No source change found.

## Session 3 (2026-10-09)

- **Byte compare operand order solved** (`mov al,[bx+4Fh]; cmp [bx+56h],al; jg`): 6.00A puts the
  lower-addressed operand in memory when both go through the same symbol. Reach one operand
  through a different extern alias of the same storage and the order follows the source:
  `D_9c69[i].cur[5] < D_9cb2[i][13] ? hi : dim` (D_9cb2 = D_9c69 + 0x49). (s036 0f8c, 08ec)
- **Big compare-tree switch**: bodies come out in case-value order (by smallest label), but the
  strings go to `_DATA` in source order; order the cases in the source for the strings.
- **Global kept in a register through a loop** (`mov dx,[g]; L: dec dx; js; mov bx,dx ...;
  mov [g],dx` with one store after the loop): the loop is written on the global itself and the
  function needs `#pragma optimize("a", on)`: `while (D_9c69[--D_3aba < 0 ? (D_3aba = 5) :
  D_3aba].status == 0);`. A local copy instead gets si/di and changes allocation everywhere.
- **`(i = f()) >= g(..)` vs `f() >= g(..)`**: the register store of an assigned call result is
  scheduled before the next push; the unassigned one-expression form stores after `push`, and
  its call order is decided by prototype position (s036 08ec, moved sub_root_0000_0c70).
- **Constant propagated through a far pointer across calls** (`cash[0] = 0; ... f(.., cash[0])`
  pushes `0` in 6.00A, original re-reads `es:[si]`): read it through a cast,
  `*(unsigned far *)cash`; 6.00A doesn't treat the cast lvalue as the stored one (s040 01c8).
- **Extra frame slot for a loop bound**: `n = 5;` set before an if/else chain and used as the
  loop limit, with `#pragma optimize("g", off)`, reproduces `mov [bp-2],0; mov si,[bp-2]`-style
  6.00 loops (s036 1858).
- **char local in a word slot** (`mov [bp-4],al` with an int at [bp-2]): declare `char t[2]`
  and use t[0]; a plain char gets [bp-1] (s036 1b80).
- **DS=far-segment reload around an inner loop only** (s033 alcform): an extra
  `if (count > 0)` guard around the inner for loop makes 6.00A place the DS load/restore like
  the original, at the price of a duplicated compare. Not solved fully.
- **Files without literals** need `/* data: _DATA=xxxx */` or the DS restore (`mov ax,SEG
  DGROUP`) is unresolved; find the start by dumping DGROUP before the first string the
  asm annotates (s033_1462: 3202).

## Session 4 (2026-10-09)

- **Union for a register-only temp** (s036 1b80 MATCHED): 6.00A gives every named local a frame
  slot even when it only ever lives in AX, so an if/else temp `d` added a slot the original
  doesn't have. Putting it in a union with the char local that owns the original slot,
  `union { int d; char t; } u;` (`u.d = 3/2/1; total -= u.d; u.t = sk66;`), reproduces both
  the `mov ax,3` arm and the frame. Try this whenever "a temp gives the right code but an
  extra slot".
- **Same variable reused for unrelated values** shows up as one [bp-x] slot written by two
  different computations (s033 1390/094a: v and the ingredient loop counter; s032 1d18: the
  uninitialised `town` read and `size` stored to the same [bp-2]). Write one variable.
- **Module-owned initialised data** (s033_1462: D_3244..D_3250 = 0,0,0,0,0,0,-1 between
  0000's literals and 0628's "alc"): define them in the file (`int D_3244 = 0;` ...) placed
  *before* the function whose literals precede them; placed before 0628 the literal "alc"
  still came first. Keep `extern` declarations at the top for earlier uses.
- **CONST slot order** follows first reference of each far symbol in the file; writing the
  stub functions fixed the slot numbers of already-correct functions (s033 1018, 14e6).
- **6.00A sum order**: `a + (b - c) + q + D_g` with q a register always comes out as
  `+ D_g` then `+ q`; source order, separate statements, casts and extern position don't
  change it (s033 1390 left; 094a has the other order in the original and matches).
- **`D_g < K && D_g != -1 ? x + D_h : D_h + D_g`**: 6.00A caches D_g in DX with /Oa;
  `#pragma optimize("a", off)` around the function plus an if/else of two whole calls
  (`if (..) srand(x + D_h); else srand(D_h + D_g);`) and the extern position of D_h
  reproduce it (s032 078e; only an i/y slot swap left).
- **6.00A global register allocation switches off in big functions**: in s036 0082 no
  variable becomes a register variable at all (only block-local caching in si/di) once the
  main if/else chain is complete; removing any one large branch gives `register si = ...`
  again. A small function of the same shape gets i=di, key=si. So the original's
  whole-function i=di/key=si cannot be had there; `register`, volatile, pragma "e"/"l" tried.
- `#pragma optimize("e", off)` makes 6.00A ignore `register` completely (no si/di vars).
- **Declaration order of locals decides operand order** (s036 197e MATCHED): in
  `(x + t + b + c + a) * sz` 6.00A emitted the memory operands c, b, a whatever the source
  order; declaring `int b, c, a;` gave the original a, c, b. The compare `best < val`
  (`mov ax,[val]; cmp [best],ax; jge`) likewise flipped with the order of the
  `best`/`val` declarations (`int i; int besti; int best; int val;`). Slots did not move.
  So for locals, permute declarations before trying expression rewrites (vt-style batch:
  all permutations, score by diff lines). No effect on register choice (s036 10d0, 147a).
- Tools (build/agent/e0): `vt.py FILE FUNC VARFILE [JOBS]` runs snippet variants in parallel
  (VARFILE: original snippet, then alternatives, separated by lines `====`) and prints the
  fmatch line plus `sc=` (number of differing instructions from sd.py); `vs.py FILE FUNC
  VARFILE K [apply]` shows/applies variant K; `dp.py FILE FUNC "DECLLINE" [STEP]` moves a
  declaration line through the file header and scores each position; `regs.py` prints
  the register/enter lines of a function for each variant.

## Session 5 (2026-10-09)

- **6.00A packs locals with disjoint lifetimes into one slot** (lmap shows two names at the same
  offset, e.g. s036 147a `e = -2; j = -2`, s033 0000 `i = -6; key = -6`). The game's 6.00 kept
  them apart (or packed differently). Fixes: extend a variable's lifetime across the other's by
  reusing it where the original source plausibly did (s036 147a: the trainer scan loop counter
  `k` -> `i` made j/n land at -6/-8 and the frame 0x16 like the original); a dead `&x;` also
  stops packing but kills strength reduction of x. In the other direction, reusing one
  variable for two loops reproduces a 6.00 shared slot (s033 0658: inner item loop on `k`).
- **Separate the loop's start from the preceding block to move DS loads**: alcform's inner loop
  `j = 0; if (D_9cd3[i].nitems > 0) for (; j < D_9cd3[i].nitems; j++)` keeps the DS=far
  segment load/restore around the inner loop with no duplicated compare (the plain guard
  around a full `for (j = 0; ...)` duplicates the test).
- `i < D_g` loop guard with `i = 0` in AX: 6.00A emits `cmp [g],ax` then reloads g for the trip
  count; 6.00's `mov bx,[g]; or bx,bx; jle; mov [bp-x],bx` came out only with a local or
  parameter bound (`a = D_g; for (i = 0; i < a; i++)`), which adds a store. Unsolved.
- 6.00A block placement: in s036 147a it moves switch cases 2..6 of a party loop behind the rest
  of the function and puts the loop tail after the first `break`; for/do-while, continue,
  goto, default, case order and pragmas don't change it. Line layout (one-line calls, joined
  `}`/`else`) had no effect on cross-jump copy selection in s040 0c22 either (255 combos).
- Tools: `pv.py FILE FUNCNAME OUT` writes a vt.py variant file wrapping a function in each
  optimize pragma; `vl.py FILE FUNC VARFILE K` prints the /Fc local map of a variant.

## e1

- **Forward tail merging = duplicated source.** When several identical `return -1` / processing
  tails all jump FORWARD to the last copy (e.g. -1 branch `jle L; jmp found; L: jmp ret_m1`),
  the original wrote each if/else-if branch out in full (own loop, own `if (i >= n) return -1;`,
  own processing + `return n;`). MSC cross-jumps the earlier copies into the last one.
  Writing one shared tail (or using goto labels) makes MSC keep the FIRST copy instead and
  re-layout the loops. (s031_18e7 04d4, 0668)
- **`dec mem; je calls; L: return n; calls: ...; jmp L`** is
  `if (--x) return n; f(); g(); return n;` (two returns, merged), not
  `if (--x == 0) { f(); g(); } return n;` (which gives `jne` and inline calls). (0e14, 04d4)
- **Unnamed stack temps are CSEs.** `mov [bp-6],bx` of an address and `mov [bp-8],al; cmp al,..`
  re-read later from the stack come from repeating the same expression
  (`F[who].items[idx].type` in each condition), not from a named local. A named local can
  share a slot with a dead one (frame too small, e.g. a char at [bp-1] overlapping a long).
- **`#pragma optimize("a", on)`** around a function makes a `long flags = ...;` stay in memory
  (stored, then re-read for the second `flags & K` test) instead of being kept in AX:DX. (11b2)
- **Local slot order** follows the order of first assignment, not declaration order
  (permuting declarations and renaming had no effect). Aggregates (struct/array locals) are
  placed below the scalars.
- `#pragma optimize("i", ...)` is rejected by 6.00A (compile error).
- Tooling: build/agent/e1/ptry.py FILE FUNC VARIANTS [JOBS] tests variants in parallel
  (set PT=xx to use separate temp files when two runs overlap); tv.py shows one variant's
  aligned diff; apply.py writes a variant into the wip file.
- (session 2) **Dead loop-base temp `mov [bp-2],ax; mov si,ax`** before a strength-reduced
  far-array loop appears in 6.00A when the function has no named char local; adding a
  named `char n` (even block-scoped, array, struct, volatile) removes it and puts n at the
  top of the frame. A local struct `{found,q,type}` fixes store order of two register
  spills but adds 4 unused frame bytes (s031_18e7 01b4/14b6, both still unsolved).
- (session 2) **STACK SLOT ORDER IN 6.00A CAN DEPEND ON NAMES.** In the big functions s030
  02c0 and 0c44 memory locals get their [bp-x] slots top-down in order of hash bucket =
  (sum of the name's characters) % 16, ascending; within one bucket the LATER-declared
  variable comes first. In small functions (s031 14b6, s030 25be) the order is first use
  and names do not matter; s030 1dd4 is mixed (renaming one local swaps two others). (Verified on s030 02c0 and 0c44 with
  /Fc listings: `msc.compile_c(src, flags, keep_asm=True)` prints `; name = -N` per local.)
  The game's MSC 6.00 instead laid locals out in declaration order, so to reproduce an
  original layout rename the locals so that their buckets ascend in the original's
  top-down order (e.g. append one letter: helper build/agent/e1/hname.py; ren.py FILE FUNC
  v1 v2 .. writes the renamed function to v2.txt). Check with lmap.py FILE FUNCSUFFIX (prints
  the /Fc local map), vmap.py FILE FUNC VARIANTS K.. (local map per variant), probe.py FILE
  FUNC VAR name1 name2 .. (slot order for several renames of one local, 4 in parallel).
  fdskip.py = fdiff.py with capstone skipdata (does not stop at jump tables).
  Register variables are not affected.
- (session 2) `if (dy > dx) ...` distance helpers: `d = dx >= dy ? dy / 2 + dx : dx / 2 + dy;`
  (other spellings of the same ternary give the arms in the other order).
- (session 2) Loops entered with `jmp test` (test at the bottom): `goto test; do { ... test:; }
  while (cond);` reproduces it where 6.00A otherwise duplicates the test at the top.
- (session 2) A constant assigned before a `switch` and overwritten in some arms is sunk into
  the arms by 6.00 (`mov dx,68h` at the start of an arm); 6.00A keeps it before the switch.
  Write the constant in each arm and put the default/after-switch value after the switch
  with the arms using `goto done` (s030 2dd6).
- (session 2) 6.00A emits `__aFchkstk` for `#pragma check_stack(on)`; the game calls `__chkstk`
  (fmatch reports it unresolved) - not fixable from C.
- (session 2) **SOURCE LINE LAYOUT CHANGES CROSS-JUMPING.** Two identical switch bodies are
  merged into a different copy depending on how the duplicate is laid out in the source:
  `case 4:\n x = 0x39;\n break;` kept the copy at case 4's (sorted) position, while the same
  body written on ONE line `case 4: x = 0x39; break;` (placed last in the switch) was merged
  into the earlier `case 13:` copy, as in the original (s030 1c76). MSC compares line-number
  records when cross-jumping. Try one-line vs multi-line arms when the kept copy is wrong.
- (session 2) build/agent/e1/greedy_lines.py: greedy search over one-line vs two-line
  `case ..: return K;` arms (hard-coded to s030 02c0's terrain switch; edit src/func/start
  marker) scoring with fdskip.py; took 02c0 from 199 to <90 diff lines in 3 passes.
- (session 2) s030 02c0 (2.4 KB) MATCHED this way: hash-order local names + greedy line
  layout of the `case ..: return K;` / `default: return K;` arms + an explicit empty
  `case 1: case 2: case 3: break;` (the jump table started at label 1: `dec ax; cmp ax,17h`).
- (session 2) **Unused locals DO get slots in hash-order functions** (s030 02c0 `int unused[5]`,
  20d2 `int gam[6], gbb[1]`): the original's never-used locals (6.00 kept slots for them)
  can be reproduced with unused arrays of the right size, given hash-ordered names. With that,
  s030 20d2's frame and every slot match (only `D_286e + row` operand order left).
- (session 3, e1c) **JUMP TABLES NEED THE PRECEDING FUNCTIONS' LENGTHS.** A `jmp cs:[bx+T]` table
  holds absolute offsets; fmatch relocates code but the table entries stay off by the length
  difference of all earlier functions in the file. A function whose only diff is its table is
  done (s030 1c76; s031_1462 0b22 10ae 2b14 39c6 43b2) - fix the earlier functions' lengths.
- (session 3) **OPERAND ORDER OF FAR STRUCT FIELDS DEPENDS ON THE BASE SYMBOL.** `F[k].x + F[k].w`
  loaded `w` first with `extern struct box far F_290e_3829[]`; declaring the table a few bytes
  early (`F_290e_3825[]`, struct with `char lead[4]`) gave `x` first and the CSE of `x` into cx
  (s031_1462 2518/1d44). Same address, so data refs are unchanged. Shifting by n*recsize with
  `[k + n]` works too. MSC 6.00A seems to order equal-cost operands by a hash of symbol+offset.
- (session 3) **Any new declaration can flip operand orders in other functions** (struct types,
  externs, a local struct in another function): s030 2dd6 `cmp si,dx`, s031_1462 43b2 `t + hi`,
  2518 all flipped while editing other functions. Keep backups; re-run the whole fmatch.
  build/agent/e1/ins.py FILE 'decl' inserts a dummy declaration at every top-level position.
- (session 3) `unsigned v` (instead of int) flipped `cmp dx,si` to `cmp si,dx` (s030 2dd6).
- (session 3) Original `test es:[bx],3 / jne L / mov [s],0 / jmp / L: mov [s],1`: 6.00A turns
  `if (*p & 3) s = 1; else s = 0;` into `and al,3 / cmp al,1 / sbb / inc`; writing
  `if (*p & 1 || *p & 2)` keeps the branches (MSC still merges it into one `test ..,3`) (1e42).
- (session 3) Ternary chains: write `if (q <= A) t = 3; else if (q <= B) t = 4; else t = 5;` -
  MSC turns it into the `mov ax,K / jmp / ... / mov [t],ax` shape in arm order; a nested `?:`
  puts the last arm first (s031_1462 43b2).
- (session 3) 6.00A folds `(x & 0xf) + (x & 0xf0)` to `x`; `x % 16 + (x & 0xf0)` keeps both ands.
- (session 3) Jump threading `cmp ..; jge next_test` from an inner if to the next `else if`:
  `if (k == 10 && e < lim) f(); else if (k == 11 && e < lim) g();` (1e42).
- (session 3) A duplicated flag test `cmp [x],0; je A; je B` is `if (x && (!x || y == i))`.
- (session 3) `if (!cond) break; {...}` in a switch case (instead of `if (cond) {...}`) changed CSE
  across a store and the loop layout (2b14 case 9).
- (session 3) `cnt = n = j = 0` vs `j = n = 0`: the rightmost-assigned register is zeroed first.
- (session 3) Two-assignment arg `f(D_x = 0, 12)`: 6.00 `sub al,al / mov [x],al / push ax`,
  6.00A stores and pushes the immediate; no fix found (s031_1462 0124).
- (session 3) A local `struct { int y2, pad1, k, pad2, pad3; } v;` pins memory locals to fixed
  slots below the scalars (fixed 0124's [bp-8]/[bp-0c]) but 6.00A adds 8 unused frame bytes.
- (session 3) 6.00 kept small-loop counters in bx (no register variable; 433c even shares bx
  between two loop counters, spilling one to [bp-4]); 6.00A uses si/di - not reproduced (42ac 433c).
- (session 3) Helpers (copied from the session scratchpad, set SCR=dir for temp files):
  jd.py FILE FUNC = fdiff that skips jump tables and masks string/CONST addresses (counts
  DIFFLINES; use it to see real code diffs when data placement is still off); mk.py FILE FUNC
  < [[(old,new),..],..] writes v.txt variants ('$PRE'/'$POST' add lines around the function,
  '\b...' = regex, member-safe); pj.py FILE FUNC [K] scores v.txt variants with jd; fvj.py
  whole-file variants; lst.py FILE SUFFIX = /Fc listing; asm.sh IMAGE_FRAME OFF = compact asm;
  dstr.py LO HI = DGROUP strings.
- (session 3) Strings decide _DATA placement: a file only gets "DATA" right when every
  function's literals are present in order, so stub unwritten functions with their literals
  (e.g. `free_buffer(D_867a, "b")`) to make the others match early.
- (session 4, e1d) **Unnamed loop-invariant temps are hoisted CSEs, not locals.** A byte stored
  once per outer iteration and then tested from memory in the inner loop (`mov [bp-8],ax` ...
  `test word ptr [bp-8],ax`) is the loop-invariant `F[who][i]` that 6.00A hoists itself; a
  named `v = F[who][i];` gets cached in a register instead. Write the array access in the
  condition (s031_1462 433c, 42ac). Same for a `[bp-8]` that is re-pushed later as a call arg:
  it was `D_867c` CSE'd by MSC, not a named `k` (s031_1462 0124: removing the named k also
  fixed the frame size and the position of the other CSE temp).
- (session 4) **Loop counter cached in bx with `mov cx,bx` in `i*3`**: a `for` loop gives
  `mov ax,bx; mov cx,ax`, a `do { } while (++i < N);` loop gives the original
  `mov ax,bx; mov cx,bx` (s031_1462 42ac MATCHED).
- (session 4) 6.00A removes a dead constant store to a local (`b = 0x80;` before
  `for (..) { b = 0; ...}`) even with /Oe /Og /Ol /Oc off; constant propagation also folds
  `if (b)`, `n += b - 0x80`, `if (i) goto inner`. 6.00 kept it. Keeping it with volatile, `&b`,
  union or struct stops the loop caching of b. Unsolved (433c).
- (session 4) **Region-cached loop variables get no frame slot** when never spilled (42ac's
  i in bx). `b = m` with `volatile int m` still gives b its own slot when the copy is outside
  the inner loop.
- (session 4) **Two locals sharing a slot -> reuse one variable** (inverse of the 6.00 unused
  slot problem): s030 1f30 had r and k sharing r's slot; `for (k = r, r = 0; r < 16; r++)`
  (k keeps the old r, r is the new counter) keeps the local count and gives every value its
  own slot (MATCHED). Adding a 4th local also matched but flipped operand orders elsewhere.
- (session 4) Removing/adding any local in s030 overland flips 29fa/2dd6/2ac8/2d44; a
  dummy `extern int D_fff0;` at any top-level position made things worse, not better.
- (session 4) A ternary written twice `(x + (c ? 16 : 24)) / 16` ... `% 16` IS CSE'd by 6.00A
  (result in cx, sbb form); `c ? x + 16 : x + 24` written twice is not (s030 16c6).
- (session 4) 14b6: stack slot order follows the order of the first stores after DSE; a dead
  early `type = 0;`, `(void)type;`, `type = type;` don't change it.

## e2

- s043 17e5 (palette fade) is compiled with DIFFERENT flags: `/AM /Zp1` only (no /G2 /Gs /Ox):
  `push bp; mov bp,sp; mov ax,N; call __chkstk`, locals in memory, no loop opts. fmatch resolves
  6.00A's __aFchkstk. Local slots there follow the name-hash order (e1 lmap.py/hname.py) -> FILE MATCH.
- s043 1810 is hand-written asm (`push si; push di` order, `xor ah,ah`, `shl ax,1; adc dx,0` rounding). Skip.
- **MSC emits an initialised global AFTER the literals of the next function.** DGROUP order in
  s043_1462: quality-name literals (table initialiser), then function 0000's literals, then the
  globals D_4ec4..D_4fce, then 0100's literals. So the globals are defined before function 0000.
  Char-array globals are word aligned (single 0 pad bytes between strings).
- fmatch limitation: near pointers in initialised data pointing into the module's own _DATA
  (`char *D_4fce[] = {"very poor", ...}`) are not relocated in the data comparison -> "DATA
  DIFFERS" although the bytes are right (checked by replacing the table with the int values).
- fmatch limitation: `__AHINCR` (huge pointer increment, `add word ptr [bp-2],1000h`) is unresolved.
  `char huge *p; *p++ = c;` is the right source (s043_1462_09c8).
- `fprintf(f, fmt, ..., vec_1c1b(), D_e4, D_e2, ...)`: a call with no args in the middle of an
  argument list. Do not mistake the following pushes for the inner call's arguments (the caller's
  single `add sp,1ah` gives it away).
- FP_SEG/FP_OFF on a far lvalue must use `unsigned far *`, or the segment is silently dropped.
- s043_1462_0000: register var reused (D_7760 kept in di, later the event counter in di) needed
  `#pragma optimize("a", on)`.
- One local reused for unrelated jobs (`nr` = nread for _dos_read AND the low byte of a count)
  shows up as one shared stack slot; separate locals make the frame 2 bytes bigger.
- **Inline far strcpy/strcat in /AM** (`les di,[bp+8]` ... `push ds; push es; pop ds; pop es`
  ... `rep movsw`): call strcpy/strcat WITHOUT a prototype (no <string.h>) and pass the far
  pointer directly: `strcpy(D_ee5b, pic)` with `char far *pic`. 6.00A has no intrinsic
  _fstrcpy, and redeclaring strcpy with far params is an error (C2086). (s043_150b_000a)
- **Tables addressed from one base pointer are one struct.** s043_150b_020e fills three int
  tables (e8a7/e8d9/e925) through one strength-reduced pointer `di` ([di-32h], [di], [di+4Ch]);
  separate externs give absolute addresses. `extern struct blocks { int ofs[25]; int x[25];
  unsigned char y[26]; int w[25]; } D_e8a7;` reproduces it.
- `x = *pic == 'x' ? 15 : 0;` vs `if (*pic == 'x') x = 15; else x = 0;` give different code
  (the if/else reuses AX=0 left by an inline strcat). Try both.
- `unsigned char *out` (instead of char *) stopped 6.00A from reusing AL=0 for
  `out[0] = 0; flag = 0;` (original: two immediate byte stores). (06fe)
- Uninitialised register local read at loop entry (`mov si,[bp-2]`) came from the decompiler;
  the original simply has `int key;` used uninitialised on a path; no extra local. (2826)
- `#pragma check_stack(on)` around a function: `push bp; mov bp,sp; mov ax,N; call __chkstk`
  (resolved by fmatch). (06fe)
- fmatch compares jump tables after relocation only when the function lands at its original
  offset: while earlier functions in the file are stubs, a switch function with identical code
  shows DIFF at its table (2d3a).
- ptry.py with 4 jobs sometimes prints bogus "MATCH" with an empty summary (compile clash);
  re-run with 1-2 jobs before trusting it.
- `#pragma optimize("a", on)` fixed the pointer-global reuse in s043_150b_1f40
  (`mov bx,[104h]` once for two stores through D_0104).

## session 2 (s043 150b: 15 -> 20/28)
- **Inline _asm functions are still partly optimised** (cross-jumping, `sub ax,ax`, no
  reloads with `#pragma optimize("a", on)`), only locals stay in memory. 04c4 matched with
  the original `_asm {}` block + pragma a + hash-named unused locals (`int uaj[4]` etc.) for
  the 0x14 frame (slot order = name hash buckets ascending, like e1's big functions).
- `a->c = b->c = cond1 && cond2 ? 0 : g;` distributes the 0 store in 6.00A; the original's
  `sub ax,ax / mov ax,[g]` then two stores came from a NESTED ternary:
  `x = y = c1 ? c2 ? 0 : g : g;` (04c4).
- A ternary argument `f(.., c ? 15 : 0)` compiles to `sub ax,ax; push ax`; the original's
  `push 0 / push 0Fh` with one shared call is `if (c) f(.., 15); else f(.., 0);` (cross-jumped).
- **Return through DX** (`mov dx,[bp-2]; mov ax,dx` / `mov dx,si; mov ax,dx`): a result
  variable assigned in a loop: `r = -1; for (..) if (..) { r = i; break; } return r;` (10fc).
- `_fstrcmp`/`_fstrcpy` need full far prototypes so a near `char *` arg gets `push ds`.
- Index temp computed BEFORE the argument pushes (`mov si,ax; push ..; mov bx,ax`): put the
  assignment inside the index of the earliest-evaluated arg: `f(.., a[k], .., b[k = expr], ..)`
  (1142). The opposite (orig computes it in order after earlier pushes) is NOT reproducible
  this way (1d56: `f(D_0104, 3, y = who*0x28+1, D_a626)` is always hoisted).
- Struct copy `rep movsw cx=7Bh` = assignment of 0xf6-byte layout structs (D_a778 = D_e96e).
- BIOS shift state: `*(unsigned char far *)0x00400017L & 2` gives `mov bx,40h; mov es,bx;
  mov bx,17h; test es:[bx],2`.
- Mouse globals reloaded into CX for every compare (`mov cx,[6eec]; cmp ax,cx`):
  `*(volatile int *)&D_6eec` in the hit-test macro (1744).
- `push ax` of a known-zero AX after `or ax,ax; jne` (2a04 `vec_1fef(1000, 0)`) came out
  right from plain C here.
- Down-counter `[bp-0a]` loop with `mov [bp-0e],0` (dead i = 0) = `for (i = 0; i < n; i++)`
  where i is unused in the body; the counter moves the other loop vars between SI/DI (2a04).
- A dead literal for an empty function: `char *msg; msg = "...";` keeps the string in _DATA
  and the function is still a bare `retf` (1f3e).
- An unused `extern int X_1;` after the new declarations restored 06fe's `[bx+si]` order
  (adding declarations elsewhere had flipped it).
- `#pragma optimize("g", off)` on 203a keeps ya/x0 in memory like the original.
- Small-function slot order (2a04, 1744): neither names nor declaration order changed it;
  aggregates are placed by first use (first-used highest). Unsolved.

## session 3 (s043 150b: 20 -> 23/28; s039 1462: 10 -> 34/43)
- **Some functions were compiled with global optimisation off** (the 1992 compiler probably ran
  out of memory in big functions): loops not strength-reduced, `for` loops emitted as
  `jmp test; inc; test: ...`, parameters compared in memory, only `register` locals in SI/DI.
  `#pragma optimize("egl", off)` (2252) or `("eglc", off)` (13a0, 203a) reproduce it. Try these
  combos (build/agent/r6w0/psweep.sh does not include them) whenever the original looks "dumb".
- In egl-off functions the register params/locals keep declaration order; a register value
  that seems to come from nowhere (`mov si,[bp+8]` never) can be a *reused parameter*:
  203a's x is the unused 2nd parameter declared `register int x` (SI), w the 3rd (DI).
- **Stack slot below all user locals = compiler CSE temp, not a variable.** 2a04 `dim` at
  -1c: write `D_2776[month]` twice (no local) -> MSC spills the CSE. 1744 q.a/q.b at -0c/-0a:
  `D_e9c4[D_e96f]` repeated instead of the struct. Same for s039 0132-style frames.
- Hash slot renaming (build/agent/e3/hren.py) worked straight away on 2252's ten locals.
- `strcat(s, c ? "\x8f" : "\x80")` gives `mov ax,lit; ...; mov di,ax`; the original's
  `push si; mov di,lit` per arm + shared strcat tail is `if (c) strcat(s, A); else strcat(s, B);`.
- s039 0ebe: a dead register value computed by an if-chain survives only when it is used in
  a dead store: `if (rand(100) < pct) pct = 1; else pct = 0;` (pct unused afterwards) keeps
  the chain and drops the compare.
- s039 1e90: `else if (k == 1 || k == 2) v = k;` vs two arms `k == 1` / `k == 2` (cross-jumped)
  decide where the shared `mov si,dx` block lands.
- s039 1de6: `mov cx,[bp-2]` (uninitialised read, enter 2) = `int i; for (i = 0; i < 5; i++)
  if (D_9c69[i].status) n++;` strength-reduced to a pointer (i's slot remains).
- 6.00A drops switch cases whose target equals the default from the jump table (also
  `case 0: case 1: case 2: break;` + common tail); the original kept them (s039 1c22). Open.
- Embedded assignments in argument lists: 6.00A evaluates/stores them before the other
  argument pushes; the original keeps strict right-to-left order (s043 1d56, 203a x1m/y1m,
  s039 026a `d = min(d = max(d = rand(..)-1, a), skill)`). Looks like 6.00 vs 6.00A.
- fmatch cannot resolve `mov ax, DGROUP` (DS restore after a far-segment loop) in a module
  without own _DATA (s039 0504: "unresolved: segment _DATA").
- s039 constants 0x270e/0x270f/0x2238/0x540 pushed as 2c4e's `hours` are durations
  (9998, 9999, 8760, 1344 hours), not strings (emit_asm annotates them as strings).

## e3

- **6.00A turns `while (x > 23) { y++; x -= 24; }` into division** (`q = x/24 (div); y += q;
  x += q * -24` or `imul +24; sub`), for globals and far-pointer fields; not for near-pointer
  fields. The game's 6.00 did it for some (near pointer, s029 08ae) and not others (globals,
  s029 05c8). To keep a loop on globals: `while (*(volatile int *)&g > 23) { y++;
  *(volatile int *)&g -= 24; }`. To get the division form on near-pointer fields write it out:
  `d->day += (unsigned)d->hour / 24; d->hour += (unsigned)d->hour / 24 * -24;` with
  `#pragma optimize("a", on)` (CSE of the quotient across the store).
- **The canonical sign of `x -= q*K` (`imul K; sub` vs `imul -K; add`) depends on whether the
  constant -K appears elsewhere in the file** (constant table shared between functions?):
  s029 09ea loop form gave `imul 18h; sub` only after the explicit `* -365` spelling in two
  other functions was replaced by `while (d >= 365) d -= 365;`. Symptom: a function matches
  alone/when any other function is deleted, but not in the full file.
- **Loop-carried local kept in memory** (`mov [bp-2],ax; cmp ax,[g]; jge; L: mov ax,[bp-2] ...`):
  `volatile int dm;` plus the guard-duplicated shape
  `if ((dm = T[i]) < g) do { ...; } while ((dm = T[i]) < g);`. When the use should be a
  memory operand (`sub di,[bp-2]`), make only the store volatile:
  `(*(volatile int *)&dm = T[m]) < d` and read `dm` plainly.
- **`for (i = 0; i < n; i++)` gives `jle` guard, original `je`**: declare `unsigned i`.
- **clamp with `mov dx,63h; mov ax,dx` returns**: `r = v; if (r > 99) r = 99; else if (v < 0)
  r = 0; return r;`.
- **cmp operand direction for two params** (`cmp dx,bx` with dx=hi): copy to a local first,
  `r = hi; if (r < v) return hi; r = lo; if (r > v) return lo; return v;`.
- Char buffer overlapping locals: `enter 14h` with flags at [bp-2]/[bp-4] and a buffer at
  [bp-14h] means the buffer is 16 bytes, not 20.
- `strcat(buf, c ? "a" : "b")` loads the string into ax; the original's `mov di,str` comes
  from `if (c) strcat(buf, "a"); else strcat(buf, "b");`.
- **Register variable written to its stack home once** (`mov [bp-4],si` with [bp-4] never read,
  e.g. root 0265 0882/09b2 number formatting): 6.00 stored a register local to its home slot.
  6.00A does it when a volatile access happens while the local is live: keep the pointer in
  a separate local `p = buf;` and write the next plain store through a volatile cast,
  `*(volatile char *)&started = 0;`. A `char * volatile p0 = buf` instead also flushes the
  *parameter* home (`mov [bp+0Ah],si`). (09b2 matched with `p0 = buf; p = p0;`.)
- **`lodsb; push ax`** for `f(font, *p++)`: `for (n = 0, p = s; *p; ) n += f(font, *p++);`
  with a separate pointer local (a param pointer `*s++` gives `mov di,si; mov al,[di]`).
- `*cur > '@'` compiled `ja`/`jb`: the pointer is `unsigned char *`.
- root 0617 (file I/O) was not compiled like the rest: `add sp,N` before `pop bp` at the
  end of frame functions with no locals, `mov ax,1; push ax` (no /G2), loop vars spilled back
  after loops. Default flags `/AM /Gs /Zp1` (= /Ot) come closest; the epilogue `add sp` form
  is found nowhere else in the game and 6.00A always turns it into `mov sp,bp` - probably an
  older compiler (5.x).

## s029 0e76 (party/event core), session e3

- Work file src/wip/s029/s029_0e76.c (started from build/agent/gen_engine output; splice new
  functions in with build/agent/e3/splice.py WIP NEW.c, which also replaces the prototype line).
  build/agent/e3/slotonly.py WIP lists functions whose only difference is the CONST slot of a
  far table (`mov es,[7eXX]`): the slots are handed out in order of first use in the file, so
  they only line up once the earlier users (02ea, 0792, 118e, 1260, 199c, 1fb8, 20c6, 21aa,
  2930, 3f30) are all ported. Original slot order: 7E18 (02ea), 7E1A (0792), 7E1C 0x75f table
  (118e), 7E1E 0x82c bits (1260), 7E20 F_2e38_10be (199c), 7E22..7E2A five tables (1fb8),
  7E2C F_290e_2937 saints (20c6), 7E2E F_290e_207f (21aa), 7E30 F_2e38_1b2c events (2930),
  7E32 0x290f table (3f30).
- Member records: `extern char D_9c69[][0x80]` and one alias extern per field
  (`extern char D_9cb2[][0x80]` = +0x49 attributes, D_9cb9 +0x50 attribute maxima,
  D_9cc0 +0x57 skills, D_9c9e +0x35, D_9cd3 +0x6a item count, `extern unsigned D_9c67[][0x40]`
  = age in days at [-2]). The base symbol decides the strength-reduced loop pointer: 1128
  needed `D_9c69[i][0x6a]` (pointer based on the record, status at [si-6Ah]).
- **Dead-looking frame slots** in 6.00 code come from locals whose stores 6.00A drops.
  `*(volatile T *)&x = value;` with plain reads afterwards reproduces "store to [bp-x],
  keep using the register" (s029 0e76 01c0, 18f2, 4442 partly; s029 1367 0938/0982).
  A volatile store also flushes register variables to their stack homes (`mov [bp+6],di`
  for a reassigned parameter in 01c0).
- Uninitialised `mov cx,[bp-2]` before a party loop (007c, 1972, 11fa) is what 6.00A also
  emits for a plain `for (i = 0; i < 5; i++) if (D_9c69[i][0] ...)` loop: write the loop.
- `i = 0; n = 0;` vs `n = i = 0;` decides `sub di,di; mov si,di` vs `sub si,si; mov di,si`
  (37b8, 4db6, 4e0c).
- The compare direction against a global (`cmp ax,cx` vs `cmp cx,ax` in 164a/16fe/174a)
  flipped with the position of `extern unsigned char D_9c1d;` among the prototypes
  (tools/porting/declpos.py found a position) - and flips back whenever lines are added
  above; redo at the end.
- Original bugs worth knowing for mods: 0b84 computes the 0..99 clamp of a new skill value
  and stores the unclamped value; 01c0 sets D_99eb for every skill (missing braces); 1696 and
  179c store 2*a instead of a+b; 18a8 discards the clamp of v; 39f0 reads event -1 when no
  event exists; 4db6/4e0c return garbage when the scan reaches slot 300.
- **Struct local initialised with constants and then pushed** (`mov [bp-0Ah],17h ...` then
  `push [bp-4]`, s029 0e76 2930) plus an int local kept in memory (`mov [bp-2],ax; inc ax`):
  `#pragma optimize("e", off)` around the function and `volatile struct {..} x;` (6.00A
  constant-propagates plain struct fields and enregisters the int).
- `fseek(f = fdopen(h, "r"), (long)city * 0x26e + 1, 0);` reproduces the interleaved
  `push 0; ...imul...; push dx; push ax; call fdopen; mov si,ax; push si; call fseek`
  (s029 0e76 4e68, city loader). String literals are placed in code-generation (push) order:
  `fopen("darkland.dsc", "r")` puts "r" first.
- `#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))` for far-table elements; with a near
  `unsigned *` cast 6.00A silently drops the segment override.
- root 0617 (fileio.c, compiled without /Ox): stack slots follow the 6.00A *name-hash* order
  even in small functions (bucket = sum of name chars % 16, ascending from [bp-2] down; the
  struct/array locals take part). Renaming locals fixed 00d4/011e and the frame of 000e.
  build/agent/e3/hperm.py FILE FUNC tries every top-down order of a function's locals.

## e3 tools (build/agent/e3)
splice.py (insert/replace functions + prototypes in a wip file), slotonly.py (DIFFs that are
only CONST-slot loads), hperm.py (try all hash-bucket orders of a function's locals by
renaming), hren.py (rename locals for a given top-down order), lay.py (stack slot map per
variant), prag_all.py (try optimize pragmas on every non-matching function), ptry/tv/apply as
in e1. PT=xx env var gives ptry separate temp files when two runs overlap.

## What s029 1367 computes (FILE MATCH, src/c/s029/s029_1367.c)
- clamps 0..99 (000a), 0..100 (0028), -100..100 (0046), lo..hi (0066), max (0084), min (0094)
- money {florins, groschen, pfennigs}, 1 fl = 20 gr, 1 gr = 12 pf: set+normalise (00a4),
  value in pfennigs (00f2), add/subtract amounts (0130/0180), copy (01ce), add/sub money
  (01ec/023e), multiply (0290), compare (02d4), format "3 florins, 2 groschen and 1 pfennig"
  (0376), add a per-member cost for every active member (054e, 0594)
- calendar: D_00e0 hour, D_00e2 day, D_00e4 month (0-11), D_00e6 year, D_2776 days per month
  (no leap years); advance by hours (05c8: hourly hook 0e76_255a, vec_1f95 for >2 h, monthly
  hook 0e76_281c, event expiry 0e76_3f30, reseeds rand) and by days (0680); wait until an
  hour (0716 via 086a); daylight 5:00-18:59 (072a); 3-hour time-of-day bands (0744/0796/07ee);
  date struct copy/set/normalise/add-hours (088a/08ae/09ea); month and day of a day-of-year
  (0938/0982); day of year (09cc); absolute day number = year*365 + 13288 + ... (0a6c);
  days since/until/between dates (0a96/0b1c/0ba4, the last capped at 32000)
- UI: message box + key (0c30), "Chance of success is X% to Y%" (0c52; D_906a 0 = numbers,
  1 = words D_4fce[x/20], 2 = off), random personal names from first-name/surname tables seeded
  by D_9c4a + seed (0db4).
- Event fill (0e76 3034, 21 parameters stored into one far record) needed
  `#pragma optimize("a", on)`; with it 6.00A also switches DS to the far segment for the whole
  function like the original. build/agent/e3/keep.py NEW.c splices into the best-so-far copy
  and keeps the result only if the total match count goes up (useful because of the flips).

## End of session e3: s029 1367 FILE MATCH; root 0265 text.c 8/10; root 0617 fileio.c 8/29;
## s029 0e76 84/143 (+6 differing only in a CONST slot).

## s029 0e76, session e3 (continued): all stubs written, 109/143, _DATA and CONST match
- **Declaration order of locals decides compare operand order** (`cmp di,si; jg` vs
  `cmp si,di; jl` between two register locals, 4c04): permute the declaration lines
  (build/agent/e3/dperm.py FILE FUNC tries all orders). Renaming does not help.
- **`for (j = 0; j < cnt[who]; j++)` without a surrounding `if (cnt > 0)`** gives the
  original `cmp byte [..],0; jle out` guard + down-counting loop (0c76/0dd8/0f54); an explicit
  `if` produces a duplicated test, a do-while produces max(n,1) code.
- Same file, two far-table access forms: `mov es,[CONST slot]` for memory operands vs
  `mov ax,SEG; mov es,ax` / `push SEG` inside DS-switched loops and for pushed addresses -
  both come from the same extern; no source change needed.
- **switch cases with own epilogue vs `jmp` to the shared one**: `return;`/`return x;` in a
  case gives an inline epilogue, `break;` jumps to the end (1a8e, 1dfe).
- Struct externs named `F_2e38_0000_3` etc. do not resolve: use `extern char far * far
  F_2e38_183e[];` (seg_offset names). Every far extern gets one CONST slot in first-use order.
- An unreferenced initialised word in _DATA between string literals (26e6) is a file-scope
  `static int pad = 0;` placed *before the previous function* (it is emitted after the strings
  of the function compiled before it).
- `if (c) y = A; else y = B;` (not the ternary) gives `cmp; jge L2; then-arm; jmp; L2:` when
  6.00A otherwise inverts the ternary (4534). A value used once after a volatile cast store can
  still be pushed from AX if the store is inside the argument: `f(*(volatile int *)&y = ...)`.
- `(x % 7 == 0)` then `f(.., 0, ..)`: 6.00A also pushes DX (the known-zero remainder) - write
  the literal 0 (4180).
- Whole function unoptimised (`cmp ax,0`, `jne +3; jmp`, `push cs` replaced by a far call to a
  same-file function, no CSE): `#pragma optimize("", off)` + hash-ordered local names
  (hren.py) - 3fb2 matched this way.
- `#pragma optimize("e", off)` fixed 32aa (frame `enter 2` + `add sp,4` before `inc ax`).
- Blockers: 4650 clears two far arrays with inline `rep stosw` through CONST slots - 6.00
  had a far memset intrinsic, 6.00A has none (`memset` cannot be redeclared far, `_fmemset`
  is not an intrinsic); 33a2..3742 + 2e74 + 3c28 choose `mov bx,ax` vs `mov bx,si` after
  `(i = f()) == -1` the other way round from 6.00 (no source form found); 1a8e case 1:
  6.00 laid out the 6-way rating ternary as if/else (`cmp; jg next; mov ax,K; jmp store`),
  6.00A always inverts it; 255a attribute caps preload the current value into DL
  (`mov al,[si+6]; mov dx,ax`) - no source form found.
- Helpers added: ours.py (our compiled code of one function, optional start address for
  functions with jump tables), bytesdiff.py (differing byte offsets: jump tables that only
  differ by a constant offset are fine), slots.py (CONST slots used per function, ours vs
  original), dperm.py (declaration-order permutations), fmatch_all.py (fmatch copy that
  shows every function). splice.py now carries multi-line comments and `static int pad_`.

## s029 0e76, session e3 round 2 (109 -> 127/143)
- **`mov bx,ax` vs `mov bx,si` after `(i = f()) == -1`** depends on the exit statement and loop
  form, not on the compare: `do { if ((i = f(k, i)) == -1) return 0; ... } while (++i < 300);`
  gives `mov bx,ax` (33a2..3742, 4aa4-style); `break` gives `mov bx,si`. For `mov bx,si` with
  an in-loop return (3c28, 3b62): `for (i = 0; i < 300; i++) { if ((i = f(k, i)) == -1)
  return -1; ... }`. 3cde: `return n;` in the -1 test and no return at the loop end.
- **Dead-looking `mov [bp-x],reg` stores / `les si,[bp-4]` reloads are 6.00 CSE temps**:
  write the expression again instead of using a local. 4442 (`F[i]->type0` three times ->
  `mov [bp-2],bx` + `mov ax,[bp-2]; push ax; push ax`), 324c (`F[i]->kind == k || k == 0x1c
  && F[i]->kind == 8`), 3180/31e6 (`F[i]` everywhere instead of `e = F[i]`: the far pointer
  spill + `les si,[bp-4]`), 2116 (`D_9cb8[who][0]` twice -> `[bp-6]`). Look for this first
  when a frame slot is written once and never/rarely read.
- 01c0: `if ((c = *(volatile char *)&t = D_9cc0[who][i]) > D_99e6) D_99e6 = c;` with
  `char t; char c;` (store to the slot, keep the value in AL).
- splice.py replaced `name();` prototypes with full ones; the prototype text changes the
  symbol-table state and flips 15a2/184a/1696/179c/4378/255a. `NOPROTO=1` keeps the old
  prototype line. Removing/adding locals in one function flips 4378/4c04: re-run dperm on
  them after each change (scratch fixflip.sh logic: dperm, apply best).
- 1600/1800: the divisor in SI is `n = n ? n : 1; return D_x = sum / n;` (reuse n, no d).
- 118e: `j = 0; if (D_9cd3[who][0] > 0) do {...} while (++j < D_9cd3[who][0]);` (no cnt
  local: the `[bp-4]` byte is the CSE spill). Dropping cnt flipped 184a; an unused
  `extern int D_fff0;` right before 184a flips it back (dummy externs are a tuning knob:
  inserting one anywhere before 15a2 flips 15a2/184a, two flip 1696/179c, three 4378/255a).
- 4b08 rewritten (who/k/d/h, `if ((t = 99 - f(who, k)) < d) d = t;`), same length now;
  left: loop pointer slot [bp-2] vs [bp-4] and the else arm (`cmp ax,[d]; jl; mov si,[d]`):
  6.00A enregisters d, or with a separate s it keeps d in SI.
- 013a/18f2: 6.00 keeps `cur` in AX across a branch target (`cmp ax,di` after `jl`) and
  indexes with `mov bx,si; shl bx,7` (who kept). The CSE-spill form (no cur local) gets
  `mov bx,si` and the `[bp-x]` spill but 6.00A hoists the load before max. Not solved.
- 2d5c/2e74: `cmp si,-1` after `mov [bp-2],si` with i dead: 6.00A always emits `inc si`.
  2e74 with `#pragma optimize("a", on)` gets the `inc ax / mov ax,-1` 3144 test right; its
  param e sits in DI only inside the loop in the original (pushed from memory later).
- Scratch tools (copied as build/agent/e3/r2_*): r2_mc.py compiles a mini C file with
  6.00A and disassembles it (0.1 s - test idioms with only the needed externs), r2_var.py
  HEADER VARIANTS [regex] runs many variants, r2_score.py scores them against an original
  listing (ours.py ... o), r2_run.sh splices NEW.c files into copies of best in parallel,
  r2_fixflip.sh FILE FUNC.. applies the best dperm order, r2_promote.sh keeps a full file if
  its count is higher.
## End of round 2: 130/143.

## e4

- **Compare operand order vs a global:** original `mov ax,[bp-144h]; cmp [9082h],ax; jg`
  (stack value in AX, global as the memory operand) while `extern int D_9082;` gave
  `mov ax,[9082h]; cmp [bp-144h],ax; jl` whatever the source order or extern position.
  `extern struct { int v; } D_9082;` and `D_9082.v` fixed it: the global is a member of a
  bigger struct in the original. (s038 16cd 0404)
- `out[n++] = x;` with the last increment dead still reproduces "store &out[n] to a temp slot
  then reload into DI" (`mov [bp-4],ax ... mov di,[bp-4]`); `out[n] = x;` gives `lea di` and
  an extra `mov ax,ss; mov es,ax`.
- Buffer that the strength-reduced loop pointer points into the middle of (`lea ax,[bp-28Ah]`
  = &tab[0].name, score at [bx-6]): write the loop with an index `tab[i].field`, not a struct
  pointer. Unused index slot: shrink the neighbouring buffer by 2 to keep the frame size.
- `fread(buf, n, 1, fp = fopen(..))` reproduces `push ax ... mov si,ax` interleaving.
- Mouse globals: `extern volatile int D_6ef0, D_6eec, D_6eee;` (as in s034 saints.c) gives
  `test byte ptr [6ef0h],1`; with `extern char D_6ef0` 6.00A emits `mov al; and al,1` inside
  && chains. With volatile, `v = expr; switch (expr)` reads twice: write `switch (v = expr)`.
- Two consecutive `g1 = 0; g2 = 0;` as immediate stores (6.00 style) instead of
  `sub ax,ax; mov [g1],ax; mov [g2],ax`: make the first `*(volatile int *)&g1 = 0;`.
- String literal order in _DATA reveals the source order of switch cases (code is laid out
  in compare order, literals in source order). s037 1e26 0172: case 0x1454 before 0x1051.
- BIOS tick: `*(int far *)0x0040006cL` gives `mov bx,40h; mov es,bx; mov bx,6Ch; mov ax,es:[bx]`.
- build/agent/e4/fd.py FILE _xxxx: difflib-aligned diff of all differing regions (fmatch -v
  stops 30 lines after the first difference). build/agent/e4/dg.py A B dumps DGROUP strings.
- **Big frames (s037 1c7d 0d24, 24 locals, 0x170 bytes)**: build/agent/e3/hren.py with the
  target top-down order (15 names) plus the rest as group 15 (bottom first) reproduced the
  whole layout in one go. CAUTION: hren also renames matching words inside string literals
  ("r" -> "rj" in fopen(.., "r")); check literals afterwards.
- `fread(buf, 1, 0x15, f)` vs `fread(buf, 0x15, 1, f)`: check the push order of the two
  middle arguments.
- 36 identical `fread(&dummy, 2, 1, f)` calls were unrolled in the source (no loop).
- `fseek(f, (long)(unsigned)(x * 0x22a), 1)` gives `push 0` for the high word (no cwd).
- `F[i] = (char far *)((unsigned long)alloc_buffer(..) << 16);` gives "call; mov es:[bx],0;
  mov es:[bx+2],ax" (FP_OFF/FP_SEG stores separately schedule differently).
- `p = D_a885 + 1; p++;` gives `lea ax,[bx+1]; mov dx,es; inc ax` (far pointer).
- `extern unsigned far _dos_findfirst(char *, unsigned, struct find_t *);` (a prototype)
  stopped 6.00A from pushing a register holding 0 for the attribute argument.
- Global vs global compare direction (`mov ax,[8c50]; cmp [8c54],ax`): moved the extern
  lines with tools/porting/declpos.py; each global may need its own position (D_8c50, D_8c54,
  D_8c4c in s037 1c7d) and a later move can break an earlier fix: re-run fmatch on all.
- Pointer loops over word arrays that read `mov di,[bp-2]` (uninitialised) before the loop
  are plain index loops `for (i = 0; i < 5; i++) if (D_a2d7[i]) ...` (s037 1c7d 09c6).
- `g1 = g2 = 2;` emits the stores in reverse order of the original `mov [g1],al; mov [g2],al`:
  write separate statements.
- Unsolved (6.00 register allocation): menu loops with a key in [bp-x], the hovered row in DI
  and the key copy in SI (s038 1462 0000, s037 1c7d 0002): 6.00A puts row and k into the same
  register. Tried volatile key, struct key, register, unsigned, merged k/row, extra locals.
- Unsolved: 6.00 constant temp `mov ax,10h; mov [bp-2],ax; push ax; push 12h; push ax` where
  [bp-2] is also a strength-reduced loop temp (s037 1c7d 030a).

## Session (s038 1462 02f8, s037 18c8)
- Tools added here: `dec16x.py IMAGE FRAME -o out.c` = dec16 that never gives up (bp+index ->
  LIDX(), unhandled instrs -> /* ASM */ comments, table jumps in goto mode, FARIDX/MK_FP) - pseudo
  C for functions dec16 refuses (s038 02f8, s037 058e/1738/22be...). `cases.py` traces jump-table
  arms through cross-jumped tails (al tracking) and prints the stores per case; `gen02f8.py` built
  the 1/2/3 colour cases from it. `fd2.py FILE _xxxx CTX nobp` = fd.py ignoring [bp+N] offsets;
  `dump.py FILE _xxxx LO HI [orig]` disassembles the compiled function; `ren.py FILE FUNC order
  'a b c'` renames locals into hash buckets 0,1,2.. (top-down frame order).
- s038 02f8: 6.00 constant temp `mov ax,8; mov [bp-10h],ax; push ax ... push [bp-10h]; imul [g]`
  is reproduced by writing the temp: `f(.., (D_e7da << 3) + 0x16, 0x71, h = 8, ..); ...
  f(.., h * D_e7da + 0x16, 0x71, h, ..)`  (may also fix s037 1c7d 030a's `mov ax,10h; mov [bp-2],ax`).
- `mov si,[bp+6]; mov ax,si; cmp ax,...` switch prologue = `register int key` parameter.
- Store groups by constant (`mov al,1ch; mov [a],al; mov [b],al; mov al,1ah; ...` in any address
  order): 6.00A regroups by itself; write the stores in the order the code shows.
- Unused filler locals ARE allocated by 6.00A: frame gaps = unused `int`/`long` locals placed
  with hash-ordered names (02f8: fillers at -2, -8, -12h).
- Jump-table bodies: 6.00A orders by smallest label; an out-of-order group in the original =
  split the group, the low labels as their own identical body (cross-jumped into the later
  copy). Does NOT give "later case jumps back into earlier case" (s037 372a case 9 -> case 8).
- 6.00 vs 6.00A, unsolved: then-block of an if inside a for loop placed before the loop increment
  (s038 02f8 'A'); CSE of i*2 in DI across far calls; cross-jump keeps the EARLIER copy of a
  `f(..); g(); break;` tail in 6.00 (later in 6.00A); global operand order `cmp [a67e],ax`.
- s037 1c7d 030a retried with the 02f8 trick: `f(.., i, 0x12, i = 0x10)` - store is dead there
  (i unused later) so 6.00A drops it; `*(volatile int *)&i = 0x10` keeps it but taking &i kills
  the strength reduction of the ids/D_9c69 loop (frame 0x26 vs 0x2a). Idea not tried: the
  original's [bp-2]/[bp-4] may be USER variables (byte offset `k += 2` < 10 and a member pointer
  `p += 0x80`), which would let `k = 0x10` share the slot.

## e5

Tools: build/agent/e5/sa.py (compact asm), mkstruct.py (struct with pads from offsets),
ds.py (DGROUP string at offset), build/agent/e1/fdiff.py FILE FUNC (full aligned diff).

## s020 (level loader)
- 05e6 MATCH. 06b6: chained memsets: original `mov bx,X/mov cx,n/mov di,bx/rep stosw` for every
  memset after the first (6.00A: `mov dx,X/mov cx,2n/mov di,dx/rep stosb`), and the first is
  preceded by `mov [mem],ax` not `sub ax,ax` -> cport.compat600 does not rewrite these
  (tool blocker). Loop pointers si/dx are loaded before the memsets in the original.
- 0000: 3-byte struct copy is `mov cx,1; rep movsw; movsb` in the original, 6.00A emits
  `movsw; movsb` (compiler-version blocker?). Far-pointer temps in byte-copy loops.

## Far segment 20a5 (s136) = communal far data (TOOL BLOCKER)
- Original code loads 20a5 variables with `mov ax,SEG FRM_s136_20a5; mov es,ax` (immediate), not a
  CONST slot. 6.00A emits exactly that only for *communal* far variables (`int far x;` without
  extern / initializer -> COMDEF). `extern int far x;` and `int far x = 0;` both use a CONST slot.
  tools/omflink.py rejects COMDEF (OMF record b0, "unsupported OMF record b0"), and a `static`
  far variable leaves an unresolved segment X5_DATA. So every function touching 20a5 is blocked
  until omflink/fmatch resolve COMDEF names like F_20a5_1f44.
  Files using 20a5: s017, s018 1ef8, s022 (03e4), s023, s024.
- Same files also have unused CONST slots in the original (s022: three 286b slots at 7dee..7df2
  though only 7df2 is read): the original compiler made a slot for the address-of uses too
  (F_286b_0212 / 0222 pushed as `push SEG`).
- `mov al,[x]; and al,7Fh; je` (no test peephole): `if ((unsigned char)(x & 0x7f) > 0)`;
  every plain `if (x & 0x7f)` form becomes `test byte ptr [x],7Fh` in 6.00A (s021 03fe).
- `push 0Fh; jmp L / push 0Ch; L: push..; call` = if/else of two whole calls (s021 0080).
- 6.00A pushes `ax` for a 0 argument when AX is known 0 after an inline strcat
  (`xor ax,ax` of repne scasb): `clamp(v += a, 0, 99)` -> `push 63h; push ax` (s021 0080).

## s021 FILE MATCH (src/c/s021/s021_1ef8.c)
- Fill loop with an int count kept in memory and a register pointer compared against
  `&list[2]`: source was `list[cnt].a = ..; list[cnt].b = ..; cnt++; if (cnt == 2) break;`
  (MSC strength-reduces list[cnt] and replaces the cnt test by the pointer compare);
  writing an explicit `q++` pointer swaps SI/DI and spills.
- The `mov [bp-2],9EEBh / mov di,[bp-2]` loop-pointer home store is emitted by 6.00A too.
- Module-owned initialised data before the literals: `char D_22ce[4] = {0}; char D_22d2[3] = {3,4,5};`.

## s025: 6/7 (0000 blocked)
- 09a6 needed `#pragma optimize("a", on)` (found by build/agent/r6w0/psweep.sh; "w" on also works).
- 0c02: `char tag[8]; strcpy(tag, "EB");` (a `= "EB"` initialiser zero-fills the rest in 6.00A).
- 0000 (enemy placement, 2470 bytes) is fully decoded in src/wip/s025/s025_1ef8.c, but the
  original keeps the slot index in SI across inline block copies by wrapping them in
  `push si; push di; ... rep movsw ...; pop di; pop si` (struct copy of the 0x80-byte record and
  the inline strcpy). 6.00A never does that: it splits the register variable's live range
  (copies it to AX/BX around the move), so every later use differs. Looks like a
  6.00 vs 6.00A difference -> blocker. Grep for `push si` / `push di` right before
  `lea di`/`mov si` + `rep movs` in the asm to spot this.
- Combatant record really starts at D_9c55 (0x80 bytes; party D_9c69 = +0x14 status):
  +4 x, +6 y, +0x0e info ptr (&D_cab4[kind], 0x90-byte kind records), +0x10, +0x14 status,
  +0x16 kind (animation slot 0..6), +0x18, +0x19/+0x1a facing, +0x1b, +0x1c own index,
  +0x22 weapon, +0x4a, +0x51, +0x7e item count. Enemy type records (0x122 bytes) are read
  from level%d.enm into D_b899 (first 0x80 bytes = initial combatant record).

## s015: 4/7 (src/wip/s015/s015_1ef8.c)
- 01a8: a `char` parameter tested with `(row & 1) == 1` gives `dec al`; declaring it `int row`
  (prototype too) gives the original `and al,1; cmp al,1` and also made the callers' `i + 1`
  CSE (`lea ax,[si+1]; mov si,ax`) match.
- 01c6/0708/0c0e (combatant / attribute / skill dumps, rows generated by build/agent/e5/gen015.py):
  the original stores the initial value of an eliminated induction variable to its home
  (`mov [bp-4],0`, and in 0c0e `mov [bp-2],3Ch; mov si,[bp-2]`), shares that slot with the
  label-loop pointer, and puts the record pointer in DI / column in SI. `*(volatile int *)&j = 0`
  gives the store and the frame size, but then the slot order (pointer temp at -2 instead of -4)
  and SI/DI stay swapped; pragma sweep and renaming did not help.

## Combat data learned (for mods)
- 13 combatant records of 0x80 bytes at D_9c55 (party 0..4 = the party records, enemies 5..12).
  D_a6b4[13] side/group (0x9d = ?), D_c2bf[13] enemy class (2 = animal: no weapon text),
  D_c200[13] anim id, D_ca45/ca75/ca8b[7] W/C/D animation buffers per kind slot,
  D_1166/D_1174[7] anim file name parts, D_c80f/D_c831 frame counts, D_d645/D_d664 start x/y.
- Enemy type records (level%d.trn 0xd6 bytes -> D_a896[] 0x122 bytes, count D_a895) are written
  to level%d.enm by s020 0000 and read back one at a time into D_b899 by s025 0000.
- Skill raise in combat (s021 0000): chance (100-v)/2 or /4, amount f(D_22d2[diff]) + D_22ce[diff].

## e6

Tools here: calls.py (asm call-arg extractor), fd.py (fmatch full diff, no 30-line cap),
st.sh, vt.py/psweep.sh (variants/pragmas, tmp in e6/tmp), vl.py (6.00A /Fc slot map per
variant), spl.py (splice a function), hren/getf/hname copies.

## Data structures
- Battle map (0x33fc bytes, file level0.flr; static copy F_20a5_0000, loaded maps via the
  far pointer D_c103): cell[40][40] unsigned long (bit31 = empty, 0x80000000 init), hgt[40][40]
  bytes at 0x1900, 0x1f42 int[5], 0x1f4c/0x1f4e start pos, 0x1f50/51, 0x2184 char nobj,
  0x218c obj[] 14 bytes {int type (7/8 doors); ...; char x4; int x6 door id}, 0x24c6 int list[]
  (row<<8)+col, -1 terminated, 0x3326.., 0x3386 pts[] 10 bytes, 0x33c8 name. Level files
  (lc_*.) have an int group count right after the map (D_c103 + 0x33fc).
- D_dadd is a `_segment`; D_db33 (tile description), D_db2d (enemy groups, 0x1e bytes:
  pos, pos2, x04, x06, x07, type[3] at 8, count[3] at 0xb), D_db2b (0xd6-byte monster
  records) are `_based(D_dadd)` pointers.  `extern _segment D_dadd; struct x _based(D_dadd) *D_db33;`
  reproduces `mov es,[dadd]; mov bx,[db33]`; pass to far params as `(char far *)D_db33->f`.

## Findings
- `struct g far *g = D_db2d + D_db05;` (far pointer local from a based pointer) + pragma "a"
  OFF gives the 6.00 `mov dx,ss:[dadd]; mov ds,dx` + `ss:` globals form (s012 08a8).
- In s012/s013 small functions, 6.00A slot order = order of stores in the code (chain
  `d = c = 0x10` decides which of c/d gets the higher slot); names/decl order don't matter.
- CSE temps: `if (F.cell[row][col] & ..) { F.cell[row][col] = ...; F.hgt[row][col] = ...` gives
  the 6.00 spilled far pointer + index temps (enter 6) - do not write explicit pointers.
- int params masked with `& 15` load as bytes (`mov dl,[bp+x]; and dx,0Fh`).
- Prototype with `unsigned char` params -> `mov al,[x]; push ax` without extension.
- Unsolved s012 028a: only stack-slot layout differs (enter 0x16 ok).
- Unsolved s013 0338 family: in the obj loop 6.00 reloads the far pointer with
  `les bx,[bp-0eh]` every iteration (and then knows es after the loop); 6.00A (pragma a on)
  keeps es. Everything else in these 8 functions matches.

## Status at end of session (2026-10-09)
- s012 1432 (src/wip/s012/s012_1432.c): 5/6. 028a: code and frame right, only stack slots
  differ (6.00A layout not driven by names or declaration order here; volatile vars group
  together - see bf028a.py/bf2.py output). Everything else matches.
- s013 1432: 0000 matches; the 8 template functions (0338 ... 0c1e) differ only in the
  `les bx,[bp-0eh]` reload in the door loop (see above); 0098 drafted (D_160c low byte must be
  a word load: `*(volatile unsigned *)&D_160c & 0xff`; shift loop `while (m >>= 1) x++` is not
  rotated in 6.00; frame 2 bytes short).
- s011 1432: 1794, 1f5c, 209e match; 16d2 right (jump table waits for earlier lengths); 1fb2
  only name[]/lv slot swap (array must be above the far pointer - 6.00A puts arrays below);
  1d94/1968 close (6.00 keeps the found-object pointer offset in DI with only its segment
  stored, 6.00A stores both or neither). Not started: 0000 029e 0484 05c6 096a 17fa 1a2a 1bc0
  1cb4 1ddc.
- s002 0e76: not started beyond dec16 (17ea matches). Its _DATA has large initialised tables
  (string pointer tables at 0x114e.., byte tables 0x1182..0x11e6) between the literals, and
  several CONST slots (7d22.. for 2e38/20a5 externs) that only line up with all functions
  written. 1750/1762/1a7a/168a/0806 are short and straightforward once the data is declared.
- Useful: `#pragma optimize("l", off)` reproduces 6.00's per-iteration store of a far loop
  pointer's segment (s011 1d94) but then adds a jump to the loop test.
