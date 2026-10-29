VERSION=4.3.5; GIT=https://github.com/warmcat/libwebsockets.git; REF=v4.3.5
DEPS="openssl zlib"
CMAKE_ARGS_EXTRA=( -DLWS_WITH_SHARED=ON -DLWS_WITH_STATIC=ON
                   -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITHOUT_TEST_SERVER=ON
                   -DLWS_WITH_MINIMAL_EXAMPLES=OFF
                   -DLWS_OPENSSL_LIBRARIES="$STAGE$PREFIX/lib/libssl.so;$STAGE$PREFIX/lib/libcrypto.so"
                   -DLWS_OPENSSL_INCLUDE_DIRS="$STAGE$PREFIX/include" )
