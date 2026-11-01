# Dependencies

Two kinds, by whether anything else from this repository is needed:

| Kind | Meaning | Components |
|---|---|---|
| 🟢 **Standalone** | builds with the QNX SDP alone and runs on a stock QNX image -- port it on its own | **271** |
| 🔴 **Dependent** | needs other components built first (listed per component, in build order) | **404** |

Dependent components split further by what they need at run time (110 run on their own once built). Each component's README (`ports/<name>/README.md`) walks through porting it by hand from the upstream source -- download, QNX patches, configure, build, install, package, test -- with its dependencies first when it has any.

Every component is in exactly one of three classes. The class is computed by `scripts/gen-dependency-index` from the recipes *and* from the binaries the build installed (their `DT_NEEDED` entries), so it describes what actually happens on the target, not only what the recipe declares.

| Class | Build | Run on the target | Components |
|---|---|---|---|
| **[Standalone](#standalone)** | QNX SDP only | stock QNX image, nothing else | **271** |
| **[Runtime-standalone](#runtime-standalone)** | needs other components first | on its own | **110** |
| **[Dependent](#dependent)** | needs other components first | needs other components installed | **294** |

Libraries that ship with the QNX SDP target (libc, libm, libsocket, libregex, libc++, the eventfd/epoll shims, ...) are part of every QNX 8.0 system and are never counted as dependencies.

On a target, `oss-get install <component>` installs the run-time closure automatically; the *runtime* column below is what it pulls in.

Machine-readable: [`ports/dependencies.json`](ports/dependencies.json).

## Standalone

271 components. Clone the upstream source, apply the patches in `ports/<name>/patches/`, build with the QNX toolchain files -- nothing else from this repository has to be built first, and the result runs on a stock QNX 8.0 target.

✅ = built against an *empty* staging tree by `scripts/verify-standalone`, proving the build cannot have picked anything up from other components.

| Component | Version | Build system | Verified alone | Used by |
|---|---|---|---|---|
| [`7zip`](ports/7zip/) | 25.01 | custom | – | 0 |
| [`abseil`](ports/abseil/) | 20240722.0 | cmake | aarch64le✅ x86_64✅ | 6 |
| [`ada`](ports/ada/) | 3.4.4 | cmake | – | 1 |
| [`adwaita-fonts`](ports/adwaita-fonts/) | 49.0 | custom | – | 0 |
| [`aom`](ports/aom/) | 3.12.1 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`apr`](ports/apr/) | 1.7.6 | autotools | – | 2 |
| [`asio`](ports/asio/) | 1.30.2 | custom | – | 1 |
| [`autoconf`](ports/autoconf/) | 2.72 | autotools | – | 0 |
| [`automake`](ports/automake/) | 1.18 | autotools | – | 0 |
| [`aws-c-common`](ports/aws-c-common/) | 0.12.6 | cmake | – | 5 |
| [`benchmark`](ports/benchmark/) | 1.9.0 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`bison`](ports/bison/) | 3.8.2 | autotools | – | 0 |
| [`boringssl`](ports/boringssl/) | 0.20260929.0 | cmake | – | 0 |
| [`brotli`](ports/brotli/) | 1.1.0 | cmake | aarch64le✅ x86_64✅ | 10 |
| [`btop`](ports/btop/) | 1.4.6 | cmake | – | 0 |
| [`bullet3`](ports/bullet3/) | 3.25 | cmake | – | 0 |
| [`busybox`](ports/busybox/) | 1.36.1 | custom | – | 0 |
| [`bzip2`](ports/bzip2/) | 1.0.8 | custom | aarch64le✅ x86_64✅ | 15 |
| [`c-ares`](ports/c-ares/) | 1.34.3 | cmake | aarch64le✅ x86_64✅ | 3 |
| [`ca-certificates`](ports/ca-certificates/) | 20261020 | custom | – | 0 |
| [`can-utils`](ports/can-utils/) | 2025.01 | custom | – | 0 |
| [`candb`](ports/candb/) | 0.1-git-20210625 | cmake | – | 0 |
| [`capicxx-core-runtime`](ports/capicxx-core-runtime/) | 3.2.4 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`capstone`](ports/capstone/) | 5.0.9 | cmake | – | 0 |
| [`catch2`](ports/catch2/) | 3.7.1 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`cereal`](ports/cereal/) | 1.3.2 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`check`](ports/check/) | 0.15.2 | cmake | – | 0 |
| [`chrony`](ports/chrony/) | 4.8-git-20260226 | custom | – | 0 |
| [`cjson`](ports/cjson/) | 1.7.18 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`clapack`](ports/clapack/) | 3.2.1 | cmake | – | 0 |
| [`cli11`](ports/cli11/) | 2.4.2 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`compiler-rt`](ports/compiler-rt/) | 22.1.8 | cmake | – | 0 |
| [`computelibrary`](ports/computelibrary/) | 53.0.0 | cmake | – | 0 |
| [`console_bridge`](ports/console_bridge/) | 1.0.2 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`cpuinfo`](ports/cpuinfo/) | 0-git-20240417 | cmake | – | 2 |
| [`csmith`](ports/csmith/) | 2.3.0 | cmake | – | 0 |
| [`cunit`](ports/cunit/) | 2.1-3 | custom | – | 0 |
| [`cyclonedds`](ports/cyclonedds/) | 0.10.5 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`dav1d`](ports/dav1d/) | 1.5.0 | meson | – | 2 |
| [`davici`](ports/davici/) | 1.6 | autotools | – | 0 |
| [`dhcpcd`](ports/dhcpcd/) | 10.5.2 | custom | – | 0 |
| [`dnsmasq`](ports/dnsmasq/) | 2.93test4 | custom | – | 0 |
| [`docbook-xsl`](ports/docbook-xsl/) | 1.79.2 | custom | – | 0 |
| [`double-conversion`](ports/double-conversion/) | 3.3.0 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`duktape`](ports/duktape/) | 2.7.0 | custom | aarch64le✅ x86_64✅ | 0 |
| [`ed`](ports/ed/) | 1.22.5 | autotools | – | 0 |
| [`eigen`](ports/eigen/) | 3.4.0 | cmake | aarch64le✅ x86_64✅ | 7 |
| [`enca`](ports/enca/) | 1.19 | autotools | aarch64le✅ x86_64✅ | 0 |
| [`encodings`](ports/encodings/) | 1.1.0 | autotools | – | 0 |
| [`expat`](ports/expat/) | 2.6.4 | cmake | aarch64le✅ x86_64✅ | 15 |
| [`fakeroot`](ports/fakeroot/) | 1.37.1.1 | autotools | – | 0 |
| [`farmhash`](ports/farmhash/) | 1.1-git-20190513 | autotools | – | 0 |
| [`fast-float`](ports/fast-float/) | 8.0.2 | cmake | – | 1 |
| [`fastcdr`](ports/fastcdr/) | 2.2.6 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`fastfetch`](ports/fastfetch/) | 2.66.1 | cmake | – | 0 |
| [`fdk-aac`](ports/fdk-aac/) | 2.0.3 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`fft2d`](ports/fft2d/) | 2006.12.28 | custom | – | 0 |
| [`flatbuffers`](ports/flatbuffers/) | 24.3.25 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`fmt`](ports/fmt/) | 11.2.0 | cmake | aarch64le✅ x86_64✅ | 3 |
| [`font-cantarell`](ports/font-cantarell/) | 0.303.1 | custom | – | 0 |
| [`font-dejavu`](ports/font-dejavu/) | 2.37 | custom | – | 1 |
| [`font-hack`](ports/font-hack/) | 3.003 | custom | – | 0 |
| [`font-inter`](ports/font-inter/) | 4.1 | custom | – | 0 |
| [`font-liberation`](ports/font-liberation/) | 2.1.5 | custom | – | 0 |
| [`font-noto-emoji`](ports/font-noto-emoji/) | 2.048 | custom | – | 0 |
| [`font-noto-sans-cjk`](ports/font-noto-sans-cjk/) | 2.004 | custom | – | 0 |
| [`font-noto-serif-cjk`](ports/font-noto-serif-cjk/) | 2.003 | custom | – | 0 |
| [`font-util`](ports/font-util/) | 1.4.1 | autotools | – | 1 |
| [`foonathan_memory`](ports/foonathan_memory/) | 0.7-3 | cmake | aarch64le✅ x86_64✅ | 1 |
| [`fp16`](ports/fp16/) | 0-git-20260620 | cmake | – | 0 |
| [`freeimage`](ports/freeimage/) | 3.18.0 | custom | – | 1 |
| [`fribidi`](ports/fribidi/) | 1.0.16 | meson | – | 8 |
| [`fxdiv`](ports/fxdiv/) | 0-git-20200417 | cmake | – | 2 |
| [`g3log`](ports/g3log/) | 2.4 | cmake | – | 0 |
| [`gdbm`](ports/gdbm/) | 1.24 | autotools | aarch64le✅ x86_64✅ | 2 |
| [`gemmlowp`](ports/gemmlowp/) | 0-git-20231103 | custom | – | 0 |
| [`geographiclib`](ports/geographiclib/) | 2.7 | cmake | – | 0 |
| [`gflags`](ports/gflags/) | 2.2.2 | cmake | aarch64le✅ x86_64✅ | 4 |
| [`giflib`](ports/giflib/) | 5.2.2 | custom | aarch64le✅ x86_64✅ | 2 |
| [`gklib`](ports/gklib/) | 5.1.1-git-20230326 | cmake | – | 1 |
| [`glm`](ports/glm/) | 1.0.1 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`gmp`](ports/gmp/) | 6.3.0 | autotools | aarch64le✅ x86_64✅ | 6 |
| [`googletest`](ports/googletest/) | 1.17.0 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`gperf`](ports/gperf/) | 3.1 | autotools | – | 0 |
| [`graphite2`](ports/graphite2/) | 1.3.14 | cmake | aarch64le✅ x86_64✅ | 2 |
| [`gsm`](ports/gsm/) | 1.0.22 | custom | aarch64le✅ x86_64✅ | 0 |
| [`hailort`](ports/hailort/) | 4.14.0 | cmake | – | 0 |
| [`hicolor-icon-theme`](ports/hicolor-icon-theme/) | 0.18 | meson | – | 2 |
| [`highway`](ports/highway/) | 1.2.0 | cmake | aarch64le✅ x86_64❌ | 1 |
| [`highwayhash`](ports/highwayhash/) | 0_git20240418 | cmake | – | 1 |
| [`hyphen`](ports/hyphen/) | 2.8.8 | autotools | aarch64le✅ x86_64✅ | 1 |
| [`iceoryx`](ports/iceoryx/) | 2.0.6 | cmake | aarch64le✅ x86_64✅ | 0 |
| [`icu`](ports/icu/) | 76.1 | custom | – | 6 |
| [`iperf2`](ports/iperf2/) | 2.2.1 | autotools | – | 0 |
| [`iso-codes`](ports/iso-codes/) | 4.17.0 | autotools | – | 1 |
| [`jansson`](ports/jansson/) | 2.14 | cmake | – | 0 |
| [`json-c`](ports/json-c/) | 0.18 | cmake | – | 0 |
| [`jsoncpp`](ports/jsoncpp/) | 1.9.6 | cmake | – | 0 |
| [`kineto`](ports/kineto/) | 0-git-20250214 | cmake | – | 0 |
| [`lame`](ports/lame/) | 3.100 | autotools | – | 2 |
| [`lely-core`](ports/lely-core/) | 2.3.4 | autotools | – | 0 |
| [`libaec`](ports/libaec/) | 1.1.7 | cmake | – | 0 |
| [`libcbor`](ports/libcbor/) | 0.11.0 | cmake | – | 0 |
| [`libccd`](ports/libccd/) | 2.1 | cmake | – | 1 |
| [`libconfig`](ports/libconfig/) | 1.8.1 | cmake | – | 0 |
| [`libdaemon`](ports/libdaemon/) | 0.14 | autotools | – | 1 |
| [`libdatrie`](ports/libdatrie/) | 0.2.13 | autotools | – | 1 |
| [`libde265`](ports/libde265/) | 1.0.15 | cmake | – | 0 |
| [`libdeflate`](ports/libdeflate/) | 1.22 | cmake | – | 0 |
| [`libdisplay-info`](ports/libdisplay-info/) | 0.3.0 | meson | – | 0 |
| [`libdrm-headers`](ports/libdrm-headers/) | 2.4.129 | custom | – | 2 |
| [`libev`](ports/libev/) | 4.33 | autotools | – | 0 |
| [`libexif`](ports/libexif/) | 0.6.24 | autotools | – | 1 |
| [`libfdt`](ports/libfdt/) | 1.7.2 | meson | – | 0 |
| [`libffi`](ports/libffi/) | 3.4.6 | autotools | – | 6 |
| [`libhevc`](ports/libhevc/) | 1.8.5 | cmake | – | 0 |
| [`libiconv`](ports/libiconv/) | 1.18 | autotools | – | 21 |
| [`libinih`](ports/libinih/) | 58 | meson | – | 1 |
| [`libjpeg-turbo`](ports/libjpeg-turbo/) | 3.0.4 | cmake | – | 21 |
| [`libjson`](ports/libjson/) | 0.8-git-20260512 | custom | – | 1 |
| [`libmd`](ports/libmd/) | 1.1.0 | autotools | – | 1 |
| [`libmetal`](ports/libmetal/) | 2024.10.0 | cmake | – | 1 |
| [`libmicrohttpd`](ports/libmicrohttpd/) | 1.0.1 | autotools | – | 0 |
| [`libmodbus`](ports/libmodbus/) | 3.1.11 | autotools | – | 0 |
| [`libogg`](ports/libogg/) | 1.3.5 | cmake | – | 8 |
| [`libpcap`](ports/libpcap/) | 1.10.5 | autotools | – | 2 |
| [`libpipeline`](ports/libpipeline/) | 1.5.8 | autotools | – | 1 |
| [`libretro-2048`](ports/libretro-2048/) | 0-git-20241227 | custom | – | 1 |
| [`libretro-fake08`](ports/libretro-fake08/) | 0-git-20260613 | custom | – | 1 |
| [`libretro-mrboom`](ports/libretro-mrboom/) | 0-git-20260625 | custom | – | 1 |
| [`libretro-retro8`](ports/libretro-retro8/) | 0-git-20260912 | custom | – | 1 |
| [`libretro-samples`](ports/libretro-samples/) | 0-git-20221110 | custom | – | 1 |
| [`libsamplerate`](ports/libsamplerate/) | 0.2.2 | cmake | – | 0 |
| [`libsfdo`](ports/libsfdo/) | 0.1.4 | meson | – | 1 |
| [`libsodium`](ports/libsodium/) | 1.0.20 | autotools | – | 3 |
| [`libtasn1`](ports/libtasn1/) | 4.19.0 | autotools | – | 3 |
| [`libtommath`](ports/libtommath/) | 1.3.0 | cmake | – | 1 |
| [`libtool`](ports/libtool/) | 2.5.4 | autotools | – | 0 |
| [`libunibreak`](ports/libunibreak/) | 6.1 | autotools | – | 0 |
| [`libunistring`](ports/libunistring/) | 1.3 | autotools | – | 4 |
| [`libunwind`](ports/libunwind/) | 1.8.3 | autotools | – | 0 |
| [`libusb`](ports/libusb/) | 1.0.27 | autotools | – | 2 |
| [`libutempter`](ports/libutempter/) | 1.2.3 | custom | – | 0 |
| [`libuuid`](ports/libuuid/) | 2.42.4 | autotools | – | 1 |
| [`libuv`](ports/libuv/) | 1.51.0 | cmake | – | 4 |
| [`libvpx`](ports/libvpx/) | 1.15.0 | custom | – | 2 |
| [`libxcrypt`](ports/libxcrypt/) | 4.4.38 | autotools | – | 7 |
| [`libxcvt`](ports/libxcvt/) | 0.1.3 | meson | – | 1 |
| [`libxo`](ports/libxo/) | 2.3.0 | autotools | – | 0 |
| [`libyaml`](ports/libyaml/) | 0.2.5 | cmake | – | 3 |
| [`linux-headers-input-codes`](ports/linux-headers-input-codes/) | 6.12 | custom | – | 3 |
| [`lmbench`](ports/lmbench/) | 3.0-git-20260929 | custom | – | 0 |
| [`lua`](ports/lua/) | 5.4.7 | custom | – | 0 |
| [`lua5.1`](ports/lua5.1/) | 5.1.5 | custom | – | 0 |
| [`lua5.3`](ports/lua5.3/) | 5.3.6 | custom | – | 1 |
| [`luajit`](ports/luajit/) | 2.1.c6ffc141 | custom | – | 4 |
| [`lxde-icon-theme`](ports/lxde-icon-theme/) | 0.5.2 | autotools | – | 1 |
| [`lz4`](ports/lz4/) | 1.10.0 | cmake | – | 11 |
| [`lzip`](ports/lzip/) | 1.26 | autotools | – | 0 |
| [`lzo`](ports/lzo/) | 2.10 | autotools | – | 3 |
| [`magic_enum`](ports/magic_enum/) | 0.9.6 | cmake | – | 0 |
| [`make`](ports/make/) | 4.4.1 | autotools | – | 0 |
| [`mbedtls`](ports/mbedtls/) | 3.6.2 | cmake | – | 0 |
| [`mimalloc`](ports/mimalloc/) | 3.1.5 | cmake | – | 0 |
| [`miniz`](ports/miniz/) | 3.1.2 | cmake | – | 0 |
| [`mnn`](ports/mnn/) | 3.6.2 | cmake | – | 0 |
| [`mpg123`](ports/mpg123/) | 1.32.9 | autotools | – | 0 |
| [`msgpack-c`](ports/msgpack-c/) | 6.1.0 | cmake | – | 0 |
| [`mujoco`](ports/mujoco/) | 3.10.0 | cmake | – | 0 |
| [`muslflt`](ports/muslflt/) | 1.0-git-20240802 | custom | – | 0 |
| [`nanomsg`](ports/nanomsg/) | 1.3.0 | cmake | – | 0 |
| [`nanosvg`](ports/nanosvg/) | 0-git-20260709 | cmake | – | 1 |
| [`ncnn`](ports/ncnn/) | 20260526 | cmake | – | 0 |
| [`ncurses`](ports/ncurses/) | 6.5 | autotools | – | 20 |
| [`netmap`](ports/netmap/) | 14.0-git-20261027 | custom | – | 0 |
| [`ninja`](ports/ninja/) | 1.13.1 | cmake | – | 0 |
| [`nlohmann_json`](ports/nlohmann_json/) | 3.12.0 | cmake | – | 0 |
| [`nng`](ports/nng/) | 1.9.0 | cmake | – | 0 |
| [`npth`](ports/npth/) | 1.8 | autotools | – | 1 |
| [`nspr`](ports/nspr/) | 4.39 | autotools | – | 1 |
| [`nsync`](ports/nsync/) | 1.30.0 | cmake | – | 0 |
| [`ntttcp-for-linux`](ports/ntttcp-for-linux/) | 1.4.0-git-20230613 | cmake | – | 0 |
| [`octomap`](ports/octomap/) | 1.10.0 | cmake | – | 1 |
| [`onednn`](ports/onednn/) | 3.14.1 | cmake | – | 0 |
| [`onetbb`](ports/onetbb/) | 2022.0.0 | cmake | – | 0 |
| [`oniguruma`](ports/oniguruma/) | 6.9.9 | cmake | – | 1 |
| [`openal-soft`](ports/openal-soft/) | 1.24.1 | cmake | – | 0 |
| [`openblas`](ports/openblas/) | 0.3.28 | cmake | – | 3 |
| [`openh264`](ports/openh264/) | 2.5.0 | meson | – | 2 |
| [`openmp`](ports/openmp/) | 22.1.8 | cmake | – | 0 |
| [`openpam`](ports/openpam/) | 20250531 | autotools | – | 0 |
| [`openssl`](ports/openssl/) | 3.5.1 | custom | – | 54 |
| [`openttd-opengfx`](ports/openttd-opengfx/) | 8.0 | custom | – | 0 |
| [`openttd-openmsx`](ports/openttd-openmsx/) | 0.4.2 | custom | – | 0 |
| [`openttd-opensfx`](ports/openttd-opensfx/) | 1.0.3 | custom | – | 0 |
| [`opus`](ports/opus/) | 1.5.2 | cmake | – | 5 |
| [`orc`](ports/orc/) | 0.4.40 | meson | – | 1 |
| [`patch`](ports/patch/) | 2.8 | autotools | – | 0 |
| [`pax-utils`](ports/pax-utils/) | 1.3.8 | meson | – | 0 |
| [`pcre`](ports/pcre/) | 8.45 | autotools | – | 0 |
| [`pdfium`](ports/pdfium/) | 0-git-84a8011 | custom | – | 0 |
| [`picocom`](ports/picocom/) | 3.1 | custom | – | 0 |
| [`pkgconf`](ports/pkgconf/) | 2.3.0 | meson | – | 0 |
| [`protobuf-c`](ports/protobuf-c/) | 1.5.0 | cmake | – | 0 |
| [`pthread-stubs`](ports/pthread-stubs/) | 0.5 | autotools | – | 1 |
| [`ptpd`](ports/ptpd/) | 2.3.1 | autotools | – | 0 |
| [`pugixml`](ports/pugixml/) | 1.14 | cmake | – | 1 |
| [`pybind11`](ports/pybind11/) | 3.1.0 | cmake | – | 0 |
| [`qhull`](ports/qhull/) | 8.0.2 | cmake | – | 1 |
| [`quickjs-ng`](ports/quickjs-ng/) | 0.8.0 | cmake | – | 0 |
| [`rapidjson`](ports/rapidjson/) | 1.1.0-24b5e7a | cmake | – | 2 |
| [`rust`](ports/rust/) | 1.90.0 | custom | – | 1 |
| [`silkit`](ports/silkit/) | 5.0.8 | cmake | – | 1 |
| [`simdjson`](ports/simdjson/) | 3.11.3 | cmake | – | 1 |
| [`simdutf`](ports/simdutf/) | 6.4.2 | cmake | – | 1 |
| [`skalibs`](ports/skalibs/) | 2.14.5.1 | custom | – | 0 |
| [`sleef`](ports/sleef/) | 3.8-git-20250214 | cmake | – | 0 |
| [`snappy`](ports/snappy/) | 1.2.1 | cmake | – | 3 |
| [`soxr`](ports/soxr/) | 0.1.3 | cmake | – | 0 |
| [`speexdsp`](ports/speexdsp/) | 1.2.1 | autotools | – | 0 |
| [`sshpass`](ports/sshpass/) | 1.10 | autotools | – | 0 |
| [`stable-diffusion.cpp`](ports/stable-diffusion.cpp/) | 0_git20260601 | cmake | – | 0 |
| [`stringencoders`](ports/stringencoders/) | 3.10.3-git-20180306 | autotools | – | 0 |
| [`svt-av1`](ports/svt-av1/) | 2.3.0 | cmake | – | 1 |
| [`tensorflow-lite`](ports/tensorflow-lite/) | 2.16.1 | custom | – | 0 |
| [`tinyxml`](ports/tinyxml/) | 2.6.2 | custom | – | 0 |
| [`tinyxml2`](ports/tinyxml2/) | 10.0.0 | cmake | – | 3 |
| [`tomlplusplus`](ports/tomlplusplus/) | 3.4.0 | cmake | – | 0 |
| [`toybox`](ports/toybox/) | 0.8.15 | custom | – | 0 |
| [`tree`](ports/tree/) | 2.2.1 | custom | – | 0 |
| [`tree-sitter`](ports/tree-sitter/) | 0.25.9 | cmake | – | 2 |
| [`tree-sitter-bash`](ports/tree-sitter-bash/) | 0.25.0 | custom | – | 0 |
| [`tree-sitter-c`](ports/tree-sitter-c/) | 0.24.1 | custom | – | 0 |
| [`tree-sitter-cmake`](ports/tree-sitter-cmake/) | 0.7.2 | custom | – | 0 |
| [`tree-sitter-lua`](ports/tree-sitter-lua/) | 0.4.0 | custom | – | 0 |
| [`tree-sitter-make`](ports/tree-sitter-make/) | 1.1.1 | custom | – | 0 |
| [`tree-sitter-markdown`](ports/tree-sitter-markdown/) | 0.5.1 | custom | – | 0 |
| [`tree-sitter-python`](ports/tree-sitter-python/) | 0.23.6 | custom | – | 0 |
| [`tree-sitter-query`](ports/tree-sitter-query/) | 0.7.0 | custom | – | 0 |
| [`tree-sitter-vim`](ports/tree-sitter-vim/) | 0.7.0 | custom | – | 0 |
| [`tree-sitter-vimdoc`](ports/tree-sitter-vimdoc/) | 4.0.0 | custom | – | 0 |
| [`trompeloeil`](ports/trompeloeil/) | 48 | cmake | – | 0 |
| [`unibilium`](ports/unibilium/) | 2.1.2 | cmake | – | 1 |
| [`urdfdom_headers`](ports/urdfdom_headers/) | 1.1.2 | cmake | – | 1 |
| [`uriparser`](ports/uriparser/) | 0.9.8 | cmake | – | 0 |
| [`utf8proc`](ports/utf8proc/) | 2.11.3 | cmake | – | 2 |
| [`uvgrtp`](ports/uvgrtp/) | 4.0.1 | cmake | – | 0 |
| [`vidstab`](ports/vidstab/) | 1.1.1 | cmake | – | 0 |
| [`webrtc`](ports/webrtc/) | m132-afaf497 | custom | – | 0 |
| [`whisper.cpp`](ports/whisper.cpp/) | 0_git20260728 | cmake | – | 0 |
| [`wlr-protocols`](ports/wlr-protocols/) | 0-git-20250702 | custom | – | 0 |
| [`wolfssl`](ports/wolfssl/) | 5.7.6 | cmake | – | 0 |
| [`x264`](ports/x264/) | stable | custom | – | 2 |
| [`x265`](ports/x265/) | 3.6 | cmake | – | 2 |
| [`xcb-proto`](ports/xcb-proto/) | 1.17.0 | autotools | – | 1 |
| [`xdg-utils`](ports/xdg-utils/) | 1.2.1 | autotools | – | 0 |
| [`xkeyboard-config`](ports/xkeyboard-config/) | 2.43 | meson | – | 1 |
| [`xmlto`](ports/xmlto/) | 0.0.29 | autotools | – | 0 |
| [`xorgproto`](ports/xorgproto/) | 2024.1 | meson | – | 25 |
| [`xtl`](ports/xtl/) | 0.8.2 | cmake | – | 0 |
| [`xtrans`](ports/xtrans/) | 1.5.2 | autotools | – | 6 |
| [`xvidcore`](ports/xvidcore/) | 1.3.7 | custom | – | 0 |
| [`xxhash`](ports/xxhash/) | 0.8.2 | cmake | – | 2 |
| [`yaml-cpp`](ports/yaml-cpp/) | 0.8.0 | cmake | – | 1 |
| [`zenoh-pico`](ports/zenoh-pico/) | 1.2.1 | cmake | – | 1 |
| [`zerotier-one`](ports/zerotier-one/) | 1.16.1 | custom | – | 0 |
| [`zimg`](ports/zimg/) | 3.0.5 | autotools | – | 0 |
| [`zlib`](ports/zlib/) | 1.3.1 | cmake | aarch64le✅ | 83 |
| [`zlib-ng`](ports/zlib-ng/) | 2.2.5 | cmake | – | 0 |
| [`zserio`](ports/zserio/) | 2.20.0 | cmake | – | 0 |
| [`zstd`](ports/zstd/) | 1.5.6 | cmake | – | 27 |

