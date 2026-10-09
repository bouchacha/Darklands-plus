# Porting functions to matching C

The goal of a port is C source that Microsoft C 6.00A compiles back to the exact original bytes. A module that matches leaves `DARKLAND.EXE` byte-identical, so "matching" is checked by a tool, not by judgement.

## Automatic decompilation (start here)

`tools/dec16.py` decompiles a whole original source file to C, and `tools/cardgen.py` runs it over
many files and checks every function:

    python3 -I tools/cardgen.py --images s044-s135 --jobs 2      # writes build/gen/<img>/<img>_<frame>.c
    python3 -I tools/fmatch.py build/gen/s070/s070_19af.c -v --func _0394   # per-function diff
    python3 -I tools/cardgen.py --promote                         # copy fully matching files to src/c

`fmatch.py` compiles the file and compares each function with the original *after relocating it to
the original address*, so one function that is too long does not hide the others. A file can only
go into the build when every function in it matches (`FILE MATCH`), because MSC calls functions of
the same file with `push cs; call near`.

To fix a generated file by hand, copy it out of `build/gen` (it is regenerated), edit the
non-matching functions, and when fmatch says `FILE MATCH` put it in `src/c/<img>/`. A file there
takes precedence over the generated one.

## Workflow (manual ports)

1. **Pick a unit.** Port one *original source file* at a time. MSC calls a function defined in the same file with `push cs` / `call near`, and anything else with `call far`, so a partial file can't match. `build/analysis/srcfiles.json` lists the inferred files; usually one file is one segment region, i.e. one `build/asm/<image>/<image>_<frame>.asm`.
2. **Create `src/c/<image>/<name>.c`** starting with:
   ```c
   /* place: s135 1838:0000 */          /* image, frame:offset of the first function */
   /* flags: /AM /G2 /Gs /Ox /Zp1 */     /* the flags the game was built with */
   ```
   Write the functions **in their original order**. The file has to cover a contiguous run of functions from the placement address. The C ends where the compiled code ends, and the rest of the region stays assembly.
3. **Check:** `python3 -I tools/cmatch.py src/c/<image>/<name>.c`. It compiles with MSC 6.00A under emu2, resolves calls and data references, and prints a side-by-side diff from the first difference. Use `-v` for the whole function.
4. **Confirm:** `python3 -I tools/build.py <DARKLAND.EXE> --skip-analysis` must print `BYTE-IDENTICAL`.

Reference material:
- `build/asm/...`: original code. Strings are annotated in comments.
- `build/ghidra/out/c/<image>.c`: Ghidra pseudo-C. It's a starting point only and often wrong in detail.
- `build/analysis/functions.json`: callers, callees and strings per function.

## Naming conventions (resolved by the linker)

| in C | means |
|---|---|
| `sub_s043_150b_000a()` | an unported function: the assembly label of the same name |
| `D_9085` | DGROUP offset 9085, e.g. `extern char D_9085[];` or `extern int D_e48a;` |
| `F_2caf_0000` | far data at 2caf:0000, e.g. `extern struct world far F_2caf_0000;` |
| a named function in `symbols/names.txt` | its real name; libc functions are `strcpy`, `_fstrcpy`, `sprintf` and so on |

Unknown prototypes are fine. Declare what you need at the top of the file, e.g. `void far sub_s029_1367_05c8(int);`. All functions in this program are `far`, except a few hand-written assembly helpers.

## Idioms learned so far (MSC 6.00A, /AM /G2 /Gs /Ox /Zp1)

