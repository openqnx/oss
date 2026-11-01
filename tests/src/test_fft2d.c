/* fft2d (Ooura): known answers for the real DFT of [1 2 3 4] and the
 * complex DFT of an impulse, and forward/inverse round trips (with the
 * documented 2/n, 1/n scaling) of the 1D real DFT, DCT and DST, the 2D
 * complex DFT and DCT, a 3D real DFT and the 8x8 DCT. */
#include "common.h"
#include <fft2d.h>
#include <math.h>
#include <stdlib.h>

static double **alloc2(int n1, int n2) {
  double **a = malloc(sizeof(double *) * n1); int i;
  for (i = 0; i < n1; i++) a[i] = calloc(n2, sizeof(double));
  return a;
}
static int near(double x, double y) { return fabs(x - y) < 1e-9; }

int main(void) {
  int ip[64], i, j, k, ok; double w[64], a[16], b[16];
  QT_BEGIN("fft2d");
  /* rdft, isgn=1: a[0]=sum, a[1]=R[n/2], a[2k]=sum a[j]cos(2pi jk/n), a[2k+1]=sum a[j]sin(2pi jk/n) */
  a[0] = 1; a[1] = 2; a[2] = 3; a[3] = 4; ip[0] = 0;
  rdft(4, 1, a, ip, w);
  QT_CHECK(near(a[0], 10) && near(a[1], -2) && near(a[2], -2) && near(a[3], -2), "rdft([1 2 3 4]) = 10, -2, -2, -2 (Ooura's layout and sign)");
  for (i = 0; i < 16; i++) a[i] = 0;
  a[0] = 1; ip[0] = 0; cdft(16, 1, a, ip, w);
  ok = 1; for (i = 0; i < 8; i++) ok &= near(a[2 * i], 1) && near(a[2 * i + 1], 0);
  QT_CHECK(ok, "cdft of an impulse: all ones");
  for (i = 0; i < 16; i++) b[i] = a[i] = sin(i * 0.7) + 0.25 * i;
  ip[0] = 0; rdft(16, 1, a, ip, w); rdft(16, -1, a, ip, w);
  ok = 1; for (i = 0; i < 16; i++) ok &= near(a[i] * 2.0 / 16, b[i]);
  QT_CHECK(ok, "rdft forward + inverse x 2/n = input");
  for (i = 0; i < 16; i++) a[i] = b[i];   /* the rdft check left a scaled by n/2 */
  ip[0] = 0; ddct(16, -1, a, ip, w); a[0] *= 0.5; ddct(16, 1, a, ip, w);
  ok = 1; for (i = 0; i < 16; i++) ok &= near(a[i] * 2.0 / 16, b[i]);
  QT_CHECK(ok, "ddct forward + inverse round trip");
  for (i = 0; i < 16; i++) a[i] = b[i];
  ip[0] = 0; ddst(16, -1, a, ip, w); a[0] *= 0.5; ddst(16, 1, a, ip, w);
  ok = 1; for (i = 0; i < 16; i++) ok &= near(a[i] * 2.0 / 16, b[i]);
  QT_CHECK(ok, "ddst forward + inverse round trip");
  {
    double **m = alloc2(4, 8), **ref = alloc2(4, 8);
    for (i = 0; i < 4; i++) for (j = 0; j < 8; j++) ref[i][j] = m[i][j] = cos(i + 2.0 * j) + i * j;
    ip[0] = 0; cdft2d(4, 8, 1, m, NULL, ip, w); cdft2d(4, 8, -1, m, NULL, ip, w);
    ok = 1; for (i = 0; i < 4; i++) for (j = 0; j < 8; j++) ok &= near(m[i][j] / 16, ref[i][j]);
    QT_CHECK(ok, "cdft2d 4 x 4 complex forward + inverse / (n1 n2)");
    for (i = 0; i < 4; i++) for (j = 0; j < 8; j++) m[i][j] = ref[i][j];   /* cdft2d left it scaled */
    ip[0] = 0; ddct2d(4, 8, -1, m, NULL, ip, w);
    for (i = 0; i < 4; i++) m[i][0] *= 0.5;
    for (j = 0; j < 8; j++) m[0][j] *= 0.5;
    ddct2d(4, 8, 1, m, NULL, ip, w);
    ok = 1; for (i = 0; i < 4; i++) for (j = 0; j < 8; j++) ok &= near(m[i][j] * 4.0 / (4 * 8), ref[i][j]);
    QT_CHECK(ok, "ddct2d 4 x 8 forward + inverse round trip");
  }
  {
    double ***c = malloc(sizeof(double **) * 4), ***r = malloc(sizeof(double **) * 4);
    for (i = 0; i < 4; i++) { c[i] = alloc2(4, 4); r[i] = alloc2(4, 4); }
    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) for (k = 0; k < 4; k++) r[i][j][k] = c[i][j][k] = i - 2.0 * j + k * k;
    ip[0] = 0; rdft3d(4, 4, 4, 1, c, NULL, ip, w); rdft3d(4, 4, 4, -1, c, NULL, ip, w);
    ok = 1; for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) for (k = 0; k < 4; k++) ok &= near(c[i][j][k] * 2.0 / 64, r[i][j][k]);
    QT_CHECK(ok, "rdft3d 4x4x4 forward + inverse x 2/(n1 n2 n3)");
  }
  {
    double **m = alloc2(8, 8), **ref = alloc2(8, 8);
    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++) ref[i][j] = m[i][j] = (i * 7 + j * 3) % 11 - 5;
    double sum = 0;
    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++) sum += ref[i][j];
    ddct8x8s(-1, m);
    QT_CHECK(near(m[0][0], sum / 8), "ddct8x8s: DC coefficient = sum / 8 (normalized DCT)");
    ddct8x8s(1, m);
    ok = 1; for (i = 0; i < 8; i++) for (j = 0; j < 8; j++) ok &= near(m[i][j], ref[i][j]);
    QT_CHECK(ok, "ddct8x8s forward + inverse = input (orthonormal)");
  }
  QT_END();
}
