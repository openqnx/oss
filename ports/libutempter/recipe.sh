# libutempter: lets unprivileged programs (terminal emulators, screen, tmux)
# add and remove utmp/wtmp records for the pseudo-terminals they open,
# through a small setgid helper (libexec/utempter/utempter). 1.2.3 with
# qnx-ports' QNX support (patches/0001): QNX's struct utmp has ut_time, not
# ut_tv, and no ut_host; there is no getresgid() or updwtmp() (wtmp is
# appended through utmpname()/pututline()); SA_RESTART is 0.
VERSION=1.2.3; GIT=https://github.com/altlinux/libutempter.git; REF=1.2.3-alt1
SUBDIR=libutempter
BUILDSYS=custom
# The fork's changes are under "#ifdef QNX", a macro its own build defines
# (the compiler predefines __QNX__): -DQNX here. A plain Makefile, built in a
# copy of the source; -Wl,-stats is a GNU ld option qcc does not forward.
_um() { make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-DQNX -D_QNX_SOURCE" CFLAGS="-O2" LINK_STATS= \
  libdir="$PREFIX/lib" libexecdir="$PREFIX/libexec" includedir="$PREFIX/include" mandir="$PREFIX/share/man" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _um -j"$(nproc)"; }
do_install() { _um DESTDIR="$STAGE" install; }
