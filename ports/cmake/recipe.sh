# CMake for the target (to build software on the board). Built with the host
# CMake and the QNX toolchain file; its bundled third-party libraries are used
# except where this tree provides them. Version as in the QNX Everywhere image.
VERSION=4.1.1; GIT=https://gitlab.kitware.com/cmake/cmake.git; REF=v4.1.1
DEPS="openssl zlib curl libuv"
# CMake's own C++ feature probes treat any compiler output as failure, and
# qcc always prints "cc: warning - lang-c++ is deprecated" (CMake's QCC module
# passes -lang-c++), so every probe "fails". The QNX 8 toolchain (GCC 12,
# libc++) has all of them; answer them.
CMAKE_ARGS_EXTRA=( -DCMake_HAVE_CXX_MAKE_UNIQUE=ON -DCMake_HAVE_CXX_UNIQUE_PTR=ON -DCMake_HAVE_CXX_FILESYSTEM=ON
                   -DCMake_HAVE_CXX_ATOMIC_BUILTIN=ON -DBUILD_TESTING=OFF -DBUILD_CursesDialog=OFF -DBUILD_QtDialog=OFF -DCMAKE_USE_OPENSSL=ON
                   -DCMAKE_USE_SYSTEM_ZLIB=ON -DCMAKE_USE_SYSTEM_CURL=ON -DCMAKE_USE_SYSTEM_LIBUV=ON
                   -DSPHINX_MAN=OFF -DSPHINX_HTML=OFF )