## Runtime-standalone

110 components. They need other components at *build* time -- header-only libraries (Eigen, nlohmann_json, ...), static archives, or generators -- but what they install loads nothing from this tree. Build the listed chain first (`qnxbuild` does it); deploy the component alone.

| Component | Version | Needs to build | Full build chain |
|---|---|---|---|
| [`adwaita-icon-theme`](ports/adwaita-icon-theme/) | 51.0 | hicolor-icon-theme | 1 |
| [`amqp-cpp`](ports/amqp-cpp/) | 4.3.27 | openssl | 1 |
| [`asciidoc`](ports/asciidoc/) | 10.2.1 | python3 | 15 |
| [`cppzmq`](ports/cppzmq/) | 4.10.0 | libzmq | 2 |
| [`glfw`](ports/glfw/) | 3.5.1 | wayland libxkbcommon linux-headers-input-codes | 19 |
| [`gsettings-desktop-schemas`](ports/gsettings-desktop-schemas/) | 49.0 | glib | 9 |
| [`itstool`](ports/itstool/) | 2.0.7 | python3 libxml2 | 16 |
| [`lcms2`](ports/lcms2/) | 2.16 | zlib | 1 |
| [`libepoxy`](ports/libepoxy/) | 1.5.10 | libx11 | 8 |
| [`libice`](ports/libice/) | 1.1.2 | xorgproto xtrans | 2 |
| [`libwebp`](ports/libwebp/) | 1.4.0 | libpng libjpeg-turbo giflib | 4 |
| [`libxau`](ports/libxau/) | 1.0.11 | xorgproto | 1 |
| [`libxdmcp`](ports/libxdmcp/) | 1.1.5 | xorgproto | 1 |
| [`libxshmfence`](ports/libxshmfence/) | 1.3.3 | xorgproto | 1 |
| [`lua-lpeg`](ports/lua-lpeg/) | 1.1.0 | luajit | 1 |
| [`mcap-cpp`](ports/mcap-cpp/) | 2.1.3 | lz4 zstd | 2 |
| [`meson`](ports/meson/) | 1.10.0 | python3 | 15 |
| [`nghttp2`](ports/nghttp2/) | 1.64.0 | zlib openssl | 2 |
| [`npm`](ports/npm/) | 11.11.0 | nodejs | 13 |
| [`open-amp`](ports/open-amp/) | 2024.10.0 | libmetal | 1 |
| [`openjpeg`](ports/openjpeg/) | 2.5.3 | zlib libpng libtiff lcms2 | 13 |
| [`orocos-kdl`](ports/orocos-kdl/) | 1.5.1 | eigen | 1 |
| [`oss-desktop`](ports/oss-desktop/) | 1.0 | labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu | 93 |
| [`pcre2`](ports/pcre2/) | 10.44 | zlib bzip2 | 2 |
| [`perl-error`](ports/perl-error/) | 0.17030 | perl | 2 |
| [`perl-io-tty`](ports/perl-io-tty/) | 1.31 | perl | 2 |
| [`perl-mozilla-ca`](ports/perl-mozilla-ca/) | 20261020 | perl | 2 |
| [`pixman`](ports/pixman/) | 0.46.4 | libpng | 2 |
| [`psutil`](ports/psutil/) | 7.2.2 | python3 | 15 |
| [`pthreadpool`](ports/pthreadpool/) | 0-git-20250207 | fxdiv | 1 |
| [`python3-argcomplete`](ports/python3-argcomplete/) | 3.7.2 | python3 | 15 |
| [`python3-astor`](ports/python3-astor/) | 0.8.1 | python3 | 15 |
| [`python3-async-generator`](ports/python3-async-generator/) | 1.10 | python3 | 15 |
| [`python3-boto3`](ports/python3-boto3/) | 1.43.123 | python3 python3-botocore python3-s3transfer python3-jmespath | 21 |
| [`python3-botocore`](ports/python3-botocore/) | 1.43.123 | python3 python3-jmespath python3-dateutil python3-urllib3 | 19 |
| [`python3-catkin-pkg`](ports/python3-catkin-pkg/) | 1.1.1 | python3 python3-dateutil python3-docutils python3-packaging python3-pyparsing | 20 |
| [`python3-click`](ports/python3-click/) | 8.5.0 | python3 | 15 |
| [`python3-colcon-argcomplete`](ports/python3-colcon-argcomplete/) | 0.3.3 | python3 python3-argcomplete python3-colcon-core | 29 |
| [`python3-colcon-bash`](ports/python3-colcon-bash/) | 0.5.0 | python3 python3-colcon-core | 28 |
| [`python3-colcon-cd`](ports/python3-colcon-cd/) | 0.1.1 | python3 python3-colcon-core python3-colcon-package-information | 29 |
| [`python3-colcon-cmake`](ports/python3-colcon-cmake/) | 0.2.30 | python3 python3-colcon-core python3-colcon-library-path python3-colcon-test-result python3-packaging | 30 |
| [`python3-colcon-common-extensions`](ports/python3-colcon-common-extensions/) | 0.3.0 | python3 python3-colcon-argcomplete python3-colcon-bash python3-colcon-cd python3-colcon-cmake python3-colcon-core python3-colcon-defaults python3-colcon-devtools python3-colcon-library-path python3-colcon-metadata python3-colcon-notification python3-colcon-output python3-colcon-package-information python3-colcon-package-selection python3-colcon-parallel-executor python3-colcon-powershell python3-colcon-python-setup-py python3-colcon-recursive-crawl python3-colcon-ros python3-colcon-test-result python3-colcon-zsh | 55 |
| [`python3-colcon-core`](ports/python3-colcon-core/) | 0.21.3 | python3 python3-distlib python3-empy python3-packaging python3-pytest python3-pytest-cov python3-pytest-repeat python3-pytest-rerunfailures python3-setuptools | 27 |
| [`python3-colcon-defaults`](ports/python3-colcon-defaults/) | 0.2.9 | python3 python3-colcon-core python3-pyyaml | 29 |
| [`python3-colcon-devtools`](ports/python3-colcon-devtools/) | 0.3.0 | python3 python3-colcon-core python3-packaging | 28 |
| [`python3-colcon-library-path`](ports/python3-colcon-library-path/) | 0.2.1 | python3 python3-colcon-core | 28 |
| [`python3-colcon-metadata`](ports/python3-colcon-metadata/) | 0.3.1 | python3 python3-colcon-core python3-pyyaml | 29 |
| [`python3-colcon-notification`](ports/python3-colcon-notification/) | 0.3.3 | python3 python3-colcon-core | 28 |
| [`python3-colcon-output`](ports/python3-colcon-output/) | 0.2.15 | python3 python3-colcon-core | 28 |
| [`python3-colcon-package-information`](ports/python3-colcon-package-information/) | 0.4.1 | python3 python3-colcon-core python3-packaging | 28 |
| [`python3-colcon-package-selection`](ports/python3-colcon-package-selection/) | 0.2.10 | python3 python3-colcon-core | 28 |
| [`python3-colcon-parallel-executor`](ports/python3-colcon-parallel-executor/) | 0.4.0 | python3 python3-colcon-core | 28 |
| [`python3-colcon-pkg-config`](ports/python3-colcon-pkg-config/) | 0.1.0 | python3 python3-colcon-core | 28 |
| [`python3-colcon-powershell`](ports/python3-colcon-powershell/) | 0.5.0 | python3 python3-colcon-core | 28 |
| [`python3-colcon-python-setup-py`](ports/python3-colcon-python-setup-py/) | 0.2.9 | python3 python3-colcon-core python3-setuptools | 28 |
| [`python3-colcon-recursive-crawl`](ports/python3-colcon-recursive-crawl/) | 0.2.3 | python3 python3-colcon-core | 28 |
| [`python3-colcon-ros`](ports/python3-colcon-ros/) | 0.5.0 | python3 python3-catkin-pkg python3-colcon-cmake python3-colcon-core python3-colcon-pkg-config python3-colcon-python-setup-py python3-colcon-recursive-crawl | 39 |
| [`python3-colcon-test-result`](ports/python3-colcon-test-result/) | 0.3.8 | python3 python3-colcon-core | 28 |
| [`python3-colcon-zsh`](ports/python3-colcon-zsh/) | 0.5.0 | python3 python3-colcon-core | 28 |
| [`python3-coverage`](ports/python3-coverage/) | 7.17.0 | python3 | 15 |
| [`python3-curio`](ports/python3-curio/) | 1.6_git20251221 | python3 | 15 |
| [`python3-cycler`](ports/python3-cycler/) | 0.12.1 | python3 | 15 |
| [`python3-dateutil`](ports/python3-dateutil/) | 2.9.0.post0 | python3 python3-six | 16 |
| [`python3-distlib`](ports/python3-distlib/) | 0.4.4 | python3 | 15 |
| [`python3-docutils`](ports/python3-docutils/) | 0.23 | python3 | 15 |
| [`python3-empy`](ports/python3-empy/) | 4.2.1 | python3 | 15 |
| [`python3-fonttools`](ports/python3-fonttools/) | 4.68.0 | python3 | 15 |
| [`python3-idna`](ports/python3-idna/) | 3.20 | python3 | 15 |
| [`python3-iniconfig`](ports/python3-iniconfig/) | 2.3.1 | python3 | 15 |
| [`python3-jmespath`](ports/python3-jmespath/) | 1.1.0 | python3 | 15 |
| [`python3-kiwisolver`](ports/python3-kiwisolver/) | 1.5.1 | python3 | 15 |
| [`python3-ml-dtypes`](ports/python3-ml-dtypes/) | 0.6.0 | numpy | 17 |
| [`python3-mypy-extensions`](ports/python3-mypy-extensions/) | 1.1.0 | python3 | 15 |
| [`python3-netifaces`](ports/python3-netifaces/) | 0.11.0 | python3 | 15 |
| [`python3-packaging`](ports/python3-packaging/) | 25.0 | python3 | 15 |
| [`python3-pip`](ports/python3-pip/) | 25.1.1 | python3 | 15 |
| [`python3-pluggy`](ports/python3-pluggy/) | 1.7.0 | python3 | 15 |
| [`python3-pretend`](ports/python3-pretend/) | 1.0.9 | python3 | 15 |
| [`python3-pygments`](ports/python3-pygments/) | 2.21.0 | python3 | 15 |
| [`python3-pyparsing`](ports/python3-pyparsing/) | 3.3.3 | python3 | 15 |
| [`python3-pytest`](ports/python3-pytest/) | 9.2.0 | python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments | 19 |
| [`python3-pytest-cov`](ports/python3-pytest-cov/) | 7.1.0 | python3 python3-pytest python3-coverage python3-pluggy | 21 |
| [`python3-pytest-repeat`](ports/python3-pytest-repeat/) | 0.9.4 | python3 python3-pytest | 20 |
| [`python3-pytest-rerunfailures`](ports/python3-pytest-rerunfailures/) | 16.7 | python3 python3-packaging python3-pytest | 20 |
| [`python3-pytest-tornasync`](ports/python3-pytest-tornasync/) | 0.6.0.post2 | python3 python3-pytest python3-tornado | 21 |
| [`python3-pytokens`](ports/python3-pytokens/) | 0.4.1 | python3 | 15 |
| [`python3-pytz`](ports/python3-pytz/) | 2025.2 | python3 | 15 |
| [`python3-pyyaml`](ports/python3-pyyaml/) | 6.0.3 | python3 | 15 |
| [`python3-s3transfer`](ports/python3-s3transfer/) | 0.19.3 | python3 python3-botocore | 20 |
| [`python3-setuptools`](ports/python3-setuptools/) | 80.8.0 | python3 | 15 |
| [`python3-six`](ports/python3-six/) | 1.17.0 | python3 | 15 |
| [`python3-sniffio`](ports/python3-sniffio/) | 1.3.1 | python3 | 15 |
| [`python3-sortedcontainers`](ports/python3-sortedcontainers/) | 2.4.0 | python3 | 15 |
| [`python3-tornado`](ports/python3-tornado/) | 6.6 | python3 | 15 |
| [`python3-traitlets`](ports/python3-traitlets/) | 5.16.1 | python3 | 15 |
| [`python3-tzdata`](ports/python3-tzdata/) | 2025.2 | python3 | 15 |
| [`python3-urllib3`](ports/python3-urllib3/) | 2.8.0 | python3 | 15 |
| [`python3-versioneer`](ports/python3-versioneer/) | 0.29 | python3 | 15 |
| [`python3-wheel`](ports/python3-wheel/) | 0.46.1 | python3 python3-packaging | 16 |
| [`readline`](ports/readline/) | 8.2 | ncurses | 1 |
| [`retropie`](ports/retropie/) | 0.3 | retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08 | 48 |
| [`ruy`](ports/ruy/) | 0-git-20240321 | cpuinfo | 1 |
| [`sdl2`](ports/sdl2/) | 2.32.10 | wayland libxkbcommon | 18 |
| [`sdl2-compat`](ports/sdl2-compat/) | 2.32.60 | sdl3 | 34 |
| [`soem`](ports/soem/) | 2.0.1 | libpcap | 1 |
| [`sqlite_orm`](ports/sqlite_orm/) | 1.9 | sqlite | 2 |
| [`tcpdump`](ports/tcpdump/) | 4.99.5 | libpcap | 1 |
| [`wayland-protocols`](ports/wayland-protocols/) | 1.47 | wayland | 17 |
| [`zenoh-c`](ports/zenoh-c/) | 1.2.1 | rust | 1 |
| [`zenoh-cpp`](ports/zenoh-cpp/) | 1.2.1 | zenoh-c zenoh-pico | 3 |

