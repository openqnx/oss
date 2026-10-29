VERSION=5.7.6; GIT=https://github.com/wolfSSL/wolfssl.git; REF=v5.7.6-stable
# GCC 12 raises a false-positive -Wstringop-overflow in tls.c; wolfSSL turns
# warnings into errors by default, so demote just that one.
CFLAGS_EXTRA="-Wno-error=stringop-overflow"
CMAKE_ARGS_EXTRA=( -DCMAKE_C_FLAGS="$CFLAGS_EXTRA" -DBUILD_SHARED_LIBS=ON -DWOLFSSL_EXAMPLES=OFF
                   -DWOLFSSL_CRYPT_TESTS=OFF -DWOLFSSL_TLS13=ON
                   -DWOLFSSL_DTLS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
