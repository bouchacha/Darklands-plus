/* place: s031 18e7:0000 */
/* orig: size=191c CONST=7e62+0004 */
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
           Variant: build/agent/e1/cse01b4.txt.
     Session 4 (e1d): 14b6 slot order follows the first stores after DSE (dead early
           `type = 0;` / `(void)type;` and `type = t, q = i;` change nothing; int v[3] / struct /
           union holders are worse).  01b4: the CSE variant with pragma a on/off still compares
           nitems straight from memory instead of saving it in the [bp-0ah] byte temp.
     Session e4: 14b6 `q = i; type = t;` gives the original store order but q's slot moves
           above type (slots follow first-store order); `if ((type = t) == -1)` / `if (t != -1)
           { ... }` / found = -1 before the test: no match (best stays 2 lines).
     Session m0: 14b6 `int q[1]; q[0] = i; type = t;` gives the original store order (aggregates
           go below scalars) but then found lands above q; found in a struct/array changes
           register allocation (who di -> si). struct {q,type} / {found,q,type}: worse. */

/* party member record in DGROUP (0x80 bytes) */
#include "darklands/card.h"
#include "darklands/game.h"
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

extern struct member party[];
extern char leader;
extern char best_skill;
extern int party_money, party_money_gr, party_money_pf;
extern struct fchar far data_file[];
extern struct itemdef far * far item_defs[];

int far rand_mod(int);
int far member_skill(int, int);
int far member_attr(int, int);
int far clamp_range(int, int, int);

int far add_item_rec_impl(int who, struct item it);
void far item_delete(int who, int idx);
int far item_take_one(int who, int idx);
int far item_remove_stack(int who, int idx);
void far item_unequip(int who, int idx, int slot);
int far equip_weight(int who);
int far equipped_type(int who, int slot);
int far equipped_item_impl(int who, int slot);

/* give one of item idx of `from` to `to` */
void far give_one(int from, int to, int idx, int slot)
{
    struct item it;

    if (to != from && party[to].nitems < 0x40) {
        if (slot == 6)
            idx = equipped_type(from, idx);
        it = data_file[from].items[idx];
        it.cnt = 1;
        add_item_rec_impl(to, it);
        item_take_one(from, idx);
    }
}

/* give the whole stack idx of `from` to `to` */
void far give_stack(int from, int to, int idx, int slot)
{
    if (to != from && party[to].nitems < 0x40) {
        if (slot == 6)
            idx = equipped_type(from, idx);
        add_item_rec_impl(to, data_file[from].items[idx]);
        item_remove_stack(from, idx);
    }
}

/* add cnt items `code` of default quality */
void far add_items_impl(int who, int code, int cnt)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = item_defs[c]->type;
    it.qual = item_defs[c]->quality;
    it.weight = item_defs[c]->weight;
    it.cnt = cnt;
    add_item_rec_impl(who, it);
}

/* add one item `code` of default quality */
void far add_item_impl(int who, int code)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = item_defs[c]->type;
    it.qual = item_defs[c]->quality;
    it.weight = item_defs[c]->weight;
    it.cnt = 1;
    add_item_rec_impl(who, it);
}

/* add one item `code` of quality qual */
void far add_item_q(int who, int code, int qual)
{
    struct item it;
    int c;

    c = code;
    it.code = c;
    it.type = item_defs[c]->type;
    it.qual = qual;
    it.weight = item_defs[c]->weight;
    it.cnt = 1;
    add_item_rec_impl(who, it);
}

