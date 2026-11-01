# libdaemon 0.14 (the last release) with qnx-ports' QNX support
# (patches/0001: SA_RESTART, which QNX does not implement, defined to 0;
# <sys/select.h> for select()); patches/0002 then retries the system calls
# daemon_fork() had relied on SA_RESTART for (EINTR from its SIGCHLD
# handler made it return -1 in the parent of a daemon that had started).
# Used by avahi-daemon to daemonize, write its PID file and pass signals
# through a pipe.
VERSION=0.14
TARBALL=http://0pointer.de/lennart/projects/libdaemon/libdaemon-0.14.tar.gz
SHA256=fd23eb5f6f986dcc7e708307355ba3289abe03cc381fc47a80bca4a50aa6b834
BUILDSYS=autotools
# The 2012 tarball's config.sub predates the *-nto-qnx8.0.0 triples:
# autoreconf replaces the generated build system with the host's current one.
do_patch() { ( cd "$SRCROOT" && autoreconf -fi ); }
# No lynx/doxygen docs; the examples are only built, never installed.
# AC_FUNC_SETPGRP is a run-test; QNX has the POSIX setpgrp(void).
CONF_ENV=( ac_cv_func_setpgrp_void=yes )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-lynx --disable-examples )
