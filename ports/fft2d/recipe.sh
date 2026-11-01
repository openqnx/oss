# Takuya Ooura's General Purpose FFT package (fft2d.tgz, 2006-12-28): 1D,
# 2D and 3D complex/real DFT, DCT and DST (split-radix "fftsg" versions) and
# the 8x8/16x16 DCTs. Upstream has no version number; this is the date of
# the only release of the 2D package, the same file TensorFlow pinned.
VERSION=2006.12.28
TARBALL=https://www.kurims.kyoto-u.ac.jp/~ooura/fft2d.tgz
SHA256=ada7e99087c4ed477bfdf11413f2ba8db8a840ba9bbf8ac94f4f3972e2a7cec9
BUILDSYS=custom
# The upstream server is often unreachable from outside Japan; the Internet
# Archive's capture of that same URL is the fallback. Either way the file
# must match the pinned checksum.
FFT2D_MIRROR="https://web.archive.org/web/20250108162814id_/$TARBALL"
do_fetch() {
  local tb="$WS/dist/fft2d-$VERSION.tgz"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -m 60 --retry 2 -o "$tb" "$TARBALL" || curl -fsSL --retry 3 -o "$tb" "$FFT2D_MIRROR" || { rm -f "$tb"; return 1; }
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "checksum mismatch: $tb"; rm -f "$tb"; return 1; }
  tar xzf "$tb" -C "$SRCROOT" --strip-components=1
}
# Upstream ships C files to copy into a project, no library and no header.
# Built here as libfft2d.so.0 from fftsg.c, fftsg2d.c, fftsg3d.c and
# shrtdct.c (fft4f2d.c is the older 2D version with the same function
# names). Only the documented entry points are exported (version script):
# the internal helpers have generic names (makewt, bitrv2, cftfsub, ...)
# that would collide with other copies of Ooura's code. fft2d.h declares
# them, with the argument descriptions in the source files. Single-threaded
# (no USE_FFT2D_PTHREADS), as TensorFlow builds it.
FFT2D_API="cdft rdft ddct ddst dfct dfst cdft2d rdft2d rdft2dsort ddct2d ddst2d
           cdft3d rdft3d rdft3dsort ddct3d ddst3d ddct8x8s ddct16x16s"
do_build() {
  local f objs=
  { echo "FFT2D_0 { global:"; for f in $FFT2D_API; do echo "  $f;"; done; echo "local: *; };"; } > "$B/fft2d.map"
  for f in fftsg fftsg2d fftsg3d shrtdct; do
    qcc -V$QCC_TARGET -O2 -fPIC -D_QNX_SOURCE -c "$S/$f.c" -o "$B/$f.o" || return 1
    objs="$objs $B/$f.o"
  done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libfft2d.so.0 -Wl,--version-script,"$B/fft2d.map" \
    -o "$B/libfft2d.so.0" $objs -lm
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/share/doc/fft2d" &&
  install -m755 "$B/libfft2d.so.0" "$L/" && ln -sf libfft2d.so.0 "$L/libfft2d.so" &&
  install -m644 "$S/readme2d.txt" "$STAGE$PREFIX/share/doc/fft2d/" &&
  cat > "$STAGE$PREFIX/include/fft2d.h" <<'H'
/* Ooura's General Purpose FFT package (libfft2d): the entry points of
   fftsg.c, fftsg2d.c, fftsg3d.c and shrtdct.c. The argument conventions
   (sizes powers of 2, isgn, the ip/w work areas with ip[0] = 0 on the first
   call, t work area or NULL) are described at the top of each source file;
   see also share/doc/fft2d/readme2d.txt. Written for this package: upstream
   ships no header. */
#ifndef FFT2D_H
#define FFT2D_H
#ifdef __cplusplus
extern "C" {
#endif
/* 1D (fftsg.c) */
void cdft(int n, int isgn, double *a, int *ip, double *w);
void rdft(int n, int isgn, double *a, int *ip, double *w);
void ddct(int n, int isgn, double *a, int *ip, double *w);
void ddst(int n, int isgn, double *a, int *ip, double *w);
void dfct(int n, double *a, double *t, int *ip, double *w);
void dfst(int n, double *a, double *t, int *ip, double *w);
/* 2D (fftsg2d.c): a[0...n1-1][0...n2-1] */
void cdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void rdft2dsort(int n1, int n2, int isgn, double **a);
void ddct2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
void ddst2d(int n1, int n2, int isgn, double **a, double *t, int *ip, double *w);
/* 3D (fftsg3d.c): a[0...n1-1][0...n2-1][0...n3-1] */
void cdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void rdft3dsort(int n1, int n2, int n3, int isgn, double ***a);
void ddct3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
void ddst3d(int n1, int n2, int n3, int isgn, double ***a, double *t, int *ip, double *w);
/* 8x8 and 16x16 DCT (shrtdct.c): a[0...7][0...7], a[0...15][0...15] */
void ddct8x8s(int isgn, double **a);
void ddct16x16s(int isgn, double **a);
#ifdef __cplusplus
}
#endif
#endif
H
  cat > "$L/pkgconfig/fft2d.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: fft2d
Description: Ooura's general purpose 1D/2D/3D FFT package
Version: $VERSION
Libs: -L\${libdir} -lfft2d
Cflags: -I\${includedir}
PC
}
