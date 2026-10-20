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
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
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

int far rand_1_n(int);        /* random 1..n */
int far _random_n(int);
void far party_panel_rows(int);
int far sub_s014_0e76_469e(void *, int);
int far cbt_anim_set(int);
void far sub_s014_1db7_05c8(int);
int far cloud_blocks_line(int, int);
int far to_hit_roll(int, int, int *);
int far melee_to_hit(int, int, unsigned char, struct cbt *, struct cbt *);
int far hit_damage(int, int, int, int);
int far damage_queue(int, int, int, int);
void far weapon_ready(int, int, int);
int far sub_s014_18bf_0cfe();
int far consume_item();

extern struct cbt combatants[];
extern struct item far inventories[][64];   /* party inventories */
extern struct item far enemy_items[][16];   /* enemy inventories ([i - 5]) */
extern unsigned char cbt_attack_rate[];          /* [13] attack rate (255 = recompute) */
extern char cbt_anim[];                   /* [13] current action/animation */
extern char cbt_anim_len[];                   /* [13] sound sequence length */
extern char cbt_anim_seq[][10];               /* [13] sound sequence */
extern char D_c7d2[];                   /* per animation set: has a miss sequence */
extern char D_c86b[];                   /* per kind slot: 1 = no parry sequence */
extern unsigned char D_c8a7[];          /* per animation set: parry sequence length | 0x80 */
extern unsigned char D_c84a[];          /* per animation set: hit sequence length | 0x80 */
extern char D_a3e7[][30];               /* sound sequences per animation set */
extern char demo_mode;                     /* demo / tutorial mode */
extern char D_c26e;                     /* attacks by the leader in demo mode */
extern char D_9f1f;                     /* */
extern char effect_cloud_count;                     /* number of effect clouds */
extern char effect_clouds[];                   /* effect clouds (10 bytes each) */
extern char D_7900[][13];               /* [t][a] a can attack t (adjacent) */
extern unsigned char D_1b88[];          /* damage table: low damage add */
extern unsigned char D_1b90[];          /* damage table: low damage divisor */
extern struct dmg cbt_queued_damage[];             /* [13] queued damage */
extern char cbt_damage_shown[];                   /* [13] damage taken this round */
extern char real_combat;                     /* no kills (practice / arena) */
extern char *anim_file[];                  /* [7] animation file name per kind slot */
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
extern char cbt_action[];                   /* [13] */
extern char D_a3d8[];                   /* [13] */
extern char enemy_class[];                   /* [13] */

/* attack rate of combatant i (recomputed when cbt_attack_rate[i] == 255) */
void far attack_rate(int i)
{
    struct cbt *c;
    int t, e, load;
    unsigned st;
    char a;

    if (cbt_attack_rate[i] == 0xff) {
        st = combatants[i].f10;
        if ((st & 2) == 2) {
            a = combatants[i].f5f;
            t = ((200 - combatants[i].f59 - a) >> 1) + (combatants[i].f52 + 15) * 2;
            if ((st & 6) == 6)
                t -= 30;
            else if ((st & 0x22) == 0x22)
                t += 60;
            else if ((st & 0xa) == 0xa)
                t += 120;
            e = combatants[i].f65 + combatants[i].f64;
            load = combatants[i].load;
            if (e / 2 < load && load <= e)
                t += 30;
            else if (e * 3 / 2 < load)
                t += 100;
            cbt_attack_rate[i] = 6000 / t;
        } else {
            t = -(combatants[i].f5f >> 1) - ((combatants[i].f59 >> 1) - 150);
            if (t < 0)
                t = 0;
            combatants[i].f1d = t;
            cbt_attack_rate[i] = 100;
        }
    }
}

