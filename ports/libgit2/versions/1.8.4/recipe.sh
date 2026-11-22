VERSION=1.8.4; GIT=https://github.com/libgit2/libgit2.git; REF=v1.8.4
DEPS="openssl zlib pcre2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DBUILD_CLI=OFF -DUSE_SSH=OFF
                   -DUSE_HTTPS=OpenSSL -DREGEX_BACKEND=pcre2 -DUSE_HTTP_PARSER=builtin -DUSE_GSSAPI=OFF
                   -DUSE_NTLMCLIENT=OFF )
