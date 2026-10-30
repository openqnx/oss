# Port verification tests -- one QNX binary per ported package.
#
# Each test links against the libraries in stage/<arch>/usr/local and exercises
# the package's core API. Building them proves headers, libraries and link lines
# are usable by a downstream consumer; running them on a QNX target proves the
# ports actually work. Cross-built binaries cannot execute on the build host.
cmake_minimum_required(VERSION 3.16)
project(qnx_port_tests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(STAGE "$ENV{STAGE}$ENV{PREFIX}" CACHE PATH "staging prefix")
set(INC "${STAGE}/include")
set(LIB "${STAGE}/lib")

# Tests find their headers and libraries the same way an application would.
include_directories(SYSTEM "${INC}" "${CMAKE_CURRENT_SOURCE_DIR}")
link_directories("${LIB}")

set(QNX_TESTS "" CACHE INTERNAL "")
function(port_test name src)
  add_executable(${name} "src/${src}")
  target_link_libraries(${name} PRIVATE ${ARGN})
  # Tests are staged next to each other so one directory can be copied to target.
  install(TARGETS ${name} RUNTIME DESTINATION bin/qnx-port-tests)
  set(QNX_TESTS "${QNX_TESTS};${name}" CACHE INTERNAL "")
endfunction()

# ---- foundation -----------------------------------------------------------
port_test(test_zlib             test_zlib.c              z)
port_test(test_lz4              test_lz4.c               lz4)
port_test(test_sqlite           test_sqlite.c            sqlite3)
port_test(test_libpcap          test_libpcap.c           pcap)
port_test(test_nlohmann_json    test_nlohmann_json.cpp)
port_test(test_fmt              test_fmt.cpp             fmt)
port_test(test_spdlog           test_spdlog.cpp          spdlog fmt)
# spdlog was built with SPDLOG_FMT_EXTERNAL=ON, so consumers must say so too --
# otherwise its headers reach for the bundled fmt copy that was not installed.
target_compile_definitions(test_spdlog PRIVATE SPDLOG_FMT_EXTERNAL SPDLOG_COMPILED_LIB)
port_test(test_yaml_cpp         test_yaml_cpp.cpp        yaml-cpp)
port_test(test_tinyxml2         test_tinyxml2.cpp        tinyxml2)
port_test(test_googletest       test_googletest.cpp      gtest gtest_main)
port_test(test_gflags           test_gflags.cpp          gflags)
port_test(test_glog             test_glog.cpp            glog gflags)
# glog 0.7 requires its consumers to opt in to the generated export header and
# to gflags support; the exported CMake target normally supplies both.
target_compile_definitions(test_glog PRIVATE GLOG_USE_GLOG_EXPORT GLOG_USE_GFLAGS)
port_test(test_foonathan_memory test_foonathan_memory.cpp foonathan_memory-0.7.3)
target_include_directories(test_foonathan_memory PRIVATE "${INC}/foonathan_memory")

# ---- security -------------------------------------------------------------
port_test(test_openssl          test_openssl.c           ssl crypto)
port_test(test_mbedtls          test_mbedtls.c           mbedcrypto)
port_test(test_wolfssl          test_wolfssl.c           wolfssl)
port_test(test_libsodium        test_libsodium.c         sodium)

# ---- boost ----------------------------------------------------------------
port_test(test_boost            test_boost.cpp
          boost_filesystem boost_thread boost_system boost_atomic
          boost_regex boost_date_time boost_chrono)

# The rest of Boost: the full set is built, so the test suite covers the parts
# most likely to break on a new platform -- Context's per-architecture assembly
# and Process, whose QNX backend is implemented in this workspace.
port_test(test_boost_extra      test_boost_extra.cpp
          boost_json boost_url boost_context boost_process boost_container
          boost_random boost_charconv boost_filesystem boost_system)

# ---- automotive middleware ------------------------------------------------
port_test(test_cyclonedds       test_cyclonedds.c        ddsc)
port_test(test_fastcdr          test_fastcdr.cpp         fastcdr)
port_test(test_fastdds          test_fastdds.cpp         fastrtps fastcdr)
port_test(test_vsomeip          test_vsomeip.cpp         vsomeip3)
port_test(test_capicxx          test_capicxx.cpp         CommonAPI CommonAPI-SomeIP vsomeip3)
target_include_directories(test_capicxx PRIVATE "${INC}/CommonAPI-3.2")
port_test(test_dlt              test_dlt.c               dlt)
port_test(test_lely_core        test_lely_core.c         lely-co lely-can lely-util)
# paho ships four variants: 3c/3cs are the synchronous MQTTClient_* API (plain
# and TLS), 3a/3as the asynchronous MQTTAsync_* one. The test drives both APIs.
port_test(test_paho_mqtt_c      test_paho_mqtt_c.c       paho-mqtt3cs paho-mqtt3as)
port_test(test_paho_mqtt_cpp    test_paho_mqtt_cpp.cpp   paho-mqttpp3 paho-mqtt3as)
port_test(test_libwebsockets    test_libwebsockets.c     websockets)
# protobuf and gRPC pull in dozens of Abseil archives transitively. Naming them
# by hand is both brittle and pointless: these two ship proper CMake package
# configs, so consume them the way an application would. That also verifies the
# installed *Config.cmake files are correct, which raw -l linking never would.
find_package(protobuf CONFIG REQUIRED)
port_test(test_protobuf         test_protobuf.cpp        protobuf::libprotobuf)

find_package(gRPC CONFIG REQUIRED)
port_test(test_grpc             test_grpc.cpp            gRPC::grpc++ gRPC::grpc)

# iceoryx installs its headers under a versioned directory.
port_test(test_iceoryx          test_iceoryx.cpp         iceoryx_hoofs iceoryx_platform)
target_include_directories(test_iceoryx PRIVATE "${INC}/iceoryx/v2.0.6")

# ---- perception / compute -------------------------------------------------
port_test(test_eigen            test_eigen.cpp)
target_include_directories(test_eigen PRIVATE "${INC}/eigen3")

port_test(test_ceres            test_ceres.cpp           ceres glog gflags)
target_compile_definitions(test_ceres PRIVATE GLOG_USE_GLOG_EXPORT GLOG_USE_GFLAGS)
target_include_directories(test_ceres PRIVATE "${INC}/eigen3")

port_test(test_opencv           test_opencv.cpp          opencv_core opencv_imgproc)
target_include_directories(test_opencv PRIVATE "${INC}/opencv4")

port_test(test_flann            test_flann.cpp           flann_cpp lz4)

port_test(test_pcl              test_pcl.cpp
          pcl_common pcl_filters pcl_kdtree pcl_search pcl_octree
          flann_cpp boost_filesystem boost_system)
target_include_directories(test_pcl PRIVATE "${INC}/pcl-1.14" "${INC}/eigen3")

port_test(test_onnxruntime      test_onnxruntime.cpp     onnxruntime)

# ---- text rendering stack ---------------------------------------------------
# ICU + FriBidi + HarfBuzz + cairo + Pango exercised together in one binary.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PANGOCAIRO REQUIRED IMPORTED_TARGET pangocairo)
pkg_check_modules(HARFBUZZ   REQUIRED IMPORTED_TARGET harfbuzz)
pkg_check_modules(FRIBIDI    REQUIRED IMPORTED_TARGET fribidi)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
port_test(test_gtk3             test_gtk3.c              PkgConfig::GTK3)

pkg_check_modules(WLCLIENT REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(WLSERVER REQUIRED IMPORTED_TARGET wayland-server)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
port_test(test_wayland          test_wayland.c
          PkgConfig::WLCLIENT PkgConfig::WLSERVER PkgConfig::XKBCOMMON)

port_test(test_textstack        test_textstack.c
          PkgConfig::PANGOCAIRO PkgConfig::HARFBUZZ PkgConfig::FRIBIDI
          icuuc icui18n)
target_include_directories(test_onnxruntime PRIVATE "${INC}/onnxruntime")

# ---- compression ----------------------------------------------------------
port_test(test_zstd             test_zstd.c              zstd)
port_test(test_xz               test_xz.c                lzma)
port_test(test_bzip2            test_bzip2.c             bz2)
port_test(test_brotli           test_brotli.c            brotlienc brotlidec brotlicommon)

# ---- hashing / serialisation ----------------------------------------------
port_test(test_xxhash           test_xxhash.c            xxhash)
port_test(test_utf8proc         test_utf8proc.c          utf8proc)
port_test(test_jansson          test_jansson.c           jansson)
port_test(test_jsoncpp          test_jsoncpp.cpp         jsoncpp)
port_test(test_simdjson         test_simdjson.cpp        simdjson)
port_test(test_cjson            test_cjson.c             cjson)
port_test(test_msgpack          test_msgpack.c           msgpack-c)
port_test(test_libconfig        test_libconfig.c         config)

# ---- text, markup and storage ---------------------------------------------
port_test(test_libarchive       test_libarchive.c        archive)
port_test(test_oniguruma        test_oniguruma.c         onig)
port_test(test_pcre2            test_pcre2.c             pcre2-8)
pkg_check_modules(LUA54 REQUIRED IMPORTED_TARGET lua5.4)
port_test(test_lua              test_lua.c               PkgConfig::LUA54)
port_test(test_expat            test_expat.c             expat)
port_test(test_libxml2          test_libxml2.c           xml2)
target_include_directories(test_libxml2 PRIVATE "${INC}/libxml2")
port_test(test_libxslt          test_libxslt.c           xslt xml2)
target_include_directories(test_libxslt PRIVATE "${INC}/libxml2")
port_test(test_gdbm             test_gdbm.c              gdbm)
port_test(test_libinih          test_libinih.c           inih)
port_test(test_tree_sitter      test_tree_sitter.c       tree-sitter)
port_test(test_ts_grammars      test_ts_grammars.c       tree-sitter)
port_test(test_monitors         test_monitors.c)
port_test(test_libmd            test_libmd.c             md)
port_test(test_miniz            test_miniz.c             miniz)
target_include_directories(test_miniz PRIVATE "${INC}/miniz")
port_test(test_capstone         test_capstone.c          capstone)
port_test(test_libaec           test_libaec.c            aec sz)
pkg_check_modules(OPUSFILE REQUIRED IMPORTED_TARGET opusfile ogg)
port_test(test_opusfile         test_opusfile.c          PkgConfig::OPUSFILE)
pkg_check_modules(LCM REQUIRED IMPORTED_TARGET lcm)
port_test(test_lcm              test_lcm.c               PkgConfig::LCM)
port_test(test_mimalloc         test_mimalloc.c          mimalloc)
target_include_directories(test_mimalloc PRIVATE "${INC}/mimalloc-3.1")
pkg_check_modules(PISTACHE REQUIRED IMPORTED_TARGET libpistache)
port_test(test_pistache         test_pistache.cpp        PkgConfig::PISTACHE)
port_test(test_mcap             test_mcap.cpp            zstd lz4)
pkg_check_modules(XSLT REQUIRED IMPORTED_TARGET libxslt libexslt libxml-2.0)
port_test(test_docbook_xsl      test_docbook_xsl.c       PkgConfig::XSLT)
port_test(test_ncnn             test_ncnn.cpp            ncnn)
target_include_directories(test_ncnn PRIVATE "${INC}/ncnn")
port_test(test_mujoco           test_mujoco.c            mujoco m)
pkg_check_modules(POPPLERGLIB REQUIRED IMPORTED_TARGET poppler-glib cairo-pdf)
port_test(test_poppler          test_poppler.c           PkgConfig::POPPLERGLIB)
# whisper.cpp in its own prefix (its own ggml): linked by path, run path to it.
port_test(test_whisper          test_whisper.c           "${STAGE}/opt/whisper.cpp/lib/libwhisper.so")
target_include_directories(test_whisper BEFORE PRIVATE "${STAGE}/opt/whisper.cpp/include")
target_link_options(test_whisper PRIVATE "-Wl,-rpath-link,${STAGE}/opt/whisper.cpp/lib")

# ---- crypto and internationalisation --------------------------------------
port_test(test_gmp              test_gmp.c               gmp)
port_test(test_nettle           test_nettle.c            nettle)
port_test(test_libtasn1         test_libtasn1.c          tasn1)
port_test(test_gnutls           test_gnutls.c            gnutls)
port_test(test_libgcrypt        test_libgcrypt.c         gcrypt gpg-error)
port_test(test_libunistring     test_libunistring.c      unistring)
port_test(test_libidn2          test_libidn2.c           idn2 unistring)
port_test(test_libpsl           test_libpsl.c            psl)

# ---- networking and event loops -------------------------------------------
port_test(test_curl             test_curl.c              curl)
port_test(test_nghttp2          test_nghttp2.c           nghttp2)
port_test(test_c_ares           test_c_ares.c            cares)
port_test(test_libuv            test_libuv.c             uv)
port_test(test_libev            test_libev.c             ev)
port_test(test_libevent         test_libevent.c          event event_core)
port_test(test_libssh2          test_libssh2.c           ssh2)
port_test(test_libusb           test_libusb.c            usb-1.0)

# ---- imaging ---------------------------------------------------------------
port_test(test_libpng           test_libpng.c            png16 z)
port_test(test_libjpeg          test_libjpeg.c           jpeg)
port_test(test_giflib           test_giflib.c            gif)
port_test(test_libwebp          test_libwebp.c           webp sharpyuv)
port_test(test_libtiff          test_libtiff.c           tiff)
port_test(test_libexif          test_libexif.c           exif)
port_test(test_lcms2            test_lcms2.c             lcms2)
port_test(test_libyuv           test_libyuv.cpp          yuv)
port_test(test_highway          test_highway.cpp         hwy)
port_test(test_pixman           test_pixman.c            pixman-1)
target_include_directories(test_pixman PRIVATE "${INC}/pixman-1")
port_test(test_freetype         test_freetype.c          freetype)
target_include_directories(test_freetype PRIVATE "${INC}/freetype2")
port_test(test_fontconfig       test_fontconfig.c        fontconfig)
port_test(test_graphite2        test_graphite2.c         graphite2)

# ---- audio -----------------------------------------------------------------
port_test(test_opus             test_opus.c              opus)
port_test(test_flac             test_flac.c              FLAC)
port_test(test_vorbis           test_vorbis.c            vorbisenc vorbis ogg)
port_test(test_theora           test_theora.c            theoraenc theoradec ogg)
port_test(test_lame             test_lame.c              mp3lame)
port_test(test_libsndfile       test_libsndfile.c        sndfile)
port_test(test_libsamplerate    test_libsamplerate.c     samplerate)

# ---- video codecs ----------------------------------------------------------
port_test(test_x264             test_x264.c              x264)
port_test(test_x265             test_x265.c              x265)
port_test(test_openh264         test_openh264.cpp          openh264)
port_test(test_libvpx           test_libvpx.c            vpx)
port_test(test_dav1d            test_dav1d.c             dav1d)
port_test(test_aom              test_aom.c               aom)
port_test(test_svt_av1          test_svt_av1.c           SvtAv1Enc)
target_include_directories(test_svt_av1 PRIVATE "${INC}/svt-av1")
port_test(test_fdk_aac          test_fdk_aac.c           fdk-aac)

# ---- the GLib stack ---------------------------------------------------------
pkg_check_modules(GLIB     REQUIRED IMPORTED_TARGET glib-2.0 gobject-2.0 gio-2.0)
port_test(test_glib             test_glib.c              PkgConfig::GLIB)
pkg_check_modules(JSONGLIB REQUIRED IMPORTED_TARGET json-glib-1.0)
port_test(test_json_glib        test_json_glib.c         PkgConfig::JSONGLIB)
pkg_check_modules(DBUS     REQUIRED IMPORTED_TARGET dbus-1)
port_test(test_dbus             test_dbus.c              PkgConfig::DBUS)
pkg_check_modules(SOUP     REQUIRED IMPORTED_TARGET libsoup-3.0)
port_test(test_libsoup          test_libsoup.c           PkgConfig::SOUP)
pkg_check_modules(PIXBUF   REQUIRED IMPORTED_TARGET gdk-pixbuf-2.0)
port_test(test_gdk_pixbuf       test_gdk_pixbuf.c        PkgConfig::PIXBUF)
pkg_check_modules(ATK      REQUIRED IMPORTED_TARGET atk)
port_test(test_atk              test_atk.c               PkgConfig::ATK)
port_test(test_libffi           test_libffi.c            ffi)

# ---- multimedia frameworks --------------------------------------------------
pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0)
port_test(test_gstreamer        test_gstreamer.c         PkgConfig::GST)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
                  libavcodec libavformat libavutil libswscale libswresample)
