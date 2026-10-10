/* place: root 047c:0066 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: NOT C (e2, 2026-10-09): 0/87.  The whole region is hand-written assembly (MASM);
 *   nothing here is MSC output, so this file only documents it and stays in src/wip.
 *   Evidence: functions start at odd offsets with no EVEN padding (MSC pads every function
 *   to an even length with nop); `push bp; mov bp,sp ... pop bp; retf` epilogues where 6.00A
 *   always emits `leave` (124f, 1386: compiled C gives `leave`, see below); register calling
 *   conventions between functions (args in AX/BX/CX/DX: 0d20, 0d62 -> 05e6/05d1, 0f46/0f60 ->
 *   0ffe, 1174/118b jump into 11ab's tail, 1120 jumps into 1174); `call far` into the same
 *   segment (1293 -> 0597, 12c3 -> 052e; MSC would use push cs/call near); DS loaded with
 *   another segment (03da, 040b, 0362: SEG 28c0); in/out/int 21h/int 33h, iret handlers
 *   (0d41, the INT 8 handler stored after 00a9), self-modifying code (cs: stores in 0066, 048a, 12dd).
 *   Contents (graphics driver interface):
 *     0066/00a9  install/remove the INT 8 timer handler (PIT reprogrammed by 02a2/0347/0358,
 *                vertical retrace calibration 0329)
 *     0362/03da/040b  palette cycling slots (SEG 28c0 tables, DAC read via 3C7h/3C9h)
 *     048a       dispatcher used by the driver thunks while the mouse pointer is shown
 *     04c7..0791 ~60 driver entry thunks: `jmp far 0:0` (patched when the driver is
 *                loaded) or `call 048a` + far address; data, not code
 *     07ce/0876/08c8/08f9/0912  load the graphics driver (DOS 4B03 overlay load, own file
 *                read, relocation of the thunk table from the driver header, free, int 21h
 *                wrapper); 0933 prints a load error and exits (int 21h 4C99h)
 *     0952/09e6/0b94/0d20/0d41/0d62/0da7/0de6  clipped spans, lines (Cohen-Sutherland with
 *                register arguments, divide-overflow INT 0 hook 0d41) and points
 *     0efe..1070 compressed picture stream decoding (F1FC read pointer, refill via EEB8)
 *     1120/1174/118b/11ab  integer distance approximation, sqrt of x*x+y*y(+z*z)
 *     11f6..1386 mouse (int 33h install/remove, pointer show/hide 1293/12c3/12dd, event
 *                handler at 1395 with DS = DGROUP)
 *   The four functions below are written as the C they look like, to show the difference:
 *   124f/1386 differ only in the epilogue (pop bp vs leave) and 1293/12c3 in the far calls
 *   (MSC: push cs/call near within one file) - i.e. they are assembly too. */

extern int D_6ef4;
extern int D_6ef6;
extern int D_6ef8;
extern int D_6eec;
extern int D_6eee;
extern char D_6a81;
extern void far *D_6efc;

