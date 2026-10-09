/* place: s031 18e7:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* data: _DATA=0000 */
/* Inventory helpers (overlay s031, second file): add / remove / move items in a party
   member's inventory and keep the equipment slots in step.
   who: party member 0..4, -1 = the current member (D_907b), -2 = every member.
   STATUS: 25/27 functions match.
     01b4: needs MSC CSE temps the original has (nitems byte at [bp-a], loop base at [bp-2]);
           best attempt keeps a `char n` (DIFF@f, see build/agent/e1/v_01b4n.txt).
     14b6: best variant (switch into temps t/i, then `type = t; q = i; i = 0; found = -1;`)
           differs only in the order of the two stores (original stores q first but keeps
           type at [bp-4]); see build/agent/e1/v_14x.txt.
     Session 2 (e1b): 14b6 tried struct/array holders (struct {found,q,type} gives the right
           store order but frame 10), volatile, dummy externs/locals, register hints, pragmas
           e/g/l/c/a/w/z/p/t: no match.  01b4: removing the named `char n` (pure CSE of
           D_9c69[who].nitems) DOES produce the dead [bp-2] loop-base temp, but then nitems is
           not cached in [bp-a]; any named char n (also block-scoped / array / struct /
           volatile / register) sits at [bp-3] above the SR temps and kills the [bp-2] temp.
           Variant: build/agent/e1/cse01b4.txt. */

/* party member record in DGROUP (0x80 bytes) */
struct member {
    char present;               /* +00 */
    char pad01[0x0d];
    char missile_type;          /* +0e  equipped missile weapon (item type, -1 none) */
    char pad0f[0x1b];
    char name[0x0b];            /* +2a */
    char pad35[2];              /* +35  +35: encumbrance */
    char vital_type;            /* +37  vital armour */
    char limb_type;             /* +38  limb armour */
    char pad39[2];
    char vital_q;               /* +3b */
    char limb_q;                /* +3c */
    char weapon_type;           /* +3d */
    char pad3e[6];
    char weapon_q;              /* +44 */
    char pad45;
    char missile_q;             /* +46 */
    char shield_q;              /* +47 */
    char shield_type;           /* +48 */
    char pad49[0x21];
    char nitems;                /* +6a */
    char pad6b[0x15];
};

/* one inventory entry (6 bytes) */
struct item {
    int code;                   /* LST item code (low 12 bits) */
    unsigned char type;         /* LST item type */
    unsigned char qual;         /* quality */
    unsigned char cnt;          /* quantity */
    unsigned char weight;
};

/* character record in far data (0x180 bytes); the items run on past the record */
struct fchar {
    char pad00[0x3e];
    struct item items[53];
    char pad17c[4];
};

/* item definition (DARKLAND.LST) */
struct itemdef {
    char name[0x14];
    char short_name[0x0a];
    char type;                  /* +1e */
    char pad1f;
    long flags;                 /* +20 */
    char pad24;
    char weight;                /* +25 */
    char quality;               /* +26 */
};

extern struct member D_9c69[];
extern char D_907b;
extern char D_99eb;
extern int D_906b, D_906d, D_906f;
extern struct fchar far F_2e38_0000[];
extern struct itemdef far * far F_2e38_0890[];

int far sub_root_0000_0c70(int);
int far sub_s029_0e76_01a0(int, int);
int far sub_s029_0e76_05ee(int, int);
int far sub_s029_1367_0066(int, int, int);

int far sub_s031_18e7_01b4(int who, struct item it);
void far sub_s031_18e7_0406(int who, int idx);
int far sub_s031_18e7_0474(int who, int idx);
int far sub_s031_18e7_07d4(int who, int idx);
void far sub_s031_18e7_12e8(int who, int idx, int slot);
int far sub_s031_18e7_140a(int who);
int far sub_s031_18e7_1460(int who, int slot);
int far sub_s031_18e7_14b6(int who, int slot);

/* give one of item idx of `from` to `to` */
void far sub_s031_18e7_0000(int from, int to, int idx, int slot)
{
    struct item it;

    if (to != from && D_9c69[to].nitems < 0x40) {
        if (slot == 6)
            idx = sub_s031_18e7_1460(from, idx);
        it = F_2e38_0000[from].items[idx];
        it.cnt = 1;
        sub_s031_18e7_01b4(to, it);
        sub_s031_18e7_0474(from, idx);
    }
}

