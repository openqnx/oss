VERSION=2.11.1; GIT=https://salsa.debian.org/minicom-team/minicom.git; REF=2.11.1
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --enable-socket --enable-lock-dir=/var/lock --enable-dfl-port=/dev/ser1 )
# The git tag carries configure, aclocal.m4 and the Makefile.in files made
# by automake 1.18, but a clone gives every file the checkout time in no
# particular order: configure.ac could come out newer than aclocal.m4, and
# make then reruns aclocal-1.18, which the build machine does not have. Put
# the generated files after their inputs again; nothing is regenerated.
do_patch() {
  ( cd "$SRCROOT" && touch aclocal.m4 && touch configure config.h.in \
    && find . -name Makefile.in -exec touch {} + )
}
