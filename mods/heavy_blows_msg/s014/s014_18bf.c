/* place: s014 18bf:0002 */
/* orig: size=10e4 CONST=7d8c+0016 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: CONST=7d8c */
/* s014 18bf: ATTACK RESOLUTION AND DAMAGE.
     0002  attack rate: D_d493[i] = 6000 / delay (melee) or 100 (missile, + aim value +0x1d)
     0102  one attack a -> t: to-hit (02a8), damage (0778), choose the swing/hit animation and
           the sound sequence of the attacker (D_c98a[a][..], length D_c96b[a])
     02a8  to-hit roll: returns 0 miss, 1 miss (parried/dodged), 2 hit, 3 hit with bonus
     0592  missile to-hit chance
     0778  damage of a hit: weapon damage + strength vs armour of the hit location
     08ca  roll damage and queue it: D_d70c[t].endur, D_d70c[t].str
     098e  apply queued damage: endurance 0 -> unconscious (2), strength 0 -> dead (3)
     0a40  ready weapon w (stats from the weapon tables D_7669..D_77e3)
     0c02/0c48  missile weapons: has ammo / use one ammo
     0cfe/0db4/0fa4  inventory: has item / consume one of item / consume slot k
   Interpretations of fields marked "?" are guesses from the arithmetic.
   status: 13/13 written, 3 match (098e, 0002: int 6000 / t, clamp t then store; 0c02: if/else
   into r + one return r, char in a union; m4).
   m4: 0cfe/0db4 are called WITHOUT prototype (orig pushes int/cbw words, like s019);
   0c48 = two whole calls + struct {id; w} locals, left: mov bx,ax / mov [bp-4],bx store order. */

struct cbt {                    /* combatant record, 0x80 bytes, D_9c55[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[4];
    char f0c;                   /* +0c weapon: ? (from F_20a5_77e3.v[w]) */
    char f0d;                   /* +0d weapon: skill value used for damage */
    int info;                   /* +0e &D_ceae[kind] (0x70-byte animation info) */
    int f10;                    /* +10 action/stance bits: 2 melee, 4/8/0x20 stance, 0x100 no damage */
    char pad12[2];
    char status;                /* +14 1 active, 2 unconscious, 3 dead */
    char pad15;
    char kind;                  /* +16 animation slot */
    char pad17[6];
    char f1d;                   /* +1d missile aim value */
    char pad1e[2];
    unsigned char def;          /* +20 defence value (vs. attacks) */
    unsigned char atk;          /* +21 attack value */
    char weapon;                /* +22 readied weapon id (-1 none) */
    char f23;                   /* +23 weapon: min strength? */
    char f24;                   /* +24 */
    char pad25[0x23];
    unsigned char f48;          /* +48 high nibble: damage bonus */
    char load;                  /* +49 carried weight */
    char target;                /* +4a current target */
    char f4b;                   /* +4b */
    char f4c;                   /* +4c */
    char f4d[2];                /* +4d armour value per hit location */
    char f4f[2];                /* +4f armour vs. strength damage per location */
    char f51;                   /* +51 default weapon */
    unsigned char f52;          /* +52 weapon speed */
    unsigned char f53;          /* +53 weapon class (high nibble 1 = shield) / damage */
    char f54;                   /* +54 weapon damage */
    unsigned char f55;          /* +55 bit 7: armour piercing; low: penetration */
    char f56;                   /* +56 weapon max strength */
    unsigned char f57;          /* +57 weapon min strength | 2-handed flag */
    char f58;                   /* +58 one-handed skill */
    char f59;                   /* +59 skill with readied weapon */
    char f5a;                   /* +5a alternate skill */
    char f5b;                   /* +5b shield skill */
    char f5c;                   /* +5c shield id (0x5f..0x61) */
    char endur;                 /* +5d current endurance */
    char str;                   /* +5e current strength */
    char f5f;                   /* +5f agility? */
    char pad60[4];
    char f64;                   /* +64 */
    char f65;                   /* +65 strength (capacity) */
    char pad66[5];
    char skill[7];              /* +6b weapon skills by category */
    char pad72[0xc];
    char nitems;                /* +7e */
    char pad7f;
};

