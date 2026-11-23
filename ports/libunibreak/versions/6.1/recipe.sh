VERSION=6.1; GIT=https://github.com/adah1972/libunibreak.git; REF=libunibreak_6_1
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# autogen.sh in these trees runs ./configure as well, which leaves the source
# directory configured for the build host -- the next (cross) configure then
# refuses with "source directory already configured". Generate the build
# system only.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
