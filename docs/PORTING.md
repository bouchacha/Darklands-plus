# Porting functions to matching C

The goal of a port is C source that Microsoft C 6.00A compiles back to the exact original bytes. A module that matches leaves `DARKLAND.EXE` byte-identical, so "matching" is checked by a tool, not by judgement.

## Workflow

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
