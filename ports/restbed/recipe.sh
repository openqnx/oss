VERSION=5.0.0; GIT=https://github.com/Corvusoft/restbed.git; REF=5.0.0
DEPS="openssl asio"
# restbed 5 is C++23 (CMAKE_CXX_STANDARD 23, std::format; QNX's libc++ 18
# has it) and builds against the staged standalone Asio: 5.0 uses
# asio::io_context, and its dependency/ submodules are gone.
# CMAKE_CXX_FLAGS replaces the toolchain's initial flags on purpose: their
# "-isystem <staging>/include" goes ahead of every include dir CMake adds
# (qcc puts it first), so an earlier restbed's headers staged there would
# shadow 5.0's own; the staging dir still comes in through Asio's and
# OpenSSL's include dirs, after src/ (patch 0002). -Wno-narrowing: uri.cpp
# puts -1 in a char table, a narrowing error where char is unsigned (aarch64).
# The shared library only: no static one, no tests (upstream's test tree
# wants Catch2 from vcpkg).
CMAKE_ARGS_EXTRA=( -DCMAKE_CXX_FLAGS=-Wno-narrowing
                   -DBUILD_SSL=ON -DBUILD_IPC=OFF -DBUILD_TESTS=OFF
                   -DBUILD_SHARED_LIBRARY=ON -DBUILD_STATIC_LIBRARY=OFF
                   -DCMAKE_INSTALL_LIBDIR=lib )