port_test(test_ffmpeg           test_ffmpeg.c            PkgConfig::FFMPEG)
pkg_check_modules(ORC REQUIRED IMPORTED_TARGET orc-0.4)
port_test(test_orc              test_orc.c               PkgConfig::ORC)
port_test(test_sdl2             test_sdl2.c              SDL2)

# ---- terminal and shell libraries -------------------------------------------
port_test(test_ncurses          test_ncurses.c           ncursesw)
target_include_directories(test_ncurses PRIVATE "${INC}/ncursesw")
port_test(test_readline         test_readline.c          readline ncursesw)
port_test(test_jq               test_jq.c                jq)

# ---- remaining imaging / text -----------------------------------------------
port_test(test_exiv2            test_exiv2.cpp           exiv2)
port_test(test_libjxl           test_libjxl.c            jxl jxl_threads)
port_test(test_icu              test_icu.cpp             icui18n icuuc icudata)

# ---- arbitrary precision, transcoding, fonts and loadable modules ----------
port_test(test_mpfr             test_mpfr.c              mpfr gmp)
port_test(test_mpc              test_mpc.c               mpc mpfr gmp)
port_test(test_simdutf          test_simdutf.cpp         simdutf)
port_test(test_woff2            test_woff2.cpp           woff2dec woff2enc woff2common brotlienc brotlidec brotlicommon)
port_test(test_openjpeg         test_openjpeg.c          openjp2)
port_test(test_ltdl             test_ltdl.c              ltdl)
pkg_check_modules(NOTIFY REQUIRED IMPORTED_TARGET libnotify)
port_test(test_libnotify        test_libnotify.c         PkgConfig::NOTIFY)