- **Flags.** `/AM /G2 /Gs /Ox /Zp1`. `/Zp1` matters: structure members are packed, e.g. a far pointer at offset 0xa47.
- **String literals** go into the module's `_DATA`. The build finds where they sit in DGROUP from the original code. So write the literal (`strcpy(D_e3d4, "$Witcd42");`) when the original code points at a module-private string. Strings shared by many modules ("Success is assured." at `D_4f48`) are global arrays: `extern char D_4f48[];`.
- **Far globals.** Code like `mov es,[DGROUP word]` / `push es:[0a49h]` / `push es:[0a47h]` is MSC accessing an **external far variable**. The DGROUP word is the module's `CONST` slot holding the variable's segment. Write `extern struct world far F_2caf_0000;` and use `F_2caf_0000.field`, with a struct whose field sits at the right offset (`/Zp1`).
- **`strcpy`** with a non-literal source becomes inline `repne scasb` … `rep movsw` / `adc cx,cx` / `rep movsb`. With a literal source it becomes a fixed `movsw`/`movsb` copy whose result goes through `ax` (`mov ax,dst / mov di,ax`).
- **4-byte return values** (`mov ax,[bp-4] / mov dx,[bp-2]`) are `struct res { int a; int b; }` returned by value. The card scripts often return an uninitialised one.
- **`char buf[50] = {0};`** gives `mov byte ptr [bp-x],0` followed by `rep stosb` of the rest. `= ""` is different: it copies a literal.
- **Chained stores** of the same value (`mov ax,1 / mov [a],ax / mov [b],ax`) come from separate statements `a = 1; b = 1;`. The optimizer reuses `ax`.
- **`r.b = D_ee8c = D_ee6c;`** gives `mov ax,[ee6c] / mov [ee8c],ax / mov [bp-2],ax`.
- **Loops filling tables** get strength-reduced: a pointer in `bx` is compared against the end address.
- **`memset`** with a constant even size: the game's compiler used `rep stosw` with the address in `bx`, while 6.00A emits `rep stosb`. `tools/cport.py:compat600()` rewrites the 6.00A form after compiling, so just write `memset(p, -1, 8);`.
- **Inline `_asm`** turns off optimisation for the whole function. Avoid it unless the original function is clearly unoptimised.
- **Stack locals.** MSC lays out locals in its own order, not declaration order. If offsets differ, try reordering declarations or changing their types.
- `push cs` / `call near ptr X` means X is defined in the same C file. A plain `call far` to a function in the same segment means X lives in another source file.
- **`/Oa`** (assume no aliasing) is needed by some modules; try it when MSC reloads a value it already has in a register.
- **Dead stores keep their loads.** `mov al,[x]; cbw` with AX overwritten unused comes from `r.b = x; r.b = -2;`. `mov ax,-1; mov [bp-4],ax` (a constant through AX used once) comes from `r.b = -1; r.a = -1;` with r.b overwritten later.
- **Cross-jumping.** MSC merges identical tails of `if`/`else` arms and `switch` cases, even in the middle of an argument list (`push 5; jmp L` / `push 6; L: call f`). Write the arms out in full (`if (c) f(5); else f(6);`); MSC merges them again.
- **Callback results.** `struct res t; t = (*D_ea14[i])(); ... g(t.a);` spills only `t.a` to the stack; the frame is still 4 bytes.
- **Struct return through AX.** `mov dx,[bp-2]` before `retf` with AX from a call: `r.a = f(); r.b = ...; return r;`.
- **switch** shows up as `dec ax`/`sub ax,N` chains or `jmp cs:[bx+T]` jump tables on a copy in AX. Compares directly on a variable (`cmp si,-1`, `or si,si`) are `if` chains.
- **Functions returning garbage.** An `int` function may fall off the end; AX then holds whatever was last computed. Do not write `return f();` for a call whose result is merely left in AX — the call would move.
- **Locals layout** follows usage (reference counts, first use, size), not declaration order, so reproducing the code's uses reproduces the layout.
- **Equal-arm ternary.** `call f; sub ax,ax; add ax,4; mov [g],ax` is `g = f() ? 4 : 4;`. Ternaries in general: `cmp ax,1; sbb r,r; and r,K; add r,M` is one `v = f() ? M : K+M;`, never split statements.
- **Return of an assignment.** `mov ax,K; mov [g],ax; (epilogue)` is `return g = K;`. `mov [g],K` followed by `mov ax,K` is `g = K;` at the end of each arm plus one `return K;`.
- **Void switch functions.** `dec ax; je ...` chains leave AX behind; that is garbage, not a return value. Write a `void` function with `switch (f()) { ... }` (often with `default:` first in the source).
- **Case ranges.** `dec ax; jge L; jmp out; L: dec ax; jle C12` are consecutive case labels `case 1: case 2:` on one body; no default.
- **Jump-table switch bodies** are laid out in case-value order whatever the source order, but string literals go into `_DATA` in source order: order the cases like the strings.
- **Values in DX across compares.** `mov dx,[g]; cmp dx,6; je; cmp dx,8` comes from a temporary: `t = g; if (t == 6 || t == 8)`.
- **Uninitialised locals** are real: `mov di,[bp-2]` with [bp-2] never written is a local that is read without being set (or set on one path only).
- **Evaluation order of two calls in one expression** depends on MSC's internal state (symbol table), not only on the source. If everything else matches, extra (unused) `extern` declarations before the function can flip it.
- **`#pragma optimize("a", on)` … `#pragma optimize("", on)`** around one function reproduces aliasing-dependent code (values kept in registers across far stores, loop-invariant loads) where `/Oa` for the whole file breaks other functions.
- **128-byte party records**: `shl bx,7; [bx-6397h]` is `struct member D_9c69[]` with 128-byte elements, not a char array.
- **MSC 6.00 vs 6.00A.** The game was built with a slightly different 6.00 release. Known differences: memset with `rep stosw` (handled by `cport.compat600`), and constants kept in a register for later pushes (`sub ax,ax; mov [x],ax; push ax; push ax`; 6.00A pushes immediates, but `D_x = 0; t.b = 0; f(.., t.b, t.b)` with a dead struct field reproduces it).

