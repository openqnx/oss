# chrpath: lists, changes or deletes the RPATH/RUNPATH of ELF executables
# and shared libraries (32/64-bit, either byte order). Version as in QNX's
# own package list.
VERSION=0.18
TARBALL=https://codeberg.org/pere/chrpath/archive/release-0.18.tar.gz
SHA256=83441d1347a09a249c9f7efeac319454e17334d72e2983f18c812c7032cfef54
DEPS=""
BUILDSYS=autotools
# Only the program, its manual and documents: "make all" also builds the
# testsuite programs, linked with the "-R <dir>" rpath option configure
# picked for the build machine (qcc has no -R), and a Debian package
# directory.
do_build() { make -C "$B" -j"$(nproc)" chrpath; }
# Makefile.am puts the documents in $prefix/doc/chrpath-0.18: share/doc.
do_install() { make -C "$B" DESTDIR="$STAGE" docdir="$PREFIX/share/doc/chrpath" install-binPROGRAMS install-man install-docDATA; }