struct item { unsigned id; unsigned char f2; unsigned char f3; unsigned char cnt; char f5; };
struct dmg { char endur; char str; };   /* queued damage */

int far sub_root_0000_0c8c(int);        /* random 1..n */
int far _random_n(int);
void far sub_root_0000_1716(int);
int far sub_s014_0e76_469e(void *, int);
int far sub_s014_14f7_343c(int);
void far sub_s014_1db7_05c8(int);
int far sub_s014_1ec9_0142(int, int);
int far sub_s014_18bf_02a8(int, int, int *);
int far sub_s014_18bf_0592(int, int, unsigned char, struct cbt *, struct cbt *);
int far sub_s014_18bf_0778(int, int, int, int);
int far sub_s014_18bf_08ca(int, int, int, int);
void far sub_s014_18bf_0a40(int, int, int);
int far sub_s014_18bf_0cfe();
int far sub_s014_18bf_0db4();

extern struct cbt D_9c55[];
extern struct item far F_2e38_003e[][64];   /* party inventories */
extern struct item far F_2e38_0dbe[][16];   /* enemy inventories ([i - 5]) */
extern unsigned char D_d493[];          /* [13] attack rate (255 = recompute) */
extern char D_ca57[];                   /* [13] current action/animation */
extern char D_c96b[];                   /* [13] sound sequence length */
extern char D_c98a[][10];               /* [13] sound sequence */
extern char D_c7d2[];                   /* per animation set: has a miss sequence */
extern char D_c86b[];                   /* per kind slot: 1 = no parry sequence */
extern unsigned char D_c8a7[];          /* per animation set: parry sequence length | 0x80 */
extern unsigned char D_c84a[];          /* per animation set: hit sequence length | 0x80 */
extern char D_a3e7[][30];               /* sound sequences per animation set */
extern char D_c21a;                     /* demo / tutorial mode */
extern char D_c26e;                     /* attacks by the leader in demo mode */
extern char D_9f1f;                     /* */
extern char D_a33d;                     /* number of effect clouds */
extern char D_a2eb[];                   /* effect clouds (10 bytes each) */
extern char D_7900[][13];               /* [t][a] a can attack t (adjacent) */
extern unsigned char D_1b88[];          /* damage table: low damage add */
extern unsigned char D_1b90[];          /* damage table: low damage divisor */
extern struct dmg D_d70c[];             /* [13] queued damage */
extern char D_d4c7[];                   /* [13] damage taken this round */
extern char D_a6c1;                     /* no kills (practice / arena) */
extern char *D_1174[];                  /* [7] animation file name per kind slot */
/* weapon tables (63 weapons) in far segment 20a5; every table is its own far variable */
struct wtab { char v[63]; };
extern struct wtab far F_20a5_75ac;     /* weapon -> skill category (2 = missile) */
extern struct wtab far F_20a5_7669;     /* speed */
extern struct wtab far F_20a5_76a8;     /* class | penetration */
extern struct wtab far F_20a5_76e7;     /* damage */
extern struct wtab far F_20a5_7726;     /* minimum skill */
extern struct wtab far F_20a5_7765;     /* minimum strength */
extern struct wtab far F_20a5_77a4;     /* maximum strength */
extern struct wtab far F_20a5_77e3;     /* */
extern struct { char v[4]; } far F_20a5_781e;      /* armour class -> value */
extern struct { char v[63][2]; } far F_20a5_75eb;  /* animation file letters */
extern struct wtab far F_20a5_7822;     /* weapon -> ammo item (0 = none) */
extern char D_ca64[];                   /* [13] */
extern char D_a3d8[];                   /* [13] */
extern char D_c2bf[];                   /* [13] */

