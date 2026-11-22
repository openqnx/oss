VERSION=5.3
TARBALL=https://ftp.gnu.org/gnu/bash/bash-5.3.tar.gz
SHA256=0d5cd86965f869a26cf64f4b71be7b96f90a3ba8b3d74e27e8e9d9d5550f31ba
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses readline libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-bash-malloc --disable-nls --with-installed-readline --with-curses )
# regcomp/regexec live in libregex on QNX: without it configure finds no
# regexec and [[ string =~ regex ]] is compiled out ("conditional binary
# operator expected").
LDFLAGS_APPEND="-lregex"