# ---- text segmentation, terminals and scripting ----------------------------
port_test(test_libdatrie        test_libdatrie.c         datrie)
port_test(test_libthai          test_libthai.c           thai datrie)
port_test(test_libunibreak      test_libunibreak.c       unibreak)
port_test(test_libedit          test_libedit.c           edit ncursesw)
port_test(test_unibilium        test_unibilium.c         unibilium)
port_test(test_duktape          test_duktape.c           duktape)

# ---- remaining media and system libraries ----------------------------------
port_test(test_libavif          test_libavif.c           avif)
port_test(test_soxr             test_soxr.c              soxr)
port_test(test_zimg             test_zimg.c              zimg)
pkg_check_modules(ASS REQUIRED IMPORTED_TARGET libass)
port_test(test_libass           test_libass.c            PkgConfig::ASS)
port_test(test_libfdt           test_libfdt.c            fdt)
port_test(test_p11kit           test_p11kit.c            p11-kit)
target_include_directories(test_p11kit PRIVATE "${INC}/p11-kit-1")

# ---- optional stacks -------------------------------------------------------
# Guarded so the suite still configures before these are staged: a port_test()
# for a package that is not built yet would fail the whole run.
if(EXISTS "${INC}/X11/Xlib.h" AND EXISTS "${INC}/xcb/xcb.h")
  pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11 xcb xau)
  port_test(test_x11            test_x11.c               PkgConfig::X11)