/* give the whole stack idx of `from` to `to` */
void far sub_s031_18e7_007c(int from, int to, int idx, int slot)
{
    if (to != from && D_9c69[to].nitems < 0x40) {
        if (slot == 6)
            idx = sub_s031_18e7_1460(from, idx);
        sub_s031_18e7_01b4(to, F_2e38_0000[from].items[idx]);
        sub_s031_18e7_07d4(from, idx);
    }
}

/* add cnt items `code` of default quality */
void far sub_s031_18e7_00e0(int who, int code, int cnt)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = F_2e38_0890[c]->type;
    it.qual = F_2e38_0890[c]->quality;
    it.weight = F_2e38_0890[c]->weight;
    it.cnt = cnt;
    sub_s031_18e7_01b4(who, it);
}

/* add one item `code` of default quality */
void far sub_s031_18e7_0128(int who, int code)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = F_2e38_0890[c]->type;
    it.qual = F_2e38_0890[c]->quality;
    it.weight = F_2e38_0890[c]->weight;
    it.cnt = 1;
    sub_s031_18e7_01b4(who, it);
}

/* add one item `code` of quality qual */
void far sub_s031_18e7_016e(int who, int code, int qual)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = F_2e38_0890[c]->type;
    it.qual = qual;
    it.weight = F_2e38_0890[c]->weight;
    it.cnt = 1;
    sub_s031_18e7_01b4(who, it);
}

/* add an item: stack it onto an equal entry or append it; returns the item count */
int far sub_s031_18e7_01b4(int who, struct item it)
{
    int i;
    char n;

    if (who == -1) {
        who = D_907b;
        for (i = 0; i < D_9c69[who].nitems; i++) {
            if (F_2e38_0000[who].items[i].code == it.code && F_2e38_0000[who].items[i].type == it.type
                    && F_2e38_0000[who].items[i].qual == it.qual
                    && F_2e38_0000[who].items[i].cnt + it.cnt <= 0xff) {
                F_2e38_0000[who].items[i].cnt += it.cnt;
                return D_9c69[who].nitems;
            }
        }
        if ((n = D_9c69[who].nitems) >= 0x40)
            return n;
        F_2e38_0000[who].items[D_9c69[who].nitems] = it;
        D_9c69[who].nitems++;
    } else if (who == -2) {
        for (who = 0; who < 5; who++) {
            if (D_9c69[who].present) {
                n = D_9c69[who].nitems;
                for (i = 0; i < D_9c69[who].nitems; i++) {
                    if (F_2e38_0000[who].items[i].code == it.code && F_2e38_0000[who].items[i].type == it.type
                            && F_2e38_0000[who].items[i].qual == it.qual
                            && F_2e38_0000[who].items[i].cnt + it.cnt <= 0xff) {
                        F_2e38_0000[who].items[i].cnt += it.cnt;
                        break;
                    }
                }
                if (n < 0x40) {
                    F_2e38_0000[who].items[n] = it;
                    D_9c69[who].nitems++;
                }
            }
        }
    } else {
        n = D_9c69[who].nitems;
        for (i = 0; i < D_9c69[who].nitems; i++) {
            if (F_2e38_0000[who].items[i].code == it.code && F_2e38_0000[who].items[i].type == it.type
                    && F_2e38_0000[who].items[i].qual == it.qual
                    && F_2e38_0000[who].items[i].cnt + it.cnt <= 0xff) {
                F_2e38_0000[who].items[i].cnt += it.cnt;
                return D_9c69[who].nitems;
            }
        }
        if (D_9c69[who].nitems < 0x40) {
            F_2e38_0000[who].items[D_9c69[who].nitems] = it;
            D_9c69[who].nitems++;
        }
    }
    return D_9c69[who].nitems;
}

/* delete entry idx (closes the gap) */
void far sub_s031_18e7_0406(int who, int idx)
{
    int i;

    for (i = idx; i < D_9c69[who].nitems; i++)
        F_2e38_0000[who].items[i] = F_2e38_0000[who].items[i + 1];
    D_9c69[who].nitems--;
}

/* take one item off stack idx */
int far sub_s031_18e7_0474(int who, int idx)
{
    if (who == -1)
        who = D_907b;
    if (--F_2e38_0000[who].items[idx].cnt)
        return D_9c69[who].nitems;
    sub_s031_18e7_12e8(who, idx, 9);
    sub_s031_18e7_0406(who, idx);
    return D_9c69[who].nitems;
}

