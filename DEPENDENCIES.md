# Dependencies

Two kinds, by whether anything else from this repository is needed:

| Kind | Meaning | Components |
|---|---|---|
| 🟢 **Standalone** | builds with the QNX SDP alone and runs on a stock QNX image -- port it on its own | **321** |
| 🔴 **Dependent** | needs other components built first (listed per component, in build order) | **633** |

Dependent components split further by what they need at run time (255 run on their own once built). Each component's README (`ports/<name>/README.md`) walks through porting it by hand from the upstream source -- download, QNX patches, configure, build, install, package, test -- with its dependencies first when it has any.

Every component is in exactly one of three classes. The class is computed by `scripts/gen-dependency-index` from the recipes *and* from the binaries the build installed (their `DT_NEEDED` entries), so it describes what actually happens on the target, not only what the recipe declares.

| Class | Build | Run on the target | Components |
|---|---|---|---|
| **[Standalone](#standalone)** | QNX SDP only | stock QNX image, nothing else | **321** |
| **[Runtime-standalone](#runtime-standalone)** | needs other components first | on its own | **255** |
| **[Dependent](#dependent)** | needs other components first | needs other components installed | **378** |

Libraries that ship with the QNX SDP target (libc, libm, libsocket, libregex, libc++, the eventfd/epoll shims, ...) are part of every QNX 8.0 system and are never counted as dependencies.

On a target, `oss-get install <component>` installs the run-time closure automatically; the *runtime* column below is what it pulls in.

Machine-readable: [`ports/dependencies.json`](ports/dependencies.json).

## Standalone

321 components. Clone the upstream source, apply the patches in `ports/<name>/patches/`, build with the QNX toolchain files -- nothing else from this repository has to be built first, and the result runs on a stock QNX 8.0 target.

✅ = built against an *empty* staging tree by `scripts/verify-standalone`, proving the build cannot have picked anything up from other components.

| Component | Version | Build system | Verified alone | Used by |
|---|---|---|---|---|
| [`7zip`](ports/7zip/) | 26.06 | custom | – | 0 |
| [`abseil`](ports/abseil/) | 20260817.0 | cmake | – | 7 |
| [`acado`](ports/acado/) | 1.2.2_beta | cmake | – | 0 |
| [`ada`](ports/ada/) | 4.1.1 | cmake | – | 1 |
| [`adwaita-fonts`](ports/adwaita-fonts/) | 49.0 | custom | – | 0 |
| [`aom`](ports/aom/) | 3.15.3 | cmake | – | 3 |
| [`apriltag`](ports/apriltag/) | 3.4.5 | cmake | – | 0 |
| [`asio`](ports/asio/) | 1-39-0 | custom | – | 1 |
| [`autoconf`](ports/autoconf/) | 2.73 | autotools | – | 0 |
| [`automake`](ports/automake/) | 1.20 | autotools | – | 0 |
| [`aws-c-common`](ports/aws-c-common/) | 0.12.6 | cmake | – | 5 |
| [`bash-completion`](ports/bash-completion/) | 2.18.0 | autotools | – | 0 |
| [`benchmark`](ports/benchmark/) | 1.10.0 | cmake | – | 1 |
| [`bison`](ports/bison/) | 3.8.2 | autotools | – | 0 |
| [`boringssl`](ports/boringssl/) | 0.20261105.0 | cmake | – | 0 |
| [`brotli`](ports/brotli/) | 1.2.1 | cmake | – | 11 |
| [`btop`](ports/btop/) | 1.4.8 | cmake | – | 0 |
| [`bullet3`](ports/bullet3/) | 3.25 | cmake | – | 1 |
| [`busybox`](ports/busybox/) | 1.38.0 | custom | – | 0 |
| [`bzip2`](ports/bzip2/) | 1.0.8 | custom | – | 16 |
| [`c-ares`](ports/c-ares/) | 1.34.9 | cmake | – | 4 |
| [`ca-certificates`](ports/ca-certificates/) | 20261020 | custom | – | 0 |
| [`capicxx-core-runtime`](ports/capicxx-core-runtime/) | 3.2.4 | cmake | – | 1 |
| [`capstone`](ports/capstone/) | 5.0.9 | cmake | – | 0 |
| [`casadi`](ports/casadi/) | 3.8.2 | cmake | – | 0 |
| [`catch2`](ports/catch2/) | 3.16.1 | cmake | – | 0 |
| [`cereal`](ports/cereal/) | 1.3.2 | cmake | – | 1 |
| [`check`](ports/check/) | 0.15.2 | cmake | – | 0 |
| [`chrony`](ports/chrony/) | 4.9 | custom | – | 0 |
| [`chrpath`](ports/chrpath/) | 0.18 | autotools | – | 0 |
| [`cjson`](ports/cjson/) | 1.7.19 | cmake | – | 1 |
| [`clapack`](ports/clapack/) | 3.2.1 | cmake | – | 0 |
| [`cli11`](ports/cli11/) | 2.7.2 | cmake | – | 0 |
| [`compiler-rt`](ports/compiler-rt/) | 22.1.8 | cmake | – | 0 |
| [`console_bridge`](ports/console_bridge/) | 1.0.2 | cmake | – | 3 |
| [`cpuinfo`](ports/cpuinfo/) | 0-git-20240417 | cmake | – | 2 |
| [`csmith`](ports/csmith/) | 2.3.0 | cmake | – | 0 |
| [`cunit`](ports/cunit/) | 2.1-3 | custom | – | 0 |
| [`cxxopts`](ports/cxxopts/) | 3.3.1 | cmake | – | 0 |
| [`cyclonedds`](ports/cyclonedds/) | 0.10.5 | cmake | – | 1 |
| [`datefudge`](ports/datefudge/) | 1.27 | custom | – | 0 |
| [`dav1d`](ports/dav1d/) | 1.5.5 | meson | – | 3 |
| [`davici`](ports/davici/) | 1.7 | autotools | – | 0 |
| [`dhcpcd`](ports/dhcpcd/) | 10.5.2 | custom | – | 0 |
| [`dnsmasq`](ports/dnsmasq/) | 2.93test4 | custom | – | 0 |
| [`dnssec-root`](ports/dnssec-root/) | 20241105 | custom | – | 0 |
| [`docbook-xml`](ports/docbook-xml/) | 4.5 | custom | – | 0 |
| [`docbook-xsl`](ports/docbook-xsl/) | 1.79.2 | custom | – | 0 |
| [`doctest`](ports/doctest/) | 2.5.3 | cmake | – | 0 |
| [`double-conversion`](ports/double-conversion/) | 3.4.0 | cmake | – | 1 |
| [`duktape`](ports/duktape/) | 2.7.0 | custom | – | 0 |
| [`ed`](ports/ed/) | 1.22.6 | autotools | – | 0 |
| [`eigen`](ports/eigen/) | 3.4.0 | cmake | – | 16 |
| [`encodings`](ports/encodings/) | 1.1.0 | autotools | – | 0 |
| [`espeak-ng`](ports/espeak-ng/) | 1.52.0 | cmake | – | 1 |
| [`expat`](ports/expat/) | 2.6.4 | cmake | – | 19 |
| [`fakeroot`](ports/fakeroot/) | 1.37.1.1 | autotools | – | 0 |
| [`farmhash`](ports/farmhash/) | 1.1-git-20190513 | autotools | – | 0 |
| [`fast-float`](ports/fast-float/) | 8.3.3 | cmake | – | 1 |
| [`fastcdr`](ports/fastcdr/) | 2.2.6 | cmake | – | 2 |
| [`fastfetch`](ports/fastfetch/) | 2.66.1 | cmake | – | 0 |
| [`fdk-aac`](ports/fdk-aac/) | 3.0.0 | cmake | – | 1 |
| [`fft2d`](ports/fft2d/) | 2006.12.28 | custom | – | 0 |
| [`fftw`](ports/fftw/) | 3.3.10 | autotools | – | 0 |
| [`flatbuffers`](ports/flatbuffers/) | 24.3.25 | cmake | – | 0 |
| [`flex`](ports/flex/) | 2.7.3 | autotools | – | 0 |
| [`fmt`](ports/fmt/) | 11.2.0 | cmake | – | 5 |
| [`font-cantarell`](ports/font-cantarell/) | 0.303.1 | custom | – | 0 |
| [`font-dejavu`](ports/font-dejavu/) | 2.37 | custom | – | 1 |
| [`font-hack`](ports/font-hack/) | 3.003 | custom | – | 0 |
| [`font-inconsolata`](ports/font-inconsolata/) | 3.000 | custom | – | 0 |
| [`font-inter`](ports/font-inter/) | 4.1 | custom | – | 0 |
| [`font-liberation`](ports/font-liberation/) | 2.1.5 | custom | – | 0 |
| [`font-noto-emoji`](ports/font-noto-emoji/) | 2.051 | custom | – | 0 |
| [`font-noto-sans-cjk`](ports/font-noto-sans-cjk/) | 2.004 | custom | – | 0 |
| [`font-noto-serif-cjk`](ports/font-noto-serif-cjk/) | 2.003 | custom | – | 0 |
| [`font-util`](ports/font-util/) | 1.4.2 | autotools | – | 1 |
| [`foonathan_memory`](ports/foonathan_memory/) | 0.7-4 | cmake | – | 1 |
| [`fp16`](ports/fp16/) | 0-git-20260620 | cmake | – | 0 |
| [`freeimage`](ports/freeimage/) | 3.18.0 | custom | – | 1 |
| [`fribidi`](ports/fribidi/) | 1.0.18 | meson | – | 8 |
| [`fxdiv`](ports/fxdiv/) | 0-git-20200417 | cmake | – | 2 |
| [`g3log`](ports/g3log/) | 2.6.2 | cmake | – | 0 |
| [`gdbm`](ports/gdbm/) | 1.26 | autotools | – | 2 |
| [`gemmlowp`](ports/gemmlowp/) | 0-git-20231103 | custom | – | 0 |
| [`geographiclib`](ports/geographiclib/) | 2.7 | cmake | – | 2 |
| [`geoip`](ports/geoip/) | 1.6.12 | autotools | – | 0 |
| [`gflags`](ports/gflags/) | 2.3.1 | cmake | – | 4 |
| [`giflib`](ports/giflib/) | 6.1.3 | custom | – | 2 |
| [`gklib`](ports/gklib/) | 5.1.1-git-20230326 | cmake | – | 1 |
| [`glm`](ports/glm/) | 1.0.3 | cmake | – | 0 |
| [`gloo`](ports/gloo/) | 0_git20261104 | cmake | – | 0 |
| [`gmp`](ports/gmp/) | 6.3.0 | autotools | – | 8 |
| [`googletest`](ports/googletest/) | 1.18.0 | cmake | – | 0 |
| [`gperf`](ports/gperf/) | 3.3 | autotools | – | 0 |
| [`graphite2`](ports/graphite2/) | 1.3.15 | cmake | – | 2 |
| [`gsm`](ports/gsm/) | 1.0.24 | custom | – | 0 |
| [`gumbo-parser`](ports/gumbo-parser/) | 0.14.1 | meson | – | 1 |
| [`hailort`](ports/hailort/) | 4.14.0 | cmake | – | 0 |
| [`help2man`](ports/help2man/) | 1.49.3 | autotools | – | 0 |
| [`hicolor-icon-theme`](ports/hicolor-icon-theme/) | 0.18 | meson | – | 2 |
| [`highway`](ports/highway/) | 1.4.0 | cmake | – | 1 |
| [`highwayhash`](ports/highwayhash/) | 0_git20240418 | cmake | – | 1 |
| [`http-parser`](ports/http-parser/) | 2.9.4 | custom | – | 0 |
| [`hwdata`](ports/hwdata/) | 0.413 | custom | – | 0 |
| [`hyphen`](ports/hyphen/) | 2.8.8 | autotools | – | 1 |
| [`iceoryx`](ports/iceoryx/) | 2.95.8 | cmake | – | 0 |
| [`icu`](ports/icu/) | 76.1 | custom | – | 8 |
| [`imath`](ports/imath/) | 3.2.3 | cmake | – | 1 |
| [`intltool`](ports/intltool/) | 0.51.0 | autotools | – | 0 |
| [`iperf2`](ports/iperf2/) | 2.2.2 | autotools | – | 0 |
| [`iso-codes`](ports/iso-codes/) | 4.17.0 | autotools | – | 1 |
| [`isotp-c`](ports/isotp-c/) | 1.9.3 | custom | – | 0 |
| [`jansson`](ports/jansson/) | 2.15.1 | cmake | – | 0 |
| [`json-c`](ports/json-c/) | 0.19-20260627 | cmake | – | 0 |
| [`jsoncpp`](ports/jsoncpp/) | 1.9.8 | cmake | – | 0 |
| [`kineto`](ports/kineto/) | 0-git-20250214 | cmake | – | 0 |
| [`lely-core`](ports/lely-core/) | 2.3.5 | autotools | – | 0 |
| [`libaec`](ports/libaec/) | 1.1.8 | cmake | – | 1 |
| [`libasyncns`](ports/libasyncns/) | 0.8 | autotools | – | 0 |
| [`libb2`](ports/libb2/) | 0.98.1 | autotools | – | 0 |
| [`libcanard`](ports/libcanard/) | 5.0.0 | custom | – | 0 |
| [`libcbor`](ports/libcbor/) | 0.15.0 | cmake | – | 0 |
| [`libccd`](ports/libccd/) | 2.1 | cmake | – | 2 |
| [`libconfig`](ports/libconfig/) | 1.8.3 | cmake | – | 0 |
| [`libdaemon`](ports/libdaemon/) | 0.14 | autotools | – | 1 |
| [`libde265`](ports/libde265/) | 1.1.3 | cmake | – | 1 |
| [`libdeflate`](ports/libdeflate/) | 1.26 | cmake | – | 1 |
| [`libdisplay-info`](ports/libdisplay-info/) | 0.4.0 | meson | – | 0 |
| [`libdrm-headers`](ports/libdrm-headers/) | 2.4.129 | custom | – | 3 |
| [`libev`](ports/libev/) | 4.33 | autotools | – | 0 |
| [`libexif`](ports/libexif/) | 0.6.26 | autotools | – | 2 |
| [`libfdt`](ports/libfdt/) | 1.8.1 | meson | – | 0 |
| [`libffi`](ports/libffi/) | 3.8.1 | autotools | – | 9 |
| [`libfyaml`](ports/libfyaml/) | 0.9.6 | cmake | – | 1 |
| [`libhevc`](ports/libhevc/) | 1.8.7 | cmake | – | 0 |
| [`libiconv`](ports/libiconv/) | 1.19.1 | autotools | – | 26 |
| [`libinih`](ports/libinih/) | 58 | meson | – | 1 |
| [`libjpeg-turbo`](ports/libjpeg-turbo/) | 3.2.1 | cmake | – | 23 |
| [`libjson`](ports/libjson/) | 0.8-git-20260512 | custom | – | 1 |
| [`libmd`](ports/libmd/) | 1.3.0 | autotools | – | 1 |
| [`libmetal`](ports/libmetal/) | 2026.10.0 | cmake | – | 1 |
| [`libmicrohttpd`](ports/libmicrohttpd/) | 1.0.11 | autotools | – | 0 |
| [`libmodbus`](ports/libmodbus/) | 3.2.0 | autotools | – | 0 |
| [`libogg`](ports/libogg/) | 1.3.6 | cmake | – | 8 |
| [`libpaper`](ports/libpaper/) | 2.3.0 | autotools | – | 0 |
| [`libpcap`](ports/libpcap/) | 1.11.0 | autotools | – | 2 |
| [`libpipeline`](ports/libpipeline/) | 1.5.8 | autotools | – | 1 |
| [`libretro-2048`](ports/libretro-2048/) | 0-git-20241227 | custom | – | 1 |
| [`libretro-fake08`](ports/libretro-fake08/) | 0-git-20260613 | custom | – | 1 |
| [`libretro-mrboom`](ports/libretro-mrboom/) | 0-git-20260625 | custom | – | 1 |
| [`libretro-retro8`](ports/libretro-retro8/) | 0-git-20260912 | custom | – | 1 |
| [`libretro-samples`](ports/libretro-samples/) | 0-git-20221110 | custom | – | 1 |
| [`libsamplerate`](ports/libsamplerate/) | 0.2.2 | cmake | – | 0 |
| [`libsass`](ports/libsass/) | 3.6.6 | autotools | – | 1 |
| [`libsfdo`](ports/libsfdo/) | 0.1.4 | meson | – | 1 |
| [`libsodium`](ports/libsodium/) | 1.0.22 | autotools | – | 3 |
| [`libtasn1`](ports/libtasn1/) | 4.21.0 | autotools | – | 3 |
| [`libtommath`](ports/libtommath/) | 1.3.0 | cmake | – | 1 |
| [`libtool`](ports/libtool/) | 2.5.4 | autotools | – | 1 |
| [`libunibreak`](ports/libunibreak/) | 8.0.1 | autotools | – | 0 |
| [`libunwind`](ports/libunwind/) | 1.8.3 | autotools | – | 0 |
| [`libusb`](ports/libusb/) | 1.0.30 | autotools | – | 2 |
| [`libutempter`](ports/libutempter/) | 1.2.3 | custom | – | 0 |
| [`libuuid`](ports/libuuid/) | 2.42.4 | autotools | – | 2 |
| [`libuv`](ports/libuv/) | 1.53.0 | cmake | – | 4 |
| [`libvpx`](ports/libvpx/) | 1.17.0 | custom | – | 2 |
| [`libvterm`](ports/libvterm/) | 0.3.3 | custom | – | 0 |
| [`libxcrypt`](ports/libxcrypt/) | 4.5.2 | autotools | – | 10 |
| [`libxcvt`](ports/libxcvt/) | 0.1.3 | meson | – | 1 |
| [`libxo`](ports/libxo/) | 2.3.0 | autotools | – | 0 |
| [`libyaml`](ports/libyaml/) | 0.2.6 | cmake | – | 3 |
| [`linux-headers-input-codes`](ports/linux-headers-input-codes/) | 6.12 | custom | – | 5 |
| [`lmbench`](ports/lmbench/) | 3.0-git-20260929 | custom | – | 0 |
| [`log4cpp`](ports/log4cpp/) | 1.1.6 | autotools | – | 0 |
| [`lua`](ports/lua/) | 5.4.7 | custom | – | 0 |
| [`lua5.1`](ports/lua5.1/) | 5.1.5 | custom | – | 1 |
| [`lua5.2`](ports/lua5.2/) | 5.2.4 | custom | – | 1 |
| [`lua5.3`](ports/lua5.3/) | 5.3.6 | custom | – | 2 |
| [`luajit`](ports/luajit/) | 2.1.c6ffc141 | custom | – | 4 |
| [`luarocks`](ports/luarocks/) | 3.12.2 | custom | – | 0 |
| [`lxde-icon-theme`](ports/lxde-icon-theme/) | 0.5.2 | autotools | – | 1 |
| [`lz4`](ports/lz4/) | 1.11.1 | cmake | – | 11 |
| [`lzip`](ports/lzip/) | 1.26 | autotools | – | 0 |
| [`lzo`](ports/lzo/) | 2.10 | autotools | – | 3 |
| [`magic_enum`](ports/magic_enum/) | 0.9.8 | cmake | – | 0 |
| [`make`](ports/make/) | 4.4.1 | autotools | – | 1 |
| [`mate-common`](ports/mate-common/) | 1.28.0 | autotools | – | 0 |
| [`mbedtls`](ports/mbedtls/) | 3.6.2 | cmake | – | 0 |
| [`md4c`](ports/md4c/) | 0.6.0 | cmake | – | 0 |
| [`mimalloc`](ports/mimalloc/) | 3.7.0 | cmake | – | 0 |
| [`miniz`](ports/miniz/) | 3.1.2 | cmake | – | 0 |
| [`mnn`](ports/mnn/) | 3.6.2 | cmake | – | 0 |
| [`mpg123`](ports/mpg123/) | 1.33.7 | autotools | – | 0 |
| [`msgpack-c`](ports/msgpack-c/) | 6.1.0 | cmake | – | 1 |
| [`mujoco`](ports/mujoco/) | 3.10.0 | cmake | – | 0 |
| [`muslflt`](ports/muslflt/) | 1.0-git-20240802 | custom | – | 0 |
| [`nanoflann`](ports/nanoflann/) | 2.0.2 | cmake | – | 1 |
| [`nanomsg`](ports/nanomsg/) | 1.3.0 | cmake | – | 0 |
| [`nanosvg`](ports/nanosvg/) | 0-git-20260709 | cmake | – | 1 |
| [`nasm`](ports/nasm/) | 2.16.03 | custom | – | 0 |
| [`ncnn`](ports/ncnn/) | 20260526 | cmake | – | 0 |
| [`ncurses`](ports/ncurses/) | 6.6 | autotools | – | 25 |
| [`netmap`](ports/netmap/) | 14.0-git-20261027 | custom | – | 0 |
| [`ninja`](ports/ninja/) | 1.13.1 | cmake | – | 0 |
| [`nlohmann_json`](ports/nlohmann_json/) | 3.12.0 | cmake | – | 2 |
| [`nng`](ports/nng/) | 1.9.0 | cmake | – | 0 |
| [`npth`](ports/npth/) | 1.8 | autotools | – | 1 |
| [`nspr`](ports/nspr/) | 4.39 | autotools | – | 1 |
| [`nsync`](ports/nsync/) | 1.30.0 | cmake | – | 0 |
| [`ntttcp-for-linux`](ports/ntttcp-for-linux/) | 1.4.0-git-20230613 | cmake | – | 0 |
| [`octomap`](ports/octomap/) | 1.10.1 | cmake | – | 2 |
| [`onednn`](ports/onednn/) | 3.14.2 | cmake | – | 0 |
| [`onetbb`](ports/onetbb/) | 2022.0.0 | cmake | – | 1 |
| [`oniguruma`](ports/oniguruma/) | 6.9.10 | cmake | – | 1 |
| [`openal-soft`](ports/openal-soft/) | 1.24.1 | cmake | – | 0 |
| [`openblas`](ports/openblas/) | 0.3.34 | cmake | – | 5 |
| [`opencl-headers`](ports/opencl-headers/) | 2026.05.29 | cmake | – | 4 |
| [`openh264`](ports/openh264/) | 2.7.0 | meson | – | 3 |
| [`openmp`](ports/openmp/) | 22.1.8 | cmake | – | 0 |
| [`openpam`](ports/openpam/) | 20250531 | autotools | – | 0 |
| [`openresolv`](ports/openresolv/) | 3.17.4 | custom | – | 0 |
| [`openssl`](ports/openssl/) | 3.5.1 | custom | – | 61 |
| [`openttd-opengfx`](ports/openttd-opengfx/) | 8.0 | custom | – | 0 |
| [`openttd-openmsx`](ports/openttd-openmsx/) | 0.4.2 | custom | – | 0 |
| [`openttd-opensfx`](ports/openttd-opensfx/) | 1.0.3 | custom | – | 0 |
| [`opus`](ports/opus/) | 1.6.1 | cmake | – | 5 |
| [`orc`](ports/orc/) | 0.4.44 | meson | – | 1 |
| [`osqp`](ports/osqp/) | 0.6.3 | cmake | – | 1 |
| [`patch`](ports/patch/) | 2.8.1 | autotools | – | 0 |
| [`pax-utils`](ports/pax-utils/) | 1.3.9 | meson | – | 0 |
| [`pcre`](ports/pcre/) | 8.45 | autotools | – | 0 |
| [`pdfium`](ports/pdfium/) | 0-git-84a8011 | custom | – | 0 |
| [`picocom`](ports/picocom/) | 3.1 | custom | – | 0 |
| [`piper-voice-en-us-joe`](ports/piper-voice-en-us-joe/) | 1.0.0 | custom | – | 0 |
| [`pkgconf`](ports/pkgconf/) | 2.3.0 | meson | – | 1 |
| [`portaudio`](ports/portaudio/) | 19.7.0 | cmake | – | 1 |
| [`protobuf-c`](ports/protobuf-c/) | 1.5.2 | cmake | – | 0 |
| [`pthread-stubs`](ports/pthread-stubs/) | 0.5 | autotools | – | 1 |
| [`ptpd`](ports/ptpd/) | 2.3.1 | autotools | – | 0 |
| [`publicsuffix-list`](ports/publicsuffix-list/) | 20261031 | custom | – | 0 |
| [`pugixml`](ports/pugixml/) | 1.16 | cmake | – | 2 |
| [`pybind11`](ports/pybind11/) | 3.1.0 | cmake | – | 1 |
| [`qhull`](ports/qhull/) | 8.0.2 | cmake | – | 2 |
| [`qnx-socketcan`](ports/qnx-socketcan/) | 1.0 | custom | – | 4 |
| [`quickjs-ng`](ports/quickjs-ng/) | 0.18.0 | cmake | – | 0 |
| [`rapidjson`](ports/rapidjson/) | 1.1.0-24b5e7a | cmake | – | 2 |
| [`rhash`](ports/rhash/) | 1.4.6 | custom | – | 0 |
| [`ruckig`](ports/ruckig/) | 0.19.4 | cmake | – | 1 |
| [`rust`](ports/rust/) | 1.90.0 | custom | – | 2 |
| [`scdoc`](ports/scdoc/) | 1.11.3 | custom | – | 0 |
| [`silkit`](ports/silkit/) | 5.0.8 | cmake | – | 1 |
| [`simdjson`](ports/simdjson/) | 3.11.3 | cmake | – | 1 |
| [`simdutf`](ports/simdutf/) | 9.4.0 | cmake | – | 1 |
| [`skalibs`](ports/skalibs/) | 2.16.0.0 | custom | – | 0 |
| [`sleef`](ports/sleef/) | 3.9.0 | cmake | – | 0 |
| [`snappy`](ports/snappy/) | 1.3.1 | cmake | – | 3 |
| [`soxr`](ports/soxr/) | 0.1.3 | cmake | – | 0 |
| [`speexdsp`](ports/speexdsp/) | 1.2.1 | autotools | – | 0 |
| [`spirv-headers`](ports/spirv-headers/) | 1.4.321.0 | cmake | – | 3 |
| [`sshpass`](ports/sshpass/) | 1.10 | autotools | – | 0 |
| [`stable-diffusion.cpp`](ports/stable-diffusion.cpp/) | 0_git20260601 | cmake | – | 0 |
| [`stringencoders`](ports/stringencoders/) | 3.10.3-git-20180306 | autotools | – | 0 |
| [`svt-av1`](ports/svt-av1/) | 2.3.0 | cmake | – | 1 |
| [`tdb`](ports/tdb/) | 1.4.15 | custom | – | 1 |
| [`tensorflow-lite`](ports/tensorflow-lite/) | 2.16.1 | custom | – | 0 |
| [`tinyxml`](ports/tinyxml/) | 2.6.2 | custom | – | 0 |
| [`tinyxml2`](ports/tinyxml2/) | 11.0.0 | cmake | – | 7 |
| [`tl-expected`](ports/tl-expected/) | 1.3.1 | cmake | – | 2 |
| [`tllist`](ports/tllist/) | 1.1.0 | meson | – | 2 |
| [`tomlplusplus`](ports/tomlplusplus/) | 3.5.1 | cmake | – | 0 |
| [`toybox`](ports/toybox/) | 0.8.15 | custom | – | 0 |
| [`tree`](ports/tree/) | 2.3.2 | custom | – | 0 |
| [`tree-sitter`](ports/tree-sitter/) | 0.27.2 | cmake | – | 2 |
| [`tree-sitter-bash`](ports/tree-sitter-bash/) | 0.25.1 | custom | – | 0 |
| [`tree-sitter-c`](ports/tree-sitter-c/) | 0.24.2 | custom | – | 0 |
| [`tree-sitter-cmake`](ports/tree-sitter-cmake/) | 0.7.6 | custom | – | 0 |
| [`tree-sitter-cpp`](ports/tree-sitter-cpp/) | 0.23.4 | custom | – | 0 |
| [`tree-sitter-lua`](ports/tree-sitter-lua/) | 0.5.0 | custom | – | 0 |
| [`tree-sitter-make`](ports/tree-sitter-make/) | 1.1.1 | custom | – | 0 |
| [`tree-sitter-markdown`](ports/tree-sitter-markdown/) | 0.5.4 | custom | – | 0 |
| [`tree-sitter-python`](ports/tree-sitter-python/) | 0.25.0 | custom | – | 0 |
| [`tree-sitter-query`](ports/tree-sitter-query/) | 0.8.0 | custom | – | 0 |
| [`tree-sitter-vim`](ports/tree-sitter-vim/) | 0.8.1 | custom | – | 0 |
| [`tree-sitter-vimdoc`](ports/tree-sitter-vimdoc/) | 4.3.0 | custom | – | 0 |
| [`trompeloeil`](ports/trompeloeil/) | 49 | cmake | – | 0 |
| [`tsl-robin-map`](ports/tsl-robin-map/) | 1.4.1 | cmake | – | 1 |
| [`tzdata`](ports/tzdata/) | 2026f | custom | – | 0 |
| [`unibilium`](ports/unibilium/) | 2.1.4 | cmake | – | 2 |
| [`unifdef`](ports/unifdef/) | 2.12 | custom | – | 0 |
| [`urdfdom_headers`](ports/urdfdom_headers/) | 1.1.2 | cmake | – | 2 |
| [`uriparser`](ports/uriparser/) | 1.0.3 | cmake | – | 0 |
| [`utf8proc`](ports/utf8proc/) | 2.12.0 | cmake | – | 4 |
| [`util-macros`](ports/util-macros/) | 1.20.2 | autotools | – | 0 |
| [`uvgrtp`](ports/uvgrtp/) | 4.0.2 | cmake | – | 0 |
| [`vidstab`](ports/vidstab/) | 1.1.2 | cmake | – | 0 |
| [`webrtc`](ports/webrtc/) | m132-afaf497 | custom | – | 1 |
| [`whisper.cpp`](ports/whisper.cpp/) | 0_git20260728 | cmake | – | 0 |
| [`wlr-protocols`](ports/wlr-protocols/) | 0-git-20250702 | custom | – | 1 |
| [`wolfssl`](ports/wolfssl/) | 5.7.6 | cmake | – | 0 |
| [`x264`](ports/x264/) | stable | custom | – | 2 |
| [`x265`](ports/x265/) | 4.2 | cmake | – | 3 |
| [`xcb-proto`](ports/xcb-proto/) | 1.17.0 | autotools | – | 1 |
| [`xdg-utils`](ports/xdg-utils/) | 1.2.1 | autotools | – | 0 |
| [`xkeyboard-config`](ports/xkeyboard-config/) | 2.49 | meson | – | 1 |
| [`xmlto`](ports/xmlto/) | 0.0.29 | autotools | – | 0 |
| [`xorgproto`](ports/xorgproto/) | 2024.1 | meson | – | 25 |
| [`xsimd`](ports/xsimd/) | 14.3.0 | cmake | – | 2 |
| [`xtl`](ports/xtl/) | 0.8.2 | cmake | – | 2 |
| [`xtrans`](ports/xtrans/) | 1.6.0 | autotools | – | 6 |
| [`xvidcore`](ports/xvidcore/) | 1.3.7 | custom | – | 0 |
| [`xxhash`](ports/xxhash/) | 0.8.4 | cmake | – | 3 |
| [`yaml-cpp`](ports/yaml-cpp/) | 0.9.0 | cmake | – | 1 |
| [`zenoh-pico`](ports/zenoh-pico/) | 1.10.2 | cmake | – | 1 |
| [`zerotier-one`](ports/zerotier-one/) | 1.16.2 | custom | – | 0 |
| [`zimg`](ports/zimg/) | 3.0.6 | autotools | – | 0 |
| [`zip`](ports/zip/) | 3.0 | custom | – | 0 |
| [`zlib`](ports/zlib/) | 1.4 | cmake | – | 99 |
| [`zlib-ng`](ports/zlib-ng/) | 2.3.3 | cmake | – | 0 |
| [`zserio`](ports/zserio/) | 2.20.0 | cmake | – | 0 |
| [`zstd`](ports/zstd/) | 1.5.7 | cmake | – | 31 |

## Runtime-standalone

255 components. They need other components at *build* time -- header-only libraries (Eigen, nlohmann_json, ...), static archives, or generators -- but what they install loads nothing from this tree. Build the listed chain first (`qnxbuild` does it); deploy the component alone.

| Component | Version | Needs to build | Full build chain |
|---|---|---|---|
| [`adwaita-icon-theme`](ports/adwaita-icon-theme/) | 51.0 | hicolor-icon-theme | 1 |
| [`amqp-cpp`](ports/amqp-cpp/) | 4.3.27 | openssl | 1 |
| [`apr`](ports/apr/) | 1.7.6 | libxcrypt | 1 |
| [`asciidoc`](ports/asciidoc/) | 10.2.1 | python3 | 15 |
| [`candb`](ports/candb/) | 0.1-git-20210625 | tl-expected | 1 |
| [`cgal`](ports/cgal/) | 6.2.1 | gmp mpfr boost | 10 |
| [`computelibrary`](ports/computelibrary/) | 53.3.2 | opencl-headers | 1 |
| [`cppzmq`](ports/cppzmq/) | 4.11.0 | libzmq | 2 |
| [`cython`](ports/cython/) | 3.3.1 | python3 | 15 |
| [`ensmallen`](ports/ensmallen/) | 3.11.1 | armadillo | 2 |
| [`glfw`](ports/glfw/) | 3.5.1 | wayland libxkbcommon linux-headers-input-codes | 19 |
| [`glib-typelibs`](ports/glib-typelibs/) | 2.90.1 | glib | 9 |
| [`gsettings-desktop-schemas`](ports/gsettings-desktop-schemas/) | 49.0 | glib | 9 |
| [`gyp`](ports/gyp/) | 0.22.4 | python3 python3-packaging python3-setuptools | 17 |
| [`icon-naming-utils`](ports/icon-naming-utils/) | 0.8.90 | perl-xml-simple | 3 |
| [`itstool`](ports/itstool/) | 2.0.7 | python3 libxml2 | 16 |
| [`kiss-icp`](ports/kiss-icp/) | 1.3.0 | eigen sophus onetbb tsl-robin-map | 4 |
| [`lcms2`](ports/lcms2/) | 2.16 | zlib | 1 |
| [`libepoxy`](ports/libepoxy/) | 1.5.10 | libx11 | 8 |
| [`libice`](ports/libice/) | 1.1.2 | xorgproto xtrans | 2 |
| [`libwebp`](ports/libwebp/) | 1.6.0 | libpng libjpeg-turbo giflib | 4 |
| [`libxau`](ports/libxau/) | 1.0.12 | xorgproto | 1 |
| [`libxdmcp`](ports/libxdmcp/) | 1.1.5 | xorgproto | 1 |
| [`libxshmfence`](ports/libxshmfence/) | 1.3.3 | xorgproto | 1 |
| [`lua-compat53`](ports/lua-compat53/) | 0.14.4 | lua5.1 lua5.2 lua5.3 | 3 |
| [`lua-lpeg`](ports/lua-lpeg/) | 1.1.0 | luajit | 1 |
| [`mcap-cpp`](ports/mcap-cpp/) | 2.1.3 | lz4 zstd | 2 |
| [`meson`](ports/meson/) | 1.12.1 | python3 | 15 |
| [`mythes`](ports/mythes/) | 1.2.4 | hunspell | 2 |
| [`native-toolchain`](ports/native-toolchain/) | 1.0 | llvm binutils pkgconf make | 21 |
| [`nghttp2`](ports/nghttp2/) | 1.71.0 | zlib openssl | 2 |
| [`npm`](ports/npm/) | 11.11.0 | nodejs | 13 |
| [`open-amp`](ports/open-amp/) | 2026.10.0 | libmetal | 1 |
| [`opencl-icd-loader`](ports/opencl-icd-loader/) | 2026.05.29 | opencl-headers | 1 |
| [`openjpeg`](ports/openjpeg/) | 2.5.4 | zlib libpng libtiff lcms2 | 13 |
| [`orocos-kdl`](ports/orocos-kdl/) | 1.5.4 | eigen | 1 |
| [`oss-desktop`](ports/oss-desktop/) | 1.1 | labwc swaybg xfce4-panel xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder dbus adwaita-icon-theme hicolor-icon-theme lxde-icon-theme font-dejavu | 96 |
| [`pcre2`](ports/pcre2/) | 10.49 | zlib bzip2 | 2 |
| [`perl-b-cow`](ports/perl-b-cow/) | 0.007 | perl | 2 |
| [`perl-class-inspector`](ports/perl-class-inspector/) | 1.37_01 | perl | 2 |
| [`perl-clone`](ports/perl-clone/) | 0.51 | perl | 2 |
| [`perl-cpan-requirements-dynamic`](ports/perl-cpan-requirements-dynamic/) | 0.003 | perl | 2 |
| [`perl-encode-locale`](ports/perl-encode-locale/) | 1.05 | perl | 2 |
| [`perl-error`](ports/perl-error/) | 0.17031 | perl | 2 |
| [`perl-extutils-config`](ports/perl-extutils-config/) | 0.010 | perl | 2 |
| [`perl-extutils-hascompiler`](ports/perl-extutils-hascompiler/) | 0.025 | perl | 2 |
| [`perl-extutils-helpers`](ports/perl-extutils-helpers/) | 0.028 | perl | 2 |
| [`perl-extutils-installpaths`](ports/perl-extutils-installpaths/) | 0.015 | perl | 2 |
| [`perl-fcgi`](ports/perl-fcgi/) | 0.82 | perl | 2 |
| [`perl-file-listing`](ports/perl-file-listing/) | 6.16 | perl | 2 |
| [`perl-file-sharedir`](ports/perl-file-sharedir/) | 1.118 | perl | 2 |
| [`perl-file-sharedir-install`](ports/perl-file-sharedir-install/) | 0.14 | perl | 2 |
| [`perl-html-parser`](ports/perl-html-parser/) | 3.87 | perl | 2 |
| [`perl-html-tagset`](ports/perl-html-tagset/) | 3.24 | perl | 2 |
| [`perl-http-cookiejar`](ports/perl-http-cookiejar/) | 0.014 | perl | 2 |
| [`perl-http-cookies`](ports/perl-http-cookies/) | 6.12 | perl | 2 |
| [`perl-http-daemon`](ports/perl-http-daemon/) | 6.17 | perl | 2 |
| [`perl-http-date`](ports/perl-http-date/) | 6.08 | perl | 2 |
| [`perl-http-message`](ports/perl-http-message/) | 7.04 | perl | 2 |
| [`perl-http-negotiate`](ports/perl-http-negotiate/) | 6.01 | perl | 2 |
| [`perl-io-html`](ports/perl-io-html/) | 1.004 | perl | 2 |
| [`perl-io-socket-ssl`](ports/perl-io-socket-ssl/) | 2.099 | perl | 2 |
| [`perl-io-tty`](ports/perl-io-tty/) | 1.31 | perl | 2 |
| [`perl-libwww`](ports/perl-libwww/) | 6.83 | perl | 2 |
| [`perl-lwp-mediatypes`](ports/perl-lwp-mediatypes/) | 6.05 | perl | 2 |
| [`perl-mime-base32`](ports/perl-mime-base32/) | 1.303 | perl | 2 |
| [`perl-module-build`](ports/perl-module-build/) | 0.4234 | perl | 2 |
| [`perl-module-build-tiny`](ports/perl-module-build-tiny/) | 0.053 | perl | 2 |
| [`perl-mozilla-ca`](ports/perl-mozilla-ca/) | 20261105 | perl | 2 |
| [`perl-net-http`](ports/perl-net-http/) | 6.24 | perl | 2 |
| [`perl-protocol-websocket`](ports/perl-protocol-websocket/) | 0.26 | perl | 2 |
| [`perl-test-deep`](ports/perl-test-deep/) | 1.205 | perl | 2 |
| [`perl-test-fatal`](ports/perl-test-fatal/) | 0.019 | perl | 2 |
| [`perl-test-leaktrace`](ports/perl-test-leaktrace/) | 0.17 | perl | 2 |
| [`perl-test-needs`](ports/perl-test-needs/) | 0.002010 | perl | 2 |
| [`perl-test-requires`](ports/perl-test-requires/) | 0.11 | perl | 2 |
| [`perl-test-requiresinternet`](ports/perl-test-requiresinternet/) | 0.05 | perl | 2 |
| [`perl-test-warnings`](ports/perl-test-warnings/) | 0.038 | perl | 2 |
| [`perl-try-tiny`](ports/perl-try-tiny/) | 0.32 | perl | 2 |
| [`perl-uri`](ports/perl-uri/) | 5.40 | perl | 2 |
| [`perl-www-robotrules`](ports/perl-www-robotrules/) | 6.03 | perl | 2 |
| [`perl-xml-simple`](ports/perl-xml-simple/) | 2.25 | perl | 2 |
| [`pixman`](ports/pixman/) | 0.46.4 | libpng | 2 |
| [`psutil`](ports/psutil/) | 7.2.2 | python3 | 15 |
| [`pthreadpool`](ports/pthreadpool/) | 0-git-20250207 | fxdiv | 1 |
| [`python3-absl-py`](ports/python3-absl-py/) | 2.5.1 | python3 | 15 |
| [`python3-appdirs`](ports/python3-appdirs/) | 1.4.4 | python3 | 15 |
| [`python3-argcomplete`](ports/python3-argcomplete/) | 3.7.2 | python3 | 15 |
| [`python3-argparse-addons`](ports/python3-argparse-addons/) | 0.12.0 | python3 | 15 |
| [`python3-astor`](ports/python3-astor/) | 0.8.1 | python3 | 15 |
| [`python3-async-generator`](ports/python3-async-generator/) | 1.10 | python3 | 15 |
| [`python3-attrs`](ports/python3-attrs/) | 26.1.0 | python3 | 15 |
| [`python3-beniget`](ports/python3-beniget/) | 0.5.1 | python3 python3-gast | 16 |
| [`python3-bitstruct`](ports/python3-bitstruct/) | 8.24.0 | python3 | 15 |
| [`python3-boto3`](ports/python3-boto3/) | 1.44.9 | python3 python3-botocore python3-s3transfer python3-jmespath | 21 |
| [`python3-botocore`](ports/python3-botocore/) | 1.44.9 | python3 python3-jmespath python3-dateutil python3-urllib3 | 19 |
| [`python3-bracex`](ports/python3-bracex/) | 3.2.4 | python3 | 15 |
| [`python3-build`](ports/python3-build/) | 1.6.1 | python3 python3-packaging python3-pyproject-hooks | 17 |
| [`python3-calver`](ports/python3-calver/) | 2025.10.20 | python3 | 15 |
| [`python3-can`](ports/python3-can/) | 4.6.1 | python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan | 19 |
| [`python3-can-isotp`](ports/python3-can-isotp/) | 2.0.7 | python3 python3-can | 20 |
| [`python3-can-j1939`](ports/python3-can-j1939/) | 2.0.12 | python3 python3-can numpy | 22 |
| [`python3-canmatrix`](ports/python3-canmatrix/) | 1.3.3 | python3 python3-attrs python3-click | 17 |
| [`python3-cantools`](ports/python3-cantools/) | 45.2.0 | python3 python3-bitstruct python3-can python3-textparser python3-argparse-addons python3-crccheck | 24 |
| [`python3-catkin-pkg`](ports/python3-catkin-pkg/) | 1.1.1 | python3 python3-dateutil python3-docutils python3-packaging python3-pyparsing | 20 |
| [`python3-cattrs`](ports/python3-cattrs/) | 26.2.1 | python3 python3-attrs python3-typing-extensions | 17 |
| [`python3-click`](ports/python3-click/) | 8.5.1 | python3 | 15 |
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
| [`python3-coverage`](ports/python3-coverage/) | 7.17.4 | python3 | 15 |
| [`python3-cparser`](ports/python3-cparser/) | 3.12 | python3 | 15 |
| [`python3-cppy`](ports/python3-cppy/) | 1.3.1 | python3 python3-setuptools | 16 |
| [`python3-crccheck`](ports/python3-crccheck/) | 1.3.1 | python3 | 15 |
| [`python3-curio`](ports/python3-curio/) | 1.6_git20251221 | python3 | 15 |
| [`python3-cycler`](ports/python3-cycler/) | 0.12.1 | python3 | 15 |
| [`python3-dateutil`](ports/python3-dateutil/) | 2.9.0.post0 | python3 python3-six | 16 |
| [`python3-distlib`](ports/python3-distlib/) | 0.4.4 | python3 | 15 |
| [`python3-distro`](ports/python3-distro/) | 1.9.0 | python3 | 15 |
| [`python3-docutils`](ports/python3-docutils/) | 0.23 | python3 | 15 |
| [`python3-editables`](ports/python3-editables/) | 0.6 | python3 | 15 |
| [`python3-empy`](ports/python3-empy/) | 4.2.1 | python3 | 15 |
| [`python3-execnet`](ports/python3-execnet/) | 2.1.2 | python3 | 15 |
| [`python3-filelock`](ports/python3-filelock/) | 4.2.12 | python3 | 15 |
| [`python3-flake8`](ports/python3-flake8/) | 7.5.0 | python3 python3-mccabe python3-pycodestyle python3-pyflakes | 18 |
| [`python3-flake8-blind-except`](ports/python3-flake8-blind-except/) | 0.2.1 | python3 python3-flake8 | 19 |
| [`python3-flake8-builtins`](ports/python3-flake8-builtins/) | 3.2.0 | python3 python3-flake8 | 19 |
| [`python3-flake8-class-newline`](ports/python3-flake8-class-newline/) | 1.6.0 | python3 python3-flake8 | 19 |
| [`python3-flake8-comprehensions`](ports/python3-flake8-comprehensions/) | 3.17.0 | python3 python3-flake8 | 19 |
| [`python3-flake8-deprecated`](ports/python3-flake8-deprecated/) | 2.4.0 | python3 python3-flake8 | 19 |
| [`python3-flake8-docstrings`](ports/python3-flake8-docstrings/) | 1.7.0 | python3 python3-flake8 python3-pydocstyle | 21 |
| [`python3-flake8-import-order`](ports/python3-flake8-import-order/) | 0.19.2 | python3 python3-pycodestyle python3-setuptools python3-flake8 | 20 |
| [`python3-flake8-quotes`](ports/python3-flake8-quotes/) | 3.4.0 | python3 python3-flake8 python3-setuptools | 20 |
| [`python3-flaky`](ports/python3-flaky/) | 3.8.1 | python3 python3-pytest | 20 |
| [`python3-flexcache`](ports/python3-flexcache/) | 0.3 | python3 python3-typing-extensions | 16 |
| [`python3-flexparser`](ports/python3-flexparser/) | 0.4 | python3 python3-typing-extensions | 16 |
| [`python3-flit-core`](ports/python3-flit-core/) | 4.2.0 | python3 | 15 |
| [`python3-fonttools`](ports/python3-fonttools/) | 4.68.0 | python3 | 15 |
| [`python3-fs`](ports/python3-fs/) | 2.4.16 | python3 python3-appdirs python3-setuptools python3-six | 18 |
| [`python3-gast`](ports/python3-gast/) | 0.7.0 | python3 | 15 |
| [`python3-gpep517`](ports/python3-gpep517/) | 23 | python3 python3-installer | 16 |
| [`python3-hatch-fancy-pypi-readme`](ports/python3-hatch-fancy-pypi-readme/) | 25.1.0 | python3 python3-hatchling | 20 |
| [`python3-hatch-vcs`](ports/python3-hatch-vcs/) | 0.5.0 | python3 python3-hatchling python3-setuptools_scm | 22 |
| [`python3-hatchling`](ports/python3-hatchling/) | 1.33.0 | python3 python3-packaging python3-pathspec python3-pluggy python3-trove-classifiers | 19 |
| [`python3-idna`](ports/python3-idna/) | 3.20 | python3 | 15 |
| [`python3-iniconfig`](ports/python3-iniconfig/) | 2.3.1 | python3 | 15 |
| [`python3-installer`](ports/python3-installer/) | 1.1.0 | python3 | 15 |
| [`python3-jinja2`](ports/python3-jinja2/) | 3.1.6 | python3 python3-markupsafe | 16 |
| [`python3-jmespath`](ports/python3-jmespath/) | 1.1.0 | python3 | 15 |
| [`python3-kiwisolver`](ports/python3-kiwisolver/) | 1.5.1 | python3 | 15 |
| [`python3-lark`](ports/python3-lark/) | 1.3.1 | python3 | 15 |
| [`python3-markupsafe`](ports/python3-markupsafe/) | 3.0.4 | python3 | 15 |
| [`python3-mccabe`](ports/python3-mccabe/) | 0.7.0 | python3 | 15 |
| [`python3-meson-python`](ports/python3-meson-python/) | 0.22.1 | python3 meson python3-packaging python3-pyproject-metadata | 18 |
| [`python3-ml-dtypes`](ports/python3-ml-dtypes/) | 0.7.0 | numpy | 17 |
| [`python3-msgpack`](ports/python3-msgpack/) | 1.2.4 | python3 | 15 |
| [`python3-mypy-extensions`](ports/python3-mypy-extensions/) | 1.2.0 | python3 | 15 |
| [`python3-netifaces`](ports/python3-netifaces/) | 0.11.0 | python3 | 15 |
| [`python3-obd`](ports/python3-obd/) | 0.7.3 | python3 python3-pyserial python3-pint | 21 |
| [`python3-opt-einsum`](ports/python3-opt-einsum/) | 3.4.0 | python3 | 15 |
| [`python3-packaging`](ports/python3-packaging/) | 25.0 | python3 | 15 |
| [`python3-pathspec`](ports/python3-pathspec/) | 1.1.1 | python3 | 15 |
| [`python3-pexpect`](ports/python3-pexpect/) | 4.9.0 | python3 python3-ptyprocess | 16 |
| [`python3-pint`](ports/python3-pint/) | 0.26.2 | python3 python3-flexcache python3-flexparser python3-platformdirs python3-typing-extensions | 19 |
| [`python3-pip`](ports/python3-pip/) | 25.1.1 | python3 | 15 |
| [`python3-platformdirs`](ports/python3-platformdirs/) | 4.14.8 | python3 | 15 |
| [`python3-pluggy`](ports/python3-pluggy/) | 1.7.0 | python3 | 15 |
| [`python3-ply`](ports/python3-ply/) | 3.11 | python3 | 15 |
| [`python3-pretend`](ports/python3-pretend/) | 1.0.9 | python3 | 15 |
| [`python3-protobuf`](ports/python3-protobuf/) | 5.29.3 | python3 | 15 |
| [`python3-ptyprocess`](ports/python3-ptyprocess/) | 0.7.0 | python3 | 15 |
| [`python3-py`](ports/python3-py/) | 1.11.0 | python3 | 15 |
| [`python3-pycodestyle`](ports/python3-pycodestyle/) | 2.15.0 | python3 | 15 |
| [`python3-pydbus`](ports/python3-pydbus/) | 0.6.0 | python3 python3-gobject3 | 37 |
| [`python3-pydocstyle`](ports/python3-pydocstyle/) | 6.3.0 | python3 python3-snowballstemmer | 16 |
| [`python3-pyflakes`](ports/python3-pyflakes/) | 4.1.0 | python3 | 15 |
| [`python3-pygments`](ports/python3-pygments/) | 2.21.0 | python3 | 15 |
| [`python3-pyparsing`](ports/python3-pyparsing/) | 3.3.3 | python3 | 15 |
| [`python3-pyproject-hooks`](ports/python3-pyproject-hooks/) | 1.3.3 | python3 | 15 |
| [`python3-pyproject-metadata`](ports/python3-pyproject-metadata/) | 0.12.1 | python3 python3-packaging | 16 |
| [`python3-pyserial`](ports/python3-pyserial/) | 3.5 | python3 | 15 |
| [`python3-pytest`](ports/python3-pytest/) | 9.2.0 | python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments | 19 |
| [`python3-pytest-cov`](ports/python3-pytest-cov/) | 7.1.0 | python3 python3-pytest python3-coverage python3-pluggy | 21 |
| [`python3-pytest-mock`](ports/python3-pytest-mock/) | 3.17.0 | python3 python3-pytest | 20 |
| [`python3-pytest-repeat`](ports/python3-pytest-repeat/) | 0.9.4 | python3 python3-pytest | 20 |
| [`python3-pytest-rerunfailures`](ports/python3-pytest-rerunfailures/) | 16.7 | python3 python3-packaging python3-pytest | 20 |
| [`python3-pytest-tornasync`](ports/python3-pytest-tornasync/) | 0.6.0.post2 | python3 python3-pytest python3-tornado | 21 |
| [`python3-pytest-xdist`](ports/python3-pytest-xdist/) | 3.8.0 | python3 python3-execnet python3-pytest | 21 |
| [`python3-pytokens`](ports/python3-pytokens/) | 0.4.1 | python3 | 15 |
| [`python3-pytz`](ports/python3-pytz/) | 2026.6 | python3 | 15 |
| [`python3-pyyaml`](ports/python3-pyyaml/) | 6.0.4 | python3 | 15 |
| [`python3-rosdistro`](ports/python3-rosdistro/) | 1.1.0 | python3 python3-packaging python3-pyyaml python3-setuptools python3-catkin-pkg python3-rospkg | 25 |
| [`python3-rospkg`](ports/python3-rospkg/) | 1.6.3 | python3 python3-catkin-pkg python3-pyyaml python3-distro | 23 |
| [`python3-s3transfer`](ports/python3-s3transfer/) | 0.19.3 | python3 python3-botocore | 20 |
| [`python3-scapy`](ports/python3-scapy/) | 2.8.0 | python3 | 15 |
| [`python3-scikit-build-core`](ports/python3-scikit-build-core/) | 1.1.2 | python3 python3-packaging python3-pathspec | 17 |
| [`python3-setuptools`](ports/python3-setuptools/) | 80.8.0 | python3 | 15 |
| [`python3-setuptools_scm`](ports/python3-setuptools_scm/) | 9.2.2 | python3 python3-packaging python3-setuptools | 17 |
| [`python3-six`](ports/python3-six/) | 1.17.0 | python3 | 15 |
| [`python3-sniffio`](ports/python3-sniffio/) | 1.3.1 | python3 | 15 |
| [`python3-snowballstemmer`](ports/python3-snowballstemmer/) | 3.1.1 | python3 | 15 |
| [`python3-sortedcontainers`](ports/python3-sortedcontainers/) | 2.4.0 | python3 | 15 |
| [`python3-sounddevice`](ports/python3-sounddevice/) | 0.5.6 | python3 portaudio python3-cffi | 18 |
| [`python3-textparser`](ports/python3-textparser/) | 0.26.2 | python3 | 15 |
| [`python3-time-machine`](ports/python3-time-machine/) | 3.6.0 | python3 | 15 |
| [`python3-tomli`](ports/python3-tomli/) | 2.5.0 | python3 | 15 |
| [`python3-tornado`](ports/python3-tornado/) | 6.6.1 | python3 | 15 |
| [`python3-traitlets`](ports/python3-traitlets/) | 5.16.1 | python3 | 15 |
| [`python3-trove-classifiers`](ports/python3-trove-classifiers/) | 2026.9.21.13 | python3 | 15 |
| [`python3-typeguard`](ports/python3-typeguard/) | 4.6.1 | python3 python3-typing-extensions | 16 |
| [`python3-typing-extensions`](ports/python3-typing-extensions/) | 4.16.0 | python3 | 15 |
| [`python3-tzdata`](ports/python3-tzdata/) | 2026.6 | python3 | 15 |
| [`python3-udsoncan`](ports/python3-udsoncan/) | 1.26.1 | python3 python3-can-isotp | 21 |
| [`python3-urllib3`](ports/python3-urllib3/) | 2.8.0 | python3 | 15 |
| [`python3-versioneer`](ports/python3-versioneer/) | 0.29 | python3 | 15 |
| [`python3-virtualenv`](ports/python3-virtualenv/) | 20.39.1 | python3 python3-distlib python3-filelock python3-platformdirs | 18 |
| [`python3-wheel`](ports/python3-wheel/) | 0.48.0 | python3 python3-packaging | 16 |
| [`python3-wrapt`](ports/python3-wrapt/) | 2.5.2 | python3 | 15 |
| [`pythran`](ports/pythran/) | 0.19.0 | python3 python3-ply python3-setuptools python3-gast python3-beniget numpy | 21 |
| [`qt6-translations`](ports/qt6-translations/) | 6.8.1 | qt6 | 69 |
| [`readline`](ports/readline/) | 8.3 | ncurses | 1 |
| [`retropie`](ports/retropie/) | 0.3 | retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08 | 41 |
| [`ruy`](ports/ruy/) | 0-git-20240321 | cpuinfo | 1 |
| [`scons`](ports/scons/) | 4.11.1 | python3 | 15 |
| [`sdl2-compat`](ports/sdl2-compat/) | 2.32.74 | sdl3 | 34 |
| [`sophus`](ports/sophus/) | 1.24.6 | eigen | 1 |
| [`spirv-tools`](ports/spirv-tools/) | 1.4.321.0 | spirv-headers | 1 |
| [`sqlite_orm`](ports/sqlite_orm/) | 1.9.1 | sqlite | 2 |
| [`vcstool`](ports/vcstool/) | 0.3.0 | python3 python3-pyyaml python3-setuptools | 17 |
| [`wayland-protocols`](ports/wayland-protocols/) | 1.50 | wayland | 17 |
| [`xtensor`](ports/xtensor/) | 0.27.2 | xtl xsimd | 2 |
| [`xz`](ports/xz/) | 5.8.4 | gettext | 4 |
| [`yt-dlp`](ports/yt-dlp/) | 2026.10.26 | python3 | 15 |
| [`zenoh-c`](ports/zenoh-c/) | 1.10.2 | rust | 1 |
| [`zenoh-cpp`](ports/zenoh-cpp/) | 1.10.2 | zenoh-c zenoh-pico | 3 |
| [`zenoh-router`](ports/zenoh-router/) | 1.10.2 | rust | 1 |

## Dependent

378 components. Both the build and the target need other components. *Runtime* is the set of components whose shared libraries these binaries load -- the packages `oss-get` installs with it.

| Component | Version | Needs to build | Needs at run time | Run-time closure |
|---|---|---|---|---|
| [`abuild`](ports/abuild/) | 3.17.0 | openssl zlib apk-tools | openssl zlib | 2 |
| [`apk-tools`](ports/apk-tools/) | 3.0.8 | openssl zlib zstd | openssl zlib zstd | 3 |
| [`appstream`](ports/appstream/) | 1.2.3 | glib curl libxml2 libfyaml libxmlb zstd wayland | curl gettext glib libfyaml libxml2 libxmlb zstd | 19 |
| [`apr-util`](ports/apr-util/) | 1.6.6 | apr expat openssl sqlite libiconv libxcrypt | apr expat libiconv libxcrypt openssl sqlite | 7 |
| [`armadillo`](ports/armadillo/) | 15.6.2 | openblas | openblas | 1 |
| [`aspell`](ports/aspell/) | 0.60.8.1 | ncurses | ncurses | 1 |
| [`assimp`](ports/assimp/) | 6.0.6 | zlib | zlib | 1 |
| [`at-spi2-core`](ports/at-spi2-core/) | 2.54.2 | glib dbus libxml2 libx11 libxtst | dbus gettext glib libx11 libxi libxtst | 17 |
| [`avahi`](ports/avahi/) | 0.9-rc3 | dbus expat libdaemon gobject-introspection | dbus expat libdaemon | 3 |
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
| [`azure-iot-sdk-c`](ports/azure-iot-sdk-c/) | LTS_08_2024 | openssl curl | curl openssl | 12 |
| [`bash`](ports/bash/) | 5.3 | ncurses readline libiconv | libiconv ncurses readline | 3 |
| [`bc`](ports/bc/) | 1.08.2 | readline | ncurses readline | 2 |
| [`binutils`](ports/binutils/) | 2.47 | zlib zstd msgpack-c | flex msgpack-c zlib zstd | 4 |
| [`boost`](ports/boost/) | 1.86.0 | zlib bzip2 xz | bzip2 libiconv xz zlib | 4 |
| [`c-blosc`](ports/c-blosc/) | 1.21.7 | lz4 zlib zstd | lz4 zlib zstd | 3 |
| [`cairo`](ports/cairo/) | 1.18.6 | pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo | fontconfig freetype glib libpng libx11 libxext libxrender lzo pixman zlib | 22 |
| [`can-utils`](ports/can-utils/) | 2025.01 | qnx-socketcan | qnx-socketcan | 1 |
| [`cannelloni`](ports/cannelloni/) | 2.1.2 | qnx-socketcan | qnx-socketcan | 1 |
| [`canopennode`](ports/canopennode/) | 4.0 | qnx-socketcan | qnx-socketcan | 1 |
| [`capicxx-someip-runtime`](ports/capicxx-someip-runtime/) | 3.2.4 | capicxx-core-runtime vsomeip | capicxx-core-runtime vsomeip | 7 |
| [`capnproto`](ports/capnproto/) | 1.5.0 | zlib openssl | openssl zlib | 2 |
| [`cartographer`](ports/cartographer/) | 2.0.0 | abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo | abseil boost cairo ceres-solver gflags glog lua5.3 protobuf | 33 |
| [`ccache`](ports/ccache/) | 4.14.3 | zstd xxhash | xxhash zstd | 2 |
| [`ceres-solver`](ports/ceres-solver/) | 2.2.0 | eigen glog gflags | gflags glog metis | 4 |
| [`civetweb`](ports/civetweb/) | 1.16 | openssl | openssl | 1 |
| [`clinfo`](ports/clinfo/) | 3.1.26.09.26 | opencl-headers opencl-icd-loader | opencl-icd-loader | 1 |
| [`cmake`](ports/cmake/) | 4.5.1 | openssl zlib curl libuv | curl libiconv libuv openssl zlib | 13 |
| [`cups`](ports/cups/) | 2.4.20 | openssl zlib avahi libiconv libxcrypt | avahi libiconv libxcrypt openssl zlib | 8 |
| [`cups-filters`](ports/cups-filters/) | 1.28.17 | cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib libexif | avahi cups fontconfig freetype gettext glib lcms2 libexif libiconv libjpeg-turbo libpng libtiff libxcrypt openssl poppler qpdf zlib | 43 |
| [`curl`](ports/curl/) | 8.23.0 | openssl zlib nghttp2 c-ares brotli zstd | brotli c-ares libidn2 nghttp2 openssl zlib zstd | 11 |
| [`dbus`](ports/dbus/) | 1.16.4 | expat | expat | 1 |
| [`dbus-glib`](ports/dbus-glib/) | 0.116 | dbus glib expat | dbus expat gettext glib | 10 |
| [`desktop-file-utils`](ports/desktop-file-utils/) | 0.29 | glib | gettext glib | 8 |
| [`dlt-daemon`](ports/dlt-daemon/) | 3.0.0 | zlib libjson | libjson | 1 |
| [`doxygen`](ports/doxygen/) | 1.18.0 | libiconv | libiconv | 1 |
| [`dump1090`](ports/dump1090/) | 0-git-20240121 | rtl-sdr | rtl-sdr | 2 |
| [`e2fsprogs`](ports/e2fsprogs/) | 1.47.4 | libuuid libarchive file | libuuid | 1 |
| [`elfutils`](ports/elfutils/) | 0.193 | zlib | zlib | 1 |
| [`elvis`](ports/elvis/) | 2.2.1-git-20240226 | ncurses | ncurses | 1 |
| [`emacs`](ports/emacs/) | 31.1 | ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter | dbus gmp gnutls lcms2 libxml2 ncurses sqlite tree-sitter zlib | 21 |
| [`emulationstation`](ports/emulationstation/) | 2.11-git-20241212 | sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc | curl freeimage freetype pugixml sdl2 vlc | 22 |
| [`enca`](ports/enca/) | 1.22 | – | libiconv | 1 |
| [`enchant`](ports/enchant/) | 2.8.21 | glib aspell | aspell gettext glib | 9 |
| [`epiphany`](ports/epiphany/) | 43.1 | webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle | at-spi2-core cairo gcr gdk-pixbuf gettext glib gmp gtk3 json-glib libarchive libdazzle libhandy libportal libsecret libsoup libxml2 nettle pango sqlite webkitgtk | 85 |
| [`exiv2`](ports/exiv2/) | 0.28.9 | zlib expat brotli libinih | brotli expat libiconv libinih zlib | 5 |
| [`exo`](ports/exo/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | 50 |
| [`fastdds`](ports/fastdds/) | 2.14.4 | fastcdr foonathan_memory tinyxml2 openssl | fastcdr foonathan_memory openssl tinyxml2 | 4 |
| [`fcft`](ports/fcft/) | 3.3.3 | tllist fontconfig freetype harfbuzz utf8proc pixman | fontconfig freetype harfbuzz pixman utf8proc | 19 |
| [`fcl`](ports/fcl/) | 0.7.0 | eigen libccd octomap | libccd octomap | 2 |
| [`ffmpeg`](ports/ffmpeg/) | 7.1.1 | zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame | bzip2 dav1d lame libiconv libtheora libvorbis libvpx openh264 opus x264 x265 xz zlib | 15 |
| [`ffmpegthumbnailer`](ports/ffmpegthumbnailer/) | 2.4.2 | ffmpeg libpng libjpeg-turbo | ffmpeg libjpeg-turbo libpng zlib | 18 |
| [`file`](ports/file/) | 5.48 | zlib bzip2 xz zstd | bzip2 xz zlib zstd | 4 |
| [`fio`](ports/fio/) | 3.43 | zlib | zlib | 1 |
| [`flac`](ports/flac/) | 1.5.0 | libogg | libogg | 1 |
| [`flann`](ports/flann/) | 1.9.2 | lz4 | lz4 | 1 |
| [`folly`](ports/folly/) | 2026.10.26.00 | boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium | boost bzip2 fmt gflags glog libevent libsodium lz4 openssl xz zlib zstd | 13 |
| [`fontconfig`](ports/fontconfig/) | 2.18.4 | freetype expat | expat freetype gettext | 10 |
| [`fontenc`](ports/fontenc/) | 1.1.8 | xorgproto zlib | zlib | 1 |
| [`foot`](ports/foot/) | 1.28.0 | pixman wayland wayland-protocols libxkbcommon fontconfig utf8proc tllist fcft ncurses linux-headers-input-codes | fcft fontconfig libxkbcommon pixman utf8proc wayland | 24 |
| [`freetype`](ports/freetype/) | 2-15-0 | zlib bzip2 libpng brotli | brotli bzip2 libpng zlib | 4 |
| [`frei0r-plugins`](ports/frei0r-plugins/) | 3.7.1 | cairo opencv freetype | cairo opencv | 24 |
| [`garcon`](ports/garcon/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | 50 |
| [`gawk`](ports/gawk/) | 5.4.2 | readline ncurses | ncurses readline | 2 |
| [`gcr`](ports/gcr/) | 3.41.2 | glib libgcrypt p11-kit libsecret gtk3 | cairo gdk-pixbuf gettext glib gtk3 libgcrypt p11-kit pango | 50 |
| [`gd`](ports/gd/) | 2.3.3 | zlib libpng libjpeg-turbo libtiff libwebp freetype fontconfig libavif libheif | fontconfig freetype libavif libheif libjpeg-turbo libpng libtiff libwebp zlib | 27 |
| [`gdb`](ports/gdb/) | 14.2 | gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd | expat gmp libiconv mpfr ncurses readline xxhash xz zlib zstd | 10 |
| [`gdk-pixbuf`](ports/gdk-pixbuf/) | 2.44.8 | glib libpng libjpeg-turbo | gettext glib libjpeg-turbo libpng | 10 |
| [`geany`](ports/geany/) | 2.1 | glib gtk3 | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango zlib | 47 |
| [`geany-plugins`](ports/geany-plugins/) | 2.1 | geany | at-spi2-core cairo gdk-pixbuf geany gettext glib gtk3 harfbuzz pango zlib | 48 |
| [`gettext`](ports/gettext/) | 0.22.5 | ncurses libiconv libunistring | libiconv libunistring ncurses | 3 |
| [`git`](ports/git/) | 2.56.1 | zlib openssl curl expat pcre2 perl perl-error libiconv | curl expat libiconv openssl pcre2 zlib | 14 |
| [`glib`](ports/glib/) | 2.90.1 | libffi pcre2 gettext zlib | gettext libffi libiconv pcre2 zlib | 7 |
| [`glib-networking`](ports/glib-networking/) | 2.90.1 | glib gnutls libproxy | gettext glib gnutls libproxy | 22 |
| [`glog`](ports/glog/) | 0.7.1 | gflags | gflags | 1 |
| [`glslang`](ports/glslang/) | 1.4.321.0 | spirv-tools spirv-headers | spirv-tools | 1 |
| [`gnome-keyring`](ports/gnome-keyring/) | 51.1 | glib gcr libgcrypt libgpg-error p11-kit | gcr gettext glib libgcrypt | 51 |
| [`gnupg`](ports/gnupg/) | 2.5.24 | libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv | bzip2 gnutls libassuan libgcrypt libgpg-error libiconv libksba ncurses npth readline sqlite zlib | 23 |
| [`gnutls`](ports/gnutls/) | 3.8.14 | nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd | brotli gmp libidn2 libtasn1 libunistring nettle unbound zlib zstd | 14 |
| [`gobject-introspection`](ports/gobject-introspection/) | 1.86.0 | glib libffi python3 | glib libffi | 8 |
| [`godot`](ports/godot/) | 4.4.1 | freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon | brotli bzip2 freetype gettext glib graphite2 harfbuzz libiconv libogg libpng libtheora libvorbis libwebp libxkbcommon pcre2 zlib zstd | 23 |
| [`gpgme`](ports/gpgme/) | 2.3.1 | libgpg-error libassuan | libassuan libgpg-error | 6 |
| [`graphene`](ports/graphene/) | 1.10.8 | glib | glib | 8 |
| [`graphicsmagick`](ports/graphicsmagick/) | 1.3.46 | libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2 libzip | bzip2 freetype lcms2 libheif libjpeg-turbo libjxl libpng libtiff libwebp libxml2 libzip xz zlib zstd | 26 |
| [`graphviz`](ports/graphviz/) | 16.1.2 | gd cairo pango expat zlib libtool freetype fontconfig libwebp poppler glib libx11 libxrender | cairo expat gd gettext glib harfbuzz libtool libwebp libx11 libxrender pango poppler zlib | 49 |
| [`grep`](ports/grep/) | 3.12 | pcre2 libiconv | pcre2 | 1 |
| [`groff`](ports/groff/) | 1.25.0 | libiconv | libiconv | 1 |
| [`grpc`](ports/grpc/) | 1.85.4 | zlib openssl c-ares abseil protobuf re2 | abseil c-ares openssl protobuf re2 zlib | 6 |
| [`gst-libav`](ports/gst-libav/) | 1.24.10 | gst-plugins-base ffmpeg | ffmpeg glib gst-plugins-base gstreamer | 24 |
| [`gst-plugins-bad`](ports/gst-plugins-bad/) | 1.24.10 | gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo wayland wayland-protocols libdrm-headers | aom cairo fdk-aac glib gst-plugins-base gstreamer libwebp openh264 pango svt-av1 wayland x265 | 42 |
| [`gst-plugins-base`](ports/gst-plugins-base/) | 1.24.10 | gstreamer orc libogg libvorbis opus libtheora zlib | gettext glib gstreamer libogg libtheora libvorbis opus zlib | 13 |
| [`gst-plugins-good`](ports/gst-plugins-good/) | 1.24.10 | gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2 gtk3 | bzip2 cairo flac gdk-pixbuf gettext glib gst-plugins-base gstreamer gtk3 lame libjpeg-turbo libpng libsoup libvpx zlib | 63 |
| [`gst-plugins-ugly`](ports/gst-plugins-ugly/) | 1.24.10 | gst-plugins-base x264 | gettext glib gst-plugins-base gstreamer x264 | 15 |
| [`gstreamer`](ports/gstreamer/) | 1.24.10 | glib libxml2 | gettext glib | 8 |
| [`gthumb`](ports/gthumb/) | 3.12.12 | glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib | cairo exiv2 gdk-pixbuf gettext glib gtk3 lcms2 libjpeg-turbo libpng libtiff libwebp pango zlib | 54 |
| [`gtk-layer-shell`](ports/gtk-layer-shell/) | 0.10.1 | glib gtk3 wayland wayland-protocols | glib gtk3 wayland | 47 |
| [`gtk-vnc`](ports/gtk-vnc/) | 1.6.0 | glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11 | cairo gdk-pixbuf gettext glib gmp gnutls gtk3 libx11 zlib | 55 |
| [`gtk3`](ports/gtk3/) | 3.24.43 | glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama | at-spi2-core cairo fontconfig fribidi gdk-pixbuf gettext glib harfbuzz libepoxy libx11 libxcomposite libxcursor libxdamage libxext libxfixes libxi libxinerama libxkbcommon libxrandr pango wayland | 46 |
| [`gtk4`](ports/gtk4/) | 4.16.13 | glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff | cairo fontconfig fribidi gdk-pixbuf gettext glib graphene harfbuzz libepoxy libjpeg-turbo libpng libtiff libxkbcommon pango wayland | 41 |
| [`gtsam`](ports/gtsam/) | 4.3.0 | boost eigen | boost suitesparse | 7 |
| [`harfbuzz`](ports/harfbuzz/) | 10.1.0 | freetype glib icu graphite2 | freetype glib graphite2 icu | 14 |
| [`hdf5`](ports/hdf5/) | 1.14.6 | zlib libaec | libaec zlib | 2 |
| [`hiredis`](ports/hiredis/) | 1.5.1 | openssl | openssl | 1 |
| [`htop`](ports/htop/) | 3.5.3 | ncurses | ncurses | 1 |
| [`hunspell`](ports/hunspell/) | 1.7.6 | libiconv | libiconv | 1 |
| [`inetutils`](ports/inetutils/) | 2.8 | ncurses readline libxcrypt | libxcrypt ncurses readline | 3 |
| [`iperf3`](ports/iperf3/) | 3.22.1 | openssl | openssl | 1 |
| [`ippusbxd`](ports/ippusbxd/) | 1.34 | libusb avahi libxml2 cups | avahi cups libusb libxml2 | 12 |
| [`javascriptcore`](ports/javascriptcore/) | 2.54.3 | icu | icu | 1 |
| [`jq`](ports/jq/) | 1.8.2 | oniguruma | oniguruma | 1 |
| [`json-glib`](ports/json-glib/) | 1.10.8 | glib | gettext glib | 8 |
| [`labwc`](ports/labwc/) | 0.20.3 | wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng | cairo glib libpng libsfdo libxkbcommon libxml2 pango pixman wayland wlroots | 36 |
| [`lame`](ports/lame/) | 4.0 | – | mpg123 | 1 |
| [`lanelet2`](ports/lanelet2/) | 1.2.3 | ros2 boost eigen geographiclib pugixml | boost geographiclib pugixml | 7 |
| [`lcm`](ports/lcm/) | 1.5.3 | glib | glib | 8 |
| [`less`](ports/less/) | 668 | ncurses | ncurses | 1 |
| [`leveldb`](ports/leveldb/) | 1.23 | snappy | snappy | 1 |
| [`libadwaita`](ports/libadwaita/) | 1.6.10 | glib gtk4 fribidi appstream | appstream fribidi gettext glib graphene gtk4 pango | 50 |
| [`libarchive`](ports/libarchive/) | 3.8.11 | zlib bzip2 xz zstd openssl libxml2 expat lz4 | bzip2 expat libb2 lz4 openssl xz zlib zstd | 8 |
| [`libass`](ports/libass/) | 0.17.5 | freetype fribidi harfbuzz fontconfig | fontconfig freetype fribidi harfbuzz libiconv libunibreak | 19 |
| [`libassuan`](ports/libassuan/) | 3.0.2 | libgpg-error | libgpg-error | 5 |
| [`libavif`](ports/libavif/) | 1.4.2 | aom dav1d libyuv | aom dav1d | 2 |
| [`libcaca`](ports/libcaca/) | 0.99.beta20 | ncurses zlib | ncurses zlib | 2 |
| [`libcamera`](ports/libcamera/) | 0.5.1+rpt20250722 | libyaml | libyaml | 1 |
| [`libcoap`](ports/libcoap/) | 4.3.5 | openssl | openssl | 1 |
| [`libdatrie`](ports/libdatrie/) | 0.2.14 | – | libiconv | 1 |
| [`libdazzle`](ports/libdazzle/) | 3.44.0 | glib gtk3 | cairo gdk-pixbuf gettext glib gtk3 pango | 47 |
| [`libdecor`](ports/libdecor/) | 0.2.5 | wayland wayland-protocols cairo pango dbus gtk3 linux-headers-input-codes | cairo dbus glib gtk3 pango wayland | 47 |
| [`libedit`](ports/libedit/) | 20240808-3.1 | ncurses | ncurses | 1 |
| [`libevent`](ports/libevent/) | 2.1.12 | openssl | openssl | 1 |
| [`libgcrypt`](ports/libgcrypt/) | 1.12.4 | libgpg-error | libgpg-error | 5 |
| [`libgit2`](ports/libgit2/) | 1.9.7 | openssl zlib pcre2 | openssl pcre2 zlib | 3 |
| [`libgme`](ports/libgme/) | 0.6.5 | zlib | zlib | 1 |
| [`libgpg-error`](ports/libgpg-error/) | 1.61 | gettext | gettext | 4 |
| [`libgsf`](ports/libgsf/) | 1.14.60 | glib libxml2 zlib bzip2 | bzip2 gettext glib libxml2 zlib | 11 |
| [`libhandy`](ports/libhandy/) | 1.8.3 | glib gtk3 | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gtk3 pango | 47 |
| [`libheif`](ports/libheif/) | 1.23.7 | libde265 x265 aom dav1d openh264 libjpeg-turbo openjpeg openjph libwebp libpng libtiff zlib | aom dav1d libde265 libjpeg-turbo libpng libtiff libwebp openh264 openjpeg openjph x264 x265 zlib | 16 |
| [`libidn2`](ports/libidn2/) | 2.3.8 | libunistring gettext | gettext libiconv libunistring | 4 |
| [`libjxl`](ports/libjxl/) | 0.13.0 | brotli highway lcms2 libpng giflib libjpeg-turbo | brotli highway lcms2 | 3 |
| [`libksba`](ports/libksba/) | 1.9.1 | libgpg-error | libgpg-error | 5 |
| [`libnotify`](ports/libnotify/) | 0.8.9 | glib gdk-pixbuf | gdk-pixbuf glib | 11 |
| [`libpanel`](ports/libpanel/) | 1.10.4 | glib gtk4 libadwaita | cairo gettext glib graphene gtk4 libadwaita | 51 |
| [`libpng`](ports/libpng/) | 1.6.59 | zlib | zlib | 1 |
| [`libportal`](ports/libportal/) | 0.11.0 | glib gtk3 | glib gtk3 | 47 |
| [`libproxy`](ports/libproxy/) | 0.5.12 | glib curl | curl glib | 15 |
| [`libpsl`](ports/libpsl/) | 0.23.3 | libidn2 libunistring libiconv | libiconv libidn2 libunistring | 5 |
| [`libraqm`](ports/libraqm/) | 0.11.0 | freetype harfbuzz fribidi | freetype fribidi harfbuzz | 16 |
| [`libsecret`](ports/libsecret/) | 0.21.8.2 | glib libgcrypt | glib libgcrypt | 10 |
| [`libsm`](ports/libsm/) | 1.2.6 | libice xorgproto xtrans | libice | 1 |
| [`libsndfile`](ports/libsndfile/) | 1.2.2 | libogg libvorbis flac opus | flac libogg libvorbis opus | 4 |
| [`libsoup`](ports/libsoup/) | 3.8.1 | glib libxml2 sqlite nghttp2 libpsl brotli zstd | brotli gettext glib libpsl nghttp2 sqlite zlib zstd | 14 |
| [`libssh`](ports/libssh/) | 0.11.5 | openssl zlib | openssl zlib | 2 |
| [`libssh2`](ports/libssh2/) | 1.11.1 | openssl zlib | openssl zlib | 2 |
| [`libtermkey`](ports/libtermkey/) | 0.22 | unibilium | unibilium | 1 |
| [`libthai`](ports/libthai/) | 0.1.30 | libdatrie | libdatrie | 2 |
| [`libtheora`](ports/libtheora/) | 1.2.0 | libogg libvorbis | libogg | 1 |
| [`libtiff`](ports/libtiff/) | 4.7.2 | zlib xz zstd libjpeg-turbo libwebp | libdeflate libjpeg-turbo libwebp xz zlib zstd | 6 |
| [`libtomcrypt`](ports/libtomcrypt/) | 1.18.2 | libtommath | libtommath | 1 |
| [`libunistring`](ports/libunistring/) | 1.5 | – | libiconv | 1 |
| [`libvalkey`](ports/libvalkey/) | 0.5.0 | openssl | openssl | 1 |
| [`libvncserver`](ports/libvncserver/) | 0.9.15 | zlib lzo libjpeg-turbo libpng gnutls openssl sdl2 | gnutls libjpeg-turbo libpng lzo openssl sdl2 zlib | 19 |
| [`libvorbis`](ports/libvorbis/) | 1.3.7 | libogg | libogg | 1 |
| [`libwebsockets`](ports/libwebsockets/) | 4.3.5 | openssl zlib | openssl | 1 |
| [`libx11`](ports/libx11/) | 1.8.14 | libxcb xorgproto xtrans | libxcb | 3 |
| [`libxcb`](ports/libxcb/) | 1.17.0 | xcb-proto libxau libxdmcp xtrans pthread-stubs | libxau libxdmcp | 2 |
| [`libxcomposite`](ports/libxcomposite/) | 0.4.7 | libx11 libxfixes xorgproto | libx11 | 4 |
| [`libxcursor`](ports/libxcursor/) | 1.2.3 | libx11 libxrender libxfixes xorgproto | libx11 libxfixes libxrender | 6 |
| [`libxdamage`](ports/libxdamage/) | 1.1.7 | libx11 libxfixes xorgproto | libx11 libxfixes | 5 |
| [`libxext`](ports/libxext/) | 1.3.7 | libx11 xorgproto | libx11 | 4 |
| [`libxfce4ui`](ports/libxfce4ui/) | 4.20.2 | glib gtk3 libxfce4util xfconf libepoxy | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4util pango xfconf zlib | 49 |
| [`libxfce4util`](ports/libxfce4util/) | 4.20.1 | glib | gettext glib | 8 |
| [`libxfce4windowing`](ports/libxfce4windowing/) | 4.20.7 | glib gtk3 wayland wayland-protocols | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango wayland zlib | 47 |
| [`libxfixes`](ports/libxfixes/) | 6.0.2 | libx11 xorgproto | libx11 | 4 |
| [`libxfont2`](ports/libxfont2/) | 2.0.10 | freetype zlib fontenc xorgproto xtrans | fontenc freetype zlib | 6 |
| [`libxft`](ports/libxft/) | 2.3.9 | libx11 libxrender freetype fontconfig xorgproto | fontconfig freetype libx11 libxrender | 16 |
| [`libxi`](ports/libxi/) | 1.8.4 | libx11 libxext libxfixes xorgproto | libx11 libxext | 5 |
| [`libxinerama`](ports/libxinerama/) | 1.1.6 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxkbcommon`](ports/libxkbcommon/) | 1.8.1 | libxml2 | libxml2 | 3 |
| [`libxkbfile`](ports/libxkbfile/) | 1.2.0 | libx11 xorgproto | libx11 | 4 |
| [`libxml2`](ports/libxml2/) | 2.13.5 | zlib xz python3 | xz zlib | 2 |
| [`libxml2-utils`](ports/libxml2-utils/) | 2.13.5 | libxml2 zlib xz | libxml2 | 3 |
| [`libxmlb`](ports/libxmlb/) | 0.3.30 | glib xz zstd | glib xz zstd | 10 |
| [`libxmu`](ports/libxmu/) | 1.3.1 | libx11 libxext libxt xorgproto | libx11 libxext libxt | 8 |
| [`libxrandr`](ports/libxrandr/) | 1.5.5 | libx11 libxext libxrender xorgproto | libx11 libxext libxrender | 6 |
| [`libxrender`](ports/libxrender/) | 0.9.13 | libx11 xorgproto | libx11 | 4 |
| [`libxscrnsaver`](ports/libxscrnsaver/) | 1.2.5 | libx11 libxext xorgproto | libx11 libxext | 5 |
| [`libxslt`](ports/libxslt/) | 1.1.45 | libxml2 zlib | libxml2 | 3 |
| [`libxt`](ports/libxt/) | 1.3.1 | libx11 libsm libice xorgproto | libice libsm libx11 | 6 |
| [`libxtst`](ports/libxtst/) | 1.2.6 | libx11 libxext libxfixes libxi xorgproto | libx11 libxext libxi | 6 |
| [`libyuv`](ports/libyuv/) | stable | libjpeg-turbo | libjpeg-turbo | 1 |
| [`libzip`](ports/libzip/) | 1.12.1 | zlib bzip2 xz zstd openssl | bzip2 openssl xz zlib zstd | 5 |
| [`libzmq`](ports/libzmq/) | 4.3.5 | libsodium | libsodium | 1 |
| [`lighttpd`](ports/lighttpd/) | 1.4.85 | openssl pcre2 zlib | libxcrypt openssl pcre2 zlib | 4 |
| [`litehtml`](ports/litehtml/) | 0.10 | gumbo-parser | gumbo-parser | 1 |
| [`llama.cpp`](ports/llama.cpp/) | 0_b9006 | – | openssl | 1 |
| [`lldpd`](ports/lldpd/) | 1.0.23 | libevent | libevent | 2 |
| [`llvm`](ports/llvm/) | 22.1.8 | zlib zstd libffi libxml2 | libxml2 zlib zstd | 4 |
| [`log4cxx`](ports/log4cxx/) | 1.8.0 | apr apr-util expat libiconv | apr apr-util expat libiconv | 8 |
| [`luv`](ports/luv/) | 1.53.0-1 | libuv luajit | libuv | 1 |
| [`lynx`](ports/lynx/) | 2.9.2 | ncurses openssl zlib brotli bzip2 libidn2 libiconv | brotli bzip2 libiconv libidn2 ncurses openssl zlib | 9 |
| [`m4`](ports/m4/) | 1.4.21 | libiconv | libiconv | 1 |
| [`mame`](ports/mame/) | 0.281 | sdl2 sdl2-ttf fontconfig freetype zlib | fontconfig freetype sdl2 sdl2-ttf zlib | 19 |
| [`man-db`](ports/man-db/) | 2.13.3 | libpipeline gdbm groff libiconv zlib | gdbm libiconv libpipeline zlib | 4 |
| [`mate-calc`](ports/mate-calc/) | 1.28.0 | glib gtk3 libxml2 gmp mpfr mpc | at-spi2-core cairo gdk-pixbuf gettext glib gmp gtk3 harfbuzz libxml2 mpc mpfr pango zlib | 50 |
| [`mesa`](ports/mesa/) | 26.3.1 | llvm wayland wayland-protocols zlib zstd expat | expat llvm spirv-tools wayland zlib zstd | 9 |
| [`metis`](ports/metis/) | 5.2.1 | gklib | gklib | 1 |
| [`minicom`](ports/minicom/) | 2.11.1 | ncurses libiconv | libiconv ncurses | 2 |
| [`mkfontscale`](ports/mkfontscale/) | 1.2.4 | freetype fontenc zlib | fontenc freetype zlib | 6 |
| [`mlpack`](ports/mlpack/) | 4.8.0 | armadillo ensmallen cereal | armadillo | 2 |
| [`mosh`](ports/mosh/) | 1.4.0 | protobuf ncurses openssl zlib | abseil libutempter ncurses openssl protobuf zlib | 6 |
| [`mosquitto`](ports/mosquitto/) | 2.1.4 | openssl cjson | cjson libedit libmicrohttpd openssl sqlite | 7 |
| [`mpc`](ports/mpc/) | 1.4.1 | gmp mpfr | gmp mpfr | 2 |
| [`mpfr`](ports/mpfr/) | 4.2.2 | gmp | gmp | 1 |
| [`nano`](ports/nano/) | 8.7.1 | ncurses zlib file | file ncurses zlib | 6 |
| [`neon`](ports/neon/) | 0.37.1 | openssl expat zlib | expat openssl zlib | 3 |
| [`neovim`](ports/neovim/) | 0.12.6 | luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext | gettext libiconv luv tree-sitter unibilium utf8proc | 9 |
| [`nettle`](ports/nettle/) | 3.10.1 | gmp | gmp | 1 |
| [`nginx`](ports/nginx/) | 1.31.9 | openssl pcre2 zlib libxcrypt | libxcrypt openssl pcre2 zlib | 4 |
| [`nodejs`](ports/nodejs/) | 24.14.1 | libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu | ada brotli c-ares icu libuv nghttp2 openssl simdjson simdutf sqlite zlib zstd | 12 |
| [`nss`](ports/nss/) | 3.131.1 | nspr sqlite zlib | nspr sqlite zlib | 3 |
| [`ntp`](ports/ntp/) | 4.2.8p18 | libevent libedit | libedit libevent | 4 |
| [`ntpsec`](ports/ntpsec/) | 1.2.6 | openssl | openssl | 1 |
| [`numpy`](ports/numpy/) | 2.5.5 | python3 openblas | openblas python3 | 17 |
| [`nuspell`](ports/nuspell/) | 5.1.9 | icu | icu | 1 |
| [`ompl`](ports/ompl/) | 1.7.0 | boost eigen | boost | 5 |
| [`onnx`](ports/onnx/) | 1.23.2 | protobuf | abseil protobuf | 3 |
| [`onnxruntime`](ports/onnxruntime/) | 1.20.1 | protobuf | libiconv zlib | 2 |
| [`open62541`](ports/open62541/) | 1.5.10 | openssl | openssl | 1 |
| [`opencv`](ports/opencv/) | 4.10.0 | zlib eigen | zlib | 1 |
| [`openexr`](ports/openexr/) | 3.5.4 | imath libdeflate openjph | imath libdeflate openjph zstd | 9 |
| [`openjph`](ports/openjph/) | 0.33.0 | libtiff | libtiff | 7 |
| [`openssh`](ports/openssh/) | 10.6p2 | openssl zlib libxcrypt | libmd libxcrypt openssl zlib | 4 |
| [`opentelemetry-cpp`](ports/opentelemetry-cpp/) | 1.30.0 | protobuf grpc abseil curl nlohmann_json zlib openssl c-ares re2 | abseil c-ares curl grpc openssl protobuf re2 zlib | 16 |
| [`openttd`](ports/openttd/) | 15.3 | sdl2 zlib xz libpng freetype fontconfig curl lzo | curl freetype libogg libpng lzo opus opusfile sdl2 xz zlib | 21 |
| [`opusfile`](ports/opusfile/) | 0.12 | opus libogg openssl | libogg openssl opus | 3 |
| [`p11-kit`](ports/p11-kit/) | 0.26.6 | libffi libtasn1 | libffi | 1 |
| [`paho.mqtt.c`](ports/paho.mqtt.c/) | 1.3.16 | openssl | openssl | 1 |
| [`paho.mqtt.cpp`](ports/paho.mqtt.cpp/) | 1.6.0 | paho.mqtt.c | openssl paho.mqtt.c | 2 |
| [`pandas`](ports/pandas/) | 2.3.3 | numpy python3-dateutil python3-pytz python3-tzdata | python3 | 16 |
| [`pango`](ports/pango/) | 1.56.4 | glib harfbuzz fribidi cairo fontconfig freetype libxft | cairo fontconfig freetype fribidi glib harfbuzz libx11 libxft libxrender | 28 |
| [`parole`](ports/parole/) | 4.20.0 | glib gtk3 dbus dbus-glib libxfce4util libxfce4ui xfconf gstreamer gst-plugins-base taglib libnotify | cairo dbus dbus-glib gdk-pixbuf gettext glib gst-plugins-base gstreamer gtk3 libnotify libxfce4ui libxfce4util taglib xfconf | 59 |
| [`pcl`](ports/pcl/) | 1.15.1 | boost eigen flann | boost flann lz4 | 7 |
| [`perl`](ports/perl/) | 5.40.2 | libxcrypt | libxcrypt | 1 |
| [`perl-net-ssleay`](ports/perl-net-ssleay/) | 1.97_01 | perl openssl zlib | openssl zlib | 2 |
| [`perl-xml-parser`](ports/perl-xml-parser/) | 2.59 | perl expat perl-file-sharedir-install | expat | 1 |
| [`pinentry`](ports/pinentry/) | 1.3.3 | libassuan libgpg-error ncurses libiconv | libassuan libgpg-error libiconv ncurses | 6 |
| [`pinocchio`](ports/pinocchio/) | 3.9.0 | eigen boost urdfdom urdfdom_headers console_bridge tinyxml2 | boost urdfdom | 8 |
| [`piper`](ports/piper/) | 1.8.0 | espeak-ng onnxruntime | espeak-ng onnxruntime | 4 |
| [`pistache`](ports/pistache/) | 0.4.26 | libevent rapidjson openssl | libevent openssl | 2 |
| [`pocl`](ports/pocl/) | 7.2 | llvm zlib zstd opencl-headers opencl-icd-loader | llvm opencl-icd-loader zlib zstd | 6 |
| [`poco`](ports/poco/) | 1.15.4 | openssl zlib pcre2 utf8proc expat | expat openssl pcre2 utf8proc zlib | 5 |
| [`poppler`](ports/poppler/) | 26.11.0 | freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib | brotli cairo fontconfig freetype gettext glib harfbuzz lcms2 libiconv libjpeg-turbo libpng libtiff openjpeg zlib | 34 |
| [`pqiv`](ports/pqiv/) | 2.13.3 | glib gtk3 gdk-pixbuf cairo | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libx11 pango zlib | 47 |
| [`protobuf`](ports/protobuf/) | 29.3 | zlib abseil | abseil zlib | 2 |
| [`pygame`](ports/pygame/) | 2.6.1_git20251005 | python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo | freetype libjpeg-turbo libpng sdl2 sdl2-image sdl2-mixer sdl2-ttf | 32 |
| [`python3`](ports/python3/) | 3.14.0 | zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd | bzip2 expat gdbm gettext libffi libuuid ncurses openssl readline sqlite xz zlib zstd | 15 |
| [`python3-cairo`](ports/python3-cairo/) | 1.29.2 | python3 cairo | cairo python3 | 31 |
| [`python3-cffi`](ports/python3-cffi/) | 2.1.1 | python3 libffi python3-cparser | libffi | 1 |
| [`python3-contourpy`](ports/python3-contourpy/) | 1.4.0 | numpy | python3 | 16 |
| [`python3-dbus`](ports/python3-dbus/) | 1.5.1 | python3 dbus glib | dbus glib python3 | 19 |
| [`python3-gobject3`](ports/python3-gobject3/) | 3.58.1 | python3 glib libffi cairo python3-cairo glib-typelibs | cairo glib libffi python3 | 31 |
| [`python3-lxml`](ports/python3-lxml/) | 6.1.3 | python3 libxml2 libxslt zlib | libxml2 libxslt zlib | 4 |
| [`python3-matplotlib`](ports/python3-matplotlib/) | 3.11.2 | numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil | freetype libraqm python3 qhull | 27 |
| [`python3-pillow`](ports/python3-pillow/) | 13.0.0 | python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm | freetype lcms2 libjpeg-turbo libraqm libtiff libwebp openjpeg zlib | 25 |
| [`python3-scipy`](ports/python3-scipy/) | 1.18.1 | numpy openblas pythran pybind11 | openblas python3 | 17 |
| [`python3-zstandard`](ports/python3-zstandard/) | 0.25.0 | python3 zstd | zstd | 1 |
| [`pytorch`](ports/pytorch/) | 2.3.1 | python3 protobuf openblas | abseil openblas protobuf | 4 |
| [`qdirstat`](ports/qdirstat/) | 2.0 | qt6 zlib gettext | gettext qt6 zlib | 59 |
| [`qpdf`](ports/qpdf/) | 11.10.1 | zlib libjpeg-turbo openssl | libjpeg-turbo openssl zlib | 3 |
| [`qt6`](ports/qt6/) | 6.8.1 | zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd wayland libxkbcommon | brotli double-conversion ffmpeg fontconfig freetype glib gst-plugins-bad gst-plugins-base gstreamer harfbuzz icu libjpeg-turbo libpng libtiff libwebp libxkbcommon openssl pcre2 zlib zstd | 58 |
| [`qt6-3d`](ports/qt6-3d/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-datavis3d`](ports/qt6-datavis3d/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-graphs`](ports/qt6-graphs/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-httpserver`](ports/qt6-httpserver/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-location`](ports/qt6-location/) | 6.8.1 | qt6 qt6-positioning | libxkbcommon qt6 qt6-positioning | 60 |
| [`qt6-lottie`](ports/qt6-lottie/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-mqtt`](ports/qt6-mqtt/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-networkauth`](ports/qt6-networkauth/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-positioning`](ports/qt6-positioning/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-quickeffectmaker`](ports/qt6-quickeffectmaker/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-remoteobjects`](ports/qt6-remoteobjects/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-scxml`](ports/qt6-scxml/) | 6.8.1 | qt6 | libxkbcommon qt6 | 59 |
| [`qt6-sensors`](ports/qt6-sensors/) | 6.8.1 | qt6 | qt6 | 59 |
| [`qt6-wayland`](ports/qt6-wayland/) | 6.8.1 | qt6 wayland | libxkbcommon qt6 wayland | 59 |
| [`qt6-webchannel`](ports/qt6-webchannel/) | 6.8.1 | qt6 | qt6 | 59 |
| [`quake3`](ports/quake3/) | 1.36-git-20261012 | sdl2 | sdl2 | 2 |
| [`raylib`](ports/raylib/) | 6.2 | sdl2 | sdl2 | 2 |
| [`re2`](ports/re2/) | 2024-07-02 | abseil | abseil | 1 |
| [`redis`](ports/redis/) | 8.10.3 | openssl | openssl | 1 |
| [`restbed`](ports/restbed/) | 5.0.0 | openssl asio | openssl | 1 |
| [`retroarch`](ports/retroarch/) | 1.15-git-20230408 | sdl2 freetype zlib fontconfig fribidi libass openssl | fontconfig freetype fribidi libass libxkbcommon openssl zlib | 24 |
| [`riegeli`](ports/riegeli/) | 0_git20231212 | abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash | abseil brotli bzip2 highwayhash lz4 protobuf snappy xz zlib zstd | 10 |
| [`rocksdb`](ports/rocksdb/) | 11.13.2 | lz4 zlib zstd | lz4 zlib zstd | 3 |
| [`ros2`](ports/ros2/) | jazzy | python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2 | benchmark console_bridge curl cyclonedds fastcdr fastdds lz4 opencv python3 sqlite tinyxml2 yaml-cpp zstd | 31 |
| [`ros2-jazzy-control`](ports/ros2-jazzy-control/) | 20260128 | ros2 eigen boost tinyxml2 fmt tl-expected | console_bridge curl fastcdr fmt orocos-kdl python3 tinyxml2 | 26 |
| [`ros2-jazzy-moveit2`](ports/ros2-jazzy-moveit2/) | 20260128 | ros2 ros2-jazzy-control ompl ruckig fcl libccd octomap bullet3 osqp qhull assimp boost eigen tinyxml2 fmt | assimp boost bullet3 console_bridge fastcdr fcl fmt octomap ompl openssl orocos-kdl osqp python3 qhull ruckig tinyxml2 urdfdom | 32 |
| [`ros2-jazzy-nav2`](ports/ros2-jazzy-nav2/) | 20260128 | ros2 ros2-jazzy-control boost eigen tinyxml2 geographiclib graphicsmagick xtensor xtl xsimd nanoflann nlohmann_json libzmq cppzmq sqlite libuuid ceres-solver suitesparse ompl glog opencv | ceres-solver console_bridge fastcdr geographiclib gflags glog graphicsmagick libuuid libzmq ompl opencv orocos-kdl python3 sqlite tinyxml2 yaml-cpp | 54 |
| [`ros2-rviz`](ports/ros2-rviz/) | 14.1.24 | ros2 qt6 assimp freetype zlib eigen orocos-kdl | assimp console_bridge freetype orocos-kdl qt6 tinyxml2 yaml-cpp | 64 |
| [`rsync`](ports/rsync/) | 3.5.2 | zstd lz4 xxhash openssl | libiconv libidn2 lz4 openssl xxhash zstd | 9 |
| [`rtl-sdr`](ports/rtl-sdr/) | 2.0.3 | libusb | libusb | 1 |
| [`ruby`](ports/ruby/) | 3.4.13 | zlib openssl libyaml libffi gmp libxcrypt | gmp libffi libxcrypt libyaml openssl zlib | 6 |
| [`s2n-tls`](ports/s2n-tls/) | 1.7.14 | openssl | openssl | 1 |
| [`samba`](ports/samba/) | 4.25.1 | gnutls tdb libiconv icu zlib libxcrypt | gnutls icu libiconv libxcrypt tdb zlib | 18 |
| [`sassc`](ports/sassc/) | 3.6.2 | libsass | libsass | 1 |
| [`sdl2`](ports/sdl2/) | 2.32.10 | libiconv | libiconv | 1 |
| [`sdl2-image`](ports/sdl2-image/) | 2.8.8 | sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl | libavif libtiff libwebp sdl2 | 12 |
| [`sdl2-mixer`](ports/sdl2-mixer/) | 2.8.2 | sdl2 opusfile | opusfile sdl2 | 6 |
| [`sdl2-net`](ports/sdl2-net/) | 2.2.0 | sdl2 | sdl2 | 2 |
| [`sdl2-ttf`](ports/sdl2-ttf/) | 2.24.0 | sdl2 freetype harfbuzz | freetype harfbuzz sdl2 | 16 |
| [`sdl3`](ports/sdl3/) | 3.4.20 | libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst | libiconv libx11 libxcursor libxext libxfixes libxi libxrandr libxrender libxscrnsaver libxtst | 13 |
| [`sentencepiece`](ports/sentencepiece/) | 0.2.1 | protobuf abseil | abseil protobuf | 3 |
| [`shaderc`](ports/shaderc/) | 2026.4 | glslang spirv-tools spirv-headers | glslang spirv-tools | 2 |
| [`shared-mime-info`](ports/shared-mime-info/) | 2.4 | glib libxml2 | glib libxml2 | 10 |
| [`silkit-adapters-tap`](ports/silkit-adapters-tap/) | 1.2.1 | silkit | silkit | 1 |
| [`socat`](ports/socat/) | 1.8.1.3 | openssl readline | ncurses openssl readline | 3 |
| [`soem`](ports/soem/) | 2.0.1 | libpcap | libpcap | 1 |
| [`spdlog`](ports/spdlog/) | 1.17.0 | fmt | fmt | 1 |
| [`sqlite`](ports/sqlite/) | 3.47.2 | zlib | zlib | 1 |
| [`sscep`](ports/sscep/) | 0.10.0 | openssl | openssl | 1 |
| [`strongswan`](ports/strongswan/) | 6.1.0 | openssl | openssl | 1 |
| [`sudo`](ports/sudo/) | 1.9.17p2 | libxcrypt openssl zlib | libxcrypt openssl zlib | 3 |
| [`suitesparse`](ports/suitesparse/) | 7.14.1 | openblas | openblas | 1 |
| [`swaybg`](ports/swaybg/) | 1.2.2 | wayland wayland-protocols cairo gdk-pixbuf glib | cairo gdk-pixbuf glib wayland | 26 |
| [`swig`](ports/swig/) | 4.3.1 | pcre2 | pcre2 | 1 |
| [`sysbench`](ports/sysbench/) | 1.0.20-git-20250309 | luajit | luajit | 1 |
| [`taglib`](ports/taglib/) | 2.3.2 | zlib | zlib | 1 |
| [`tar`](ports/tar/) | 1.35 | libiconv | libiconv | 1 |
| [`tcl`](ports/tcl/) | 8.6.17 | zlib | zlib | 1 |
| [`tcpdump`](ports/tcpdump/) | 4.99.7 | libpcap | libpcap | 1 |
| [`texinfo`](ports/texinfo/) | 7.3 | ncurses | ncurses | 1 |
| [`tftp-hpa`](ports/tftp-hpa/) | 7.2 | readline ncurses | ncurses readline | 2 |
| [`thorvg`](ports/thorvg/) | 1.1.4 | libjpeg-turbo libpng zlib | libjpeg-turbo libpng | 3 |
| [`thunar`](ports/thunar/) | 4.20.10 | glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel | at-spi2-core cairo exo gdk-pixbuf gettext glib gtk3 harfbuzz libexif libice libnotify libsm libx11 libxfce4ui libxfce4util pango pcre2 xfce4-panel xfconf zlib | 59 |
| [`tmux`](ports/tmux/) | 3.8 | libevent ncurses | libevent ncurses | 3 |
| [`tumbler`](ports/tumbler/) | 4.20.2 | glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer | cairo curl ffmpegthumbnailer freetype gdk-pixbuf gettext glib gst-plugins-base gstreamer libgsf libjpeg-turbo libpng libxfce4util poppler | 59 |
| [`unbound`](ports/unbound/) | 1.26.2 | openssl expat | expat openssl | 2 |
| [`up-core-api`](ports/up-core-api/) | 1.6.0-alpha.7 | protobuf | abseil protobuf | 3 |
| [`up-cpp`](ports/up-cpp/) | 1.0.1 | protobuf up-core-api | abseil protobuf up-core-api | 4 |
| [`up-transport-zenoh-cpp`](ports/up-transport-zenoh-cpp/) | 1.0.0-git-20250319 | zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog | abseil fmt protobuf spdlog up-core-api up-cpp zenoh-c | 8 |
| [`urdfdom`](ports/urdfdom/) | 4.0.3 | urdfdom_headers console_bridge tinyxml2 | console_bridge tinyxml2 | 2 |
| [`vala`](ports/vala/) | 0.56.19 | glib gettext | gettext glib | 8 |
| [`valkey`](ports/valkey/) | 9.2.0 | openssl | openssl | 1 |
| [`vim`](ports/vim/) | 9.2.1382 | ncurses libiconv libsodium | cairo gettext glib harfbuzz libiconv libsodium ncurses pango pixman wayland zlib | 31 |
| [`virtualgl`](ports/virtualgl/) | 3.1.5 | libx11 libxext libjpeg-turbo | libjpeg-turbo libx11 libxext | 6 |
| [`vlc`](ports/vlc/) | 3.0.24 | bzip2 libiconv lz4 openssl xz zstd | bzip2 libb2 libiconv lz4 openssl xz zstd | 7 |
| [`vsomeip`](ports/vsomeip/) | 3.8.0 | boost | boost | 5 |
| [`vte`](ports/vte/) | 0.78.7 | gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4 | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gnutls graphene gtk3 gtk4 icu lz4 pango pcre2 | 61 |
| [`wayland`](ports/wayland/) | 1.26.0 | libffi expat libxml2 | libffi | 1 |
| [`webkitgtk`](ports/webkitgtk/) | 2.54.3 | gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret expat | at-spi2-core cairo expat fontconfig freetype gdk-pixbuf gettext glib gst-plugins-base gstreamer gtk3 harfbuzz hyphen icu lcms2 libavif libepoxy libgcrypt libgpg-error libjpeg-turbo libjxl libpng libsecret libsoup libtasn1 libwebp libxml2 libxslt pango sqlite zlib | 72 |
| [`webrtc-screencast`](ports/webrtc-screencast/) | 1.1 | webrtc wayland wlr-protocols libxkbcommon | libxkbcommon wayland | 6 |
| [`wget`](ports/wget/) | 1.25.0 | gnutls libidn2 zlib pcre2 libpsl | gnutls libiconv libpsl libunistring nettle pcre2 zlib | 17 |
| [`wlroots`](ports/wlroots/) | 0.20.2 | wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes | lcms2 libxkbcommon pixman wayland | 8 |
| [`woff2`](ports/woff2/) | 1.0.2 | brotli | brotli | 1 |
| [`xcb-util`](ports/xcb-util/) | 0.4.1 | libxcb | libxcb | 3 |
| [`xfce4-appfinder`](ports/xfce4-appfinder/) | 4.20.0 | glib gtk3 libxfce4util libxfce4ui xfconf garcon | cairo garcon gdk-pixbuf glib gtk3 libxfce4ui libxfce4util xfconf | 51 |
| [`xfce4-dev-tools`](ports/xfce4-dev-tools/) | 4.21.2 | glib | gettext glib | 8 |
| [`xfce4-panel`](ports/xfce4-panel/) | 4.20.8 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 libxfce4ui libxfce4util libxfce4windowing pango wayland xfconf | 54 |
| [`xfce4-session`](ports/xfce4-session/) | 4.20.4 | glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell | at-spi2-core cairo gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util libxfce4windowing pango xfconf zlib | 52 |
| [`xfce4-settings`](ports/xfce4-settings/) | 4.20.5 | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 libxfce4ui libxfce4util pango wayland xfconf | 53 |
| [`xfce4-terminal`](ports/xfce4-terminal/) | 1.1.5 | glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libutempter libxfce4ui libxfce4util pango pcre2 vte xfconf | 67 |
| [`xfconf`](ports/xfconf/) | 4.20.0 | glib libxfce4util dbus | gettext glib libxfce4util | 9 |
| [`xkbcomp`](ports/xkbcomp/) | 1.5.0 | libx11 libxkbfile xorgproto | libx11 libxkbfile | 5 |
| [`xnnpack`](ports/xnnpack/) | 0-git-20250209 | cpuinfo pthreadpool fxdiv | pthreadpool | 1 |
| [`xsltproc`](ports/xsltproc/) | 1.1.45 | libxslt libxml2 | libxml2 libxslt | 4 |
| [`xwayland`](ports/xwayland/) | 24.1.14 | wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util | libmd libxau libxcvt libxfont2 pixman wayland | 13 |
| [`yubico-c-client`](ports/yubico-c-client/) | 2.15 | curl | curl | 12 |
| [`zsh`](ports/zsh/) | 5.9 | ncurses pcre2 libiconv | libiconv ncurses | 2 |

## Consistency

| Component | Run-time dependency not declared in DEPS | Unresolved soname | Linked by the -dev name |
|---|---|---|---|
| [`binutils`](ports/binutils/) | flex | – | – |
| [`ceres-solver`](ports/ceres-solver/) | metis | – | – |
| [`cmake`](ports/cmake/) | libiconv | – | – |
| [`curl`](ports/curl/) | libidn2 | – | – |
| [`enca`](ports/enca/) | libiconv | – | – |
| [`exiv2`](ports/exiv2/) | libiconv | – | – |
| [`fontconfig`](ports/fontconfig/) | gettext | – | – |
| [`gnutls`](ports/gnutls/) | unbound | – | – |
| [`graphicsmagick`](ports/graphicsmagick/) | libheif | – | – |
| [`gtsam`](ports/gtsam/) | suitesparse | – | – |
| [`lame`](ports/lame/) | mpg123 | – | – |
| [`libarchive`](ports/libarchive/) | libb2 | – | – |
| [`libass`](ports/libass/) | libunibreak | – | – |
| [`libdatrie`](ports/libdatrie/) | libiconv | – | – |
| [`libheif`](ports/libheif/) | x264 | – | – |
| [`libtiff`](ports/libtiff/) | libdeflate | – | – |
| [`libunistring`](ports/libunistring/) | libiconv | – | – |
| [`lighttpd`](ports/lighttpd/) | libxcrypt | – | – |
| [`llama.cpp`](ports/llama.cpp/) | openssl | – | – |
| [`mesa`](ports/mesa/) | spirv-tools | – | – |
| [`mosh`](ports/mosh/) | libutempter | – | – |
| [`mosquitto`](ports/mosquitto/) | libedit libmicrohttpd sqlite | – | – |
| [`onnxruntime`](ports/onnxruntime/) | libiconv | – | – |
| [`openssh`](ports/openssh/) | libmd | – | – |
| [`openttd`](ports/openttd/) | libogg opus opusfile | – | – |
| [`poppler`](ports/poppler/) | harfbuzz | – | – |
| [`python3`](ports/python3/) | libuuid | – | – |
| [`retroarch`](ports/retroarch/) | libxkbcommon | – | – |
| [`ros2-jazzy-control`](ports/ros2-jazzy-control/) | orocos-kdl | libaction_msgs__rosidl_generator_c.so libaction_msgs__rosidl_typesupport_c.so libaction_msgs__rosidl_typesupport_cpp.so libaction_msgs__rosidl_typesupport_fastrtps_c.so libaction_msgs__rosidl_typesupport_introspection_c.so libament_index_cpp.so libbuiltin_interfaces__rosidl_generator_c.so libbuiltin_interfaces__rosidl_generator_py.so libbuiltin_interfaces__rosidl_typesupport_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_cpp.so libbuiltin_interfaces__rosidl_typesupport_introspection_c.so libbuiltin_interfaces__rosidl_typesupport_introspection_cpp.so libclass_loader.so libdiagnostic_msgs__rosidl_typesupport_cpp.so libgeometry_msgs__rosidl_generator_c.so libgeometry_msgs__rosidl_generator_py.so libgeometry_msgs__rosidl_typesupport_c.so libgeometry_msgs__rosidl_typesupport_cpp.so libgeometry_msgs__rosidl_typesupport_fastrtps_c.so libgeometry_msgs__rosidl_typesupport_fastrtps_cpp.so libgeometry_msgs__rosidl_typesupport_introspection_c.so libgeometry_msgs__rosidl_typesupport_introspection_cpp.so libkdl_parser.so liblibstatistics_collector.so liblifecycle_msgs__rosidl_generator_c.so liblifecycle_msgs__rosidl_generator_py.so liblifecycle_msgs__rosidl_typesupport_c.so liblifecycle_msgs__rosidl_typesupport_fastrtps_c.so liblifecycle_msgs__rosidl_typesupport_fastrtps_cpp.so liblifecycle_msgs__rosidl_typesupport_introspection_c.so liblifecycle_msgs__rosidl_typesupport_introspection_cpp.so libnav_msgs__rosidl_typesupport_cpp.so librcl.so librcl_action.so librcl_interfaces__rosidl_typesupport_cpp.so librclcpp.so librclcpp_action.so librclcpp_lifecycle.so librcpputils.so librcutils.so librmw.so librosidl_dynamic_typesupport.so librosidl_runtime_c.so librosidl_typesupport_c.so librosidl_typesupport_cpp.so librosidl_typesupport_fastrtps_c.so librosidl_typesupport_fastrtps_cpp.so librosidl_typesupport_introspection_c.so librosidl_typesupport_introspection_cpp.so libsensor_msgs__rosidl_generator_c.so libsensor_msgs__rosidl_generator_py.so libsensor_msgs__rosidl_typesupport_c.so libsensor_msgs__rosidl_typesupport_cpp.so libsensor_msgs__rosidl_typesupport_fastrtps_c.so libsensor_msgs__rosidl_typesupport_fastrtps_cpp.so libsensor_msgs__rosidl_typesupport_introspection_c.so libsensor_msgs__rosidl_typesupport_introspection_cpp.so libservice_msgs__rosidl_generator_c.so libservice_msgs__rosidl_generator_py.so libservice_msgs__rosidl_typesupport_c.so libservice_msgs__rosidl_typesupport_fastrtps_c.so libservice_msgs__rosidl_typesupport_fastrtps_cpp.so libservice_msgs__rosidl_typesupport_introspection_c.so libservice_msgs__rosidl_typesupport_introspection_cpp.so libstatistics_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_generator_c.so libstd_msgs__rosidl_generator_py.so libstd_msgs__rosidl_typesupport_c.so libstd_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_typesupport_fastrtps_c.so libstd_msgs__rosidl_typesupport_fastrtps_cpp.so libstd_msgs__rosidl_typesupport_introspection_c.so libstd_msgs__rosidl_typesupport_introspection_cpp.so libstd_srvs__rosidl_typesupport_cpp.so libtf2.so libtf2_eigen_kdl.so libtf2_msgs__rosidl_typesupport_cpp.so libtf2_ros.so libtrajectory_msgs__rosidl_generator_c.so libtrajectory_msgs__rosidl_generator_py.so libtrajectory_msgs__rosidl_typesupport_c.so libtrajectory_msgs__rosidl_typesupport_cpp.so libtrajectory_msgs__rosidl_typesupport_fastrtps_c.so libtrajectory_msgs__rosidl_typesupport_fastrtps_cpp.so libtrajectory_msgs__rosidl_typesupport_introspection_c.so libtrajectory_msgs__rosidl_typesupport_introspection_cpp.so libunique_identifier_msgs__rosidl_generator_c.so libunique_identifier_msgs__rosidl_generator_py.so libunique_identifier_msgs__rosidl_typesupport_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_cpp.so libunique_identifier_msgs__rosidl_typesupport_introspection_c.so libunique_identifier_msgs__rosidl_typesupport_introspection_cpp.so liburdf.so | – |
| [`ros2-jazzy-moveit2`](ports/ros2-jazzy-moveit2/) | orocos-kdl urdfdom | libaction_msgs__rosidl_generator_c.so libaction_msgs__rosidl_typesupport_c.so libaction_msgs__rosidl_typesupport_cpp.so libaction_msgs__rosidl_typesupport_fastrtps_c.so libaction_msgs__rosidl_typesupport_introspection_c.so libament_index_cpp.so libbackward.so libbuiltin_interfaces__rosidl_generator_c.so libbuiltin_interfaces__rosidl_generator_py.so libbuiltin_interfaces__rosidl_typesupport_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_cpp.so libbuiltin_interfaces__rosidl_typesupport_introspection_c.so libbuiltin_interfaces__rosidl_typesupport_introspection_cpp.so libclass_loader.so libcontrol_msgs__rosidl_typesupport_cpp.so libcontroller_manager_msgs__rosidl_typesupport_cpp.so libgeometry_msgs__rosidl_generator_c.so libgeometry_msgs__rosidl_generator_py.so libgeometry_msgs__rosidl_typesupport_c.so libgeometry_msgs__rosidl_typesupport_cpp.so libgeometry_msgs__rosidl_typesupport_fastrtps_c.so libgeometry_msgs__rosidl_typesupport_fastrtps_cpp.so libgeometry_msgs__rosidl_typesupport_introspection_c.so libgeometry_msgs__rosidl_typesupport_introspection_cpp.so libkdl_parser.so liblibstatistics_collector.so librcl.so librcl_action.so librclcpp.so librclcpp_action.so librcpputils.so librcutils.so librealtime_tools.so libresource_retriever.so librmw.so librmw_implementation.so librosidl_dynamic_typesupport.so librosidl_runtime_c.so librosidl_typesupport_c.so librosidl_typesupport_cpp.so librosidl_typesupport_fastrtps_c.so librosidl_typesupport_fastrtps_cpp.so librosidl_typesupport_introspection_c.so librosidl_typesupport_introspection_cpp.so librsl.so libsensor_msgs__rosidl_generator_c.so libsensor_msgs__rosidl_generator_py.so libsensor_msgs__rosidl_typesupport_c.so libsensor_msgs__rosidl_typesupport_cpp.so libsensor_msgs__rosidl_typesupport_fastrtps_c.so libsensor_msgs__rosidl_typesupport_fastrtps_cpp.so libsensor_msgs__rosidl_typesupport_introspection_c.so libsensor_msgs__rosidl_typesupport_introspection_cpp.so libservice_msgs__rosidl_generator_c.so libservice_msgs__rosidl_generator_py.so libservice_msgs__rosidl_typesupport_c.so libservice_msgs__rosidl_typesupport_fastrtps_c.so libservice_msgs__rosidl_typesupport_fastrtps_cpp.so libservice_msgs__rosidl_typesupport_introspection_c.so libservice_msgs__rosidl_typesupport_introspection_cpp.so libshape_msgs__rosidl_generator_c.so libshape_msgs__rosidl_generator_py.so libshape_msgs__rosidl_typesupport_c.so libshape_msgs__rosidl_typesupport_fastrtps_c.so libshape_msgs__rosidl_typesupport_fastrtps_cpp.so libshape_msgs__rosidl_typesupport_introspection_c.so libshape_msgs__rosidl_typesupport_introspection_cpp.so libstatistics_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_generator_c.so libstd_msgs__rosidl_generator_py.so libstd_msgs__rosidl_typesupport_c.so libstd_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_typesupport_fastrtps_c.so libstd_msgs__rosidl_typesupport_fastrtps_cpp.so libstd_msgs__rosidl_typesupport_introspection_c.so libstd_msgs__rosidl_typesupport_introspection_cpp.so libstd_srvs__rosidl_typesupport_cpp.so libtf2.so libtf2_eigen_kdl.so libtf2_msgs__rosidl_typesupport_cpp.so libtf2_ros.so libtrajectory_msgs__rosidl_generator_c.so libtrajectory_msgs__rosidl_generator_py.so libtrajectory_msgs__rosidl_typesupport_c.so libtrajectory_msgs__rosidl_typesupport_cpp.so libtrajectory_msgs__rosidl_typesupport_fastrtps_c.so libtrajectory_msgs__rosidl_typesupport_fastrtps_cpp.so libtrajectory_msgs__rosidl_typesupport_introspection_c.so libtrajectory_msgs__rosidl_typesupport_introspection_cpp.so libunique_identifier_msgs__rosidl_generator_c.so libunique_identifier_msgs__rosidl_generator_py.so libunique_identifier_msgs__rosidl_typesupport_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_cpp.so libunique_identifier_msgs__rosidl_typesupport_introspection_c.so libunique_identifier_msgs__rosidl_typesupport_introspection_cpp.so liburdf.so libvisualization_msgs__rosidl_typesupport_cpp.so | – |
| [`ros2-jazzy-nav2`](ports/ros2-jazzy-nav2/) | orocos-kdl | libaction_msgs__rosidl_generator_c.so libaction_msgs__rosidl_typesupport_c.so libaction_msgs__rosidl_typesupport_cpp.so libaction_msgs__rosidl_typesupport_fastrtps_c.so libaction_msgs__rosidl_typesupport_introspection_c.so libament_index_cpp.so libbackward.so libbuiltin_interfaces__rosidl_generator_c.so libbuiltin_interfaces__rosidl_generator_py.so libbuiltin_interfaces__rosidl_typesupport_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_c.so libbuiltin_interfaces__rosidl_typesupport_fastrtps_cpp.so libbuiltin_interfaces__rosidl_typesupport_introspection_c.so libbuiltin_interfaces__rosidl_typesupport_introspection_cpp.so libclass_loader.so libcv_bridge.so libdiagnostic_msgs__rosidl_generator_c.so libdiagnostic_msgs__rosidl_typesupport_c.so libdiagnostic_msgs__rosidl_typesupport_fastrtps_c.so libdiagnostic_msgs__rosidl_typesupport_introspection_c.so libdiagnostic_updater.so libgeometry_msgs__rosidl_generator_c.so libgeometry_msgs__rosidl_generator_py.so libgeometry_msgs__rosidl_typesupport_c.so libgeometry_msgs__rosidl_typesupport_cpp.so libgeometry_msgs__rosidl_typesupport_fastrtps_c.so libgeometry_msgs__rosidl_typesupport_fastrtps_cpp.so libgeometry_msgs__rosidl_typesupport_introspection_c.so libgeometry_msgs__rosidl_typesupport_introspection_cpp.so liblaser_geometry.so liblibstatistics_collector.so liblifecycle_msgs__rosidl_typesupport_cpp.so libmap_msgs__rosidl_typesupport_cpp.so libmessage_filters.so libnav_msgs__rosidl_generator_c.so libnav_msgs__rosidl_generator_py.so libnav_msgs__rosidl_typesupport_c.so libnav_msgs__rosidl_typesupport_cpp.so libnav_msgs__rosidl_typesupport_fastrtps_c.so libnav_msgs__rosidl_typesupport_fastrtps_cpp.so libnav_msgs__rosidl_typesupport_introspection_c.so libnav_msgs__rosidl_typesupport_introspection_cpp.so librcl.so librcl_action.so librcl_interfaces__rosidl_typesupport_cpp.so librclcpp.so librclcpp_action.so librclcpp_lifecycle.so librcpputils.so librcutils.so librmw.so librosidl_dynamic_typesupport.so librosidl_runtime_c.so librosidl_typesupport_c.so librosidl_typesupport_cpp.so librosidl_typesupport_fastrtps_c.so librosidl_typesupport_fastrtps_cpp.so librosidl_typesupport_introspection_c.so librosidl_typesupport_introspection_cpp.so libsensor_msgs__rosidl_typesupport_cpp.so libservice_msgs__rosidl_generator_c.so libservice_msgs__rosidl_generator_py.so libservice_msgs__rosidl_typesupport_c.so libservice_msgs__rosidl_typesupport_fastrtps_c.so libservice_msgs__rosidl_typesupport_fastrtps_cpp.so libservice_msgs__rosidl_typesupport_introspection_c.so libservice_msgs__rosidl_typesupport_introspection_cpp.so libstatistics_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_generator_c.so libstd_msgs__rosidl_generator_py.so libstd_msgs__rosidl_typesupport_c.so libstd_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_typesupport_fastrtps_c.so libstd_msgs__rosidl_typesupport_fastrtps_cpp.so libstd_msgs__rosidl_typesupport_introspection_c.so libstd_msgs__rosidl_typesupport_introspection_cpp.so libstd_srvs__rosidl_typesupport_cpp.so libtf2.so libtf2_msgs__rosidl_typesupport_cpp.so libtf2_ros.so libunique_identifier_msgs__rosidl_generator_c.so libunique_identifier_msgs__rosidl_generator_py.so libunique_identifier_msgs__rosidl_typesupport_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_c.so libunique_identifier_msgs__rosidl_typesupport_fastrtps_cpp.so libunique_identifier_msgs__rosidl_typesupport_introspection_c.so libunique_identifier_msgs__rosidl_typesupport_introspection_cpp.so libvisualization_msgs__rosidl_typesupport_cpp.so | – |
| [`ros2-rviz`](ports/ros2-rviz/) | – | libament_index_cpp.so libclass_loader.so libgeometry_msgs__rosidl_typesupport_cpp.so libimage_transport.so liblaser_geometry.so liblibstatistics_collector.so libmap_msgs__rosidl_typesupport_cpp.so libmessage_filters.so libnav_msgs__rosidl_typesupport_cpp.so libpoint_cloud_transport.so librcl.so librclcpp.so librcpputils.so librcutils.so libresource_retriever.so librmw.so libsensor_msgs__rosidl_typesupport_cpp.so libstatistics_msgs__rosidl_typesupport_cpp.so libstd_msgs__rosidl_typesupport_cpp.so libstd_srvs__rosidl_typesupport_cpp.so libtf2.so libtf2_msgs__rosidl_typesupport_cpp.so libtf2_ros.so liburdf.so libvisualization_msgs__rosidl_typesupport_cpp.so | – |
| [`rsync`](ports/rsync/) | libiconv libidn2 | – | – |
| [`thunar`](ports/thunar/) | libice libsm | – | – |
| [`tumbler`](ports/tumbler/) | cairo poppler | – | – |
| [`vim`](ports/vim/) | cairo gettext glib harfbuzz pango pixman wayland zlib | – | – |
| [`vlc`](ports/vlc/) | libb2 | – | – |
| [`xfce4-terminal`](ports/xfce4-terminal/) | libutempter | – | – |

## Blocked

1 components tried and not buildable for QNX as things stand (`STATUS=blocked`; each recipe's comment says what stops it): [`weston`](ports/weston/)

---
<sub>Generated by <code>scripts/gen-dependency-index</code>. Edit recipes, not this file.</sub>
