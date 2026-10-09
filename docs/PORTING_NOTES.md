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
