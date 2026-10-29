# Dependencies

Two kinds, by whether anything else from this repository is needed:

| Kind | Meaning | Components |
|---|---|---|
| 🟢 **Standalone** | builds with the QNX SDP alone and runs on a stock QNX image -- port it on its own | **200** |
| 🔴 **Dependent** | needs other components built first (listed per component, in build order) | **257** |

Dependent components split further by what they need at run time (38 run on their own once built). Each component's README (`ports/<name>/README.md`) walks through porting it by hand from the upstream source -- download, QNX patches, configure, build, install, package, test -- with its dependencies first when it has any.

Every component is in exactly one of three classes. The class is computed by `scripts/gen-dependency-index` from the recipes *and* from the binaries the build installed (their `DT_NEEDED` entries), so it describes what actually happens on the target, not only what the recipe declares.

| Class | Build | Run on the target | Components |
|---|---|---|---|
| **[Standalone](#standalone)** | QNX SDP only | stock QNX image, nothing else | **200** |
| **[Runtime-standalone](#runtime-standalone)** | needs other components first | on its own | **38** |
| **[Dependent](#dependent)** | needs other components first | needs other components installed | **219** |

Libraries that ship with the QNX SDP target (libc, libm, libsocket, libregex, libc++, the eventfd/epoll shims, ...) are part of every QNX 8.0 system and are never counted as dependencies.

On a target, `oss-get install <component>` installs the run-time closure automatically; the *runtime* column below is what it pulls in.

Machine-readable: [`ports/dependencies.json`](ports/dependencies.json).

## Standalone

200 components. Clone the upstream source, apply the patches in `ports/<name>/patches/`, build with the QNX toolchain files -- nothing else from this repository has to be built first, and the result runs on a stock QNX 8.0 target.

✅ = built against an *empty* staging tree by `scripts/verify-standalone`, proving the build cannot have picked anything up from other components.

| Component | Version | Build system | Verified alone | Used by |
|---|---|---|---|---|
| [`abseil`](ports/abseil/) | 20240722.0 | cmake | aarch64le✅ x86_64✅ | 3 |
| [`ada`](ports/ada/) | 3.4.4 | cmake | – | 1 |
| [`adwaita-fonts`](ports/adwaita-fonts/) | 49.0 | custom | – | 0 |
| [`aom`](ports/aom/) | 3.12.1 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`asio`](ports/asio/) | 1.30.2 | custom | – | 1 |
| [`autoconf`](ports/autoconf/) | 2.72 | autotools | – | 0 |
| [`automake`](ports/automake/) | 1.18 | autotools | – | 0 |
| [`aws-c-common`](ports/aws-c-common/) | 0.12.6 | cmake | – | 5 |
| [`benchmark`](ports/benchmark/) | 1.9.0 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`brotli`](ports/brotli/) | 1.1.0 | cmake | aarch64le✅ x86_64✅ | 9 |
| [`busybox`](ports/busybox/) | 1.36.1 | custom | – | 0 |
| [`bzip2`](ports/bzip2/) | 1.0.8 | custom | aarch64le✅ x86_64✅ | 11 |
| [`c-ares`](ports/c-ares/) | 1.34.3 | cmake | aarch64le✅ x86_64✅ | 3 |
| [`ca-certificates`](ports/ca-certificates/) | 20261020 | custom | – | 0 |
| [`candb`](ports/candb/) | 0.1-git-20210625 | cmake | – | 0 |
| [`capicxx-core-runtime`](ports/capicxx-core-runtime/) | 3.2.4 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`catch2`](ports/catch2/) | 3.7.1 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`cereal`](ports/cereal/) | 1.3.2 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`check`](ports/check/) | 0.15.2 | cmake | – | 0 |
| [`chrony`](ports/chrony/) | 4.8-git-20260226 | custom | – | 0 |
| [`cjson`](ports/cjson/) | 1.7.18 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`clapack`](ports/clapack/) | 3.2.1 | cmake | – | 0 |
| [`cli11`](ports/cli11/) | 2.4.2 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`computelibrary`](ports/computelibrary/) | 53.0.0 | cmake | – | 0 |
| [`console_bridge`](ports/console_bridge/) | 1.0.2 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`cpuinfo`](ports/cpuinfo/) | 0-git-20240417 | cmake | – | 2 |
| [`csmith`](ports/csmith/) | 2.3.0 | cmake | – | 0 |
| [`cunit`](ports/cunit/) | 2.1-3 | custom | – | 0 |
| [`cyclonedds`](ports/cyclonedds/) | 0.10.5 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`dav1d`](ports/dav1d/) | 1.5.0 | meson | – | 2 |
| [`dnsmasq`](ports/dnsmasq/) | 2.93test4 | custom | – | 0 |
| [`double-conversion`](ports/double-conversion/) | 3.3.0 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`duktape`](ports/duktape/) | 2.7.0 | custom | aarch64le✅ x86_64✅ | 0 |
| [`eigen`](ports/eigen/) | 3.4.0 | cmake | aarch64le✅ x86_64✅ | 6 |
| [`enca`](ports/enca/) | 1.19 | autotools | aarch64le✅ x86_64✅ | 0 |
| [`encodings`](ports/encodings/) | 1.1.0 | autotools | – | 0 |
| [`expat`](ports/expat/) | 2.6.4 | cmake | aarch64le✅ x86_64✅ | 9 |
| [`fakeroot`](ports/fakeroot/) | 1.37.1.1 | autotools | – | 0 |
| [`farmhash`](ports/farmhash/) | 1.1-git-20190513 | autotools | – | 0 |
| [`fastcdr`](ports/fastcdr/) | 2.2.6 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`fdk-aac`](ports/fdk-aac/) | 2.0.3 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`flatbuffers`](ports/flatbuffers/) | 24.3.25 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`fmt`](ports/fmt/) | 11.2.0 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`font-cantarell`](ports/font-cantarell/) | 0.303.1 | custom | – | 0 |
| [`font-dejavu`](ports/font-dejavu/) | 2.37 | custom | – | 1 |
| [`font-hack`](ports/font-hack/) | 3.003 | custom | – | 0 |
| [`font-inter`](ports/font-inter/) | 4.1 | custom | – | 0 |
| [`font-liberation`](ports/font-liberation/) | 2.1.5 | custom | – | 0 |
| [`font-noto-emoji`](ports/font-noto-emoji/) | 2.048 | custom | – | 0 |
| [`font-noto-sans-cjk`](ports/font-noto-sans-cjk/) | 2.004 | custom | – | 0 |
| [`font-noto-serif-cjk`](ports/font-noto-serif-cjk/) | 2.003 | custom | – | 0 |
| [`font-util`](ports/font-util/) | 1.4.1 | autotools | – | 0 |
| [`foonathan_memory`](ports/foonathan_memory/) | 0.7-3 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`fp16`](ports/fp16/) | 0-git-20260620 | cmake | – | 0 |
| [`freeimage`](ports/freeimage/) | 3.18.0 | custom | – | 1 |
| [`fribidi`](ports/fribidi/) | 1.0.16 | meson | – | 6 |
| [`fxdiv`](ports/fxdiv/) | 0-git-20200417 | cmake | – | 2 |
| [`g3log`](ports/g3log/) | 2.4 | cmake | – | 0 |
| [`gdbm`](ports/gdbm/) | 1.24 | autotools | aarch64le✅ x86_64✅ | 2 |
| [`gflags`](ports/gflags/) | 2.2.2 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`giflib`](ports/giflib/) | 5.2.2 | custom | aarch64le✅ x86_64✅ | 2 |
| [`gklib`](ports/gklib/) | 5.1.1-git-20230326 | cmake | – | 1 |
| [`glm`](ports/glm/) | 1.0.1 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`gmp`](ports/gmp/) | 6.3.0 | autotools | aarch64le✅ x86_64✅ | 6 |
| [`googletest`](ports/googletest/) | 1.17.0 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`gperf`](ports/gperf/) | 3.1 | autotools | – | 0 |
| [`graphite2`](ports/graphite2/) | 1.3.14 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`gsm`](ports/gsm/) | 1.0.22 | custom | aarch64le✅ x86_64✅ | 0 |
| [`hicolor-icon-theme`](ports/hicolor-icon-theme/) | 0.18 | meson | – | 1 |
| [`highway`](ports/highway/) | 1.2.0 | cmake | aarch64le✅ x86_64❌ | 1 |
| [`hyphen`](ports/hyphen/) | 2.8.8 | autotools | aarch64le✅ x86_64✅ | 1 |
| [`iceoryx`](ports/iceoryx/) | 2.0.6 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`icu`](ports/icu/) | 76.1 | custom | – | 6 |
| [`iso-codes`](ports/iso-codes/) | 4.17.0 | autotools | – | 1 |
| [`jansson`](ports/jansson/) | 2.14 | cmake | – | 0 |
| [`json-c`](ports/json-c/) | 0.18 | cmake | – | 0 |
| [`jsoncpp`](ports/jsoncpp/) | 1.9.6 | cmake | – | 0 |
| [`kineto`](ports/kineto/) | 0-git-20250214 | cmake | – | 0 |
| [`lame`](ports/lame/) | 3.100 | autotools | – | 2 |
| [`lely-core`](ports/lely-core/) | 2.3.4 | autotools | – | 0 |
| [`libcbor`](ports/libcbor/) | 0.11.0 | cmake | – | 0 |
| [`libccd`](ports/libccd/) | 2.1 | cmake | – | 1 |
| [`libconfig`](ports/libconfig/) | 1.8.1 | cmake | – | 0 |
| [`libdatrie`](ports/libdatrie/) | 0.2.13 | autotools | – | 1 |
| [`libde265`](ports/libde265/) | 1.0.15 | cmake | – | 0 |
| [`libdeflate`](ports/libdeflate/) | 1.22 | cmake | – | 0 |
| [`libdisplay-info`](ports/libdisplay-info/) | 0.3.0 | meson | – | 0 |
| [`libdrm-headers`](ports/libdrm-headers/) | 2.4.129 | custom | – | 1 |
| [`libev`](ports/libev/) | 4.33 | autotools | – | 0 |
| [`libexif`](ports/libexif/) | 0.6.24 | autotools | – | 1 |
| [`libfdt`](ports/libfdt/) | 1.7.2 | meson | – | 0 |
| [`libffi`](ports/libffi/) | 3.4.6 | autotools | – | 6 |
| [`libiconv`](ports/libiconv/) | 1.18 | autotools | – | 16 |
| [`libinih`](ports/libinih/) | 58 | meson | – | 1 |
| [`libjpeg-turbo`](ports/libjpeg-turbo/) | 3.0.4 | cmake | – | 13 |
| [`libjson`](ports/libjson/) | 0.8-git-20260512 | custom | – | 1 |
| [`libmetal`](ports/libmetal/) | 2024.10.0 | cmake | – | 1 |
| [`libmicrohttpd`](ports/libmicrohttpd/) | 1.0.1 | autotools | – | 0 |
| [`libmodbus`](ports/libmodbus/) | 3.1.11 | autotools | – | 0 |
| [`libogg`](ports/libogg/) | 1.3.5 | cmake | – | 7 |
| [`libpcap`](ports/libpcap/) | 1.10.5 | autotools | – | 0 |
| [`libpipeline`](ports/libpipeline/) | 1.5.8 | autotools | – | 1 |
| [`libretro-2048`](ports/libretro-2048/) | 0-git-20241227 | custom | – | 1 |
| [`libretro-fake08`](ports/libretro-fake08/) | 0-git-20260613 | custom | – | 1 |
| [`libretro-mrboom`](ports/libretro-mrboom/) | 0-git-20260625 | custom | – | 1 |
| [`libretro-retro8`](ports/libretro-retro8/) | 0-git-20260912 | custom | – | 1 |
| [`libretro-samples`](ports/libretro-samples/) | 0-git-20221110 | custom | – | 1 |
| [`libsamplerate`](ports/libsamplerate/) | 0.2.2 | cmake | – | 0 |
| [`libsfdo`](ports/libsfdo/) | 0.1.4 | meson | – | 1 |
| [`libsodium`](ports/libsodium/) | 1.0.20 | autotools | – | 2 |
| [`libtasn1`](ports/libtasn1/) | 4.19.0 | autotools | – | 3 |
| [`libtool`](ports/libtool/) | 2.5.4 | autotools | – | 0 |
| [`libunibreak`](ports/libunibreak/) | 6.1 | autotools | – | 0 |
| [`libunistring`](ports/libunistring/) | 1.3 | autotools | – | 4 |
| [`libusb`](ports/libusb/) | 1.0.27 | autotools | – | 1 |
| [`libuv`](ports/libuv/) | 1.51.0 | cmake | – | 4 |
| [`libvpx`](ports/libvpx/) | 1.15.0 | custom | – | 2 |
| [`libxcrypt`](ports/libxcrypt/) | 4.4.38 | autotools | – | 3 |
| [`libyaml`](ports/libyaml/) | 0.2.5 | cmake | – | 2 |
| [`linux-headers-input-codes`](ports/linux-headers-input-codes/) | 6.12 | custom | – | 1 |
| [`lmbench`](ports/lmbench/) | 3.0-git-20260929 | custom | – | 0 |
| [`lua`](ports/lua/) | 5.4.7 | custom | – | 0 |
| [`lua5.1`](ports/lua5.1/) | 5.1.5 | custom | – | 0 |
| [`lua5.3`](ports/lua5.3/) | 5.3.6 | custom | – | 0 |
| [`luajit`](ports/luajit/) | 2.1.c6ffc141 | custom | – | 4 |
| [`lxde-icon-theme`](ports/lxde-icon-theme/) | 0.5.2 | autotools | – | 1 |
| [`lz4`](ports/lz4/) | 1.10.0 | cmake | – | 6 |
| [`lzo`](ports/lzo/) | 2.10 | autotools | – | 2 |
| [`magic_enum`](ports/magic_enum/) | 0.9.6 | cmake | – | 0 |
| [`make`](ports/make/) | 4.4.1 | autotools | – | 0 |
| [`mbedtls`](ports/mbedtls/) | 3.6.2 | cmake | – | 0 |
| [`mpg123`](ports/mpg123/) | 1.32.9 | autotools | – | 0 |
| [`msgpack-c`](ports/msgpack-c/) | 6.1.0 | cmake | – | 0 |
| [`muslflt`](ports/muslflt/) | 1.0-git-20240802 | custom | – | 0 |
| [`nanosvg`](ports/nanosvg/) | 0-git-20260709 | cmake | – | 1 |
| [`ncurses`](ports/ncurses/) | 6.5 | autotools | – | 16 |
| [`ninja`](ports/ninja/) | 1.13.1 | cmake | – | 0 |
| [`nlohmann_json`](ports/nlohmann_json/) | 3.12.0 | cmake | – | 0 |
| [`nng`](ports/nng/) | 1.9.0 | cmake | – | 0 |
| [`octomap`](ports/octomap/) | 1.10.0 | cmake | – | 1 |
| [`onetbb`](ports/onetbb/) | 2022.0.0 | cmake | – | 0 |
| [`oniguruma`](ports/oniguruma/) | 6.9.9 | cmake | – | 1 |
| [`openal-soft`](ports/openal-soft/) | 1.24.1 | cmake | – | 0 |
| [`openblas`](ports/openblas/) | 0.3.28 | cmake | – | 3 |
| [`openh264`](ports/openh264/) | 2.5.0 | meson | – | 2 |
| [`openssl`](ports/openssl/) | 3.5.1 | custom | – | 41 |
| [`opus`](ports/opus/) | 1.5.2 | cmake | – | 4 |
| [`orc`](ports/orc/) | 0.4.40 | meson | – | 1 |
| [`pax-utils`](ports/pax-utils/) | 1.3.8 | meson | – | 0 |
| [`pcre`](ports/pcre/) | 8.45 | autotools | – | 0 |
| [`pdfium`](ports/pdfium/) | 0-git-84a8011 | custom | – | 0 |
| [`picocom`](ports/picocom/) | 3.1 | custom | – | 0 |
| [`pkgconf`](ports/pkgconf/) | 2.3.0 | meson | – | 0 |
| [`protobuf-c`](ports/protobuf-c/) | 1.5.0 | cmake | – | 0 |
| [`pthread-stubs`](ports/pthread-stubs/) | 0.5 | autotools | – | 1 |
| [`pugixml`](ports/pugixml/) | 1.14 | cmake | – | 1 |
| [`quickjs-ng`](ports/quickjs-ng/) | 0.8.0 | cmake | – | 0 |
| [`rapidjson`](ports/rapidjson/) | 1.1.0-24b5e7a | cmake | – | 1 |
| [`rust`](ports/rust/) | 1.90.0 | custom | – | 1 |
| [`simdjson`](ports/simdjson/) | 3.11.3 | cmake | – | 1 |
| [`simdutf`](ports/simdutf/) | 6.4.2 | cmake | – | 1 |
| [`sleef`](ports/sleef/) | 3.8-git-20250214 | cmake | – | 0 |
| [`snappy`](ports/snappy/) | 1.2.1 | cmake | – | 1 |
| [`soxr`](ports/soxr/) | 0.1.3 | cmake | – | 0 |
| [`speexdsp`](ports/speexdsp/) | 1.2.1 | autotools | – | 0 |
| [`svt-av1`](ports/svt-av1/) | 2.3.0 | cmake | – | 1 |
| [`tensorflow-lite`](ports/tensorflow-lite/) | 2.16.1 | custom | – | 0 |
| [`tinyxml2`](ports/tinyxml2/) | 10.0.0 | cmake | – | 2 |
| [`tomlplusplus`](ports/tomlplusplus/) | 3.4.0 | cmake | – | 0 |
| [`tree`](ports/tree/) | 2.2.1 | custom | – | 0 |
| [`tree-sitter`](ports/tree-sitter/) | 0.25.9 | cmake | – | 2 |
| [`tree-sitter-c`](ports/tree-sitter-c/) | 0.24.1 | custom | – | 0 |
| [`tree-sitter-lua`](ports/tree-sitter-lua/) | 0.4.0 | custom | – | 0 |
| [`tree-sitter-markdown`](ports/tree-sitter-markdown/) | 0.5.1 | custom | – | 0 |
| [`tree-sitter-query`](ports/tree-sitter-query/) | 0.7.0 | custom | – | 0 |
| [`tree-sitter-vim`](ports/tree-sitter-vim/) | 0.7.0 | custom | – | 0 |
| [`tree-sitter-vimdoc`](ports/tree-sitter-vimdoc/) | 4.0.0 | custom | – | 0 |
| [`trompeloeil`](ports/trompeloeil/) | 48 | cmake | – | 0 |
| [`unibilium`](ports/unibilium/) | 2.1.2 | cmake | – | 1 |
| [`urdfdom_headers`](ports/urdfdom_headers/) | 1.1.2 | cmake | – | 0 |
| [`uriparser`](ports/uriparser/) | 0.9.8 | cmake | – | 0 |
| [`utf8proc`](ports/utf8proc/) | 2.11.3 | cmake | – | 1 |
| [`vidstab`](ports/vidstab/) | 1.1.1 | cmake | – | 0 |
| [`webrtc`](ports/webrtc/) | m132-afaf497 | custom | – | 0 |
| [`wlr-protocols`](ports/wlr-protocols/) | 0-git-20250702 | custom | – | 0 |
| [`wolfssl`](ports/wolfssl/) | 5.7.6 | cmake | – | 0 |
| [`x264`](ports/x264/) | stable | custom | – | 2 |
| [`x265`](ports/x265/) | 3.6 | cmake | – | 2 |
| [`xcb-proto`](ports/xcb-proto/) | 1.17.0 | autotools | – | 1 |
| [`xdg-utils`](ports/xdg-utils/) | 1.2.1 | autotools | – | 0 |
| [`xkeyboard-config`](ports/xkeyboard-config/) | 2.43 | meson | – | 0 |
| [`xorgproto`](ports/xorgproto/) | 2024.1 | meson | – | 16 |
| [`xtrans`](ports/xtrans/) | 1.5.2 | autotools | – | 2 |
| [`xvidcore`](ports/xvidcore/) | 1.3.7 | custom | – | 0 |
| [`xxhash`](ports/xxhash/) | 0.8.2 | cmake | – | 2 |
| [`yaml-cpp`](ports/yaml-cpp/) | 0.8.0 | cmake | – | 1 |
| [`zimg`](ports/zimg/) | 3.0.5 | autotools | – | 0 |
| [`zlib`](ports/zlib/) | 1.3.1 | cmake | aarch64le✅ | 57 |
| [`zlib-ng`](ports/zlib-ng/) | 2.2.5 | cmake | – | 0 |
| [`zstd`](ports/zstd/) | 1.5.6 | cmake | – | 17 |

