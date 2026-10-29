VERSION=3.5a
TARBALL=https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz
SHA256=16216bd0877170dfcc64157085ba9013610b12b082548c7c9542cc0103198951
DEPS="libevent ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-utf8proc --enable-static=no )
# tmux calls regcomp/regexec, which QNX keeps in libregex rather than libc.
CONF_ENV=( LIBS="-lregex" )
