VERSION=1.1-git-20190513; GIT=https://github.com/google/farmhash.git; COMMIT=0d859a811870d10f53a594927d0d0b97573ad06d
BUILDSYS=autotools
# farmhash.h's little-endian test misspells __BYTE_ORDER__ and falls through to
# <endian.h>, a glibc header. Both QNX targets are little endian; say so.
CFLAGS_APPEND="-D__LITTLE_ENDIAN__"
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# The QNX patch touches configure.ac/Makefile.am; regenerate the build files
# (the tarball's were made with automake 1.14, which make then asks for).
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
