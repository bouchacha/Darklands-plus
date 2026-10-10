/* cpu.c - flags and arithmetic helpers for recompiled code */
#include "cpu.h"

CPU R;
u8 *M;

static int parity(u32 v) { v &= 0xff; v ^= v >> 4; v ^= v >> 2; v ^= v >> 1; return !(v & 1); }

int get_cf(void)
{
    u32 m = msk(R.fw);
    switch (R.fop) {
    case FOP_ADD: return R.fr < R.fa;
    case FOP_ADC: return R.fcf ? R.fr <= R.fa : R.fr < R.fa;
    case FOP_SUB: return R.fa < R.fb;
    case FOP_SBB: return R.fcf ? R.fa <= R.fb : R.fa < R.fb;
    case FOP_LOGIC: return 0;
    case FOP_INC: case FOP_DEC: return R.fcf;
    case FOP_NEG: return (R.fb & m) != 0;
    default: return R.fcf;
    }
}
int get_zf(void) { return R.fop == FOP_EXPLICIT ? R.fzf : (R.fr & msk(R.fw)) == 0; }
int get_sf(void) { return R.fop == FOP_EXPLICIT ? R.fsf : (R.fr & sgn(R.fw)) != 0; }
int get_pf(void) { return R.fop == FOP_EXPLICIT ? R.fpf : parity(R.fr); }
int get_af(void)
{
    switch (R.fop) {
    case FOP_ADD: case FOP_ADC: case FOP_SUB: case FOP_SBB: case FOP_NEG:
        return ((R.fa ^ R.fb ^ R.fr) & 0x10) != 0;
    case FOP_INC: return (R.fr & 0xf) == 0;
    case FOP_DEC: return (R.fr & 0xf) == 0xf;
    case FOP_EXPLICIT: return R.faf;
    default: return 0;
    }
}
int get_of(void)
{
    u32 s = sgn(R.fw);
    switch (R.fop) {
    case FOP_ADD: case FOP_ADC: return ((R.fa ^ R.fr) & (R.fb ^ R.fr) & s) != 0;
    case FOP_SUB: case FOP_SBB: case FOP_NEG: return ((R.fa ^ R.fb) & (R.fa ^ R.fr) & s) != 0;
    case FOP_LOGIC: return 0;
    case FOP_INC: return R.fr == s;
    case FOP_DEC: return R.fr == s - 1;
    default: return R.fof;
    }
}
u16 get_flags(void)
{
    return (u16)(0x7002 | get_cf() | (get_pf() << 2) | (get_af() << 4) | (get_zf() << 6) | (get_sf() << 7) |
                 (R.tf << 8) | (R.iflag << 9) | (R.df << 10) | (get_of() << 11));
}
void set_flags(u16 f)
{
    R.fop = FOP_EXPLICIT;
    R.fcf = f & 1; R.fpf = (f >> 2) & 1; R.faf = (f >> 4) & 1; R.fzf = (f >> 6) & 1; R.fsf = (f >> 7) & 1;
    R.tf = (f >> 8) & 1; R.iflag = (f >> 9) & 1; R.df = (f >> 10) & 1; R.fof = (f >> 11) & 1;
}
void flags_explicit(int cf, int of, u32 res, int w)
{
    res &= msk(w);
    R.fop = FOP_EXPLICIT; R.fcf = cf; R.fof = of; R.fzf = res == 0; R.fsf = (res & sgn(w)) != 0;
    R.fpf = parity(res); R.faf = 0; R.fr = res; R.fw = w;
}

