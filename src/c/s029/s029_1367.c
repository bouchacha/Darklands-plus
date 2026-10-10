/* place: s029 1367:000a */
/* orig: size=0f9e CONST=7e34+000a _DATA=2776+007e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: FILE MATCH (e3) */
/* Strategic core: rule helpers (s029 1367).
   - clamps (0..99, 0..100, -100..100, lo..hi), max, min
   - money: struct money {florins, groschen, pfennigs}; 1 florin = 20 groschen,
     1 groschen = 12 pfennigs. normalise, to-pfennigs, add, subtract, multiply,
     compare, format ("3 florins, 2 groschen and 1 pfennig"), party-wide costs
   - calendar: the current time is D_00e0 hour, D_00e2 day, D_00e4 month (0-11),
     D_00e6 year; D_2776 is days per month. Advance by hours/days, wait until an hour,
     daylight test, time-of-day bands, absolute day numbers and day differences
   - "Chance of success is X% to Y%" text, random personal names. */
#include "darklands/card.h"
#include "darklands/game.h"

struct money { int fl; int gr; int pf; };
struct date { int hour; int day; int month; int year; };
struct member { char status; char _p01[0x7f]; };

int days_per_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

extern int cur_hour;              /* hour 0-23 */
extern int cur_day;              /* day of month 0-based */
extern int cur_month;              /* month 0-11 */
extern int cur_year;              /* year */
extern int side_win;
extern int party_seed;
extern int party_size;
extern char game_difficulty;             /* 0 numbers, 1 words, 2 nothing */
extern char *quality_words[];
extern struct member party[];
extern char far var_named[][0x1e];
extern char far * far bell_names[];
extern char far * far F_290e_235f[];
extern char far * far F_290e_267f[];
extern char far * far F_290e_2517[];
extern char far * far F_290e_28cb[];

char far *_fstrcat(char far *, const char far *);
char far *_fstrcpy(char far *, const char far *);
int getch(void);
void srand(unsigned);
long time(long *);
char *itoa(int, char *, int);
char *strcat(char *, const char *);
int far random_n(int);
int far quit_to_dos();
int far draw_text();
int far itoa_commas();
int far hours_pass();
int far sub_s029_0e76_281c();
int far sub_s029_0e76_3f30();
int far pass_hours_hourglass_vec();

/* clamp to 0..99 */
int far clamp99(int v)
{
    int r;

    r = v;
    if (r > 99)
        r = 99;
    else if (v < 0)
        r = 0;
    return r;
}

/* clamp to 0..100 */
int far clamp100(int v)
{
    int r;

    r = v;
    if (r > 100)
        r = 100;
    else if (v < 0)
        r = 0;
    return r;
}

/* clamp to -100..100 */
int far clamp_pm100(int v)
{
    int r;

    r = v;
    if (r > 100)
        r = 100;
    else if (v < -100)
        r = -100;
    return r;
}

/* clamp v to lo..hi */
int far clamp_range(int lo, int hi, int v)
{
    int r;

    r = hi;
    if (r < v)
        return hi;
    r = lo;
    if (r > v)
        return lo;
    return v;
}

/* max */
int far imax(int a, int b)
{
    return b < a ? a : b;
}

/* min */
int far imin(int a, int b)
{
    return b > a ? a : b;
}

/* set money from florins, groschen, pfennigs, carrying overflow upwards */
#pragma optimize("a", on)
void far money_set(struct money *m, int fl, unsigned gr, unsigned pf)
{
    if (pf >= 12) {
        gr += pf / 12;
        pf %= 12;
    }
    m->pf = pf;
    if (gr >= 20) {
        fl += gr / 20;
        gr %= 20;
    }
    m->gr = gr;
    m->fl = fl;
}
#pragma optimize("", on)

/* money value in pfennigs */
long far money_pfennigs(struct money *m)
{
    return ((long)m->fl * 20 + m->gr) * 12 + m->pf;
}

/* add florins, groschen, pfennigs to money */
void far money_add_fgp(struct money *m, int fl, int gr, int pf)
{
    m->pf += pf;
    if (m->pf >= 12) {
        m->gr += m->pf / 12;
        m->pf %= 12;
    }
    m->gr += gr;
    if (m->gr >= 20) {
        m->fl += (unsigned)m->gr / 20;
        m->gr %= 20;
    }
    m->fl += fl;
}

/* subtract florins, groschen, pfennigs from money, borrowing */
#pragma optimize("a", on)
void far money_sub_fgp(struct money *m, int fl, int gr, int pf)
{
    unsigned q;

    if ((m->pf -= pf) < 0) {
        q = (unsigned)(11 - m->pf) / 12;
        m->gr -= q;
        m->pf += q * 12;
    }
    if ((m->gr -= gr) < 0) {
        q = (unsigned)(19 - m->gr) / 20;
        m->fl -= q;
        m->gr += q * 20;
    }
    m->fl -= fl;
}
#pragma optimize("", on)

