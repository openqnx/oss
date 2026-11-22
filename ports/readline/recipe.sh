VERSION=8.3
TARBALL=https://ftp.gnu.org/gnu/readline/readline-8.3.tar.gz
SHA256=fe5383204467828cd495ee8d1d3c037a7eba1389c22bc6a041f627976f9061cc
BUILDSYS=autotools
DEPS="ncurses"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-curses )