## Runtime-standalone

38 components. They need other components at *build* time -- header-only libraries (Eigen, nlohmann_json, ...), static archives, or generators -- but what they install loads nothing from this tree. Build the listed chain first (`qnxbuild` does it); deploy the component alone.

| Component | Version | Needs to build | Full build chain |
|---|---|---|---|
| [`amqp-cpp`](ports/amqp-cpp/) | 4.3.27 | openssl | 1 |
| [`cppzmq`](ports/cppzmq/) | 4.10.0 | libzmq | 2 |
| [`gsettings-desktop-schemas`](ports/gsettings-desktop-schemas/) | 49.0 | glib | 9 |
| [`itstool`](ports/itstool/) | 2.0.7 | python3 libxml2 | 16 |
| [`lcms2`](ports/lcms2/) | 2.16 | zlib | 1 |
| [`libcamera`](ports/libcamera/) | 0.5.1+rpt20250722 | libyaml | 1 |
| [`libepoxy`](ports/libepoxy/) | 1.5.10 | libx11 | 8 |
| [`libwebp`](ports/libwebp/) | 1.4.0 | libpng libjpeg-turbo giflib | 4 |
| [`libxau`](ports/libxau/) | 1.0.11 | xorgproto | 1 |
| [`libxdmcp`](ports/libxdmcp/) | 1.1.5 | xorgproto | 1 |
| [`lua-lpeg`](ports/lua-lpeg/) | 1.1.0 | luajit | 1 |
| [`meson`](ports/meson/) | 1.10.0 | python3 | 15 |
| [`nghttp2`](ports/nghttp2/) | 1.64.0 | zlib openssl | 2 |
| [`npm`](ports/npm/) | 11.11.0 | nodejs | 13 |
| [`open-amp`](ports/open-amp/) | 2024.10.0 | libmetal | 1 |
| [`openjpeg`](ports/openjpeg/) | 2.5.3 | zlib libpng libtiff lcms2 | 13 |
| [`orocos-kdl`](ports/orocos-kdl/) | 1.5.1 | eigen | 1 |
| [`oss-desktop`](ports/oss-desktop/) | 1.0 | labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu | 88 |
| [`pcre2`](ports/pcre2/) | 10.44 | zlib bzip2 | 2 |
| [`perl-error`](ports/perl-error/) | 0.17030 | perl | 2 |
| [`pixman`](ports/pixman/) | 0.46.4 | libpng | 2 |
| [`pthreadpool`](ports/pthreadpool/) | 0-git-20250207 | fxdiv | 1 |
| [`python3-dateutil`](ports/python3-dateutil/) | 2.9.0.post0 | python3 python3-six | 16 |
| [`python3-packaging`](ports/python3-packaging/) | 25.0 | python3 | 15 |
| [`python3-pip`](ports/python3-pip/) | 25.1.1 | python3 | 15 |
| [`python3-pytz`](ports/python3-pytz/) | 2025.2 | python3 | 15 |
| [`python3-setuptools`](ports/python3-setuptools/) | 80.8.0 | python3 | 15 |
| [`python3-six`](ports/python3-six/) | 1.17.0 | python3 | 15 |
| [`python3-tzdata`](ports/python3-tzdata/) | 2025.2 | python3 | 15 |
| [`python3-wheel`](ports/python3-wheel/) | 0.46.1 | python3 python3-packaging | 16 |
| [`readline`](ports/readline/) | 8.2 | ncurses | 1 |
| [`retropie`](ports/retropie/) | 0.3 | retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08 | 44 |
| [`ruy`](ports/ruy/) | 0-git-20240321 | cpuinfo | 1 |
| [`sdl2`](ports/sdl2/) | 2.32.10 | wayland libxkbcommon | 11 |
| [`sdl2-compat`](ports/sdl2-compat/) | 2.32.60 | sdl3 | 25 |
| [`sqlite_orm`](ports/sqlite_orm/) | 1.9 | sqlite | 2 |
| [`wayland-protocols`](ports/wayland-protocols/) | 1.47 | wayland | 10 |
| [`zenoh-c`](ports/zenoh-c/) | 1.2.1 | rust | 1 |

