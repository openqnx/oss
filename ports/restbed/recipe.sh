VERSION=4.8; GIT=https://github.com/Corvusoft/restbed.git; REF=4.8
DEPS="openssl asio"
# restbed builds as C++14, where standalone Asio reaches for
# <experimental/string_view> -- a libstdc++ header QNX's libc++ does not ship.
# Tell Asio not to use it. (CMAKE_CXX_FLAGS replaces the toolchain's initial
# flags, so those are repeated.) -Wno-narrowing: uri.cpp puts -1 in a char
# table, which is a narrowing error where char is unsigned (aarch64).
CMAKE_ARGS_EXTRA=( "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -DASIO_DISABLE_STD_EXPERIMENTAL_STRING_VIEW -Wno-narrowing" -DBUILD_SSL=ON -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBRARY=OFF
                   -DCMAKE_INSTALL_LIBDIR=lib )