/* attack rate of combatant i (recomputed when D_d493[i] == 255) */
void far sub_s014_18bf_0002(int i)
{
    struct cbt *c;
    int t, e, load;
    unsigned st;
    char a;

    if (D_d493[i] == 0xff) {
        st = D_9c55[i].f10;
        if ((st & 2) == 2) {
            a = D_9c55[i].f5f;
            t = ((200 - D_9c55[i].f59 - a) >> 1) + (D_9c55[i].f52 + 15) * 2;
            if ((st & 6) == 6)
                t -= 30;
            else if ((st & 0x22) == 0x22)
                t += 60;
            else if ((st & 0xa) == 0xa)
                t += 120;
            e = D_9c55[i].f65 + D_9c55[i].f64;
            load = D_9c55[i].load;
            if (e / 2 < load && load <= e)
                t += 30;
            else if (e * 3 / 2 < load)
                t += 100;
            D_d493[i] = 6000 / t;
        } else {
            t = -(D_9c55[i].f5f >> 1) - ((D_9c55[i].f59 >> 1) - 150);
            if (t < 0)
                t = 0;
            D_9c55[i].f1d = t;
            D_d493[i] = 100;
        }
    }
}

/* one attack of a on t; returns the to-hit result */
int far sub_s014_18bf_0102(int a, int t)
{
    int r;
    int crit;
    int set;
    int kind;
    int k, n;
    unsigned char m;

    r = sub_s014_18bf_02a8(a, t, &crit);
    kind = D_9c55[a].kind;
    set = sub_s014_14f7_343c(a);
    if (r < 2) {
        if (r == 0) {
            if (D_c7d2[set] == 0)
                r = 1;
        } else
            D_ca57[a] = 6;
    } else {
        if (sub_s014_18bf_0778(a, t, r, crit) < 2)
            D_ca57[a] = 1;
        else
            D_ca57[a] = 2;
    }
    D_9c55[a].info = kind * 0x70 + 0xceae;
    if (D_c21a != 0 && a == 5)
        D_c26e++;
    if (r == 0) {
        n = _random_n(4);
        for (k = 0; k < 5; k++)
            D_c98a[a][k] = D_a3e7[set][(n + 2) * 5 + k];
        D_c96b[a] = 5;
        return 0;
    }
    if (r == 1) {
        if (D_c21a != 0)
            return 1;
        if (D_c86b[kind] != 1) {
            m = D_c8a7[set];
            if ((D_c96b[a] = m & 0x7f) == 0)
                return 1;
            for (k = 0; k < D_c96b[a]; k++)
                D_c98a[a][k] = D_a3e7[set][5 + k];
            return 1;
        }
    }
    if (D_c21a == 0) {
        m = D_c84a[set];
        if ((D_c96b[a] = m & 0x7f) != 0)
            for (k = 0; k < D_c96b[a]; k++)
                D_c98a[a][k] = D_a3e7[set][k];
    }
    return r;
}

