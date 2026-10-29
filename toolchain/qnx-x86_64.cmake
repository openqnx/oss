# QNX SDP 8.0 CMake toolchain -- x86_64
# Follows qnx-ports/build-files conventions (qcc/q++ with -V<target>).
set(QNX TRUE)
set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_VERSION 8.0.0)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(QNX_ARCH   "x86_64"    CACHE STRING "QNX cpuvardir")
set(QNX_HOST   "$ENV{QNX_HOST}")
set(QNX_TARGET "$ENV{QNX_TARGET}")
set(QNX_STAGE  "$ENV{STAGE}")
set(QNX_PREFIX "$ENV{PREFIX}")

set(CMAKE_C_COMPILER   "${QNX_HOST}/usr/bin/qcc")
set(CMAKE_CXX_COMPILER "${QNX_HOST}/usr/bin/q++")
set(CMAKE_C_COMPILER_TARGET   gcc_ntox86_64)
set(CMAKE_CXX_COMPILER_TARGET gcc_ntox86_64)
# Assemble with the cross-GCC driver, not qcc: qcc accepts -march on its
# command line but does not forward it to the assembler, so .S files that
# need an ISA extension (ONNX Runtime's MLAS FP16 NEON kernels ask for
# -march=armv8.2-a+fp16) fail with "selected processor does not support".
set(CMAKE_ASM_COMPILER "${QNX_HOST}/usr/bin/x86_64-pc-nto-qnx8.0.0-gcc")

set(CMAKE_AR      "${QNX_HOST}/usr/bin/ntox86_64-ar"      CACHE FILEPATH "ar")
set(CMAKE_RANLIB  "${QNX_HOST}/usr/bin/ntox86_64-ranlib"  CACHE FILEPATH "ranlib")
set(CMAKE_NM      "${QNX_HOST}/usr/bin/ntox86_64-nm"      CACHE FILEPATH "nm")
set(CMAKE_OBJCOPY "${QNX_HOST}/usr/bin/ntox86_64-objcopy" CACHE FILEPATH "objcopy")
set(CMAKE_OBJDUMP "${QNX_HOST}/usr/bin/ntox86_64-objdump" CACHE FILEPATH "objdump")
set(CMAKE_STRIP   "${QNX_HOST}/usr/bin/ntox86_64-strip"   CACHE FILEPATH "strip")

# QNX splits libc hard. Beyond libsocket and libm, the Linux-style event file
# descriptors each live in their own shim library: libeventfd, libepoll,
# libsignalfd, libtimerfd. Portable code calls these without ever naming a
# library, so put them on every link line.
# The math/classification
# helpers (__isinf, __isnan, ...) in libm. Neither is linked by default,
# so put both on every link line.
# QNX 8.0's C++ runtime is libc++, which hard-errors below C++11
# ("libc++ only supports C++03 with Clang-based compilers"). CMake's
# check_cxx_source_compiles often clears CMAKE_REQUIRED_FLAGS, so any such probe
# runs at the compiler's default standard and fails for reasons unrelated to
# what it is testing -- Eigen's math-library check is one. Put the standard in
# CMAKE_CXX_FLAGS itself, which those probes always inherit.
# Headers installed by earlier packages must be on the default search path:
# CMake only adds them for targets that go through find_package + a linked
# target, and plenty of code just does #include <lz4.h>. qnx-ports' common.mk
# takes the same approach.
# -isystem, not -I: these flags are prepended to every compile, so with -I the
# staging headers outrank a project's own include directories. protobuf and
# gRPC each vendor their own abseil, and a rebuild then compiled their
# sources against whichever abseil headers happened to be installed --
# "IsNegative is not a member of numbers_internal". -isystem directories are
# searched after every -I, so the project always sees its own copy first.
set(CMAKE_C_FLAGS_INIT   "-isystem ${QNX_STAGE}${QNX_PREFIX}/include")
set(CMAKE_CXX_FLAGS_INIT "-std=gnu++17 -isystem ${QNX_STAGE}${QNX_PREFIX}/include")