/* copy money */
void far money_copy(struct money *d, struct money *s)
{
    d->pf = s->pf;
    d->gr = s->gr;
    d->fl = s->fl;
}

/* a += b */
void far money_add(struct money *a, struct money *b)
{
    a->pf += b->pf;
    if (a->pf >= 12) {
        a->gr += a->pf / 12;
        a->pf %= 12;
    }
    a->gr += b->gr;
    if (a->gr >= 20) {
        a->fl += a->gr / 20;
        a->gr %= 20;
    }
    a->fl += b->fl;
}

/* a -= b */
#pragma optimize("a", on)
void far money_sub(struct money *a, struct money *b)
{
    unsigned q;

    if ((a->pf -= b->pf) < 0) {
        q = (unsigned)(11 - a->pf) / 12;
        a->gr -= q;
        a->pf += q * 12;
    }
    if ((a->gr -= b->gr) < 0) {
        q = (unsigned)(19 - a->gr) / 20;
        a->fl -= q;
        a->gr += q * 20;
    }
    a->fl -= b->fl;
}
#pragma optimize("", on)

/* d = unit * n */
void far money_mul(struct money *d, struct money *unit, int n)
{
    struct money t;
    int i;

    t.fl = 0;
    t.gr = 0;
    t.pf = 0;
    for (i = 1; i <= n; i++)
        money_add(&t, unit);
    money_copy(d, &t);
}

/* compare: 1 if a > b, -1 if a < b, 0 if equal */
#pragma optimize("a", on)
int far money_cmp(struct money *a, struct money *b)
{
    int r;
    unsigned long ta;
    unsigned long tb;

    r = 0;
    ta = ((long)a->fl * 20 + a->gr) * 12 + a->pf;
    tb = ((long)b->fl * 20 + b->gr) * 12 + b->pf;
    if (tb < ta)
        r = 1;
    else if (tb > ta)
        r = -1;
    return r;
}
#pragma optimize("", on)

/* format money as text: "3 florins, 2 groschen and 1 pfennig" */
void far money_format(struct money *m, char *buf)
{
    int fl;
    int gr;
    char num[16];

    fl = 0;
    gr = 0;
    *buf = 0;
    if (m->fl > 0) {
        if (m->fl == 1)
            strcat(buf, "1 florin");
        else {
            itoa_commas(m->fl, num);
            strcat(buf, num);
            strcat(buf, " florins");
        }
        fl = 1;
    }
    if (m->gr > 0) {
        if (fl == 1)
            if (m->pf)
                strcat(buf, ", ");
            else
                strcat(buf, " and ");
        itoa_commas(m->gr, num);
        strcat(buf, num);
        strcat(buf, " groschen");
        gr = 1;
    }
    if (m->pf > 0) {
        if (fl == 1 || gr == 1)
            strcat(buf, " and ");
        if (m->pf == 1)
            strcat(buf, "1 pfennig");
        else {
            itoa_commas(m->pf, num);
            strcat(buf, num);
            strcat(buf, " pfennigs");
        }
    }
}

/* total += pf pfennigs for every active party member */
void far money_per_member_pf(struct money *total, int pf)
{
    struct money t;
    int i;

    money_set(&t, 0, 0, pf);
    for (i = 1; i <= party_size; i++)
        if (party[i].status == 1)
            money_add(total, &t);
}

/* total += cost for every active party member */
void far money_per_member(struct money *total, struct money *cost)
{
    int i;

    for (i = 1; i <= party_size; i++)
        if (party[i].status == 1)
            money_add(total, cost);
}

void far do_nothing(void)
{
}

/* advance the clock by n hours (calls the per-hour and per-month updates) */
void far advance_hours(int n)
{
    volatile int dm;

    hours_pass(n);
    if (n > 2)
        pass_hours_hourglass_vec(n);
    cur_hour += n;
    while (*(volatile int *)&cur_hour > 23) {
        cur_day++;
        *(volatile int *)&cur_hour -= 24;
    }
    if ((dm = days_per_month[cur_month % 12]) < cur_day)
        do {
            cur_day -= dm;
            cur_month++;
            sub_s029_0e76_281c(cur_month);
        } while ((dm = days_per_month[cur_month % 12]) < cur_day);
    while (*(volatile int *)&cur_month > 11) {
        cur_year++;
        *(volatile int *)&cur_month -= 12;
    }
    sub_s029_0e76_3f30();
    srand((unsigned)time(0));
}

