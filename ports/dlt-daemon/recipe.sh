VERSION=2.18.10; GIT=https://github.com/COVESA/dlt-daemon.git; REF=v2.18.10
# dlt-qnx-system reads its slog2 context map with the libjson port (patch
# 0002), not with the libjson of QNX: "-ljson" finds the port first, and
# the ports use ported libraries rather than those of QNX where they can.
DEPS="zlib libjson"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWITH_DLT_QNX_SYSTEM=ON
                   -DWITH_DLT_CXX11_EXT=ON -DDLT_IPC=UNIX_SOCKET
                   -DWITH_DLT_ADAPTOR=ON -DWITH_DLT_USE_IPv6=OFF
                   -DWITH_LIB_SHORT_VERSION=ON -DWITH_SYSTEMD=OFF
                   -DWITH_DLT_UNIT_TESTS=OFF -DWITH_DLT_INSTALLED_TESTS=OFF
                   -DWITH_DLT_EXAMPLES=OFF
                   # upstream adds dlt-test-cpp-extension twice when CXX11_EXT is on
                   -DWITH_DLT_TESTS=OFF )
