/* place: s020 1ef8:0000 */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* status: wip 1/3 (05e6). 06b6 blocked by memset chain form (cport.compat600 only rewrites
 * the first memset after `sub ax,ax`); 0000: 3-byte struct copy `mov cx,1; rep movsw` (6.00). Combat level loader (overlay s020, child of s014):
 * 0000 reads level%d.trn (enemy type table, 0xd6-byte records) into D_a895 count +
 * D_a896[] (0x122-byte enemy type records), writes level%d.enm, sizes the animation buffer
 * from tacanim.db; 05e6 sets far buffer pointers; 06b6 clears combat state. */

typedef struct { char *_ptr; int _cnt; char *_base; char _flag; char _file; } FILE;
struct s3 { char b[3]; };
struct s6 { char b[6]; };
struct s19 { char b[19]; };
struct etype {
    char pad0[30];
    char f1e;   /* +1e */
    char f1f;   /* +1f */
    char pad20[2];
    char f22;   /* +22 */
    char pad23[1];
    char f24;   /* +24 */
    char pad25[35];
    char f48;   /* +48 */
    char f49;   /* +49 */
    char pad4a[1];
    char f4b[2];   /* +4b */
    char pad4d[2];
    char f4f[2];   /* +4f */
    char f51;   /* +51 */
    char pad52[3];
    char f55;   /* +55 */
    char pad56[2];
    char f58;   /* +58 */
    char pad59[1];
    char f5a;   /* +5a */
    char f5b;   /* +5b */
    char f5c;   /* +5c */
    char f5d[7];   /* +5d */
    char f64[7];   /* +64 */
    struct s19 f6b;   /* +6b */
    char f7e;   /* +7e */
    char pad7f[1];
    char name[4];   /* +80 */
    char name2[0x17];   /* +84 */
    char f9b;   /* +9b */
    unsigned char f9c;   /* +9c */
    char pad9d[1];
    int f9e;   /* +9e */
    int fa0;   /* +a0 */
    int fa2;   /* +a2 */
    int fa4;   /* +a4 */
    char fa6;   /* +a6 */
    struct s3 fa7;   /* +a7 */
    char faa;   /* +aa */
    char fab;   /* +ab */
    int fac;   /* +ac */
    char fae[10];   /* +ae */
    char fb8[10];   /* +b8 */
    struct s6 fc2[16];   /* +c2 */
};
struct etsrc {
    int f0;   /* +00 */
    int f2;   /* +02 */
    int f4;   /* +04 */
    int f6;   /* +06 */
    char f8;   /* +08 */
    struct s3 f9;   /* +09 */
    char fc;   /* +0c */
    char fd;   /* +0d */
    char name[4];   /* +0e */
    char name2[0x14];   /* +12 */
    unsigned char f26;   /* +26 */
    char f27;   /* +27 */
    char f28;   /* +28 */
    char f29;   /* +29 */
    char f2a;   /* +2a */
    char f2b;   /* +2b */
    char f2c;   /* +2c */
    char f2d;   /* +2d */
    char f2e;   /* +2e */
    char f2f[2];   /* +2f */
    char f31[2];   /* +31 */
    char f33;   /* +33 */
    char f34;   /* +34 */
    char f35;   /* +35 */
    char f36[7];   /* +36 */
    char pad3d[7];
    struct s19 f44;   /* +44 */
    char pad57[7];
    int f5e;   /* +5e */
    char f60[10];   /* +60 */
    char f6a[10];   /* +6a */
    struct s6 f74[16];   /* +74 */
    char fd4;   /* +d4 */
    char padd5[1];
};
struct ent { char name[4]; unsigned long size; int type; };
struct anrec { char name[10]; int size; };
struct cm { char pad[10]; char a; char b; char rest[116]; };
struct s12 { int x, y, z; char pad[12]; };

extern unsigned char D_a895;
extern struct etype D_a896[];
extern unsigned char far *D_c103;
extern int D_017c;
extern int D_c105;
extern int D_c654;
extern int D_c656;
extern int D_d6f5;
extern int D_d6f7;
extern int D_e0ea, D_e0ec, D_e0ee, D_e0f0, D_e0f2, D_e0f4, D_e0f6, D_e0f8, D_e0fa, D_e0fc,
    D_e0fe, D_e100, D_e102, D_e104, D_e106, D_e108, D_e10a, D_e10c, D_e10e, D_e110, D_e112, D_e114;
extern int D_e15e;
extern int D_e164;
extern int D_e167;
extern int D_e16a;
extern int D_e16c;
extern int D_e16e;
extern int D_e170;
extern int D_e172;
extern int D_e238;
extern int D_e23a;
extern char D_07d3;
extern int D_07d4, D_da53, D_da51, D_da4f, D_da47, D_c2bb, D_da4d, D_da49, D_da4b;
extern int D_da6f[], D_da55[], D_da89[], D_d996[], D_d9b0[], D_daa3[], D_dabd[], D_d948[],
    D_d962[], D_d97c[], D_d9ca[];
extern struct cm D_9c69[];
extern struct s12 D_a53e[];

char far *_fstrcpy(char far *, const char far *);
int sprintf(char *, const char *, ...);
void *memset(void *, int, unsigned);
FILE *fopen(const char *, const char *);
unsigned fread(void *, unsigned, unsigned, FILE *);
int fclose(FILE *);
int stricmp(const char *, const char *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int getch(void);
void exit_(int);
int far vec_1b35();
void far sub_root_0617_016a();
void far sub_root_06a1_2052();
unsigned far sub_root_047c_04ce(void);
void far sub_root_047c_056f(void);
void far sub_root_0000_23e6();
void far sub_root_044a_01f4(void);
void far sub_root_047c_0791(void);
void far sub_root_0000_1354(void);
void far sub_s020_1ef8_0000(int);
void far sub_s020_1ef8_05e6(void);
void far sub_s020_1ef8_06b6(void);

void far sub_s020_1ef8_0000(int level)
{
    char msg[0x50];
    char key[0x1e];
    char fname[0x10];
    struct anrec rec;
    struct ent tmp;
    struct ent list[20];
    struct etype *dst;
    struct etsrc far *src;
    int n, i, j, cnt, best, k;
    unsigned long a, b, c, avail;
    FILE *fp;

    sprintf(msg, "level%d.trn", level);
    vec_1b35(msg, D_c103);
    D_a895 = n = *D_c103;
    dst = D_a896;
    src = (struct etsrc far *)(D_c103 + 1);
    cnt = 0;
    for (i = 0; i < n; i++) {
        cnt++;
        _fstrcpy(dst->name, src->name);
        _fstrcpy(list[i].name, src->name);
        _fstrcpy(dst->name2, src->name2);
        for (j = 0; j < 16; j++)
            dst->fc2[j] = src->f74[j];
        dst->f7e = src->fd4;
        dst->f1e = dst->f1f = 8;
        dst->f22 = src->f2a;
        dst->f5a = src->f2b;
        dst->f9e = src->f6;
        dst->f24 = src->f2c;
        dst->f48 = src->f2d;
        dst->f49 = src->f2e;
        list[i].type = dst->f9c = src->f26;
        dst->fac = src->f5e;
        for (j = 0; j < 10; j++) {
            dst->fae[j] = src->f60[j];
            dst->fb8[j] = src->f6a[j];
        }
        for (j = 0; j < 2; j++) {
            dst->f4b[j] = src->f2f[j];
            dst->f4f[j] = src->f31[j];
        }
        dst->f51 = src->f28;
        dst->f58 = src->f29;
        dst->f55 = src->f33;
        dst->f5b = src->f34;
        dst->f5c = src->f35;
        dst->f9b = src->f27;
        dst->fa4 = src->f4;
        dst->fa6 = src->f8;
        dst->faa = src->fc;
        dst->fab = src->fd;
        dst->fa2 = src->f2;
        dst->fa0 = src->f0;
        dst->fa7 = src->f9;
        for (j = 0; j < 7; j++)
            dst->f5d[j] = dst->f64[j] = src->f36[j];
        dst->f6b = src->f44;
        dst++;
        src++;
    }
    sprintf(fname, "level%d.enm", level);
    sub_root_0617_016a(fname, (char far *)&D_a895, n * 0x122 + 1);
    fp = fopen("tacanim.db", "rb");
    for (i = 0; i < cnt; i++) {
        strcpy(key, list[i].name);
        strcat(key, "WK");
        a = b = c = 0;
        sub_root_06a1_2052(fp);
        while (!(fp->_flag & 0x10)) {
            fread(&rec, 12, 1, fp);
            rec.name[5] = 0;
            if (stricmp(rec.name, key) == 0) {
                a = rec.size;
                fread(&rec, 12, 1, fp);
                b = rec.size;
                fread(&rec, 12, 1, fp);
                c = rec.size;
                break;
            }
        }
        list[i].size = c + b + a;
    }
    fclose(fp);
    for (i = 0; i < cnt - 1; i++) {
        best = i;
        tmp = list[i];
        for (j = i + 1; j < cnt; j++)
            if (list[j].size > tmp.size) {
                best = j;
                tmp = list[j];
            }
        list[best] = list[i];
        list[i] = tmp;
    }
    a = c = 0;
    k = 0;
    for (i = 0; i < cnt; i++) {
        if (list[i].type == 1) {
            c = list[i].size;
            if (i > 1)
                break;
        } else if (k < 2) {
            a += list[i].size;
            k++;
        }
    }
    avail = (unsigned long)sub_root_047c_04ce() << 4;
    a += c;
    if (avail < a) {
        sub_root_047c_056f();
        sprintf(msg, "Error!   Level %d Enemy buffer sizing", level);
        sub_root_0000_23e6(D_017c, 0x1e, 0, msg);
        sprintf(msg, "Mem req'd: %ld  available: %ld", a, avail);
        sub_root_0000_23e6(D_017c, 0x3c, 0, msg);
        sprintf(msg, "Press a key to return to DOS");
        sub_root_0000_23e6(D_017c, 0x5a, 0, msg);
        getch();
        sub_root_044a_01f4();
        sub_root_047c_0791();
        sub_root_0000_1354();
        exit_(0);
    }
}

void far sub_s020_1ef8_05e6(void)
{
    D_e238 = 0x26ec;
    D_e23a = 0x1766;
    D_d6f7 = D_c105;
    D_d6f5 = D_c105;
    D_e164 = 0x578;
    D_e167 = 0x19f4;
    D_e16a = 0x20f4;
    D_e16c = 0x220c;
    D_e16e = 0x2324;
    D_e170 = 0x243c;
    D_e172 = 0x2554;
    D_c656 = D_c105;
    D_c654 = -0x4000;
    D_e15e = 0;
    D_e0ea = 0;
    D_e0ec = 0x9c4;
    D_e0ee = 0x9ce;
    D_e0f0 = 0x9d8;
    D_e0f2 = 0x9e2;
    D_e0f4 = 0x9f6;
    D_e0f6 = 0xa0a;
    D_e0f8 = 0xa1e;
    D_e0fa = 0xa32;
    D_e0fc = 0xa46;
    D_e0fe = 0xa5a;
    D_e100 = 0xa64;
    D_e102 = 0xa6e;
    D_e104 = 0xd8e;
    D_e106 = 0x10ae;
    D_e108 = 0x13ce;
    D_e10a = 0x13d8;
    D_e10c = 0x14d2;
    D_e10e = 0x1522;
    D_e110 = 0x1572;
    D_e112 = 0x15c2;
    D_e114 = 0x1752;
}

void far sub_s020_1ef8_06b6(void)
{
    struct cm *p;
    struct s12 *q;

    D_07d3 = 0;
    D_da4b = D_da49 = D_da4d = D_c2bb = D_da47 = D_da4f = D_da51 = D_da53 = D_07d4 = 0;
    memset(D_da6f, 0, 26);
    memset(D_da55, 0, 26);
    memset(D_da89, 0, 26);
    memset(D_d996, 0, 26);
    memset(D_d9b0, 0, 26);
    memset(D_daa3, 0, 26);
    memset(D_dabd, 0, 26);
    memset(D_d948, 0, 26);
    memset(D_d962, 0, 26);
    memset(D_d97c, 0, 26);
    memset(D_d9ca, 0, 26);
    for (p = &D_9c69[0], q = D_a53e; p < &D_9c69[13]; p++, q++) {
        p->a = p->b = 8;
        q->x = q->y = q->z = -536;
    }
}
