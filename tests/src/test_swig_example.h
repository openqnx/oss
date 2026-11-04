/* The C library the swig test wraps (tests/src/test_swig_example.i): plain
   functions, a global variable, a struct, a string result. */
#ifndef TEST_SWIG_EXAMPLE_H
#define TEST_SWIG_EXAMPLE_H
extern int qs_calls;                       /* counts the calls below */
int qs_gcd(int a, int b);
double qs_hypot3(double x, double y, double z);
unsigned qs_crc8(const char *s);           /* CRC-8 (poly 0x07) of a string */
char *qs_reverse(const char *s);           /* new string, caller frees */
typedef struct { int n; long total; } qs_acc;
void qs_acc_add(qs_acc *a, int v);
#endif
