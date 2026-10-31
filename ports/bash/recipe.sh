VERSION=5.2.37
TARBALL=https://ftp.gnu.org/gnu/bash/bash-5.2.37.tar.gz
SHA256=9599b22ecd1d5787ad7d3b7bf0c59f312b3396d1e281175dd1f8a4014da621ff
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses readline libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-bash-malloc --disable-nls --with-installed-readline --with-curses )
# regcomp/regexec live in libregex on QNX: without it configure finds no
# regexec and [[ string =~ regex ]] is compiled out ("conditional binary
# operator expected").
LDFLAGS_APPEND="-lregex"