/* take one item `code` away; -1 if there is none */
int far sub_s031_18e7_04d4(int who, int code)
{
    int i, j;

    if (who == -1) {
        who = D_907b;
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code)
                break;
        if (i >= D_9c69[who].nitems)
            return -1;
        if (--F_2e38_0000[who].items[i].cnt)
            return D_9c69[who].nitems;
        sub_s031_18e7_12e8(who, i, 9);
        sub_s031_18e7_0406(who, i);
        return D_9c69[who].nitems;
    } else if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (D_9c69[j].present) {
                for (i = 0; i < D_9c69[j].nitems; i++)
                    if (F_2e38_0000[j].items[i].code == code)
                        break;
                if (i < D_9c69[j].nitems) {
                    if (--F_2e38_0000[j].items[i].cnt)
                        return D_9c69[j].nitems;
                    sub_s031_18e7_12e8(j, i, 9);
                    sub_s031_18e7_0406(j, i);
                    return D_9c69[j].nitems;
                }
            }
        }
        return -1;
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code)
                break;
        if (i >= D_9c69[who].nitems)
            return -1;
        if (--F_2e38_0000[who].items[i].cnt)
            return D_9c69[who].nitems;
        sub_s031_18e7_12e8(who, i, 9);
        sub_s031_18e7_0406(who, i);
        return D_9c69[who].nitems;
    }
}

/* remove the first stack of item `code`; -1 if there is none */
int far sub_s031_18e7_0668(int who, int code)
{
    int i, j;
    int r;

    if (who == -1) {
        who = D_907b;
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code)
                break;
        if (i >= D_9c69[who].nitems)
            return -1;
        sub_s031_18e7_12e8(who, i, 9);
        sub_s031_18e7_0406(who, i);
        return D_9c69[who].nitems;
    } else if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (D_9c69[j].present) {
                for (i = 0; i < D_9c69[j].nitems; i++)
                    if (F_2e38_0000[j].items[i].code == code)
                        break;
                if (i < D_9c69[j].nitems) {
                    sub_s031_18e7_12e8(j, i, 9);
                    sub_s031_18e7_0406(j, i);
                    r = D_9c69[j].nitems;
                }
            }
        }
        return r;
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code)
                break;
        if (i >= D_9c69[who].nitems)
            return -1;
        sub_s031_18e7_12e8(who, i, 9);
        sub_s031_18e7_0406(who, i);
        return D_9c69[who].nitems;
    }
}

/* remove stack idx */
int far sub_s031_18e7_07d4(int who, int idx)
{
    int n;

    if (who == -1) {
        sub_s031_18e7_12e8(who = D_907b, idx, 9);
        sub_s031_18e7_0406(who, idx);
        return D_9c69[who].nitems;
    } else if (who == -2) {
        for (who = 0; who < 5; who++) {
            if (D_9c69[who].present) {
                sub_s031_18e7_12e8(who, idx, 9);
                sub_s031_18e7_0406(who, idx);
                n += D_9c69[who].nitems;
            }
        }
        return n;
    }
    sub_s031_18e7_12e8(who, idx, 9);
    sub_s031_18e7_0406(who, idx);
    return D_9c69[who].nitems;
}

/* lose items: heavy stacks always, light ones by chance */
void far sub_s031_18e7_0854(int who)
{
    int i, j;
    int h;

    if (who == -1) {
        who = D_907b;
        h = (sub_s029_0e76_05ee(who, 2) + sub_s029_0e76_01a0(who, 15)) / 2;
        for (i = 0; i < D_9c69[who].nitems; ) {
            if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt > 2)
                sub_s031_18e7_07d4(who, i);
            else if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt == 2 && sub_root_0000_0c70(100) < h / 2)
                sub_s031_18e7_07d4(who, i);
            else if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt < 2 && sub_root_0000_0c70(100) < h)
                sub_s031_18e7_07d4(who, i);
            else
                i++;
        }
    } else if (who == -2) {
        D_906b = 0;
        D_906d = 0;
        D_906f = 0;
        for (j = 0; j < 5; j++) {
            if (D_9c69[j].present) {
                h = (sub_s029_0e76_05ee(j, 2) + sub_s029_0e76_01a0(j, 15)) / 2;
                for (i = 0; i < D_9c69[j].nitems; ) {
                    if (F_2e38_0000[j].items[i].weight * F_2e38_0000[j].items[i].cnt > 2)
                        sub_s031_18e7_07d4(j, i);
                    else if (F_2e38_0000[j].items[i].weight * F_2e38_0000[j].items[i].cnt == 2 && sub_root_0000_0c70(100) < h / 2)
                        sub_s031_18e7_07d4(j, i);
                    else if (F_2e38_0000[j].items[i].weight * F_2e38_0000[j].items[i].cnt < 2 && sub_root_0000_0c70(100) < h)
                        sub_s031_18e7_07d4(j, i);
                    else
                        i++;
                }
            }
        }
    } else {
        h = (sub_s029_0e76_05ee(who, 2) + sub_s029_0e76_01a0(who, 15)) / 2;
        for (i = 0; i < D_9c69[who].nitems; ) {
            if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt > 2)
                sub_s031_18e7_07d4(who, i);
            else if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt == 2 && sub_root_0000_0c70(100) < h / 2)
                sub_s031_18e7_07d4(who, i);
            else if (F_2e38_0000[who].items[i].weight * F_2e38_0000[who].items[i].cnt < 2 && sub_root_0000_0c70(100) < h)
                sub_s031_18e7_07d4(who, i);
            else
                i++;
        }
    }
}

