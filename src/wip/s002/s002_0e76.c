/* place: s002 0e76:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status (e6, 2026-10-09): dec16 output only (17ea matches). Battle entry/exit. The module's
 * _DATA holds initialised tables (near string-pointer tables at 0x114e.., byte tables
 * 0x1182..0x11e6) between its literals; CONST slots 7d22.. (segments 2e38/20a5) only line up
 * once all functions are written. See build/agent/e6/NOTES.md.
 */

struct res { int a; int b; };
extern char D_12a6[];
extern char D_4276[];
extern int D_c943;
extern int D_c945;
extern int D_c947;
extern int D_c949;
extern int D_c94b;
extern int D_c94d;
extern int D_c94f;
extern int D_c951;
extern int D_c953;
extern int D_c955;
extern int D_c957;
extern int D_c959;
extern int D_c95b;
extern int D_c95d;
extern int D_c95f;
extern int D_c961;
extern int D_c963;
extern int D_c965;
extern int D_c967;
extern int D_c969;
int far _fmemset();
int far fclose();
int far fopen();
int far fread();
int far fseek();
int far sprintf();
int far stricmp();
void *memset(void *, int, unsigned);
int far sub_root_0000_25f2();
int far sub_root_0000_2646();
int far sub_root_047c_0066();
int far sub_root_047c_00a9();
int far sub_root_047c_0568();
int far sub_root_047c_0783();
int far sub_root_047c_0791();
int far sub_root_047c_07ce();
int far sub_root_047c_08c8();
int far sub_root_047c_08f9();
int far sub_root_047c_12c3();
int far sub_root_05ba_0004();
int far sub_root_05d9_019a();
int far sub_root_0617_011e();
int far sub_root_06a1_2052();
int far vec_1adb();
void far sub_s002_0e76_0000();
void far sub_s002_0e76_0806();
void far sub_s002_0e76_08fe();
void far sub_s002_0e76_0c8c();
void far sub_s002_0e76_168a();
void far sub_s002_0e76_1750();
void far sub_s002_0e76_1762();
int far sub_s002_0e76_17ea();
void far sub_s002_0e76_1830();
void far sub_s002_0e76_1a7a();

/* sub_s002_0e76_0000: not decompiled: es:[reg] unknown es=const(64,) ptr=const(108,) */
void far sub_s002_0e76_0000(void) { }

/* sub_s002_0e76_0806: not decompiled: memory via unknown si */
void far sub_s002_0e76_0806(void) { }

/* sub_s002_0e76_08fe: not decompiled: mov es from non-slot */
void far sub_s002_0e76_08fe(void) { }

/* sub_s002_0e76_0c8c: not decompiled: base+index addressing */
void far sub_s002_0e76_0c8c(void) { }

/* sub_s002_0e76_168a: not decompiled: memory via unknown si */
void far sub_s002_0e76_168a(void) { }

/* sub_s002_0e76_1750: not decompiled: far pointer with constant segment and variable offset */
void far sub_s002_0e76_1750(void) { }

/* sub_s002_0e76_1762: not decompiled: read of undefined dx */
void far sub_s002_0e76_1762(void) { }

int far sub_s002_0e76_17ea(void)
{
    D_c949 = -0x3748;
    D_c94f = -0x3748;
    D_c957 = -0x3748;
    D_c965 = -0x3748;
    D_c943 = -0x370b;
    D_c945 = -0x370b;
    D_c94b = -0x370b;
    D_c951 = -0x370b;
    D_c955 = -0x370b;
    D_c95d = -0x370b;
    D_c95f = -0x370b;
    D_c963 = -0x370b;
    D_c967 = -0x370b;
    D_c947 = -0x36ed;
    D_c94d = -0x36ed;
    D_c953 = -0x36ed;
    D_c959 = -0x36ed;
    D_c95b = -0x36ed;
    D_c961 = -0x36ed;
    return D_c969 = -0x36ed;
}

/* sub_s002_0e76_1830: not decompiled: unhandled instruction rep stosb byte ptr es:[di], al */
void far sub_s002_0e76_1830(void) { }

/* sub_s002_0e76_1a7a: not decompiled: unhandled instruction rep stosw word ptr es:[di], ax */
void far sub_s002_0e76_1a7a(void) { }
