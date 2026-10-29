VERSION=2.3.4; GIT=https://gitlab.com/lely_industries/lely-core.git; REF=v2.3.4
BUILDSYS=autotools
# CANopen stack. QNX has no SocketCAN, so the generic (non-socketcan) CAN
# backend is selected; the vehicle interface is bound at integration time.
# lely bundles a getopt replacement whose header declares optarg/optind/opterr/
# optopt as tentative definitions. Since GCC 10 the default is -fno-common, so
# every translation unit that includes it emits its own definition and the
# library fails to link with "multiple definition of optarg".
CFLAGS_APPEND="-fcommon"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-cython
                  --disable-python --disable-tools --disable-doc
                  --disable-tests --disable-daemon )