/* each stack survives with probability pct% */
void far sub_s031_18e7_0ab8(int who, int pct)
{
    int i, j;

    if (who == -1) {
        who = D_907b;
        for (i = D_9c69[who].nitems; i >= 0; i--)
            if (sub_root_0000_0c70(100) > pct)
                sub_s031_18e7_07d4(who, i);
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (D_9c69[j].present)
                for (i = D_9c69[j].nitems; i >= 0; i--)
                    if (sub_root_0000_0c70(100) > pct)
                        sub_s031_18e7_07d4(j, i);
    } else {
        for (i = D_9c69[who].nitems; i >= 0; i--)
            if (sub_root_0000_0c70(100) > pct)
                sub_s031_18e7_07d4(who, i);
    }
}

/* remove all items of LST type `type` (unused) */
int far sub_s031_18e7_0b7e(int who, int type)
{
    int i, j;

    for (i = 0; i < D_9c69[who].nitems; i++) {
        if (F_2e38_0000[who].items[i].type == type) {
            sub_s031_18e7_12e8(who, i, 9);
            for (j = i; j < D_9c69[who].nitems; j++)
                F_2e38_0000[who].items[j] = F_2e38_0000[who].items[j + 1];
            D_9c69[who].nitems--;
        }
    }
    return D_9c69[who].nitems;
}

/* remove all items whose LST flags intersect mask */
int far sub_s031_18e7_0c4a(int who, long mask)
{
    int i, j, w;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (w = 0; w < 5; w++) {
            if (D_9c69[w].present) {
                for (i = 0; i < D_9c69[w].nitems; i++) {
                    if (F_2e38_0890[F_2e38_0000[w].items[i].code]->flags & mask) {
                        sub_s031_18e7_12e8(w, i, 9);
                        for (j = i; j < D_9c69[w].nitems; j++)
                            F_2e38_0000[w].items[j] = F_2e38_0000[w].items[j + 1];
                        D_9c69[w].nitems--;
                        i--;
                    }
                }
            }
        }
        return 0;
    }
    for (i = 0; i < D_9c69[who].nitems; i++) {
        if (F_2e38_0890[F_2e38_0000[who].items[i].code]->flags & mask) {
            sub_s031_18e7_12e8(who, i, 9);
            for (j = i; j < D_9c69[who].nitems; j++)
                F_2e38_0000[who].items[j] = F_2e38_0000[who].items[j + 1];
            D_9c69[who].nitems--;
            i--;
        }
    }
    return D_9c69[who].nitems;
}

/* take one item whose LST flags intersect mask */
int far sub_s031_18e7_0e14(int who, long mask)
{
    int i, j;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (D_9c69[j].present) {
                for (i = 0; i < D_9c69[j].nitems; i++) {
                    if (F_2e38_0890[F_2e38_0000[j].items[i].code]->flags & mask) {
                        if (--F_2e38_0000[j].items[i].cnt)
                            return D_9c69[j].nitems;
                        sub_s031_18e7_12e8(j, i, 9);
                        sub_s031_18e7_0406(j, i);
                        return D_9c69[j].nitems;
                    }
                }
            }
        }
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++) {
            if (F_2e38_0890[F_2e38_0000[who].items[i].code]->flags & mask) {
                if (--F_2e38_0000[who].items[i].cnt)
                    return D_9c69[who].nitems;
                sub_s031_18e7_12e8(who, i, 9);
                sub_s031_18e7_0406(who, i);
                return D_9c69[who].nitems;
            }
        }
    }
    return 0;
}

