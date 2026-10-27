// professions.cpp - see professions.h.
#include <stdio.h>
#include "professions.h"

bool cg_prof_rule(int i, const CgCtx &c)
{
    switch (i) {                                  /* (s037 18c8:22be, as decompiled: src/wip/s037/s037_18c8.c) */
    case 0:
        if (!(c.taken[0] & 1) && c.age < 0x1e)
            return true;
        else if (!(c.taken[0] & 1) && c.age < 0x1a && c.attr[1] >= 0x19)
            return true;
        return false;
    case 1:
        if ((c.taken[0] & 1) || (c.taken[2] & 1) || (c.taken[3] & 1) || (c.taken[5] & 1)
            || (c.taken[20] & 1) || (c.taken[36] & 1) || (c.taken[1] & 1))
            return true;
        return false;
    case 2:
        if ((c.taken[1] & 1) || (c.taken[2] & 1) || (c.taken[5] & 1) || (c.taken[3] & 1))
            return true;
        return false;
    case 3:
        if (c.attr[4] >= 0x14 && c.attr[3] >= 0x14 && c.attr[5] >= 0x14 && ((c.taken[6] & 1)
            || (c.taken[2] & 1) || (c.taken[7] & 1) || (c.taken[5] & 1) || (c.taken[9] & 1)
            || (c.taken[20] & 1) || (c.taken[3] & 1)))
            return true;
        return false;
    case 4:
        if ((c.taken[6] & 1) || (c.taken[4] & 1) || c.child == 0)
            return true;
        return false;
    case 5:
        if (c.skill[9] > 0xf && ((c.age < 0x14 && c.child == 0) || (c.taken[7] & 1)
            || (c.taken[5] & 1) || c.last == 4 || c.last == 6 || c.last == 9 || c.last == 0xf
            || c.last == 3))
            return true;
        return false;
    case 6:
        if ((c.age < 0x14 && (c.child == 0 || c.child == 1)) || (c.taken[7] & 1)
            || (c.taken[6] & 1) || c.last == 5 || c.last == 8 || c.last == 9 || c.last == 0xf
            || c.last == 0x14 || c.last == 3)
            return true;
        return false;
    case 7:
        if (c.last == 7 || ((c.prev == 4 || c.prev == 6 || c.prev == 9 || c.prev == 0xf)
            && (c.last == 4 || c.last == 6 || c.last == 9 || c.last == 0xf)))
            return true;
        return false;
    case 8:
        if (c.female != 1 && c.attr[4] >= 0x14 && c.attr[3] >= 0x14 && c.attr[5] >= 0x14
            && ((c.taken[7] & 1) || (c.taken[9] & 1) || (c.taken[15] & 1) || (c.taken[8] & 1)
            || c.last == 6 || c.last == 0xe || c.last == 0x14 || c.last == 0x16
            || c.last == 0x18 || c.last == 4 || ((c.prev == 0xc || c.prev == 0xd
            || c.prev == 0x15 || c.prev == 0xa) && (c.last == 0xc || c.last == 0xd
            || c.last == 0x15 || c.last == 0xa))))
            return true;
        return false;
    case 9:
        if (c.female != 1 && c.attr[4] >= 0x19 && c.attr[3] >= 0x19 && c.attr[5] >= 0x19
            && (c.last == 9 || c.last == 0xf || ((c.prev == 7 || c.prev == 8 || c.prev == 6)
            && (c.last == 7 || c.last == 8 || c.last == 6))))
            return true;
        return false;
    case 10:
        if (c.female != 1 && (c.last == 8 || c.last == 0xa || c.last == 0xb || c.last == 0xd
            || c.last == 0xe || c.last == 0xf))
            return true;
        return false;
    case 11:
        if (c.skill[9] >= 0xf)
            return true;
        return false;
    case 12:
        if ((c.age < 0x14 && c.child != 4 && c.child != 5) || (c.attr[4] >= 0xc
            && !(c.taken[13] & 1) && !(c.taken[14] & 1) && !(c.taken[15] & 1)
            && !(c.taken[8] & 1) && !(c.taken[9] & 1) && !(c.taken[10] & 1)
            && !(c.taken[15] & 1) && !(c.taken[8] & 1) && !(c.taken[22] & 1)
            && !(c.taken[23] & 1) && !(c.taken[24] & 1) && !(c.taken[25] & 1)))
            return true;
        return false;
    case 13:
        if (c.age < 0x14 || (!(c.taken[13] & 1) && !(c.taken[4] & 1) && !(c.taken[6] & 1)
            && !(c.taken[7] & 1) && !(c.taken[14] & 1) && !(c.taken[15] & 1)
            && !(c.taken[8] & 1) && !(c.taken[9] & 1) && !(c.taken[22] & 1)
            && !(c.taken[23] & 1) && !(c.taken[24] & 1) && !(c.taken[25] & 1)
            && !(c.taken[10] & 1)))
            return true;
        return false;
    case 14:
        if (c.attr[4] >= 0xf && c.attr[3] >= 0xf && c.attr[5] >= 0xf && c.skill[8] >= 5
            && ((c.taken[4] & 1) || (c.taken[6] & 1) || (c.taken[13] & 1) || (c.taken[7] & 1)
            || (c.taken[14] & 1) || (c.taken[15] & 1) || (c.taken[8] & 1) || (c.taken[9] & 1)
            || (c.taken[22] & 1) || (c.taken[23] & 1) || (c.taken[24] & 1) || (c.taken[25] & 1)
            || (c.taken[10] & 1) || (c.taken[21] & 1)))
            return true;
        return false;
    case 15:
        if (c.attr[4] >= 0x14 && c.attr[3] >= 0x14 && c.attr[5] >= 0x14 && c.skill[8] >= 0xf
            && c.attr[4] >= 0xc && (c.last == 4 || c.last == 6 || c.last == 8 || c.last == 7
            || c.last == 9 || c.last == 0xf || ((c.prev == 0xe || c.prev == 0x18)
            && (c.last == 0xe || c.last == 0x18))))
            return true;
        return false;
    case 16:
        if ((c.age < 0x14 && (c.child == 4 || c.child == 5)) || (c.last != 4 && c.last != 3
            && c.last != 5 && c.last != 6 && c.last != 8 && c.last != 0x12 && c.last != 0x18
            && c.last != 0x19 && c.last != 0x1c && (!c.taken[7] & 1) && (!c.taken[15] & 1)
            && (!c.taken[9] & 1) && (!c.taken[19] & 1) && (!c.taken[20] & 1)
            && (!c.taken[26] & 1) && (!c.taken[29] & 1)))
            return true;
        return false;
    case 17:
        if ((c.age < 0x14 && c.child != 4 && c.child != 5) || (c.skill[10] >= 5
            && c.attr[4] >= 0xc && ((c.taken[3] & 1) || (c.taken[4] & 1) || (c.taken[6] & 1)
            || (c.taken[8] & 1) || (c.taken[9] & 1) || (c.taken[14] & 1) || (c.taken[16] & 1)
            || (c.taken[15] & 1) || (c.taken[18] & 1) || (c.taken[22] & 1) || (c.taken[23] & 1)
            || (c.taken[24] & 1) || (c.taken[25] & 1) || (c.taken[28] & 1) || (c.taken[29] & 1)
            || (c.taken[32] & 1) || (c.taken[17] & 1))))
            return true;
        return false;
    case 18:
        if ((c.age < 0x14 && (c.child == 0 || c.child == 1)) || (c.skill[10] >= 0xa
            && c.attr[4] >= 0xf && ((c.taken[17] & 1) || (c.taken[4] & 1) || (c.taken[7] & 1)
            || (c.taken[19] & 1) || (c.taken[18] & 1) || (c.taken[24] & 1) || (c.taken[26] & 1)
            || (c.taken[29] & 1))))
            return true;
        return false;
    case 19:
        if (c.skill[10] >= 0xa && c.attr[4] >= 0x14 && (c.last == 7 || c.last == 9
            || c.last == 0x12 || c.last == 0x13))
            return true;
        return false;
    case 20:
        if ((c.taken[34] & 1) && ((c.taken[4] & 1) || (c.taken[2] & 1) || (c.taken[3] & 1)
            || (c.taken[5] & 1) || (c.taken[7] & 1) || (c.taken[8] & 1) || (c.taken[15] & 1)
            || (c.taken[9] & 1) || (c.taken[19] & 1) || (c.taken[24] & 1) || (c.taken[29] & 1)
            || (c.taken[26] & 1) || (c.taken[20] & 1)))
            return true;
        return false;
    case 21:
        if (c.age < 0x14 || (c.skill[12] >= 6 && c.attr[4] >= 0xc && (c.last == 0
            || c.last == 2 || c.last == 1 || c.last == 0xa || c.last == 0xb || c.last == 0xc
            || c.last == 0xd || c.last == 0xe || c.last == 0x10 || c.last == 0x11
            || c.last == 0x12 || c.last == 0x15 || c.last == 0x1c || c.last == 0x1b
            || c.last == 4 || c.last == 0x20 || c.last == 0x21)))
            return true;
        return false;
    case 22:
        if ((c.age < 0x14 && c.child == 1) || (c.skill[12] >= 0xf && c.attr[4] >= 0xc
            && ((c.taken[4] & 1) || (c.taken[6] & 1) || (c.taken[3] & 1) || (c.taken[5] & 1)
            || (c.taken[20] & 1) || (c.taken[8] & 1) || (c.taken[15] & 1) || (c.taken[9] & 1)
            || (c.taken[19] & 1) || (c.taken[24] & 1) || (c.taken[21] & 1) || (c.taken[22] & 1)
            || (c.taken[25] & 1) || (c.taken[26] & 1) || ((c.prev == 0xc || c.prev == 0xe
            || c.prev == 0x12) && (c.last == 0xc || c.last == 0xe || c.last == 0x12)))))
            return true;
        return false;
    case 23:
        if (c.skill[13] >= 0xf && ((c.taken[21] & 1) || (c.taken[22] & 1) || (c.taken[24] & 1)
            || (c.taken[25] & 1) || (c.taken[26] & 1) || (c.taken[23] & 1)))
            return true;
        return false;
    case 24:
        if (c.skill[12] >= 0x14 && ((c.taken[9] & 1) || (c.taken[22] & 1) || (c.taken[23] & 1)
            || (c.taken[25] & 1) || (c.taken[26] & 1) || (c.taken[15] & 1) || (c.taken[24] & 1)))
            return true;
        return false;
    case 25:
        if (c.attr[4] >= 0x1e && ((c.taken[8] & 1) || (c.taken[9] & 1) || (c.taken[15] & 1)
            || (c.taken[21] & 1) || (c.taken[24] & 1) || (c.taken[22] & 1) || (c.taken[25] & 1)
            || (c.taken[26] & 1) || ((c.prev == 0xc || c.prev == 0xe || c.prev == 0xa
            || c.prev == 0x17) && (c.last == 0xc || c.last == 0xe || c.last == 0xa
            || c.last == 0x17))))
            return true;
        return false;
    case 26:
        if (c.attr[4] >= 0x23 && (c.taken[25] & 1))
            return true;
        return false;
    case 27:
        if ((c.age < 0x14 && c.child != 0) || (c.age >= 0x14 && !(c.taken[27] & 1)
            && !(c.taken[7] & 1) && !(c.taken[9] & 1) && !(c.taken[15] & 1)
            && !(c.taken[19] & 1) && !(c.taken[26] & 1) && !(c.taken[28] & 1)
            && !(c.taken[29] & 1) && c.last != 3 && c.last != 4 && c.last != 5 && c.last != 6
            && c.last != 8 && c.last != 0x14 && c.last != 0x16 && c.last != 0x12
            && c.last != 0x17 && c.last != 0x18 && c.last != 0x19))
            return true;
        return false;
    case 28:
        if ((c.age < 0x14 && (c.child == 2 || c.child == 3)) || (c.taken[23] & 1)
            || (c.taken[25] & 1) || (c.taken[26] & 1) || (c.taken[28] & 1) || (c.taken[29] & 1)
            || (c.taken[27] & 1))
            return true;
        return false;
    case 29:
        if (c.attr[4] >= 0xc && ((c.taken[19] & 1) || (c.taken[26] & 1) || (c.taken[28] & 1)))
            return true;
        return false;
    case 30:
        if ((c.age < 0x14 && c.child != 0) || (c.age >= 0x14 && !(c.taken[7] & 1)
            && !(c.taken[9] & 1) && !(c.taken[15] & 1) && !(c.taken[19] & 1)
            && !(c.taken[26] & 1) && !(c.taken[29] & 1) && c.last != 3 && c.last != 4
            && c.last != 5 && c.last != 6 && c.last != 8 && c.last != 0x14 && c.last != 0x18
            && c.last != 0x12 && c.last != 0x19))
            return true;
        return false;
    case 31:
        if ((c.age < 0x14 && c.child != 0) || (c.age >= 0x14 && !(c.taken[7] & 1)
            && !(c.taken[9] & 1) && !(c.taken[15] & 1) && !(c.taken[19] & 1)
            && !(c.taken[26] & 1) && !(c.taken[28] & 1) && !(c.taken[29] & 1) && c.last != 3
            && c.last != 4 && c.last != 5 && c.last != 6 && c.last != 8 && c.last != 0x14
            && c.last != 0x18 && c.last != 0x12 && c.last != 0x19))
            return true;
        return false;
    case 32:
        if ((c.age < 0x14 && (c.child == 0 || c.child == 1 || c.child == 2))
            || (c.attr[4] >= 0x19 && c.skill[16] >= 0xf && ((c.taken[1] & 1)
            || (c.taken[2] & 1) || (c.taken[3] & 1) || (c.taken[4] & 1) || (c.taken[8] & 1)
            || (c.taken[10] & 1) || (c.taken[14] & 1) || (c.taken[16] & 1) || (c.taken[17] & 1)
            || (c.taken[18] & 1) || (c.taken[21] & 1) || (c.taken[22] & 1) || (c.taken[24] & 1)
            || (c.taken[25] & 1) || (c.taken[28] & 1) || (c.taken[30] & 1) || (c.taken[33] & 1)
            || (c.taken[31] & 1))))
            return true;
        return false;
    case 33:
        if ((c.age < 0x14 && (c.child != 3 || c.child != 5)) || (c.skill[16] >= 0xa
            && ((c.taken[1] & 1) || (c.taken[2] & 1) || (c.taken[8] & 1) || (c.taken[10] & 1)
            || (c.taken[35] & 1) || (c.taken[16] & 1) || (c.taken[17] & 1) || (c.taken[18] & 1)
            || (c.taken[21] & 1) || (c.taken[22] & 1) || (c.taken[24] & 1) || (c.taken[25] & 1)
            || (c.taken[28] & 1) || (c.taken[30] & 1) || (c.taken[33] & 1) || (c.taken[31] & 1)
            || (c.taken[36] & 1))))
            return true;
        return false;
    case 34:
        if ((c.age < 0x14 && (c.child != 0 || c.child != 1)) || (!(c.taken[7] & 1)
            && !(c.taken[9] & 1) && !(c.taken[15] & 1) && !(c.taken[19] & 1)
            && !(c.taken[26] & 1) && !(c.taken[29] & 1) && c.last != 3 && c.last != 4
            && c.last != 5 && c.last != 6 && c.last != 8 && c.last != 0x1c && c.last != 0x18
            && c.last != 0x12 && c.last != 0x19))
            return true;
        return false;
    case 35:
        if ((c.age < 0x14 && (c.child == 0 || c.child == 3 || c.child == 5))
            || (c.skill[18] >= 0xf && (c.last != 7 || c.last != 0xf || c.last != 0x1a
            || c.last != 0x13 || c.last != 0x1d) && ((c.taken[0] & 1) || (c.taken[1] & 1)
            || (c.taken[2] & 1) || (c.taken[3] & 1) || (c.taken[5] & 1) || (c.taken[10] & 1)
            || (c.taken[16] & 1) || (c.taken[11] & 1) || (c.taken[20] & 1) || (c.taken[18] & 1)
            || (c.taken[34] & 1) || (c.taken[35] & 1) || (c.taken[36] & 1))))
            return true;
        return false;
    case 36:
        if (c.age < 0x14 || (c.taken[0] & 1) || (c.taken[1] & 1) || (c.taken[2] & 1)
            || (c.taken[3] & 1) || (c.taken[5] & 1) || (c.taken[33] & 1) || (c.taken[36] & 1)
            || (c.taken[35] & 1) || c.last == 0xa || c.last == 0xb || c.last == 0x14
            || c.last == 0x10 || c.last == 0x1e || c.last == 0x1f || c.last == 0x20
            || c.last == 0x22)
            return true;
        return false;
    }
    return false;
}

