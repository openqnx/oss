# fftw: the Fastest Fourier Transform in the West -- discrete Fourier
# transforms (complex, real, r2r/DCT/DST) of any size and dimension.
VERSION=3.3.10
TARBALL=https://www.fftw.org/fftw-3.3.10.tar.gz
SHA256=56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467
BUILDSYS=autotools
# FFTW builds one precision per configure run, so the hooks configure,
# build and install it twice, as distributions do: double (libfftw3) and
# single (libfftw3f), each with its pthread library (libfftw3_threads,
# libfftw3f_threads). Autotools rather than FFTW's CMake build, which the
# FFTW authors call experimental and which has no NEON option.
# SIMD: on x86_64 SSE2/AVX/AVX2 codelets (FFTW checks the CPU with cpuid at
# run time, so the library still runs on a CPU without AVX); on aarch64 NEON,
# which FFTW has for single precision only. No Fortran wrappers (no Fortran
# compiler in the SDP). Shared libraries only.
FFTW_PRECS="double float"
fftw_simd() {
  case "$QNX_ARCH:$1" in
    x86_64:double) echo "--enable-sse2 --enable-avx --enable-avx2" ;;
    x86_64:float)  echo "--enable-float --enable-sse --enable-sse2 --enable-avx --enable-avx2" ;;
    aarch64le:float) echo "--enable-float --enable-neon" ;;
    *:float) echo "--enable-float" ;;
  esac
}
do_configure() {
  local p
  for p in $FFTW_PRECS; do
    mkdir -p "$B/$p" && ( cd "$B/$p" && \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    CFLAGS="-O3 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lm" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" --enable-shared --disable-static \
      --enable-threads --disable-fortran --disable-doc $(fftw_simd $p) ) || return 1
  done
}
do_build() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" -j"$(nproc)" || return 1; done; }
do_install() { local p; for p in $FFTW_PRECS; do make -C "$B/$p" DESTDIR="$STAGE" install; done; }
