# ed: the POSIX line editor (scripts and the shell's fc use it; QNX ships it too).
VERSION=1.22.5
TARBALL=https://ftp.gnu.org/gnu/ed/ed-1.22.5.tar.lz
SHA256=56e107ddc2f29dad6690376c15bf9751509e1ee3b8241710e44edbe5c3a158cc
DEPS=""
BUILDSYS=autotools
# Its own configure (not autoconf) ignores --host and the environment and
# takes the compiler as CC=...: without it, it built ed for the build machine
# (a Linux executable, for both architectures).
# POSIX regcomp/regexec live in libregex on QNX.
CONF_ARGS_EXTRA=( CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lregex" )