std::vector<int> cg_prof_offered(const CgCtx &c)
{
    std::vector<int> v;
    for (int i = 0; i < 37; i++) { if (cg_prof_rule(i, c)) v.push_back(i); if (v.size() > 14) break; }
    return v;
}

const char *cg_prof_needs(int p)
{
    /* (in words, from cg_prof_rule; "first" is the first profession, straight after the childhood;
       "once" anywhere before, "just now" the one just done) */
    static const char *N[37] = {
        /* Recruit */ "Never a Recruit before, and younger than 30.",
        /* Soldier */ "Once a Recruit, Soldier, Veteran, Captain, Knight, Schulz or Bandit.",
        /* Veteran */ "Once a Soldier, Veteran, Captain or Knight.",
        /* Captain */ "{Intelligence:20} {Perception:20} {Charisma:20}; once a Veteran, Captain, Knight, Courtier, Manorial Lord, Bishop or Schulz.",
        /* Noble Heir */ "A childhood in the Nobility, or once a Noble Heir or Courtier.",
        /* Knight */ "{Virtue:16}; and a first profession after a childhood in the Nobility, or once a Knight or Manorial Lord, or just now a Noble Heir, Courtier, Captain, Bishop or Abbot/Abbess.",
        /* Courtier */ "A first profession after a childhood in the Nobility or a Wealthy Urban Family; or once a Courtier or Manorial Lord; or just now a Knight, Captain, Priest, Bishop, Abbot/Abbess or Schulz.",
        /* Manorial Lord */ "Just now a Manorial Lord, or the last two professions both a Noble Heir, Courtier, Bishop or Abbot/Abbess.",
        /* Priest */ "A man with {Intelligence:20} {Perception:20} {Charisma:20}; once a Manorial Lord, Priest, Bishop or Abbot, or just now a Noble Heir, Courtier, Monk, Schulz, Clerk or Professor, or the last two both an Oblate, Novice Monk, Friar or Student.",
        /* Bishop */ "A man with {Intelligence:25} {Perception:25} {Charisma:25}; just now a Bishop or Abbot, or the last two both a Courtier, Manorial Lord or Priest.",
        /* Friar */ "A man; just now a Priest, Friar, Hermit, Novice Monk, Monk or Abbot.",
        /* Hermit */ "{Virtue:15}.",
        /* Oblate */ "A first profession for any but a commoners' child; otherwise {Intelligence:12}, and never a Novice, Monk/Nun, Abbot/Abbess, Priest, Bishop, Friar, Clerk, Physician, Professor or Alchemist.",
        /* Novice */ "A first profession; or never a Noble Heir, Courtier, Manorial Lord, Priest, Bishop, Friar, Novice, Monk/Nun, Abbot/Abbess, Clerk, Physician, Professor or Alchemist.",
        /* Monk/Nun */ "{Intelligence:15} {Perception:15} {Charisma:15}, {Religion:5}; once a Noble Heir, Courtier, Manorial Lord, Priest, Bishop, Friar, Novice, Monk/Nun, Abbot/Abbess, Student, Clerk, Physician, Professor or Alchemist.",
        /* Abbot */ "{Intelligence:20} {Perception:20} {Charisma:20}, {Religion:15}; just now a Noble Heir, Courtier, Manorial Lord, Priest, Bishop or Abbot/Abbess, or the last two both a Monk/Nun or Professor.",
        /* Peddler */ "Not just after being a Noble Heir, Courtier, Captain, Knight, Priest, Travelling Merchant, Professor, Alchemist or Journeyman, and never a Manorial Lord, Bishop, Abbot/Abbess, Merchant-Proprietor, Schulz, Master Alchemist or Master Craftsman.",
        /* Local Trader */ "A first profession for any but a commoners' child; otherwise {Speak Common:5}, {Intelligence:12}, and once a Captain, Noble Heir, Courtier, Priest, Bishop, Monk/Nun, Abbot/Abbess, Peddler, Local Trader, Travelling Merchant, Clerk, Physician, Professor, Alchemist, Journeyman, Master Craftsman or Swindler.",
        /* Travelling Merchant */ "A first profession after a childhood in the Nobility or a Wealthy Urban Family; otherwise {Speak Common:10}, {Intelligence:15}, and once a Noble Heir, Manorial Lord, Local Trader, Travelling Merchant, Merchant-Proprietor, Professor, Master Alchemist or Master Craftsman.",
        /* Merchant-Proprietor */ "{Speak Common:10}, {Intelligence:20}; just now a Manorial Lord, Bishop, Travelling Merchant or Merchant-Proprietor.",
        /* Schulz */ "Once a Peasant, and once a Veteran, Captain, Knight, Noble Heir, Manorial Lord, Priest, Bishop, Abbot/Abbess, Merchant-Proprietor, Schulz, Professor, Master Alchemist or Master Craftsman.",
        /* Student */ "A first profession; or {Read & Write:6}, {Intelligence:12}, and just now a Recruit, Soldier, Veteran, Noble Heir, Friar, Hermit, Oblate, Novice, Monk/Nun, Peddler, Local Trader, Travelling Merchant, Student, Apprentice, Journeyman, Swindler or Thief.",
        /* Clerk */ "A first profession after a childhood in a Wealthy Urban Family; otherwise {Read & Write:15}, {Intelligence:12}, and once a Captain, Noble Heir, Knight, Courtier, Priest, Bishop, Abbot/Abbess, Merchant-Proprietor, Schulz, Student, Clerk, Professor, Alchemist or Master Alchemist, or the last two both an Oblate, Monk/Nun or Travelling Merchant.",
        /* Physician */ "{Healing:15}; once a Student, Clerk, Physician, Professor, Alchemist or Master Alchemist.",
        /* Professor */ "{Read & Write:20}; once a Bishop, Abbot/Abbess, Clerk, Physician, Professor, Alchemist or Master Alchemist.",
        /* Alchemist */ "{Intelligence:30}; once a Priest, Bishop, Abbot/Abbess, Student, Clerk, Professor, Alchemist or Master Alchemist, or the last two both an Oblate, Monk/Nun, Friar or Physician.",
        /* Master Alchemist */ "{Intelligence:35}; once an Alchemist.",
        /* Apprentice */ "A first profession for any but a child of the Nobility; later never an Apprentice, Journeyman, Master Craftsman, Manorial Lord, Bishop, Abbot/Abbess, Merchant-Proprietor or Master Alchemist, and not just after being a Captain, Noble Heir, Knight, Courtier, Priest, Schulz, Clerk, Travelling Merchant, Physician, Professor or Alchemist.",
        /* Journeyman */ "A first profession after a childhood in a Town Trades or Country Crafts Family; or once an Apprentice, Journeyman, Master Craftsman, Physician, Alchemist or Master Alchemist.",
        /* Master Craftsman */ "{Intelligence:12}; once a Journeyman, Merchant-Proprietor or Master Alchemist.",
        /* Laborer */ "A first profession for any but a child of the Nobility; later never a Manorial Lord, Bishop, Abbot/Abbess, Merchant-Proprietor, Master Alchemist or Master Craftsman, and not just after being a Captain, Noble Heir, Knight, Courtier, Priest, Schulz, Travelling Merchant, Professor or Alchemist.",
        /* Vagabond */ "A first profession for any but a child of the Nobility; later never a Manorial Lord, Bishop, Abbot/Abbess, Merchant-Proprietor, Journeyman, Master Alchemist or Master Craftsman, and not just after being a Captain, Noble Heir, Knight, Courtier, Priest, Schulz, Travelling Merchant, Professor or Alchemist.",
        /* Swindler */ "A first profession after a childhood in the Nobility, a Wealthy Urban or a Town Trades Family; otherwise {Intelligence:25}, {Streetwise:15}, and once a Soldier, Veteran, Captain, Noble Heir, Priest, Friar, Monk/Nun, Peddler, Local Trader, Travelling Merchant, Student, Clerk, Professor, Alchemist, Journeyman, Laborer, Vagabond or Thief.",
        /* Thief */ "A first profession; or {Streetwise:10} and once a Soldier, Veteran, Priest, Friar, Peddler, Local Trader, Travelling Merchant, Student, Clerk, Professor, Alchemist, Journeyman, Laborer, Vagabond, Thief, Hunter or Bandit.",
        /* Peasant */ "A first profession; or never a Manorial Lord, Bishop, Abbot/Abbess, Merchant-Proprietor, Master Alchemist or Master Craftsman, and not just after being a Captain, Noble Heir, Knight, Courtier, Priest, Travelling Merchant, Professor, Alchemist or Journeyman.",
        /* Hunter */ "A first profession after a childhood in the Nobility, a Country Crafts Family or Rural Commoners; otherwise {Woodwise:15} and once a Recruit, Soldier, Veteran, Captain, Knight, Friar, Hermit, Peddler, Travelling Merchant, Schulz, Peasant, Hunter or Bandit.",
        /* Bandit */ "A first profession; or once a Recruit, Soldier, Veteran, Captain, Knight, Thief, Hunter or Bandit, or just now a Friar, Hermit, Peddler, Schulz, Laborer, Vagabond, Swindler or Peasant.",
    };
    return p >= 0 && p < 37 ? N[p] : "";
}

