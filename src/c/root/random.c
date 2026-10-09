/* place: root 0410:0008 */
/* orig: size=0012 */
/* Random numbers (root segment 0410). */

int rand(void);

/* random_n: uniform random integer in [0, n) - scales rand()'s 15 bits by n */
int far random_n(int n)
{
    return (int)(((unsigned long)(unsigned)(rand() << 1) * (unsigned)n) >> 16);
}