/* advance the clock by n days */
void far advance_days(int n)
{
    volatile int dm;

    cur_day += n;
    hours_pass(n * 24);
    if ((dm = days_per_month[cur_month % 12]) < cur_day)
        do {
            cur_day -= dm;
            cur_month++;
            sub_s029_0e76_281c(cur_month);
        } while ((dm = days_per_month[cur_month % 12]) < cur_day);
    while (*(volatile int *)&cur_month > 11) {
        cur_year++;
        *(volatile int *)&cur_month -= 12;
    }
    sub_s029_0e76_3f30();
    srand((unsigned)time(0));
}

int far hours_until(int hour);

/* wait until the given hour */
void far wait_until_hour(int hour)
{
    advance_hours(hours_until(hour));
}

/* daylight: 5:00 to 18:59 */
int far is_daylight(void)
{
    if (cur_hour >= 5 && cur_hour < 19)
        return 1;
    return 0;
}

/* current hour */
int far get_hour(void)
{
    return cur_hour;
}

/* time-of-day band of an hour: 0-2 7 (night), 3-5 0, 6-8 1, ... 18-20 5, 21-23 6 */
int far hour_band(int h)
{
    if (h < 3)
        return 7;
    if (h < 6)
        return 0;
    if (h < 9)
        return 1;
    if (h < 12)
        return 2;
    if (h < 15)
        return 3;
    if (h < 18)
        return 4;
    if (h < 21)
        return 5;
    return 6;
}

/* time-of-day band of the current hour */
int far cur_band(void)
{
    if (cur_hour < 3)
        return 7;
    if (cur_hour < 6)
        return 0;
    if (cur_hour < 9)
        return 1;
    if (cur_hour < 12)
        return 2;
    if (cur_hour < 15)
        return 3;
    if (cur_hour < 18)
        return 4;
    if (cur_hour < 21)
        return 5;
    return 6;
}

/* the same band numbered 1..7, 0 for 21-23 */
int far cur_band1(void)
{
    if (cur_hour < 3)
        return 1;
    if (cur_hour < 6)
        return 2;
    if (cur_hour < 9)
        return 3;
    if (cur_hour < 12)
        return 4;
    if (cur_hour < 15)
        return 5;
    if (cur_hour < 18)
        return 6;
    if (cur_hour < 21)
        return 7;
    return 0;
}

/* copy name i of table 20c3 */
char far *far bell_name(int i, char far *dst)
{
    return _fstrcpy(dst, bell_names[i]);
}

/* hours from now until the given hour (0..23) */
int far hours_until(int hour)
{
    if (cur_hour <= hour)
        return hour - cur_hour;
    return hour - cur_hour + 24;
}

/* copy date */
void far date_copy(struct date *d, struct date *s)
{
    d->hour = s->hour;
    d->day = s->day;
    d->month = s->month;
    d->year = s->year;
}

/* set and normalise a date */
#pragma optimize("a", on)
void far date_set(struct date *d, int year, int month, int day, int hour)
{
    volatile int dm;

    d->day = day;
    d->month = month;
    d->year = year;
    d->hour = hour;
    if (d->hour > 23) {
        d->day += (unsigned)d->hour / 24;
        d->hour -= (unsigned)d->hour / 24 * 24;
    }
    if ((dm = days_per_month[d->month % 12]) < d->day)
        do {
            d->day -= dm;
            d->month++;
        } while ((dm = days_per_month[d->month % 12]) < d->day);
    if (d->month > 11) {
        d->year += (unsigned)d->month / 12;
        d->month -= (unsigned)d->month / 12 * 12;
    }
}
#pragma optimize("", on)

/* month (0-11) of a day of the year */
int far month_of_yday(int d)
{
    int m;
    int dm;

    m = 0;
    while (d >= 365)
        d -= 365;
    if ((*(volatile int *)&dm = days_per_month[m]) < d)
        do {
            d -= dm;
            if (++m > 11)
                m = 0;
        } while ((*(volatile int *)&dm = days_per_month[m]) < d);
    return m;
}

/* day of month of a day of the year */
int far mday_of_yday(int d)
{
    int m;
    int dm;

    m = 0;
    while (d >= 365)
        d -= 365;
    if ((*(volatile int *)&dm = days_per_month[m]) < d)
        do {
            d -= dm;
            if (++m > 11)
                m = 0;
        } while ((*(volatile int *)&dm = days_per_month[m]) < d);
    return d;
}

/* current day of the year */
int far yday_now(void)
{
    int i;
    int s;

    s = 0;
    for (i = 0; i < cur_month; i++)
        s += days_per_month[i];
    return s + cur_day;
}