std::vector<int> cg_prof_leads_to(int p)
{
    std::vector<int> v;
    if (p < 0 || p >= 37) return v;
    CgCtx a; a.age = 20;                                      /* (a later profession, every attribute and skill high: only the past counts) */
    for (int k = 0; k < 6; k++) a.attr[k] = 99;
    for (int k = 0; k < 19; k++) a.skill[k] = 99;
    CgCtx b = a; b.taken[p] = 1; b.last = b.prev = p;          /* (p once, and the last two terms) */
    for (int q = 0; q < 37; q++) if (q != p && cg_prof_rule(q, b) && !cg_prof_rule(q, a)) v.push_back(q);
    return v;
}

static std::string short_of(int p, const CgCtx &c, const CgCtx &past);

std::string cg_prof_short(int p, const CgCtx &c) { return short_of(p, c, c); }

std::string cg_prof_why(int p, const CgCtx &c)
{
    if (cg_prof_rule(p, c)) return "";
    CgCtx hi = c;
    for (int k = 0; k < 6; k++) hi.attr[k] = 99;
    for (int k = 0; k < 19; k++) hi.skill[k] = 99;
    std::string r;
    CgCtx m = hi; m.female = 0;
    if (c.female && !cg_prof_rule(p, hi) && cg_prof_rule(p, m)) { r = "Only for a man"; hi.female = 0; }
    CgCtx past = c; past.female = hi.female;
    if (!cg_prof_rule(p, hi)) {                               /* the past is not right: some past would be */
        int q = -1;
        for (int k = 0; k < 37 && q < 0; k++) { CgCtx t = hi; t.taken[k] = 1; t.last = t.prev = k; if (cg_prof_rule(p, t)) q = k; }
        r += r.empty() ? "Not with this character's past" : ", and not with this past";
        if (q < 0) return r + ".";
        past.taken[q] = 1; past.last = past.prev = q;
    }
    std::string s = short_of(p, c, past);
    if (!s.empty()) r += (r.empty() ? "Short of " : "; also short of ") + s;
    return r + ".";
}

