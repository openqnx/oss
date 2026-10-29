VERSION=2.9; GIT=https://salsa.debian.org/minicom-team/minicom.git; REF=2.9
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --enable-socket --enable-lock-dir=/var/lock --enable-dfl-port=/dev/ser1 )
