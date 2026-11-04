/* Implementation of test_swig_example.h, linked into both wrappers. */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "test_swig_example.h"
int qs_calls;
int qs_gcd(int a, int b) { qs_calls++; while (b) { int t = a % b; a = b; b = t; } return a < 0 ? -a : a; }
double qs_hypot3(double x, double y, double z) { qs_calls++; return sqrt(x * x + y * y + z * z); }
unsigned qs_crc8(const char *s)
{
    unsigned c = 0;
    qs_calls++;
    for (; *s; s++) {
        int i;
        c ^= (unsigned char)*s;
        for (i = 0; i < 8; i++) c = (c & 0x80) ? ((c << 1) ^ 0x07) & 0xff : (c << 1) & 0xff;
    }
    return c;
}
char *qs_reverse(const char *s)
{
    size_t n = strlen(s), i;
    char *r = malloc(n + 1);
    qs_calls++;
    for (i = 0; i < n; i++) r[i] = s[n - 1 - i];
    r[n] = 0;
    return r;
}
void qs_acc_add(qs_acc *a, int v) { qs_calls++; a->n++; a->total += v; }