## Dependent

219 components. Both the build and the target need other components. *Runtime* is the set of components whose shared libraries these binaries load -- the packages `oss-get` installs with it.

| Component | Version | Needs to build | Needs at run time | Run-time closure |
|---|---|---|---|---|
| [`abuild`](ports/abuild/) | 3.16.0 | openssl zlib apk-tools | openssl zlib | 2 |
| [`apk-tools`](ports/apk-tools/) | 3.0.0_rc5 | openssl zlib zstd | openssl zlib zstd | 3 |
| [`aspell`](ports/aspell/) | 0.60.8.1 | ncurses | ncurses | 1 |
| [`at-spi2-core`](ports/at-spi2-core/) | 2.54.0 | glib dbus libxml2 libx11 libxtst | dbus gettext glib libx11 libxi libxtst | 17 |
| [`aws-c-auth`](ports/aws-c-auth/) | 0.10.1 | aws-c-http aws-c-sdkutils | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils openssl s2n-tls | 8 |
| [`aws-c-cal`](ports/aws-c-cal/) | 0.9.13 | aws-c-common openssl | aws-c-common openssl | 2 |
| [`aws-c-compression`](ports/aws-c-compression/) | 0.3.2 | aws-c-common | aws-c-common | 1 |
| [`aws-c-event-stream`](ports/aws-c-event-stream/) | 0.7.0 | aws-c-io aws-checksums | aws-c-cal aws-c-common aws-c-io aws-checksums openssl s2n-tls | 6 |
| [`aws-c-http`](ports/aws-c-http/) | 0.10.14 | aws-c-io aws-c-compression | aws-c-cal aws-c-common aws-c-compression aws-c-io openssl s2n-tls | 6 |
| [`aws-c-io`](ports/aws-c-io/) | 0.26.1 | aws-c-common aws-c-cal s2n-tls | aws-c-cal aws-c-common openssl s2n-tls | 4 |
| [`aws-c-mqtt`](ports/aws-c-mqtt/) | 0.15.2 | aws-c-http | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io openssl s2n-tls | 7 |
| [`aws-c-s3`](ports/aws-c-s3/) | 0.12.2 | aws-c-auth aws-checksums | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils aws-checksums openssl s2n-tls | 10 |
| [`aws-c-sdkutils`](ports/aws-c-sdkutils/) | 0.2.4 | aws-c-common | aws-c-common | 1 |
| [`aws-checksums`](ports/aws-checksums/) | 0.2.10 | aws-c-common | aws-c-common | 1 |
| [`aws-crt-cpp`](ports/aws-crt-cpp/) | 0.38.5 | aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3 | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-event-stream aws-c-http aws-c-io aws-c-mqtt aws-c-s3 aws-c-sdkutils aws-checksums openssl s2n-tls | 13 |
| [`azure-iot-sdk-c`](ports/azure-iot-sdk-c/) | LTS_08_2024 | openssl curl | curl openssl | 7 |
| [`bash`](ports/bash/) | 5.2.37 | ncurses readline libiconv | libiconv ncurses readline | 3 |
| [`bc`](ports/bc/) | 1.08.2 | readline | ncurses readline | 2 |
| [`binutils`](ports/binutils/) | 2.43 | zlib zstd | zlib zstd | 2 |
| [`boost`](ports/boost/) | 1.86.0 | zlib bzip2 xz | bzip2 xz zlib | 7 |
| [`cairo`](ports/cairo/) | 1.18.2 | pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo | fontconfig freetype glib libpng libx11 libxext libxrender lzo pixman zlib | 22 |
| [`capicxx-someip-runtime`](ports/capicxx-someip-runtime/) | 3.2.4 | capicxx-core-runtime vsomeip | capicxx-core-runtime vsomeip | 10 |
| [`capnproto`](ports/capnproto/) | 1.0.2 | zlib openssl | openssl zlib | 2 |
| [`ceres-solver`](ports/ceres-solver/) | 2.2.0 | eigen glog gflags | gflags glog | 2 |
| [`civetweb`](ports/civetweb/) | 1.16 | openssl | openssl | 1 |
| [`cmake`](ports/cmake/) | 4.1.1 | openssl zlib curl libuv | curl libuv openssl zlib | 8 |
| [`curl`](ports/curl/) | 8.11.1 | openssl zlib nghttp2 c-ares brotli zstd | brotli c-ares nghttp2 openssl zlib zstd | 6 |
| [`dbus`](ports/dbus/) | 1.16.0 | expat | expat | 1 |
| [`desktop-file-utils`](ports/desktop-file-utils/) | 0.28 | glib | gettext glib | 8 |
| [`dlt-daemon`](ports/dlt-daemon/) | 2.18.10 | zlib libjson | libjson | 1 |
| [`dump1090`](ports/dump1090/) | 0-git-20240121 | rtl-sdr | rtl-sdr | 2 |
| [`emacs`](ports/emacs/) | 30.2 | ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter | dbus gmp gnutls lcms2 libxml2 ncurses sqlite tree-sitter zlib | 19 |
| [`emulationstation`](ports/emulationstation/) | 2.11-git-20241212 | sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc | curl freeimage freetype pugixml sdl2 vlc | 20 |
| [`enchant`](ports/enchant/) | 2.8.2 | glib aspell | aspell gettext glib | 9 |
| [`epiphany`](ports/epiphany/) | 43.1 | webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle | at-spi2-core cairo gcr gdk-pixbuf gettext glib gmp gtk3 json-glib libarchive libdazzle libhandy libportal libsecret libsoup libxml2 nettle pango sqlite webkitgtk | 90 |
| [`exiv2`](ports/exiv2/) | 0.28.5 | zlib expat brotli libinih | brotli expat libinih zlib | 4 |
| [`exo`](ports/exo/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | 50 |
| [`fastdds`](ports/fastdds/) | 2.14.4 | fastcdr foonathan_memory tinyxml2 openssl | fastcdr foonathan_memory openssl tinyxml2 | 4 |
| [`fcl`](ports/fcl/) | 0.7.0 | eigen libccd octomap | libccd octomap | 2 |
| [`ffmpeg`](ports/ffmpeg/) | 7.1.1 | zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame | bzip2 dav1d lame libtheora libvorbis libvpx openh264 opus x264 x265 xz zlib | 17 |
| [`ffmpegthumbnailer`](ports/ffmpegthumbnailer/) | 2.2.3 | ffmpeg libpng libjpeg-turbo | ffmpeg libjpeg-turbo libpng zlib | 20 |
| [`file`](ports/file/) | 5.46 | zlib bzip2 xz zstd | bzip2 xz zlib zstd | 8 |
| [`flac`](ports/flac/) | 1.4.3 | libogg | libogg | 1 |
| [`flann`](ports/flann/) | 1.9.2 | lz4 | lz4 | 1 |
| [`fontconfig`](ports/fontconfig/) | 2.15.0 | freetype expat | expat freetype | 6 |
| [`fontenc`](ports/fontenc/) | 1.1.8 | xorgproto zlib | zlib | 1 |
| [`freetype`](ports/freetype/) | 2.13.3 | zlib bzip2 libpng brotli | brotli bzip2 libpng zlib | 4 |
| [`garcon`](ports/garcon/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | 50 |
| [`gawk`](ports/gawk/) | 5.3.2 | readline ncurses | ncurses readline | 2 |
| [`gcr`](ports/gcr/) | 3.41.2 | glib libgcrypt p11-kit libsecret gtk3 | cairo gdk-pixbuf gettext glib gtk3 libgcrypt p11-kit pango | 50 |
| [`gdb`](ports/gdb/) | 14.2 | gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd | expat gmp libiconv mpfr ncurses readline xxhash xz zlib zstd | 12 |
| [`gdk-pixbuf`](ports/gdk-pixbuf/) | 2.42.12 | glib libpng libjpeg-turbo | gettext glib libjpeg-turbo libpng | 10 |
| [`geany`](ports/geany/) | 2.1 | glib gtk3 | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango zlib | 47 |
| [`geany-plugins`](ports/geany-plugins/) | 2.1 | geany | at-spi2-core cairo gdk-pixbuf geany gettext glib gtk3 harfbuzz pango zlib | 48 |
| [`gettext`](ports/gettext/) | 0.22.5 | ncurses libiconv libunistring | libiconv libunistring ncurses | 3 |
| [`git`](ports/git/) | 2.51.0 | zlib openssl curl expat pcre2 perl perl-error libiconv | curl expat libiconv openssl pcre2 zlib | 10 |
| [`glib`](ports/glib/) | 2.82.4 | libffi pcre2 gettext zlib | gettext libffi libiconv pcre2 zlib | 7 |
| [`glib-networking`](ports/glib-networking/) | 2.80.1 | glib gnutls libproxy | gettext glib gnutls libproxy | 20 |
| [`glog`](ports/glog/) | 0.7.1 | gflags | gflags | 1 |
| [`gnutls`](ports/gnutls/) | 3.8.8 | nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd | brotli gmp libidn2 libtasn1 libunistring nettle zlib zstd | 11 |
| [`gobject-introspection`](ports/gobject-introspection/) | 1.86.0 | glib libffi python3 | glib libffi | 8 |
| [`godot`](ports/godot/) | 4.4.1 | freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon | brotli bzip2 freetype gettext glib graphite2 harfbuzz libiconv libogg libpng libtheora libvorbis libwebp libxkbcommon pcre2 zlib zstd | 23 |
| [`graphene`](ports/graphene/) | 1.10.8 | glib | glib | 8 |
| [`grep`](ports/grep/) | 3.11 | pcre2 libiconv | libiconv pcre2 | 2 |
| [`groff`](ports/groff/) | 1.23.0 | libiconv | libiconv | 1 |
| [`grpc`](ports/grpc/) | 1.69.0 | zlib openssl c-ares abseil protobuf re2 | abseil c-ares openssl protobuf re2 zlib | 6 |
| [`gst-libav`](ports/gst-libav/) | 1.24.10 | gst-plugins-base ffmpeg | ffmpeg glib gst-plugins-base gstreamer | 23 |
| [`gst-plugins-bad`](ports/gst-plugins-bad/) | 1.24.10 | gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo | aom cairo fdk-aac glib gst-plugins-base gstreamer libwebp openh264 pango svt-av1 x265 | 41 |
| [`gst-plugins-base`](ports/gst-plugins-base/) | 1.24.10 | gstreamer orc libogg libvorbis opus libtheora zlib | gettext glib gstreamer libogg libtheora libvorbis opus zlib | 13 |
| [`gst-plugins-good`](ports/gst-plugins-good/) | 1.24.10 | gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2 | bzip2 cairo flac gdk-pixbuf gettext glib gst-plugins-base gstreamer lame libjpeg-turbo libpng libsoup libvpx zlib | 39 |
| [`gst-plugins-ugly`](ports/gst-plugins-ugly/) | 1.24.10 | gst-plugins-base x264 | gettext glib gst-plugins-base gstreamer x264 | 15 |
| [`gstreamer`](ports/gstreamer/) | 1.24.10 | glib libxml2 | gettext glib | 8 |
| [`gthumb`](ports/gthumb/) | 3.12.8.2 | glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib | cairo exiv2 gdk-pixbuf gettext glib gtk3 lcms2 libjpeg-turbo libpng libtiff libwebp pango zlib | 53 |
| [`gtk-layer-shell`](ports/gtk-layer-shell/) | 0.9.0 | glib gtk3 wayland wayland-protocols | cairo glib gtk3 wayland | 47 |
| [`gtk3`](ports/gtk3/) | 3.24.43 | glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama | at-spi2-core cairo fontconfig fribidi gdk-pixbuf gettext glib harfbuzz libepoxy libx11 libxcomposite libxcursor libxdamage libxext libxfixes libxi libxinerama libxkbcommon libxrandr pango wayland | 46 |
| [`gtk4`](ports/gtk4/) | 4.16.7 | glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff | cairo fontconfig fribidi gdk-pixbuf gettext glib graphene harfbuzz libepoxy libjpeg-turbo libpng libtiff libxkbcommon pango wayland | 40 |
| [`gtsam`](ports/gtsam/) | 4.2 | boost eigen | boost | 8 |
| [`harfbuzz`](ports/harfbuzz/) | 10.1.0 | freetype glib icu graphite2 | freetype glib graphite2 icu | 14 |
| [`iperf3`](ports/iperf3/) | 3.17.1 | openssl | openssl | 1 |
| [`javascriptcore`](ports/javascriptcore/) | 2.44.4 | icu | icu | 1 |
| [`jq`](ports/jq/) | 1.7.1 | oniguruma | oniguruma | 1 |
| [`json-glib`](ports/json-glib/) | 1.10.6 | glib | gettext glib | 8 |
| [`labwc`](ports/labwc/) | 0.20.2 | wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng | cairo glib libpng libsfdo libxkbcommon libxml2 pango pixman wayland wlroots | 36 |
| [`less`](ports/less/) | 668 | ncurses | ncurses | 1 |
| [`leveldb`](ports/leveldb/) | 1.23 | snappy | snappy | 1 |
| [`libarchive`](ports/libarchive/) | 3.7.7 | zlib bzip2 xz zstd openssl libxml2 expat lz4 | bzip2 expat lz4 openssl xz zlib zstd | 11 |
| [`libass`](ports/libass/) | 0.17.3 | freetype fribidi harfbuzz fontconfig | fontconfig freetype fribidi harfbuzz | 18 |
| [`libavif`](ports/libavif/) | 1.1.1 | aom dav1d libyuv | aom dav1d | 2 |
| [`libcoap`](ports/libcoap/) | 4.3.5 | openssl | openssl | 1 |
| [`libdazzle`](ports/libdazzle/) | 3.44.0 | glib gtk3 | cairo gdk-pixbuf gettext glib gtk3 pango | 47 |
| [`libedit`](ports/libedit/) | 20240808-3.1 | ncurses | ncurses | 1 |
| [`libevent`](ports/libevent/) | 2.1.12 | openssl | openssl | 1 |
| [`libgcrypt`](ports/libgcrypt/) | 1.11.0 | libgpg-error | libgpg-error | 5 |
| [`libgit2`](ports/libgit2/) | 1.8.4 | openssl zlib pcre2 | openssl pcre2 zlib | 3 |
| [`libgpg-error`](ports/libgpg-error/) | 1.50 | gettext | gettext | 4 |
| [`libgsf`](ports/libgsf/) | 1.14.53 | glib libxml2 zlib bzip2 | bzip2 gettext glib libxml2 zlib | 11 |
| [`libhandy`](ports/libhandy/) | 1.8.3 | glib gtk3 | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gtk3 pango | 47 |
| [`libidn2`](ports/libidn2/) | 2.3.7 | libunistring gettext | gettext libunistring | 4 |
| [`libjxl`](ports/libjxl/) | 0.11.1 | brotli highway lcms2 libpng giflib libjpeg-turbo | brotli highway lcms2 | 3 |
| [`libnotify`](ports/libnotify/) | 0.8.3 | glib gdk-pixbuf | gdk-pixbuf glib | 11 |
| [`libpng`](ports/libpng/) | 1.6.44 | zlib | zlib | 1 |
| [`libportal`](ports/libportal/) | 0.8.1 | glib gtk3 | glib gtk3 | 47 |
| [`libproxy`](ports/libproxy/) | 0.5.9 | glib curl | curl glib | 14 |
| [`libpsl`](ports/libpsl/) | 0.21.5 | libidn2 libunistring libiconv | libiconv libidn2 libunistring | 5 |
| [`libsecret`](ports/libsecret/) | 0.21.4 | glib libgcrypt | glib libgcrypt | 10 |
| [`libsndfile`](ports/libsndfile/) | 1.2.2 | libogg libvorbis flac opus | flac libogg libvorbis opus | 4 |
| [`libsoup`](ports/libsoup/) | 3.6.1 | glib libxml2 sqlite nghttp2 libpsl brotli | brotli gettext glib libpsl nghttp2 sqlite zlib | 13 |
| [`libssh`](ports/libssh/) | 0.11.1 | openssl zlib | openssl zlib | 2 |
| [`libssh2`](ports/libssh2/) | 1.11.1 | openssl zlib | openssl zlib | 2 |
| [`libthai`](ports/libthai/) | 0.1.29 | libdatrie | libdatrie | 1 |
| [`libtheora`](ports/libtheora/) | 1.1.1 | libogg libvorbis | libogg | 1 |
| [`libtiff`](ports/libtiff/) | 4.7.0 | zlib xz zstd libjpeg-turbo libwebp | libjpeg-turbo libwebp xz zlib zstd | 9 |
| [`libvorbis`](ports/libvorbis/) | 1.3.7 | libogg | libogg | 1 |
| [`libwebsockets`](ports/libwebsockets/) | 4.3.5 | openssl zlib | openssl | 1 |
| [`libx11`](ports/libx11/) | 1.8.10 | libxcb xorgproto xtrans | libxcb | 3 |
| [`libxcb`](ports/libxcb/) | 1.17.0 | xcb-proto libxau libxdmcp xtrans pthread-stubs | libxau libxdmcp | 2 |
| [`libxcomposite`](ports/libxcomposite/) | 0.4.6 | libx11 libxfixes xorgproto | libx11 | 4 |
| [`libxcursor`](ports/libxcursor/) | 1.2.2 | libx11 libxrender libxfixes xorgproto | libx11 libxfixes libxrender | 6 |
| [`libxdamage`](ports/libxdamage/) | 1.1.6 | libx11 libxfixes xorgproto | libx11 libxfixes | 5 |
| [`libxext`](ports/libxext/) | 1.3.6 | libx11 xorgproto | libx11 | 4 |
| [`libxfce4ui`](ports/libxfce4ui/) | 4.20.0 | glib gtk3 libxfce4util xfconf libepoxy | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4util pango xfconf zlib | 49 |
| [`libxfce4util`](ports/libxfce4util/) | 4.20.0 | glib | gettext glib | 8 |
| [`libxfce4windowing`](ports/libxfce4windowing/) | 4.20.0 | glib gtk3 wayland wayland-protocols | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango wayland zlib | 47 |
| [`libxfixes`](ports/libxfixes/) | 6.0.1 | libx11 xorgproto | libx11 | 4 |
| [`libxft`](ports/libxft/) | 2.3.8 | libx11 libxrender freetype fontconfig xorgproto | fontconfig freetype libx11 libxrender | 12 |
| [`libxi`](ports/libxi/) | 1.8.2 | libx11 libxext libxfixes xorgproto | libx11 libxext | 5 |
| [`libxinerama`](ports/libxinerama/) | 1.1.5 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxkbcommon`](ports/libxkbcommon/) | 1.8.1 | libxml2 | libxml2 | 7 |
| [`libxml2`](ports/libxml2/) | 2.13.5 | zlib xz | xz zlib | 6 |
| [`libxrandr`](ports/libxrandr/) | 1.5.4 | libx11 libxext libxrender xorgproto | libx11 libxext libxrender | 6 |
| [`libxrender`](ports/libxrender/) | 0.9.11 | libx11 xorgproto | libx11 | 4 |
| [`libxscrnsaver`](ports/libxscrnsaver/) | 1.2.4 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxslt`](ports/libxslt/) | 1.1.42 | libxml2 zlib | libxml2 | 7 |
| [`libxtst`](ports/libxtst/) | 1.2.5 | libx11 libxext libxfixes libxi xorgproto | libx11 libxext libxi | 6 |
| [`libyuv`](ports/libyuv/) | stable | libjpeg-turbo | libjpeg-turbo | 1 |
| [`libzip`](ports/libzip/) | 1.11.1 | zlib bzip2 xz zstd openssl | bzip2 openssl xz zlib zstd | 9 |
| [`libzmq`](ports/libzmq/) | 4.3.5 | libsodium | libsodium | 1 |
| [`lighttpd`](ports/lighttpd/) | 1.4.77 | openssl pcre2 zlib | openssl pcre2 zlib | 3 |
| [`llvm`](ports/llvm/) | 22.1.8 | zlib zstd libffi libxml2 | libxml2 zlib zstd | 8 |
| [`luv`](ports/luv/) | 1.51.0-1 | libuv luajit | libuv | 1 |
| [`m4`](ports/m4/) | 1.4.19 | libiconv | libiconv | 1 |
| [`mame`](ports/mame/) | 0.281 | sdl2 sdl2-ttf fontconfig freetype zlib | fontconfig freetype sdl2 sdl2-ttf zlib | 19 |
| [`man-db`](ports/man-db/) | 2.13.1 | libpipeline gdbm groff libiconv zlib | gdbm libiconv libpipeline zlib | 4 |
| [`mate-calc`](ports/mate-calc/) | 1.28.0 | glib gtk3 libxml2 gmp mpfr mpc | at-spi2-core cairo gdk-pixbuf gettext glib gmp gtk3 harfbuzz libxml2 mpc mpfr pango zlib | 50 |
| [`metis`](ports/metis/) | 5.2.1 | gklib | gklib | 1 |
| [`minicom`](ports/minicom/) | 2.9 | ncurses libiconv | libiconv ncurses | 2 |
| [`mkfontscale`](ports/mkfontscale/) | 1.2.3 | freetype fontenc zlib | fontenc freetype zlib | 6 |
| [`mosquitto`](ports/mosquitto/) | 2.0.20 | openssl cjson | cjson openssl | 2 |
| [`mpc`](ports/mpc/) | 1.3.1 | gmp mpfr | gmp mpfr | 2 |
| [`mpfr`](ports/mpfr/) | 4.2.1 | gmp | gmp | 1 |
| [`nano`](ports/nano/) | 8.2 | ncurses zlib file | file ncurses zlib | 9 |
| [`neovim`](ports/neovim/) | 0.12.3 | luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext | gettext libiconv luv tree-sitter unibilium utf8proc | 9 |
| [`nettle`](ports/nettle/) | 3.10.1 | gmp | gmp | 1 |
| [`nodejs`](ports/nodejs/) | 24.14.1 | libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu | ada brotli c-ares icu libuv nghttp2 openssl simdjson simdutf sqlite zlib zstd | 12 |
| [`ntpsec`](ports/ntpsec/) | 1.2.4 | openssl | openssl | 1 |
| [`numpy`](ports/numpy/) | 2.3.4 | python3 openblas | openblas python3 | 15 |
| [`onnx`](ports/onnx/) | 1.17.0 | protobuf | abseil protobuf | 3 |
| [`onnxruntime`](ports/onnxruntime/) | 1.20.1 | protobuf | zlib | 1 |
| [`open62541`](ports/open62541/) | 1.4.8 | openssl | openssl | 1 |
| [`opencv`](ports/opencv/) | 4.10.0 | zlib eigen | zlib | 1 |
| [`openssh`](ports/openssh/) | 10.2p1 | openssl zlib libxcrypt | libxcrypt openssl zlib | 3 |
| [`openttd`](ports/openttd/) | 15.0-git-20250501 | sdl2 zlib xz libpng freetype fontconfig curl lzo | curl freetype libpng lzo sdl2 xz zlib | 17 |
| [`p11-kit`](ports/p11-kit/) | 0.25.5 | libffi libtasn1 | libffi | 1 |
| [`paho.mqtt.c`](ports/paho.mqtt.c/) | 1.3.14 | openssl | openssl | 1 |
| [`paho.mqtt.cpp`](ports/paho.mqtt.cpp/) | 1.5.2 | paho.mqtt.c | openssl paho.mqtt.c | 2 |
| [`pandas`](ports/pandas/) | 2.3.3 | numpy python3-dateutil python3-pytz python3-tzdata | python3 | 14 |
| [`pango`](ports/pango/) | 1.54.0 | glib harfbuzz fribidi cairo fontconfig freetype libxft | cairo fontconfig freetype fribidi glib harfbuzz libx11 libxft libxrender | 28 |
| [`pcl`](ports/pcl/) | 1.14.1 | boost eigen flann | boost flann lz4 | 10 |
| [`perl`](ports/perl/) | 5.40.2 | libxcrypt | libxcrypt | 1 |
| [`pqiv`](ports/pqiv/) | 2.13.2 | glib gtk3 gdk-pixbuf cairo | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libx11 pango zlib | 47 |
| [`protobuf`](ports/protobuf/) | 29.3 | zlib abseil | abseil zlib | 2 |
| [`python3`](ports/python3/) | 3.14.0 | zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd | bzip2 expat gdbm gettext libffi ncurses openssl sqlite xz zlib zstd | 13 |
| [`pytorch`](ports/pytorch/) | 2.3.1 | python3 protobuf openblas | abseil openblas protobuf | 4 |
| [`qt6`](ports/qt6/) | 6.8.1 | zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd | brotli double-conversion ffmpeg fontconfig freetype glib gst-plugins-bad gst-plugins-base gstreamer harfbuzz icu libjpeg-turbo libpng libtiff libwebp openssl pcre2 zlib zstd | 53 |
| [`re2`](ports/re2/) | 2024-07-02 | abseil | abseil | 1 |
| [`restbed`](ports/restbed/) | 4.8 | openssl asio | openssl | 1 |
| [`retroarch`](ports/retroarch/) | 1.15-git-20230408 | sdl2 freetype zlib fontconfig fribidi libass openssl | fontconfig freetype fribidi libass libxkbcommon openssl zlib | 23 |
| [`ros2`](ports/ros2/) | jazzy | python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2 | benchmark console_bridge curl cyclonedds fastcdr fastdds libyaml lz4 opencv python3 sqlite tinyxml2 yaml-cpp zstd | 29 |
| [`rsync`](ports/rsync/) | 3.3.0 | zstd lz4 xxhash openssl | lz4 openssl xxhash zstd | 4 |
| [`rtl-sdr`](ports/rtl-sdr/) | 2.0.2 | libusb | libusb | 1 |
| [`s2n-tls`](ports/s2n-tls/) | 1.7.0 | openssl | openssl | 1 |
| [`sdl2-image`](ports/sdl2-image/) | 2.8.8 | sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl | libavif libtiff libwebp sdl2 | 14 |
| [`sdl2-net`](ports/sdl2-net/) | 2.2.0 | sdl2 | sdl2 | 1 |
| [`sdl2-ttf`](ports/sdl2-ttf/) | 2.24.0 | sdl2 freetype harfbuzz | freetype harfbuzz sdl2 | 16 |
| [`sdl3`](ports/sdl3/) | 3.4.18 | libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst | libiconv libx11 libxcursor libxext libxfixes libxi libxrandr libxrender libxscrnsaver libxtst | 13 |
| [`shared-mime-info`](ports/shared-mime-info/) | 2.4 | glib libxml2 | glib libxml2 | 10 |
| [`spdlog`](ports/spdlog/) | 1.15.3 | fmt | fmt | 1 |
| [`sqlite`](ports/sqlite/) | 3.47.2 | zlib | zlib | 1 |
| [`sscep`](ports/sscep/) | 0.10.0 | openssl | openssl | 1 |
| [`strongswan`](ports/strongswan/) | 6.0.6 | openssl | openssl | 1 |
| [`sudo`](ports/sudo/) | 1.9.17p2 | libxcrypt openssl zlib | libxcrypt openssl zlib | 3 |
| [`suitesparse`](ports/suitesparse/) | 7.11.0 | openblas | openblas | 1 |
| [`swaybg`](ports/swaybg/) | 1.2.2 | wayland wayland-protocols cairo gdk-pixbuf glib | cairo gdk-pixbuf glib wayland | 26 |
| [`sysbench`](ports/sysbench/) | 1.0.20-git-20250309 | luajit | luajit | 1 |
| [`taglib`](ports/taglib/) | 2.0.2 | zlib | zlib | 1 |
| [`tar`](ports/tar/) | 1.35 | libiconv | libiconv | 1 |
| [`tftp-hpa`](ports/tftp-hpa/) | 5.2 | readline ncurses | ncurses readline | 2 |
| [`thorvg`](ports/thorvg/) | 0.15.8 | libjpeg-turbo libpng zlib | libjpeg-turbo libpng | 3 |
| [`thunar`](ports/thunar/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel | at-spi2-core cairo exo gdk-pixbuf gettext glib gtk3 harfbuzz libexif libnotify libx11 libxfce4ui libxfce4util pango pcre2 xfce4-panel xfconf zlib | 57 |
| [`tmux`](ports/tmux/) | 3.5a | libevent ncurses | libevent ncurses | 3 |
| [`tumbler`](ports/tumbler/) | 4.20.0 | glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer | curl ffmpegthumbnailer freetype gdk-pixbuf gettext glib gst-plugins-base gstreamer libgsf libjpeg-turbo libpng libxfce4util libxml2 | 37 |
| [`unbound`](ports/unbound/) | 1.24.1 | openssl expat | expat openssl | 2 |
| [`vim`](ports/vim/) | 9.1.0900 | ncurses libiconv libsodium | libiconv libsodium ncurses | 3 |
| [`vlc`](ports/vlc/) | 3.0.21 | bzip2 libiconv lz4 openssl xz zstd | bzip2 libiconv lz4 openssl xz zstd | 9 |
| [`vsomeip`](ports/vsomeip/) | 3.5.6 | boost | boost | 8 |
| [`vte`](ports/vte/) | 0.78.2 | gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4 | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gnutls graphene gtk3 gtk4 icu lz4 pango pcre2 | 58 |
| [`wayland`](ports/wayland/) | 1.24.0 | libffi expat libxml2 | libffi | 1 |
| [`webkitgtk`](ports/webkitgtk/) | 2.44.4 | gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret | at-spi2-core cairo fontconfig freetype gdk-pixbuf gettext glib gst-plugins-bad gst-plugins-base gstreamer gtk3 harfbuzz hyphen icu lcms2 libavif libepoxy libgcrypt libgpg-error libjpeg-turbo libjxl libpng libsecret libsoup libtasn1 libwebp libxml2 libxslt pango sqlite woff2 zlib | 77 |
| [`wlroots`](ports/wlroots/) | 0.20.2 | wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes | lcms2 libxkbcommon pixman wayland | 12 |
| [`woff2`](ports/woff2/) | 1.0.2 | brotli | brotli | 1 |
| [`xfce4-appfinder`](ports/xfce4-appfinder/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf garcon | cairo garcon gdk-pixbuf glib gtk3 libxfce4ui libxfce4util xfconf | 51 |
| [`xfce4-panel`](ports/xfce4-panel/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util libxfce4windowing pango wayland xfconf zlib | 54 |
| [`xfce4-session`](ports/xfce4-session/) | 4.20.2 | glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util libxfce4windowing pango xfconf | 52 |
| [`xfce4-settings`](ports/xfce4-settings/) | 4.20.1 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util pango wayland xfconf zlib | 53 |
| [`xfce4-terminal`](ports/xfce4-terminal/) | 1.1.5 | glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util pango pcre2 vte xfconf | 63 |
| [`xfconf`](ports/xfconf/) | 4.20.0 | glib libxfce4util dbus | gettext glib libxfce4util | 9 |
| [`xnnpack`](ports/xnnpack/) | 0-git-20250209 | cpuinfo pthreadpool fxdiv | pthreadpool | 1 |
| [`xz`](ports/xz/) | 5.6.3 | gettext | gettext | 4 |
| [`zsh`](ports/zsh/) | 5.9 | ncurses pcre2 libiconv | libiconv ncurses | 2 |

## Consistency

Every run-time dependency is declared in the recipe's `DEPS` (directly or transitively), and every `DT_NEEDED` resolves either to a component of this tree or to the QNX SDP.

## Blocked

1 components tried and not buildable for QNX as things stand (`STATUS=blocked`; each recipe's comment says what stops it): [`weston`](ports/weston/)

---
<sub>Generated by <code>scripts/gen-dependency-index</code>. Edit recipes, not this file.</sub>