## Round 3 findings (details: docs/PORTING_NOTES.md, helpers: tools/porting/)

- **Declaration position decides order.** Call order inside one expression, operand order of sums of globals and the direction of compares (`cmp si,[g]` vs `cmp [g],si`) follow the position of the prototypes/extern lines in the file (MSC's symbol table), not the source order. Move `extern` lines or insert unused `extern int D_ffxx;` lines; `tools/porting/declpos.py` searches positions automatically.
- **One expression vs. temporaries.** `push args; mov si,ax` with the store scheduled after the next pushes means the original was one big expression.
- **Pragmas that help:** `#pragma optimize("a", on)` (reuse of a global in DX, register vars kept), `#pragma optimize("e", off)` / `volatile` (keep a local on the stack), `#pragma optimize("l", off)` (loop shapes).
- **Frames:** `enter N,0` bigger than the visible locals means a local assigned a constant and later passed, a `struct res` whose `.a` is returned, or a user temp left in AX.
- **Shared tails:** write every switch/if arm out in full; MSC cross-jumps identical tails itself.
- **Strings:** split hex escapes (`"\xff" "egain"`); a string pushed by many calls but stored once is `char *m = "...";`.
- **Twins:** many card functions are copies of functions in other files with constants changed; grep src/c first.
- **Known unsolved (probably MSC 6.00 vs 6.00A):** party-loop bottom test `cmp si,[a67e]; jl` in some files (sometimes fixed by extern position), DS reload placement in s033 alcform, `long` return clobbered by idiv.
- **Round 4:** `#pragma optimize("a", on)` is the first thing to try when SI/DI are swapped or a register variable is spilled. A `char * volatile p` loop pointer fixes "loop pointer takes DI first". In large functions MSC 6.00A orders stack slots by a hash of the local's *name* (sum of characters mod 16), so renaming locals reproduces the original frame layout (build/agent/e1/lmap.py prints the slot map). The source line layout (`case 1: return K;` on one or two lines) decides which duplicate block survives cross-jumping.
- Tool support: `__aFchkstk` (6.00A's stack probe, functions compiled with stack checks) resolves to the game's `__chkstk`; memset after `or ax,ax / jne` is converted too.
- **Round 5:** `*(volatile int *)&x` at one access site gives 6.00-style memory loads/stores without making `x` volatile everywhere (dead stores to parameters, re-reads). A temp that only lives in a register: `union { int d; char t; } u;`. dec16's `goto` tails should be rewritten as full arms. dec16 turns comparisons through the `long` helper `sub_s029_1367_00f2` into bogus `switch (... >> 16)`: declare it `long far` and compare normally. A file's `_DATA` only lines up once every function's string literals are present in order — stub unwritten functions with just their literals. Jump tables of later functions stay "different" until every earlier function has the right length.
- **Exceptions:** some original functions were compiled with different flags (s043 17e5: `/AM /Zp1`, stack checks, no optimisation) or contain inline `_asm` (s043 150b `04c4`) — then `_asm` in the C is legitimate. root 0617 (file I/O) ends frames with `add sp,N; pop bp`, which 6.00A never emits: probably an older compiler; leave it in assembly.
- Tool support: `__AHINCR` (huge-pointer increment) resolves to 1000h; near pointers inside initialised data (tables of strings) are relocated by fmatch.
- **Round 6–7:** a stack slot below all real locals that is written but rarely read is usually the old compiler's temporary for a repeated expression — write the expression twice instead of using a local. `#pragma optimize("egl", off)` / `("eglc", off)` for functions that look unoptimised. How a `-1` test exits (`return 0;` vs `break;`) decides `mov bx,ax` vs `mov bx,si`. Twins: build/agent/r7w0/twins.py finds matched files with the same code shape.
- **Known 6.00 vs 6.00A limits** (leave these functions; the rest of the file can still be documented): an assignment inside a call argument is evaluated before the other pushes by 6.00A; far memset intrinsic (`rep stosw` into a far array) missing in 6.00A; SI/DI saved around inline block copies; `cmp si,-1` vs `inc si` for dead values; some out-of-line ternary arms.
- Tool support: COMDEF records (`int far x;` communal variables) are read; memset chains after `mov [mem],ax` stores are converted.
