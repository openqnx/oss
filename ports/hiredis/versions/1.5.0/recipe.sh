# hiredis: the minimalistic C client library for Redis (and Valkey), with its
# TLS layer (libhiredis_ssl) built against the staged OpenSSL.
VERSION=1.5.0; GIT=https://github.com/redis/hiredis.git; REF=v1.5.0
DEPS="openssl"
BUILDSYS=cmake
# Tests off: they need a running redis-server on the build machine. No NuGet
# packaging file (a Windows thing).
CMAKE_ARGS_EXTRA=( -DENABLE_SSL=ON -DDISABLE_TESTS=ON -DENABLE_NUGET=OFF -DBUILD_SHARED_LIBS=ON )
