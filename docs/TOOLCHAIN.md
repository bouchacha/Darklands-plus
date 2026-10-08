# How DARKLAND.EXE was built

These are findings about the GOG release, version 483.07 (26 February 1993).
`DARKLAND.EXE` is 1,675,637 bytes, MD5 `0d51cc9458a6724e1bd34a9fe0d81e7f`.

## Summary

| item | finding | evidence |
|---|---|---|
| Compiler | Microsoft C 6.0 (6.00 or 6.00A) | run-time banner "MS Run-Time Library - Copyright (c) 1990, Microsoft Corp"; crt0 layout (`__astart`, `__chkstk`, `__nullcheck` with the 0x55 checksum, `_amsg_exit`) |
| Memory model | Medium (`/AM`): far code, near data | `sprintf`, `fread`, `fwrite` take near pointers; `malloc` jumps to `_nmalloc`. Far data is used explicitly: `_fstrcpy` is the most-called function, with 1,998 call sites |
| CPU target | 80286 (`/G2`) | `enter`/`leave` (5,872 / 8,205 uses), `push imm`, `imul r,r/m,imm`, shifts by an immediate |
| Stack checks | Mostly off (`/Gs`) | `__chkstk` has only 7 callers |
| Optimisation | On | frameless leaf functions; register-allocated locals |
| Same-module calls | `push cs` / `call near` | MSC emits this when the callee is in the same source file. It marks module boundaries |
| Linker / overlays | Pocket Soft **RTLink/Plus**, internal overlays | "RTLink CACHE" and `eov0001..eov0009` messages; section table in the manager segment |
| Hand-written assembly | About 41 KB in 170 functions | port I/O, flag-returning routines, `loop`/`lods`, near `ret` |
| Other libraries | PKWARE Data Compression Library 1.02 (in s003, battle save/restore); EMS/XMS drivers; MicroProse driver interface to `MGRAPHIC.EXE` (same architecture as F-15 Strike Eagle II) | strings and call graph |
| Floating point | None | no FPU-emulator interrupts (34h–3Dh) anywhere |

## Executable layout (RTLink/Plus)

```
file offset   contents
0x000000      MZ header + 1,463 relocations (header padded to 6,144 bytes)
0x001800      root load module: 59,222 bytes, segments 0000-0e75 (resident code)
0x00ff60      136 overlay sections. Each is a relocation list, padding to a paragraph, then the image.
0x17d0b0      section 136: preloaded far data and DGROUP (frame 321a). The last section is not padded.
```

Memory at link time, in paragraphs:

```
0000-0e76   root: game shell, C run-time library (06a1), RTLink manager (09c0) + cache (0d14)
0e76-20a5   overlay area: a tree of sections, three levels deep (siblings share memory)
20a5-41fa   section 136: far data segments 20a5, 286b, 28c0, 28f5, 290e, 2caf, 2dae, 2e38; DGROUP 321a
41fa-       stack
```

The section table lives at `09c0:0b7a`, with 137 entries of 0x12 bytes:

| word | meaning |
|---|---|
| w0 | load segment, at link time |
| w1 | constant 0x1763 (file descriptor id) |
| w2, w3 | file position in paragraphs = w2 \| (w3 & 0xff) << 16; w3 >> 8 = flags (04 = demand-loaded, 01 = preloaded) |
| w4 | size in memory, in paragraphs |
| w5 | relocation count |
| w6 | parent section (0xffff = top level) |
| w7 | section id (index + 2) |
| w8 | stored size, in paragraphs (smaller than w4 when the section has BSS) |

Each relocation entry is `(offset, frame)`, where frame is the frame of the segment that contains the location.

### Overlay calls

A call into an overlay that isn't an ancestor goes through one of 753 ten-byte vector stubs at `09c0:17e3–354d`:

```
e8 xx xx        call rtl_vector_handler   (09c0:052b: load the section if needed)
ea oo oo ss ss  jmp far target            (relocated)
nn nn           section number            (0xffff = root)
```

Sibling overlays can call each other. The manager evicts the caller and reloads it on return, using its "reload stack". For example, every card script calls into the rules toolkit in s039.

## What this means for a matching decompilation

* About 94% of the code by size is MSC 6 C output. The F-15 Strike Eagle II reconstruction, also MicroProse with MSC 5.1, shows that instruction-level matching is achievable for this kind of code.
* The **one missing input is the compiler itself**: Microsoft C 6.00/6.00A (CL, C1, C2, C3 and the medium-model libraries). It can't be downloaded from this environment. Once a copy is placed in `dos/msc600/`, the exact flags can be pinned down empirically on a few known functions. Start with `/AM /G2 /Gs /Ox` and adjust.
* RTLink/Plus isn't needed. `tools/omflink.py` takes its place and already rebuilds a byte-identical executable.
* Port one original module (one region) at a time. MSC only emits `push cs` / `call near` for callees in the same compilation unit, so a function that calls a not-yet-ported neighbour in the same segment will differ only at that call site until its whole module is in C.
