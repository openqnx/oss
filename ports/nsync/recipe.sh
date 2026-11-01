VERSION=1.30.0; GIT=https://github.com/google/nsync.git; REF=1.30.0
# Google's nsync: C (libnsync) and C++ (libnsync_cpp) synchronisation
# primitives. On QNX upstream's CMake takes its generic POSIX platform
# (semaphores on a pthread mutex and condition variable; no futex), which is
# right; but it picks the compiler's atomics only for CMake's "GNU", and qcc
# is "QCC": patches/0001.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DNSYNC_ENABLE_TESTS=OFF )
