# Component index

525 components. **524** build for both `aarch64le` and `x86_64` from upstream source, carrying 251 QNX patches between them.

| Class | | Components |
|---|---|---|
| 🟢 **standalone** | builds with the QNX SDP only, runs on a stock target | **223** |
| 🟡 **build-deps only** | needs other components to build, runs on its own | **63** |
| 🔴 **dependent** | needs other components to build and to run | **238** |
| 📋 **planned** | recorded for porting, not ported yet | **0** |
| ⛔ **blocked** | tried; cannot be built for QNX as things stand (the recipe says why) | **1** |

How the class is decided, and every component's full chain: [DEPENDENCIES.md](DEPENDENCIES.md).

Click a component for its source URL, its patches, the exact build commands and how it is verified on a target.

| Component | Version | Build | Upstream | Patches | Class | Run-time deps | aarch64le | x86_64 |
|---|---|---|---|---|---|---|---|---|
| [abseil](ports/abseil/README.md) | 20240722.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [abuild](ports/abuild/README.md) | 3.16.0 | custom | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [ada](ports/ada/README.md) | 3.4.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-fonts](ports/adwaita-fonts/README.md) | 49.0 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-icon-theme](ports/adwaita-icon-theme/README.md) | 51.0 | meson | download.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [amqp-cpp](ports/amqp-cpp/README.md) | 4.3.27 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [aom](ports/aom/README.md) | 3.12.1 | cmake | aomedia.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [apk-tools](ports/apk-tools/README.md) | 3.0.0_rc5 | meson | github.com | 3 | 🔴 dependent | 3 | ✅ | ✅ |
| [asio](ports/asio/README.md) | 1.30.2 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [aspell](ports/aspell/README.md) | 0.60.8.1 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [at-spi2-core](ports/at-spi2-core/README.md) | 2.54.0 | meson | download.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [autoconf](ports/autoconf/README.md) | 2.72 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [automake](ports/automake/README.md) | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [aws-c-auth](ports/aws-c-auth/README.md) | 0.10.1 | cmake | github.com | — | 🔴 dependent | 8 | ✅ | ✅ |
| [aws-c-cal](ports/aws-c-cal/README.md) | 0.9.13 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [aws-c-common](ports/aws-c-common/README.md) | 0.12.6 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [aws-c-compression](ports/aws-c-compression/README.md) | 0.3.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-c-event-stream](ports/aws-c-event-stream/README.md) | 0.7.0 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [aws-c-http](ports/aws-c-http/README.md) | 0.10.14 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [aws-c-io](ports/aws-c-io/README.md) | 0.26.1 | cmake | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [aws-c-mqtt](ports/aws-c-mqtt/README.md) | 0.15.2 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [aws-c-s3](ports/aws-c-s3/README.md) | 0.12.2 | cmake | github.com | — | 🔴 dependent | 10 | ✅ | ✅ |
| [aws-c-sdkutils](ports/aws-c-sdkutils/README.md) | 0.2.4 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-checksums](ports/aws-checksums/README.md) | 0.2.10 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-crt-cpp](ports/aws-crt-cpp/README.md) | 0.38.5 | cmake | github.com | 1 | 🔴 dependent | 13 | ✅ | ✅ |
| [azure-iot-sdk-c](ports/azure-iot-sdk-c/README.md) | LTS_08_2024 | cmake | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [bash](ports/bash/README.md) | 5.2.37 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [bc](ports/bc/README.md) | 1.08.2 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [benchmark](ports/benchmark/README.md) | 1.9.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [binutils](ports/binutils/README.md) | 2.43 | autotools | ftp.gnu.org | 10 | 🔴 dependent | 2 | ✅ | ✅ |
| [boost](ports/boost/README.md) | 1.86.0 | custom | archives.boost.io | 4 | 🔴 dependent | 3 | ✅ | ✅ |
| [brotli](ports/brotli/README.md) | 1.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [btop](ports/btop/README.md) | 1.4.6 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [busybox](ports/busybox/README.md) | 1.36.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [bzip2](ports/bzip2/README.md) | 1.0.8 | custom | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [c-ares](ports/c-ares/README.md) | 1.34.3 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ca-certificates](ports/ca-certificates/README.md) | 20261020 | custom | curl.se | — | 🟢 standalone | — | ✅ | ✅ |
| [cairo](ports/cairo/README.md) | 1.18.2 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [candb](ports/candb/README.md) | 0.1-git-20210625 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [capicxx-core-runtime](ports/capicxx-core-runtime/README.md) | 3.2.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [capicxx-someip-runtime](ports/capicxx-someip-runtime/README.md) | 3.2.4 | cmake | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [capnproto](ports/capnproto/README.md) | 1.0.2 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [capstone](ports/capstone/README.md) | 5.0.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [catch2](ports/catch2/README.md) | 3.7.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cereal](ports/cereal/README.md) | 1.3.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ceres-solver](ports/ceres-solver/README.md) | 2.2.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [check](ports/check/README.md) | 0.15.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [chrony](ports/chrony/README.md) | 4.8-git-20260226 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [civetweb](ports/civetweb/README.md) | 1.16 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [cjson](ports/cjson/README.md) | 1.7.18 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [clapack](ports/clapack/README.md) | 3.2.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [cli11](ports/cli11/README.md) | 2.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cmake](ports/cmake/README.md) | 4.1.1 | cmake | gitlab.kitware.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [computelibrary](ports/computelibrary/README.md) | 53.0.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | n/a |
| [console_bridge](ports/console_bridge/README.md) | 1.0.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cppzmq](ports/cppzmq/README.md) | 4.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [cpuinfo](ports/cpuinfo/README.md) | 0-git-20240417 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [csmith](ports/csmith/README.md) | 2.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cunit](ports/cunit/README.md) | 2.1-3 | custom | downloads.sourceforge.net | 1 | 🟢 standalone | — | ✅ | ✅ |
| [curl](ports/curl/README.md) | 8.11.1 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [cyclonedds](ports/cyclonedds/README.md) | 0.10.5 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [dav1d](ports/dav1d/README.md) | 1.5.0 | meson | code.videolan.org | — | 🟢 standalone | — | ✅ | ✅ |
| [dbus](ports/dbus/README.md) | 1.16.0 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [desktop-file-utils](ports/desktop-file-utils/README.md) | 0.28 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [dlt-daemon](ports/dlt-daemon/README.md) | 2.18.10 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [dnsmasq](ports/dnsmasq/README.md) | 2.93test4 | custom | thekelleys.org.uk | 1 | 🟢 standalone | — | ✅ | ✅ |
| [docbook-xsl](ports/docbook-xsl/README.md) | 1.79.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [double-conversion](ports/double-conversion/README.md) | 3.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [doxygen](ports/doxygen/README.md) | 1.17.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [duktape](ports/duktape/README.md) | 2.7.0 | custom | duktape.org | — | 🟢 standalone | — | ✅ | ✅ |
| [dump1090](ports/dump1090/README.md) | 0-git-20240121 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [ed](ports/ed/README.md) | 1.22.5 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [eigen](ports/eigen/README.md) | 3.4.0 | cmake | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [emacs](ports/emacs/README.md) | 30.2 | custom | ftp.gnu.org | 4 | 🔴 dependent | 9 | ✅ | ✅ |
| [emulationstation](ports/emulationstation/README.md) | 2.11-git-20241212 | cmake | github.com | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [enca](ports/enca/README.md) | 1.19 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [enchant](ports/enchant/README.md) | 2.8.2 | autotools | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [encodings](ports/encodings/README.md) | 1.1.0 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [epiphany](ports/epiphany/README.md) | 43.1 | meson | download.gnome.org | 5 | 🔴 dependent | 20 | ✅ | ✅ |
| [exiv2](ports/exiv2/README.md) | 0.28.5 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [exo](ports/exo/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [expat](ports/expat/README.md) | 2.6.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fakeroot](ports/fakeroot/README.md) | 1.37.1.1 | autotools | deb.debian.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [farmhash](ports/farmhash/README.md) | 1.1-git-20190513 | autotools | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [fastcdr](ports/fastcdr/README.md) | 2.2.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fastdds](ports/fastdds/README.md) | 2.14.4 | cmake | github.com | 2 | 🔴 dependent | 4 | ✅ | ✅ |
| [fcl](ports/fcl/README.md) | 0.7.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fdk-aac](ports/fdk-aac/README.md) | 2.0.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ffmpeg](ports/ffmpeg/README.md) | 7.1.1 | custom | github.com | — | 🔴 dependent | 12 | ✅ | ✅ |
| [ffmpegthumbnailer](ports/ffmpegthumbnailer/README.md) | 2.2.3 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [file](ports/file/README.md) | 5.46 | autotools | astron.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [flac](ports/flac/README.md) | 1.4.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flann](ports/flann/README.md) | 1.9.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flatbuffers](ports/flatbuffers/README.md) | 24.3.25 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fmt](ports/fmt/README.md) | 11.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-cantarell](ports/font-cantarell/README.md) | 0.303.1 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [font-dejavu](ports/font-dejavu/README.md) | 2.37 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-hack](ports/font-hack/README.md) | 3.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-inter](ports/font-inter/README.md) | 4.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-liberation](ports/font-liberation/README.md) | 2.1.5 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-emoji](ports/font-noto-emoji/README.md) | 2.048 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-sans-cjk](ports/font-noto-sans-cjk/README.md) | 2.004 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-serif-cjk](ports/font-noto-serif-cjk/README.md) | 2.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-util](ports/font-util/README.md) | 1.4.1 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [fontconfig](ports/fontconfig/README.md) | 2.15.0 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fontenc](ports/fontenc/README.md) | 1.1.8 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [foonathan_memory](ports/foonathan_memory/README.md) | 0.7-3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fp16](ports/fp16/README.md) | 0-git-20260620 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [freeimage](ports/freeimage/README.md) | 3.18.0 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [freetype](ports/freetype/README.md) | 2.13.3 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [fribidi](ports/fribidi/README.md) | 1.0.16 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [fxdiv](ports/fxdiv/README.md) | 0-git-20200417 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [g3log](ports/g3log/README.md) | 2.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [garcon](ports/garcon/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [gawk](ports/gawk/README.md) | 5.3.2 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [gcr](ports/gcr/README.md) | 3.41.2 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 8 | ✅ | ✅ |
| [gdb](ports/gdb/README.md) | 14.2 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 10 | ✅ | ✅ |
| [gdbm](ports/gdbm/README.md) | 1.24 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gdk-pixbuf](ports/gdk-pixbuf/README.md) | 2.42.12 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [geany-plugins](ports/geany-plugins/README.md) | 2.1 | autotools | plugins.geany.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [geany](ports/geany/README.md) | 2.1 | autotools | download.geany.org | 1 | 🔴 dependent | 9 | ✅ | ✅ |
| [gettext](ports/gettext/README.md) | 0.22.5 | autotools | ftp.gnu.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [gflags](ports/gflags/README.md) | 2.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [giflib](ports/giflib/README.md) | 5.2.2 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [git](ports/git/README.md) | 2.51.0 | custom | mirrors.edge.kernel.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [gklib](ports/gklib/README.md) | 5.1.1-git-20230326 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [glib-networking](ports/glib-networking/README.md) | 2.80.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [glib](ports/glib/README.md) | 2.82.4 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 5 | ✅ | ✅ |
| [glm](ports/glm/README.md) | 1.0.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [glog](ports/glog/README.md) | 0.7.1 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [gmp](ports/gmp/README.md) | 6.3.0 | autotools | gmplib.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gnutls](ports/gnutls/README.md) | 3.8.8 | autotools | www.gnupg.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [gobject-introspection](ports/gobject-introspection/README.md) | 1.86.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [godot](ports/godot/README.md) | 4.4.1 | custom | github.com | 1 | 🔴 dependent | 17 | ✅ | ✅ |
| [googletest](ports/googletest/README.md) | 1.17.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gperf](ports/gperf/README.md) | 3.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [graphene](ports/graphene/README.md) | 1.10.8 | meson | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [graphicsmagick](ports/graphicsmagick/README.md) | 1.3.46 | autotools | downloads.sourceforge.net | — | 🔴 dependent | 13 | ✅ | ✅ |
| [graphite2](ports/graphite2/README.md) | 1.3.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [grep](ports/grep/README.md) | 3.11 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [groff](ports/groff/README.md) | 1.23.0 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [grpc](ports/grpc/README.md) | 1.69.0 | custom | github.com | 2 | 🔴 dependent | 6 | ✅ | ✅ |
| [gsettings-desktop-schemas](ports/gsettings-desktop-schemas/README.md) | 49.0 | meson | gitlab.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [gsm](ports/gsm/README.md) | 1.0.22 | custom | www.quut.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gst-libav](ports/gst-libav/README.md) | 1.24.10 | meson | custom | — | 🔴 dependent | 4 | ✅ | ✅ |
| [gst-plugins-bad](ports/gst-plugins-bad/README.md) | 1.24.10 | meson | custom | — | 🔴 dependent | 11 | ✅ | ✅ |
| [gst-plugins-base](ports/gst-plugins-base/README.md) | 1.24.10 | meson | custom | — | 🔴 dependent | 8 | ✅ | ✅ |
| [gst-plugins-good](ports/gst-plugins-good/README.md) | 1.24.10 | meson | custom | — | 🔴 dependent | 14 | ✅ | ✅ |
| [gst-plugins-ugly](ports/gst-plugins-ugly/README.md) | 1.24.10 | meson | custom | — | 🔴 dependent | 5 | ✅ | ✅ |
| [gstreamer](ports/gstreamer/README.md) | 1.24.10 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [gthumb](ports/gthumb/README.md) | 3.12.8.2 | meson | download.gnome.org | 4 | 🔴 dependent | 13 | ✅ | ✅ |
| [gtk-layer-shell](ports/gtk-layer-shell/README.md) | 0.9.0 | meson | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [gtk-vnc](ports/gtk-vnc/README.md) | 1.3.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 9 | ✅ | ✅ |
| [gtk3](ports/gtk3/README.md) | 3.24.43 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 21 | ✅ | ✅ |
| [gtk4](ports/gtk4/README.md) | 4.16.7 | meson | download.gnome.org | 1 | 🔴 dependent | 15 | ✅ | ✅ |
| [gtsam](ports/gtsam/README.md) | 4.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [harfbuzz](ports/harfbuzz/README.md) | 10.1.0 | meson | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [hicolor-icon-theme](ports/hicolor-icon-theme/README.md) | 0.18 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [highway](ports/highway/README.md) | 1.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [htop](ports/htop/README.md) | 3.5.0 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [hyphen](ports/hyphen/README.md) | 2.8.8 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [iceoryx](ports/iceoryx/README.md) | 2.0.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [icu](ports/icu/README.md) | 76.1 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [iperf3](ports/iperf3/README.md) | 3.17.1 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [iso-codes](ports/iso-codes/README.md) | 4.17.0 | autotools | salsa.debian.org | — | 🟢 standalone | — | ✅ | ✅ |
| [itstool](ports/itstool/README.md) | 2.0.7 | autotools | files.itstool.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [jansson](ports/jansson/README.md) | 2.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [javascriptcore](ports/javascriptcore/README.md) | 2.44.4 | cmake | webkitgtk.org | 4 | 🔴 dependent | 1 | ✅ | ✅ |
| [jq](ports/jq/README.md) | 1.7.1 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [json-c](ports/json-c/README.md) | 0.18 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [json-glib](ports/json-glib/README.md) | 1.10.6 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [jsoncpp](ports/jsoncpp/README.md) | 1.9.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [kineto](ports/kineto/README.md) | 0-git-20250214 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [labwc](ports/labwc/README.md) | 0.20.2 | meson | github.com | 2 | 🔴 dependent | 10 | ✅ | ✅ |
| [lame](ports/lame/README.md) | 3.100 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [lcm](ports/lcm/README.md) | 1.5.2 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [lcms2](ports/lcms2/README.md) | 2.16 | meson | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lely-core](ports/lely-core/README.md) | 2.3.4 | autotools | gitlab.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [less](ports/less/README.md) | 668 | autotools | www.greenwoodsoftware.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [leveldb](ports/leveldb/README.md) | 1.23 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libaec](ports/libaec/README.md) | 1.1.7 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libarchive](ports/libarchive/README.md) | 3.7.7 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libass](ports/libass/README.md) | 0.17.3 | autotools | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [libavif](ports/libavif/README.md) | 1.1.1 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libcamera](ports/libcamera/README.md) | 0.5.1+rpt20250722 | custom | github.com | 1 | 🟡 build-deps only | — | ✅ | n/a |
| [libcbor](ports/libcbor/README.md) | 0.11.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libccd](ports/libccd/README.md) | 2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libcoap](ports/libcoap/README.md) | 4.3.5 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libconfig](ports/libconfig/README.md) | 1.8.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdatrie](ports/libdatrie/README.md) | 0.2.13 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdazzle](ports/libdazzle/README.md) | 3.44.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [libde265](ports/libde265/README.md) | 1.0.15 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdeflate](ports/libdeflate/README.md) | 1.22 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdisplay-info](ports/libdisplay-info/README.md) | 0.3.0 | meson | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdrm-headers](ports/libdrm-headers/README.md) | 2.4.129 | custom | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libedit](ports/libedit/README.md) | 20240808-3.1 | autotools | thrysoee.dk | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libepoxy](ports/libepoxy/README.md) | 1.5.10 | meson | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libev](ports/libev/README.md) | 4.33 | autotools | dist.schmorp.de | — | 🟢 standalone | — | ✅ | ✅ |
| [libevent](ports/libevent/README.md) | 2.1.12 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libexif](ports/libexif/README.md) | 0.6.24 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libfdt](ports/libfdt/README.md) | 1.7.2 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libffi](ports/libffi/README.md) | 3.4.6 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libgcrypt](ports/libgcrypt/README.md) | 1.11.0 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgit2](ports/libgit2/README.md) | 1.8.4 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [libgpg-error](ports/libgpg-error/README.md) | 1.50 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgsf](ports/libgsf/README.md) | 1.14.53 | autotools | gitlab.gnome.org | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libhandy](ports/libhandy/README.md) | 1.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [libice](ports/libice/README.md) | 1.1.2 | autotools | xorg.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libiconv](ports/libiconv/README.md) | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libidn2](ports/libidn2/README.md) | 2.3.7 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libinih](ports/libinih/README.md) | 58 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjpeg-turbo](ports/libjpeg-turbo/README.md) | 3.0.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjson](ports/libjson/README.md) | 0.8-git-20260512 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjxl](ports/libjxl/README.md) | 0.11.1 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libmd](ports/libmd/README.md) | 1.1.0 | autotools | libbsd.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmetal](ports/libmetal/README.md) | 2024.10.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libmicrohttpd](ports/libmicrohttpd/README.md) | 1.0.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmodbus](ports/libmodbus/README.md) | 3.1.11 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libnotify](ports/libnotify/README.md) | 0.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libogg](ports/libogg/README.md) | 1.3.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libpcap](ports/libpcap/README.md) | 1.10.5 | autotools | www.tcpdump.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpipeline](ports/libpipeline/README.md) | 1.5.8 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpng](ports/libpng/README.md) | 1.6.44 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libportal](ports/libportal/README.md) | 0.8.1 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libproxy](ports/libproxy/README.md) | 0.5.9 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libpsl](ports/libpsl/README.md) | 0.21.5 | meson | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libretro-2048](ports/libretro-2048/README.md) | 0-git-20241227 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-fake08](ports/libretro-fake08/README.md) | 0-git-20260613 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-mrboom](ports/libretro-mrboom/README.md) | 0-git-20260625 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libretro-retro8](ports/libretro-retro8/README.md) | 0-git-20260912 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-samples](ports/libretro-samples/README.md) | 0-git-20221110 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsamplerate](ports/libsamplerate/README.md) | 0.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsecret](ports/libsecret/README.md) | 0.21.4 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libsfdo](ports/libsfdo/README.md) | 0.1.4 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsm](ports/libsm/README.md) | 1.2.6 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libsndfile](ports/libsndfile/README.md) | 1.2.2 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [libsodium](ports/libsodium/README.md) | 1.0.20 | autotools | download.libsodium.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsoup](ports/libsoup/README.md) | 3.6.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libssh](ports/libssh/README.md) | 0.11.1 | cmake | www.libssh.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libssh2](ports/libssh2/README.md) | 1.11.1 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libtasn1](ports/libtasn1/README.md) | 4.19.0 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libthai](ports/libthai/README.md) | 0.1.29 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libtheora](ports/libtheora/README.md) | 1.1.1 | autotools | downloads.xiph.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libtiff](ports/libtiff/README.md) | 4.7.0 | cmake | gitlab.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libtool](ports/libtool/README.md) | 2.5.4 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunibreak](ports/libunibreak/README.md) | 6.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libunistring](ports/libunistring/README.md) | 1.3 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunwind](ports/libunwind/README.md) | 1.8.3 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libusb](ports/libusb/README.md) | 1.0.27 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libuv](ports/libuv/README.md) | 1.51.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libvncserver](ports/libvncserver/README.md) | 0.9.15 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libvorbis](ports/libvorbis/README.md) | 1.3.7 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libvpx](ports/libvpx/README.md) | 1.15.0 | custom | chromium.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libwebp](ports/libwebp/README.md) | 1.4.0 | cmake | chromium.googlesource.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libwebsockets](ports/libwebsockets/README.md) | 4.3.5 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libx11](ports/libx11/README.md) | 1.8.10 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxau](ports/libxau/README.md) | 1.0.11 | autotools | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxcb](ports/libxcb/README.md) | 1.17.0 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxcomposite](ports/libxcomposite/README.md) | 0.4.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxcrypt](ports/libxcrypt/README.md) | 4.4.38 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libxcursor](ports/libxcursor/README.md) | 1.2.2 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxcvt](ports/libxcvt/README.md) | 0.1.3 | meson | xorg.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libxdamage](ports/libxdamage/README.md) | 1.1.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxdmcp](ports/libxdmcp/README.md) | 1.1.5 | autotools | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxext](ports/libxext/README.md) | 1.3.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxfce4ui](ports/libxfce4ui/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [libxfce4util](ports/libxfce4util/README.md) | 4.20.0 | autotools | archive.xfce.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libxfce4windowing](ports/libxfce4windowing/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 10 | ✅ | ✅ |
| [libxfixes](ports/libxfixes/README.md) | 6.0.1 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxfont2](ports/libxfont2/README.md) | 2.0.7 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxft](ports/libxft/README.md) | 2.3.8 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [libxi](ports/libxi/README.md) | 1.8.2 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxinerama](ports/libxinerama/README.md) | 1.1.5 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxkbcommon](ports/libxkbcommon/README.md) | 1.8.1 | meson | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libxkbfile](ports/libxkbfile/README.md) | 1.2.0 | meson | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxml2](ports/libxml2/README.md) | 2.13.5 | cmake | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxmu](ports/libxmu/README.md) | 1.3.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxrandr](ports/libxrandr/README.md) | 1.5.4 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxrender](ports/libxrender/README.md) | 0.9.11 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxscrnsaver](ports/libxscrnsaver/README.md) | 1.2.4 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxshmfence](ports/libxshmfence/README.md) | 1.3.3 | autotools | xorg.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxslt](ports/libxslt/README.md) | 1.1.42 | cmake | gitlab.gnome.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxt](ports/libxt/README.md) | 1.3.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxtst](ports/libxtst/README.md) | 1.2.5 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libyaml](ports/libyaml/README.md) | 0.2.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libyuv](ports/libyuv/README.md) | stable | cmake | chromium.googlesource.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libzip](ports/libzip/README.md) | 1.11.1 | cmake | github.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libzmq](ports/libzmq/README.md) | 4.3.5 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [lighttpd](ports/lighttpd/README.md) | 1.4.77 | cmake | git.lighttpd.net | — | 🔴 dependent | 3 | ✅ | ✅ |
| [linux-headers-input-codes](ports/linux-headers-input-codes/README.md) | 6.12 | custom | git.kernel.org | — | 🟢 standalone | — | ✅ | ✅ |
| [llvm](ports/llvm/README.md) | 22.1.8 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [lmbench](ports/lmbench/README.md) | 3.0-git-20260929 | custom | github.com | 5 | 🟢 standalone | — | ✅ | ✅ |
| [lua-lpeg](ports/lua-lpeg/README.md) | 1.1.0 | custom | www.inf.puc-rio.br | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lua](ports/lua/README.md) | 5.4.7 | custom | www.lua.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lua5.1](ports/lua5.1/README.md) | 5.1.5 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lua5.3](ports/lua5.3/README.md) | 5.3.6 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [luajit](ports/luajit/README.md) | 2.1.c6ffc141 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [luv](ports/luv/README.md) | 1.51.0-1 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [lxde-icon-theme](ports/lxde-icon-theme/README.md) | 0.5.2 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lz4](ports/lz4/README.md) | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [lzip](ports/lzip/README.md) | 1.26 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lzo](ports/lzo/README.md) | 2.10 | autotools | www.oberhumer.com | — | 🟢 standalone | — | ✅ | ✅ |
| [m4](ports/m4/README.md) | 1.4.19 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [magic_enum](ports/magic_enum/README.md) | 0.9.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [make](ports/make/README.md) | 4.4.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [mame](ports/mame/README.md) | 0.281 | custom | github.com | 1 | 🔴 dependent | 5 | ✅ | ✅ |
| [man-db](ports/man-db/README.md) | 2.13.1 | autotools | download.savannah.gnu.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [mate-calc](ports/mate-calc/README.md) | 1.28.0 | autotools | pub.mate-desktop.org | 3 | 🔴 dependent | 13 | ✅ | ✅ |
| [mbedtls](ports/mbedtls/README.md) | 3.6.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mcap-cpp](ports/mcap-cpp/README.md) | 2.1.3 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [meson](ports/meson/README.md) | 1.10.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [metis](ports/metis/README.md) | 5.2.1 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [mimalloc](ports/mimalloc/README.md) | 3.1.5 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [minicom](ports/minicom/README.md) | 2.9 | autotools | salsa.debian.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [miniz](ports/miniz/README.md) | 3.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mkfontscale](ports/mkfontscale/README.md) | 1.2.3 | autotools | www.x.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [mosquitto](ports/mosquitto/README.md) | 2.0.20 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpc](ports/mpc/README.md) | 1.3.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpfr](ports/mpfr/README.md) | 4.2.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [mpg123](ports/mpg123/README.md) | 1.32.9 | autotools | www.mpg123.de | — | 🟢 standalone | — | ✅ | ✅ |
| [msgpack-c](ports/msgpack-c/README.md) | 6.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mujoco](ports/mujoco/README.md) | 3.10.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [muslflt](ports/muslflt/README.md) | 1.0-git-20240802 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nano](ports/nano/README.md) | 8.2 | autotools | www.nano-editor.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [nanosvg](ports/nanosvg/README.md) | 0-git-20260709 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ncnn](ports/ncnn/README.md) | 20260526 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ncurses](ports/ncurses/README.md) | 6.5 | autotools | ftp.gnu.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [neovim](ports/neovim/README.md) | 0.12.3 | custom | github.com | 4 | 🔴 dependent | 6 | ✅ | ✅ |
| [nettle](ports/nettle/README.md) | 3.10.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [nghttp2](ports/nghttp2/README.md) | 1.64.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [ninja](ports/ninja/README.md) | 1.13.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [nlohmann_json](ports/nlohmann_json/README.md) | 3.12.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nng](ports/nng/README.md) | 1.9.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [nodejs](ports/nodejs/README.md) | 24.14.1 | custom | github.com | 9 | 🔴 dependent | 12 | ✅ | ✅ |
| [npm](ports/npm/README.md) | 11.11.0 | custom | registry.npmjs.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [ntpsec](ports/ntpsec/README.md) | 1.2.4 | custom | gitlab.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [numpy](ports/numpy/README.md) | 2.3.4 | custom | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [octomap](ports/octomap/README.md) | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [onetbb](ports/onetbb/README.md) | 2022.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [oniguruma](ports/oniguruma/README.md) | 6.9.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [onnx](ports/onnx/README.md) | 1.17.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [onnxruntime](ports/onnxruntime/README.md) | 1.20.1 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [open-amp](ports/open-amp/README.md) | 2024.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [open62541](ports/open62541/README.md) | 1.4.8 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openal-soft](ports/openal-soft/README.md) | 1.24.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [openblas](ports/openblas/README.md) | 0.3.28 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [opencv](ports/opencv/README.md) | 4.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openh264](ports/openh264/README.md) | 2.5.0 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [openjpeg](ports/openjpeg/README.md) | 2.5.3 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [openssh](ports/openssh/README.md) | 10.2p1 | autotools | cdn.openbsd.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [openssl](ports/openssl/README.md) | 3.5.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-opengfx](ports/openttd-opengfx/README.md) | 8.0 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-openmsx](ports/openttd-openmsx/README.md) | 0.4.2 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-opensfx](ports/openttd-opensfx/README.md) | 1.0.3 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd](ports/openttd/README.md) | 15.0-git-20250501 | custom | github.com | 2 | 🔴 dependent | 7 | ✅ | ✅ |
| [opus](ports/opus/README.md) | 1.5.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [opusfile](ports/opusfile/README.md) | 0.12 | autotools | downloads.xiph.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [orc](ports/orc/README.md) | 0.4.40 | meson | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [orocos-kdl](ports/orocos-kdl/README.md) | 1.5.1 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [oss-desktop](ports/oss-desktop/README.md) | 1.0 | custom | custom | — | 🟡 build-deps only | — | ✅ | ✅ |
| [p11-kit](ports/p11-kit/README.md) | 0.25.5 | meson | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [paho.mqtt.c](ports/paho.mqtt.c/README.md) | 1.3.14 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [paho.mqtt.cpp](ports/paho.mqtt.cpp/README.md) | 1.5.2 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [pandas](ports/pandas/README.md) | 2.3.3 | custom | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [pango](ports/pango/README.md) | 1.54.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 9 | ✅ | ✅ |
| [patch](ports/patch/README.md) | 2.8 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [pax-utils](ports/pax-utils/README.md) | 1.3.8 | meson | dev.gentoo.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [pcl](ports/pcl/README.md) | 1.14.1 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [pcre](ports/pcre/README.md) | 8.45 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [pcre2](ports/pcre2/README.md) | 10.44 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pdfium](ports/pdfium/README.md) | 0-git-84a8011 | custom | pdfium.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [perl-error](ports/perl-error/README.md) | 0.17030 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-mozilla-ca](ports/perl-mozilla-ca/README.md) | 20261020 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl](ports/perl/README.md) | 5.40.2 | custom | www.cpan.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [picocom](ports/picocom/README.md) | 3.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [pistache](ports/pistache/README.md) | 0.4.26 | meson | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [pixman](ports/pixman/README.md) | 0.46.4 | meson | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pkgconf](ports/pkgconf/README.md) | 2.3.0 | meson | distfiles.ariadne.space | — | 🟢 standalone | — | ✅ | ✅ |
| [poppler](ports/poppler/README.md) | 25.12.0 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 12 | ✅ | ✅ |
| [pqiv](ports/pqiv/README.md) | 2.13.2 | custom | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [protobuf-c](ports/protobuf-c/README.md) | 1.5.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [protobuf](ports/protobuf/README.md) | 29.3 | custom | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [pthread-stubs](ports/pthread-stubs/README.md) | 0.5 | autotools | xorg.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [pthreadpool](ports/pthreadpool/README.md) | 0-git-20250207 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pugixml](ports/pugixml/README.md) | 1.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [python3-astor](ports/python3-astor/README.md) | 0.8.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-async-generator](ports/python3-async-generator/README.md) | 1.10 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-click](ports/python3-click/README.md) | 8.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-coverage](ports/python3-coverage/README.md) | 7.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-curio](ports/python3-curio/README.md) | 1.6_git20251221 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-dateutil](ports/python3-dateutil/README.md) | 2.9.0.post0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-idna](ports/python3-idna/README.md) | 3.20 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-iniconfig](ports/python3-iniconfig/README.md) | 2.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-mypy-extensions](ports/python3-mypy-extensions/README.md) | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-packaging](ports/python3-packaging/README.md) | 25.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pip](ports/python3-pip/README.md) | 25.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pluggy](ports/python3-pluggy/README.md) | 1.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pretend](ports/python3-pretend/README.md) | 1.0.9 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pygments](ports/python3-pygments/README.md) | 2.21.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-cov](ports/python3-pytest-cov/README.md) | 7.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-tornasync](ports/python3-pytest-tornasync/README.md) | 0.6.0.post2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest](ports/python3-pytest/README.md) | 9.2.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytokens](ports/python3-pytokens/README.md) | 0.4.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytz](ports/python3-pytz/README.md) | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-setuptools](ports/python3-setuptools/README.md) | 80.8.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-six](ports/python3-six/README.md) | 1.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sniffio](ports/python3-sniffio/README.md) | 1.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sortedcontainers](ports/python3-sortedcontainers/README.md) | 2.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tornado](ports/python3-tornado/README.md) | 6.6 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-traitlets](ports/python3-traitlets/README.md) | 5.16.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tzdata](ports/python3-tzdata/README.md) | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-versioneer](ports/python3-versioneer/README.md) | 0.29 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-wheel](ports/python3-wheel/README.md) | 0.46.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3](ports/python3/README.md) | 3.14.0 | autotools | www.python.org | 1 | 🔴 dependent | 11 | ✅ | ✅ |
| [pytorch](ports/pytorch/README.md) | 2.3.1 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [qt6](ports/qt6/README.md) | 6.8.1 | custom | download.qt.io | 1 | 🔴 dependent | 19 | ✅ | ✅ |
| [quickjs-ng](ports/quickjs-ng/README.md) | 0.8.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [rapidjson](ports/rapidjson/README.md) | 1.1.0-24b5e7a | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [re2](ports/re2/README.md) | 2024-07-02 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [readline](ports/readline/README.md) | 8.2 | autotools | ftp.gnu.org | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [restbed](ports/restbed/README.md) | 4.8 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [retroarch](ports/retroarch/README.md) | 1.15-git-20230408 | custom | github.com | 2 | 🔴 dependent | 7 | ✅ | ✅ |
| [retropie](ports/retropie/README.md) | 0.3 | custom | custom | — | 🟡 build-deps only | — | ✅ | ✅ |
| [ros2](ports/ros2/README.md) | jazzy | custom | custom | — | 🔴 dependent | 14 | ✅ | ✅ |
| [rsync](ports/rsync/README.md) | 3.3.0 | autotools | download.samba.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [rtl-sdr](ports/rtl-sdr/README.md) | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [rust](ports/rust/README.md) | 1.90.0 | custom | static.rust-lang.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ruy](ports/ruy/README.md) | 0-git-20240321 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [s2n-tls](ports/s2n-tls/README.md) | 1.7.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl2-compat](ports/sdl2-compat/README.md) | 2.32.60 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl2-image](ports/sdl2-image/README.md) | 2.8.8 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [sdl2-net](ports/sdl2-net/README.md) | 2.2.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl2-ttf](ports/sdl2-ttf/README.md) | 2.24.0 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [sdl2](ports/sdl2/README.md) | 2.32.10 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl3](ports/sdl3/README.md) | 3.4.18 | cmake | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [shared-mime-info](ports/shared-mime-info/README.md) | 2.4 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [simdjson](ports/simdjson/README.md) | 3.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [simdutf](ports/simdutf/README.md) | 6.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [sleef](ports/sleef/README.md) | 3.8-git-20250214 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [snappy](ports/snappy/README.md) | 1.2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [soxr](ports/soxr/README.md) | 0.1.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [spdlog](ports/spdlog/README.md) | 1.15.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [speexdsp](ports/speexdsp/README.md) | 1.2.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [sqlite](ports/sqlite/README.md) | 3.47.2 | autotools | www.sqlite.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sqlite_orm](ports/sqlite_orm/README.md) | 1.9 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sscep](ports/sscep/README.md) | 0.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [stable-diffusion.cpp](ports/stable-diffusion.cpp/README.md) | 0_git20260601 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [strongswan](ports/strongswan/README.md) | 6.0.6 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [sudo](ports/sudo/README.md) | 1.9.17p2 | autotools | www.sudo.ws | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [suitesparse](ports/suitesparse/README.md) | 7.11.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [svt-av1](ports/svt-av1/README.md) | 2.3.0 | cmake | gitlab.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [swaybg](ports/swaybg/README.md) | 1.2.2 | meson | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [sysbench](ports/sysbench/README.md) | 1.0.20-git-20250309 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [taglib](ports/taglib/README.md) | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [tar](ports/tar/README.md) | 1.35 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [tensorflow-lite](ports/tensorflow-lite/README.md) | 2.16.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [tftp-hpa](ports/tftp-hpa/README.md) | 5.2 | custom | git.kernel.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [thorvg](ports/thorvg/README.md) | 0.15.8 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [thunar](ports/thunar/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 18 | ✅ | ✅ |
| [tinyxml2](ports/tinyxml2/README.md) | 10.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tmux](ports/tmux/README.md) | 3.5a | autotools | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [tomlplusplus](ports/tomlplusplus/README.md) | 3.4.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-bash](ports/tree-sitter-bash/README.md) | 0.25.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-c](ports/tree-sitter-c/README.md) | 0.24.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-cmake](ports/tree-sitter-cmake/README.md) | 0.7.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-lua](ports/tree-sitter-lua/README.md) | 0.4.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-make](ports/tree-sitter-make/README.md) | 1.1.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-markdown](ports/tree-sitter-markdown/README.md) | 0.5.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-python](ports/tree-sitter-python/README.md) | 0.23.6 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-query](ports/tree-sitter-query/README.md) | 0.7.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-vim](ports/tree-sitter-vim/README.md) | 0.7.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-vimdoc](ports/tree-sitter-vimdoc/README.md) | 4.0.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter](ports/tree-sitter/README.md) | 0.25.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree](ports/tree/README.md) | 2.2.1 | custom | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [trompeloeil](ports/trompeloeil/README.md) | 48 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tumbler](ports/tumbler/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 13 | ✅ | ✅ |
| [unbound](ports/unbound/README.md) | 1.24.1 | autotools | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [unibilium](ports/unibilium/README.md) | 2.1.2 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [urdfdom_headers](ports/urdfdom_headers/README.md) | 1.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [uriparser](ports/uriparser/README.md) | 0.9.8 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [utf8proc](ports/utf8proc/README.md) | 2.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [vidstab](ports/vidstab/README.md) | 1.1.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [vim](ports/vim/README.md) | 9.1.0900 | autotools | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [virtualgl](ports/virtualgl/README.md) | 3.1.5 | cmake | github.com | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [vlc](ports/vlc/README.md) | 3.0.21 | custom | code.videolan.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [vsomeip](ports/vsomeip/README.md) | 3.5.6 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [vte](ports/vte/README.md) | 0.78.2 | meson | download.gnome.org | 3 | 🔴 dependent | 14 | ✅ | ✅ |
| [wayland-protocols](ports/wayland-protocols/README.md) | 1.47 | meson | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [wayland](ports/wayland/README.md) | 1.24.0 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [webkitgtk](ports/webkitgtk/README.md) | 2.44.4 | cmake | webkitgtk.org | 5 | 🔴 dependent | 32 | ✅ | ✅ |
| [webrtc](ports/webrtc/README.md) | m132-afaf497 | custom | webrtc.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [weston](ports/weston/README.md) | 13.0.3 | — | gitlab.freedesktop.org | — | ⛔ blocked | — | — | — |
| [wget](ports/wget/README.md) | 1.25.0 | autotools | ftp.gnu.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [whisper.cpp](ports/whisper.cpp/README.md) | 0_git20260728 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [wlr-protocols](ports/wlr-protocols/README.md) | 0-git-20250702 | custom | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [wlroots](ports/wlroots/README.md) | 0.20.2 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [woff2](ports/woff2/README.md) | 1.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [wolfssl](ports/wolfssl/README.md) | 5.7.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [x264](ports/x264/README.md) | stable | custom | code.videolan.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [x265](ports/x265/README.md) | 3.6 | cmake | bitbucket.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [xcb-proto](ports/xcb-proto/README.md) | 1.17.0 | autotools | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xcb-util](ports/xcb-util/README.md) | 0.4.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [xdg-utils](ports/xdg-utils/README.md) | 1.2.1 | autotools | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [xfce4-appfinder](ports/xfce4-appfinder/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [xfce4-panel](ports/xfce4-panel/README.md) | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 17 | ✅ | ✅ |
| [xfce4-session](ports/xfce4-session/README.md) | 4.20.2 | autotools | archive.xfce.org | 2 | 🔴 dependent | 11 | ✅ | ✅ |
| [xfce4-settings](ports/xfce4-settings/README.md) | 4.20.1 | autotools | archive.xfce.org | — | 🔴 dependent | 16 | ✅ | ✅ |
| [xfce4-terminal](ports/xfce4-terminal/README.md) | 1.1.5 | meson | archive.xfce.org | 2 | 🔴 dependent | 12 | ✅ | ✅ |
| [xfconf](ports/xfconf/README.md) | 4.20.0 | autotools | archive.xfce.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [xkbcomp](ports/xkbcomp/README.md) | 1.5.0 | meson | xorg.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [xkeyboard-config](ports/xkeyboard-config/README.md) | 2.43 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xnnpack](ports/xnnpack/README.md) | 0-git-20250209 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [xorgproto](ports/xorgproto/README.md) | 2024.1 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xtrans](ports/xtrans/README.md) | 1.5.2 | autotools | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xvidcore](ports/xvidcore/README.md) | 1.3.7 | custom | downloads.xvid.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xwayland](ports/xwayland/README.md) | 24.1.14 | meson | gitlab.freedesktop.org | 5 | 🔴 dependent | 6 | ✅ | ✅ |
| [xxhash](ports/xxhash/README.md) | 0.8.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xz](ports/xz/README.md) | 5.6.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [yaml-cpp](ports/yaml-cpp/README.md) | 0.8.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zenoh-c](ports/zenoh-c/README.md) | 1.2.1 | custom | github.com | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [zimg](ports/zimg/README.md) | 3.0.5 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [zlib-ng](ports/zlib-ng/README.md) | 2.2.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zlib](ports/zlib/README.md) | 1.3.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zsh](ports/zsh/README.md) | 5.9 | autotools | downloads.sourceforge.net | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [zstd](ports/zstd/README.md) | 1.5.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |

