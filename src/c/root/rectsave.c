/* place: root 03ec:000e */
/* flags: /AM /G2 /Gs /Ox /Zp1 */
/* Copy a rectangle between graphics pages, keeping copies of rectangles saved away from the
   screen in DOS memory blocks (root segment 03ec). */

struct page { int id; int x; int y; };
struct saved { int x; int y; int w; int h; unsigned seg; };

extern char D_9069;
extern char D_907a;
extern struct saved D_8462[12];

unsigned far sub_root_047c_053e(int);
void far sub_root_047c_0553(struct page *, int, int, int, int, struct page *, int, int);
unsigned _dos_allocmem(unsigned, unsigned *);
unsigned _dos_freemem(unsigned);
void movedata(unsigned, unsigned, unsigned, unsigned, unsigned);

int far copy_rect_saved(struct page *src, int sx, int sy, int w, int h,
                        struct page *dst, int dx, int dy)
{
    int i;
    int err;
    unsigned srcseg, dstseg;
    unsigned srcoff, dstoff;

    if (D_9069 != 0 || D_907a == 1) {
        sub_root_047c_0553(src, sx, sy, w, h, dst, dx, dy);
        return 0;
    }
    sx += src->x;
    sy += src->y;
    dx += dst->x;
    dy += dst->y;
    if (dst->id > src->id) {
        for (i = 0; i < 12; i++)
            if (D_8462[i].seg != 0 && D_8462[i].x == sx && D_8462[i].y == sy &&
                D_8462[i].w == w && D_8462[i].h == h)
                break;
        if (i == 12) {
            for (i = 0; i < 12; i++)
                if (D_8462[i].seg == 0)
                    break;
            if (i == 12)
                return 8;
            if ((err = _dos_allocmem((unsigned)(w * h + 15) >> 4, &dstseg)) != 0)
                return err;
            D_8462[i].seg = dstseg;
            D_8462[i].x = sx;
            D_8462[i].y = sy;
            D_8462[i].w = w;
            D_8462[i].h = h;
        }
        dstoff = 0;
        dstseg = D_8462[i].seg;
        srcoff = sy * 320 + sx;
        srcseg = sub_root_047c_053e((src->id + 5) * 2);
        while (h-- > 0) {
            movedata(srcseg, srcoff, dstseg, dstoff, w);
            srcoff += 320;
            dstoff += w;
        }
    } else {
        for (i = 0; i < 12; i++)
            if (D_8462[i].seg != 0 && D_8462[i].x == dx && D_8462[i].y == dy &&
                D_8462[i].w == w && D_8462[i].h == h)
                break;
        if (i == 12)
            return 8;
        srcoff = 0;
        srcseg = D_8462[i].seg;
        dstoff = dy * 320 + dx;
        dstseg = sub_root_047c_053e((dst->id + 5) * 2);
        while (h-- > 0) {
            movedata(srcseg, srcoff, dstseg, dstoff, w);
            srcoff += w;
            dstoff += 320;
        }
        D_8462[i].seg = 0;
        if ((err = _dos_freemem(srcseg)) != 0)
            return err;
    }
    return 0;
}