/* one attack of a on t; returns the to-hit result */
int far cbt_attack(int a, int t)
{
    int r;
    int crit;
    int set;
    int kind;
    int k, n;
    unsigned char m;

    r = to_hit_roll(a, t, &crit);
    kind = combatants[a].kind;
    set = cbt_anim_set(a);
    if (r < 2) {
        if (r == 0) {
            if (D_c7d2[set] == 0)
                r = 1;
        } else
            cbt_anim[a] = 6;
    } else {
        if (hit_damage(a, t, r, crit) < 2)
            cbt_anim[a] = 1;
        else
            cbt_anim[a] = 2;
    }
    combatants[a].info = kind * 0x70 + 0xceae;
    if (demo_mode != 0 && a == 5)
        D_c26e++;
    if (r == 0) {
        n = _random_n(4);
        for (k = 0; k < 5; k++)
            cbt_anim_seq[a][k] = D_a3e7[set][(n + 2) * 5 + k];
        cbt_anim_len[a] = 5;
        return 0;
    }
    if (r == 1) {
        if (demo_mode != 0)
            return 1;
        if (D_c86b[kind] != 1) {
            m = D_c8a7[set];
            if ((cbt_anim_len[a] = m & 0x7f) == 0)
                return 1;
            for (k = 0; k < cbt_anim_len[a]; k++)
                cbt_anim_seq[a][k] = D_a3e7[set][5 + k];
            return 1;
        }
    }
    if (demo_mode == 0) {
        m = D_c84a[set];
        if ((cbt_anim_len[a] = m & 0x7f) != 0)
            for (k = 0; k < cbt_anim_len[a]; k++)
                cbt_anim_seq[a][k] = D_a3e7[set][k];
    }
    return r;
}

