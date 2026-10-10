/* place: s014 1d47:0000 */
/* orig: size=022c */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* s014 1d47: missile flight (arrows, bolts, thrown items, spell missiles).
   0000 advances every active missile record one combat tick along a Bresenham line
   (013e) towards its target; on arrival it either hits the target combatant or
   triggers the impact effect (vec_1dc9).
   status: 2/2 written, 0 match. m4: sprite is unsigned char (jae); 0000 is left with the two
   'cmp [si+18h],al' compares reversed (decl positions/types/source order tried). */
#include "darklands/card.h"
#include "darklands/game.h"

struct cbt {                    /* combatant record, 0x80 bytes, combatants[13] */
    char pad0[4];
    int x;                      /* +04 */
    int y;                      /* +06 */
    char pad8[0xc];
    char status;                /* +14 1 active, 2/3 = ? (hit reactions) */
    char pad15[0x6b];
};

struct msl {                    /* missile / effect record, 0x1a bytes, missile_recs[16], count missile_count */
    char pad0[8];
    int x;                      /* +08 current position */
    int y;                      /* +0a */
    int tx;                     /* +0c target position */
    int ty;                     /* +0e */
    char target;                /* +10 target combatant (-1 = fixed spot) */
    char done;                  /* +11 0 = flying, bit 7 = arrived/finished */
    char facing;                /* +12 */
    unsigned char shooter;      /* +13 low nibble = shooter index (15 = none) */
    unsigned char sprite;       /* +14 bits 0..6: < 6 = physical missile, else spell effect */
    unsigned char f15;          /* +15 effect parameter */
    unsigned char f16;          /* +16 effect parameter */
    unsigned char speed;        /* +17 squares per tick */
    char level;                 /* +18 floor level of the missile */
    char pad19;
};

void far damage_apply(int);
void far party_panel_rows(int);
void far cbt_cancel_action(int);
void far effect_spell_vec(int, int, int, int, int, int, int);
void far sub_s014_1d47_013e(int *, int *, int *, int *, int);

extern struct cbt combatants[];
extern struct msl missile_recs[];
extern char missile_count;             /* number of missile records */
extern char cbt_floor[];           /* [13] floor level of each combatant */
extern char cbt_was_hit[];           /* [13] "was hit" flag */
extern char cbt_anim[];           /* [13] busy/animation flag */
extern char cbt_action[];           /* [13] */

/* advance all missiles by one tick */
void far missiles_advance(void)
{
    int sh;
    int i;
    int t;
    struct msl *m;

    for (i = 0; i < missile_count; i++) {
        m = &missile_recs[i];
        sh = missile_recs[i].shooter & 0xf;
        t = missile_recs[i].target;
        if (t != -1 && m->done == 0 && m->level == cbt_floor[t]) {
            /* homing: follow the target combatant */
            m->tx = combatants[t].x;
            m->ty = combatants[t].y;
        }
        sub_s014_1d47_013e(&m->x, &m->y, &m->tx, &m->ty, m->speed);
        if (m->x == m->tx && m->y == m->ty) {
            if ((m->sprite & 0x7f) < 6) {
                /* physical missile reached its target combatant */
                if (m->done == 0 && m->level == cbt_floor[t]) {
                    cbt_was_hit[t] = 1;
                    damage_apply(t);
                    if (combatants[t].status == 2 || combatants[t].status == 3) {
                        cbt_anim[sh] = 0;
                        cbt_anim[t] = 0;
                        if (sh != 15)
                            party_panel_rows(sh);
                        party_panel_rows(t);
                        cbt_action[sh] = 0;
                        cbt_cancel_action(sh);
                    }
                }
            } else {
                /* spell / area effect lands */
                effect_spell_vec(m->tx, m->ty, sh, m->f15, m->f16, m->target, m->level);
            }
            m->done |= 0x80;
        }
    }
}

/* move (*x,*y) up to `speed` steps along the Bresenham line towards (*tx,*ty) */
void far sub_s014_1d47_013e(int *x, int *y, int *tx, int *ty, int speed)
{
    int x0, y0, dx, dy, adx, ady, sx, sy, steep, err, n;

    y0 = *y;
    x0 = *x;
    dx = *tx - x0;
    adx = abs(dx);
    dy = *ty - y0;
    ady = abs(dy);
    sx = dx >= 0 ? 1 : -1;
    sy = dy >= 0 ? 1 : -1;
    if (ady > adx) {
        n = adx;
        adx = ady;
        ady = n;
        steep = 1;
    } else
        steep = 0;
    err = 2 * ady - adx;
    n = speed;
    if (n > adx)
        n = adx;
    for (; n > 0; n--) {
        while (err >= 0) {
            if (steep == 1)
                x0 += sx;
            else
                y0 += sy;
            err -= 2 * adx;
        }
        if (steep == 1)
            y0 += sy;
        else
            x0 += sx;
        err += 2 * ady;
    }
    *x = x0;
    *y = y0;
}
