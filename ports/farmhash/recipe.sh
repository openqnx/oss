VERSION=1.1-git-20190513; GIT=https://github.com/google/farmhash.git; COMMIT=0d859a811870d10f53a594927d0d0b97573ad06d
BUILDSYS=autotools
# farmhash.h's little-endian test misspelled __BYTE_ORDER__ and fell through
# to <endian.h>, a glibc header: patches/0003 fixes the installed header, so
# programs that include it compile on QNX too (no -D__LITTLE_ENDIAN__).
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# The QNX patch touches configure.ac/Makefile.am; regenerate the build files
# (the tarball's were made with automake 1.14, which make then asks for).
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