/* to-hit roll of a against t; *crit = random hit location (0/1) */
int far sub_s014_18bf_02a8(int a, int t, int *crit)
{
    int roll;
    int chance;
    struct cbt *ca, *ct;
    unsigned st;
    unsigned char atk;
    int lo, hi, k, near_, engaged, eng, cloud, eff, sh, e, q, sk;

    roll = sub_root_0000_0c8c(100);
    if (D_c21a != 0 && a == 5 && D_9f1f == -1)
        return 0;
    if (D_d493[a] < roll)
        return 0;
    ct = &D_9c55[t];
    *crit = roll & 1;
    ca = &D_9c55[a];
    st = ca->f10;
    if ((st >> 8 & 1) == 1)
        return 2;                       /* helpless target: automatic hit */
    if ((st & 2) == 2) {
        chance = sub_s014_18bf_0592(a, t, (unsigned char)st, ca, ct);
    } else {
        atk = ca->atk;
        if (a < 5) {
            lo = 0;
            hi = 5;
        } else {
            lo = 5;
            hi = 13;
        }
        /* number of allies of a standing next to t */
        near_ = 0;
        for (k = lo; k <= hi; k++)
            if (D_7900[t][k] != 0 && D_9c55[k].status == 1)
                near_++;
        if (near_ == 0)
            eng = 10;
        else if ((eng = (5 - near_) * 2) < 1)
            eng = 1;
        /* number of combatants attacking a */
        engaged = 0;
        for (k = 0; k < 13; k++)
            if (D_9c55[k].target == a && D_9c55[k].status == 1 && (D_9c55[k].f10 & 2) == 2)
                engaged++;
        if (engaged == 0)
            q = 10;
        else {
            engaged = engaged * 3 - 10;
            q = -engaged;
            if (engaged >= 0)
                q = 1;
        }
        cloud = sub_s014_1ec9_0142(a, t) == 1 ? 2 : 10;
        eff = cloud * (q * (eng * atk / 10) / 10) / 10;
        if (eff < 0)
            eff = 0;
        /* target's shield/dodge */
        e = (ct->f64 + ct->f65) * 5;
        k = ct->load;
        if (k <= e >> 3)
            sk = ct->f5f;
        else if (k <= e >> 2)
            sk = (ct->f5f << 1) / 3;
        else if (k <= (e >> 2) * 3 >> 1)
            sk = ct->f5f / 3;
        else
            sk = 0;
        sh = ct->f5b;
        if ((ct->f53 & 0xf0) == 0x10 && ct->f5c == 0x5f)
            sh = (sh << 1) / 7 + 1;
        else if ((ct->f53 & 0xf0) == 0x10 && ct->f5c == 0x60)
            sh = (sh >> 1) + 4;
        else if ((ct->f53 & 0xf0) == 0x10 && ct->f5c == 0x61)
            sh = (sh << 1) / 3 + 8;
        else
            sh = -5;
        sh += sk * 2;
        if (sh < 0)
            sh = 0;
        chance = (eff - sh) * 2 / 3 + 50;
    }
    if (chance < roll) {
        if (roll > 5 && ca->f59 - ca->f23 >= roll * 2)
            return 2;
        return 1;
    }
    if (chance < roll + 10)
        return 3;
    return 2;
}

/* missile to-hit chance of a (record ca) against t (record ct) */
int far sub_s014_18bf_0592(int a, int t, unsigned char st, struct cbt *ca, struct cbt *ct)
{
    int sh;
    int m;
    int n1, n2, k;
    char s;
    char *p;

    if (F_20a5_75ac.v[ca->f51] == 2 || (ct->f53 & 0xf0) != 0x10)
        sh = -5;
    else if (ct->f5c == 0x5f)
        sh = (ct->f5b << 1) / 7;
    else if (ct->f5c == 0x60)
        sh = (ct->f5b << 1) / 5;
    else if (ct->f5c == 0x61)
        sh = ct->f5b / 2;
    else
        sh = -5;
    sh += ct->def;
    if (sh < 0)
        sh = 0;
    m = 0;
    if ((st & 0xa) == 0xa)
        m = -5;
    else if ((st & 0x22) == 0x22)
        m = -2 - (ca->f59 >> 2);
    else if ((st & 6) == 6) {
        s = ca->f59 >> 2;
        if (s < 10)
            s = 10;
        m = s;
    }
    if ((ct->f10 & 0x22) == 0x22)
        m -= (ct->f59 >> 2) + 5;
    else if ((ct->f10 & 6) == 6) {
        m = -(ca->f59 / 4);
        if (m < -10)
            m = -10;
    }
    n1 = 0;
    for (k = 0; k < 13; k++)
        if (D_9c55[k].target == t && D_9c55[k].status == 1 && (D_9c55[k].f10 & 2) == 2)
            n1++;
    n2 = 0;
    for (k = 0; k < 13; k++)
        if (D_9c55[k].target == a && D_9c55[k].status == 1 && (D_9c55[k].f10 & 2) == 2)
            n2++;
    m += n1 * 10 - n2 * 10;
    for (k = 0, p = D_a2eb; k < D_a33d; k++, p += 10)
        if (sub_s014_0e76_469e(p, a) == 1) {
            m -= 15;                    /* shooter inside an effect cloud */
            break;
        }
    m = (ca->def - sh) * 2 / 3 + m + 50;
    if ((st & 6) == 6 && m < 10)
        m = 10;
    return m;
}

