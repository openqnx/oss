# Component index

525 components. **524** build for both `aarch64le` and `x86_64` from upstream source, carrying 288 QNX patches between them.

| Class | | Components |
|---|---|---|
| 🟢 **standalone** | builds with the QNX SDP only, runs on a stock target | **223** |
| 🟡 **build-deps only** | needs other components to build, runs on its own | **63** |
| 🔴 **dependent** | needs other components to build and to run | **238** |
| 📋 **planned** | recorded for porting, not ported yet | **0** |
| ⛔ **blocked** | tried; cannot be built for QNX as things stand (the recipe says why) | **1** |

How the class is decided, and every component's full chain: [DEPENDENCIES.md](DEPENDENCIES.md).

Click a component for its source URL, its patches, the exact build commands and how it is verified on a target.

| Component | What it is | Version | Build | Upstream | Patches | Class | Run-time deps | aarch64le | x86_64 |
|---|---|---|---|---|---|---|---|---|---|
| [abseil](ports/abseil/README.md) | Collection of C++ libraries augmenting the C++ standard library | 20240722.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [abuild](ports/abuild/README.md) | Alpine's tools for building APK packages | 3.16.0 | custom | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [ada](ports/ada/README.md) | Fast WHATWG-compliant URL parser library for C++ | 3.4.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-fonts](ports/adwaita-fonts/README.md) | Adwaita Sans and Adwaita Mono, the GNOME fonts | 49.0 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-icon-theme](ports/adwaita-icon-theme/README.md) | GNOME's Adwaita icon theme and default cursors | 51.0 | meson | download.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [amqp-cpp](ports/amqp-cpp/README.md) | C++ library for communicating with a RabbitMQ message broker | 4.3.27 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [aom](ports/aom/README.md) | AV1 video codec library from the Alliance for Open Media | 3.12.1 | cmake | aomedia.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [apk-tools](ports/apk-tools/README.md) | Alpine Package Keeper (apk), the Alpine package manager | 3.0.0_rc5 | meson | github.com | 4 | 🔴 dependent | 3 | ✅ | ✅ |
| [asio](ports/asio/README.md) | Header-only C++ library for asynchronous network and low-level I/O | 1.30.2 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [aspell](ports/aspell/README.md) | GNU Aspell spell checker and library | 0.60.8.1 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [at-spi2-core](ports/at-spi2-core/README.md) | Accessibility infrastructure: AT-SPI D-Bus interfaces, libatspi and ATK | 2.54.0 | meson | download.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [autoconf](ports/autoconf/README.md) | GNU tool for generating configure scripts from M4 macros | 2.72 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [automake](ports/automake/README.md) | GNU tool for generating portable Makefile.in files | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [aws-c-auth](ports/aws-c-auth/README.md) | C library for AWS client-side authentication: credentials and signing | 0.10.1 | cmake | github.com | — | 🔴 dependent | 8 | ✅ | ✅ |
| [aws-c-cal](ports/aws-c-cal/README.md) | AWS crypto abstraction layer, C wrapper for cryptography primitives | 0.9.13 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [aws-c-common](ports/aws-c-common/README.md) | Core C library of the AWS SDK: primitives, data structures, errors | 0.12.6 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [aws-c-compression](ports/aws-c-compression/README.md) | C library of compression algorithms for the AWS SDK (Huffman coding) | 0.3.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-c-event-stream](ports/aws-c-event-stream/README.md) | C implementation of the AWS event-stream message encoding | 0.7.0 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [aws-c-http](ports/aws-c-http/README.md) | C implementation of HTTP/1.1 and HTTP/2 for the AWS SDK | 0.10.14 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [aws-c-io](ports/aws-c-io/README.md) | Event-driven I/O and TLS library for the AWS SDK for C | 0.26.1 | cmake | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [aws-c-mqtt](ports/aws-c-mqtt/README.md) | C implementation of the MQTT 3.1.1 and MQTT 5 protocols for AWS | 0.15.2 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [aws-c-s3](ports/aws-c-s3/README.md) | Asynchronous high-throughput AWS S3 client library in C | 0.12.2 | cmake | github.com | — | 🔴 dependent | 10 | ✅ | ✅ |
| [aws-c-sdkutils](ports/aws-c-sdkutils/README.md) | C library of AWS SDK utilities: ARN parsing, AWS profile reading | 0.2.4 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-checksums](ports/aws-checksums/README.md) | Hardware-accelerated CRC32 and CRC32C checksum library for the AWS SDK | 0.2.10 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [aws-crt-cpp](ports/aws-crt-cpp/README.md) | C++ wrapper around the AWS Common Runtime (aws-c-*) libraries | 0.38.5 | cmake | github.com | 1 | 🔴 dependent | 13 | ✅ | ✅ |
| [azure-iot-sdk-c](ports/azure-iot-sdk-c/README.md) | C SDK for connecting devices to Azure IoT Hub and Device Provisioning | LTS_08_2024 | cmake | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [bash](ports/bash/README.md) | GNU Bourne Again shell | 5.2.37 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [bc](ports/bc/README.md) | GNU bc, an arbitrary-precision calculator language | 1.08.2 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [benchmark](ports/benchmark/README.md) | Google's C++ library for benchmarking code snippets | 1.9.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [binutils](ports/binutils/README.md) | GNU binary tools: assembler, linker, objdump, readelf and others | 2.43 | autotools | ftp.gnu.org | 10 | 🔴 dependent | 2 | ✅ | ✅ |
| [boost](ports/boost/README.md) | Boost peer-reviewed portable C++ source libraries | 1.86.0 | custom | archives.boost.io | 4 | 🔴 dependent | 3 | ✅ | ✅ |
| [brotli](ports/brotli/README.md) | Generic lossless compression library and tool using the Brotli format | 1.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [btop](ports/btop/README.md) | Terminal resource monitor for CPU, memory, disks, network and processes | 1.4.6 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [busybox](ports/busybox/README.md) | Tiny versions of many common UNIX utilities in a single executable | 1.36.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [bzip2](ports/bzip2/README.md) | Block-sorting file compressor and libbz2 library | 1.0.8 | custom | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [c-ares](ports/c-ares/README.md) | Asynchronous DNS resolver library in C | 1.34.3 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ca-certificates](ports/ca-certificates/README.md) | Mozilla's CA certificate bundle for TLS verification | 20261020 | custom | curl.se | — | 🟢 standalone | — | ✅ | ✅ |
| [cairo](ports/cairo/README.md) | 2D vector graphics library with multiple output backends | 1.18.2 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [candb](ports/candb/README.md) | Library for parsing CAN bus database description formats | 0.1-git-20210625 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [capicxx-core-runtime](ports/capicxx-core-runtime/README.md) | COVESA CommonAPI C++ core runtime for IPC interfaces | 3.2.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [capicxx-someip-runtime](ports/capicxx-someip-runtime/README.md) | COVESA CommonAPI C++ SOME/IP binding runtime | 3.2.4 | cmake | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [capnproto](ports/capnproto/README.md) | Cap'n Proto data serialization format and capability-based RPC system | 1.0.2 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [capstone](ports/capstone/README.md) | Multi-architecture disassembly framework | 5.0.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [catch2](ports/catch2/README.md) | C++ unit testing framework with micro-benchmarking and BDD macros | 3.7.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cereal](ports/cereal/README.md) | Header-only C++11 library for serialization | 1.3.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ceres-solver](ports/ceres-solver/README.md) | C++ library for modeling and solving large optimization problems | 2.2.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [check](ports/check/README.md) | Unit testing framework for C | 0.15.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [chrony](ports/chrony/README.md) | NTP client and server for synchronizing the system clock | 4.8-git-20260226 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [civetweb](ports/civetweb/README.md) | Embeddable C/C++ web server with optional Lua and WebSocket support | 1.16 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [cjson](ports/cjson/README.md) | Lightweight JSON parser in ANSI C | 1.7.18 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [clapack](ports/clapack/README.md) | LAPACK linear algebra routines translated to C with f2c | 3.2.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [cli11](ports/cli11/README.md) | Header-only command line parser for C++11 and later | 2.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cmake](ports/cmake/README.md) | Cross-platform build system generator | 4.1.1 | cmake | gitlab.kitware.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [computelibrary](ports/computelibrary/README.md) | Arm Compute Library of optimized NEON/SVE kernels for Arm CPUs | 53.0.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | n/a |
| [console_bridge](ports/console_bridge/README.md) | ROS-independent C++ logging library mirroring rosconsole calls | 1.0.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cppzmq](ports/cppzmq/README.md) | Header-only C++ binding for the ZeroMQ messaging library | 4.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [cpuinfo](ports/cpuinfo/README.md) | C library for detecting CPU features relevant to performance tuning | 0-git-20240417 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [csmith](ports/csmith/README.md) | Random generator of C programs for testing compilers | 2.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cunit](ports/cunit/README.md) | Unit testing framework for C | 2.1-3 | custom | downloads.sourceforge.net | 1 | 🟢 standalone | — | ✅ | ✅ |
| [curl](ports/curl/README.md) | Command line tool and library for transferring data with URLs | 8.11.1 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [cyclonedds](ports/cyclonedds/README.md) | Eclipse Cyclone DDS, an implementation of the OMG DDS standard | 0.10.5 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [dav1d](ports/dav1d/README.md) | Fast cross-platform AV1 video decoder library | 1.5.0 | meson | code.videolan.org | — | 🟢 standalone | — | ✅ | ✅ |
| [dbus](ports/dbus/README.md) | D-Bus message bus daemon and library for interprocess communication | 1.16.0 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [desktop-file-utils](ports/desktop-file-utils/README.md) | Command line utilities for validating and installing .desktop files | 0.28 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [dlt-daemon](ports/dlt-daemon/README.md) | COVESA Diagnostic Log and Trace (DLT) daemon, library and tools | 2.18.10 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [dnsmasq](ports/dnsmasq/README.md) | Lightweight DNS forwarder and DHCP server | 2.93test4 | custom | thekelleys.org.uk | 1 | 🟢 standalone | — | ✅ | ✅ |
| [docbook-xsl](ports/docbook-xsl/README.md) | DocBook XSL stylesheets for converting DocBook XML to HTML and man pages | 1.79.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [double-conversion](ports/double-conversion/README.md) | C++ routines for binary-decimal conversion of IEEE doubles | 3.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [doxygen](ports/doxygen/README.md) | Documentation generator for C, C++ and other languages | 1.17.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [duktape](ports/duktape/README.md) | Small embeddable ECMAScript (JavaScript) engine with a C API | 2.7.0 | custom | duktape.org | — | 🟢 standalone | — | ✅ | ✅ |
| [dump1090](ports/dump1090/README.md) | Mode S (ADS-B) aircraft transponder decoder for RTL-SDR devices | 0-git-20240121 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [ed](ports/ed/README.md) | GNU ed, the POSIX line-oriented text editor | 1.22.5 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [eigen](ports/eigen/README.md) | Header-only C++ template library for linear algebra | 3.4.0 | cmake | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [emacs](ports/emacs/README.md) | GNU Emacs extensible text editor (terminal build) | 30.2 | custom | ftp.gnu.org | 4 | 🔴 dependent | 9 | ✅ | ✅ |
| [emulationstation](ports/emulationstation/README.md) | Graphical front-end for browsing and launching emulated games | 2.11-git-20241212 | cmake | github.com | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [enca](ports/enca/README.md) | Detects and converts the character encoding of text files | 1.19 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [enchant](ports/enchant/README.md) | Generic spell checking library wrapping several spell checker backends | 2.8.2 | autotools | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [encodings](ports/encodings/README.md) | X.Org font encoding tables for libfontenc | 1.1.0 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [epiphany](ports/epiphany/README.md) | GNOME Web, a WebKitGTK-based web browser | 43.1 | meson | download.gnome.org | 5 | 🔴 dependent | 20 | ✅ | ✅ |
| [exiv2](ports/exiv2/README.md) | C++ library and tool for reading and writing Exif, IPTC and XMP metadata | 0.28.5 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [exo](ports/exo/README.md) | Extension library and helper applications for the Xfce desktop | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [expat](ports/expat/README.md) | Stream-oriented XML parser library in C | 2.6.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fakeroot](ports/fakeroot/README.md) | Runs a command in an environment faking root privileges for packaging | 1.37.1.1 | autotools | deb.debian.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [farmhash](ports/farmhash/README.md) | Google's FarmHash family of hash functions for strings | 1.1-git-20190513 | autotools | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [fastcdr](ports/fastcdr/README.md) | eProsima Fast CDR, C++ serialization library for the CDR format | 2.2.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fastdds](ports/fastdds/README.md) | eProsima Fast DDS, C++ implementation of the OMG DDS standard | 2.14.4 | cmake | github.com | 2 | 🔴 dependent | 4 | ✅ | ✅ |
| [fcl](ports/fcl/README.md) | Flexible Collision Library for collision and proximity queries | 0.7.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fdk-aac](ports/fdk-aac/README.md) | Fraunhofer FDK AAC audio codec library | 2.0.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ffmpeg](ports/ffmpeg/README.md) | Libraries and tools for decoding, encoding and processing audio and video | 7.1.1 | custom | github.com | — | 🔴 dependent | 12 | ✅ | ✅ |
| [ffmpegthumbnailer](ports/ffmpegthumbnailer/README.md) | Lightweight video thumbnailer library and tool using FFmpeg | 2.2.3 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [file](ports/file/README.md) | File type identification tool and libmagic library | 5.46 | autotools | astron.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [flac](ports/flac/README.md) | Free Lossless Audio Codec library and tools | 1.4.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flann](ports/flann/README.md) | Fast Library for Approximate Nearest Neighbors search | 1.9.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flatbuffers](ports/flatbuffers/README.md) | Memory-efficient serialization library and flatc schema compiler | 24.3.25 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fmt](ports/fmt/README.md) | Fast and safe C++ formatting library | 11.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-cantarell](ports/font-cantarell/README.md) | Cantarell, the GNOME user interface font | 0.303.1 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [font-dejavu](ports/font-dejavu/README.md) | DejaVu font family (sans, serif, mono) | 2.37 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-hack](ports/font-hack/README.md) | Hack, a monospace font for source code | 3.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-inter](ports/font-inter/README.md) | Inter, a sans-serif font family for user interfaces | 4.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-liberation](ports/font-liberation/README.md) | Liberation fonts, metric-compatible with Arial, Times New Roman, Courier | 2.1.5 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-emoji](ports/font-noto-emoji/README.md) | Noto Color Emoji font | 2.048 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-sans-cjk](ports/font-noto-sans-cjk/README.md) | Noto Sans CJK fonts for Chinese, Japanese and Korean | 2.004 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-serif-cjk](ports/font-noto-serif-cjk/README.md) | Noto Serif CJK fonts for Chinese, Japanese and Korean | 2.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-util](ports/font-util/README.md) | X.Org font utilities and autoconf macros for building font packages | 1.4.1 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [fontconfig](ports/fontconfig/README.md) | Library for font configuration, discovery and customization | 2.15.0 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fontenc](ports/fontenc/README.md) | X.Org font encoding library | 1.1.8 | autotools | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [foonathan_memory](ports/foonathan_memory/README.md) | C++ memory allocator library improving on the STL allocator model | 0.7-3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fp16](ports/fp16/README.md) | Header-only C library for converting to and from half-precision floats | 0-git-20260620 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [freeimage](ports/freeimage/README.md) | Library for loading and saving popular image formats (PNG, JPEG, TIFF...) | 3.18.0 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [freetype](ports/freetype/README.md) | Portable font rendering engine library | 2.13.3 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [fribidi](ports/fribidi/README.md) | Implementation of the Unicode Bidirectional Algorithm | 1.0.16 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [fxdiv](ports/fxdiv/README.md) | Header-only C library for integer division via fixed-point multiplication | 0-git-20200417 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [g3log](ports/g3log/README.md) | Asynchronous crash-safe C++ logging library | 2.4 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [garcon](ports/garcon/README.md) | Freedesktop.org compliant menu library for Xfce | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [gawk](ports/gawk/README.md) | GNU awk pattern scanning and text processing language | 5.3.2 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [gcr](ports/gcr/README.md) | GNOME library for displaying certificates and accessing key stores | 3.41.2 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 8 | ✅ | ✅ |
| [gdb](ports/gdb/README.md) | GNU debugger | 14.2 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 10 | ✅ | ✅ |
| [gdbm](ports/gdbm/README.md) | GNU dbm key/value database library | 1.24 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gdk-pixbuf](ports/gdk-pixbuf/README.md) | GTK library for loading and scaling images | 2.42.12 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [geany-plugins](ports/geany-plugins/README.md) | Collection of plugins for the Geany IDE | 2.1 | autotools | plugins.geany.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [geany](ports/geany/README.md) | Fast and lightweight IDE using GTK | 2.1 | autotools | download.geany.org | 1 | 🔴 dependent | 9 | ✅ | ✅ |
| [gettext](ports/gettext/README.md) | GNU internationalization tools and libraries for translated messages | 0.22.5 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [gflags](ports/gflags/README.md) | C++ library for command line flags processing | 2.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [giflib](ports/giflib/README.md) | Library and tools for reading and writing GIF images | 5.2.2 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [git](ports/git/README.md) | Fast distributed version control system | 2.51.0 | custom | mirrors.edge.kernel.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [gklib](ports/gklib/README.md) | Helper routines library used by METIS and related software | 5.1.1-git-20230326 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [glib-networking](ports/glib-networking/README.md) | TLS (GnuTLS) and proxy support modules for GLib's GIO | 2.80.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [glib](ports/glib/README.md) | Low-level core C library of GTK and GNOME: data types, main loop, GIO | 2.82.4 | meson | gitlab.gnome.org | 2 | 🔴 dependent | 5 | ✅ | ✅ |
| [glm](ports/glm/README.md) | Header-only C++ mathematics library for graphics based on GLSL | 1.0.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [glog](ports/glog/README.md) | Google's C++ application-level logging library | 0.7.1 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [gmp](ports/gmp/README.md) | GNU Multiple Precision arithmetic library | 6.3.0 | autotools | gmplib.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gnutls](ports/gnutls/README.md) | GNU TLS/SSL protocol implementation library | 3.8.8 | autotools | www.gnupg.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [gobject-introspection](ports/gobject-introspection/README.md) | GObject Introspection library and tools for API metadata | 1.86.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [godot](ports/godot/README.md) | Godot game engine runtime (export templates) for 2D and 3D games | 4.4.1 | custom | github.com | 1 | 🔴 dependent | 17 | ✅ | ✅ |
| [googletest](ports/googletest/README.md) | Google's C++ testing and mocking framework (GoogleTest, GoogleMock) | 1.17.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gperf](ports/gperf/README.md) | GNU perfect hash function generator | 3.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [graphene](ports/graphene/README.md) | Thin layer of math types (vectors, matrices, points) for graphics libraries | 1.10.8 | meson | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [graphicsmagick](ports/graphicsmagick/README.md) | Image processing tools and libraries (gm, Magick++) | 1.3.46 | autotools | downloads.sourceforge.net | — | 🔴 dependent | 13 | ✅ | ✅ |
| [graphite2](ports/graphite2/README.md) | Font rendering engine for complex scripts (SIL Graphite smart fonts) | 1.3.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [grep](ports/grep/README.md) | GNU grep, searches files for lines matching a pattern | 3.11 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [groff](ports/groff/README.md) | GNU troff document formatting system | 1.23.0 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [grpc](ports/grpc/README.md) | Remote procedure call (RPC) framework from Google | 1.69.0 | custom | github.com | 2 | 🔴 dependent | 6 | ✅ | ✅ |
| [gsettings-desktop-schemas](ports/gsettings-desktop-schemas/README.md) | GSettings schemas for settings shared by desktop components | 49.0 | meson | gitlab.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [gsm](ports/gsm/README.md) | GSM 06.10 lossy speech compression library | 1.0.22 | custom | www.quut.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gst-libav](ports/gst-libav/README.md) | GStreamer plugin for FFmpeg (libav) codecs and formats | 1.24.10 | meson | custom | — | 🔴 dependent | 4 | ✅ | ✅ |
| [gst-plugins-bad](ports/gst-plugins-bad/README.md) | GStreamer plugins of lesser quality or maturity (bad set) | 1.24.10 | meson | custom | — | 🔴 dependent | 11 | ✅ | ✅ |
| [gst-plugins-base](ports/gst-plugins-base/README.md) | Essential GStreamer plugins and helper libraries (base set) | 1.24.10 | meson | custom | — | 🔴 dependent | 8 | ✅ | ✅ |
| [gst-plugins-good](ports/gst-plugins-good/README.md) | Well-supported GStreamer plugins under preferred licenses (good set) | 1.24.10 | meson | custom | — | 🔴 dependent | 14 | ✅ | ✅ |
| [gst-plugins-ugly](ports/gst-plugins-ugly/README.md) | Quality GStreamer plugins with possible licensing issues (ugly set) | 1.24.10 | meson | custom | — | 🔴 dependent | 5 | ✅ | ✅ |
| [gstreamer](ports/gstreamer/README.md) | GStreamer streaming media framework core | 1.24.10 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [gthumb](ports/gthumb/README.md) | Image viewer and organizer for GNOME | 3.12.8.2 | meson | download.gnome.org | 4 | 🔴 dependent | 13 | ✅ | ✅ |
| [gtk-layer-shell](ports/gtk-layer-shell/README.md) | Library for using the Wayland layer shell protocol with GTK 3 | 0.9.0 | meson | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [gtk-vnc](ports/gtk-vnc/README.md) | VNC client library and GTK 3 widget, with the gvncviewer viewer | 1.3.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 9 | ✅ | ✅ |
| [gtk3](ports/gtk3/README.md) | GTK 3 multi-platform toolkit for graphical user interfaces | 3.24.43 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 21 | ✅ | ✅ |
| [gtk4](ports/gtk4/README.md) | GTK 4 multi-platform toolkit for graphical user interfaces | 4.16.7 | meson | download.gnome.org | 1 | 🔴 dependent | 15 | ✅ | ✅ |
| [gtsam](ports/gtsam/README.md) | Georgia Tech Smoothing and Mapping library for robotics and SLAM | 4.2 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [harfbuzz](ports/harfbuzz/README.md) | OpenType text shaping engine | 10.1.0 | meson | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [hicolor-icon-theme](ports/hicolor-icon-theme/README.md) | Freedesktop.org default fallback icon theme | 0.18 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [highway](ports/highway/README.md) | Performance-portable C++ SIMD/vector intrinsics library | 1.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [htop](ports/htop/README.md) | Interactive process viewer | 3.5.0 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [hyphen](ports/hyphen/README.md) | Hyphenation library using converted TeX hyphenation patterns | 2.8.8 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [iceoryx](ports/iceoryx/README.md) | Eclipse iceoryx, true zero-copy inter-process communication middleware | 2.0.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [icu](ports/icu/README.md) | International Components for Unicode: Unicode and locale support libraries | 76.1 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [iperf3](ports/iperf3/README.md) | TCP, UDP and SCTP network bandwidth measurement tool | 3.17.1 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [iso-codes](ports/iso-codes/README.md) | Lists of ISO country, language, script and currency codes with translations | 4.17.0 | autotools | salsa.debian.org | — | 🟢 standalone | — | ✅ | ✅ |
| [itstool](ports/itstool/README.md) | Translates XML documents with PO files using W3C ITS rules | 2.0.7 | autotools | files.itstool.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [jansson](ports/jansson/README.md) | C library for encoding, decoding and manipulating JSON data | 2.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [javascriptcore](ports/javascriptcore/README.md) | WebKit's JavaScriptCore JavaScript engine (standalone JSCOnly port) | 2.44.4 | cmake | webkitgtk.org | 4 | 🔴 dependent | 1 | ✅ | ✅ |
| [jq](ports/jq/README.md) | Command line JSON processor and libjq library | 1.7.1 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [json-c](ports/json-c/README.md) | JSON implementation in C | 0.18 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [json-glib](ports/json-glib/README.md) | JSON parser and generator library for GLib | 1.10.6 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [jsoncpp](ports/jsoncpp/README.md) | C++ library for reading and writing JSON | 1.9.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [kineto](ports/kineto/README.md) | PyTorch profiler library (libkineto) for CPU performance tracing | 0-git-20250214 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [labwc](ports/labwc/README.md) | Stacking Wayland compositor (window manager) based on wlroots | 0.20.2 | meson | github.com | 2 | 🔴 dependent | 10 | ✅ | ✅ |
| [lame](ports/lame/README.md) | LAME MP3 audio encoder and library | 3.100 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [lcm](ports/lcm/README.md) | Lightweight Communications and Marshalling: message passing for real time | 1.5.2 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [lcms2](ports/lcms2/README.md) | Little CMS, small-footprint ICC color management engine | 2.16 | meson | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lely-core](ports/lely-core/README.md) | Lely C/C++ libraries for CANopen, CAN, event loop and I/O | 2.3.4 | autotools | gitlab.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [less](ports/less/README.md) | Terminal pager for viewing text files one screen at a time | 668 | autotools | www.greenwoodsoftware.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [leveldb](ports/leveldb/README.md) | Fast ordered key-value storage library from Google | 1.23 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libaec](ports/libaec/README.md) | Adaptive Entropy Coding library for lossless compression, SZIP compatible | 1.1.7 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libarchive](ports/libarchive/README.md) | Library and tools (bsdtar, bsdcpio) for reading and writing archives | 3.7.7 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libass](ports/libass/README.md) | Subtitle renderer library for the ASS/SSA subtitle format | 0.17.3 | autotools | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [libavif](ports/libavif/README.md) | Library for encoding and decoding AVIF images | 1.1.1 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libcamera](ports/libcamera/README.md) | Raspberry Pi camera image processing algorithms from libcamera | 0.5.1+rpt20250722 | custom | github.com | 2 | 🟡 build-deps only | — | ✅ | n/a |
| [libcbor](ports/libcbor/README.md) | C library for parsing and generating CBOR binary data | 0.11.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libccd](ports/libccd/README.md) | Library for collision detection between convex shapes | 2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libcoap](ports/libcoap/README.md) | C implementation of the Constrained Application Protocol (CoAP) | 4.3.5 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libconfig](ports/libconfig/README.md) | C/C++ library for processing structured configuration files | 1.8.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdatrie](ports/libdatrie/README.md) | Double-array trie data structure library | 0.2.13 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdazzle](ports/libdazzle/README.md) | Companion library of extra widgets and utilities for GObject and GTK 3 | 3.44.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [libde265](ports/libde265/README.md) | H.265/HEVC video decoder library | 1.0.15 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdeflate](ports/libdeflate/README.md) | Fast library for DEFLATE, zlib and gzip compression and decompression | 1.22 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdisplay-info](ports/libdisplay-info/README.md) | EDID and DisplayID parsing library | 0.3.0 | meson | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdrm-headers](ports/libdrm-headers/README.md) | Headers for the DRM kernel interface and the libdrm API (headers only) | 2.4.129 | custom | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libedit](ports/libedit/README.md) | Command line editor library: line editing, history and tokenization | 20240808-3.1 | autotools | thrysoee.dk | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libepoxy](ports/libepoxy/README.md) | Library for managing OpenGL, EGL and GLX function pointers | 1.5.10 | meson | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libev](ports/libev/README.md) | Full-featured event loop library for C | 4.33 | autotools | dist.schmorp.de | — | 🟢 standalone | — | ✅ | ✅ |
| [libevent](ports/libevent/README.md) | Asynchronous event notification library | 2.1.12 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libexif](ports/libexif/README.md) | Library for parsing, editing and saving EXIF data | 0.6.24 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libfdt](ports/libfdt/README.md) | Device Tree Compiler (dtc) and libfdt for flat device trees | 1.7.2 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libffi](ports/libffi/README.md) | Portable foreign function interface library | 3.4.6 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libgcrypt](ports/libgcrypt/README.md) | GNU general purpose cryptographic library | 1.11.0 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgit2](ports/libgit2/README.md) | Portable C library implementing the Git core methods | 1.8.4 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [libgpg-error](ports/libgpg-error/README.md) | Common error values and runtime library for GnuPG components | 1.50 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgsf](ports/libgsf/README.md) | Library for reading and writing structured files (MS OLE, Zip) | 1.14.53 | autotools | gitlab.gnome.org | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libhandy](ports/libhandy/README.md) | GTK 3 widgets for adaptive and mobile user interfaces | 1.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [libice](ports/libice/README.md) | X Inter-Client Exchange (ICE) protocol library | 1.1.2 | autotools | xorg.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libiconv](ports/libiconv/README.md) | GNU character set conversion library (iconv) | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libidn2](ports/libidn2/README.md) | Library for internationalized domain names (IDNA2008, Punycode, TR46) | 2.3.7 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libinih](ports/libinih/README.md) | Simple INI file parser library for C and C++ | 58 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjpeg-turbo](ports/libjpeg-turbo/README.md) | SIMD-accelerated JPEG codec library and tools (libjpeg, TurboJPEG APIs) | 3.0.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjson](ports/libjson/README.md) | Small streaming JSON parser and printer in C | 0.8-git-20260512 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjxl](ports/libjxl/README.md) | JPEG XL image format reference library and tools | 0.11.1 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libmd](ports/libmd/README.md) | Message digest functions from BSD systems (MD5, SHA-1, SHA-2, RIPEMD) | 1.1.0 | autotools | libbsd.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmetal](ports/libmetal/README.md) | Abstraction library for device access, interrupts and shared memory | 2024.10.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [libmicrohttpd](ports/libmicrohttpd/README.md) | GNU C library for embedding an HTTP server | 1.0.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmodbus](ports/libmodbus/README.md) | Library for communicating with Modbus devices over serial or TCP | 3.1.11 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libnotify](ports/libnotify/README.md) | Library for sending desktop notifications | 0.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libogg](ports/libogg/README.md) | Library for the Ogg multimedia bitstream container format | 1.3.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libpcap](ports/libpcap/README.md) | Portable network traffic capture library | 1.10.5 | autotools | www.tcpdump.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpipeline](ports/libpipeline/README.md) | C library for setting up and running pipelines of processes | 1.5.8 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpng](ports/libpng/README.md) | C library for reading and writing PNG images | 1.6.44 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libportal](ports/libportal/README.md) | GIO-style client library for XDG desktop portals | 0.8.1 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libproxy](ports/libproxy/README.md) | Library for automatic proxy configuration management | 0.5.9 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libpsl](ports/libpsl/README.md) | C library for handling the Public Suffix List | 0.21.5 | meson | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libretro-2048](ports/libretro-2048/README.md) | 2048 puzzle game as a libretro core for RetroArch | 0-git-20241227 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-fake08](ports/libretro-fake08/README.md) | fake-08 PICO-8 emulator as a libretro core for RetroArch | 0-git-20260613 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-mrboom](ports/libretro-mrboom/README.md) | Mr.Boom, an 8-player Bomberman clone, as a libretro core | 0-git-20260625 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libretro-retro8](ports/libretro-retro8/README.md) | retro8 PICO-8 fantasy console emulator as a libretro core | 0-git-20260912 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-samples](ports/libretro-samples/README.md) | libretro sample cores: test patterns and input/audio tests | 0-git-20221110 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsamplerate](ports/libsamplerate/README.md) | Audio sample rate conversion library (Secret Rabbit Code) | 0.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsecret](ports/libsecret/README.md) | GObject library for storing and retrieving secrets via Secret Service | 0.21.4 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libsfdo](ports/libsfdo/README.md) | Libraries implementing freedesktop.org desktop entry, icon and basedir specs | 0.1.4 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsm](ports/libsm/README.md) | X Session Management library | 1.2.6 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libsndfile](ports/libsndfile/README.md) | C library for reading and writing sampled audio files | 1.2.2 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [libsodium](ports/libsodium/README.md) | Easy-to-use library for encryption, signatures and password hashing | 1.0.20 | autotools | download.libsodium.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsoup](ports/libsoup/README.md) | GLib-based HTTP client and server library | 3.6.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libssh](ports/libssh/README.md) | C library implementing the SSH client and server protocols | 0.11.1 | cmake | www.libssh.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libssh2](ports/libssh2/README.md) | Client-side C library implementing the SSH2 protocol | 1.11.1 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libtasn1](ports/libtasn1/README.md) | GNU library for ASN.1 and DER manipulation | 4.19.0 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libthai](ports/libthai/README.md) | Thai language support library (word breaking, input, sorting) | 0.1.29 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libtheora](ports/libtheora/README.md) | Theora video codec library | 1.1.1 | autotools | downloads.xiph.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libtiff](ports/libtiff/README.md) | Library for reading and writing TIFF images | 4.7.0 | cmake | gitlab.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libtool](ports/libtool/README.md) | GNU libltdl, a portable dynamic module loading library | 2.5.4 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunibreak](ports/libunibreak/README.md) | Implementation of Unicode line, word and grapheme breaking algorithms | 6.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libunistring](ports/libunistring/README.md) | GNU library for manipulating Unicode strings | 1.3 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunwind](ports/libunwind/README.md) | Library for determining and unwinding the call chain of a program | 1.8.3 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libusb](ports/libusb/README.md) | C library for accessing USB devices from user space | 1.0.27 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libuv](ports/libuv/README.md) | Multi-platform support library for asynchronous I/O | 1.51.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libvncserver](ports/libvncserver/README.md) | C libraries for implementing VNC servers and clients | 0.9.15 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libvorbis](ports/libvorbis/README.md) | Ogg Vorbis audio codec libraries | 1.3.7 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libvpx](ports/libvpx/README.md) | WebM VP8/VP9 video codec library and tools | 1.15.0 | custom | chromium.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libwebp](ports/libwebp/README.md) | Library and tools for the WebP image format | 1.4.0 | cmake | chromium.googlesource.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libwebsockets](ports/libwebsockets/README.md) | Lightweight C library for HTTP, WebSocket and MQTT clients and servers | 4.3.5 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libx11](ports/libx11/README.md) | Core X11 protocol client library (Xlib) | 1.8.10 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxau](ports/libxau/README.md) | X authorization file management library | 1.0.11 | autotools | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxcb](ports/libxcb/README.md) | X protocol C-language binding (XCB) libraries | 1.17.0 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxcomposite](ports/libxcomposite/README.md) | X Composite extension client library | 0.4.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxcrypt](ports/libxcrypt/README.md) | Extended crypt library for one-way password hashing | 4.4.38 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libxcursor](ports/libxcursor/README.md) | X cursor management library | 1.2.2 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxcvt](ports/libxcvt/README.md) | Library and tool for generating VESA CVT standard timing modelines | 0.1.3 | meson | xorg.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libxdamage](ports/libxdamage/README.md) | X Damage extension client library | 1.1.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxdmcp](ports/libxdmcp/README.md) | X Display Manager Control Protocol library | 1.1.5 | autotools | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxext](ports/libxext/README.md) | X11 library for common protocol extensions | 1.3.6 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxfce4ui](ports/libxfce4ui/README.md) | Shared widgets library for Xfce applications | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [libxfce4util](ports/libxfce4util/README.md) | Non-GUI utility library for the Xfce desktop | 4.20.0 | autotools | archive.xfce.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libxfce4windowing](ports/libxfce4windowing/README.md) | X11/Wayland windowing abstraction library for Xfce | 4.20.7 | autotools | archive.xfce.org | — | 🔴 dependent | 10 | ✅ | ✅ |
| [libxfixes](ports/libxfixes/README.md) | X Fixes extension client library | 6.0.1 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxfont2](ports/libxfont2/README.md) | X font handling library for X servers and utilities | 2.0.7 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxft](ports/libxft/README.md) | X FreeType library for anti-aliased font rendering in X clients | 2.3.8 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [libxi](ports/libxi/README.md) | X Input extension client library | 1.8.2 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxinerama](ports/libxinerama/README.md) | Xinerama extension client library for multi-monitor X displays | 1.1.5 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxkbcommon](ports/libxkbcommon/README.md) | Keyboard keymap compiler and support library (XKB) | 1.8.1 | meson | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libxkbfile](ports/libxkbfile/README.md) | Library for reading, writing and compiling XKB keyboard descriptions | 1.2.0 | meson | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxml2](ports/libxml2/README.md) | XML parser and toolkit library in C | 2.13.5 | cmake | gitlab.gnome.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libxmu](ports/libxmu/README.md) | X miscellaneous utility routines library | 1.3.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxrandr](ports/libxrandr/README.md) | X Resize, Rotate and Reflect (RandR) extension client library | 1.5.4 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxrender](ports/libxrender/README.md) | X Render extension client library | 0.9.11 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxscrnsaver](ports/libxscrnsaver/README.md) | X11 Screen Saver (MIT-SCREEN-SAVER) extension client library | 1.2.4 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libxshmfence](ports/libxshmfence/README.md) | Shared memory fence synchronization library for X | 1.3.3 | autotools | xorg.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libxslt](ports/libxslt/README.md) | XSLT processor library based on libxml2, with EXSLT extensions | 1.1.42 | cmake | gitlab.gnome.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxt](ports/libxt/README.md) | X Toolkit Intrinsics library (Xt) | 1.3.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxtst](ports/libxtst/README.md) | X XTEST and RECORD extensions client library | 1.2.5 | autotools | gitlab.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libyaml](ports/libyaml/README.md) | C library for parsing and emitting YAML | 0.2.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libyuv](ports/libyuv/README.md) | Library for YUV scaling and color space conversion | stable | cmake | chromium.googlesource.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libzip](ports/libzip/README.md) | C library for reading, creating and modifying zip archives | 1.11.1 | cmake | github.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libzmq](ports/libzmq/README.md) | ZeroMQ lightweight messaging library | 4.3.5 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [lighttpd](ports/lighttpd/README.md) | Lightweight, flexible web server | 1.4.77 | cmake | git.lighttpd.net | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [linux-headers-input-codes](ports/linux-headers-input-codes/README.md) | Linux input event codes header (input-event-codes.h), header only | 6.12 | custom | git.kernel.org | — | 🟢 standalone | — | ✅ | ✅ |
| [llvm](ports/llvm/README.md) | LLVM compiler infrastructure with the Clang compiler and LLD linker | 22.1.8 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [lmbench](ports/lmbench/README.md) | Suite of micro-benchmarks for operating system and hardware performance | 3.0-git-20260929 | custom | github.com | 5 | 🟢 standalone | — | ✅ | ✅ |
| [lua-lpeg](ports/lua-lpeg/README.md) | Parsing Expression Grammars (LPeg) module for Lua | 1.1.0 | custom | www.inf.puc-rio.br | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lua](ports/lua/README.md) | Lua 5.4 scripting language interpreter and library | 5.4.7 | custom | www.lua.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lua5.1](ports/lua5.1/README.md) | Lua 5.1 scripting language interpreter and library | 5.1.5 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lua5.3](ports/lua5.3/README.md) | Lua 5.3 scripting language interpreter and library | 5.3.6 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [luajit](ports/luajit/README.md) | Just-in-time compiler for the Lua programming language | 2.1.c6ffc141 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [luv](ports/luv/README.md) | libuv bindings for Lua and LuaJIT | 1.51.0-1 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [lxde-icon-theme](ports/lxde-icon-theme/README.md) | LXDE (nuoveXT) icon theme | 0.5.2 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lz4](ports/lz4/README.md) | Fast lossless compression library and tool (LZ4) | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [lzip](ports/lzip/README.md) | Lossless data compressor based on LZMA, with integrity checking | 1.26 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lzo](ports/lzo/README.md) | LZO real-time data compression library | 2.10 | autotools | www.oberhumer.com | — | 🟢 standalone | — | ✅ | ✅ |
| [m4](ports/m4/README.md) | GNU implementation of the Unix m4 macro processor | 1.4.19 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [magic_enum](ports/magic_enum/README.md) | Header-only C++17 library for static reflection of enums | 0.9.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [make](ports/make/README.md) | GNU make build automation tool | 4.4.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [mame](ports/mame/README.md) | MAME multi-purpose arcade and computer emulator | 0.281 | custom | github.com | 1 | 🔴 dependent | 5 | ✅ | ✅ |
| [man-db](ports/man-db/README.md) | Manual page reader suite (man, apropos, whatis) | 2.13.1 | autotools | download.savannah.gnu.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [mate-calc](ports/mate-calc/README.md) | MATE desktop calculator | 1.28.0 | autotools | pub.mate-desktop.org | 3 | 🔴 dependent | 13 | ✅ | ✅ |
| [mbedtls](ports/mbedtls/README.md) | Small C library for cryptography, X.509 and TLS/DTLS | 3.6.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mcap-cpp](ports/mcap-cpp/README.md) | Header-only C++ library for the MCAP robotics message log format | 2.1.3 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [meson](ports/meson/README.md) | Meson build system | 1.10.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [metis](ports/metis/README.md) | Programs and library for partitioning graphs and finite element meshes | 5.2.1 | cmake | github.com | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [mimalloc](ports/mimalloc/README.md) | Compact general-purpose memory allocator from Microsoft | 3.1.5 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [minicom](ports/minicom/README.md) | Menu-driven serial communication program and terminal emulator | 2.9 | autotools | salsa.debian.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [miniz](ports/miniz/README.md) | Single-file zlib/Deflate compression library with ZIP archive support | 3.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mkfontscale](ports/mkfontscale/README.md) | Creates fonts.scale and fonts.dir index files for X11 font directories | 1.2.3 | autotools | www.x.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [mosquitto](ports/mosquitto/README.md) | Eclipse Mosquitto MQTT broker, client libraries and pub/sub utilities | 2.0.20 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpc](ports/mpc/README.md) | GNU MPC library for complex number arithmetic with arbitrary precision | 1.3.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpfr](ports/mpfr/README.md) | C library for multiple-precision floating-point computations | 4.2.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [mpg123](ports/mpg123/README.md) | MPEG audio (MP3) decoder library and signal synthesis library | 1.32.9 | autotools | www.mpg123.de | — | 🟢 standalone | — | ✅ | ✅ |
| [msgpack-c](ports/msgpack-c/README.md) | MessagePack binary object serialization library for C | 6.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mujoco](ports/mujoco/README.md) | MuJoCo physics engine for multi-joint dynamics with contact | 3.10.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [muslflt](ports/muslflt/README.md) | musl's libm and float conversion routines as a separate library | 1.0-git-20240802 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [nano](ports/nano/README.md) | GNU nano, a small and simple terminal text editor | 8.2 | autotools | www.nano-editor.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [nanosvg](ports/nanosvg/README.md) | Header-only SVG parser and rasterizer | 0-git-20260709 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ncnn](ports/ncnn/README.md) | Neural network inference framework for mobile and embedded devices | 20260526 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ncurses](ports/ncurses/README.md) | Terminal handling and text user interface library | 6.5 | autotools | ftp.gnu.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [neovim](ports/neovim/README.md) | Vim-based extensible text editor | 0.12.3 | custom | github.com | 4 | 🔴 dependent | 6 | ✅ | ✅ |
| [nettle](ports/nettle/README.md) | Low-level cryptographic library | 3.10.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [nghttp2](ports/nghttp2/README.md) | HTTP/2 protocol implementation library in C | 1.64.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [ninja](ports/ninja/README.md) | Small build system with a focus on speed | 1.13.1 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [nlohmann_json](ports/nlohmann_json/README.md) | Header-only JSON library for modern C++ | 3.12.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nng](ports/nng/README.md) | nanomsg-next-gen lightweight messaging library (pub/sub, req/rep and more) | 1.9.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [nodejs](ports/nodejs/README.md) | Node.js cross-platform JavaScript runtime environment | 24.14.1 | custom | github.com | 9 | 🔴 dependent | 12 | ✅ | ✅ |
| [npm](ports/npm/README.md) | Package manager for JavaScript and Node.js | 11.11.0 | custom | registry.npmjs.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [ntpsec](ports/ntpsec/README.md) | Hardened implementation of the Network Time Protocol | 1.2.4 | custom | gitlab.com | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [numpy](ports/numpy/README.md) | Fundamental package for array computing in Python | 2.3.4 | custom | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [octomap](ports/octomap/README.md) | Probabilistic 3D mapping framework based on octrees | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [onetbb](ports/onetbb/README.md) | oneAPI Threading Building Blocks, C++ library for parallel programming | 2022.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [oniguruma](ports/oniguruma/README.md) | Regular expression library | 6.9.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [onnx](ports/onnx/README.md) | Open Neural Network Exchange (ONNX) format library for AI models | 1.17.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [onnxruntime](ports/onnxruntime/README.md) | Cross-platform machine learning inference engine for ONNX models | 1.20.1 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [open-amp](ports/open-amp/README.md) | OpenAMP framework for asymmetric multiprocessing (remoteproc, RPMsg) | 2024.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [open62541](ports/open62541/README.md) | C implementation of OPC UA (IEC 62541) clients and servers | 1.4.8 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openal-soft](ports/openal-soft/README.md) | Cross-platform 3D audio library implementing the OpenAL API | 1.24.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [openblas](ports/openblas/README.md) | Optimized BLAS linear algebra library based on GotoBLAS2 | 0.3.28 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [opencv](ports/opencv/README.md) | Open Source Computer Vision library | 4.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openh264](ports/openh264/README.md) | H.264 video encoding and decoding library for real-time use | 2.5.0 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [openjpeg](ports/openjpeg/README.md) | JPEG 2000 image codec library and tools | 2.5.3 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [openssh](ports/openssh/README.md) | OpenSSH client and server for secure remote login and file transfer | 10.2p1 | autotools | cdn.openbsd.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [openssl](ports/openssl/README.md) | Toolkit and libraries for TLS, DTLS, QUIC and general cryptography | 3.5.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-opengfx](ports/openttd-opengfx/README.md) | OpenGFX, the free base graphics set for OpenTTD | 8.0 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-openmsx](ports/openttd-openmsx/README.md) | OpenMSX, the free base music set for OpenTTD | 0.4.2 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd-opensfx](ports/openttd-opensfx/README.md) | OpenSFX, the free base sound set for OpenTTD | 1.0.3 | custom | cdn.openttd.org | — | 🟢 standalone | — | ✅ | ✅ |
| [openttd](ports/openttd/README.md) | OpenTTD, an open source simulation game based on Transport Tycoon Deluxe | 15.0-git-20250501 | custom | github.com | 2 | 🔴 dependent | 7 | ✅ | ✅ |
| [opus](ports/opus/README.md) | Opus interactive speech and audio codec library | 1.5.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [opusfile](ports/opusfile/README.md) | High-level library for decoding and seeking in Ogg Opus files | 0.12 | autotools | downloads.xiph.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [orc](ports/orc/README.md) | Oil Runtime Compiler library for optimized inner loops (SIMD) | 0.4.40 | meson | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [orocos-kdl](ports/orocos-kdl/README.md) | Orocos Kinematics and Dynamics Library for robot kinematic chains | 1.5.1 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [oss-desktop](ports/oss-desktop/README.md) | Desktop session with labwc, Xfce panel, terminal and file manager | 1.0 | custom | custom | — | 🟡 build-deps only | — | ✅ | ✅ |
| [p11-kit](ports/p11-kit/README.md) | Library and proxy module for loading and sharing PKCS#11 modules | 0.25.5 | meson | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [paho.mqtt.c](ports/paho.mqtt.c/README.md) | Eclipse Paho MQTT client library for C | 1.3.14 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [paho.mqtt.cpp](ports/paho.mqtt.cpp/README.md) | Eclipse Paho MQTT client library for C++ | 1.5.2 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [pandas](ports/pandas/README.md) | Python data analysis library with labeled data structures | 2.3.3 | custom | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [pango](ports/pango/README.md) | Library for layout and rendering of internationalized text | 1.54.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 9 | ✅ | ✅ |
| [patch](ports/patch/README.md) | GNU patch, applies diff files to original files | 2.8 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [pax-utils](ports/pax-utils/README.md) | ELF security and QA utilities (scanelf, lddtree) | 1.3.8 | meson | dev.gentoo.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [pcl](ports/pcl/README.md) | Point Cloud Library for 2D/3D point cloud processing | 1.14.1 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [pcre](ports/pcre/README.md) | Perl compatible regular expressions library (legacy PCRE1 API) | 8.45 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [pcre2](ports/pcre2/README.md) | Perl compatible regular expressions library (PCRE2 API) | 10.44 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pdfium](ports/pdfium/README.md) | PDFium, Chromium's PDF rendering library | 0-git-84a8011 | custom | pdfium.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [perl-error](ports/perl-error/README.md) | Error.pm, try/catch style exception handling for Perl | 0.17030 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-mozilla-ca](ports/perl-mozilla-ca/README.md) | Mozilla's CA certificate bundle for Perl (Mozilla::CA) | 20261020 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl](ports/perl/README.md) | Perl 5 programming language interpreter | 5.40.2 | custom | www.cpan.org | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [picocom](ports/picocom/README.md) | Minimal dumb-terminal emulator for serial ports | 3.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [pistache](ports/pistache/README.md) | C++ REST framework for HTTP servers and clients | 0.4.26 | meson | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [pixman](ports/pixman/README.md) | Low-level pixel manipulation library (compositing, rasterization) | 0.46.4 | meson | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pkgconf](ports/pkgconf/README.md) | Tool and library for managing compiler and linker flags (pkg-config) | 2.3.0 | meson | distfiles.ariadne.space | — | 🟢 standalone | — | ✅ | ✅ |
| [poppler](ports/poppler/README.md) | PDF rendering library and command line tools | 25.12.0 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 12 | ✅ | ✅ |
| [pqiv](ports/pqiv/README.md) | Minimal command line image viewer based on GTK 3 | 2.13.2 | custom | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [protobuf-c](ports/protobuf-c/README.md) | Protocol Buffers implementation in C (runtime library) | 1.5.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [protobuf](ports/protobuf/README.md) | Google's Protocol Buffers data serialization library and compiler | 29.3 | custom | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [pthread-stubs](ports/pthread-stubs/README.md) | pkg-config file for pthread symbols needed by X libraries | 0.5 | autotools | xorg.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [pthreadpool](ports/pthreadpool/README.md) | Portable thread pool library in C | 0-git-20250207 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pugixml](ports/pugixml/README.md) | Lightweight XML parser for C++ with XPath support | 1.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [python3-astor](ports/python3-astor/README.md) | Python module for reading, rewriting and writing Python ASTs as source | 0.8.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-async-generator](ports/python3-async-generator/README.md) | Python async generators and async context managers backport | 1.10 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-click](ports/python3-click/README.md) | Python package for creating composable command line interfaces | 8.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-coverage](ports/python3-coverage/README.md) | Code coverage measurement for Python | 7.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-curio](ports/python3-curio/README.md) | Coroutine-based Python library for concurrent systems programming | 1.6_git20251221 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-dateutil](ports/python3-dateutil/README.md) | Python extensions to the standard datetime module | 2.9.0.post0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-idna](ports/python3-idna/README.md) | Python support for internationalized domain names (IDNA 2008, UTS 46) | 3.20 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-iniconfig](ports/python3-iniconfig/README.md) | Minimal INI file parsing module for Python | 2.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-mypy-extensions](ports/python3-mypy-extensions/README.md) | Type system extensions for programs checked with mypy | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-packaging](ports/python3-packaging/README.md) | Core utilities for Python packages: versions, specifiers, markers | 25.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pip](ports/python3-pip/README.md) | Python package installer | 25.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pluggy](ports/python3-pluggy/README.md) | Plugin and hook calling mechanism for Python (used by pytest) | 1.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pretend](ports/python3-pretend/README.md) | Python library for creating test stubs | 1.0.9 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pygments](ports/python3-pygments/README.md) | Python syntax highlighter for many languages | 2.21.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-cov](ports/python3-pytest-cov/README.md) | pytest plugin for measuring code coverage | 7.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-tornasync](ports/python3-pytest-tornasync/README.md) | pytest plugin for testing tornado coroutines | 0.6.0.post2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest](ports/python3-pytest/README.md) | Python testing framework | 9.2.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytokens](ports/python3-pytokens/README.md) | Fast spec-compliant Python 3.12+ tokenizer | 0.4.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytz](ports/python3-pytz/README.md) | World timezone definitions for Python | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-setuptools](ports/python3-setuptools/README.md) | Python library and tools for building and distributing packages | 80.8.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-six](ports/python3-six/README.md) | Python 2 and 3 compatibility library | 1.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sniffio](ports/python3-sniffio/README.md) | Detects which async library a Python program is running under | 1.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sortedcontainers](ports/python3-sortedcontainers/README.md) | Sorted list, dict and set types in pure Python | 2.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tornado](ports/python3-tornado/README.md) | Python web framework and asynchronous networking library | 6.6 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-traitlets](ports/python3-traitlets/README.md) | Typed, validated and observable attributes for Python configuration | 5.16.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tzdata](ports/python3-tzdata/README.md) | IANA time zone database as a Python package | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-versioneer](ports/python3-versioneer/README.md) | Python tool for deriving version strings from version-control tags | 0.29 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-wheel](ports/python3-wheel/README.md) | Command line tool and library for the Python wheel package format | 0.46.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3](ports/python3/README.md) | Python 3 programming language interpreter and standard library | 3.14.0 | autotools | www.python.org | 2 | 🔴 dependent | 11 | ✅ | ✅ |
| [pytorch](ports/pytorch/README.md) | PyTorch tensor computation and deep learning library | 2.3.1 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [qt6](ports/qt6/README.md) | Qt 6 cross-platform application and UI framework | 6.8.1 | custom | download.qt.io | 1 | 🔴 dependent | 19 | ✅ | ✅ |
| [quickjs-ng](ports/quickjs-ng/README.md) | Small and embeddable JavaScript engine | 0.8.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [rapidjson](ports/rapidjson/README.md) | Header-only fast JSON parser and generator for C++ (SAX and DOM APIs) | 1.1.0-24b5e7a | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [re2](ports/re2/README.md) | Fast, safe, thread-friendly regular expression library | 2024-07-02 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [readline](ports/readline/README.md) | GNU Readline library for command line editing and history | 8.2 | autotools | ftp.gnu.org | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [restbed](ports/restbed/README.md) | C++ framework for building HTTP and HTTPS client and server applications | 4.8 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [retroarch](ports/retroarch/README.md) | RetroArch, the libretro frontend for emulators and games | 1.15-git-20230408 | custom | github.com | 4 | 🔴 dependent | 7 | ✅ | ✅ |
| [retropie](ports/retropie/README.md) | RetroPie retro gaming stack: RetroArch, EmulationStation, cores, configs | 0.3 | custom | custom | — | 🟡 build-deps only | — | ✅ | ✅ |
| [ros2](ports/ros2/README.md) | ROS 2 Jazzy core: robotics middleware, client libraries and tools | jazzy | custom | custom | — | 🔴 dependent | 14 | ✅ | ✅ |
| [rsync](ports/rsync/README.md) | Fast local and remote file copying tool using delta transfer | 3.3.0 | autotools | download.samba.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [rtl-sdr](ports/rtl-sdr/README.md) | Library and tools turning RTL2832 DVB dongles into SDR receivers | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [rust](ports/rust/README.md) | Rust cross toolchain: rustc and cargo for the host, std for the targets | 1.90.0 | custom | static.rust-lang.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ruy](ports/ruy/README.md) | Matrix multiplication library for neural network inference | 0-git-20240321 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [s2n-tls](ports/s2n-tls/README.md) | Small, fast C99 implementation of the TLS/SSL protocols from AWS | 1.7.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl2-compat](ports/sdl2-compat/README.md) | SDL2 compatibility layer implemented on top of SDL3 | 2.32.60 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl2-image](ports/sdl2-image/README.md) | Image loading library for SDL2 | 2.8.8 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [sdl2-net](ports/sdl2-net/README.md) | Portable network library for SDL2 | 2.2.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl2-ttf](ports/sdl2-ttf/README.md) | TrueType font rendering library for SDL2 using FreeType and HarfBuzz | 2.24.0 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [sdl2](ports/sdl2/README.md) | Simple DirectMedia Layer 2, cross-platform multimedia and input library | 2.32.10 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl3](ports/sdl3/README.md) | Simple DirectMedia Layer 3, cross-platform multimedia and input library | 3.4.18 | cmake | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [shared-mime-info](ports/shared-mime-info/README.md) | Freedesktop.org shared MIME type database and update-mime-database | 2.4 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [simdjson](ports/simdjson/README.md) | SIMD-accelerated JSON parsing library for C++ | 3.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [simdutf](ports/simdutf/README.md) | Fast SIMD Unicode validation and transcoding library | 6.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [sleef](ports/sleef/README.md) | SIMD library of vectorized elementary math functions and DFT | 3.8-git-20250214 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [snappy](ports/snappy/README.md) | Fast compression and decompression library | 1.2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [soxr](ports/soxr/README.md) | SoX Resampler, high quality one-dimensional sample-rate conversion library | 0.1.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [spdlog](ports/spdlog/README.md) | Fast C++ logging library | 1.15.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [speexdsp](ports/speexdsp/README.md) | Speech processing DSP library for the Speex codec | 1.2.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [sqlite](ports/sqlite/README.md) | SQLite embedded SQL database engine and command line shell | 3.47.2 | autotools | www.sqlite.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sqlite_orm](ports/sqlite_orm/README.md) | Header-only SQLite ORM library for modern C++ | 1.9 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sscep](ports/sscep/README.md) | Simple SCEP client for certificate enrollment | 0.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [stable-diffusion.cpp](ports/stable-diffusion.cpp/README.md) | Stable Diffusion and FLUX image generation inference in C/C++ | 0_git20260601 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [strongswan](ports/strongswan/README.md) | IPsec-based VPN solution | 6.0.6 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [sudo](ports/sudo/README.md) | Runs commands with the privileges of another user | 1.9.17p2 | autotools | www.sudo.ws | 3 | 🔴 dependent | 3 | ✅ | ✅ |
| [suitesparse](ports/suitesparse/README.md) | Suite of sparse matrix algorithms and factorization libraries | 7.11.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [svt-av1](ports/svt-av1/README.md) | Scalable Video Technology AV1 encoder library and tool | 2.3.0 | cmake | gitlab.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [swaybg](ports/swaybg/README.md) | Wallpaper utility for wlroots-based Wayland compositors | 1.2.2 | meson | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [sysbench](ports/sysbench/README.md) | Scriptable multi-threaded benchmark tool for CPU, memory, I/O and threads | 1.0.20-git-20250309 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [taglib](ports/taglib/README.md) | Library for reading and editing audio file metadata tags | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [tar](ports/tar/README.md) | GNU tar archiving utility | 1.35 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [tensorflow-lite](ports/tensorflow-lite/README.md) | TensorFlow Lite C/C++ runtime for machine learning inference | 2.16.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [tftp-hpa](ports/tftp-hpa/README.md) | TFTP client and server | 5.2 | custom | git.kernel.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [thorvg](ports/thorvg/README.md) | Lightweight library for rendering vector graphics, SVG and Lottie | 0.15.8 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [thunar](ports/thunar/README.md) | Thunar, the Xfce file manager, and its extension library | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 18 | ✅ | ✅ |
| [tinyxml2](ports/tinyxml2/README.md) | Small C++ XML parser | 10.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tmux](ports/tmux/README.md) | Terminal multiplexer | 3.5a | autotools | github.com | 3 | 🔴 dependent | 2 | ✅ | ✅ |
| [tomlplusplus](ports/tomlplusplus/README.md) | Header-only TOML configuration file parser and serializer for C++17 | 3.4.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-bash](ports/tree-sitter-bash/README.md) | Bash grammar for tree-sitter | 0.25.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-c](ports/tree-sitter-c/README.md) | C grammar for tree-sitter | 0.24.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-cmake](ports/tree-sitter-cmake/README.md) | CMake grammar for tree-sitter | 0.7.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-lua](ports/tree-sitter-lua/README.md) | Lua grammar for tree-sitter | 0.4.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-make](ports/tree-sitter-make/README.md) | Makefile grammar for tree-sitter | 1.1.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-markdown](ports/tree-sitter-markdown/README.md) | Markdown and inline Markdown grammars for tree-sitter | 0.5.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-python](ports/tree-sitter-python/README.md) | Python grammar for tree-sitter | 0.23.6 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-query](ports/tree-sitter-query/README.md) | Tree-sitter query file grammar for tree-sitter | 0.7.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-vim](ports/tree-sitter-vim/README.md) | Vimscript grammar for tree-sitter | 0.7.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-vimdoc](ports/tree-sitter-vimdoc/README.md) | Vim help file grammar for tree-sitter | 4.0.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter](ports/tree-sitter/README.md) | Incremental parsing library for programming tools | 0.25.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree](ports/tree/README.md) | Displays directory contents as an indented tree | 2.2.1 | custom | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [trompeloeil](ports/trompeloeil/README.md) | Header-only C++14 mocking framework | 48 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tumbler](ports/tumbler/README.md) | D-Bus thumbnailing service for Xfce applications | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 13 | ✅ | ✅ |
| [unbound](ports/unbound/README.md) | Validating, recursive and caching DNS resolver and library | 1.24.1 | autotools | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [unibilium](ports/unibilium/README.md) | Basic terminfo library for reading terminfo files and format strings | 2.1.2 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [urdfdom_headers](ports/urdfdom_headers/README.md) | Header-only C++ data structures for the Unified Robot Description Format | 1.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [uriparser](ports/uriparser/README.md) | Strictly RFC 3986 compliant URI parsing and handling library | 0.9.8 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [utf8proc](ports/utf8proc/README.md) | C library for Unicode normalization, case-folding and UTF-8 processing | 2.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [vidstab](ports/vidstab/README.md) | Video stabilization library for FFmpeg and Transcode | 1.1.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [vim](ports/vim/README.md) | Vi IMproved, a highly configurable text editor | 9.1.0900 | autotools | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [virtualgl](ports/virtualgl/README.md) | VirtualGL client (vglclient) for displaying remotely rendered OpenGL | 3.1.5 | cmake | github.com | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [vlc](ports/vlc/README.md) | VLC media player and libvlc multimedia engine | 3.0.21 | custom | code.videolan.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [vsomeip](ports/vsomeip/README.md) | COVESA SOME/IP protocol stack for automotive communication | 3.5.6 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [vte](ports/vte/README.md) | Virtual terminal widget library for GTK | 0.78.2 | meson | download.gnome.org | 5 | 🔴 dependent | 14 | ✅ | ✅ |
| [wayland-protocols](ports/wayland-protocols/README.md) | Extension protocol definitions for Wayland (XML files) | 1.47 | meson | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [wayland](ports/wayland/README.md) | Wayland display protocol client and server libraries | 1.24.0 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [webkitgtk](ports/webkitgtk/README.md) | WebKitGTK web content engine for GTK applications | 2.44.4 | cmake | webkitgtk.org | 5 | 🔴 dependent | 32 | ✅ | ✅ |
| [webrtc](ports/webrtc/README.md) | WebRTC native C++ library for real-time audio, video and data | m132-afaf497 | custom | webrtc.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [weston](ports/weston/README.md) | Weston, the reference Wayland compositor | 13.0.3 | — | gitlab.freedesktop.org | — | ⛔ blocked | — | — | — |
| [wget](ports/wget/README.md) | GNU Wget, non-interactive HTTP, HTTPS and FTP downloader | 1.25.0 | autotools | ftp.gnu.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [whisper.cpp](ports/whisper.cpp/README.md) | OpenAI Whisper speech recognition inference in C/C++ | 0_git20260728 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [wlr-protocols](ports/wlr-protocols/README.md) | Wayland protocol extensions from wlroots (XML files) | 0-git-20250702 | custom | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [wlroots](ports/wlroots/README.md) | Modular library for building Wayland compositors | 0.20.2 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [woff2](ports/woff2/README.md) | Library and tools for WOFF2 font compression and decompression | 1.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [wolfssl](ports/wolfssl/README.md) | Lightweight embedded SSL/TLS library in C | 5.7.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [x264](ports/x264/README.md) | H.264/MPEG-4 AVC video encoder library | stable | custom | code.videolan.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [x265](ports/x265/README.md) | H.265/HEVC video encoder library | 3.6 | cmake | bitbucket.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [xcb-proto](ports/xcb-proto/README.md) | XML-XCB protocol descriptions used to generate libxcb | 1.17.0 | autotools | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xcb-util](ports/xcb-util/README.md) | XCB utility libraries: atom cache, events and convenience functions | 0.4.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [xdg-utils](ports/xdg-utils/README.md) | Desktop integration scripts such as xdg-open | 1.2.1 | autotools | gitlab.freedesktop.org | 2 | 🟢 standalone | — | ✅ | ✅ |
| [xfce4-appfinder](ports/xfce4-appfinder/README.md) | Xfce application finder and launcher | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [xfce4-panel](ports/xfce4-panel/README.md) | Xfce panel with launchers, menus and workspace switcher | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 17 | ✅ | ✅ |
| [xfce4-session](ports/xfce4-session/README.md) | Xfce session manager | 4.20.2 | autotools | archive.xfce.org | 2 | 🔴 dependent | 11 | ✅ | ✅ |
| [xfce4-settings](ports/xfce4-settings/README.md) | Xfce settings daemon, manager and editor | 4.20.1 | autotools | archive.xfce.org | — | 🔴 dependent | 16 | ✅ | ✅ |
| [xfce4-terminal](ports/xfce4-terminal/README.md) | Xfce terminal emulator | 1.1.5 | meson | archive.xfce.org | 2 | 🔴 dependent | 12 | ✅ | ✅ |
| [xfconf](ports/xfconf/README.md) | Hierarchical configuration storage system for Xfce | 4.20.0 | autotools | archive.xfce.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [xkbcomp](ports/xkbcomp/README.md) | XKB keymap compiler | 1.5.0 | meson | xorg.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [xkeyboard-config](ports/xkeyboard-config/README.md) | XKB keyboard configuration data: rules, layouts, symbols | 2.43 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xnnpack](ports/xnnpack/README.md) | Optimized neural network inference operators for ARM and x86 | 0-git-20250209 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [xorgproto](ports/xorgproto/README.md) | X Window System core and extension protocol headers | 2024.1 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xtrans](ports/xtrans/README.md) | Header-only X network transport layer code shared by X libraries | 1.5.2 | autotools | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xvidcore](ports/xvidcore/README.md) | Xvid MPEG-4 Part 2 video codec library | 1.3.7 | custom | downloads.xvid.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xwayland](ports/xwayland/README.md) | X server running as a Wayland client (Xwayland) | 24.1.14 | meson | gitlab.freedesktop.org | 5 | 🔴 dependent | 6 | ✅ | ✅ |
| [xxhash](ports/xxhash/README.md) | Fast non-cryptographic hash algorithm library | 0.8.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xz](ports/xz/README.md) | XZ Utils, LZMA compression library and tools | 5.6.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [yaml-cpp](ports/yaml-cpp/README.md) | YAML parser and emitter for C++ | 0.8.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zenoh-c](ports/zenoh-c/README.md) | C bindings for Eclipse Zenoh pub/sub, storage and query protocol | 1.2.1 | custom | github.com | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [zimg](ports/zimg/README.md) | Library for image scaling, colorspace conversion and dithering | 3.0.5 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [zlib-ng](ports/zlib-ng/README.md) | zlib data compression library for next generation systems, native API | 2.2.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zlib](ports/zlib/README.md) | General purpose data compression library | 1.3.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zsh](ports/zsh/README.md) | Z shell | 5.9 | autotools | downloads.sourceforge.net | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [zstd](ports/zstd/README.md) | Fast real-time lossless compression library (Zstandard) | 1.5.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |

