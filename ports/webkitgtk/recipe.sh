VERSION=2.44.4
# 2.44 is the last series that still renders with Cairo: 2.46 switched the GTK
# port to Skia, which brings its own graphics stack and hard-requires GBM.
TARBALL=https://webkitgtk.org/releases/webkitgtk-2.44.4.tar.xz
SHA256=2ce4ec1b78413035037aba8326b31ed72696626b7bea7bace5e46ac0d8cbe796
# Also linked at run time (found by scripts/gen-dependency-index): gst-plugins-bad lcms2 libavif libjxl libsecret
DEPS="gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu
      libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt
      libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland
      wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret"
BUILDSYS=cmake
# What QNX cannot provide, and why each is off:
#   USE_GBM / USE_LIBDRM  - no DRM/KMS on QNX; these are the buffer-sharing path
#                           for accelerated compositing between processes.
#   ENABLE_WEBGL          - pulls in ANGLE and a real GL driver.
#   BUBBLEWRAP_SANDBOX    - Linux namespaces and seccomp.
#   ENABLE_GAMEPAD        - libmanette, which is udev-based.
#   JOURNALD_LOG          - systemd.
#   ENABLE_INTROSPECTION  - would have to run target binaries.
# GL entry points come through libepoxy, which resolves them at run time, so no
# GL library is linked here.
CMAKE_ARGS_EXTRA=( -DPORT=GTK -DUSE_GTK4=OFF
                   -DENABLE_JIT=OFF -DENABLE_C_LOOP=ON -DENABLE_FTL_JIT=OFF
                   -DENABLE_SAMPLING_PROFILER=OFF -DENABLE_WEBASSEMBLY=OFF
                   -DUSE_GBM=OFF -DUSE_LIBDRM=OFF -DENABLE_WEBGL=OFF
                   -DENABLE_BUBBLEWRAP_SANDBOX=OFF -DENABLE_GAMEPAD=OFF
                   -DENABLE_JOURNALD_LOG=OFF -DENABLE_INTROSPECTION=OFF
                   -DENABLE_DOCUMENTATION=OFF -DENABLE_MINIBROWSER=OFF
                   -DENABLE_API_TESTS=OFF -DENABLE_SPELLCHECK=OFF
                   -DUSE_LIBHYPHEN=ON -DUSE_WOFF2=ON -DUSE_SYSTEM_MALLOC=ON
                   -DENABLE_VIDEO=ON -DENABLE_WEB_AUDIO=ON
                   # GStreamer's GL elements need EGL/GLES and libgstgl, which
                   # this tree has no way to build; media decodes into system
                   # memory instead of GL textures.
                   -DUSE_GSTREAMER_GL=OFF
                   # QNX ships its own <backtrace.h> -- its native backtrace API,
                   # nothing to do with GCC's libbacktrace -- so WebKit's
                   # FindLibBacktrace matches the header name and enables a
                   # library that is not there: StackTrace.cpp then fails on
                   # backtrace_create_state/backtrace_pcinfo/backtrace_syminfo.
                   -DUSE_LIBBACKTRACE=OFF
                   # QNX's only EGL belongs to Screen, and it types
                   # EGLNativeDisplayType as an integer handle.  WebKit's X11 and
                   # Wayland display backends pass a Display*/wl_display* to it:
                   #   invalid conversion from 'Display*' to 'EGLNativeDisplayType'
                   # There is no Mesa here to provide an X11- or Wayland-bound EGL,
                   # so those two windowing targets cannot work.  Everything else
                   # -- the engine, DOM, layout, networking, media -- does not
                   # depend on them.  A browser window would need a PlatformDisplay
                   # backend written against QNX Screen, which is new development,
                   # not a port.
                   -DENABLE_X11_TARGET=OFF -DENABLE_WAYLAND_TARGET=OFF
                   # gtest's death-test regex uses regcomp/regexec/regfree, which on
                   # QNX live in libregex, not libc.  OptionsJSCOnly.cmake forces
                   # ENABLE_API_TESTS back on for every non-Windows port, so gtest is
                   # built whatever -DENABLE_API_TESTS says.  The list repeats the
                   # toolchain's default because a -D replaces it rather than adding.
                   -DCMAKE_CXX_STANDARD_LIBRARIES="-lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify -lregex"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_CXX_STANDARD=20 )
