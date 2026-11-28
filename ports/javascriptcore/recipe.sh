VERSION=2.54.3
# WebKit's own release tarball; the JSCOnly port builds JavaScriptCore, WTF and
# bmalloc without any of the web engine or platform integration.  Worth having
# on its own -- a scripting engine for HMI code -- and it settles whether this
# codebase compiles for QNX at all before the GTK port's much longer build.
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.54.3.tar.xz
SHA256=2779c1b46f88762e8400650271ba14452dc4e30b37059981552d8b562b7d0862
DEPS="icu"
BUILDSYS=cmake
# -DENABLE_JIT=OFF / -DENABLE_C_LOOP=ON: the interpreter, not the JIT.  JSC's
# JIT needs W^X handling and signal-based OSR that are written per-OS; the
# CLoop interpreter is the portable path and is what a first port should prove.
# ENABLE_FTL_JIT and the sampling profiler follow the JIT off.
# ENABLE_STATIC_JSC=OFF keeps the shared library that consumers link.
CMAKE_ARGS_EXTRA=( -DPORT=JSCOnly -DENABLE_STATIC_JSC=OFF
                   -DENABLE_JIT=OFF -DENABLE_C_LOOP=ON -DENABLE_FTL_JIT=OFF
                   -DENABLE_SAMPLING_PROFILER=OFF
                   -DENABLE_WEBASSEMBLY=OFF
                   -DENABLE_API_TESTS=OFF -DENABLE_TOOLS=OFF
                   -DUSE_SYSTEM_MALLOC=ON
                   # gtest's death-test regex uses regcomp/regexec/regfree, which on
                   # QNX live in libregex, not libc.  OptionsJSCOnly.cmake forces
                   # ENABLE_API_TESTS back on for every non-Windows port, so gtest is
                   # built whatever -DENABLE_API_TESTS says.  The list repeats the
                   # toolchain's default because a -D replaces it rather than adding.
                   -DCMAKE_CXX_STANDARD_LIBRARIES="-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify -lregex"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# (No CMAKE_CXX_STANDARD here: OptionsCommon.cmake sets C++23 itself, and
# WebKitCommon.cmake requires GCC >= 12.2 -- exactly the SDP's compiler.)

# JSCOnly installs the library and nothing else: the public C API headers are
# generated into the build tree and left there, so a consumer has no way to
# include <JavaScriptCore/JavaScript.h>.  Install them, and write the .pc file
# the port does not provide either.
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -d "$STAGE$PREFIX/include/JavaScriptCore" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/JavaScriptCore/Headers/JavaScriptCore/*.h \
                "$STAGE$PREFIX/include/JavaScriptCore/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/javascriptcore.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: JavaScriptCore
Description: WebKit's JavaScript engine (JSCOnly port, CLoop interpreter)
Version: $VERSION
Libs: -L\${libdir} -lJavaScriptCore
Cflags: -I\${includedir}
PC
}