else()
  message(STATUS "skipping test_x11: the X11 client stack is not staged")
endif()

if(EXISTS "${INC}/python3.14/Python.h")
  port_test(test_python         test_python.c            python3.14)
  target_include_directories(test_python PRIVATE "${INC}/python3.14")
else()
  message(STATUS "skipping test_python: CPython is not staged")
endif()

# ---- GTK4, Xfce and the rest of the desktop tier ---------------------------
pkg_check_modules(GRAPHENE REQUIRED IMPORTED_TARGET graphene-gobject-1.0)
port_test(test_graphene         test_graphene.c          PkgConfig::GRAPHENE)

pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
port_test(test_gtk4             test_gtk4.c              PkgConfig::GTK4)

pkg_check_modules(SECRET REQUIRED IMPORTED_TARGET libsecret-1)
port_test(test_libsecret        test_libsecret.c         PkgConfig::SECRET)

pkg_check_modules(XFCE REQUIRED IMPORTED_TARGET
                  libxfce4util-1.0 libxfce4ui-2 libxfconf-0 garcon-1)
port_test(test_xfce             test_xfce.c              PkgConfig::XFCE)

port_test(test_sdl2_ext         test_sdl2_ext.c          SDL2_image SDL2_ttf SDL2_net SDL2)
port_test(test_thorvg           test_thorvg.c            thorvg)

