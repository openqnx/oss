VERSION=8.2
TARBALL=https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz
SHA256=3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35
BUILDSYS=autotools
DEPS="ncurses"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-curses )
