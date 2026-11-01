# folly: Facebook's open source C++ library -- containers (F14 hash maps,
# small_vector, fbvector, ConcurrentHashMap), futures and executors,
# coroutines, string formatting and conversion (to/from), dynamic/JSON,
# synchronization primitives, IOBuf, async I/O on libevent, and more.
# folly tags a release every week; this is one of them.
VERSION=2026.10.26.00; GIT=https://github.com/facebook/folly.git; REF=v2026.10.26.00
DEPS="boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium"
# Not snappy: the snappy component is built without RTTI (as upstream builds
# it) and folly subclasses snappy::Source, whose typeinfo then does not exist
# (the same reason as in the rocksdb recipe).
# Optional dependencies not used: libdwarf/libiberty/libunwind/backtrace
# (folly's symbolizer and stack traces are written for glibc/Linux ELF
# introspection), libaio and liburing (Linux-only I/O). libevent is found by
# library name: the libevent component's CMake package file names
# /usr/local/lib directly instead of a path relative to itself, which does
# not exist on the build machine.
# Run-time probes a cross build cannot run (hazard 20), answered for QNX:
#  unaligned loads work on x86_64 and on ARMv8-A (exit 0); an undefined weak
#  symbol resolves to null as on any ELF system (0); there is no
#  linux-vdso.so.1 (1); wcstol works (0); HAVE_VSNPRINTF_ERRORS is read by a
#  test only (1).
# Shared libraries (upstream supports them, and recommends static for ABI
# reasons): the package then carries one libfolly.so instead of an archive
# that every consumer has to link with folly's dozen dependencies itself.
# The libraries' version, and so their soname, is folly's PACKAGE_VERSION,
# which in a source build defaults to "0.58.0-dev" (libfolly.so.0.58.0-dev).
# The release tag is the version: folly makes no ABI promise between weekly
# releases, so each one gets a soname of its own.
# (No double-conversion: this folly converts floating point with fast_float.)
CMAKE_ARGS_EXTRA=( -DPACKAGE_VERSION=$VERSION -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DBUILD_BENCHMARKS=OFF -DPYTHON_EXTENSIONS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibDwarf=ON -DCMAKE_DISABLE_FIND_PACKAGE_Libiberty=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibAIO=ON -DCMAKE_DISABLE_FIND_PACKAGE_LibUring=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_LibUnwind=ON -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_Libevent=ON -DCMAKE_DISABLE_FIND_PACKAGE_Snappy=ON
                   -DFOLLY_HAVE_UNALIGNED_ACCESS_EXITCODE=0 -DFOLLY_HAVE_WEAK_SYMBOLS_EXITCODE=0
                   -DFOLLY_HAVE_LINUX_VDSO_EXITCODE=1 -DFOLLY_HAVE_WCHAR_SUPPORT_EXITCODE=0
                   -DHAVE_VSNPRINTF_ERRORS_EXITCODE=1 )