/* add an item: stack it onto an equal entry or append it; returns the item count */
int far add_item_rec_impl(int who, struct item it)
{
    int i;
    char n;

    if (who == -1) {
        who = leader;
        for (i = 0; i < party[who].nitems; i++) {
            if (data_file[who].items[i].code == it.code && data_file[who].items[i].type == it.type
                    && data_file[who].items[i].qual == it.qual
                    && data_file[who].items[i].cnt + it.cnt <= 0xff) {
                data_file[who].items[i].cnt += it.cnt;
                return party[who].nitems;
            }
        }
        if ((n = party[who].nitems) >= 0x40)
            return n;
        data_file[who].items[party[who].nitems] = it;
        party[who].nitems++;
    } else if (who == -2) {
        for (who = 0; who < 5; who++) {
            if (party[who].present) {
                n = party[who].nitems;
                for (i = 0; i < party[who].nitems; i++) {
                    if (data_file[who].items[i].code == it.code && data_file[who].items[i].type == it.type
                            && data_file[who].items[i].qual == it.qual
                            && data_file[who].items[i].cnt + it.cnt <= 0xff) {
                        data_file[who].items[i].cnt += it.cnt;
                        break;
                    }
                }
                if (n < 0x40) {
                    data_file[who].items[n] = it;
                    party[who].nitems++;
                }
            }
        }
    } else {
        n = party[who].nitems;
        for (i = 0; i < party[who].nitems; i++) {
            if (data_file[who].items[i].code == it.code && data_file[who].items[i].type == it.type
                    && data_file[who].items[i].qual == it.qual
                    && data_file[who].items[i].cnt + it.cnt <= 0xff) {
                data_file[who].items[i].cnt += it.cnt;
                return party[who].nitems;
            }
        }
        if (party[who].nitems < 0x40) {
            data_file[who].items[party[who].nitems] = it;
            party[who].nitems++;
        }
    }
    return party[who].nitems;
}

/* delete entry idx (closes the gap) */
void far item_delete(int who, int idx)
{
    int i;

    for (i = idx; i < party[who].nitems; i++)
        data_file[who].items[i] = data_file[who].items[i + 1];
    party[who].nitems--;
}

/* take one item off stack idx */
int far item_take_one(int who, int idx)
{
    if (who == -1)
        who = leader;
    if (--data_file[who].items[idx].cnt)
        return party[who].nitems;
    item_unequip(who, idx, 9);
    item_delete(who, idx);
    return party[who].nitems;
}

/* take one item `code` away; -1 if there is none */
int far take_item_impl(int who, int code)
{
    int i, j;

    if (who == -1) {
        who = leader;
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code)
                break;
        if (i >= party[who].nitems)
            return -1;
        if (--data_file[who].items[i].cnt)
            return party[who].nitems;
        item_unequip(who, i, 9);
        item_delete(who, i);
        return party[who].nitems;
    } else if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (party[j].present) {
                for (i = 0; i < party[j].nitems; i++)
                    if (data_file[j].items[i].code == code)
                        break;
                if (i < party[j].nitems) {
                    if (--data_file[j].items[i].cnt)
                        return party[j].nitems;
                    item_unequip(j, i, 9);
                    item_delete(j, i);
                    return party[j].nitems;
                }
            }
        }
        return -1;
    } else {
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code)
                break;
        if (i >= party[who].nitems)
            return -1;
        if (--data_file[who].items[i].cnt)
            return party[who].nitems;
        item_unequip(who, i, 9);
        item_delete(who, i);
        return party[who].nitems;
    }
}

/* remove the first stack of item `code`; -1 if there is none */
int far remove_item_stack_impl(int who, int code)
{
    int i, j;
    int r;

    if (who == -1) {
        who = leader;
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code)
                break;
        if (i >= party[who].nitems)
            return -1;
        item_unequip(who, i, 9);
        item_delete(who, i);
        return party[who].nitems;
    } else if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (party[j].present) {
                for (i = 0; i < party[j].nitems; i++)
                    if (data_file[j].items[i].code == code)
                        break;
                if (i < party[j].nitems) {
                    item_unequip(j, i, 9);
                    item_delete(j, i);
                    r = party[j].nitems;
                }
            }
        }
        return r;
    } else {
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code)
                break;
        if (i >= party[who].nitems)
            return -1;
        item_unequip(who, i, 9);
        item_delete(who, i);
        return party[who].nitems;
    }
}

/* remove stack idx */
int far item_remove_stack(int who, int idx)
{
    int n;

    if (who == -1) {
        item_unequip(who = leader, idx, 9);
        item_delete(who, idx);
        return party[who].nitems;
    } else if (who == -2) {
        for (who = 0; who < 5; who++) {
            if (party[who].present) {
                item_unequip(who, idx, 9);
                item_delete(who, idx);
                n += party[who].nitems;
            }
        }
        return n;
    }
    item_unequip(who, idx, 9);
    item_delete(who, idx);
    return party[who].nitems;
}