## Dependent

294 components. Both the build and the target need other components. *Runtime* is the set of components whose shared libraries these binaries load -- the packages `oss-get` installs with it.

| Component | Version | Needs to build | Needs at run time | Run-time closure |
|---|---|---|---|---|
| [`abuild`](ports/abuild/) | 3.16.0 | openssl zlib apk-tools | openssl zlib | 2 |
| [`apk-tools`](ports/apk-tools/) | 3.0.0_rc5 | openssl zlib zstd | openssl zlib zstd | 3 |
| [`appstream`](ports/appstream/) | 1.0.6 | glib curl libxml2 libyaml libxmlb zstd | curl gettext glib libxml2 libxmlb libyaml zstd | 18 |
| [`apr-util`](ports/apr-util/) | 1.6.5 | apr expat openssl sqlite libiconv libxcrypt | apr expat libiconv libxcrypt openssl sqlite | 7 |
| [`aspell`](ports/aspell/) | 0.60.8.1 | ncurses | ncurses | 1 |
| [`at-spi2-core`](ports/at-spi2-core/) | 2.54.0 | glib dbus libxml2 libx11 libxtst | dbus gettext glib libx11 libxi libxtst | 17 |
| [`avahi`](ports/avahi/) | 0.9-rc3 | dbus expat libdaemon | dbus expat libdaemon | 3 |
| [`avro-cpp`](ports/avro-cpp/) | 1.12.2 | fmt zlib snappy zstd | snappy zlib zstd | 3 |
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
| [`c-blosc`](ports/c-blosc/) | 1.21.6 | lz4 zlib zstd | lz4 zlib zstd | 3 |
| [`cairo`](ports/cairo/) | 1.18.2 | pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo | fontconfig freetype glib libpng libx11 libxext libxrender lzo pixman zlib | 22 |
| [`capicxx-someip-runtime`](ports/capicxx-someip-runtime/) | 3.2.4 | capicxx-core-runtime vsomeip | capicxx-core-runtime vsomeip | 10 |
| [`capnproto`](ports/capnproto/) | 1.0.2 | zlib openssl | openssl zlib | 2 |
| [`cartographer`](ports/cartographer/) | 2.0.0 | abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo | abseil boost cairo ceres-solver gflags glog lua5.3 protobuf | 31 |
| [`ceres-solver`](ports/ceres-solver/) | 2.2.0 | eigen glog gflags | gflags glog | 2 |
| [`civetweb`](ports/civetweb/) | 1.16 | openssl | openssl | 1 |
| [`cmake`](ports/cmake/) | 4.1.1 | openssl zlib curl libuv | curl libuv openssl zlib | 8 |
| [`cups`](ports/cups/) | 2.4.16 | openssl zlib avahi libiconv libxcrypt | avahi libiconv libxcrypt openssl zlib | 8 |
| [`cups-filters`](ports/cups-filters/) | 1.28.17 | cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib | avahi cups fontconfig freetype gettext glib lcms2 libexif libiconv libjpeg-turbo libpng libtiff libxcrypt openssl poppler qpdf zlib | 39 |
| [`curl`](ports/curl/) | 8.11.1 | openssl zlib nghttp2 c-ares brotli zstd | brotli c-ares nghttp2 openssl zlib zstd | 6 |
| [`dbus`](ports/dbus/) | 1.16.0 | expat | expat | 1 |
| [`desktop-file-utils`](ports/desktop-file-utils/) | 0.28 | glib | gettext glib | 8 |
| [`dlt-daemon`](ports/dlt-daemon/) | 2.18.10 | zlib libjson | libjson | 1 |
| [`doxygen`](ports/doxygen/) | 1.17.0 | libiconv | libiconv | 1 |
| [`dump1090`](ports/dump1090/) | 0-git-20240121 | rtl-sdr | rtl-sdr | 2 |
| [`e2fsprogs`](ports/e2fsprogs/) | 1.47.4 | libuuid | libuuid | 1 |
| [`elfutils`](ports/elfutils/) | 0.193 | zlib | zlib | 1 |
| [`elvis`](ports/elvis/) | 2.2.1-git-20240226 | ncurses | ncurses | 1 |
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
| [`folly`](ports/folly/) | 2026.10.26.00 | boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium | boost bzip2 fmt gflags glog libevent libsodium lz4 openssl xz zlib zstd | 16 |
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
| [`gnupg`](ports/gnupg/) | 2.4.9 | libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv | bzip2 gnutls libassuan libgcrypt libgpg-error libiconv libksba ncurses npth readline sqlite zlib | 20 |
| [`gnutls`](ports/gnutls/) | 3.8.8 | nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd | brotli gmp libidn2 libtasn1 libunistring nettle zlib zstd | 11 |
| [`gobject-introspection`](ports/gobject-introspection/) | 1.86.0 | glib libffi python3 | glib libffi | 8 |
| [`godot`](ports/godot/) | 4.4.1 | freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon | brotli bzip2 freetype gettext glib graphite2 harfbuzz libiconv libogg libpng libtheora libvorbis libwebp libxkbcommon pcre2 zlib zstd | 23 |
| [`gpgme`](ports/gpgme/) | 2.2.0 | libgpg-error libassuan | libassuan libgpg-error | 6 |
| [`graphene`](ports/graphene/) | 1.10.8 | glib | glib | 8 |
| [`graphicsmagick`](ports/graphicsmagick/) | 1.3.46 | libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2 | bzip2 freetype lcms2 libjpeg-turbo libjxl libpng libtiff libwebp libxml2 libzip xz zlib zstd | 20 |
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
| [`gtk-vnc`](ports/gtk-vnc/) | 1.3.1 | glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11 | cairo gdk-pixbuf gettext glib gnutls gtk3 libgcrypt libx11 zlib | 55 |
| [`gtk3`](ports/gtk3/) | 3.24.43 | glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama | at-spi2-core cairo fontconfig fribidi gdk-pixbuf gettext glib harfbuzz libepoxy libx11 libxcomposite libxcursor libxdamage libxext libxfixes libxi libxinerama libxkbcommon libxrandr pango wayland | 46 |
| [`gtk4`](ports/gtk4/) | 4.16.7 | glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff | cairo fontconfig fribidi gdk-pixbuf gettext glib graphene harfbuzz libepoxy libjpeg-turbo libpng libtiff libxkbcommon pango wayland | 40 |
| [`gtsam`](ports/gtsam/) | 4.2 | boost eigen | boost | 8 |
| [`harfbuzz`](ports/harfbuzz/) | 10.1.0 | freetype glib icu graphite2 | freetype glib graphite2 icu | 14 |
| [`htop`](ports/htop/) | 3.5.0 | ncurses | ncurses | 1 |
| [`inetutils`](ports/inetutils/) | 2.6 | ncurses readline libxcrypt | libxcrypt ncurses readline | 3 |
| [`iperf3`](ports/iperf3/) | 3.17.1 | openssl | openssl | 1 |
| [`ippusbxd`](ports/ippusbxd/) | 1.34 | libusb avahi libxml2 cups | avahi cups libusb libxml2 | 15 |
| [`javascriptcore`](ports/javascriptcore/) | 2.44.4 | icu | icu | 1 |
| [`jq`](ports/jq/) | 1.7.1 | oniguruma | oniguruma | 1 |
| [`json-glib`](ports/json-glib/) | 1.10.6 | glib | gettext glib | 8 |
| [`labwc`](ports/labwc/) | 0.20.2 | wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng | cairo glib libpng libsfdo libxkbcommon libxml2 pango pixman wayland wlroots | 36 |
| [`lcm`](ports/lcm/) | 1.5.2 | glib | glib | 8 |
| [`less`](ports/less/) | 668 | ncurses | ncurses | 1 |
| [`leveldb`](ports/leveldb/) | 1.23 | snappy | snappy | 1 |
| [`libadwaita`](ports/libadwaita/) | 1.6.10 | glib gtk4 fribidi appstream | appstream fribidi gettext glib graphene gtk4 pango | 48 |
| [`libarchive`](ports/libarchive/) | 3.7.7 | zlib bzip2 xz zstd openssl libxml2 expat lz4 | bzip2 expat lz4 openssl xz zlib zstd | 11 |
| [`libass`](ports/libass/) | 0.17.3 | freetype fribidi harfbuzz fontconfig | fontconfig freetype fribidi harfbuzz | 18 |
| [`libassuan`](ports/libassuan/) | 3.0.2 | libgpg-error | libgpg-error | 5 |
| [`libavif`](ports/libavif/) | 1.1.1 | aom dav1d libyuv | aom dav1d | 2 |
| [`libcamera`](ports/libcamera/) | 0.5.1+rpt20250722 | libyaml | libyaml | 1 |
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
| [`libksba`](ports/libksba/) | 1.8.1 | libgpg-error | libgpg-error | 5 |
| [`libnotify`](ports/libnotify/) | 0.8.3 | glib gdk-pixbuf | gdk-pixbuf glib | 11 |
| [`libpanel`](ports/libpanel/) | 1.10.4 | glib gtk4 libadwaita | cairo gettext glib graphene gtk4 libadwaita | 49 |
| [`libpng`](ports/libpng/) | 1.6.44 | zlib | zlib | 1 |
| [`libportal`](ports/libportal/) | 0.8.1 | glib gtk3 | glib gtk3 | 47 |
| [`libproxy`](ports/libproxy/) | 0.5.9 | glib curl | curl glib | 14 |
| [`libpsl`](ports/libpsl/) | 0.21.5 | libidn2 libunistring libiconv | libiconv libidn2 libunistring | 5 |
| [`libraqm`](ports/libraqm/) | 0.10.5 | freetype harfbuzz fribidi | freetype fribidi harfbuzz | 16 |
| [`libsecret`](ports/libsecret/) | 0.21.4 | glib libgcrypt | glib libgcrypt | 10 |
| [`libsm`](ports/libsm/) | 1.2.6 | libice xorgproto xtrans | libice | 1 |
| [`libsndfile`](ports/libsndfile/) | 1.2.2 | libogg libvorbis flac opus | flac libogg libvorbis opus | 4 |
| [`libsoup`](ports/libsoup/) | 3.6.1 | glib libxml2 sqlite nghttp2 libpsl brotli | brotli gettext glib libpsl nghttp2 sqlite zlib | 13 |
| [`libssh`](ports/libssh/) | 0.11.1 | openssl zlib | openssl zlib | 2 |
| [`libssh2`](ports/libssh2/) | 1.11.1 | openssl zlib | openssl zlib | 2 |
| [`libthai`](ports/libthai/) | 0.1.29 | libdatrie | libdatrie | 1 |
| [`libtheora`](ports/libtheora/) | 1.1.1 | libogg libvorbis | libogg | 1 |
| [`libtiff`](ports/libtiff/) | 4.7.0 | zlib xz zstd libjpeg-turbo libwebp | libjpeg-turbo libwebp xz zlib zstd | 9 |
| [`libtomcrypt`](ports/libtomcrypt/) | 1.18.2 | libtommath | libtommath | 1 |
| [`libvncserver`](ports/libvncserver/) | 0.9.15 | zlib lzo libjpeg-turbo libpng gnutls openssl sdl2 | gnutls libjpeg-turbo libpng lzo openssl sdl2 zlib | 17 |
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
| [`libxfce4windowing`](ports/libxfce4windowing/) | 4.20.7 | glib gtk3 wayland wayland-protocols | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango wayland zlib | 47 |
| [`libxfixes`](ports/libxfixes/) | 6.0.1 | libx11 xorgproto | libx11 | 4 |
| [`libxfont2`](ports/libxfont2/) | 2.0.7 | freetype zlib fontenc xorgproto xtrans | fontenc freetype zlib | 6 |
| [`libxft`](ports/libxft/) | 2.3.8 | libx11 libxrender freetype fontconfig xorgproto | fontconfig freetype libx11 libxrender | 12 |
| [`libxi`](ports/libxi/) | 1.8.2 | libx11 libxext libxfixes xorgproto | libx11 libxext | 5 |
| [`libxinerama`](ports/libxinerama/) | 1.1.5 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxkbcommon`](ports/libxkbcommon/) | 1.8.1 | libxml2 | libxml2 | 7 |
| [`libxkbfile`](ports/libxkbfile/) | 1.2.0 | libx11 xorgproto | libx11 | 4 |
| [`libxml2`](ports/libxml2/) | 2.13.5 | zlib xz python3 | xz zlib | 6 |
| [`libxml2-utils`](ports/libxml2-utils/) | 2.13.5 | libxml2 zlib xz | libxml2 | 7 |
| [`libxmlb`](ports/libxmlb/) | 0.3.24 | glib xz zstd | glib xz zstd | 10 |
| [`libxmu`](ports/libxmu/) | 1.3.1 | libx11 libxext libxt xorgproto | libx11 libxext libxt | 8 |
| [`libxrandr`](ports/libxrandr/) | 1.5.4 | libx11 libxext libxrender xorgproto | libx11 libxext libxrender | 6 |
| [`libxrender`](ports/libxrender/) | 0.9.11 | libx11 xorgproto | libx11 | 4 |
| [`libxscrnsaver`](ports/libxscrnsaver/) | 1.2.4 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxslt`](ports/libxslt/) | 1.1.42 | libxml2 zlib | libxml2 | 7 |
| [`libxt`](ports/libxt/) | 1.3.1 | libx11 libsm libice xorgproto | libice libsm libx11 | 6 |
| [`libxtst`](ports/libxtst/) | 1.2.5 | libx11 libxext libxfixes libxi xorgproto | libx11 libxext libxi | 6 |
| [`libyuv`](ports/libyuv/) | stable | libjpeg-turbo | libjpeg-turbo | 1 |
| [`libzip`](ports/libzip/) | 1.11.1 | zlib bzip2 xz zstd openssl | bzip2 openssl xz zlib zstd | 9 |
| [`libzmq`](ports/libzmq/) | 4.3.5 | libsodium | libsodium | 1 |
| [`lighttpd`](ports/lighttpd/) | 1.4.77 | openssl pcre2 zlib | libxcrypt openssl pcre2 zlib | 4 |
| [`llama.cpp`](ports/llama.cpp/) | 0_b9006 | – | openssl | 1 |
| [`lldpd`](ports/lldpd/) | 1.0.22 | libevent | libevent | 2 |
| [`llvm`](ports/llvm/) | 22.1.8 | zlib zstd libffi libxml2 | libxml2 zlib zstd | 8 |
| [`log4cxx`](ports/log4cxx/) | 1.8.0 | apr apr-util expat libiconv | apr apr-util expat libiconv | 8 |
| [`luv`](ports/luv/) | 1.51.0-1 | libuv luajit | libuv | 1 |
| [`m4`](ports/m4/) | 1.4.19 | libiconv | libiconv | 1 |
| [`mame`](ports/mame/) | 0.281 | sdl2 sdl2-ttf fontconfig freetype zlib | fontconfig freetype sdl2 sdl2-ttf zlib | 19 |
| [`man-db`](ports/man-db/) | 2.13.1 | libpipeline gdbm groff libiconv zlib | gdbm libiconv libpipeline zlib | 4 |
| [`mate-calc`](ports/mate-calc/) | 1.28.0 | glib gtk3 libxml2 gmp mpfr mpc | at-spi2-core cairo gdk-pixbuf gettext glib gmp gtk3 harfbuzz libxml2 mpc mpfr pango zlib | 50 |
| [`mesa`](ports/mesa/) | 26.2.6 | llvm wayland wayland-protocols zlib zstd expat | expat llvm wayland zlib zstd | 12 |
| [`metis`](ports/metis/) | 5.2.1 | gklib | gklib | 1 |
| [`minicom`](ports/minicom/) | 2.9 | ncurses libiconv | libiconv ncurses | 2 |
| [`mkfontscale`](ports/mkfontscale/) | 1.2.3 | freetype fontenc zlib | fontenc freetype zlib | 6 |
| [`mosh`](ports/mosh/) | 1.4.0 | protobuf ncurses openssl zlib | abseil ncurses openssl protobuf zlib | 5 |
| [`mosquitto`](ports/mosquitto/) | 2.0.20 | openssl cjson | cjson openssl | 2 |
| [`mpc`](ports/mpc/) | 1.3.1 | gmp mpfr | gmp mpfr | 2 |
| [`mpfr`](ports/mpfr/) | 4.2.1 | gmp | gmp | 1 |
| [`nano`](ports/nano/) | 8.2 | ncurses zlib file | file ncurses zlib | 9 |
| [`neon`](ports/neon/) | 0.37.1 | openssl expat zlib | expat openssl zlib | 3 |
| [`neovim`](ports/neovim/) | 0.12.3 | luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext | gettext libiconv luv tree-sitter unibilium utf8proc | 9 |
| [`nettle`](ports/nettle/) | 3.10.1 | gmp | gmp | 1 |
| [`nginx`](ports/nginx/) | 1.30.6 | openssl pcre2 zlib libxcrypt | libxcrypt openssl pcre2 zlib | 4 |
| [`nodejs`](ports/nodejs/) | 24.14.1 | libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu | ada brotli c-ares icu libuv nghttp2 openssl simdjson simdutf sqlite zlib zstd | 12 |
| [`nss`](ports/nss/) | 3.131.1 | nspr sqlite zlib | nspr sqlite zlib | 3 |
| [`ntp`](ports/ntp/) | 4.2.8p18 | libevent libedit | libedit libevent | 4 |
| [`ntpsec`](ports/ntpsec/) | 1.2.4 | openssl | openssl | 1 |
| [`numpy`](ports/numpy/) | 2.3.4 | python3 openblas | openblas python3 | 16 |
| [`onnx`](ports/onnx/) | 1.17.0 | protobuf | abseil protobuf | 3 |
| [`onnxruntime`](ports/onnxruntime/) | 1.20.1 | protobuf | zlib | 1 |
| [`open62541`](ports/open62541/) | 1.4.8 | openssl | openssl | 1 |
| [`opencv`](ports/opencv/) | 4.10.0 | zlib eigen | zlib | 1 |
| [`openssh`](ports/openssh/) | 10.2p1 | openssl zlib libxcrypt | libxcrypt openssl zlib | 3 |
| [`openttd`](ports/openttd/) | 15.0-git-20250501 | sdl2 zlib xz libpng freetype fontconfig curl lzo | curl freetype libpng lzo sdl2 xz zlib | 17 |
| [`opusfile`](ports/opusfile/) | 0.12 | opus libogg openssl | libogg openssl opus | 3 |
| [`p11-kit`](ports/p11-kit/) | 0.25.5 | libffi libtasn1 | libffi | 1 |
| [`paho.mqtt.c`](ports/paho.mqtt.c/) | 1.3.14 | openssl | openssl | 1 |
| [`paho.mqtt.cpp`](ports/paho.mqtt.cpp/) | 1.5.2 | paho.mqtt.c | openssl paho.mqtt.c | 2 |
| [`pandas`](ports/pandas/) | 2.3.3 | numpy python3-dateutil python3-pytz python3-tzdata | python3 | 15 |
| [`pango`](ports/pango/) | 1.54.0 | glib harfbuzz fribidi cairo fontconfig freetype libxft | cairo fontconfig freetype fribidi glib harfbuzz libx11 libxft libxrender | 28 |
| [`pcl`](ports/pcl/) | 1.14.1 | boost eigen flann | boost flann lz4 | 10 |
| [`perl`](ports/perl/) | 5.40.2 | libxcrypt | libxcrypt | 1 |
| [`pistache`](ports/pistache/) | 0.4.26 | libevent rapidjson openssl | libevent openssl | 2 |
| [`poco`](ports/poco/) | 1.15.4 | openssl zlib pcre2 utf8proc expat | expat openssl pcre2 utf8proc zlib | 5 |
| [`poppler`](ports/poppler/) | 25.12.0 | freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib | cairo fontconfig freetype gettext glib lcms2 libiconv libjpeg-turbo libpng libtiff openjpeg zlib | 30 |
| [`pqiv`](ports/pqiv/) | 2.13.2 | glib gtk3 gdk-pixbuf cairo | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libx11 pango zlib | 47 |
| [`protobuf`](ports/protobuf/) | 29.3 | zlib abseil | abseil zlib | 2 |
| [`pygame`](ports/pygame/) | 2.6.1_git20251005 | python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo | freetype libjpeg-turbo libpng sdl2 sdl2-image sdl2-mixer sdl2-ttf | 31 |
| [`python3`](ports/python3/) | 3.14.0 | zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd | bzip2 expat gdbm gettext libffi ncurses openssl readline sqlite xz zlib zstd | 14 |
| [`python3-contourpy`](ports/python3-contourpy/) | 1.4.0 | numpy | python3 | 15 |
| [`python3-lxml`](ports/python3-lxml/) | 6.1.3 | python3 libxml2 libxslt zlib | libxml2 libxslt zlib | 8 |
| [`python3-matplotlib`](ports/python3-matplotlib/) | 3.11.2 | numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil | freetype libraqm python3 qhull | 26 |
| [`python3-pillow`](ports/python3-pillow/) | 13.0.0 | python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm | freetype lcms2 libjpeg-turbo libraqm libtiff libwebp openjpeg zlib | 24 |
| [`pytorch`](ports/pytorch/) | 2.3.1 | python3 protobuf openblas | abseil openblas protobuf | 4 |
| [`qdirstat`](ports/qdirstat/) | 2.0 | qt6 zlib gettext | gettext qt6 zlib | 54 |
| [`qpdf`](ports/qpdf/) | 11.10.1 | zlib libjpeg-turbo openssl | libjpeg-turbo openssl zlib | 3 |
| [`qt6`](ports/qt6/) | 6.8.1 | zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd | brotli double-conversion ffmpeg fontconfig freetype glib gst-plugins-bad gst-plugins-base gstreamer harfbuzz icu libjpeg-turbo libpng libtiff libwebp openssl pcre2 zlib zstd | 53 |
| [`quake3`](ports/quake3/) | 1.36-git-20261012 | sdl2 | sdl2 | 1 |
| [`raylib`](ports/raylib/) | 6.2 | sdl2 | sdl2 | 1 |
| [`re2`](ports/re2/) | 2024-07-02 | abseil | abseil | 1 |
| [`redis`](ports/redis/) | 8.10.3 | openssl | openssl | 1 |
| [`restbed`](ports/restbed/) | 4.8 | openssl asio | openssl | 1 |
| [`retroarch`](ports/retroarch/) | 1.15-git-20230408 | sdl2 freetype zlib fontconfig fribidi libass openssl | fontconfig freetype fribidi libass libxkbcommon openssl zlib | 23 |
| [`riegeli`](ports/riegeli/) | 0_git20231212 | abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash | abseil brotli bzip2 highwayhash lz4 protobuf snappy xz zlib zstd | 14 |
| [`rocksdb`](ports/rocksdb/) | 11.8.1 | lz4 zlib zstd | lz4 zlib zstd | 3 |
| [`ros2`](ports/ros2/) | jazzy | python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2 | benchmark console_bridge curl cyclonedds fastcdr fastdds libyaml lz4 opencv python3 sqlite tinyxml2 yaml-cpp zstd | 30 |
| [`rsync`](ports/rsync/) | 3.3.0 | zstd lz4 xxhash openssl | lz4 openssl xxhash zstd | 4 |
| [`rtl-sdr`](ports/rtl-sdr/) | 2.0.2 | libusb | libusb | 1 |
| [`s2n-tls`](ports/s2n-tls/) | 1.7.0 | openssl | openssl | 1 |
| [`sdl2-image`](ports/sdl2-image/) | 2.8.8 | sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl | libavif libtiff libwebp sdl2 | 14 |
| [`sdl2-mixer`](ports/sdl2-mixer/) | 2.8.2 | sdl2 opusfile | opusfile sdl2 | 5 |
| [`sdl2-net`](ports/sdl2-net/) | 2.2.0 | sdl2 | sdl2 | 1 |
| [`sdl2-ttf`](ports/sdl2-ttf/) | 2.24.0 | sdl2 freetype harfbuzz | freetype harfbuzz sdl2 | 16 |
| [`sdl3`](ports/sdl3/) | 3.4.18 | libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst | libiconv libx11 libxcursor libxext libxfixes libxi libxrandr libxrender libxscrnsaver libxtst | 13 |
| [`sentencepiece`](ports/sentencepiece/) | 0.2.1 | protobuf abseil | abseil protobuf | 3 |
| [`shared-mime-info`](ports/shared-mime-info/) | 2.4 | glib libxml2 | glib libxml2 | 10 |
| [`silkit-adapters-tap`](ports/silkit-adapters-tap/) | 1.2.0 | silkit | silkit | 1 |
| [`socat`](ports/socat/) | 1.8.0.3 | openssl readline | ncurses openssl readline | 3 |
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
| [`up-core-api`](ports/up-core-api/) | 1.6.0-alpha2 | protobuf | abseil protobuf | 3 |
| [`up-cpp`](ports/up-cpp/) | 1.0.1 | protobuf up-core-api | abseil protobuf up-core-api | 4 |
| [`up-transport-zenoh-cpp`](ports/up-transport-zenoh-cpp/) | 1.0.0-git-20250319 | zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog | abseil fmt protobuf spdlog up-core-api up-cpp zenoh-c | 8 |
| [`urdfdom`](ports/urdfdom/) | 4.0.3 | urdfdom_headers console_bridge tinyxml2 | console_bridge tinyxml2 | 2 |
| [`vim`](ports/vim/) | 9.1.0900 | ncurses libiconv libsodium | libiconv libsodium ncurses | 3 |
| [`virtualgl`](ports/virtualgl/) | 3.1.5 | libx11 libxext libjpeg-turbo | libjpeg-turbo libx11 libxext | 6 |
| [`vlc`](ports/vlc/) | 3.0.21 | bzip2 libiconv lz4 openssl xz zstd | bzip2 libiconv lz4 openssl xz zstd | 9 |
| [`vsomeip`](ports/vsomeip/) | 3.5.6 | boost | boost | 8 |
| [`vte`](ports/vte/) | 0.78.2 | gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4 | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gnutls graphene gtk3 gtk4 icu lz4 pango pcre2 | 58 |
| [`wayland`](ports/wayland/) | 1.24.0 | libffi expat libxml2 | libffi | 1 |
| [`webkitgtk`](ports/webkitgtk/) | 2.44.4 | gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret | at-spi2-core cairo fontconfig freetype gdk-pixbuf gettext glib gst-plugins-bad gst-plugins-base gstreamer gtk3 harfbuzz hyphen icu lcms2 libavif libepoxy libgcrypt libgpg-error libjpeg-turbo libjxl libpng libsecret libsoup libtasn1 libwebp libxml2 libxslt pango sqlite woff2 zlib | 77 |
| [`wget`](ports/wget/) | 1.25.0 | gnutls libidn2 zlib pcre2 libpsl | gnutls libiconv libpsl libunistring nettle pcre2 zlib | 14 |
| [`wlroots`](ports/wlroots/) | 0.20.2 | wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes | lcms2 libxkbcommon pixman wayland | 12 |
| [`woff2`](ports/woff2/) | 1.0.2 | brotli | brotli | 1 |
| [`xcb-util`](ports/xcb-util/) | 0.4.1 | libxcb | libxcb | 3 |
| [`xfce4-appfinder`](ports/xfce4-appfinder/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf garcon | cairo garcon gdk-pixbuf glib gtk3 libxfce4ui libxfce4util xfconf | 51 |
| [`xfce4-panel`](ports/xfce4-panel/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util libxfce4windowing pango wayland xfconf zlib | 54 |
| [`xfce4-session`](ports/xfce4-session/) | 4.20.2 | glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util libxfce4windowing pango xfconf | 52 |
| [`xfce4-settings`](ports/xfce4-settings/) | 4.20.1 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util pango wayland xfconf zlib | 53 |
| [`xfce4-terminal`](ports/xfce4-terminal/) | 1.1.5 | glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util pango pcre2 vte xfconf | 63 |
| [`xfconf`](ports/xfconf/) | 4.20.0 | glib libxfce4util dbus | gettext glib libxfce4util | 9 |
| [`xkbcomp`](ports/xkbcomp/) | 1.5.0 | libx11 libxkbfile xorgproto | libx11 libxkbfile | 5 |
| [`xnnpack`](ports/xnnpack/) | 0-git-20250209 | cpuinfo pthreadpool fxdiv | pthreadpool | 1 |
| [`xsltproc`](ports/xsltproc/) | 1.1.42 | libxslt libxml2 | libxml2 libxslt | 8 |
| [`xwayland`](ports/xwayland/) | 24.1.14 | wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util | libmd libxau libxcvt libxfont2 pixman wayland | 13 |
| [`xz`](ports/xz/) | 5.6.3 | gettext | gettext | 4 |
| [`zsh`](ports/zsh/) | 5.9 | ncurses pcre2 libiconv | libiconv ncurses | 2 |

## Consistency

| Component | Run-time dependency not declared in DEPS | Unresolved soname | Linked by the -dev name |
|---|---|---|---|
| [`cups-filters`](ports/cups-filters/) | libexif | – | – |
| [`graphicsmagick`](ports/graphicsmagick/) | libzip | – | – |
| [`lighttpd`](ports/lighttpd/) | libxcrypt | – | – |
| [`llama.cpp`](ports/llama.cpp/) | openssl | – | – |

## Blocked

1 components tried and not buildable for QNX as things stand (`STATUS=blocked`; each recipe's comment says what stops it): [`weston`](ports/weston/)

---
<sub>Generated by <code>scripts/gen-dependency-index</code>. Edit recipes, not this file.</sub>
