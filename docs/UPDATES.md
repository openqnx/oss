# Upstream updates

Written by `scripts/check-updates`: the newest upstream release of every component against the version ported here. To port one: `scripts/port-update <component> [<version>]` (keeps the current version's recipe, patches, packages and sources).

| | count |
|---|---|
| update | 316 |
| current | 86 |
| unknown | 20 |
| error | 21 |
| skipped | 8 |

## Updates available

| Component | Ported | Upstream | Found via | Note |
|---|---|---|---|---|
| [`abseil`](../ports/abseil/) | 20240722.0 | **20260817.0** | tags |  |
| [`abuild`](../ports/abuild/) | 3.16.0 | **3.17.0** | tags |  |
| [`ada`](../ports/ada/) | 3.4.4 | **4.0.0** | tags |  |
| [`aom`](../ports/aom/) | 3.12.1 | **3.15.2** | tags |  |
| [`apk-tools`](../ports/apk-tools/) | 3.0.0_rc5 | **3.0.8** | tags |  |
| [`asio`](../ports/asio/) | 1.30.2 | **1-38-2** | tags |  |
| [`aspell`](../ports/aspell/) | 0.60.8.1 | **0.60.8-20190921** | tags |  |
| [`at-spi2-core`](../ports/at-spi2-core/) | 2.54.0 | **2.54.2** | listing |  |
| [`autoconf`](../ports/autoconf/) | 2.72 | **2.73** | listing |  |
| [`automake`](../ports/automake/) | 1.18 | **1.19** | listing |  |
| [`aws-c-auth`](../ports/aws-c-auth/) | 0.10.1 | **1.0.2** | tags |  |
| [`aws-c-cal`](../ports/aws-c-cal/) | 0.9.13 | **1.0.0** | tags |  |
| [`aws-c-common`](../ports/aws-c-common/) | 0.12.6 | **1.0.5** | tags |  |
| [`aws-c-compression`](../ports/aws-c-compression/) | 0.3.2 | **1.0.0** | tags |  |
| [`aws-c-event-stream`](../ports/aws-c-event-stream/) | 0.7.0 | **1.0.0** | tags |  |
| [`aws-c-http`](../ports/aws-c-http/) | 0.10.14 | **1.1.1** | tags |  |
| [`aws-c-io`](../ports/aws-c-io/) | 0.26.1 | **1.1.1** | tags |  |
| [`aws-c-mqtt`](../ports/aws-c-mqtt/) | 0.15.2 | **1.1.2** | tags |  |
| [`aws-c-s3`](../ports/aws-c-s3/) | 0.12.2 | **1.5.1** | tags |  |
| [`aws-c-sdkutils`](../ports/aws-c-sdkutils/) | 0.2.4 | **1.0.1** | tags |  |
| [`aws-checksums`](../ports/aws-checksums/) | 0.2.10 | **1.0.1** | tags |  |
| [`aws-crt-cpp`](../ports/aws-crt-cpp/) | 0.38.5 | **0.44.2** | tags | custom fetch: check the recipe |
| [`azure-iot-sdk-c`](../ports/azure-iot-sdk-c/) | LTS_08_2024 | **20151102** | tags |  |
| [`bash`](../ports/bash/) | 5.2.37 | **5.3** | listing |  |
| [`benchmark`](../ports/benchmark/) | 1.9.0 | **1.10.0** | tags |  |
| [`binutils`](../ports/binutils/) | 2.43 | **2.47** | listing |  |
| [`brotli`](../ports/brotli/) | 1.1.0 | **1.2.1** | tags |  |
| [`busybox`](../ports/busybox/) | 1.36.1 | **1.38.0** | tags |  |
| [`c-ares`](../ports/c-ares/) | 1.34.3 | **1.34.9** | tags |  |
| [`cairo`](../ports/cairo/) | 1.18.2 | **1.18.6** | tags |  |
| [`capnproto`](../ports/capnproto/) | 1.0.2 | **1.5.0** | tags |  |
| [`catch2`](../ports/catch2/) | 3.7.1 | **3.16.1** | tags |  |
| [`chrony`](../ports/chrony/) | 4.8-git-20260226 | **4.9** | tags |  |
| [`cjson`](../ports/cjson/) | 1.7.18 | **1.7.19** | tags |  |
| [`cli11`](../ports/cli11/) | 2.4.2 | **2.7.2** | tags |  |
| [`cmake`](../ports/cmake/) | 4.1.1 | **4.4.5** | tags |  |
| [`computelibrary`](../ports/computelibrary/) | 53.0.0 | **53.3.1** | tags |  |
| [`cppzmq`](../ports/cppzmq/) | 4.10.0 | **4.11.0** | tags |  |
| [`cpuinfo`](../ports/cpuinfo/) | 0-git-20240417 | **d53799f29aee** | branch | pinned commit 3c8b1533ac03 |
| [`curl`](../ports/curl/) | 8.11.1 | **8.23.0** | tags |  |
| [`cyclonedds`](../ports/cyclonedds/) | 0.10.5 | **11.0.1** | tags |  |
| [`dav1d`](../ports/dav1d/) | 1.5.0 | **1.5.4** | tags |  |
| [`dbus`](../ports/dbus/) | 1.16.0 | **1.16.2** | tags |  |
| [`dlt-daemon`](../ports/dlt-daemon/) | 2.18.10 | **3.0.0** | tags |  |
| [`double-conversion`](../ports/double-conversion/) | 3.3.0 | **3.4.0** | tags |  |
| [`dump1090`](../ports/dump1090/) | 0-git-20240121 | **efe64db3c6ba** | branch | pinned commit 0c3bb23eb447 |
| [`eigen`](../ports/eigen/) | 3.4.0 | **5.0.1** | tags |  |
| [`emacs`](../ports/emacs/) | 30.2 | **31.1** | listing |  |
| [`emulationstation`](../ports/emulationstation/) | 2.11-git-20241212 | **2.11.2** | tags |  |
| [`enca`](../ports/enca/) | 1.19 | **1.22** | tags |  |
| [`enchant`](../ports/enchant/) | 2.8.2 | **2.8.21** | tags |  |
| [`exiv2`](../ports/exiv2/) | 0.28.5 | **0.28.9** | tags |  |
| [`expat`](../ports/expat/) | 2.6.4 | **20000512** | tags |  |
| [`fakeroot`](../ports/fakeroot/) | 1.37.1.1 | **2.1.4.orig** | listing |  |
| [`farmhash`](../ports/farmhash/) | 1.1-git-20190513 | **9d99331eb762** | branch | pinned commit 0d859a811870 |
| [`fastcdr`](../ports/fastcdr/) | 2.2.6 | **2.4.0** | tags |  |
| [`fastdds`](../ports/fastdds/) | 2.14.4 | **3.6.2.0** | tags |  |
| [`fdk-aac`](../ports/fdk-aac/) | 2.0.3 | **3.0.0** | tags |  |
| [`ffmpegthumbnailer`](../ports/ffmpegthumbnailer/) | 2.2.3 | **2.4.0** | tags |  |
| [`file`](../ports/file/) | 5.46 | **5.48** | listing |  |
| [`flac`](../ports/flac/) | 1.4.3 | **1.5.0** | tags |  |
| [`flatbuffers`](../ports/flatbuffers/) | 24.3.25 | **25.12.19** | tags |  |
| [`fmt`](../ports/fmt/) | 11.2.0 | **12.2.0** | tags |  |
| [`font-noto-emoji`](../ports/font-noto-emoji/) | 2.048 | **2.051** | tags | custom fetch: check the recipe |
| [`font-util`](../ports/font-util/) | 1.4.1 | **1.4.2** | listing |  |
| [`fontconfig`](../ports/fontconfig/) | 2.15.0 | **2.18.3** | tags |  |
| [`foonathan_memory`](../ports/foonathan_memory/) | 0.7-3 | **0.7-4** | tags |  |
| [`freetype`](../ports/freetype/) | 2.13.3 | **2-15-0** | tags |  |
| [`fribidi`](../ports/fribidi/) | 1.0.16 | **1.0.17** | tags |  |
| [`fxdiv`](../ports/fxdiv/) | 0-git-20200417 | **63058eff77e1** | branch | pinned commit b408327ac2a1 |
| [`g3log`](../ports/g3log/) | 2.4 | **2.6.2** | tags |  |
| [`gawk`](../ports/gawk/) | 5.3.2 | **5.4.1** | listing |  |
| [`gcr`](../ports/gcr/) | 3.41.2 | **4.4.1** | tags |  |
| [`gdb`](../ports/gdb/) | 14.2 | **18.1** | listing |  |
| [`gdbm`](../ports/gdbm/) | 1.24 | **1.26** | listing |  |
| [`gdk-pixbuf`](../ports/gdk-pixbuf/) | 2.42.12 | **2.90.3** | tags |  |
| [`gettext`](../ports/gettext/) | 0.22.5 | **1.0** | listing |  |
| [`gflags`](../ports/gflags/) | 2.2.2 | **2.3.1** | tags |  |
| [`git`](../ports/git/) | 2.51.0 | **2.56.0** | listing |  |
| [`gklib`](../ports/gklib/) | 5.1.1-git-20230326 | **3b7d61b9f885** | branch | pinned commit 8bd6bad750b2 |
| [`glib`](../ports/glib/) | 2.82.4 | **2.90.1** | tags |  |
| [`glib-networking`](../ports/glib-networking/) | 2.80.1 | **2.90.1** | tags |  |
| [`glm`](../ports/glm/) | 1.0.1 | **1.0.3** | tags |  |
| [`gnutls`](../ports/gnutls/) | 3.8.8 | **3.8.13** | listing |  |
| [`googletest`](../ports/googletest/) | 1.17.0 | **1.18.0** | tags |  |
| [`gperf`](../ports/gperf/) | 3.1 | **3.3** | listing |  |
| [`graphite2`](../ports/graphite2/) | 1.3.14 | **1.3.15** | tags |  |
| [`grep`](../ports/grep/) | 3.11 | **3.12** | listing |  |
| [`groff`](../ports/groff/) | 1.23.0 | **1.24.2** | listing |  |
| [`grpc`](../ports/grpc/) | 1.69.0 | **1.85.0** | tags |  |
| [`gsettings-desktop-schemas`](../ports/gsettings-desktop-schemas/) | 49.0 | **51.0** | tags |  |
| [`gsm`](../ports/gsm/) | 1.0.22 | **1.0.24** | listing |  |
| [`gstreamer`](../ports/gstreamer/) | 1.24.10 | **1.29.2** | tags |  |
| [`gthumb`](../ports/gthumb/) | 3.12.8.2 | **3.12.12** | listing |  |
| [`gtk-layer-shell`](../ports/gtk-layer-shell/) | 0.9.0 | **0.10.1** | tags |  |
| [`gtk3`](../ports/gtk3/) | 3.24.43 | **4.24.1** | tags |  |
| [`gtk4`](../ports/gtk4/) | 4.16.7 | **4.16.13** | listing |  |
| [`gtsam`](../ports/gtsam/) | 4.2 | **4.3.0** | tags |  |
| [`harfbuzz`](../ports/harfbuzz/) | 10.1.0 | **15.0.0** | tags |  |
| [`highway`](../ports/highway/) | 1.2.0 | **1.4.0** | tags |  |
| [`iceoryx`](../ports/iceoryx/) | 2.0.6 | **2.95.8** | tags |  |
| [`icu`](../ports/icu/) | 76.1 | **79.1** | tags |  |
| [`iperf3`](../ports/iperf3/) | 3.17.1 | **3.22** | tags |  |
| [`jansson`](../ports/jansson/) | 2.14 | **2.15.1** | tags |  |
| [`javascriptcore`](../ports/javascriptcore/) | 2.44.4 | **2.54.2** | listing |  |
| [`jq`](../ports/jq/) | 1.7.1 | **1.8.2** | tags |  |
| [`json-c`](../ports/json-c/) | 0.18 | **0.19-20260627** | tags |  |
| [`json-glib`](../ports/json-glib/) | 1.10.6 | **1.10.8** | tags |  |
| [`jsoncpp`](../ports/jsoncpp/) | 1.9.6 | **1.9.8** | tags |  |
| [`kineto`](../ports/kineto/) | 0-git-20250214 | **0.4.0** | tags |  |
| [`lely-core`](../ports/lely-core/) | 2.3.4 | **3.2.0** | tags |  |
| [`libarchive`](../ports/libarchive/) | 3.7.7 | **3.8.10** | tags |  |
| [`libass`](../ports/libass/) | 0.17.3 | **0.17.5** | tags |  |
| [`libavif`](../ports/libavif/) | 1.1.1 | **1.4.2** | tags |  |
| [`libcamera`](../ports/libcamera/) | 0.5.1+rpt20250722 | **1.2** | tags |  |
| [`libcbor`](../ports/libcbor/) | 0.11.0 | **0.14.0** | tags |  |
| [`libcoap`](../ports/libcoap/) | 4.3.5 | **4.3.5a** | tags |  |
| [`libconfig`](../ports/libconfig/) | 1.8.1 | **1.8.3** | tags |  |
| [`libdatrie`](../ports/libdatrie/) | 0.2.13 | **0.2.14** | tags |  |
| [`libde265`](../ports/libde265/) | 1.0.15 | **1.1.3** | tags |  |
| [`libdeflate`](../ports/libdeflate/) | 1.22 | **1.26** | tags |  |
| [`libdisplay-info`](../ports/libdisplay-info/) | 0.3.0 | **0.4.0** | tags |  |
| [`libexif`](../ports/libexif/) | 0.6.24 | **0.6.26** | tags |  |
| [`libfdt`](../ports/libfdt/) | 1.7.2 | **1.8.1** | tags |  |
| [`libffi`](../ports/libffi/) | 3.4.6 | **3.8.0** | tags |  |
| [`libgcrypt`](../ports/libgcrypt/) | 1.11.0 | **1.12.4** | listing |  |
| [`libgit2`](../ports/libgit2/) | 1.8.4 | **1.9.7** | tags |  |
| [`libgpg-error`](../ports/libgpg-error/) | 1.50 | **1.61** | listing |  |
| [`libgsf`](../ports/libgsf/) | 1.14.53 | **1.14.60** | tags |  |
| [`libiconv`](../ports/libiconv/) | 1.18 | **1.19** | listing |  |
| [`libidn2`](../ports/libidn2/) | 2.3.7 | **2.3.8** | listing |  |
| [`libjpeg-turbo`](../ports/libjpeg-turbo/) | 3.0.4 | **3.2.1** | tags |  |
| [`libjxl`](../ports/libjxl/) | 0.11.1 | **0.12.0** | tags |  |
| [`libmetal`](../ports/libmetal/) | 2024.10.0 | **2026.04.0** | tags |  |
| [`libmicrohttpd`](../ports/libmicrohttpd/) | 1.0.1 | **1.0.10** | listing |  |
| [`libmodbus`](../ports/libmodbus/) | 3.1.11 | **3.2.0** | tags |  |
| [`libnotify`](../ports/libnotify/) | 0.8.3 | **0.8.9** | tags |  |
| [`libogg`](../ports/libogg/) | 1.3.5 | **1.3.6** | tags |  |
| [`libpcap`](../ports/libpcap/) | 1.10.5 | **1.11.0** | listing |  |
| [`libpng`](../ports/libpng/) | 1.6.44 | **1.6.59** | tags |  |
| [`libportal`](../ports/libportal/) | 0.8.1 | **0.11.0** | tags |  |
| [`libproxy`](../ports/libproxy/) | 0.5.9 | **0.5.12** | tags |  |
| [`libpsl`](../ports/libpsl/) | 0.21.5 | **0.23.3** | tags |  |
| [`libretro-2048`](../ports/libretro-2048/) | 0-git-20241227 | **39333f7b13dc** | branch | pinned commit 86e02d3c2dd7 |
| [`libretro-fake08`](../ports/libretro-fake08/) | 0-git-20260613 | **0.0.2.20** | tags |  |
| [`libretro-mrboom`](../ports/libretro-mrboom/) | 0-git-20260625 | **5.3** | tags |  |
| [`libretro-retro8`](../ports/libretro-retro8/) | 0-git-20260912 | **0.1b** | tags |  |
| [`libretro-samples`](../ports/libretro-samples/) | 0-git-20221110 | **7c2ad112d7a5** | branch | pinned commit bce193bc1b8c |
| [`libsecret`](../ports/libsecret/) | 0.21.4 | **0.21.8.2** | tags |  |
| [`libsodium`](../ports/libsodium/) | 1.0.20 | **1.0.22** | listing |  |
| [`libsoup`](../ports/libsoup/) | 3.6.1 | **3.8.1** | tags |  |
| [`libssh`](../ports/libssh/) | 0.11.1 | **0.11.5** | listing |  |
| [`libtasn1`](../ports/libtasn1/) | 4.19.0 | **4.21.0** | listing |  |
| [`libthai`](../ports/libthai/) | 0.1.29 | **0.1.30** | tags |  |
| [`libtheora`](../ports/libtheora/) | 1.1.1 | **1.2.0** | listing |  |
| [`libtiff`](../ports/libtiff/) | 4.7.0 | **4.7.2** | tags |  |
| [`libtool`](../ports/libtool/) | 2.5.4 | **2.6.2** | listing |  |
| [`libunibreak`](../ports/libunibreak/) | 6.1 | **8.0** | tags |  |
| [`libunistring`](../ports/libunistring/) | 1.3 | **1.5** | listing |  |
| [`libusb`](../ports/libusb/) | 1.0.27 | **1.0.30** | tags |  |
| [`libuv`](../ports/libuv/) | 1.51.0 | **1.53.0** | tags |  |
| [`libvpx`](../ports/libvpx/) | 1.15.0 | **1.17.0** | tags |  |
| [`libwebp`](../ports/libwebp/) | 1.4.0 | **1.6.0** | tags |  |
| [`libwebsockets`](../ports/libwebsockets/) | 4.3.5 | **5.0.0** | tags |  |
| [`libx11`](../ports/libx11/) | 1.8.10 | **1.8.14** | tags |  |
| [`libxau`](../ports/libxau/) | 1.0.11 | **1.0.12** | tags |  |
| [`libxcomposite`](../ports/libxcomposite/) | 0.4.6 | **0-6-1** | tags |  |
| [`libxcrypt`](../ports/libxcrypt/) | 4.4.38 | **4.5.2** | tags |  |
| [`libxcursor`](../ports/libxcursor/) | 1.2.2 | **1.2.3** | tags |  |
| [`libxdamage`](../ports/libxdamage/) | 1.1.6 | **1.1.7** | tags |  |
| [`libxext`](../ports/libxext/) | 1.3.6 | **1.3.7** | tags |  |
| [`libxfce4ui`](../ports/libxfce4ui/) | 4.20.0 | **4.20.2** | listing |  |
| [`libxfce4util`](../ports/libxfce4util/) | 4.20.0 | **4.20.1** | listing |  |
| [`libxfce4windowing`](../ports/libxfce4windowing/) | 4.20.0 | **4.20.7** | listing |  |
| [`libxfixes`](../ports/libxfixes/) | 6.0.1 | **6.0.2** | tags |  |
| [`libxft`](../ports/libxft/) | 2.3.8 | **2.3.9** | tags |  |
| [`libxi`](../ports/libxi/) | 1.8.2 | **1.8.4** | tags |  |
| [`libxinerama`](../ports/libxinerama/) | 1.1.5 | **1.1.6** | tags |  |
| [`libxml2`](../ports/libxml2/) | 2.13.5 | **6.0** | tags |  |
| [`libxrandr`](../ports/libxrandr/) | 1.5.4 | **1.5.5** | tags |  |
| [`libxrender`](../ports/libxrender/) | 0.9.11 | **0.9.13** | tags |  |
| [`libxscrnsaver`](../ports/libxscrnsaver/) | 1.2.4 | **1.2.5** | tags |  |
| [`libxslt`](../ports/libxslt/) | 1.1.42 | **1.1.45** | tags |  |
| [`libxtst`](../ports/libxtst/) | 1.2.5 | **1.2.6** | tags |  |
| [`libzip`](../ports/libzip/) | 1.11.1 | **1.12.1** | tags |  |
| [`lighttpd`](../ports/lighttpd/) | 1.4.77 | **1.4.85** | tags |  |
| [`lua`](../ports/lua/) | 5.4.7 | **5.5.1** | listing |  |
| [`lua5.1`](../ports/lua5.1/) | 5.1.5 | **5.5.1** | listing |  |
| [`lua5.3`](../ports/lua5.3/) | 5.3.6 | **5.5.1** | listing |  |
| [`luv`](../ports/luv/) | 1.51.0-1 | **1.53.0-0** | tags |  |
| [`m4`](../ports/m4/) | 1.4.19 | **1.4.21** | listing |  |
| [`magic_enum`](../ports/magic_enum/) | 0.9.6 | **0.9.8** | tags |  |
| [`mbedtls`](../ports/mbedtls/) | 3.6.2 | **4.2.0** | tags |  |
| [`minicom`](../ports/minicom/) | 2.9 | **2.11.1** | tags |  |
| [`mkfontscale`](../ports/mkfontscale/) | 1.2.3 | **1.2.4** | listing |  |
| [`mosquitto`](../ports/mosquitto/) | 2.0.20 | **2.1.2** | tags |  |
| [`mpc`](../ports/mpc/) | 1.3.1 | **1.4.1** | listing |  |
| [`mpfr`](../ports/mpfr/) | 4.2.1 | **4.2.2** | listing |  |
| [`mpg123`](../ports/mpg123/) | 1.32.9 | **1.33.7** | listing |  |
| [`nano`](../ports/nano/) | 8.2 | **8.7.1** | listing |  |
| [`ncurses`](../ports/ncurses/) | 6.5 | **6.6** | listing |  |
| [`neovim`](../ports/neovim/) | 0.12.3 | **0.12.6** | tags |  |
| [`nettle`](../ports/nettle/) | 3.10.1 | **4.0** | listing |  |
| [`nghttp2`](../ports/nghttp2/) | 1.64.0 | **1.70.0** | tags |  |
| [`ninja`](../ports/ninja/) | 1.13.1 | **120715** | tags |  |
| [`nng`](../ports/nng/) | 1.9.0 | **2.1.0** | tags |  |
| [`nodejs`](../ports/nodejs/) | 24.14.1 | **26.13.0** | tags |  |
| [`ntpsec`](../ports/ntpsec/) | 1.2.4 | **1.2.6** | tags |  |
| [`numpy`](../ports/numpy/) | 2.3.4 | **2.5.4** | tags |  |
| [`octomap`](../ports/octomap/) | 1.10.0 | **1.10.1** | tags |  |
| [`onetbb`](../ports/onetbb/) | 2022.0.0 | **2023.2.0** | tags |  |
| [`oniguruma`](../ports/oniguruma/) | 6.9.9 | **6.9.10** | tags |  |
| [`onnx`](../ports/onnx/) | 1.17.0 | **1.23.2** | tags |  |
| [`onnxruntime`](../ports/onnxruntime/) | 1.20.1 | **1.31.1** | tags |  |
| [`open-amp`](../ports/open-amp/) | 2024.10.0 | **2026.04.0** | tags |  |
| [`open62541`](../ports/open62541/) | 1.4.8 | **1.5.9** | tags |  |
| [`openal-soft`](../ports/openal-soft/) | 1.24.1 | **1.25.2** | tags |  |
| [`openblas`](../ports/openblas/) | 0.3.28 | **0.3.34** | tags |  |
| [`opencv`](../ports/opencv/) | 4.10.0 | **5.0.0** | tags |  |
| [`openh264`](../ports/openh264/) | 2.5.0 | **2.7.0** | tags |  |
| [`openjpeg`](../ports/openjpeg/) | 2.5.3 | **2.5.4** | tags |  |
| [`openssh`](../ports/openssh/) | 10.2p1 | **10.6p1** | listing |  |
| [`openssl`](../ports/openssl/) | 3.5.1 | **4.1.0** | tags |  |
| [`openttd`](../ports/openttd/) | 15.0-git-20250501 | **15.3** | tags |  |
| [`opus`](../ports/opus/) | 1.5.2 | **1.6.1** | tags |  |
| [`orc`](../ports/orc/) | 0.4.40 | **0.4.44** | tags |  |
| [`orocos-kdl`](../ports/orocos-kdl/) | 1.5.1 | **1.5.4** | tags |  |
| [`p11-kit`](../ports/p11-kit/) | 0.25.5 | **0.26.5** | tags |  |
| [`paho.mqtt.c`](../ports/paho.mqtt.c/) | 1.3.14 | **1.3.16** | tags |  |
| [`paho.mqtt.cpp`](../ports/paho.mqtt.cpp/) | 1.5.2 | **1.6.0** | tags |  |
| [`pandas`](../ports/pandas/) | 2.3.3 | **3.1.0** | tags |  |
| [`pango`](../ports/pango/) | 1.54.0 | **1.90** | tags |  |
| [`pax-utils`](../ports/pax-utils/) | 1.3.8 | **1.3.9** | listing |  |
| [`pcl`](../ports/pcl/) | 1.14.1 | **1.15.1** | tags |  |
| [`pcre2`](../ports/pcre2/) | 10.44 | **10.49** | tags |  |
| [`pdfium`](../ports/pdfium/) | 0-git-84a8011 | **f96a940dc045** | branch | pinned commit 84a8011ec69d |
| [`perl`](../ports/perl/) | 5.40.2 | **5.45.4** | listing | custom fetch: check the recipe |
| [`perl-error`](../ports/perl-error/) | 0.17030 | **0.17031** | listing |  |
| [`pixman`](../ports/pixman/) | 0.44.2 | **0.46.4** | tags |  |
| [`pkgconf`](../ports/pkgconf/) | 2.3.0 | **3.0.10** | listing |  |
| [`pqiv`](../ports/pqiv/) | 2.13.2 | **2.13.3** | tags |  |
| [`protobuf`](../ports/protobuf/) | 29.3 | **36.2** | tags |  |
| [`protobuf-c`](../ports/protobuf-c/) | 1.5.0 | **1.5.2** | tags |  |
| [`pthreadpool`](../ports/pthreadpool/) | 0-git-20250207 | **15a6644ba1c4** | branch | pinned commit f94ab76fe997 |
| [`pugixml`](../ports/pugixml/) | 1.14 | **1.16** | tags |  |
| [`pytorch`](../ports/pytorch/) | 2.3.1 | **2.15.0** | tags | custom fetch: check the recipe |
| [`quickjs-ng`](../ports/quickjs-ng/) | 0.8.0 | **0.17.0** | tags |  |
| [`re2`](../ports/re2/) | 2024-07-02 | **2025-11-05** | tags |  |
| [`readline`](../ports/readline/) | 8.2 | **8.3** | listing |  |
| [`restbed`](../ports/restbed/) | 4.8 | **5.0.0** | tags |  |
| [`rsync`](../ports/rsync/) | 3.3.0 | **3.5.1** | listing |  |
| [`rtl-sdr`](../ports/rtl-sdr/) | 2.0.2 | **2.0.3** | tags |  |
| [`ruy`](../ports/ruy/) | 0-git-20240321 | **226475377719** | branch | pinned commit c08ec529fc91 |
| [`s2n-tls`](../ports/s2n-tls/) | 1.7.0 | **1.7.13** | tags |  |
| [`sdl2`](../ports/sdl2/) | 2.32.10 | **3.4.18** | tags |  |
| [`sdl2-compat`](../ports/sdl2-compat/) | 2.32.60 | **2.32.74** | tags |  |
| [`sdl2-image`](../ports/sdl2-image/) | 2.8.8 | **3.4.8** | tags |  |
| [`sdl2-net`](../ports/sdl2-net/) | 2.2.0 | **3.2.0** | tags |  |
| [`sdl2-ttf`](../ports/sdl2-ttf/) | 2.24.0 | **3.2.2** | tags |  |
| [`simdjson`](../ports/simdjson/) | 3.11.3 | **5.0.8** | tags |  |
| [`simdutf`](../ports/simdutf/) | 6.4.2 | **9.3.3** | tags |  |
| [`sleef`](../ports/sleef/) | 3.8-git-20250214 | **3.9.0** | tags |  |
| [`snappy`](../ports/snappy/) | 1.2.1 | **1.3.1** | tags |  |
| [`spdlog`](../ports/spdlog/) | 1.15.3 | **1.17.0** | tags |  |
| [`sqlite_orm`](../ports/sqlite_orm/) | 1.9 | **1.9.1** | tags |  |
| [`strongswan`](../ports/strongswan/) | 6.0.6 | **6.1.0** | tags |  |
| [`suitesparse`](../ports/suitesparse/) | 7.11.0 | **7.14.1** | tags |  |
| [`svt-av1`](../ports/svt-av1/) | 2.3.0 | **4.2.0** | tags |  |
| [`taglib`](../ports/taglib/) | 2.0.2 | **2.3.2** | tags |  |
| [`tensorflow-lite`](../ports/tensorflow-lite/) | 2.16.1 | **2.21.0** | tags |  |
| [`tftp-hpa`](../ports/tftp-hpa/) | 5.2 | **7.2** | tags |  |
| [`thorvg`](../ports/thorvg/) | 0.15.8 | **1.1.3** | tags |  |
| [`thunar`](../ports/thunar/) | 4.20.0 | **4.20.10** | listing |  |
| [`tinyxml2`](../ports/tinyxml2/) | 10.0.0 | **11.0.0** | tags |  |
| [`tmux`](../ports/tmux/) | 3.5a | **3.8** | tags |  |
| [`tree`](../ports/tree/) | 2.2.1 | **2.3.2** | tags |  |
| [`tree-sitter`](../ports/tree-sitter/) | 0.25.9 | **0.27.1** | tags |  |
| [`tree-sitter-c`](../ports/tree-sitter-c/) | 0.24.1 | **0.24.2** | tags |  |
| [`tree-sitter-lua`](../ports/tree-sitter-lua/) | 0.4.0 | **0.5.0** | tags |  |
| [`tree-sitter-markdown`](../ports/tree-sitter-markdown/) | 0.5.1 | **0.5.4** | tags |  |
| [`tree-sitter-query`](../ports/tree-sitter-query/) | 0.7.0 | **0.8.0** | tags |  |
| [`tree-sitter-vim`](../ports/tree-sitter-vim/) | 0.7.0 | **0.8.1** | tags |  |
| [`tree-sitter-vimdoc`](../ports/tree-sitter-vimdoc/) | 4.0.0 | **4.3.0** | tags |  |
| [`trompeloeil`](../ports/trompeloeil/) | 48 | **49** | tags |  |
| [`tumbler`](../ports/tumbler/) | 4.20.0 | **4.20.2** | listing |  |
| [`unbound`](../ports/unbound/) | 1.24.1 | **1.26.2** | tags |  |
| [`unibilium`](../ports/unibilium/) | 2.1.2 | **2.1.4** | tags |  |
| [`urdfdom_headers`](../ports/urdfdom_headers/) | 1.1.2 | **3.0.1** | tags |  |
| [`uriparser`](../ports/uriparser/) | 0.9.8 | **1.0.2** | tags |  |
| [`utf8proc`](../ports/utf8proc/) | 2.11.3 | **2.12.0** | tags |  |
| [`vidstab`](../ports/vidstab/) | 1.1.1 | **1.1.2** | tags |  |
| [`vim`](../ports/vim/) | 9.1.0900 | **9.2.1252** | tags |  |
| [`vlc`](../ports/vlc/) | 3.0.21 | **3.0.24** | tags |  |
| [`vsomeip`](../ports/vsomeip/) | 3.5.6 | **3.7.6** | tags |  |
| [`vte`](../ports/vte/) | 0.78.2 | **0.78.7** | listing |  |
| [`wayland`](../ports/wayland/) | 1.23.1 | **1.26.0** | tags |  |
| [`wayland-protocols`](../ports/wayland-protocols/) | 1.38 | **1.50** | tags |  |
| [`webkitgtk`](../ports/webkitgtk/) | 2.44.4 | **2.54.2** | listing |  |
| [`webrtc`](../ports/webrtc/) | m132-afaf497 | **dd414f9a218b** | branch | pinned commit afaf497805cb |
| [`wlr-protocols`](../ports/wlr-protocols/) | 0-git-20250702 | **1106a4a1c575** | branch | pinned commit a5028afbe4a1 |
| [`x265`](../ports/x265/) | 3.6 | **4.2** | tags |  |
| [`xfce4-panel`](../ports/xfce4-panel/) | 4.20.0 | **4.20.8** | listing |  |
| [`xfce4-session`](../ports/xfce4-session/) | 4.20.2 | **4.20.4** | listing |  |
| [`xfce4-settings`](../ports/xfce4-settings/) | 4.20.1 | **4.20.5** | listing |  |
| [`xkeyboard-config`](../ports/xkeyboard-config/) | 2.43 | **2.49** | tags |  |
| [`xnnpack`](../ports/xnnpack/) | 0-git-20250209 | **1025db72bf1a** | branch | pinned commit e55b3998cadb |
| [`xorgproto`](../ports/xorgproto/) | 2024.1 | **2026.1** | tags |  |
| [`xtrans`](../ports/xtrans/) | 1.5.2 | **1.6.0** | tags |  |
| [`xxhash`](../ports/xxhash/) | 0.8.2 | **0.8.4** | tags |  |
| [`xz`](../ports/xz/) | 5.6.3 | **5.8.4** | tags |  |
| [`yaml-cpp`](../ports/yaml-cpp/) | 0.8.0 | **0.9.0** | tags |  |
| [`zenoh-c`](../ports/zenoh-c/) | 1.2.1 | **1.10.2** | tags |  |
| [`zimg`](../ports/zimg/) | 3.0.5 | **3.0.6** | tags |  |
| [`zlib`](../ports/zlib/) | 1.3.1 | **1.3.2** | tags |  |
| [`zlib-ng`](../ports/zlib-ng/) | 2.2.5 | **2.3.3** | tags |  |
| [`zstd`](../ports/zstd/) | 1.5.6 | **1.5.7** | tags |  |

## Could not tell

| Component | Ported | Status | Note |
|---|---|---|---|
| `boost` | 1.86.0 | unknown | versioned download directory: check https://archives.boost.io/release/1.86.0 |
| `ca-certificates` | 20261020 | unknown | custom fetch: check the recipe |
| `cunit` | 2.1-3 | error | HTTP Error 404: Not Found |
| `duktape` | 2.7.0 | unknown |  |
| `font-dejavu` | 2.37 | unknown | custom fetch: check the recipe |
| `font-liberation` | 2.1.5 | error | HTTP Error 404: Not Found |
| `freeimage` | 3.18.0 | unknown | custom fetch: check the recipe |
| `giflib` | 5.2.2 | error | HTTP Error 404: Not Found |
| `gmp` | 6.3.0 | error | <urlopen error [Errno 111] Connection refused> |
| `godot` | 4.4.1 | unknown |  |
| `hyphen` | 2.8.8 | error | HTTP Error 404: Not Found |
| `iso-codes` | 4.17.0 | unknown |  |
| `itstool` | 2.0.7 | error | HTTP Error 404: Not Found |
| `lame` | 3.100 | error | HTTP Error 404: Not Found |
| `less` | 668 | unknown |  |
| `libedit` | 20240808-3.1 | unknown |  |
| `libinih` | 58 | unknown |  |
| `libxkbcommon` | 1.7.0 | unknown |  |
| `libyuv` | stable | unknown |  |
| `llvm` | 22.1.8 | unknown |  |
| `luajit` | 2.1.c6ffc141 | unknown |  |
| `mame` | 0.281 | unknown |  |
| `meson` | 1.10.0 | error | HTTP Error 404: Not Found |
| `npm` | 11.11.0 | error | HTTP Error 404: Not Found |
| `pcre` | 8.45 | error | HTTP Error 404: Not Found |
| `python3` | 3.14.0 | unknown | versioned download directory: check https://www.python.org/ftp/python |
| `python3-dateutil` | 2.9.0.post0 | error | HTTP Error 404: Not Found |
| `python3-packaging` | 25.0 | error | HTTP Error 404: Not Found |
| `python3-pip` | 25.1.1 | error | HTTP Error 404: Not Found |
| `python3-pytz` | 2025.2 | error | HTTP Error 404: Not Found |
| `python3-setuptools` | 80.8.0 | error | HTTP Error 404: Not Found |
| `python3-six` | 1.17.0 | error | HTTP Error 404: Not Found |
| `python3-tzdata` | 2025.2 | error | HTTP Error 404: Not Found |
| `python3-wheel` | 0.46.1 | error | HTTP Error 404: Not Found |
| `qt6` | 6.8.1 | unknown | versioned download directory: check https://download.qt.io/archive/qt/6.8/6.8.1 |
| `rust` | 1.90.0 | error | HTTP Error 416: Requested Range Not Satisfiable |
| `shared-mime-info` | 2.4 | unknown |  |
| `sqlite` | 3.47.2 | error | HTTP Error 404: Not Found |
| `x264` | stable | unknown |  |
| `xdg-utils` | 1.2.1 | unknown |  |
| `zsh` | 5.9 | error | HTTP Error 404: Not Found |