/* lose items: heavy stacks always, light ones by chance */
void far lose_items_impl(int who)
{
    int i, j;
    int h;

    if (who == -1) {
        who = leader;
        h = (member_attr(who, 2) + member_skill(who, 15)) / 2;
        for (i = 0; i < party[who].nitems; ) {
            if (data_file[who].items[i].weight * data_file[who].items[i].cnt > 2)
                item_remove_stack(who, i);
            else if (data_file[who].items[i].weight * data_file[who].items[i].cnt == 2 && rand_mod(100) < h / 2)
                item_remove_stack(who, i);
            else if (data_file[who].items[i].weight * data_file[who].items[i].cnt < 2 && rand_mod(100) < h)
                item_remove_stack(who, i);
            else
                i++;
        }
    } else if (who == -2) {
        party_money = 0;
        party_money_gr = 0;
        party_money_pf = 0;
        for (j = 0; j < 5; j++) {
            if (party[j].present) {
                h = (member_attr(j, 2) + member_skill(j, 15)) / 2;
                for (i = 0; i < party[j].nitems; ) {
                    if (data_file[j].items[i].weight * data_file[j].items[i].cnt > 2)
                        item_remove_stack(j, i);
                    else if (data_file[j].items[i].weight * data_file[j].items[i].cnt == 2 && rand_mod(100) < h / 2)
                        item_remove_stack(j, i);
                    else if (data_file[j].items[i].weight * data_file[j].items[i].cnt < 2 && rand_mod(100) < h)
                        item_remove_stack(j, i);
                    else
                        i++;
                }
            }
        }
    } else {
        h = (member_attr(who, 2) + member_skill(who, 15)) / 2;
        for (i = 0; i < party[who].nitems; ) {
            if (data_file[who].items[i].weight * data_file[who].items[i].cnt > 2)
                item_remove_stack(who, i);
            else if (data_file[who].items[i].weight * data_file[who].items[i].cnt == 2 && rand_mod(100) < h / 2)
                item_remove_stack(who, i);
            else if (data_file[who].items[i].weight * data_file[who].items[i].cnt < 2 && rand_mod(100) < h)
                item_remove_stack(who, i);
            else
                i++;
        }
    }
}

/* each stack survives with probability pct% */
void far keep_items_pct_impl(int who, int pct)
{
    int i, j;

    if (who == -1) {
        who = leader;
        for (i = party[who].nitems; i >= 0; i--)
            if (rand_mod(100) > pct)
                item_remove_stack(who, i);
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (party[j].present)
                for (i = party[j].nitems; i >= 0; i--)
                    if (rand_mod(100) > pct)
                        item_remove_stack(j, i);
    } else {
        for (i = party[who].nitems; i >= 0; i--)
            if (rand_mod(100) > pct)
                item_remove_stack(who, i);
    }
}

/* remove all items of LST type `type` (unused) */
int far sub_s031_18e7_0b7e(int who, int type)
{
    int i, j;

    for (i = 0; i < party[who].nitems; i++) {
        if (data_file[who].items[i].type == type) {
            item_unequip(who, i, 9);
            for (j = i; j < party[who].nitems; j++)
                data_file[who].items[j] = data_file[who].items[j + 1];
            party[who].nitems--;
        }
    }
    return party[who].nitems;
}

/* remove all items whose LST flags intersect mask */
int far remove_items_flags_impl(int who, long mask)
{
    int i, j, w;

    if (who == -1)
        who = leader;
    if (who == -2) {
        for (w = 0; w < 5; w++) {
            if (party[w].present) {
                for (i = 0; i < party[w].nitems; i++) {
                    if (item_defs[data_file[w].items[i].code]->flags & mask) {
                        item_unequip(w, i, 9);
                        for (j = i; j < party[w].nitems; j++)
                            data_file[w].items[j] = data_file[w].items[j + 1];
                        party[w].nitems--;
                        i--;
                    }
                }
            }
        }
        return 0;
    }
    for (i = 0; i < party[who].nitems; i++) {
        if (item_defs[data_file[who].items[i].code]->flags & mask) {
            item_unequip(who, i, 9);
            for (j = i; j < party[who].nitems; j++)
                data_file[who].items[j] = data_file[who].items[j + 1];
            party[who].nitems--;
            i--;
        }
    }
    return party[who].nitems;
}

