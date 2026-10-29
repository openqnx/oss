# Z shell. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.9
TARBALL=https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
SHA256=9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses pcre2 libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-multibyte --with-tcsetpgrp --disable-gdbm )