/* damage of a hit by a on t at location loc; r = to-hit result (3 = good hit) */
int far sub_s014_18bf_0778(int a, int t, int r, int loc)
{
    int arm;
    int bonus;
    unsigned mn;
    char mx;
    int pen;
    int k;

    if ((D_9c55[a].f10 & 0x100) == 0x100)
        return 0;
    arm = D_9c55[t].f4d[loc] + (D_9c55[t].f55 & 0x7f);
    bonus = (D_9c55[a].f48 >> 4) + D_9c55[a].f54;
    mn = D_9c55[a].f57 & 0x7f;
    mx = D_9c55[a].f56;
    pen = (D_9c55[a].f48 + D_9c55[a].f53) & 0xf;
    if ((D_9c55[a].f10 & 0xa) == 0xa)
        pen += sub_root_0000_0c8c(4);
    if (r == 3)
        pen -= sub_root_0000_0c8c(4);
    if (pen <= arm) {
        if (arm == pen)
            bonus /= 2;
        else
            bonus = abs(bonus) >> 3;    /* sign kept */
    }
    k = D_9c55[a].f65;
    if (mn < k)
        bonus += (k - mn) / 5 + 1;
    else if (mx > k)
        bonus += (k - mx) / 5 - 1;
    k = (D_9c55[a].f55 & 0x80) ? D_9c55[t].f24 / 2 : 0;
    k += D_9c55[t].f4f[loc];
    if (k > 99)
        k = 99;
    return sub_s014_18bf_08ca((D_9c55[a].f0d - k) / 10 + bonus, t, pen, arm);
}

/* roll damage d (0..40) against t and queue it; pen/arm decide the strength part */
static char _based(_segname("_CODE")) modmsg[] = "heavy blows!";
char far *mod_msg_ptr(void) { return (char far *)modmsg; }
int far sub_s014_18bf_08ca(int d, int t, int pen, int arm)
{
    int roll;
    int dmg;

    if (d > 40)
        d = 40;
    if (d < 0)
        d = 0;
    roll = sub_root_0000_0c8c(6) + sub_root_0000_0c8c(6);
    if (d <= 7)
        dmg = roll / D_1b90[d] + D_1b88[d];
    else if (d < 8 || d > 20)
        dmg = d + roll - 10;
    else
        dmg = (roll >> 1) + d - 6;
    if (t >= 5)                         /* MOD heavy_blows: enemies (5..12) take double damage */
        dmg *= 2;
    D_d70c[t].endur += dmg;
    if (arm < pen)
        roll = sub_root_0000_0c8c(6) + 3;
    else if (arm == pen)
        roll = sub_root_0000_0c8c(4) + 1;
    else
        roll = _random_n(3);
    D_d70c[t].str += roll * dmg / 10;
    return dmg;
}

/* apply the queued damage of combatant i */
#pragma optimize("a", on)
void far sub_s014_18bf_098e(int i)
{
    char d;

    if (D_9c55[i].status == 1) {
        if (D_d70c[i].endur > 42)
            D_d70c[i].endur = 42;
        d = D_d70c[i].endur;
        D_9c55[i].endur -= d;
        D_d4c7[i] += d;
        D_d70c[i].endur = 0;
        if (D_9c55[i].endur <= 0) {
            D_9c55[i].endur = 0;
            D_9c55[i].status = 2;       /* unconscious */
        }
        D_9c55[i].str -= D_d70c[i].str;
        D_d70c[i].str = 0;
        if (D_9c55[i].str <= 0) {
            D_9c55[i].str = 0;
            D_9c55[i].status = 3;       /* dead */
        }
        if (D_a6c1 == 0) {
            if (D_9c55[i].endur == 0) {
                D_9c55[i].endur = 1;
                D_9c55[i].status = 1;
            }
            if (D_9c55[i].str == 0) {
                D_9c55[i].str = 1;
                D_9c55[i].status = 1;
            }
        }
        sub_root_0000_1716(0x100);
    }
}
#pragma optimize("", on)

