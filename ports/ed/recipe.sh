# ed: the POSIX line editor (scripts and the shell's fc use it; QNX ships it too).
VERSION=1.22.6
TARBALL=https://ftp.gnu.org/gnu/ed/ed-1.22.6.tar.lz
SHA256=3f33b22135219c39c3c695f7b7171c2567d3e2a17c798c0a90607320cbb268f2
DEPS=""
BUILDSYS=autotools
# Its own configure (not autoconf) ignores --host and the environment and
# takes the compiler as CC=...: without it, it built ed for the build machine
# (a Linux executable, for both architectures).
# POSIX regcomp/regexec live in libregex on QNX.
CONF_ARGS_EXTRA=( CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lregex" )