pkg_check_modules(GSF REQUIRED IMPORTED_TARGET libgsf-1)
port_test(test_misc_libs        test_misc_libs.c         PkgConfig::GSF hyphen enca gsm)

pkg_check_modules(ENCHANT REQUIRED IMPORTED_TARGET enchant-2)
port_test(test_enchant          test_enchant.c           PkgConfig::ENCHANT)

# SDL3 through its CMake package, not its .pc: sdl3.pc lists "alsa" under
# Requires.private, and QNX's alsa-lib is part of the OS -- it ships no
# pkgconfig directory, so pkg-config cannot resolve it and declares sdl3
# missing.  SDL3Config.cmake has no such indirection.
find_package(SDL3 REQUIRED CONFIG)
pkg_check_modules(NEWSTACK REQUIRED IMPORTED_TARGET libde265 atspi-2)
port_test(test_newstack         test_newstack.c          PkgConfig::NEWSTACK SDL3::SDL3)

pkg_check_modules(VTE REQUIRED IMPORTED_TARGET vte-2.91 libpcre2-8)
port_test(test_vte              test_vte.c               PkgConfig::VTE)

pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)
find_library(LUV_LIB NAMES luv REQUIRED)
port_test(test_luajit           test_luajit.c            PkgConfig::LUAJIT ${LUV_LIB})

