VERSION=3.8
TARBALL=https://github.com/tmux/tmux/releases/download/3.8/tmux-3.8.tar.gz
SHA256=e79c699c7e949dccd0a4a125e17b8d1261e311b16979dbc8eb34542f3966d82e
DEPS="libevent ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-utf8proc --enable-static=no )
# tmux calls regcomp/regexec, which QNX keeps in libregex rather than libc.
CONF_ENV=( LIBS="-lregex" )
