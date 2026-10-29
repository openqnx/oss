VERSION=8.11.1; GIT=https://github.com/curl/curl.git; REF=curl-8_11_1
DEPS="openssl zlib nghttp2 c-ares brotli zstd"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_CURL_EXE=ON -DBUILD_TESTING=OFF
                   -DCURL_USE_OPENSSL=ON -DOPENSSL_ROOT_DIR="$STAGE$PREFIX"
                   -DUSE_NGHTTP2=ON -DENABLE_ARES=ON
                   -DCURL_BROTLI=ON -DCURL_ZSTD=ON
                   -DCURL_USE_LIBPSL=OFF -DCURL_USE_LIBSSH2=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