/* ready weapon w (-1 with alt == 0: bare hands = 9); alt selects the alternate skill */
void far sub_s014_18bf_0a40(int i, int w, int alt)
{
    int m2;
    int m1;
    int p1;
    int p2;
    int sk, d;
    char lo, s;
    unsigned char hi;
    char *n;

    p2 = 0;
    m2 = 0;
    p1 = 0;
    m1 = 0;
    if (w == -1 && alt == 0)
        w = 9;
    D_d493[i] = 0xff;
    D_9c55[i].f0d = alt == 0 ? D_9c55[i].f58 : D_9c55[i].f5a;
    D_9c55[i].f52 = F_20a5_7669.v[w];
    D_9c55[i].f53 = F_20a5_76a8.v[w];
    D_9c55[i].f54 = F_20a5_76e7.v[w];
    D_9c55[i].f23 = F_20a5_7726.v[w];
    D_9c55[i].f56 = lo = F_20a5_7765.v[w];
    D_9c55[i].f57 = hi = F_20a5_77a4.v[w];
    D_9c55[i].f0c = F_20a5_77e3.v[w];
    s = D_9c55[i].f4b;
    if (s < 0)
        s = 0;
    D_9c55[i].f4d[0] = F_20a5_781e.v[s];
    s = D_9c55[i].f4c;
    if (s < 0)
        s = 0;
    D_9c55[i].f4d[1] = F_20a5_781e.v[s];
    D_9c55[i].f59 = s = D_9c55[i].skill[F_20a5_75ac.v[w]];
    sk = s;
    D_9c55[i].f57 = hi | alt;
    n = D_1174[D_9c55[i].kind];
    n[0] = F_20a5_75eb.v[w][0];
    n[1] = F_20a5_75eb.v[w][1];
    d = lo - D_9c55[i].f65;
    if (d > 0) {
        m1 = d * 3;                     /* too weak for the weapon */
        p1 = d;
    }
    d = F_20a5_7726.v[w] - sk;
    if (d > 0) {
        m2 = d * 2;
        p2 = d * 2;
    }
    d = D_9c55[i].f65 - F_20a5_77a4.v[w];
    if (d < 1)
        d = 0;
    else
        d *= 2;
    p2 = sk - p1 - p2;
    sk = d - m1 - m2 + sk;
    if (sk < 0)
        sk = 0;
    if (p2 < 0)
        p2 = 0;
    if (sk > 0xff || p2 > 0xff) {
        if (sk > 0xff)
            sk = 0xff;
        if (p2 > 0xff)
            p2 = 0xff;
    }
    D_9c55[i].def = sk;
    D_9c55[i].atk = p2;
}

/* 1 if i can fire its readied weapon (no ammo needed, or has ammo of quality 25) */
int far sub_s014_18bf_0c02(int i)
{
    union { char am; int d; } u;
    int r;

    u.am = F_20a5_7822.v[D_9c55[i].weapon];
    if (u.am == 0)
        r = 1;
    else
        r = sub_s014_18bf_0cfe(i, u.am, 0x19);
    return r;
}