/* to-hit roll of a against t; *crit = random hit location (0/1) */
int far to_hit_roll(int a, int t, int *crit)
{
    int roll;
    int chance;
    struct cbt *ca, *ct;
    unsigned st;
    unsigned char atk;
    int lo, hi, k, near_, engaged, eng, cloud, eff, sh, e, q, sk;

    roll = rand_1_n(100);
    if (demo_mode != 0 && a == 5 && D_9f1f == -1)
        return 0;
    if (cbt_attack_rate[a] < roll)
        return 0;
    ct = &combatants[t];
    *crit = roll & 1;
    ca = &combatants[a];
    st = ca->f10;
    if ((st >> 8 & 1) == 1)
        return 2;                       /* helpless target: automatic hit */
    if ((st & 2) == 2) {
        chance = melee_to_hit(a, t, (unsigned char)st, ca, ct);
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
            if (D_7900[t][k] != 0 && combatants[k].status == 1)
                near_++;
        if (near_ == 0)
            eng = 10;
        else if ((eng = (5 - near_) * 2) < 1)
            eng = 1;
        /* number of combatants attacking a */
        engaged = 0;
        for (k = 0; k < 13; k++)
            if (combatants[k].target == a && combatants[k].status == 1 && (combatants[k].f10 & 2) == 2)
                engaged++;
        if (engaged == 0)
            q = 10;
        else {
            engaged = engaged * 3 - 10;
            q = -engaged;
            if (engaged >= 0)
                q = 1;
        }
        cloud = cloud_blocks_line(a, t) == 1 ? 2 : 10;
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

/* the chance of a blow (an order with bit 2: Std Attack, Vulnerable, Berserk, Parry) by a (record ca)
   against t (record ct): +0x20 ("def" here; the melee rating, weapon_ready) on both sides, the
   defender's shield against blows (none against a flail), the stances and the crowd. The shot or throw
   is the caller's own branch, with +0x21 ("atk": the aim) against the shield and twice the Agility. */
int far melee_to_hit(int a, int t, unsigned char st, struct cbt *ca, struct cbt *ct)
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
        if (combatants[k].target == t && combatants[k].status == 1 && (combatants[k].f10 & 2) == 2)
            n1++;
    n2 = 0;
    for (k = 0; k < 13; k++)
        if (combatants[k].target == a && combatants[k].status == 1 && (combatants[k].f10 & 2) == 2)
            n2++;
    m += n1 * 10 - n2 * 10;
    for (k = 0, p = effect_clouds; k < effect_cloud_count; k++, p += 10)
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
int far hit_damage(int a, int t, int r, int loc)
{
    int arm;
    int bonus;
    unsigned mn;
    char mx;
    int pen;
    int k;

    if ((combatants[a].f10 & 0x100) == 0x100)
        return 0;
    arm = combatants[t].f4d[loc] + (combatants[t].f55 & 0x7f);
    bonus = (combatants[a].f48 >> 4) + combatants[a].f54;
    mn = combatants[a].f57 & 0x7f;
    mx = combatants[a].f56;
    pen = (combatants[a].f48 + combatants[a].f53) & 0xf;
    if ((combatants[a].f10 & 0xa) == 0xa)
        pen += rand_1_n(4);
    if (r == 3)
        pen -= rand_1_n(4);
    if (pen <= arm) {
        if (arm == pen)
            bonus /= 2;
        else
            bonus = abs(bonus) >> 3;    /* sign kept */
    }
    k = combatants[a].f65;
    if (mn < k)
        bonus += (k - mn) / 5 + 1;
    else if (mx > k)
        bonus += (k - mx) / 5 - 1;
    k = (combatants[a].f55 & 0x80) ? combatants[t].f24 / 2 : 0;
    k += combatants[t].f4f[loc];
    if (k > 99)
        k = 99;
    return damage_queue((combatants[a].f0d - k) / 10 + bonus, t, pen, arm);
}

/* roll damage d (0..40) against t and queue it; pen/arm decide the strength part */
int far damage_queue(int d, int t, int pen, int arm)
{
    int roll;
    int dmg;

    if (d > 40)
        d = 40;
    if (d < 0)
        d = 0;
    roll = rand_1_n(6) + rand_1_n(6);
    if (d <= 7)
        dmg = roll / D_1b90[d] + D_1b88[d];
    else if (d < 8 || d > 20)
        dmg = d + roll - 10;
    else
        dmg = (roll >> 1) + d - 6;
    cbt_queued_damage[t].endur += dmg;
    if (arm < pen)
        roll = rand_1_n(6) + 3;
    else if (arm == pen)
        roll = rand_1_n(4) + 1;
    else
        roll = _random_n(3);
    cbt_queued_damage[t].str += roll * dmg / 10;
    return dmg;
}

/* apply the queued damage of combatant i */
#pragma optimize("a", on)
void far damage_apply(int i)
{
    char d;

    if (combatants[i].status == 1) {
        if (cbt_queued_damage[i].endur > 42)
            cbt_queued_damage[i].endur = 42;
        d = cbt_queued_damage[i].endur;
        combatants[i].endur -= d;
        cbt_damage_shown[i] += d;
        cbt_queued_damage[i].endur = 0;
        if (combatants[i].endur <= 0) {
            combatants[i].endur = 0;
            combatants[i].status = 2;       /* unconscious */
        }
        combatants[i].str -= cbt_queued_damage[i].str;
        cbt_queued_damage[i].str = 0;
        if (combatants[i].str <= 0) {
            combatants[i].str = 0;
            combatants[i].status = 3;       /* dead */
        }
        if (real_combat == 0) {
            if (combatants[i].endur == 0) {
                combatants[i].endur = 1;
                combatants[i].status = 1;
            }
            if (combatants[i].str == 0) {
                combatants[i].str = 1;
                combatants[i].status = 1;
            }
        }
        party_panel_rows(0x100);
    }
}
#pragma optimize("", on)

/* ready weapon w (-1 with alt == 0: bare hands = 9); alt selects the alternate skill */
void far weapon_ready(int i, int w, int alt)
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
    cbt_attack_rate[i] = 0xff;
    combatants[i].f0d = alt == 0 ? combatants[i].f58 : combatants[i].f5a;
    combatants[i].f52 = F_20a5_7669.v[w];
    combatants[i].f53 = F_20a5_76a8.v[w];
    combatants[i].f54 = F_20a5_76e7.v[w];
    combatants[i].f23 = F_20a5_7726.v[w];
    combatants[i].f56 = lo = F_20a5_7765.v[w];
    combatants[i].f57 = hi = F_20a5_77a4.v[w];
    combatants[i].f0c = F_20a5_77e3.v[w];
    s = combatants[i].f4b;
    if (s < 0)
        s = 0;
    combatants[i].f4d[0] = F_20a5_781e.v[s];
    s = combatants[i].f4c;
    if (s < 0)
        s = 0;
    combatants[i].f4d[1] = F_20a5_781e.v[s];
    combatants[i].f59 = s = combatants[i].skill[F_20a5_75ac.v[w]];
    sk = s;
    combatants[i].f57 = hi | alt;
    n = anim_file[combatants[i].kind];
    n[0] = F_20a5_75eb.v[w][0];
    n[1] = F_20a5_75eb.v[w][1];
    d = lo - combatants[i].f65;
    if (d > 0) {
        m1 = d * 3;                     /* too weak for the weapon */
        p1 = d;
    }
    d = F_20a5_7726.v[w] - sk;
    if (d > 0) {
        m2 = d * 2;
        p2 = d * 2;
    }
    d = combatants[i].f65 - F_20a5_77a4.v[w];
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
    combatants[i].def = sk;
    combatants[i].atk = p2;
}

/* 1 if i can fire its readied weapon (no ammo needed, or has ammo of quality 25) */
int far can_fire(int i)
{
    union { char am; int d; } u;
    int r;

    u.am = F_20a5_7822.v[combatants[i].weapon];
    if (u.am == 0)
        r = 1;
    else
        r = sub_s014_18bf_0cfe(i, u.am, 0x19);
    return r;
}

/* use one ammo / thrown weapon of i; when it runs out switch back to the default weapon */
int far use_ammo(int i)
{
    struct { int id; char w; char pad; } s;
    int r;

    cbt_attack_rate[i] = 0xff;
    s.id = combatants[i].weapon;
    if ((s.w = F_20a5_7822.v[s.id]) == 0)
        r = consume_item(i, s.id, combatants[i].f5a);
    else
        r = consume_item(i, s.w, 0x19);
    if (r == -1) {
        if (F_20a5_7822.v[combatants[i].weapon] == 0)
            combatants[i].weapon = -1;
        if (i >= 5)
            sub_s014_1db7_05c8(i);
        cbt_action[i] = 0xd;
        D_a3d8[i] = 2;
        party_panel_rows(i);
        enemy_class[i] = 0;
        combatants[i].target = -1;
        weapon_ready(i, combatants[i].f51, 0);
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
        if (combatants[i].nitems > 0)
            for (k = 0, p = inventories[i]; k < combatants[i].nitems; k++, p++)
                if (p->f2 == id && p->f3 == q)
                    return 1;
    } else {
        if (combatants[i].nitems > 0)
            for (k = 0, p = enemy_items[i - 5]; k < combatants[i].nitems; k++, p++)
                if (p->f2 == id && p->f3 == q)
                    return 1;
    }
    return 0;
}
#pragma optimize("e", on)

/* consume one of item (id, q); returns the remaining count or -1 when gone / not found */
int far consume_item(int i, unsigned char id, unsigned char q)
{
    char *pn;
    int found;
    int k, n;
    struct item far *p;

    found = 0;
    if (i < 5) {
        pn = &combatants[i].nitems;
        if (*pn > 0)
            for (k = 0, p = inventories[i]; k < *pn; k++, p++)
                if (p->f2 == id && p->f3 == q) {
                    found = 1;
                    break;
                }
        if (!found)
            return -1;
        if ((n = inventories[i][k].cnt-- - 1) > 0)
            return n;
        if (k < *pn - 1)
            for (n = combatants[i].nitems - k - 1, p = &inventories[i][k]; n != 0; n--, p++)
                p[0] = p[1];
    } else {
        k = 0;
        pn = &combatants[i].nitems;
        if (*pn > 0)
            for (p = enemy_items[i - 5]; k < *pn; k++, p++)
                if (p->f2 == id && p->f3 == q) {
                    found = 1;
                    break;
                }
        if (!found)
            return -1;
        if ((n = enemy_items[i - 5][k].cnt-- - 1) > 0)
            return n;
        if (k < *pn - 1)
            for (n = combatants[i].nitems - k - 1, p = &enemy_items[i - 5][k]; n != 0; n--, p++)
                p[0] = p[1];
    }
    (*pn)--;
    return -1;
}

/* consume one of inventory slot k of i; -1 when the slot is used up */
int far consume_slot(int i, int k)
{
    int n;
    char *pn;
    int c;
    struct item far *p;

    if (i < 5) {
        if ((n = inventories[i][k].cnt-- - 1) < 1) {
            pn = &combatants[i].nitems;
            if (k < *pn - 1)
                for (p = &inventories[i][k], c = combatants[i].nitems - k - 1; c != 0; c--, p++)
                    p[0] = p[1];
            (*pn)--;
            return -1;
        }
    } else {
        if ((n = enemy_items[i - 5][k].cnt-- - 1) < 1) {
            pn = &combatants[i].nitems;
            if (k < *pn - 1)
                for (p = &enemy_items[i - 5][k], c = combatants[i].nitems - k - 1; c != 0; c--, p++)
                    p[0] = p[1];
            (*pn)--;
            return -1;
        }
    }
    return n;
}