/* the attributes and skills c lacks for p, its past taken as `past` */
static std::string short_of(int p, const CgCtx &c, const CgCtx &past)
{
    static const struct { bool skill; int k; const char *name; } S[] = {
        { false, 1, "Strength" }, { false, 3, "Perception" }, { false, 4, "Intelligence" }, { false, 5, "Charisma" },
        { true, 8, "Religion" }, { true, 9, "Virtue" }, { true, 10, "Speak Common" }, { true, 12, "Read & Write" },
        { true, 13, "Healing" }, { true, 16, "Streetwise" }, { true, 18, "Woodwise" } };
    const int NS = (int)(sizeof S / sizeof S[0]);
    auto val = [&](const CgCtx &x, int i) -> signed char { return S[i].skill ? x.skill[S[i].k] : x.attr[S[i].k]; };
    auto set = [&](CgCtx &x, int i, int v) { if (S[i].skill) x.skill[S[i].k] = (signed char)v; else x.attr[S[i].k] = (signed char)v; };
    CgCtx hi = past;
    for (int i = 0; i < NS; i++) if (val(hi, i) < 99) set(hi, i, 99);
    if (!cg_prof_rule(p, hi)) return "";                      /* (the past, or being a woman, decides) */
    std::string r;
    for (int i = 0; i < NS; i++) {
        CgCtx t = hi; set(t, i, val(c, i));
        if (cg_prof_rule(p, t)) continue;                     /* (enough of this one already) */
        int need = val(c, i) + 1;
        while (need < 99) { set(t, i, need); if (cg_prof_rule(p, t)) break; need++; }
        char b[64]; snprintf(b, sizeof b, "%s%s %d (has %d)", r.empty() ? "" : ", ", S[i].name, need, (int)val(c, i));
        r += b;
    }
    return r;
}