/* take one item whose LST flags intersect mask */
int far sub_s031_18e7_0e14(int who, long mask)
{
    int i, j;

    if (who == -1)
        who = leader;
    if (who == -2) {
        for (j = 0; j < 5; j++) {
            if (party[j].present) {
                for (i = 0; i < party[j].nitems; i++) {
                    if (item_defs[data_file[j].items[i].code]->flags & mask) {
                        if (--data_file[j].items[i].cnt)
                            return party[j].nitems;
                        item_unequip(j, i, 9);
                        item_delete(j, i);
                        return party[j].nitems;
                    }
                }
            }
        }
    } else {
        for (i = 0; i < party[who].nitems; i++) {
            if (item_defs[data_file[who].items[i].code]->flags & mask) {
                if (--data_file[who].items[i].cnt)
                    return party[who].nitems;
                item_unequip(who, i, 9);
                item_delete(who, i);
                return party[who].nitems;
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
        who = leader;
    if (who == -2) {
        for (j = 0; j < 5; j++)
            if (party[j].present)
                for (i = 0; i < party[j].nitems; i++)
                    if (item_defs[data_file[j].items[i].code]->flags & mask)
                        return data_file[j].items[i].code;
    } else {
        for (i = 0; i < party[who].nitems; i++)
            if (item_defs[data_file[who].items[i].code]->flags & mask)
                return data_file[who].items[i].code;
    }
    return 0;
}

/* move entry `from` to position `to` */
void far item_move(int who, int from, int to)
{
    struct item it;

    if (to != from && party[who].nitems > from && party[who].nitems > to) {
        it = data_file[who].items[from];
        do {
            if (to < from) {
                data_file[who].items[from] = data_file[who].items[from - 1];
                from--;
            } else {
                data_file[who].items[from] = data_file[who].items[from + 1];
                from++;
            }
        } while (to != from);
        data_file[who].items[to] = it;
    }
}

/* equip entry idx */
#pragma optimize("a", on)
void far sub_s031_18e7_11b2(int who, int idx)
{
    long flags;

    flags = item_defs[data_file[who].items[idx].code & 0xfff]->flags;
    if (flags & 0xf) {
        party[who].weapon_type = data_file[who].items[idx].type;
        party[who].weapon_q = data_file[who].items[idx].qual;
    } else if (flags & 0x8000030L) {
        party[who].missile_type = data_file[who].items[idx].type;
        party[who].missile_q = data_file[who].items[idx].qual;
    } else if (data_file[who].items[idx].type >= 0x43 && data_file[who].items[idx].type < 0x4c) {
        party[who].vital_type = data_file[who].items[idx].type;
        party[who].vital_q = data_file[who].items[idx].qual;
    } else if (data_file[who].items[idx].type >= 0x4c && data_file[who].items[idx].type < 0x5f) {
        party[who].limb_type = data_file[who].items[idx].type;
        party[who].limb_q = data_file[who].items[idx].qual;
    } else if (data_file[who].items[idx].type >= 0x5f && data_file[who].items[idx].type <= 0x61) {
        party[who].shield_type = data_file[who].items[idx].type;
        party[who].shield_q = data_file[who].items[idx].qual;
    }
    equip_weight(who);
}
#pragma optimize("", on)

/* unequip entry idx if it is equipped */
void far item_unequip(int who, int idx, int slot)
{
    int code, type, q;

    if (slot == 6)
        idx = equipped_type(who, idx);
    code = data_file[who].items[idx].code & 0xfff;
    type = data_file[who].items[idx].type;
    q = data_file[who].items[idx].qual;
    if ((item_defs[code]->flags & 0xf) && party[who].weapon_type == type && party[who].weapon_q == q)
        party[who].weapon_type = -1;
    else if ((item_defs[code]->flags & 0x8000030L) && party[who].missile_type == type
             && party[who].missile_q == q)
        party[who].missile_type = -1;
    else if (party[who].vital_type == type && party[who].vital_q == q)
        party[who].vital_type = -1;
    else if (party[who].limb_type == type && party[who].limb_q == q)
        party[who].limb_type = -1;
    else if (party[who].shield_type == type && party[who].shield_q == q)
        party[who].shield_type = -1;
    equip_weight(who);
}

/* recompute the weight of the equipped items */
int far equip_weight(int who)
{
    int slot, code, w;

    w = 0;
    for (slot = 0; slot < 5; slot++) {
        code = equipped_item_impl(who, slot);
        if (code != -1)
            w += item_defs[code]->weight;
    }
    return party[who].pad35[0] = clamp_range(0, 500, w);
}

/* item type equipped in slot */
int far equipped_type(int who, int slot)
{
    switch (slot) {
    case 0:
        return party[who].weapon_type;
    case 1:
        return party[who].vital_type;
    case 2:
        return party[who].limb_type;
    case 3:
        return party[who].shield_type;
    case 4:
        return party[who].missile_type;
    }
}

/* item code equipped in slot, -1 if none */
int far equipped_item_impl(int who, int slot)
{
    int type, q, i, found, t;

    switch (slot) {
    case 0:
        t = party[who].weapon_type;
        i = party[who].weapon_q;
        break;
    case 1:
        t = party[who].vital_type;
        i = party[who].vital_q;
        break;
    case 2:
        t = party[who].limb_type;
        i = party[who].limb_q;
        break;
    case 3:
        t = party[who].shield_type;
        i = party[who].shield_q;
        break;
    case 4:
        t = party[who].missile_type;
        i = party[who].missile_q;
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
    for (; i < party[who].nitems; i++) {
        if (data_file[who].items[i].type == type) {
            found = i;
            best_skill = i;
            if (data_file[who].items[i].qual == q)
                return data_file[who].items[i].code;
        }
    }
    if (found != -1)
        return data_file[who].items[found].code;
fail:
    return -1;
}

/* quality of entry idx */
int far item_entry_quality(int who, int idx)
{
    return data_file[who].items[idx].qual;
}

/* best quality of item `code` */
int far item_quality_impl(int who, int code)
{
    int i, j, best;

    best = 0;
    if (who == -1) {
        who = leader;
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code && data_file[who].items[i].qual > best)
                best = data_file[who].items[i].qual;
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (party[j].present && party[j].nitems > 0)
                for (i = 0; i < party[j].nitems; i++)
                    if (data_file[j].items[i].code == code && data_file[j].items[i].qual > best)
                        best = data_file[j].items[i].qual;
    } else {
        for (i = 0; i < party[who].nitems; i++)
            if (data_file[who].items[i].code == code && data_file[who].items[i].qual > best)
                best = data_file[who].items[i].qual;
    }
    return best;
}

/* best quality of the items with LST flags == flags */
int far item_quality_flags_impl(int who, long flags)
{
    int i, j, best;

    best = 0;
    if (who == -1) {
        who = leader;
        for (i = 0; i < party[who].nitems; i++)
            if (item_defs[0]->flags == flags && data_file[who].items[i].qual > best)
                best = data_file[who].items[i].qual;
    } else if (who == -2) {
        for (j = 0; j < 5; j++)
            if (party[j].present && party[j].nitems > 0)
                for (i = 0; i < party[j].nitems; i++)
                    if (item_defs[0]->flags == flags && data_file[j].items[i].qual > best)
                        best = data_file[j].items[i].qual;
    } else {
        for (i = 0; i < party[who].nitems; i++)
            if (item_defs[0]->flags == flags && data_file[who].items[i].qual > best)
                best = data_file[who].items[i].qual;
    }
    return best;
}

/* a random item code whose LST flags == flags */
int far random_item_flags(long flags)
{
    int i, n, k;

    n = 0;
    for (i = 0; i < 172; i++)
        if (item_defs[i]->flags == flags)
            n++;
    k = rand_mod(n);
    for (i = 0; i < 172; i++) {
        if (item_defs[i]->flags == flags) {
            if (k == 0)
                return i;
            k--;
        }
    }
}
