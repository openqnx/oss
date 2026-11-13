# clinfo: prints every property of the OpenCL platforms and devices the ICD
# loader finds -- names, versions, extensions, device limits, the loader's
# own profile -- as text (default, --human), raw values (--raw) or JSON
# (--json); -l lists platforms and devices as a tree. The first thing to run
# when an OpenCL program finds no device. Without an installed OpenCL
# implementation it prints "Number of platforms 0" and exits 0.
#
# The source is one portable C99 file and needed no change for QNX.
# Upstream's Makefile compiles into the source directory (one tree for both
# architectures here) and links -ldl, which QNX does not have (dlopen() is in
# libc), so the file is compiled directly: against opencl-headers and
# opencl-icd-loader's libOpenCL.so.1. The manual page goes to share/man.
VERSION=3.1.26.09.26; GIT=https://github.com/Oblomov/clinfo.git; REF=3.1.26.09.26
DEPS="opencl-headers opencl-icd-loader"
BUILDSYS=custom
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -std=c99 -Wall -Wextra -I"$STAGE$PREFIX/include" \
    -o "$B/clinfo" "$S/src/clinfo.c" -L"$STAGE$PREFIX/lib" -lOpenCL
}
do_install() {
  install -Dm755 "$B/clinfo" "$STAGE$PREFIX/bin/clinfo"
  install -Dm644 "$S/man1/clinfo.1" "$STAGE$PREFIX/share/man/man1/clinfo.1"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/clinfo/LICENSE"
  install -m644 "$S/legalcode.txt" "$STAGE$PREFIX/share/licenses/clinfo/"
}