pkg_check_modules(WEBKIT REQUIRED IMPORTED_TARGET webkit2gtk-4.1)
port_test(test_webkit           test_webkit.c            PkgConfig::WEBKIT)

pkg_check_modules(JSC REQUIRED IMPORTED_TARGET javascriptcore)
port_test(test_javascriptcore   test_javascriptcore.c    PkgConfig::JSC)

pkg_check_modules(XFCEAPPS REQUIRED IMPORTED_TARGET
                  gtk+-x11-3.0 cairo-xlib libxfce4panel-2.0 thunarx-3)
port_test(test_xfce_apps        test_xfce_apps.c         PkgConfig::XFCEAPPS)

# ---- packages that nothing else exercised ---------------------------------
# These were found by scripts/check-test-coverage, which reconstructs which
# staged file came from which package and then looks for a test that links,
# includes or runs it.
pkg_check_modules(EPOXY REQUIRED IMPORTED_TARGET epoxy)
port_test(test_libepoxy         test_libepoxy.c          PkgConfig::EPOXY)

pkg_check_modules(HANDY REQUIRED IMPORTED_TARGET libhandy-1)
port_test(test_libhandy         test_libhandy.c          PkgConfig::HANDY)

pkg_check_modules(LAYERSHELL REQUIRED IMPORTED_TARGET gtk-layer-shell-0)
port_test(test_gtk_layer_shell  test_gtk_layer_shell.c   PkgConfig::LAYERSHELL PkgConfig::GTK3)

# The compositor stack of the desktop: wlroots (headless backend, pixman
# rendering, read back), labwc running with swaybg on it, libsfdo.
# wlroots-0.20.pc requires libdrm, whose .pc the wlroots build makes for the
# SDP's libdrm (scripts/lib/sdp-pc.sh) in the staging tree's .toolchain/.
set(ENV{PKG_CONFIG_LIBDIR} "$ENV{STAGE}/.toolchain/sdp-pkgconfig:$ENV{PKG_CONFIG_LIBDIR}")
pkg_check_modules(WLROOTS REQUIRED IMPORTED_TARGET wlroots-0.20)
port_test(test_wlroots          test_wlroots.c           PkgConfig::WLROOTS PkgConfig::WLSERVER)
target_include_directories(test_wlroots PRIVATE "${INC}/libdrm-uapi")
port_test(test_labwc            test_labwc.c             PkgConfig::WLCLIENT)
pkg_check_modules(SFDO REQUIRED IMPORTED_TARGET libsfdo-desktop libsfdo-basedir)
port_test(test_libsfdo          test_libsfdo.c           PkgConfig::SFDO)