/* use one ammo / thrown weapon of i; when it runs out switch back to the default weapon */
int far sub_s014_18bf_0c48(int i)
{
    struct { int id; char w; char pad; } s;
    int r;

    D_d493[i] = 0xff;
    s.id = D_9c55[i].weapon;
    if ((s.w = F_20a5_7822.v[s.id]) == 0)
        r = sub_s014_18bf_0db4(i, s.id, D_9c55[i].f5a);
    else
        r = sub_s014_18bf_0db4(i, s.w, 0x19);
    if (r == -1) {
        if (F_20a5_7822.v[D_9c55[i].weapon] == 0)
            D_9c55[i].weapon = -1;
        if (i >= 5)
            sub_s014_1db7_05c8(i);
        D_ca64[i] = 0xd;
        D_a3d8[i] = 2;
        sub_root_0000_1716(i);
        D_c2bf[i] = 0;
        D_9c55[i].target = -1;
        sub_s014_18bf_0a40(i, D_9c55[i].f51, 0);
    }
    return r;
}

/* 1 if combatant i carries item (id, q) */
#pragma optimize("e", off)
int far sub_s014_18bf_0cfe(int i, unsigned char id, unsigned char q)
{
    int k;
    struct item far *p;

    if (i < 5) {
        if (D_9c55[i].nitems > 0)
            for (k = 0, p = F_2e38_003e[i]; k < D_9c55[i].nitems; k++, p++)
                if (p->f2 == id && p->f3 == q)
                    return 1;
    } else {
        if (D_9c55[i].nitems > 0)
            for (k = 0, p = F_2e38_0dbe[i - 5]; k < D_9c55[i].nitems; k++, p++)
                if (p->f2 == id && p->f3 == q)
                    return 1;
    }
    return 0;
}
#pragma optimize("e", on)

/* consume one of item (id, q); returns the remaining count or -1 when gone / not found */
int far sub_s014_18bf_0db4(int i, unsigned char id, unsigned char q)
{
    char *pn;
    int found;
    int k, n;
    struct item far *p;

    found = 0;
    if (i < 5) {
        pn = &D_9c55[i].nitems;
        if (*pn > 0)
            for (k = 0, p = F_2e38_003e[i]; k < *pn; k++, p++)
                if (p->f2 == id && p->f3 == q) {
                    found = 1;
                    break;
                }
        if (!found)
            return -1;
        if ((n = F_2e38_003e[i][k].cnt-- - 1) > 0)
            return n;
        if (k < *pn - 1)
            for (n = D_9c55[i].nitems - k - 1, p = &F_2e38_003e[i][k]; n != 0; n--, p++)
                p[0] = p[1];
    } else {
        k = 0;
        pn = &D_9c55[i].nitems;
        if (*pn > 0)
            for (p = F_2e38_0dbe[i - 5]; k < *pn; k++, p++)
                if (p->f2 == id && p->f3 == q) {
                    found = 1;
                    break;
                }
        if (!found)
            return -1;
        if ((n = F_2e38_0dbe[i - 5][k].cnt-- - 1) > 0)
            return n;
        if (k < *pn - 1)
            for (n = D_9c55[i].nitems - k - 1, p = &F_2e38_0dbe[i - 5][k]; n != 0; n--, p++)
                p[0] = p[1];
    }
    (*pn)--;
    return -1;
}

/* consume one of inventory slot k of i; -1 when the slot is used up */
int far sub_s014_18bf_0fa4(int i, int k)
{
    int n;
    char *pn;
    int c;
    struct item far *p;

    if (i < 5) {
        if ((n = F_2e38_003e[i][k].cnt-- - 1) < 1) {
            pn = &D_9c55[i].nitems;
            if (k < *pn - 1)
                for (p = &F_2e38_003e[i][k], c = D_9c55[i].nitems - k - 1; c != 0; c--, p++)
                    p[0] = p[1];
            (*pn)--;
            return -1;
        }
    } else {
        if ((n = F_2e38_0dbe[i - 5][k].cnt-- - 1) < 1) {
            pn = &D_9c55[i].nitems;
            if (k < *pn - 1)
                for (p = &F_2e38_0dbe[i - 5][k], c = D_9c55[i].nitems - k - 1; c != 0; c--, p++)
                    p[0] = p[1];
            (*pn)--;
            return -1;
        }
    }
    return n;
}