/* shifts and rotates (count already masked to 0..31) */
u32 f_shl(u32 a, int n, int w)
{
    if (!n) return a;
    u32 m = msk(w), r = (a << n) & m;
    int cf = n <= w ? (int)((a >> (w - n)) & 1) : 0;
    flags_explicit(cf, ((r & sgn(w)) != 0) ^ cf, r, w);
    return r;
}
u32 f_shr(u32 a, int n, int w)
{
    if (!n) return a;
    a &= msk(w);
    int cf = n <= w ? (int)((a >> (n - 1)) & 1) : 0;
    u32 r = n < 32 ? a >> n : 0;
    flags_explicit(cf, n == 1 ? (a & sgn(w)) != 0 : 0, r, w);
    return r;
}
u32 f_sar(u32 a, int n, int w)
{
    if (!n) return a;
    s32 v = w == 8 ? (s8)a : (s16)a;
    int k = n > w ? w : n;
    int cf = (int)((v >> (k - 1)) & 1);
    u32 r = (u32)(v >> k) & msk(w);
    flags_explicit(cf, 0, r, w);
    return r;
}
u32 f_rol(u32 a, int n, int w)
{
    if (!n) return a;
    u32 m = msk(w); a &= m;
    int k = n % w;
    u32 r = k ? ((a << k) | (a >> (w - k))) & m : a;
    int cf = r & 1;
    int zf = get_zf(), sf = get_sf(), pf = get_pf();
    flags_explicit(cf, ((r & sgn(w)) != 0) ^ cf, r, w);
    R.fzf = zf; R.fsf = sf; R.fpf = pf;      /* rotates leave ZF/SF/PF alone */
    return r;
}
u32 f_ror(u32 a, int n, int w)
{
    if (!n) return a;
    u32 m = msk(w); a &= m;
    int k = n % w;
    u32 r = k ? ((a >> k) | (a << (w - k))) & m : a;
    int cf = (r & sgn(w)) != 0;
    int zf = get_zf(), sf = get_sf(), pf = get_pf();
    flags_explicit(cf, ((r ^ (r << 1)) & sgn(w)) != 0, r, w);
    R.fzf = zf; R.fsf = sf; R.fpf = pf;
    return r;
}
u32 f_rcl(u32 a, int n, int w)
{
    if (!n) return a;
    u32 m = msk(w); a &= m;
    int cf = get_cf();
    int zf = get_zf(), sf = get_sf(), pf = get_pf();
    for (int i = 0; i < n % (w + 1); i++) {
        int out = (a & sgn(w)) != 0;
        a = ((a << 1) | cf) & m;
        cf = out;
    }
    flags_explicit(cf, ((a & sgn(w)) != 0) ^ cf, a, w);
    R.fzf = zf; R.fsf = sf; R.fpf = pf;
    return a;
}
u32 f_rcr(u32 a, int n, int w)
{
    if (!n) return a;
    u32 m = msk(w); a &= m;
    int cf = get_cf();
    int zf = get_zf(), sf = get_sf(), pf = get_pf();
    int of = ((a & sgn(w)) != 0) ^ cf;
    for (int i = 0; i < n % (w + 1); i++) {
        int out = a & 1;
        a = (a >> 1) | (cf ? sgn(w) : 0);
        cf = out;
    }
    flags_explicit(cf, of, a, w);
    R.fzf = zf; R.fsf = sf; R.fpf = pf;
    return a;
}

void f_mul8(u8 v) { u16 r = (u16)(AL * v); AX = r; int c = AH != 0; flags_explicit(c, c, r, 16); }
void f_mul16(u16 v) { u32 r = (u32)AX * v; AX = (u16)r; DX = (u16)(r >> 16); int c = DX != 0; flags_explicit(c, c, AX, 16); }
void f_imul8(s8 v) { s16 r = (s16)((s8)AL * v); AX = (u16)r; int c = r != (s8)r; flags_explicit(c, c, AX, 16); }
void f_imul16(s16 v) { s32 r = (s32)(s16)AX * v; AX = (u16)r; DX = (u16)((u32)r >> 16); int c = r != (s16)r; flags_explicit(c, c, AX, 16); }
u16 f_imul3(s16 a, s16 b) { s32 r = (s32)a * b; int c = r != (s16)r; flags_explicit(c, c, (u16)r, 16); return (u16)r; }
void f_div8(u8 v)
{
    if (!v) { hle_int(0); return; }
    u16 q = AX / v, rm = AX % v;
    if (q > 0xff) { hle_int(0); return; }
    AL = (u8)q; AH = (u8)rm;
}
void f_div16(u16 v)
{
    if (!v) { hle_int(0); return; }
    u32 n = ((u32)DX << 16) | AX;
    u32 q = n / v, rm = n % v;
    if (q > 0xffff) { hle_int(0); return; }
    AX = (u16)q; DX = (u16)rm;
}
void f_idiv8(s8 v)
{
    if (!v) { hle_int(0); return; }
    s16 n = (s16)AX;
    s16 q = n / v, rm = n % v;
    if (q > 127 || q < -128) { hle_int(0); return; }
    AL = (u8)q; AH = (u8)rm;
}
void f_idiv16(s16 v)
{
    if (!v) { hle_int(0); return; }
    s32 n = (s32)(((u32)DX << 16) | AX);
    s32 q = n / v, rm = n % v;
    if (q > 32767 || q < -32768) { hle_int(0); return; }
    AX = (u16)q; DX = (u16)rm;
}
void f_daa(void)
{
    int cf = get_cf(), af = get_af();
    u8 al = AL, old = al;
    if ((al & 0xf) > 9 || af) { al += 6; af = 1; } else af = 0;
    if (old > 0x99 || cf) { al += 0x60; cf = 1; } else cf = 0;
    AL = al; flags_explicit(cf, 0, al, 8); R.faf = af;
}
void f_das(void)
{
    int cf = get_cf(), af = get_af();
    u8 al = AL, old = al;
    if ((al & 0xf) > 9 || af) { al -= 6; af = 1; } else af = 0;
    if (old > 0x99 || cf) { al -= 0x60; cf = 1; } else cf = 0;
    AL = al; flags_explicit(cf, 0, al, 8); R.faf = af;
}