# libunwind 1.8.3 in its own prefix (the SDP's 1.8.2 is the system one): the
# test names it by path, so the SDP's cannot stand in for it.
# The X.Org libraries that work without an X server (one test, known answers).
pkg_check_modules(XORGLIBS REQUIRED IMPORTED_TARGET libxcvt xshmfence xcb-util xmu xt ice sm xkbfile xfont2 x11)
port_test(test_xorg_libs        test_xorg_libs.c         PkgConfig::XORGLIBS)

port_test(test_libunwind        test_libunwind.c         "${STAGE}/opt/libunwind/lib/libunwind.so")
target_include_directories(test_libunwind BEFORE PRIVATE "${STAGE}/opt/libunwind/include")
target_compile_options(test_libunwind PRIVATE -fno-omit-frame-pointer -O0)

pkg_check_modules(VIDSTAB REQUIRED IMPORTED_TARGET vidstab)
port_test(test_vidstab          test_vidstab.c           PkgConfig::VIDSTAB)

# xvidcore ships no pkg-config file; it is linked by name.
port_test(test_xvidcore         test_xvidcore.c          xvidcore)

port_test(test_gst_plugins_good test_gst_plugins_good.c  PkgConfig::GST)

# ---- added in openqnx/oss ---------------------------------------------------
# Data formats and serialisation
port_test(test_libyaml          test_libyaml.c           yaml)
port_test(test_pugixml          test_pugixml.cpp         pugixml)
port_test(test_rapidjson        test_rapidjson.cpp)
port_test(test_json_c           test_json_c.c            json-c)
port_test(test_flatbuffers      test_flatbuffers.cpp     flatbuffers)
port_test(test_protobuf_c       test_protobuf_c.c        protobuf-c)
port_test(test_tomlplusplus     test_tomlplusplus.cpp)
port_test(test_cli11            test_cli11.cpp)
port_test(test_libcbor          test_libcbor.c           cbor)
port_test(test_uriparser        test_uriparser.c         uriparser)
port_test(test_cereal           test_cereal.cpp)
port_test(test_magic_enum       test_magic_enum.cpp)
port_test(test_glm              test_glm.cpp)
port_test(test_double_conversion test_double_conversion.cpp double-conversion)
pkg_check_modules(RE2 REQUIRED IMPORTED_TARGET re2)
port_test(test_re2              test_re2.cpp             PkgConfig::RE2)

# Messaging, industrial and web protocols
port_test(test_libzmq           test_libzmq.cpp          zmq)
port_test(test_nng              test_nng.c               nng)
port_test(test_zenoh_c          test_zenoh_c.c           zenohc)
port_test(test_mosquitto        test_mosquitto.c         mosquitto)
port_test(test_open62541        test_open62541.c         open62541 ssl crypto)
port_test(test_libcoap          test_libcoap.c           coap-3)
port_test(test_libmicrohttpd    test_libmicrohttpd.c     microhttpd)
port_test(test_civetweb         test_civetweb.c          civetweb ssl crypto)
port_test(test_libmodbus        test_libmodbus.c         modbus)
target_include_directories(test_libmodbus PRIVATE "${INC}/modbus")
port_test(test_libssh           test_libssh.c            ssh)
port_test(test_libgit2          test_libgit2.c           git2)

# Compression and storage
port_test(test_libdeflate       test_libdeflate.c        deflate)
port_test(test_snappy           test_snappy.cpp          snappy)
port_test(test_lzo              test_lzo.c               lzo2)
port_test(test_libzip           test_libzip.c            zip)
port_test(test_leveldb          test_leveldb.cpp         leveldb snappy)

# Scripting, audio, test frameworks
port_test(test_quickjs          test_quickjs.c           qjs)
port_test(test_speexdsp         test_speexdsp.c          speexdsp)
port_test(test_mpg123           test_mpg123.c            mpg123)
port_test(test_taglib           test_taglib.cpp          tag)
port_test(test_benchmark        test_benchmark.cpp       benchmark)
port_test(test_catch2           test_catch2.cpp          Catch2)