/* add hours to a (far) date and normalise */
#pragma optimize("a", on)
void far date_add_hours(struct date far *d, int hours)
{
    volatile int dm;

    d->hour += hours;
    while (d->hour > 23) {
        d->day++;
        d->hour -= 24;
    }
    if ((dm = days_per_month[d->month % 12]) < d->day)
        do {
            d->day -= dm;
            d->month++;
        } while ((dm = days_per_month[d->month % 12]) < d->day);
    if (d->month > 11) {
        d->year += (unsigned)d->month / 12;
        d->month -= (unsigned)d->month / 12 * 12;
    }
}
#pragma optimize("", on)

/* absolute day number of now */
int far day_number_now(void)
{
    unsigned i;
    int s;

    s = cur_year * 365 + 0x33e8;
    for (i = 0; i < cur_month; i++)
        s += days_per_month[i];
    return s + cur_day;
}

/* days since a date (0 if in the future) */
int far days_since(struct date far *d)
{
    unsigned i;
    int now;
    int then;

    now = cur_year * 365 + 0x33e8;
    for (i = 0; i < cur_month; i++)
        now += days_per_month[i];
    now += cur_day;
    then = d->year * 365 + 0x33e8;
    for (i = 0; i < d->month; i++)
        then += days_per_month[i];
    then += d->day;
    if (now - then < 0)
        return 0;
    return now - then;
}

/* days until a date (0 if past) */
int far days_until(struct date far *d)
{
    unsigned i;
    int now;
    int then;

    now = cur_year * 365 + 0x33e8;
    for (i = 0; i < cur_month; i++)
        now += days_per_month[i];
    now += cur_day;
    then = d->year * 365 + 0x33e8;
    for (i = 0; i < d->month; i++)
        then += days_per_month[i];
    then += d->day;
    if (then - now < 0)
        return 0;
    return then - now;
}

/* days from date a to date b, limited to 0..32000 */
int far days_between(struct date *a, struct date *b)
{
    unsigned i;
    unsigned ta;
    unsigned tb;

    ta = b->year * 365 + 0x33e8;
    for (i = 0; i < b->month; i++)
        ta += days_per_month[i];
    ta += b->day;
    tb = a->year * 365 + 0x33e8;
    for (i = 0; i < a->month; i++)
        tb += days_per_month[i];
    tb += a->day;
    if ((int)(tb - ta) < 0)
        return 0;
    if ((long)tb - (long)ta > 32000L)
        return 32000;
    return tb - ta;
}

/* show a message box and wait for a key */
void far message_box(char *msg)
{
    draw_text(side_win, 10, 10, msg);
    getch();
    quit_to_dos();
}

/* append "Chance of success is X% to Y%" (or the word form) */
void far chance_text(char *buf, int lo, int hi)
{
    char num[20];

    if (game_difficulty == 2)
        return;
    strcat(buf, "Chance of success is ");
    if (game_difficulty == 0) {
        itoa(lo, num, 10);
        strcat(buf, num);
        strcat(buf, "%");
    } else
        strcat(buf, quality_words[lo / 20]);
    strcat(buf, " to ");
    if (hi > 100)
        hi = 100;
    if (game_difficulty == 0) {
        itoa(hi, num, 10);
        strcat(buf, num);
        strcat(buf, "%");
    } else
        strcat(buf, quality_words[hi / 20]);
}

/* random personal name into name slot idx; kind 0-5 picks first-name table and
   surname/byname table; seeded from party_seed + seed so it is reproducible */
void far random_name(int seed, unsigned kind, int idx)
{
    srand(party_seed + seed);
    random_n(1000);
    switch (kind) {
    case 0:
        _fstrcpy(var_named[idx], F_290e_235f[random_n(0x6c)]);
        _fstrcat(var_named[idx], " ");
        _fstrcat(var_named[idx], F_290e_267f[random_n(0x92)]);
        break;
    case 1:
        _fstrcpy(var_named[idx], F_290e_2517[random_n(0x58)]);
        _fstrcat(var_named[idx], " ");
        _fstrcat(var_named[idx], F_290e_267f[random_n(0x92)]);
        break;
    case 2:
        _fstrcpy(var_named[idx], F_290e_235f[random_n(0x6c)]);
        _fstrcat(var_named[idx], " ");
        _fstrcat(var_named[idx], F_290e_28cb[random_n(0x10)]);
        break;
    case 3:
        _fstrcpy(var_named[idx], F_290e_2517[random_n(0x58)]);
        _fstrcat(var_named[idx], " ");
        _fstrcat(var_named[idx], F_290e_28cb[random_n(0x10)]);
        break;
    case 4:
        _fstrcpy(var_named[idx], F_290e_235f[random_n(0x6c)]);
        break;
    case 5:
        _fstrcpy(var_named[idx], F_290e_2517[random_n(0x58)]);
        break;
    }
    srand((unsigned)time(0));
}
