# libvalkey: the Valkey project's C client library (a hiredis fork with
# cluster support), with its TLS layer (libvalkey_tls) against the staged
# OpenSSL. RDMA support is Linux-only (rdma-core) and stays off.
VERSION=0.5.0; GIT=https://github.com/valkey-io/libvalkey.git; REF=0.5.0
DEPS="openssl"
BUILDSYS=cmake
# Tests off: they need a running valkey-server on the build machine.
CMAKE_ARGS_EXTRA=( -DENABLE_TLS=ON -DENABLE_RDMA=OFF -DDISABLE_TESTS=ON -DBUILD_SHARED_LIBS=ON )