void far sub_root_047c_0066(void) { }  /* asm */
void far sub_root_047c_00a9(void) { }  /* asm */
void far sub_root_047c_02a2(void) { }  /* asm */
void far sub_root_047c_0329(void) { }  /* asm */
void far sub_root_047c_0347(void) { }  /* asm */
void far sub_root_047c_0358(void) { }  /* asm */
void far sub_root_047c_0362(void) { }  /* asm */
void far sub_root_047c_03da(void) { }  /* asm */
void far sub_root_047c_040b(void) { }  /* asm */
void far sub_root_047c_048a(void) { }  /* asm */
void far sub_root_047c_04c7(void) { }  /* asm */
void far sub_root_047c_04ce(void) { }  /* asm */
void far sub_root_047c_04d5(void) { }  /* asm */
void far sub_root_047c_04dc(void) { }  /* asm */
void far sub_root_047c_04e3(void) { }  /* asm */
void far sub_root_047c_04ea(void) { }  /* asm */
void far sub_root_047c_04ef(void) { }  /* asm */
void far sub_root_047c_04f8(void) { }  /* asm */
void far sub_root_047c_0504(void) { }  /* asm */
void far sub_root_047c_0514(void) { }  /* asm */
void far sub_root_047c_051b(void) { }  /* asm */
void far sub_root_047c_052e(void) { }  /* asm */
void far sub_root_047c_053e(void) { }  /* asm */
void far sub_root_047c_0545(void) { }  /* asm */
void far sub_root_047c_054c(void) { }  /* asm */
void far sub_root_047c_0553(void) { }  /* asm */
void far sub_root_047c_055a(void) { }  /* asm */
void far sub_root_047c_055f(void) { }  /* asm */
void far sub_root_047c_0568(void) { }  /* asm */
void far sub_root_047c_056f(void) { }  /* asm */
void far sub_root_047c_0576(void) { }  /* asm */
void far sub_root_047c_0584(void) { }  /* asm */
void far sub_root_047c_058b(void) { }  /* asm */
void far sub_root_047c_0597(void) { }  /* asm */
void far sub_root_047c_059e(void) { }  /* asm */
void far sub_root_047c_05a5(void) { }  /* asm */
void far sub_root_047c_05ae(void) { }  /* asm */
void far sub_root_047c_05b5(void) { }  /* asm */
void far sub_root_047c_05bc(void) { }  /* asm */
void far sub_root_047c_05ca(void) { }  /* asm */
void far sub_root_047c_05d1(void) { }  /* asm */
void far sub_root_047c_05d8(void) { }  /* asm */
void far sub_root_047c_05df(void) { }  /* asm */
void far sub_root_047c_05e6(void) { }  /* asm */
void far sub_root_047c_05ed(void) { }  /* asm */
void far sub_root_047c_0617(void) { }  /* asm */
void far sub_root_047c_061e(void) { }  /* asm */
void far sub_root_047c_073d(void) { }  /* asm */
void far sub_root_047c_0744(void) { }  /* asm */
void far sub_root_047c_0767(void) { }  /* asm */
void far sub_root_047c_0783(void) { }  /* asm */
void far sub_root_047c_078a(void) { }  /* asm */
void far sub_root_047c_0791(void) { }  /* asm */
void far sub_root_047c_07ce(void) { }  /* asm */
void far sub_root_047c_0876(void) { }  /* asm */
void far sub_root_047c_08c8(void) { }  /* asm */
void far sub_root_047c_08f9(void) { }  /* asm */
void far sub_root_047c_0912(void) { }  /* asm */
void far sub_root_047c_0952(void) { }  /* asm */
void far sub_root_047c_09e6(void) { }  /* asm */
void far sub_root_047c_0b94(void) { }  /* asm */
void far sub_root_047c_0d20(void) { }  /* asm */
void far sub_root_047c_0d41(void) { }  /* asm */
void far sub_root_047c_0d62(void) { }  /* asm */
void far sub_root_047c_0da7(void) { }  /* asm */
void far sub_root_047c_0de6(void) { }  /* asm */
void far sub_root_047c_0efe(void) { }  /* asm */
void far sub_root_047c_0f46(void) { }  /* asm */
void far sub_root_047c_0f60(void) { }  /* asm */
void far sub_root_047c_0f7f(void) { }  /* asm */
void far sub_root_047c_0fc8(void) { }  /* asm */
void far sub_root_047c_0ffe(void) { }  /* asm */
void far sub_root_047c_1070(void) { }  /* asm */
void far sub_root_047c_1120(void) { }  /* asm */
void far sub_root_047c_1174(void) { }  /* asm */
void far sub_root_047c_118b(void) { }  /* asm */
void far sub_root_047c_11ab(void) { }  /* asm */
void far sub_root_047c_11f6(void) { }  /* asm */
void far sub_root_047c_123f(void) { }  /* asm */
/* mouse hot spot / pointer setup: three words */
void far sub_root_047c_124f(int x, int y, int p)
{
    D_6ef6 = x;
    D_6ef8 = y;
    D_6ef4 = p;
}

void far sub_root_047c_1266(void) { }  /* asm */
/* show the mouse pointer (driver thunk 0597) unless hidden already */
void far sub_root_047c_1293(void)
{
    if (D_6ef4 && !D_6a81) {
        sub_root_047c_0597(D_6eec - D_6ef6, D_6eee - D_6ef8, D_6ef4);
        D_6a81 = 1;
    }
}

/* hide the mouse pointer (driver thunk 052e) */
void far sub_root_047c_12c3(void)
{
    if (D_6ef4 && D_6a81) {
        D_6a81 = 0;
        sub_root_047c_052e();
    }
}

void far sub_root_047c_12dd(void) { }  /* asm */
void far sub_root_047c_1320(void) { }  /* asm */
void far sub_root_047c_1354(void) { }  /* asm */
void far sub_root_047c_1386(void far *p)
{
    D_6efc = p;
}