/* code of the first item whose LST flags intersect mask, 0 if none */
int far sub_s031_18e7_0fac(int who, long mask)
{
    int i, j;

    if (who == -1)
        who = D_907b;
    if (who == -2) {
        for (j = 0; j < 5; j++)
            if (D_9c69[j].present)
                for (i = 0; i < D_9c69[j].nitems; i++)
                    if (F_2e38_0890[F_2e38_0000[j].items[i].code]->flags & mask)
                        return F_2e38_0000[j].items[i].code;
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0890[F_2e38_0000[who].items[i].code]->flags & mask)
                return F_2e38_0000[who].items[i].code;
    }
    return 0;
}

/* move entry `from` to position `to` */
void far sub_s031_18e7_10d2(int who, int from, int to)
{
    struct item it;

    if (to != from && D_9c69[who].nitems > from && D_9c69[who].nitems > to) {
        it = F_2e38_0000[who].items[from];
        do {
            if (to < from) {
                F_2e38_0000[who].items[from] = F_2e38_0000[who].items[from - 1];
                from--;
            } else {
                F_2e38_0000[who].items[from] = F_2e38_0000[who].items[from + 1];
                from++;
            }
        } while (to != from);
        F_2e38_0000[who].items[to] = it;
    }
}

/* equip entry idx */
#pragma optimize("a", on)
void far sub_s031_18e7_11b2(int who, int idx)
{
    long flags;

    flags = F_2e38_0890[F_2e38_0000[who].items[idx].code & 0xfff]->flags;
    if (flags & 0xf) {
        D_9c69[who].weapon_type = F_2e38_0000[who].items[idx].type;
        D_9c69[who].weapon_q = F_2e38_0000[who].items[idx].qual;
    } else if (flags & 0x8000030L) {
        D_9c69[who].missile_type = F_2e38_0000[who].items[idx].type;
        D_9c69[who].missile_q = F_2e38_0000[who].items[idx].qual;
    } else if (F_2e38_0000[who].items[idx].type >= 0x43 && F_2e38_0000[who].items[idx].type < 0x4c) {
        D_9c69[who].vital_type = F_2e38_0000[who].items[idx].type;
        D_9c69[who].vital_q = F_2e38_0000[who].items[idx].qual;
    } else if (F_2e38_0000[who].items[idx].type >= 0x4c && F_2e38_0000[who].items[idx].type < 0x5f) {
        D_9c69[who].limb_type = F_2e38_0000[who].items[idx].type;
        D_9c69[who].limb_q = F_2e38_0000[who].items[idx].qual;
    } else if (F_2e38_0000[who].items[idx].type >= 0x5f && F_2e38_0000[who].items[idx].type <= 0x61) {
        D_9c69[who].shield_type = F_2e38_0000[who].items[idx].type;
        D_9c69[who].shield_q = F_2e38_0000[who].items[idx].qual;
    }
    sub_s031_18e7_140a(who);
}
#pragma optimize("", on)

/* unequip entry idx if it is equipped */
void far sub_s031_18e7_12e8(int who, int idx, int slot)
{
    int code, type, q;

    if (slot == 6)
        idx = sub_s031_18e7_1460(who, idx);
    code = F_2e38_0000[who].items[idx].code & 0xfff;
    type = F_2e38_0000[who].items[idx].type;
    q = F_2e38_0000[who].items[idx].qual;
    if ((F_2e38_0890[code]->flags & 0xf) && D_9c69[who].weapon_type == type && D_9c69[who].weapon_q == q)
        D_9c69[who].weapon_type = -1;
    else if ((F_2e38_0890[code]->flags & 0x8000030L) && D_9c69[who].missile_type == type
             && D_9c69[who].missile_q == q)
        D_9c69[who].missile_type = -1;
    else if (D_9c69[who].vital_type == type && D_9c69[who].vital_q == q)
        D_9c69[who].vital_type = -1;
    else if (D_9c69[who].limb_type == type && D_9c69[who].limb_q == q)
        D_9c69[who].limb_type = -1;
    else if (D_9c69[who].shield_type == type && D_9c69[who].shield_q == q)
        D_9c69[who].shield_type = -1;
    sub_s031_18e7_140a(who);
}