# Robotics
port_test(test_console_bridge   test_console_bridge.cpp  console_bridge)
port_test(test_orocos_kdl       test_orocos_kdl.cpp      orocos-kdl)
target_include_directories(test_orocos_kdl PRIVATE "${INC}/eigen3")
port_test(test_octomap          test_octomap.cpp         octomap octomath)
port_test(test_fcl              test_fcl.cpp             fcl ccd octomap octomath)
target_include_directories(test_fcl PRIVATE "${INC}/eigen3")

# Components in a prefix of their own ($PREFIX/opt/<name>, see
# docs/porting-guide.md): linked from there, found there at run time. Their
# headers go in with -I, ahead of the staging tree's -isystem: TFLite's
# generated schema accepts only the flatbuffers it carries (23.5.26), not the
# flatbuffers component ("Non-compatible flatbuffers version included").
set(OPT "${STAGE}/opt")
port_test(test_tflite           test_tflite.cpp          tensorflow-lite)
target_include_directories(test_tflite BEFORE PRIVATE "${OPT}/tensorflow-lite/include")
target_link_directories(test_tflite PRIVATE "${OPT}/tensorflow-lite/lib")
target_link_options(test_tflite PRIVATE "-Wl,-rpath-link,${OPT}/tensorflow-lite/lib")
port_test(test_torch            test_torch.cpp           torch torch_cpu c10)
target_include_directories(test_torch BEFORE PRIVATE "${OPT}/pytorch/include"
                           "${OPT}/pytorch/include/torch/csrc/api/include")
target_link_directories(test_torch PRIVATE "${OPT}/pytorch/lib")
target_link_options(test_torch PRIVATE "-Wl,-rpath-link,${OPT}/pytorch/lib")

# Qt 6 headers sit in one directory per module; Qt also insists its users are
# position independent ("Compile your code with -fPIC").
port_test(test_qt6              test_qt6.cpp             Qt6Gui Qt6Core)
target_include_directories(test_qt6 PRIVATE "${INC}/QtCore" "${INC}/QtGui")
set_target_properties(test_qt6 PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ROS 2 is checked with its own demo nodes; this test is a script that runs them.
install(PROGRAMS src/test_ros2.sh DESTINATION bin/qnx-port-tests RENAME test_ros2)
install(PROGRAMS src/test_xkbcomp.sh DESTINATION bin/qnx-port-tests RENAME test_xkbcomp)
install(PROGRAMS src/test_cli_tools.sh DESTINATION bin/qnx-port-tests RENAME test_cli_tools)
install(PROGRAMS src/test_pymods.sh DESTINATION bin/qnx-port-tests RENAME test_pymods)
install(PROGRAMS src/test_sdcpp.sh DESTINATION bin/qnx-port-tests RENAME test_sdcpp)
install(PROGRAMS src/test_openttd_sets.sh DESTINATION bin/qnx-port-tests RENAME test_openttd_sets)
install(PROGRAMS src/test_adwaita.sh DESTINATION bin/qnx-port-tests RENAME test_adwaita)
install(PROGRAMS src/test_perl_mozilla_ca.sh DESTINATION bin/qnx-port-tests RENAME test_perl_mozilla_ca)
install(PROGRAMS src/test_doxygen.sh DESTINATION bin/qnx-port-tests RENAME test_doxygen)
install(PROGRAMS src/test_graphicsmagick.sh DESTINATION bin/qnx-port-tests RENAME test_graphicsmagick)
set(QNX_TESTS "${QNX_TESTS};test_ros2" CACHE INTERNAL "")

# A manifest the on-target runner reads, so it never guesses at binary names.
# QNX_TESTS starts with a ";" from the first append, which would leave a blank
# first line in the manifest.
string(REGEX REPLACE "^;" "" _clean "${QNX_TESTS}")
string(REPLACE ";" "\n" _list "${_clean}")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/bin/tests.list" CONTENT "${_list}\n")
install(FILES "${CMAKE_BINARY_DIR}/bin/tests.list" DESTINATION bin/qnx-port-tests)
