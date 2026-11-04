# help2man: generates a man page from a program's --help and --version
# output. A Perl script. Version as in QNX's own package list.
VERSION=1.49.3
TARBALL=https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz
SHA256=4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f
DEPS=""
RUNTIME_DEPS="perl"
BUILDSYS=autotools
# --disable-nls: otherwise configure finds the build machine perl's
# Locale::gettext and generates help2man "--with-gettext": the script then
# requires Locale::gettext, which the perl port does not have, and installs
# bindtextdomain.so, an LD_PRELOAD helper for --locale. Without NLS it is a
# plain script that needs only perl's core modules.
CONF_ARGS_EXTRA=( --disable-nls )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/help2man"
}