/* recompute the weight of the equipped items */
int far sub_s031_18e7_140a(int who)
{
    int slot, code, w;

    w = 0;
    for (slot = 0; slot < 5; slot++) {
        code = sub_s031_18e7_14b6(who, slot);
        if (code != -1)
            w += F_2e38_0890[code]->weight;
    }
    return D_9c69[who].pad35[0] = sub_s029_1367_0066(0, 500, w);
}

/* item type equipped in slot */
int far sub_s031_18e7_1460(int who, int slot)
{
    switch (slot) {
    case 0:
        return D_9c69[who].weapon_type;
    case 1:
        return D_9c69[who].vital_type;
    case 2:
        return D_9c69[who].limb_type;
    case 3:
        return D_9c69[who].shield_type;
    case 4:
        return D_9c69[who].missile_type;
    }
}

/* item code equipped in slot, -1 if none */
int far sub_s031_18e7_14b6(int who, int slot)
{
    int type, q, i, found, t;

    switch (slot) {
    case 0:
        t = D_9c69[who].weapon_type;
        i = D_9c69[who].weapon_q;
        break;
    case 1:
        t = D_9c69[who].vital_type;
        i = D_9c69[who].vital_q;
        break;
    case 2:
        t = D_9c69[who].limb_type;
        i = D_9c69[who].limb_q;
        break;
    case 3:
        t = D_9c69[who].shield_type;
        i = D_9c69[who].shield_q;
        break;
    case 4:
        t = D_9c69[who].missile_type;
        i = D_9c69[who].missile_q;
        break;
    default:
        goto fail;
    }
    if (t == -1)
        goto fail;
    type = t;
    q = i;
    i = 0;
    found = -1;
    for (; i < D_9c69[who].nitems; i++) {
        if (F_2e38_0000[who].items[i].type == type) {
            found = i;
            D_99eb = i;
            if (F_2e38_0000[who].items[i].qual == q)
                return F_2e38_0000[who].items[i].code;
        }
    }
    if (found != -1)
        return F_2e38_0000[who].items[found].code;
fail:
    return -1;
}

/* quality of entry idx */
int far sub_s031_18e7_15d6(int who, int idx)
{
    return F_2e38_0000[who].items[idx].qual;
}

/* best quality of item `code` */
int far sub_s031_18e7_15f8(int who, int code)
{
    int i, j, best;

    best = 0;
    if (who == -1) {
        who = D_907b;
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code && F_2e38_0000[who].items[i].qual > best)
                best = F_2e38_0000[who].items[i].qual;
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (D_9c69[j].present && D_9c69[j].nitems > 0)
                for (i = 0; i < D_9c69[j].nitems; i++)
                    if (F_2e38_0000[j].items[i].code == code && F_2e38_0000[j].items[i].qual > best)
                        best = F_2e38_0000[j].items[i].qual;
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0000[who].items[i].code == code && F_2e38_0000[who].items[i].qual > best)
                best = F_2e38_0000[who].items[i].qual;
    }
    return best;
}

/* best quality of the items with LST flags == flags */
int far sub_s031_18e7_171c(int who, long flags)
{
    int i, j, best;

    best = 0;
    if (who == -1) {
        who = D_907b;
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0890[0]->flags == flags && F_2e38_0000[who].items[i].qual > best)
                best = F_2e38_0000[who].items[i].qual;
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (D_9c69[j].present && D_9c69[j].nitems > 0)
                for (i = 0; i < D_9c69[j].nitems; i++)
                    if (F_2e38_0890[0]->flags == flags && F_2e38_0000[j].items[i].qual > best)
                        best = F_2e38_0000[j].items[i].qual;
    } else {
        for (i = 0; i < D_9c69[who].nitems; i++)
            if (F_2e38_0890[0]->flags == flags && F_2e38_0000[who].items[i].qual > best)
                best = F_2e38_0000[who].items[i].qual;
    }
    return best;
}

/* a random item code whose LST flags == flags */
int far sub_s031_18e7_1894(long flags)
{
    int i, n, k;

    n = 0;
    for (i = 0; i < 172; i++)
        if (F_2e38_0890[i]->flags == flags)
            n++;
    k = sub_root_0000_0c70(n);
    for (i = 0; i < 172; i++) {
        if (F_2e38_0890[i]->flags == flags) {
            if (k == 0)
                return i;
            k--;
        }
    }
}
