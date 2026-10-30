# htop: the interactive process viewer, with QNX's QNX platform (patches/0001,
# from github.com/qnx-ports/htop: processes from /proc, CPU and memory from
# QNX's system page).
VERSION=3.5.0; GIT=https://github.com/htop-dev/htop.git; REF=3.5.0
DEPS="ncurses"
BUILDSYS=autotools
do_patch() { cd "$SRCROOT" && ./autogen.sh; }
CONF_ARGS_EXTRA=( --enable-unicode --disable-sensors --disable-capabilities )
