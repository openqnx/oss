# libunwind: call-chain unwinding (unw_backtrace, unw_step, remote and core
# file unwinding). The QNX SDP ships libunwind 1.8.2 as a system library
# (/usr/lib/libunwind.so.8, libunwind-nto, libunwind-coredump: QNX's build);
# this is upstream's newer release, built from source -- its QNX support
# (the nto remote target) is upstream's own. It gets a prefix of its own,
# /usr/local/opt/libunwind, so it never takes the place of the system
# library: a program uses it on purpose, with -I/-L.../opt/libunwind/
# {include,lib} and LD_LIBRARY_PATH or a run path. libtool numbers QNX
# libraries by their interface number: this one is libunwind.so.10 (the
# SDP's QNX build: libunwind.so.8, the same interface); libunwind-nto.so.0
# and libunwind-coredump.so.0 have the SDP's names, one more reason for
# the prefix of their own.
VERSION=1.8.3; GIT=https://github.com/libunwind/libunwind.git; REF=v1.8.3
DEPS=""
BUILDSYS=autotools
UNW_PREFIX="$PREFIX/opt/libunwind"
CONF_ARGS_EXTRA=( --prefix="$UNW_PREFIX" --disable-tests --disable-documentation
                  --disable-minidebuginfo --disable-zlibdebuginfo --enable-shared --enable-static )
