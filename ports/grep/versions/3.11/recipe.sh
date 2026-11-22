# GNU grep. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.11
TARBALL=https://ftp.gnu.org/gnu/grep/grep-3.11.tar.xz
SHA256=1db2aedde89d0dea42b16d9528f894c8d15dae4e190b59aecc78f5a951276eab
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="pcre2 libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )

# egrep and fgrep are #!/bin/bash wrappers (configure's SHELL); a QNX target
# has no /bin/bash, and they shadow the base system's egrep/fgrep in PATH.
# They are POSIX sh scripts: run them with /bin/sh (hazard 105).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i '1s|^#!.*|#!/bin/sh|' "$STAGE$PREFIX/bin/egrep" "$STAGE$PREFIX/bin/fgrep"
}