set(CMAKE_C_STANDARD_LIBRARIES_INIT   "-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify")
# libc always needed. With --as-needed (Meson's default, and ROS's
# toolchain) a library that calls nothing in libc itself drops libc.so, and
# the libc functions libgcc_eh/crt still need are then taken from QNX's static
# libcS.a: the .so ends up EXPORTING its own copies of memchr, malloc, ... ,
# and the dynamic linker binds every later library's calls to them instead of
# libc's (32 such libraries: glib's libgthread, GStreamer plugins, ROS 2's
# typesupport libraries). scripts/check-embedded-libc audits for it.
set(CMAKE_EXE_LINKER_FLAGS_INIT    "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -Wl,--build-id=md5 -L${QNX_STAGE}${QNX_PREFIX}/lib -Wl,-rpath-link,${QNX_STAGE}${QNX_PREFIX}/lib -L${QNX_TARGET}/x86_64/usr/lib -L${QNX_TARGET}/x86_64/lib")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -Wl,--build-id=md5 -Wl,--allow-shlib-undefined -L${QNX_STAGE}${QNX_PREFIX}/lib -Wl,-rpath-link,${QNX_STAGE}${QNX_PREFIX}/lib -L${QNX_TARGET}/x86_64/usr/lib -L${QNX_TARGET}/x86_64/lib")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -Wl,--build-id=md5 -Wl,--allow-shlib-undefined -L${QNX_STAGE}${QNX_PREFIX}/lib -Wl,-rpath-link,${QNX_STAGE}${QNX_PREFIX}/lib -L${QNX_TARGET}/x86_64/usr/lib -L${QNX_TARGET}/x86_64/lib")

set(CMAKE_SYSROOT "${QNX_TARGET}")
# Staging first. With the SDP ahead of it, find_library(ZLIB) resolved to the
# SDP's prebuilt libz -- whose soname is libz.so.2 where upstream zlib builds
# libz.so.1 -- and the resulting binary loaded the SDP copy on the target no
# matter what LD_LIBRARY_PATH said. scripts/check-sdp-leak audits for this.
set(CMAKE_FIND_ROOT_PATH "${QNX_STAGE}${QNX_PREFIX};${QNX_STAGE};${QNX_TARGET};${QNX_TARGET}/x86_64")
set(CMAKE_PREFIX_PATH    "${QNX_STAGE}${QNX_PREFIX};${QNX_TARGET}/x86_64/usr")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# ...but staging still holds *target* executables, and CMAKE_PREFIX_PATH makes
# CMake search "<prefix>/bin" for host programs.  With PROGRAM mode NEVER there
# is no sysroot guard to stop it, so the moment pkgconf was staged,
# find_package(PkgConfig) matched stage/.../bin/pkgconf -- a QNX binary -- tried
# to run it for its version, failed, and reported PkgConfig *missing*.  Nothing
# named the real cause.  Never look for host programs in the staging tree.
list(APPEND CMAKE_IGNORE_PATH "${QNX_STAGE}${QNX_PREFIX}/bin"
                              "${QNX_STAGE}${QNX_PREFIX}/sbin")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# QNX libs/headers land under <target>/<arch>/usr, which is not a default search dir
include_directories(SYSTEM "${QNX_TARGET}/usr/include")
link_directories("${QNX_TARGET}/x86_64/usr/lib" "${QNX_TARGET}/x86_64/lib" "${QNX_STAGE}${QNX_PREFIX}/lib")

# --- QNX feature-test macros ---------------------------------------------
# QNX headers gate POSIX declarations behind __EXT_POSIX*/__EXT_QNX, which are
# disabled when the compiler runs in strict-ANSI mode (-std=c++17 rather than
# -std=gnu++17). Without this, ::stat/::fstat/::getpid/::isatty vanish and most
# portable POSIX code fails to compile. _QNX_SOURCE re-enables the full set.
add_compile_definitions(_QNX_SOURCE)
set(CMAKE_C_EXTENSIONS   ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_THREAD_LIBS_INIT "")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_PTHREADS_INIT 1)
