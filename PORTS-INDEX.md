# Component index

916 components. **914** build for both `aarch64le` and `x86_64` from upstream source, carrying 529 QNX patches between them.

| Class | | Components |
|---|---|---|
| 🟢 **standalone** | builds with the QNX SDP only, runs on a stock target | **322** |
| 🟡 **build-deps only** | needs other components to build, runs on its own | **231** |
| 🔴 **dependent** | needs other components to build and to run | **362** |
| 📋 **planned** | recorded for porting, not ported yet | **0** |
| ⛔ **blocked** | tried; cannot be built for QNX as things stand (the recipe says why) | **1** |

How the class is decided, and every component's full chain: [DEPENDENCIES.md](DEPENDENCIES.md).

Click a component for its source URL, its patches, the exact build commands and how it is verified on a target.

| Component | What it is | Version | Build | Upstream | Patches | Class | Run-time deps | aarch64le | x86_64 |
|---|---|---|---|---|---|---|---|---|---|
| [7zip](ports/7zip/README.md) | File archiver with a high compression ratio (7-Zip command line, 7zz) | 26.05 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [abseil](ports/abseil/README.md) | Collection of C++ libraries augmenting the C++ standard library | 20260817.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [abuild](ports/abuild/README.md) | Alpine's tools for building APK packages | 3.17.0 | custom | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [acado](ports/acado/README.md) | ACADO Toolkit: optimal control, MPC/MHE and embedded solver code generation | 1.2.2_beta | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [ada](ports/ada/README.md) | Fast WHATWG-compliant URL parser library for C++ | 4.0.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-fonts](ports/adwaita-fonts/README.md) | Adwaita Sans and Adwaita Mono, the GNOME fonts | 49.0 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [adwaita-icon-theme](ports/adwaita-icon-theme/README.md) | GNOME's Adwaita icon theme and default cursors | 51.0 | meson | download.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [amqp-cpp](ports/amqp-cpp/README.md) | C++ library for communicating with a RabbitMQ message broker | 4.3.27 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [aom](ports/aom/README.md) | AV1 video codec library from the Alliance for Open Media | 3.15.3 | cmake | aomedia.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [apk-tools](ports/apk-tools/README.md) | Alpine Package Keeper (apk), the Alpine package manager | 3.0.8 | meson | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [appstream](ports/appstream/README.md) | Tools and libraries to work with AppStream metadata | 1.0.6 | meson | www.freedesktop.org | 3 | 🔴 dependent | 7 | ✅ | ✅ |
| [apr-util](ports/apr-util/README.md) | Apache Portable Runtime utility library | 1.6.5 | autotools | archive.apache.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [apr](ports/apr/README.md) | Apache Portable Runtime, a portable C API for system services | 1.7.6 | autotools | archive.apache.org | — | 🟢 standalone | — | ✅ | ✅ |
| [apriltag](ports/apriltag/README.md) | AprilTag visual fiducial detection library (tag families 36h11, 25h9, 16h5, Standard, Circle) | 3.4.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [armadillo](ports/armadillo/README.md) | C++ linear algebra library with a MATLAB-like API (on OpenBLAS) | 15.6.2 | cmake | downloads.sourceforge.net | — | 🔴 dependent | 1 | ✅ | ✅ |
| [asciidoc](ports/asciidoc/README.md) | Text document format for short documents, articles, books and UNIX man pages | 10.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [asio](ports/asio/README.md) | Header-only C++ library for asynchronous network and low-level I/O | 1-38-2 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [aspell](ports/aspell/README.md) | GNU Aspell spell checker and library | 0.60.8.1 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [assimp](ports/assimp/README.md) | Open Asset Import Library: loads 3D model formats (OBJ, STL, COLLADA, glTF, FBX, ...) | 6.0.2 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [at-spi2-core](ports/at-spi2-core/README.md) | Accessibility infrastructure: AT-SPI D-Bus interfaces, libatspi and ATK | 2.54.2 | meson | download.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [autoconf](ports/autoconf/README.md) | GNU tool for generating configure scripts from M4 macros | 2.72 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [automake](ports/automake/README.md) | GNU tool for generating portable Makefile.in files | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [avahi](ports/avahi/README.md) | Multicast/unicast DNS-SD framework (mDNS daemon, client library and tools) | 0.9-rc3 | autotools | github.com | 4 | 🔴 dependent | 3 | ✅ | ✅ |
| [avro-cpp](ports/avro-cpp/README.md) | C++ library for Apache Avro, a data serialization system | 1.12.2 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
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
| [bash-completion](ports/bash-completion/README.md) | Programmable command-line completion for bash | 2.18.0 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [bash](ports/bash/README.md) | GNU Bourne Again shell | 5.2.37 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [bc](ports/bc/README.md) | GNU bc, an arbitrary-precision calculator language | 1.08.2 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [benchmark](ports/benchmark/README.md) | Google's C++ library for benchmarking code snippets | 1.9.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [binutils](ports/binutils/README.md) | GNU binary tools: assembler, linker, objdump, readelf and others | 2.43 | autotools | ftp.gnu.org | 10 | 🔴 dependent | 2 | ✅ | ✅ |
| [bison](ports/bison/README.md) | The GNU general-purpose parser generator | 3.8.2 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [boost](ports/boost/README.md) | Boost peer-reviewed portable C++ source libraries | 1.86.0 | custom | archives.boost.io | 4 | 🔴 dependent | 3 | ✅ | ✅ |
| [boringssl](ports/boringssl/README.md) | Fork of OpenSSL designed to meet Google's needs (libcrypto, libssl, bssl) | 0.20260929.0 | cmake | github.com | 4 | 🟢 standalone | — | ✅ | ✅ |
| [brotli](ports/brotli/README.md) | Generic lossless compression library and tool using the Brotli format | 1.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [btop](ports/btop/README.md) | Terminal resource monitor for CPU, memory, disks, network and processes | 1.4.8 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [bullet3](ports/bullet3/README.md) | Bullet real-time collision detection and multi-physics simulation library | 3.25 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [busybox](ports/busybox/README.md) | Tiny versions of many common UNIX utilities in a single executable | 1.36.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [bzip2](ports/bzip2/README.md) | Block-sorting file compressor and libbz2 library | 1.0.8 | custom | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [c-ares](ports/c-ares/README.md) | Asynchronous DNS resolver library in C | 1.34.3 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [c-blosc](ports/c-blosc/README.md) | Blocking, shuffling and lossless compression library | 1.21.6 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [ca-certificates](ports/ca-certificates/README.md) | Mozilla's CA certificate bundle for TLS verification | 20261020 | custom | curl.se | — | 🟢 standalone | — | ✅ | ✅ |
| [cairo](ports/cairo/README.md) | 2D vector graphics library with multiple output backends | 1.18.2 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [can-utils](ports/can-utils/README.md) | CAN log converters (asc2log, log2asc, log2long), can-calc-bit-timing | 2025.01 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [candb](ports/candb/README.md) | Library for parsing CAN bus database description formats | 0.1-git-20210625 | cmake | github.com | 5 | 🟡 build-deps only | — | ✅ | ✅ |
| [capicxx-core-runtime](ports/capicxx-core-runtime/README.md) | COVESA CommonAPI C++ core runtime for IPC interfaces | 3.2.4 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [capicxx-someip-runtime](ports/capicxx-someip-runtime/README.md) | COVESA CommonAPI C++ SOME/IP binding runtime | 3.2.4 | cmake | github.com | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [capnproto](ports/capnproto/README.md) | Cap'n Proto data serialization format and capability-based RPC system | 1.0.2 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [capstone](ports/capstone/README.md) | Multi-architecture disassembly framework | 5.0.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cartographer](ports/cartographer/README.md) | Real-time 2D and 3D SLAM library across multiple platforms and sensors | 2.0.0 | cmake | github.com | 3 | 🔴 dependent | 8 | ✅ | ✅ |
| [casadi](ports/casadi/README.md) | CasADi: symbolic framework for nonlinear optimisation and algorithmic differentiation | 3.8.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [catch2](ports/catch2/README.md) | C++ unit testing framework with micro-benchmarking and BDD macros | 3.7.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ccache](ports/ccache/README.md) | Compiler cache: speeds up recompilation by caching object files keyed on a hash of the input | 4.12.1 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [cereal](ports/cereal/README.md) | Header-only C++11 library for serialization | 1.3.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ceres-solver](ports/ceres-solver/README.md) | C++ library for modeling and solving large optimization problems | 2.2.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [cgal](ports/cgal/README.md) | CGAL computational geometry library (header-only): hulls, triangulations, meshes, exact predicates | 6.2.1 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [check](ports/check/README.md) | Unit testing framework for C | 0.15.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [chrony](ports/chrony/README.md) | NTP client and server for synchronizing the system clock | 4.8-git-20260226 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [chrpath](ports/chrpath/README.md) | Lists, changes or deletes the RPATH/RUNPATH of ELF binaries | 0.18 | autotools | codeberg.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [civetweb](ports/civetweb/README.md) | Embeddable C/C++ web server with optional Lua and WebSocket support | 1.16 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [cjson](ports/cjson/README.md) | Lightweight JSON parser in ANSI C | 1.7.18 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [clapack](ports/clapack/README.md) | LAPACK linear algebra routines translated to C with f2c | 3.2.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [cli11](ports/cli11/README.md) | Header-only command line parser for C++11 and later | 2.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cmake](ports/cmake/README.md) | Cross-platform build system generator | 4.1.1 | cmake | gitlab.kitware.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [compiler-rt](ports/compiler-rt/README.md) | LLVM compiler runtime: builtins and profiling libraries | 22.1.8 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [computelibrary](ports/computelibrary/README.md) | Arm Compute Library of optimized NEON/SVE kernels for Arm CPUs | 53.0.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | n/a |
| [console_bridge](ports/console_bridge/README.md) | ROS-independent C++ logging library mirroring rosconsole calls | 1.0.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cppzmq](ports/cppzmq/README.md) | Header-only C++ binding for the ZeroMQ messaging library | 4.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [cpuinfo](ports/cpuinfo/README.md) | C library for detecting CPU features relevant to performance tuning | 0-git-20240417 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [csmith](ports/csmith/README.md) | Random generator of C programs for testing compilers | 2.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cunit](ports/cunit/README.md) | Unit testing framework for C | 2.1-3 | custom | downloads.sourceforge.net | 1 | 🟢 standalone | — | ✅ | ✅ |
| [cups-filters](ports/cups-filters/README.md) | OpenPrinting CUPS filters | 1.28.17 | autotools | github.com | 2 | 🔴 dependent | 17 | ✅ | ✅ |
| [cups](ports/cups/README.md) | Standards-based printing system: scheduler, libcups and command-line tools | 2.4.16 | custom | github.com | 2 | 🔴 dependent | 5 | ✅ | ✅ |
| [curl](ports/curl/README.md) | Command line tool and library for transferring data with URLs | 8.11.1 | cmake | github.com | — | 🔴 dependent | 6 | ✅ | ✅ |
| [cxxopts](ports/cxxopts/README.md) | Header-only C++ command line option parser | 3.3.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [cyclonedds](ports/cyclonedds/README.md) | Eclipse Cyclone DDS, an implementation of the OMG DDS standard | 0.10.5 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [cython](ports/cython/README.md) | Cython: compiles Python and Cython code to C extension modules | 3.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [datefudge](ports/datefudge/README.md) | Runs a program with a faked current time (LD_PRELOAD library and wrapper) | 1.24 | custom | deb.debian.org | 2 | 🟢 standalone | — | ✅ | ✅ |
| [dav1d](ports/dav1d/README.md) | Fast cross-platform AV1 video decoder library | 1.5.0 | meson | code.videolan.org | — | 🟢 standalone | — | ✅ | ✅ |
| [davici](ports/davici/README.md) | Decoupled Asynchronous VICI client library for strongSwan | 1.6 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [dbus-glib](ports/dbus-glib/README.md) | Legacy GLib bindings for D-Bus | 0.116 | autotools | dbus.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [dbus](ports/dbus/README.md) | D-Bus message bus daemon and library for interprocess communication | 1.16.0 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [desktop-file-utils](ports/desktop-file-utils/README.md) | Command line utilities for validating and installing .desktop files | 0.28 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [dhcpcd](ports/dhcpcd/README.md) | DHCP, DHCPv6 and IPv4LL (ZeroConf) client | 10.5.2 | custom | github.com | 5 | 🟢 standalone | — | ✅ | ✅ |
| [dlt-daemon](ports/dlt-daemon/README.md) | COVESA Diagnostic Log and Trace (DLT) daemon, library and tools | 2.18.10 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [dnsmasq](ports/dnsmasq/README.md) | Lightweight DNS forwarder and DHCP server | 2.93test4 | custom | thekelleys.org.uk | 1 | 🟢 standalone | — | ✅ | ✅ |
| [dnssec-root](ports/dnssec-root/README.md) | DNSSEC root zone trust anchors (the root KSKs) from IANA | 20241105 | custom | data.iana.org | — | 🟢 standalone | — | ✅ | ✅ |
| [docbook-xml](ports/docbook-xml/README.md) | DocBook XML DTDs 4.1.2 to 4.5 with XML catalogs, for validating and transforming DocBook documents offline | 4.5 | custom | archive.docbook.org | — | 🟢 standalone | — | ✅ | ✅ |
| [docbook-xsl](ports/docbook-xsl/README.md) | DocBook XSL stylesheets for converting DocBook XML to HTML and man pages | 1.79.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [doctest](ports/doctest/README.md) | Single-header C++ unit testing framework | 2.5.3 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [double-conversion](ports/double-conversion/README.md) | C++ routines for binary-decimal conversion of IEEE doubles | 3.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [doxygen](ports/doxygen/README.md) | Documentation generator for C, C++ and other languages | 1.17.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [duktape](ports/duktape/README.md) | Small embeddable ECMAScript (JavaScript) engine with a C API | 2.7.0 | custom | duktape.org | — | 🟢 standalone | — | ✅ | ✅ |
| [dump1090](ports/dump1090/README.md) | Mode S (ADS-B) aircraft transponder decoder for RTL-SDR devices | 0-git-20240121 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [e2fsprogs](ports/e2fsprogs/README.md) | Ext2/3/4 filesystem utilities | 1.47.4 | autotools | mirrors.edge.kernel.org | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [ed](ports/ed/README.md) | GNU ed, the POSIX line-oriented text editor | 1.22.5 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [eigen](ports/eigen/README.md) | Header-only C++ template library for linear algebra | 3.4.0 | cmake | gitlab.com | — | 🟢 standalone | — | ✅ | ✅ |
| [elfutils](ports/elfutils/README.md) | Library to read and write ELF files (libelf from elfutils) | 0.193 | autotools | sourceware.org | 4 | 🔴 dependent | 1 | ✅ | ✅ |
| [elvis](ports/elvis/README.md) | Clone of the ex/vi text editor | 2.2.1-git-20240226 | custom | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [emacs](ports/emacs/README.md) | GNU Emacs extensible text editor (terminal build) | 30.2 | custom | ftp.gnu.org | 4 | 🔴 dependent | 9 | ✅ | ✅ |
| [emulationstation](ports/emulationstation/README.md) | Graphical front-end for browsing and launching emulated games | 2.11-git-20241212 | cmake | github.com | 2 | 🔴 dependent | 6 | ✅ | ✅ |
| [enca](ports/enca/README.md) | Detects and converts the character encoding of text files | 1.19 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [enchant](ports/enchant/README.md) | Generic spell checking library wrapping several spell checker backends | 2.8.2 | autotools | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [encodings](ports/encodings/README.md) | X.Org font encoding tables for libfontenc | 1.1.0 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [ensmallen](ports/ensmallen/README.md) | Header-only C++ numerical optimisation library on Armadillo | 3.11.1 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [epiphany](ports/epiphany/README.md) | GNOME Web, a WebKitGTK-based web browser | 43.1 | meson | download.gnome.org | 5 | 🔴 dependent | 20 | ✅ | ✅ |
| [exiv2](ports/exiv2/README.md) | C++ library and tool for reading and writing Exif, IPTC and XMP metadata | 0.28.5 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [exo](ports/exo/README.md) | Extension library and helper applications for the Xfce desktop | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [expat](ports/expat/README.md) | Stream-oriented XML parser library in C | 2.6.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fakeroot](ports/fakeroot/README.md) | Runs a command in an environment faking root privileges for packaging | 1.37.1.1 | autotools | deb.debian.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [farmhash](ports/farmhash/README.md) | Google's FarmHash family of hash functions for strings | 1.1-git-20190513 | autotools | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [fast-float](ports/fast-float/README.md) | Fast and exact implementation of from_chars for numbers | 8.0.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fastcdr](ports/fastcdr/README.md) | eProsima Fast CDR, C++ serialization library for the CDR format | 2.2.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fastdds](ports/fastdds/README.md) | eProsima Fast DDS, C++ implementation of the OMG DDS standard | 2.14.4 | cmake | github.com | 2 | 🔴 dependent | 4 | ✅ | ✅ |
| [fastfetch](ports/fastfetch/README.md) | Like neofetch, but much faster because written in C | 2.66.1 | cmake | github.com | 6 | 🟢 standalone | — | ✅ | ✅ |
| [fcft](ports/fcft/README.md) | Font loading and glyph rasterization library on FontConfig, FreeType and pixman | 3.3.2 | meson | codeberg.org | 2 | 🔴 dependent | 5 | ✅ | ✅ |
| [fcl](ports/fcl/README.md) | Flexible Collision Library for collision and proximity queries | 0.7.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fdk-aac](ports/fdk-aac/README.md) | Fraunhofer FDK AAC audio codec library | 2.0.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ffmpeg](ports/ffmpeg/README.md) | Libraries and tools for decoding, encoding and processing audio and video | 7.1.1 | custom | github.com | — | 🔴 dependent | 12 | ✅ | ✅ |
| [ffmpegthumbnailer](ports/ffmpegthumbnailer/README.md) | Lightweight video thumbnailer library and tool using FFmpeg | 2.2.3 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [fft2d](ports/fft2d/README.md) | General purpose 1D/2D/3D FFT, DCT and DST package (Ooura) | 2006.12.28 | custom | www.kurims.kyoto-u.ac.jp | — | 🟢 standalone | — | ✅ | ✅ |
| [fftw](ports/fftw/README.md) | Discrete Fourier transform (FFT) library, double and single precision | 3.3.10 | autotools | www.fftw.org | — | 🟢 standalone | — | ✅ | ✅ |
| [file](ports/file/README.md) | File type identification tool and libmagic library | 5.46 | autotools | astron.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [fio](ports/fio/README.md) | Flexible I/O tester: storage benchmark and data verification with job files | 3.41 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [flac](ports/flac/README.md) | Free Lossless Audio Codec library and tools | 1.4.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flann](ports/flann/README.md) | Fast Library for Approximate Nearest Neighbors search | 1.9.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [flatbuffers](ports/flatbuffers/README.md) | Memory-efficient serialization library and flatc schema compiler | 24.3.25 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [flex](ports/flex/README.md) | Fast lexical analyser generator (flex) and its libfl | 2.6.4 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [fmt](ports/fmt/README.md) | Fast and safe C++ formatting library | 11.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [folly](ports/folly/README.md) | Open-source C++ library developed and used at Facebook | 2026.10.26.00 | cmake | github.com | 6 | 🔴 dependent | 12 | ✅ | ✅ |
| [font-cantarell](ports/font-cantarell/README.md) | Cantarell, the GNOME user interface font | 0.303.1 | custom | download.gnome.org | — | 🟢 standalone | — | ✅ | ✅ |
| [font-dejavu](ports/font-dejavu/README.md) | DejaVu font family (sans, serif, mono) | 2.37 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-hack](ports/font-hack/README.md) | Hack, a monospace font for source code | 3.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-inconsolata](ports/font-inconsolata/README.md) | Inconsolata, a monospace font for code and terminals (OpenType) | 3.000 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-inter](ports/font-inter/README.md) | Inter, a sans-serif font family for user interfaces | 4.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-liberation](ports/font-liberation/README.md) | Liberation fonts, metric-compatible with Arial, Times New Roman, Courier | 2.1.5 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-emoji](ports/font-noto-emoji/README.md) | Noto Color Emoji font | 2.048 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-sans-cjk](ports/font-noto-sans-cjk/README.md) | Noto Sans CJK fonts for Chinese, Japanese and Korean | 2.004 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-noto-serif-cjk](ports/font-noto-serif-cjk/README.md) | Noto Serif CJK fonts for Chinese, Japanese and Korean | 2.003 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [font-util](ports/font-util/README.md) | X.Org font utilities and autoconf macros for building font packages | 1.4.1 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [fontconfig](ports/fontconfig/README.md) | Library for font configuration, discovery and customization | 2.15.0 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fontenc](ports/fontenc/README.md) | X.Org font encoding library | 1.1.8 | autotools | gitlab.freedesktop.org | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [foonathan_memory](ports/foonathan_memory/README.md) | C++ memory allocator library improving on the STL allocator model | 0.7-3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [foot](ports/foot/README.md) | Fast, lightweight and minimalistic Wayland terminal emulator | 1.23.1 | meson | codeberg.org | 6 | 🔴 dependent | 6 | ✅ | ✅ |
| [fp16](ports/fp16/README.md) | Header-only C library for converting to and from half-precision floats | 0-git-20260620 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [freeimage](ports/freeimage/README.md) | Library for loading and saving popular image formats (PNG, JPEG, TIFF...) | 3.18.0 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [freetype](ports/freetype/README.md) | Portable font rendering engine library | 2.13.3 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [frei0r-plugins](ports/frei0r-plugins/README.md) | Minimalistic plugin API and collection of video sources and filters | 2.4.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [fribidi](ports/fribidi/README.md) | Implementation of the Unicode Bidirectional Algorithm | 1.0.16 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [fxdiv](ports/fxdiv/README.md) | Header-only C library for integer division via fixed-point multiplication | 0-git-20200417 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [g3log](ports/g3log/README.md) | Asynchronous crash-safe C++ logging library | 2.4 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [garcon](ports/garcon/README.md) | Freedesktop.org compliant menu library for Xfce | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 11 | ✅ | ✅ |
| [gawk](ports/gawk/README.md) | GNU awk pattern scanning and text processing language | 5.3.2 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [gcr](ports/gcr/README.md) | GNOME library for displaying certificates and accessing key stores | 3.41.2 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 8 | ✅ | ✅ |
| [gd](ports/gd/README.md) | Library for the dynamic creation of images (PNG, JPEG, GIF, WebP, TIFF, AVIF, HEIF) | 2.3.3 | cmake | github.com | 1 | 🔴 dependent | 9 | ✅ | ✅ |
| [gdb](ports/gdb/README.md) | GNU debugger | 14.2 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 10 | ✅ | ✅ |
| [gdbm](ports/gdbm/README.md) | GNU dbm key/value database library | 1.24 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gdk-pixbuf](ports/gdk-pixbuf/README.md) | GTK library for loading and scaling images | 2.42.12 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [geany-plugins](ports/geany-plugins/README.md) | Collection of plugins for the Geany IDE | 2.1 | autotools | plugins.geany.org | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [geany](ports/geany/README.md) | Fast and lightweight IDE using GTK | 2.1 | autotools | download.geany.org | 1 | 🔴 dependent | 9 | ✅ | ✅ |
| [gemmlowp](ports/gemmlowp/README.md) | Small self-contained low-precision (8-bit) GEMM library | 0-git-20231103 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [geographiclib](ports/geographiclib/README.md) | Small C++ library for geodesic and other geographic conversions | 2.7 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [geoip](ports/geoip/README.md) | Legacy MaxMind GeoIP library: look up countries and cities by IP address | 1.6.12 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gettext](ports/gettext/README.md) | GNU internationalization tools and libraries for translated messages | 0.22.5 | autotools | ftp.gnu.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [gflags](ports/gflags/README.md) | C++ library for command line flags processing | 2.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [giflib](ports/giflib/README.md) | Library and tools for reading and writing GIF images | 5.2.2 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [git](ports/git/README.md) | Fast distributed version control system | 2.51.0 | custom | mirrors.edge.kernel.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [gklib](ports/gklib/README.md) | Helper routines library used by METIS and related software | 5.1.1-git-20230326 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [glfw](ports/glfw/README.md) | Multi-platform library for OpenGL, OpenGL ES and Vulkan windowing | 3.5.1 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [glib-networking](ports/glib-networking/README.md) | TLS (GnuTLS) and proxy support modules for GLib's GIO | 2.80.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [glib-typelibs](ports/glib-typelibs/README.md) | GObject introspection data (typelibs and GIR files) of the ported GLib, for PyGObject | 2.82.4 | custom | gitlab.gnome.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [glib](ports/glib/README.md) | Low-level core C library of GTK and GNOME: data types, main loop, GIO | 2.82.4 | meson | gitlab.gnome.org | 3 | 🔴 dependent | 5 | ✅ | ✅ |
| [glm](ports/glm/README.md) | Header-only C++ mathematics library for graphics based on GLSL | 1.0.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [glog](ports/glog/README.md) | Google's C++ application-level logging library | 0.7.1 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [gloo](ports/gloo/README.md) | Gloo collective communications library (allreduce, broadcast, barrier) with the TCP transport | 0_git20261104 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [glslang](ports/glslang/README.md) | Khronos reference front end for GLSL, ESSL and HLSL, and SPIR-V generator | 1.4.321.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [gmp](ports/gmp/README.md) | GNU Multiple Precision arithmetic library | 6.3.0 | autotools | gmplib.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gnome-keyring](ports/gnome-keyring/README.md) | GNOME Keyring: the session secret store daemon (org.freedesktop.secrets) | 51.1 | meson | download.gnome.org | 2 | 🔴 dependent | 4 | ✅ | ✅ |
| [gnupg](ports/gnupg/README.md) | GNU Privacy Guard 2: OpenPGP and S/MIME encryption and signing tools | 2.4.9 | autotools | gnupg.org | 1 | 🔴 dependent | 12 | ✅ | ✅ |
| [gnutls](ports/gnutls/README.md) | GNU TLS/SSL protocol implementation library | 3.8.8 | autotools | www.gnupg.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [gobject-introspection](ports/gobject-introspection/README.md) | GObject Introspection library and tools for API metadata | 1.86.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [godot](ports/godot/README.md) | Godot game engine runtime (export templates) for 2D and 3D games | 4.4.1 | custom | github.com | 1 | 🔴 dependent | 17 | ✅ | ✅ |
| [googletest](ports/googletest/README.md) | Google's C++ testing and mocking framework (GoogleTest, GoogleMock) | 1.17.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [gperf](ports/gperf/README.md) | GNU perfect hash function generator | 3.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gpgme](ports/gpgme/README.md) | GnuPG Made Easy | 2.2.0 | autotools | gnupg.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [graphene](ports/graphene/README.md) | Thin layer of math types (vectors, matrices, points) for graphics libraries | 1.10.8 | meson | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [graphicsmagick](ports/graphicsmagick/README.md) | Image processing tools and libraries (gm, Magick++) | 1.3.46 | autotools | downloads.sourceforge.net | — | 🔴 dependent | 13 | ✅ | ✅ |
| [graphite2](ports/graphite2/README.md) | Font rendering engine for complex scripts (SIL Graphite smart fonts) | 1.3.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [graphviz](ports/graphviz/README.md) | Graph visualization tools: dot, neato and other layout engines | 13.1.2 | cmake | gitlab.com | 2 | 🔴 dependent | 13 | ✅ | ✅ |
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
| [gumbo-parser](ports/gumbo-parser/README.md) | HTML5 parsing library in pure C99 | 0.13.2 | meson | codeberg.org | — | 🟢 standalone | — | ✅ | ✅ |
| [gyp](ports/gyp/README.md) | GYP meta-build system: generates Makefiles and Ninja files from .gyp files | 0.22.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [hailort](ports/hailort/README.md) | Light-weight, high-performance inference framework for Hailo devices | 4.14.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [harfbuzz](ports/harfbuzz/README.md) | OpenType text shaping engine | 10.1.0 | meson | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [help2man](ports/help2man/README.md) | Generates man pages from a program's --help and --version output | 1.49.3 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [hicolor-icon-theme](ports/hicolor-icon-theme/README.md) | Freedesktop.org default fallback icon theme | 0.18 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [highway](ports/highway/README.md) | Performance-portable C++ SIMD/vector intrinsics library | 1.2.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [highwayhash](ports/highwayhash/README.md) | Fast strong hash functions: SipHash/HighwayHash | 0_git20240418 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [hiredis](ports/hiredis/README.md) | Minimalistic C client library for Redis and Valkey | 1.5.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [htop](ports/htop/README.md) | Interactive process viewer | 3.5.0 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [http-parser](ports/http-parser/README.md) | HTTP request/response parser library in C (from Node.js) | 2.9.4 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [hunspell](ports/hunspell/README.md) | Hunspell spell checker and morphological analyzer library and program | 1.7.2 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [hwdata](ports/hwdata/README.md) | Hardware identification databases (pci.ids, usb.ids, pnp.ids, OUI) | 0.413 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [hyphen](ports/hyphen/README.md) | Hyphenation library using converted TeX hyphenation patterns | 2.8.8 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [iceoryx](ports/iceoryx/README.md) | Eclipse iceoryx, true zero-copy inter-process communication middleware | 2.0.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [icon-naming-utils](ports/icon-naming-utils/README.md) | Maps legacy GNOME/KDE icon names to the freedesktop.org icon naming specification | 0.8.90 | autotools | tango.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [icu](ports/icu/README.md) | International Components for Unicode: Unicode and locale support libraries | 76.1 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [imath](ports/imath/README.md) | C++ library of 2D/3D vector, matrix and half-float math for computer graphics | 3.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [inetutils](ports/inetutils/README.md) | A collection of common network programs (GNU inetutils) | 2.6 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [intltool](ports/intltool/README.md) | Translation tools for non-C files: intltool-merge, -extract, -update, intltoolize | 0.51.0 | autotools | launchpad.net | — | 🟢 standalone | — | ✅ | ✅ |
| [iperf2](ports/iperf2/README.md) | Tool for measuring network bandwidth, TCP and UDP performance (iperf 2) | 2.2.1 | autotools | downloads.sourceforge.net | 1 | 🟢 standalone | — | ✅ | ✅ |
| [iperf3](ports/iperf3/README.md) | TCP, UDP and SCTP network bandwidth measurement tool | 3.17.1 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [ippusbxd](ports/ippusbxd/README.md) | Daemon for IPP over USB printer support | 1.34 | cmake | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
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
| [lanelet2](ports/lanelet2/README.md) | Lanelet2 HD map framework for automated driving: maps, OSM I/O, projection, traffic rules, routing | 1.2.3 | custom | custom | — | 🔴 dependent | 3 | ✅ | ✅ |
| [lcm](ports/lcm/README.md) | Lightweight Communications and Marshalling: message passing for real time | 1.5.2 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [lcms2](ports/lcms2/README.md) | Little CMS, small-footprint ICC color management engine | 2.16 | meson | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lely-core](ports/lely-core/README.md) | Lely C/C++ libraries for CANopen, CAN, event loop and I/O | 2.3.4 | autotools | gitlab.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [less](ports/less/README.md) | Terminal pager for viewing text files one screen at a time | 668 | autotools | www.greenwoodsoftware.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [leveldb](ports/leveldb/README.md) | Fast ordered key-value storage library from Google | 1.23 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libadwaita](ports/libadwaita/README.md) | Building blocks for modern GNOME applications | 1.6.10 | meson | download.gnome.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libaec](ports/libaec/README.md) | Adaptive Entropy Coding library for lossless compression, SZIP compatible | 1.1.7 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libarchive](ports/libarchive/README.md) | Library and tools (bsdtar, bsdcpio) for reading and writing archives | 3.7.7 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libass](ports/libass/README.md) | Subtitle renderer library for the ASS/SSA subtitle format | 0.17.3 | autotools | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [libassuan](ports/libassuan/README.md) | IPC library used by GnuPG and related software | 3.0.2 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libasyncns](ports/libasyncns/README.md) | Asynchronous name service query library | 0.8 | autotools | 0pointer.de | — | 🟢 standalone | — | ✅ | ✅ |
| [libavif](ports/libavif/README.md) | Library for encoding and decoding AVIF images | 1.1.1 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libb2](ports/libb2/README.md) | BLAKE2 cryptographic hash library (BLAKE2b, BLAKE2s, BLAKE2bp, BLAKE2sp) | 0.98.1 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libcaca](ports/libcaca/README.md) | Graphics library that outputs text instead of pixels | 0.99.beta20 | autotools | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libcamera](ports/libcamera/README.md) | Raspberry Pi camera image processing algorithms from libcamera | 0.5.1+rpt20250722 | custom | github.com | 2 | 🔴 dependent | 1 | ✅ | n/a |
| [libcbor](ports/libcbor/README.md) | C library for parsing and generating CBOR binary data | 0.11.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libccd](ports/libccd/README.md) | Library for collision detection between convex shapes | 2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libcoap](ports/libcoap/README.md) | C implementation of the Constrained Application Protocol (CoAP) | 4.3.5 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libconfig](ports/libconfig/README.md) | C/C++ library for processing structured configuration files | 1.8.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdaemon](ports/libdaemon/README.md) | Lightweight C library that eases the writing of UNIX daemons | 0.14 | autotools | 0pointer.de | 2 | 🟢 standalone | — | ✅ | ✅ |
| [libdatrie](ports/libdatrie/README.md) | Double-array trie data structure library | 0.2.13 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdazzle](ports/libdazzle/README.md) | Companion library of extra widgets and utilities for GObject and GTK 3 | 3.44.0 | meson | gitlab.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [libde265](ports/libde265/README.md) | H.265/HEVC video decoder library | 1.0.15 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdecor](ports/libdecor/README.md) | Client-side decorations library for Wayland clients | 0.2.4 | meson | gitlab.freedesktop.org | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [libdeflate](ports/libdeflate/README.md) | Fast library for DEFLATE, zlib and gzip compression and decompression | 1.22 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libdisplay-info](ports/libdisplay-info/README.md) | EDID and DisplayID parsing library | 0.3.0 | meson | gitlab.freedesktop.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libdrm-headers](ports/libdrm-headers/README.md) | Headers for the DRM kernel interface and the libdrm API (headers only) | 2.4.129 | custom | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libedit](ports/libedit/README.md) | Command line editor library: line editing, history and tokenization | 20240808-3.1 | autotools | thrysoee.dk | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libepoxy](ports/libepoxy/README.md) | Library for managing OpenGL, EGL and GLX function pointers | 1.5.10 | meson | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [libev](ports/libev/README.md) | Full-featured event loop library for C | 4.33 | autotools | dist.schmorp.de | — | 🟢 standalone | — | ✅ | ✅ |
| [libevent](ports/libevent/README.md) | Asynchronous event notification library | 2.1.12 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libexif](ports/libexif/README.md) | Library for parsing, editing and saving EXIF data | 0.6.24 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libfdt](ports/libfdt/README.md) | Device Tree Compiler (dtc) and libfdt for flat device trees | 1.7.2 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libffi](ports/libffi/README.md) | Portable foreign function interface library | 3.4.6 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libfyaml](ports/libfyaml/README.md) | YAML 1.2 parser and emitter library in C, with the fy-tool program | 0.9.6 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [libgcrypt](ports/libgcrypt/README.md) | GNU general purpose cryptographic library | 1.11.0 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgit2](ports/libgit2/README.md) | Portable C library implementing the Git core methods | 1.8.4 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [libgme](ports/libgme/README.md) | Video game music file emulation and playback library | 0.6.4 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libgpg-error](ports/libgpg-error/README.md) | Common error values and runtime library for GnuPG components | 1.50 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libgsf](ports/libgsf/README.md) | Library for reading and writing structured files (MS OLE, Zip) | 1.14.53 | autotools | gitlab.gnome.org | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libhandy](ports/libhandy/README.md) | GTK 3 widgets for adaptive and mobile user interfaces | 1.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [libheif](ports/libheif/README.md) | HEIF and AVIF image file format decoder and encoder | 1.20.2 | cmake | github.com | — | 🔴 dependent | 12 | ✅ | ✅ |
| [libhevc](ports/libhevc/README.md) | Android's HEVC (H.265) video decoder and encoder libraries | 1.8.5 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [libice](ports/libice/README.md) | X Inter-Client Exchange (ICE) protocol library | 1.1.2 | autotools | xorg.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [libiconv](ports/libiconv/README.md) | GNU character set conversion library (iconv) | 1.18 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libidn2](ports/libidn2/README.md) | Library for internationalized domain names (IDNA2008, Punycode, TR46) | 2.3.7 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libinih](ports/libinih/README.md) | Simple INI file parser library for C and C++ | 58 | meson | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjpeg-turbo](ports/libjpeg-turbo/README.md) | SIMD-accelerated JPEG codec library and tools (libjpeg, TurboJPEG APIs) | 3.0.4 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjson](ports/libjson/README.md) | Small streaming JSON parser and printer in C | 0.8-git-20260512 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libjxl](ports/libjxl/README.md) | JPEG XL image format reference library and tools | 0.11.1 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libksba](ports/libksba/README.md) | Library to access X.509 certificates and CMS data | 1.8.1 | autotools | gnupg.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libmd](ports/libmd/README.md) | Message digest functions from BSD systems (MD5, SHA-1, SHA-2, RIPEMD) | 1.1.0 | autotools | libbsd.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmetal](ports/libmetal/README.md) | Abstraction library for device access, interrupts and shared memory | 2024.10.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [libmicrohttpd](ports/libmicrohttpd/README.md) | GNU C library for embedding an HTTP server | 1.0.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libmodbus](ports/libmodbus/README.md) | Library for communicating with Modbus devices over serial or TCP | 3.1.11 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libnotify](ports/libnotify/README.md) | Library for sending desktop notifications | 0.8.3 | meson | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libogg](ports/libogg/README.md) | Library for the Ogg multimedia bitstream container format | 1.3.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libpanel](ports/libpanel/README.md) | Dock/panel library for GTK 4 | 1.10.4 | meson | download.gnome.org | — | 🔴 dependent | 6 | ✅ | ✅ |
| [libpaper](ports/libpaper/README.md) | Paper size library and the paper command | 2.3.0 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libpcap](ports/libpcap/README.md) | Portable network traffic capture library | 1.10.5 | autotools | www.tcpdump.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpipeline](ports/libpipeline/README.md) | C library for setting up and running pipelines of processes | 1.5.8 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libpng](ports/libpng/README.md) | C library for reading and writing PNG images | 1.6.44 | autotools | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libportal](ports/libportal/README.md) | GIO-style client library for XDG desktop portals | 0.8.1 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libproxy](ports/libproxy/README.md) | Library for automatic proxy configuration management | 0.5.9 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libpsl](ports/libpsl/README.md) | C library for handling the Public Suffix List | 0.21.5 | meson | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libraqm](ports/libraqm/README.md) | Library for complex text layout (bidirectional text, shaping) | 0.10.5 | meson | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libretro-2048](ports/libretro-2048/README.md) | 2048 puzzle game as a libretro core for RetroArch | 0-git-20241227 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-fake08](ports/libretro-fake08/README.md) | fake-08 PICO-8 emulator as a libretro core for RetroArch | 0-git-20260613 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-mrboom](ports/libretro-mrboom/README.md) | Mr.Boom, an 8-player Bomberman clone, as a libretro core | 0-git-20260625 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libretro-retro8](ports/libretro-retro8/README.md) | retro8 PICO-8 fantasy console emulator as a libretro core | 0-git-20260912 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libretro-samples](ports/libretro-samples/README.md) | libretro sample cores: test patterns and input/audio tests | 0-git-20221110 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsamplerate](ports/libsamplerate/README.md) | Audio sample rate conversion library (Secret Rabbit Code) | 0.2.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsass](ports/libsass/README.md) | LibSass, a C/C++ Sass (SCSS) to CSS compiler library | 3.6.6 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libsecret](ports/libsecret/README.md) | GObject library for storing and retrieving secrets via Secret Service | 0.21.4 | meson | gitlab.gnome.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libsfdo](ports/libsfdo/README.md) | Libraries implementing freedesktop.org desktop entry, icon and basedir specs | 0.1.4 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsm](ports/libsm/README.md) | X Session Management library | 1.2.6 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libsndfile](ports/libsndfile/README.md) | C library for reading and writing sampled audio files | 1.2.2 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [libsodium](ports/libsodium/README.md) | Easy-to-use library for encryption, signatures and password hashing | 1.0.20 | autotools | download.libsodium.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libsoup](ports/libsoup/README.md) | GLib-based HTTP client and server library | 3.6.1 | meson | gitlab.gnome.org | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libssh](ports/libssh/README.md) | C library implementing the SSH client and server protocols | 0.11.1 | cmake | www.libssh.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libssh2](ports/libssh2/README.md) | Client-side C library implementing the SSH2 protocol | 1.11.1 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [libtasn1](ports/libtasn1/README.md) | GNU library for ASN.1 and DER manipulation | 4.19.0 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libtermkey](ports/libtermkey/README.md) | Keyboard entry processing library for terminal programs | 0.22 | custom | www.leonerd.org.uk | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libthai](ports/libthai/README.md) | Thai language support library (word breaking, input, sorting) | 0.1.29 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [libtheora](ports/libtheora/README.md) | Theora video codec library | 1.1.1 | autotools | downloads.xiph.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libtiff](ports/libtiff/README.md) | Library for reading and writing TIFF images | 4.7.0 | cmake | gitlab.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [libtomcrypt](ports/libtomcrypt/README.md) | Public-domain modular cryptographic toolkit written in C | 1.18.2 | custom | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libtommath](ports/libtommath/README.md) | Multiple-precision integer library written in portable C | 1.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libtool](ports/libtool/README.md) | GNU libltdl, a portable dynamic module loading library | 2.5.4 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunibreak](ports/libunibreak/README.md) | Implementation of Unicode line, word and grapheme breaking algorithms | 6.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libunistring](ports/libunistring/README.md) | GNU library for manipulating Unicode strings | 1.3 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libunwind](ports/libunwind/README.md) | Library for determining and unwinding the call chain of a program | 1.8.3 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libusb](ports/libusb/README.md) | C library for accessing USB devices from user space | 1.0.27 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libutempter](ports/libutempter/README.md) | Privileged helper and library for utmp/wtmp updates by terminal emulators | 1.2.3 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libuuid](ports/libuuid/README.md) | Library for generating and handling UUIDs (from util-linux) | 2.42.4 | autotools | mirrors.edge.kernel.org | — | 🟢 standalone | — | ✅ | ✅ |
| [libuv](ports/libuv/README.md) | Multi-platform support library for asynchronous I/O | 1.51.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [libvalkey](ports/libvalkey/README.md) | Valkey client library in C (with cluster and TLS support) | 0.5.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libvncserver](ports/libvncserver/README.md) | C libraries for implementing VNC servers and clients | 0.9.15 | cmake | github.com | — | 🔴 dependent | 7 | ✅ | ✅ |
| [libvorbis](ports/libvorbis/README.md) | Ogg Vorbis audio codec libraries | 1.3.7 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libvpx](ports/libvpx/README.md) | WebM VP8/VP9 video codec library and tools | 1.15.0 | custom | chromium.googlesource.com | — | 🟢 standalone | — | ✅ | ✅ |
| [libvterm](ports/libvterm/README.md) | Abstract VT220/xterm/ECMA-48 terminal emulator library | 0.3.3 | custom | www.leonerd.org.uk | 1 | 🟢 standalone | — | ✅ | ✅ |
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
| [libxml2-utils](ports/libxml2-utils/README.md) | xmllint and xmlcatalog, the command-line tools of libxml2 | 2.13.5 | cmake | gitlab.gnome.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [libxml2](ports/libxml2/README.md) | XML parser and toolkit library in C | 2.13.5 | cmake | gitlab.gnome.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [libxmlb](ports/libxmlb/README.md) | Library to help create and query binary XML blobs | 0.3.24 | meson | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxmu](ports/libxmu/README.md) | X miscellaneous utility routines library | 1.3.1 | autotools | xorg.freedesktop.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [libxo](ports/libxo/README.md) | Library for generating text, XML, JSON, and HTML output | 2.3.0 | autotools | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
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
| [lighttpd](ports/lighttpd/README.md) | Lightweight, flexible web server | 1.4.77 | cmake | git.lighttpd.net | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [linux-headers-input-codes](ports/linux-headers-input-codes/README.md) | Linux input event codes header (input-event-codes.h), header only | 6.12 | custom | git.kernel.org | — | 🟢 standalone | — | ✅ | ✅ |
| [litehtml](ports/litehtml/README.md) | Fast and lightweight HTML/CSS rendering engine | 0.9 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [llama.cpp](ports/llama.cpp/README.md) | LLM inference in C/C++ | 0_b9006 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [lldpd](ports/lldpd/README.md) | Implementation of IEEE 802.1ab (LLDP) | 1.0.22 | autotools | github.com | 8 | 🔴 dependent | 1 | ✅ | ✅ |
| [llvm](ports/llvm/README.md) | LLVM compiler infrastructure with the Clang compiler and LLD linker | 22.1.8 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [lmbench](ports/lmbench/README.md) | Suite of micro-benchmarks for operating system and hardware performance | 3.0-git-20260929 | custom | github.com | 5 | 🟢 standalone | — | ✅ | ✅ |
| [log4cpp](ports/log4cpp/README.md) | C++ logging library modelled on log4j | 1.1.4 | autotools | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [log4cxx](ports/log4cxx/README.md) | C++ logging framework modeled after Apache log4j | 1.8.0 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [lua-compat53](ports/lua-compat53/README.md) | Lua 5.3-style standard library functions for Lua 5.1, 5.2 and LuaJIT | 0.14.4 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lua-lpeg](ports/lua-lpeg/README.md) | Parsing Expression Grammars (LPeg) module for Lua | 1.1.0 | custom | www.inf.puc-rio.br | — | 🟡 build-deps only | — | ✅ | ✅ |
| [lua](ports/lua/README.md) | Lua 5.4 scripting language interpreter and library | 5.4.7 | custom | www.lua.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lua5.1](ports/lua5.1/README.md) | Lua 5.1 scripting language interpreter and library | 5.1.5 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lua5.2](ports/lua5.2/README.md) | Lua 5.2 scripting language interpreter and library | 5.2.4 | custom | www.lua.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lua5.3](ports/lua5.3/README.md) | Lua 5.3 scripting language interpreter and library | 5.3.6 | custom | www.lua.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [luajit](ports/luajit/README.md) | Just-in-time compiler for the Lua programming language | 2.1.c6ffc141 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [luarocks](ports/luarocks/README.md) | LuaRocks, the package manager for Lua modules (rocks) | 3.12.2 | custom | luarocks.org | — | 🟢 standalone | — | ✅ | ✅ |
| [luv](ports/luv/README.md) | libuv bindings for Lua and LuaJIT | 1.51.0-1 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [lxde-icon-theme](ports/lxde-icon-theme/README.md) | LXDE (nuoveXT) icon theme | 0.5.2 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [lynx](ports/lynx/README.md) | Text-mode web browser | 2.9.2 | autotools | invisible-mirror.net | 1 | 🔴 dependent | 7 | ✅ | ✅ |
| [lz4](ports/lz4/README.md) | Fast lossless compression library and tool (LZ4) | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [lzip](ports/lzip/README.md) | Lossless data compressor based on LZMA, with integrity checking | 1.26 | autotools | download.savannah.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [lzo](ports/lzo/README.md) | LZO real-time data compression library | 2.10 | autotools | www.oberhumer.com | — | 🟢 standalone | — | ✅ | ✅ |
| [m4](ports/m4/README.md) | GNU implementation of the Unix m4 macro processor | 1.4.19 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [magic_enum](ports/magic_enum/README.md) | Header-only C++17 library for static reflection of enums | 0.9.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [make](ports/make/README.md) | GNU make build automation tool | 4.4.1 | autotools | ftp.gnu.org | — | 🟢 standalone | — | ✅ | ✅ |
| [mame](ports/mame/README.md) | MAME multi-purpose arcade and computer emulator | 0.281 | custom | github.com | 1 | 🔴 dependent | 5 | ✅ | ✅ |
| [man-db](ports/man-db/README.md) | Manual page reader suite (man, apropos, whatis) | 2.13.1 | autotools | download.savannah.gnu.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [mate-calc](ports/mate-calc/README.md) | MATE desktop calculator | 1.28.0 | autotools | pub.mate-desktop.org | 3 | 🔴 dependent | 13 | ✅ | ✅ |
| [mate-common](ports/mate-common/README.md) | Autoconf macros and bootstrap scripts for MATE desktop modules | 1.28.0 | autotools | pub.mate-desktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [mbedtls](ports/mbedtls/README.md) | Small C library for cryptography, X.509 and TLS/DTLS | 3.6.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mcap-cpp](ports/mcap-cpp/README.md) | Header-only C++ library for the MCAP robotics message log format | 2.1.3 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [md4c](ports/md4c/README.md) | CommonMark Markdown parser in C, with Markdown-to-HTML library and md2html | 0.6.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mesa](ports/mesa/README.md) | Mesa 3D: software OpenGL, GLES, EGL (llvmpipe) and Vulkan (lavapipe) | 26.2.6 | meson | archive.mesa3d.org | 6 | 🔴 dependent | 5 | ✅ | ✅ |
| [meson](ports/meson/README.md) | Meson build system | 1.10.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [metis](ports/metis/README.md) | Programs and library for partitioning graphs and finite element meshes | 5.2.1 | cmake | github.com | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [mimalloc](ports/mimalloc/README.md) | Compact general-purpose memory allocator from Microsoft | 3.1.5 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [minicom](ports/minicom/README.md) | Menu-driven serial communication program and terminal emulator | 2.9 | autotools | salsa.debian.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [miniz](ports/miniz/README.md) | Single-file zlib/Deflate compression library with ZIP archive support | 3.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mkfontscale](ports/mkfontscale/README.md) | Creates fonts.scale and fonts.dir index files for X11 font directories | 1.2.3 | autotools | www.x.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [mlpack](ports/mlpack/README.md) | C++ machine learning library and command-line programs (on Armadillo) | 4.8.0 | cmake | github.com | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [mnn](ports/mnn/README.md) | Lightweight deep learning inference engine from Alibaba | 3.6.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mosh](ports/mosh/README.md) | Mobile shell that supports roaming and intelligent local echo | 1.4.0 | autotools | github.com | 1 | 🔴 dependent | 6 | ✅ | ✅ |
| [mosquitto](ports/mosquitto/README.md) | Eclipse Mosquitto MQTT broker, client libraries and pub/sub utilities | 2.0.20 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpc](ports/mpc/README.md) | GNU MPC library for complex number arithmetic with arbitrary precision | 1.3.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [mpfr](ports/mpfr/README.md) | C library for multiple-precision floating-point computations | 4.2.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [mpg123](ports/mpg123/README.md) | MPEG audio (MP3) decoder library and signal synthesis library | 1.32.9 | autotools | www.mpg123.de | — | 🟢 standalone | — | ✅ | ✅ |
| [msgpack-c](ports/msgpack-c/README.md) | MessagePack binary object serialization library for C | 6.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [mujoco](ports/mujoco/README.md) | MuJoCo physics engine for multi-joint dynamics with contact | 3.10.0 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [muslflt](ports/muslflt/README.md) | musl's libm and float conversion routines as a separate library | 1.0-git-20240802 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [mythes](ports/mythes/README.md) | MyThes thesaurus library (OpenOffice/LibreOffice thesaurus format) | 1.2.4 | autotools | downloads.sourceforge.net | — | 🟡 build-deps only | — | ✅ | ✅ |
| [nano](ports/nano/README.md) | GNU nano, a small and simple terminal text editor | 8.2 | autotools | www.nano-editor.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [nanoflann](ports/nanoflann/README.md) | Header-only C++ KD-tree library for nearest-neighbour search | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nanomsg](ports/nanomsg/README.md) | Socket library that provides several common communication patterns | 1.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nanosvg](ports/nanosvg/README.md) | Header-only SVG parser and rasterizer | 0-git-20260709 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nasm](ports/nasm/README.md) | Netwide Assembler: x86 and x86-64 assembler and disassembler (ndisasm) | 2.16.03 | custom | www.nasm.us | — | 🟢 standalone | — | ✅ | ✅ |
| [ncnn](ports/ncnn/README.md) | Neural network inference framework for mobile and embedded devices | 20260526 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ncurses](ports/ncurses/README.md) | Terminal handling and text user interface library | 6.5 | autotools | ftp.gnu.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [neon](ports/neon/README.md) | HTTP and WebDAV client library, with a C interface | 0.37.1 | autotools | notroj.github.io | — | 🔴 dependent | 3 | ✅ | ✅ |
| [neovim](ports/neovim/README.md) | Vim-based extensible text editor | 0.12.3 | custom | github.com | 4 | 🔴 dependent | 6 | ✅ | ✅ |
| [netmap](ports/netmap/README.md) | netmap apps (pkt-gen, bridge, lb, nmreplay, tlem) for io-sock's netmap | 14.0-git-20261027 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [nettle](ports/nettle/README.md) | Low-level cryptographic library | 3.10.1 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [nghttp2](ports/nghttp2/README.md) | HTTP/2 protocol implementation library in C | 1.64.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [nginx](ports/nginx/README.md) | HTTP web server, reverse proxy, load balancer and TCP/UDP/mail proxy | 1.30.6 | custom | nginx.org | 2 | 🔴 dependent | 4 | ✅ | ✅ |
| [ninja](ports/ninja/README.md) | Small build system with a focus on speed | 1.13.1 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [nlohmann_json](ports/nlohmann_json/README.md) | Header-only JSON library for modern C++ | 3.12.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [nng](ports/nng/README.md) | nanomsg-next-gen lightweight messaging library (pub/sub, req/rep and more) | 1.9.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [nodejs](ports/nodejs/README.md) | Node.js cross-platform JavaScript runtime environment | 24.14.1 | custom | github.com | 9 | 🔴 dependent | 12 | ✅ | ✅ |
| [npm](ports/npm/README.md) | Package manager for JavaScript and Node.js | 11.11.0 | custom | registry.npmjs.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [npth](ports/npth/README.md) | New GNU Portable Threads library | 1.8 | autotools | gnupg.org | — | 🟢 standalone | — | ✅ | ✅ |
| [nspr](ports/nspr/README.md) | Netscape Portable Runtime | 4.39 | autotools | ftp.mozilla.org | 3 | 🟢 standalone | — | ✅ | ✅ |
| [nss](ports/nss/README.md) | Mozilla Network Security Services | 3.131.1 | custom | ftp.mozilla.org | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [nsync](ports/nsync/README.md) | C library of synchronization primitives (locks, condition variables) | 1.30.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ntp](ports/ntp/README.md) | Network Time Protocol reference implementation: ntpd, ntpq, ntpdate | 4.2.8p18 | autotools | downloads.nwtime.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [ntpsec](ports/ntpsec/README.md) | Hardened implementation of the Network Time Protocol | 1.2.4 | custom | gitlab.com | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [ntttcp-for-linux](ports/ntttcp-for-linux/README.md) | Multi-threaded Linux network throughput benchmark (NTTTCP for Linux) | 1.4.0-git-20230613 | cmake | github.com | 4 | 🟢 standalone | — | ✅ | ✅ |
| [numpy](ports/numpy/README.md) | Fundamental package for array computing in Python | 2.3.4 | custom | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [nuspell](ports/nuspell/README.md) | Nuspell spell checking library and program for Hunspell dictionaries | 5.1.9 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [octomap](ports/octomap/README.md) | Probabilistic 3D mapping framework based on octrees | 1.10.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [ompl](ports/ompl/README.md) | Open Motion Planning Library: sampling-based motion planners (RRT, PRM, KPIECE, BIT*) | 1.7.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [onednn](ports/onednn/README.md) | oneAPI Deep Neural Network Library | 3.14.1 | cmake | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [onetbb](ports/onetbb/README.md) | oneAPI Threading Building Blocks, C++ library for parallel programming | 2022.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [oniguruma](ports/oniguruma/README.md) | Regular expression library | 6.9.9 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [onnx](ports/onnx/README.md) | Open Neural Network Exchange (ONNX) format library for AI models | 1.17.0 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [onnxruntime](ports/onnxruntime/README.md) | Cross-platform machine learning inference engine for ONNX models | 1.20.1 | custom | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [open-amp](ports/open-amp/README.md) | OpenAMP framework for asymmetric multiprocessing (remoteproc, RPMsg) | 2024.10.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [open62541](ports/open62541/README.md) | C implementation of OPC UA (IEC 62541) clients and servers | 1.4.8 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openal-soft](ports/openal-soft/README.md) | Cross-platform 3D audio library implementing the OpenAL API | 1.24.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [openblas](ports/openblas/README.md) | Optimized BLAS linear algebra library based on GotoBLAS2 | 0.3.28 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [opencv](ports/opencv/README.md) | Open Source Computer Vision library | 4.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openexr](ports/openexr/README.md) | High dynamic-range (HDR) image file format library and tools | 3.4.1 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [openh264](ports/openh264/README.md) | H.264 video encoding and decoding library for real-time use | 2.5.0 | meson | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [openjpeg](ports/openjpeg/README.md) | JPEG 2000 image codec library and tools | 2.5.3 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [openjph](ports/openjph/README.md) | HTJ2K (High-Throughput JPEG 2000) encoder and decoder library | 0.24.1 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [openmp](ports/openmp/README.md) | LLVM OpenMP runtime library (libomp) | 22.1.8 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [openpam](ports/openpam/README.md) | BSD-licensed implementation of the Pluggable Authentication Modules API | 20250531 | autotools | git.des.dev | — | 🟢 standalone | — | ✅ | ✅ |
| [openresolv](ports/openresolv/README.md) | resolvconf: manages resolv.conf from several DNS information sources | 3.17.4 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
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
| [osqp](ports/osqp/README.md) | OSQP: operator splitting solver for convex quadratic programs (with QDLDL) | 0.6.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
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
| [perl-b-cow](ports/perl-b-cow/README.md) | B::COW, Perl's copy-on-write string internals (is a scalar COW, its refcount) | 0.007 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-class-inspector](ports/perl-class-inspector/README.md) | Class::Inspector, information about loaded Perl classes and their methods | 1.36 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-clone](ports/perl-clone/README.md) | Clone, recursive deep copy of Perl data structures (XS) | 0.51 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-cpan-requirements-dynamic](ports/perl-cpan-requirements-dynamic/README.md) | CPAN::Requirements::Dynamic, dynamic prerequisites for Perl distributions (used by Module::Build::Tiny) | 0.003 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-encode-locale](ports/perl-encode-locale/README.md) | Encode::Locale, the encodings of the current locale and console for Perl | 1.05 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-error](ports/perl-error/README.md) | Error.pm, try/catch style exception handling for Perl | 0.17030 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-extutils-config](ports/perl-extutils-config/README.md) | ExtUtils::Config, a wrapper for Perl's configuration (used by Perl module builders) | 0.010 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-extutils-hascompiler](ports/perl-extutils-hascompiler/README.md) | ExtUtils::HasCompiler, checks whether Perl can compile and load an XS module | 0.025 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-extutils-helpers](ports/perl-extutils-helpers/README.md) | ExtUtils::Helpers, portable file and argument helpers for Perl module builders | 0.028 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-extutils-installpaths](ports/perl-extutils-installpaths/README.md) | ExtUtils::InstallPaths, where a Perl distribution's files are installed | 0.015 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-fcgi](ports/perl-fcgi/README.md) | FCGI, the FastCGI application interface for Perl (XS, bundles the fcgi client library) | 0.82 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-file-listing](ports/perl-file-listing/README.md) | File::Listing, parses directory listings (ls -l, FTP LIST output) for Perl | 6.16 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-file-sharedir-install](ports/perl-file-sharedir-install/README.md) | File::ShareDir::Install, installs a Perl distribution's shared data files (a Makefile.PL helper) | 0.14 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-file-sharedir](ports/perl-file-sharedir/README.md) | File::ShareDir, locates the per-distribution shared data files of Perl modules | 1.118 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-html-parser](ports/perl-html-parser/README.md) | HTML::Parser, an event-driven HTML parser for Perl (XS), with HTML::Entities and HTML::LinkExtor | 3.86 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-html-tagset](ports/perl-html-tagset/README.md) | HTML::Tagset, data tables about HTML elements for Perl HTML parsers | 3.24 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-cookiejar](ports/perl-http-cookiejar/README.md) | HTTP::CookieJar, a minimalist RFC 6265 HTTP cookie jar for Perl | 0.014 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-cookies](ports/perl-http-cookies/README.md) | HTTP::Cookies, the HTTP cookie jar of libwww-perl (LWP) | 6.12 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-daemon](ports/perl-http-daemon/README.md) | HTTP::Daemon, a simple HTTP/1.1 server class for Perl | 6.17 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-date](ports/perl-http-date/README.md) | HTTP::Date, date conversion routines for HTTP (RFC 1123/850, ISO 8601) in Perl | 6.08 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-message](ports/perl-http-message/README.md) | HTTP::Message, HTTP request, response and header objects for Perl (libwww-perl) | 7.04 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-http-negotiate](ports/perl-http-negotiate/README.md) | HTTP::Negotiate, HTTP content negotiation (choose a variant by Accept headers) for Perl | 6.01 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-io-html](ports/perl-io-html/README.md) | IO::HTML, opens an HTML file with its encoding detected (BOM, <meta charset>) | 1.004 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-io-socket-ssl](ports/perl-io-socket-ssl/README.md) | IO::Socket::SSL, TLS sockets for Perl on top of Net::SSLeay (OpenSSL) | 2.099 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-io-tty](ports/perl-io-tty/README.md) | Perl IO::Tty and IO::Pty: pseudo-terminals and terminal constants | 1.31 | custom | cpan.metacpan.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-libwww](ports/perl-libwww/README.md) | libwww-perl (LWP), the World-Wide Web library for Perl: LWP::UserAgent and the lwp-* tools | 6.83 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-lwp-mediatypes](ports/perl-lwp-mediatypes/README.md) | LWP::MediaTypes, guesses the media type (MIME type) of a file or URL for Perl | 6.05 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-mime-base32](ports/perl-mime-base32/README.md) | MIME::Base32, Base32 encoding (RFC 4648) for Perl (URI::otpauth uses it) | 1.303 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-module-build-tiny](ports/perl-module-build-tiny/README.md) | Module::Build::Tiny, a tiny Build.PL based build tool for Perl modules | 0.053 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-module-build](ports/perl-module-build/README.md) | Module::Build, the Build.PL based build and install system for Perl modules | 0.4234 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-mozilla-ca](ports/perl-mozilla-ca/README.md) | Mozilla's CA certificate bundle for Perl (Mozilla::CA) | 20261020 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-net-http](ports/perl-net-http/README.md) | Net::HTTP, the low-level HTTP/1.1 client connection class of libwww-perl | 6.24 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-net-ssleay](ports/perl-net-ssleay/README.md) | Net::SSLeay, Perl bindings to OpenSSL's libssl and libcrypto (XS) | 1.96 | custom | cpan.metacpan.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [perl-protocol-websocket](ports/perl-protocol-websocket/README.md) | Protocol::WebSocket, the WebSocket protocol (handshake and frames, RFC 6455) for Perl | 0.26 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-deep](ports/perl-test-deep/README.md) | Test::Deep, deep comparison of data structures with patterns, for Perl tests | 1.205 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-fatal](ports/perl-test-fatal/README.md) | Test::Fatal, captures exceptions in Perl tests | 0.019 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-leaktrace](ports/perl-test-leaktrace/README.md) | Test::LeakTrace, finds memory leaks (leaked SVs) in Perl code (XS) | 0.17 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-needs](ports/perl-test-needs/README.md) | Test::Needs, skips Perl tests when optional modules are missing | 0.002010 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-requires](ports/perl-test-requires/README.md) | Test::Requires, skips Perl tests when required modules are missing | 0.11 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-requiresinternet](ports/perl-test-requiresinternet/README.md) | Test::RequiresInternet, skips Perl tests when the network is not reachable | 0.05 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-test-warnings](ports/perl-test-warnings/README.md) | Test::Warnings, tests for (unexpected) warnings in Perl tests | 0.038 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-try-tiny](ports/perl-try-tiny/README.md) | Try::Tiny, minimal try/catch/finally with proper $@ handling for Perl | 0.32 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-uri](ports/perl-uri/README.md) | URI, uniform resource identifiers (parse, build, normalise, resolve) for Perl | 5.40 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-www-robotrules](ports/perl-www-robotrules/README.md) | WWW::RobotRules, parses robots.txt files for Perl web robots | 6.03 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl-xml-parser](ports/perl-xml-parser/README.md) | XML::Parser, the Perl interface to the Expat XML parser (XS) | 2.59 | custom | cpan.metacpan.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [perl-xml-simple](ports/perl-xml-simple/README.md) | XML::Simple, reads and writes XML as Perl data structures (on XML::Parser) | 2.25 | custom | cpan.metacpan.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [perl](ports/perl/README.md) | Perl 5 programming language interpreter | 5.40.2 | custom | www.cpan.org | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [picocom](ports/picocom/README.md) | Minimal dumb-terminal emulator for serial ports | 3.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [pinentry](ports/pinentry/README.md) | PIN and passphrase entry dialogs for GnuPG (curses and tty) | 1.3.3 | autotools | gnupg.org | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [pistache](ports/pistache/README.md) | C++ REST framework for HTTP servers and clients | 0.4.26 | meson | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [pixman](ports/pixman/README.md) | Low-level pixel manipulation library (compositing, rasterization) | 0.46.4 | meson | gitlab.freedesktop.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [pkgconf](ports/pkgconf/README.md) | Tool and library for managing compiler and linker flags (pkg-config) | 2.3.0 | meson | distfiles.ariadne.space | — | 🟢 standalone | — | ✅ | ✅ |
| [poco](ports/poco/README.md) | Cross-platform C++ libraries for network-based applications | 1.15.4 | cmake | github.com | — | 🔴 dependent | 5 | ✅ | ✅ |
| [poppler](ports/poppler/README.md) | PDF rendering library and command line tools | 25.12.0 | cmake | gitlab.freedesktop.org | — | 🔴 dependent | 12 | ✅ | ✅ |
| [portaudio](ports/portaudio/README.md) | Portable cross-platform library for real-time audio input and output | 19.7.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [pqiv](ports/pqiv/README.md) | Minimal command line image viewer based on GTK 3 | 2.13.2 | custom | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [protobuf-c](ports/protobuf-c/README.md) | Protocol Buffers implementation in C (runtime library) | 1.5.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [protobuf](ports/protobuf/README.md) | Google's Protocol Buffers data serialization library and compiler | 29.3 | custom | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [psutil](ports/psutil/README.md) | Cross-platform process and system utilities module for Python | 7.2.2 | custom | github.com | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [pthread-stubs](ports/pthread-stubs/README.md) | pkg-config file for pthread symbols needed by X libraries | 0.5 | autotools | xorg.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [pthreadpool](ports/pthreadpool/README.md) | Portable thread pool library in C | 0-git-20250207 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [ptpd](ports/ptpd/README.md) | Precision Time Protocol (IEEE 1588) daemon | 2.3.1 | autotools | github.com | 4 | 🟢 standalone | — | ✅ | ✅ |
| [publicsuffix-list](ports/publicsuffix-list/README.md) | Public Suffix List: the cross-vendor database of public domain suffixes | 20261031 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [pugixml](ports/pugixml/README.md) | Lightweight XML parser for C++ with XPath support | 1.14 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [pybind11](ports/pybind11/README.md) | Seamless operability between C++11 and Python (header-only) | 3.1.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [pygame](ports/pygame/README.md) | Python game development library on SDL2 | 2.6.1_git20251005 | custom | github.com | 2 | 🔴 dependent | 7 | ✅ | ✅ |
| [python3-absl-py](ports/python3-absl-py/README.md) | Abseil Python common libraries: command-line flags, logging, app and test helpers | 2.5.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-appdirs](ports/python3-appdirs/README.md) | The platform's user data directories (platformdirs' predecessor) | 1.4.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-argcomplete](ports/python3-argcomplete/README.md) | Bash tab completion for argparse | 3.7.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-astor](ports/python3-astor/README.md) | Python module for reading, rewriting and writing Python ASTs as source | 0.8.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-async-generator](ports/python3-async-generator/README.md) | Python async generators and async context managers backport | 1.10 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-attrs](ports/python3-attrs/README.md) | Classes without boilerplate | 26.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-beniget](ports/python3-beniget/README.md) | Static analysis of Python code: definition-use chains on a gast tree | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-boto3](ports/python3-boto3/README.md) | The AWS SDK for Python | 1.43.123 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-botocore](ports/python3-botocore/README.md) | Low-level, data-driven core of boto 3 | 1.43.123 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-bracex](ports/python3-bracex/README.md) | Bash-style brace expansion for Python | 3.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-build](ports/python3-build/README.md) | A simple PEP 517 package builder (pyproject-build) | 1.6.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cairo](ports/python3-cairo/README.md) | Python bindings for the cairo 2D graphics library | 1.29.2 | custom | files.pythonhosted.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [python3-calver](ports/python3-calver/README.md) | Calendar versioning for setuptools | 2025.10.20 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-catkin-pkg](ports/python3-catkin-pkg/README.md) | Library for catkin packages (package.xml parsing) | 1.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cattrs](ports/python3-cattrs/README.md) | Structuring and unstructuring of attrs classes and dataclasses | 26.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cffi](ports/python3-cffi/README.md) | C foreign function interface for Python (ABI and API mode, on libffi) | 2.1.1 | custom | files.pythonhosted.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [python3-click](ports/python3-click/README.md) | Python package for creating composable command line interfaces | 8.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-argcomplete](ports/python3-colcon-argcomplete/README.md) | Completion for colcon command lines using argcomplete | 0.3.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-bash](ports/python3-colcon-bash/README.md) | Extension for colcon to provide Bash scripts | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-cd](ports/python3-colcon-cd/README.md) | Shell function for colcon to change to a package's directory | 0.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-cmake](ports/python3-colcon-cmake/README.md) | Extension for colcon to support CMake packages | 0.2.30 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-common-extensions](ports/python3-colcon-common-extensions/README.md) | Meta package aggregating colcon-core and common extensions | 0.3.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-core](ports/python3-colcon-core/README.md) | Command line tool to build sets of software packages | 0.21.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-defaults](ports/python3-colcon-defaults/README.md) | Extension for colcon to read defaults from a config file | 0.2.9 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-devtools](ports/python3-colcon-devtools/README.md) | Extension for colcon to list its extension points and extensions | 0.3.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-library-path](ports/python3-colcon-library-path/README.md) | Extension for colcon adding an environment variable to find libraries | 0.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-metadata](ports/python3-colcon-metadata/README.md) | Extension for colcon to read package metadata from files | 0.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-notification](ports/python3-colcon-notification/README.md) | Extension for colcon to provide status notifications | 0.3.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-output](ports/python3-colcon-output/README.md) | Extension for colcon to customize the output in various ways | 0.2.15 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-package-information](ports/python3-colcon-package-information/README.md) | Extension for colcon to output package information | 0.4.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-package-selection](ports/python3-colcon-package-selection/README.md) | Extension for colcon to select the packages to process | 0.2.10 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-parallel-executor](ports/python3-colcon-parallel-executor/README.md) | Extension for colcon to process packages in parallel | 0.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-pkg-config](ports/python3-colcon-pkg-config/README.md) | Extension for colcon adding an environment variable for pkg-config files | 0.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-powershell](ports/python3-colcon-powershell/README.md) | Extension for colcon to provide PowerShell scripts | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-python-setup-py](ports/python3-colcon-python-setup-py/README.md) | Extension for colcon to support Python packages described by setup.py | 0.2.9 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-recursive-crawl](ports/python3-colcon-recursive-crawl/README.md) | Extension for colcon to recursively crawl for packages | 0.2.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-ros](ports/python3-colcon-ros/README.md) | Extension for colcon to support ROS packages | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-test-result](ports/python3-colcon-test-result/README.md) | Extension for colcon to provide information about the test results | 0.3.8 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-colcon-zsh](ports/python3-colcon-zsh/README.md) | Extension for colcon to provide Z shell scripts | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-contourpy](ports/python3-contourpy/README.md) | Python library for calculating contours of 2D quadrilateral grids | 1.4.0 | custom | files.pythonhosted.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [python3-coverage](ports/python3-coverage/README.md) | Code coverage measurement for Python | 7.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cparser](ports/python3-cparser/README.md) | C parser in pure Python (cffi parses C declarations with it) | 3.12 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cppy](ports/python3-cppy/README.md) | C++ helper headers for writing Python C extensions | 1.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-curio](ports/python3-curio/README.md) | Coroutine-based Python library for concurrent systems programming | 1.6_git20251221 | custom | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-cycler](ports/python3-cycler/README.md) | Composable style cycles for Python (matplotlib's property cycler) | 0.12.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-dateutil](ports/python3-dateutil/README.md) | Python extensions to the standard datetime module | 2.9.0.post0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-dbus](ports/python3-dbus/README.md) | Python bindings for D-Bus (libdbus), with GLib main loop integration | 1.5.1 | custom | files.pythonhosted.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [python3-distlib](ports/python3-distlib/README.md) | Distribution utilities | 0.4.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-distro](ports/python3-distro/README.md) | OS distribution information (name, version, ID) for Python | 1.9.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-docutils](ports/python3-docutils/README.md) | Python documentation utilities (reStructuredText processing) | 0.23 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-editables](ports/python3-editables/README.md) | Editable installs for build backends | 0.6 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-empy](ports/python3-empy/README.md) | Templating system for Python | 4.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-execnet](ports/python3-execnet/README.md) | Distributed Python execution over channels (pytest-xdist's workers) | 2.1.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-filelock](ports/python3-filelock/README.md) | A platform independent file lock | 3.32.7 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-blind-except](ports/python3-flake8-blind-except/README.md) | flake8 plugin that reports blind except clauses | 0.2.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-builtins](ports/python3-flake8-builtins/README.md) | flake8 plugin that reports names shadowing Python builtins | 3.2.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-class-newline](ports/python3-flake8-class-newline/README.md) | flake8 plugin requiring a blank line after a class definition | 1.6.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-comprehensions](ports/python3-flake8-comprehensions/README.md) | flake8 plugin that suggests better comprehensions | 3.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-deprecated](ports/python3-flake8-deprecated/README.md) | flake8 plugin that reports deprecated method names | 2.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-docstrings](ports/python3-flake8-docstrings/README.md) | flake8 plugin running the pydocstyle docstring checks | 1.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-import-order](ports/python3-flake8-import-order/README.md) | flake8 plugin that checks the order of import statements | 0.19.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8-quotes](ports/python3-flake8-quotes/README.md) | flake8 plugin that checks string quote consistency | 3.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flake8](ports/python3-flake8/README.md) | Python linter combining pyflakes, pycodestyle and mccabe, with plugins | 7.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flaky](ports/python3-flaky/README.md) | A pytest plugin that reruns flaky tests | 3.8.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-flit-core](ports/python3-flit-core/README.md) | The PEP 517 build backend of Flit | 4.2.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-fonttools](ports/python3-fonttools/README.md) | Library to manipulate font files from Python | 4.68.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-fs](ports/python3-fs/README.md) | PyFilesystem2: a common interface to memory, OS, archive and other filesystems | 2.4.16 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-gast](ports/python3-gast/README.md) | Python AST that is the same across Python versions | 0.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-gobject3](ports/python3-gobject3/README.md) | Python bindings for GObject-based libraries through GObject introspection (PyGObject) | 3.58.1 | custom | files.pythonhosted.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [python3-gpep517](ports/python3-gpep517/README.md) | A minimal PEP 517 backend runner for distribution packaging | 23 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-hatch-fancy-pypi-readme](ports/python3-hatch-fancy-pypi-readme/README.md) | README composition for hatchling | 25.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-hatch-vcs](ports/python3-hatch-vcs/README.md) | Version from version control for hatchling | 0.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-hatchling](ports/python3-hatchling/README.md) | The build backend of Hatch | 1.31.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-idna](ports/python3-idna/README.md) | Python support for internationalized domain names (IDNA 2008, UTS 46) | 3.20 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-iniconfig](ports/python3-iniconfig/README.md) | Minimal INI file parsing module for Python | 2.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-installer](ports/python3-installer/README.md) | Installs Python wheels | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-jinja2](ports/python3-jinja2/README.md) | Jinja2 template engine for Python (HTML, configuration and code generation) | 3.1.6 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-jmespath](ports/python3-jmespath/README.md) | JSON Matching Expressions | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-kiwisolver](ports/python3-kiwisolver/README.md) | Fast implementation of the Cassowary constraint solver | 1.5.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-lark](ports/python3-lark/README.md) | A parsing toolkit (Earley and LALR(1) parsers from EBNF grammars) | 1.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-lxml](ports/python3-lxml/README.md) | Pythonic XML processing library combining libxml2/libxslt with ElementTree | 6.1.3 | custom | files.pythonhosted.org | — | 🔴 dependent | 3 | ✅ | ✅ |
| [python3-markupsafe](ports/python3-markupsafe/README.md) | MarkupSafe: HTML/XML-safe strings for Python, with its C speedups (Jinja2's escaping) | 3.0.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-matplotlib](ports/python3-matplotlib/README.md) | Comprehensive library for static and animated visualizations in Python | 3.11.2 | custom | files.pythonhosted.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [python3-mccabe](ports/python3-mccabe/README.md) | McCabe complexity checker for Python code (flake8 plugin) | 0.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-meson-python](ports/python3-meson-python/README.md) | The PEP 517 build backend for Meson projects | 0.22.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-ml-dtypes](ports/python3-ml-dtypes/README.md) | NumPy dtype extensions for machine learning (bfloat16, float8, int4) | 0.6.0 | custom | files.pythonhosted.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-msgpack](ports/python3-msgpack/README.md) | MessagePack binary serialisation for Python, with its C extension | 1.2.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-mypy-extensions](ports/python3-mypy-extensions/README.md) | Type system extensions for programs checked with mypy | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-netifaces](ports/python3-netifaces/README.md) | Portable network interface information for Python | 0.11.0 | custom | files.pythonhosted.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-opt-einsum](ports/python3-opt-einsum/README.md) | Optimised tensor contraction order for einsum expressions | 3.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-packaging](ports/python3-packaging/README.md) | Core utilities for Python packages: versions, specifiers, markers | 25.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pathspec](ports/python3-pathspec/README.md) | Gitignore-style path pattern matching | 1.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pexpect](ports/python3-pexpect/README.md) | Drives interactive child programs through a pseudo terminal | 4.9.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pillow](ports/python3-pillow/README.md) | Python Imaging Library (fork): image processing and file formats | 13.0.0 | custom | files.pythonhosted.org | — | 🔴 dependent | 8 | ✅ | ✅ |
| [python3-pip](ports/python3-pip/README.md) | Python package installer | 25.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-platformdirs](ports/python3-platformdirs/README.md) | The platform's user and site data/config/cache directories | 4.14.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pluggy](ports/python3-pluggy/README.md) | Plugin and hook calling mechanism for Python (used by pytest) | 1.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-ply](ports/python3-ply/README.md) | PLY: lex and yacc parsing tools for Python | 3.11 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pretend](ports/python3-pretend/README.md) | Python library for creating test stubs | 1.0.9 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-protobuf](ports/python3-protobuf/README.md) | Protocol Buffers runtime for Python, with the upb C extension | 5.29.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-ptyprocess](ports/python3-ptyprocess/README.md) | Runs a subprocess in a pseudo terminal | 0.7.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-py](ports/python3-py/README.md) | Legacy pylib: py.path local paths and related helpers | 1.11.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pycodestyle](ports/python3-pycodestyle/README.md) | PEP 8 style checker for Python code | 2.15.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pydbus](ports/python3-pydbus/README.md) | Pythonic D-Bus library built on GDBus through PyGObject | 0.6.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pydocstyle](ports/python3-pydocstyle/README.md) | PEP 257 docstring style checker | 6.3.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pyflakes](ports/python3-pyflakes/README.md) | Passive checker for errors in Python programs | 4.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pygments](ports/python3-pygments/README.md) | Python syntax highlighter for many languages | 2.21.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pyparsing](ports/python3-pyparsing/README.md) | Classes and methods to define and execute parsing grammars | 3.3.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pyproject-hooks](ports/python3-pyproject-hooks/README.md) | Calls PEP 517 build backend hooks | 1.3.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pyproject-metadata](ports/python3-pyproject-metadata/README.md) | PEP 621 metadata from pyproject.toml | 0.12.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-cov](ports/python3-pytest-cov/README.md) | pytest plugin for measuring code coverage | 7.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-mock](ports/python3-pytest-mock/README.md) | The mocker fixture for pytest | 3.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-repeat](ports/python3-pytest-repeat/README.md) | pytest plugin for repeating tests | 0.9.4 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-rerunfailures](ports/python3-pytest-rerunfailures/README.md) | pytest plugin to re-run tests to eliminate flaky failures | 16.7 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-tornasync](ports/python3-pytest-tornasync/README.md) | pytest plugin for testing tornado coroutines | 0.6.0.post2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest-xdist](ports/python3-pytest-xdist/README.md) | Runs pytest tests in parallel worker processes (-n) | 3.8.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytest](ports/python3-pytest/README.md) | Python testing framework | 9.2.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytokens](ports/python3-pytokens/README.md) | Fast spec-compliant Python 3.12+ tokenizer | 0.4.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pytz](ports/python3-pytz/README.md) | World timezone definitions for Python | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-pyyaml](ports/python3-pyyaml/README.md) | YAML parser and emitter for Python | 6.0.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-rosdistro](ports/python3-rosdistro/README.md) | Library for ROS distribution files (index, distributions, caches) | 1.1.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-rospkg](ports/python3-rospkg/README.md) | ROS package library: package and stack lookup, OS detection | 1.6.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-s3transfer](ports/python3-s3transfer/README.md) | An Amazon S3 Transfer Manager | 0.19.3 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-scikit-build-core](ports/python3-scikit-build-core/README.md) | The PEP 517 build backend for CMake projects | 1.1.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-scipy](ports/python3-scipy/README.md) | Scientific computing for Python: linear algebra, optimisation, FFT, signal processing, statistics | 1.18.1 | custom | files.pythonhosted.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [python3-setuptools](ports/python3-setuptools/README.md) | Python library and tools for building and distributing packages | 80.8.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-setuptools_scm](ports/python3-setuptools_scm/README.md) | Package versions from git/hg metadata | 9.2.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-six](ports/python3-six/README.md) | Python 2 and 3 compatibility library | 1.17.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sniffio](ports/python3-sniffio/README.md) | Detects which async library a Python program is running under | 1.3.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-snowballstemmer](ports/python3-snowballstemmer/README.md) | Snowball stemming algorithms in pure Python | 3.1.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sortedcontainers](ports/python3-sortedcontainers/README.md) | Sorted list, dict and set types in pure Python | 2.4.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-sounddevice](ports/python3-sounddevice/README.md) | Play and record sound with Python through PortAudio | 0.5.6 | custom | files.pythonhosted.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-time-machine](ports/python3-time-machine/README.md) | Mocks the clock in Python tests (time-machine, a C extension) | 3.6.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tomli](ports/python3-tomli/README.md) | A TOML parser | 2.5.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tornado](ports/python3-tornado/README.md) | Python web framework and asynchronous networking library | 6.6 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-traitlets](ports/python3-traitlets/README.md) | Typed, validated and observable attributes for Python configuration | 5.16.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-trove-classifiers](ports/python3-trove-classifiers/README.md) | The canonical list of PyPI trove classifiers | 2026.9.21.13 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-typeguard](ports/python3-typeguard/README.md) | Run-time type checking | 4.6.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-typing-extensions](ports/python3-typing-extensions/README.md) | Backported and experimental type hints (typing_extensions) | 4.16.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-tzdata](ports/python3-tzdata/README.md) | IANA time zone database as a Python package | 2025.2 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-urllib3](ports/python3-urllib3/README.md) | HTTP library with thread-safe connection pooling, file post, and more | 2.8.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-versioneer](ports/python3-versioneer/README.md) | Python tool for deriving version strings from version-control tags | 0.29 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-virtualenv](ports/python3-virtualenv/README.md) | Isolated Python environments | 20.39.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-wheel](ports/python3-wheel/README.md) | Command line tool and library for the Python wheel package format | 0.46.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [python3-zstandard](ports/python3-zstandard/README.md) | Zstandard compression bindings for Python (C backend on the system zstd) | 0.25.0 | custom | files.pythonhosted.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [python3](ports/python3/README.md) | Python 3 programming language interpreter and standard library | 3.14.0 | autotools | www.python.org | 2 | 🔴 dependent | 12 | ✅ | ✅ |
| [pythran](ports/pythran/README.md) | Ahead-of-time compiler of Python/numpy kernels to C++ extension modules | 0.19.0 | custom | files.pythonhosted.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [pytorch](ports/pytorch/README.md) | PyTorch tensor computation and deep learning library | 2.3.1 | custom | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [qdirstat](ports/qdirstat/README.md) | Qt-based directory statistics: disk usage with a treemap | 2.0 | custom | github.com | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [qhull](ports/qhull/README.md) | Computational geometry: convex hulls, Delaunay triangulation, Voronoi | 8.0.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [qpdf](ports/qpdf/README.md) | PDF Transformation System | 11.10.1 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [qt6-3d](ports/qt6-3d/README.md) | Qt 6 3D: a 3D scene graph and rendering framework (Qt 3D) for C++ and QML | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-datavis3d](ports/qt6-datavis3d/README.md) | Qt 6 Data Visualization: 3D bar, scatter and surface graphs | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-graphs](ports/qt6-graphs/README.md) | Qt 6 Graphs: 2D and 3D graphs for Qt Quick, on Qt Quick 3D | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-httpserver](ports/qt6-httpserver/README.md) | Qt 6 HTTP Server: embed a small HTTP/1.1 server with routing in a Qt application | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-location](ports/qt6-location/README.md) | Qt 6 Location: maps, places, routing and geocoding from C++ and QML | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 2 | ✅ | ✅ |
| [qt6-lottie](ports/qt6-lottie/README.md) | Qt 6 Lottie Animation: render Bodymovin (After Effects) JSON animations in Qt Quick | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-mqtt](ports/qt6-mqtt/README.md) | Qt 6 MQTT: an MQTT 3.1, 3.1.1 and 5 client for Qt applications | 6.8.1 | custom | code.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-networkauth](ports/qt6-networkauth/README.md) | Qt 6 Network Authorization: OAuth 1 and OAuth 2 client support | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-positioning](ports/qt6-positioning/README.md) | Qt 6 Positioning: geographic coordinates, positions, areas and position sources | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-quickeffectmaker](ports/qt6-quickeffectmaker/README.md) | Qt 6 Quick Effect Maker: a tool to compose shader effects for Qt Quick, and its runtime module | 6.8.1 | custom | download.qt.io | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-remoteobjects](ports/qt6-remoteobjects/README.md) | Qt 6 Remote Objects: share QObjects' properties, signals and slots between processes and devices | 6.8.1 | custom | download.qt.io | 3 | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-scxml](ports/qt6-scxml/README.md) | Qt 6 SCXML and State Machine: state machines from SCXML documents and the QStateMachine framework | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-sensors](ports/qt6-sensors/README.md) | Qt 6 Sensors: access to sensor hardware (accelerometer, gyroscope, ...) from C++ and QML | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6-translations](ports/qt6-translations/README.md) | Qt 6 translations: the .qm message catalogs of Qt's own user-visible strings | 6.8.1 | custom | download.qt.io | — | 🟡 build-deps only | — | ✅ | ✅ |
| [qt6-wayland](ports/qt6-wayland/README.md) | Qt 6 Wayland: the wayland platform plugin, so Qt programs run on Wayland compositors (labwc) | 6.8.1 | custom | download.qt.io | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [qt6-webchannel](ports/qt6-webchannel/README.md) | Qt 6 WebChannel: expose QObjects to JavaScript clients (web pages, QML) over a transport | 6.8.1 | custom | download.qt.io | — | 🔴 dependent | 1 | ✅ | ✅ |
| [qt6](ports/qt6/README.md) | Qt 6 cross-platform application and UI framework | 6.8.1 | custom | download.qt.io | 2 | 🔴 dependent | 20 | ✅ | ✅ |
| [quake3](ports/quake3/README.md) | ioquake3, the maintained open-source Quake III Arena engine | 1.36-git-20261012 | cmake | github.com | 4 | 🔴 dependent | 1 | ✅ | ✅ |
| [quickjs-ng](ports/quickjs-ng/README.md) | Small and embeddable JavaScript engine | 0.8.0 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [rapidjson](ports/rapidjson/README.md) | Header-only fast JSON parser and generator for C++ (SAX and DOM APIs) | 1.1.0-24b5e7a | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [raylib](ports/raylib/README.md) | Simple and easy-to-use library to enjoy videogames programming | 6.2 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [re2](ports/re2/README.md) | Fast, safe, thread-friendly regular expression library | 2024-07-02 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [readline](ports/readline/README.md) | GNU Readline library for command line editing and history | 8.2 | autotools | ftp.gnu.org | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [redis](ports/redis/README.md) | Advanced key-value store (server and command-line client) | 8.10.3 | custom | github.com | 5 | 🔴 dependent | 1 | ✅ | ✅ |
| [restbed](ports/restbed/README.md) | C++ framework for building HTTP and HTTPS client and server applications | 4.8 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [retroarch](ports/retroarch/README.md) | RetroArch, the libretro frontend for emulators and games | 1.15-git-20230408 | custom | github.com | 5 | 🔴 dependent | 7 | ✅ | ✅ |
| [retropie](ports/retropie/README.md) | RetroPie retro gaming stack: RetroArch, EmulationStation, cores, configs | 0.3 | custom | custom | — | 🟡 build-deps only | — | ✅ | ✅ |
| [rhash](ports/rhash/README.md) | Hash sum and magnet link tool (rhash) and the LibRHash library: CRC32, MD5, SHA-1/2/3, BLAKE2/3, TTH, BTIH and more | 1.4.5 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [riegeli](ports/riegeli/README.md) | File format for storing a sequence of records, typically protocol buffers | 0_git20231212 | custom | github.com | 3 | 🔴 dependent | 10 | ✅ | ✅ |
| [rocksdb](ports/rocksdb/README.md) | Embeddable, persistent key-value store for fast storage | 11.8.1 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [ros2-jazzy-control](ports/ros2-jazzy-control/README.md) | ros2_control and ros2_controllers for ROS 2 Jazzy: controller manager, hardware interfaces, standard controllers | 20260128 | custom | custom | — | 🔴 dependent | 7 | ✅ | ✅ |
| [ros2-jazzy-moveit2](ports/ros2-jazzy-moveit2/README.md) | MoveIt 2 for ROS 2 Jazzy (headless core): motion planning (OMPL, Pilz, CHOMP, STOMP), kinematics, collision checking, move_group, Servo | 20260128 | custom | custom | — | 🔴 dependent | 17 | — | ✅ |
| [ros2-jazzy-nav2](ports/ros2-jazzy-nav2/README.md) | Nav2 for ROS 2 Jazzy: navigation servers, planners, controllers, costmaps, BehaviorTree navigator | 20260128 | custom | custom | — | 🔴 dependent | 15 | ✅ | ✅ |
| [ros2-rviz](ports/ros2-rviz/README.md) | RViz 2 for ROS 2 Jazzy: the 3D visualiser, drawn natively on QNX Screen (OGRE on OpenGL ES) | 14.1.24 | custom | custom | — | 🔴 dependent | 7 | ✅ | ✅ |
| [ros2](ports/ros2/README.md) | ROS 2 Jazzy core: robotics middleware, client libraries and tools | jazzy | custom | custom | — | 🔴 dependent | 14 | ✅ | ✅ |
| [rsync](ports/rsync/README.md) | Fast local and remote file copying tool using delta transfer | 3.3.0 | autotools | download.samba.org | — | 🔴 dependent | 4 | ✅ | ✅ |
| [rtl-sdr](ports/rtl-sdr/README.md) | Library and tools turning RTL2832 DVB dongles into SDR receivers | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [ruby](ports/ruby/README.md) | Ruby programming language: interpreter, standard library, extensions and gems | 3.4.12 | autotools | cache.ruby-lang.org | 5 | 🔴 dependent | 6 | ✅ | ✅ |
| [ruckig](ports/ruckig/README.md) | Ruckig: time-optimal, jerk-limited online trajectory generation for robots | 0.17.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [rust](ports/rust/README.md) | Rust cross toolchain: rustc and cargo for the host, std for the targets | 1.90.0 | custom | static.rust-lang.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [ruy](ports/ruy/README.md) | Matrix multiplication library for neural network inference | 0-git-20240321 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [s2n-tls](ports/s2n-tls/README.md) | Small, fast C99 implementation of the TLS/SSL protocols from AWS | 1.7.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [samba](ports/samba/README.md) | SMB/CIFS file sharing: client tools, libsmbclient and the smbd/nmbd server | 4.25.1 | custom | download.samba.org | 7 | 🔴 dependent | 6 | ✅ | ✅ |
| [sassc](ports/sassc/README.md) | SassC, the command-line Sass (SCSS) to CSS compiler over LibSass | 3.6.2 | autotools | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [scdoc](ports/scdoc/README.md) | Man page generator: scdoc markup to roff | 1.11.3 | custom | git.sr.ht | — | 🟢 standalone | — | ✅ | ✅ |
| [scons](ports/scons/README.md) | Software construction tool with Python build scripts | 4.11.1 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl2-compat](ports/sdl2-compat/README.md) | SDL2 compatibility layer implemented on top of SDL3 | 2.32.60 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sdl2-image](ports/sdl2-image/README.md) | Image loading library for SDL2 | 2.8.8 | cmake | github.com | — | 🔴 dependent | 4 | ✅ | ✅ |
| [sdl2-mixer](ports/sdl2-mixer/README.md) | Audio mixer library for SDL2 (WAV, FLAC, MP3, Ogg Vorbis, Opus, MIDI) | 2.8.2 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [sdl2-net](ports/sdl2-net/README.md) | Portable network library for SDL2 | 2.2.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl2-ttf](ports/sdl2-ttf/README.md) | TrueType font rendering library for SDL2 using FreeType and HarfBuzz | 2.24.0 | cmake | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [sdl2](ports/sdl2/README.md) | Simple DirectMedia Layer 2, cross-platform multimedia and input library | 2.32.10 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [sdl3](ports/sdl3/README.md) | Simple DirectMedia Layer 3, cross-platform multimedia and input library | 3.4.18 | cmake | github.com | 1 | 🔴 dependent | 10 | ✅ | ✅ |
| [sentencepiece](ports/sentencepiece/README.md) | Unsupervised text tokenizer for neural network-based text generation | 0.2.1 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [shaderc](ports/shaderc/README.md) | Tools and libraries for Vulkan shader compilation (glslc, libshaderc) | 2026.2 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [shared-mime-info](ports/shared-mime-info/README.md) | Freedesktop.org shared MIME type database and update-mime-database | 2.4 | meson | gitlab.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [silkit-adapters-tap](ports/silkit-adapters-tap/README.md) | SIL Kit adapter connecting TAP network devices to SIL Kit Ethernet networks | 1.2.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [silkit](ports/silkit/README.md) | Open-source library for connecting software-in-the-loop simulations | 5.0.8 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [simdjson](ports/simdjson/README.md) | SIMD-accelerated JSON parsing library for C++ | 3.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [simdutf](ports/simdutf/README.md) | Fast SIMD Unicode validation and transcoding library | 6.4.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [skalibs](ports/skalibs/README.md) | Set of general-purpose C programming libraries for skarnet.org software | 2.14.5.1 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [sleef](ports/sleef/README.md) | SIMD library of vectorized elementary math functions and DFT | 3.8-git-20250214 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [snappy](ports/snappy/README.md) | Fast compression and decompression library | 1.2.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [socat](ports/socat/README.md) | Multipurpose relay for binary protocols (SOcket CAT) | 1.8.0.3 | autotools | www.dest-unreach.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [soem](ports/soem/README.md) | Simple Open EtherCAT Master library and tools | 2.0.1 | cmake | github.com | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [soxr](ports/soxr/README.md) | SoX Resampler, high quality one-dimensional sample-rate conversion library | 0.1.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [spdlog](ports/spdlog/README.md) | Fast C++ logging library | 1.15.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [speexdsp](ports/speexdsp/README.md) | Speech processing DSP library for the Speex codec | 1.2.1 | autotools | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [spirv-headers](ports/spirv-headers/README.md) | Machine-readable headers and grammar files for the SPIR-V registry | 1.4.321.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [spirv-tools](ports/spirv-tools/README.md) | SPIR-V assembler, disassembler, validator, optimizer and linker | 1.4.321.0 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sqlite](ports/sqlite/README.md) | SQLite embedded SQL database engine and command line shell | 3.47.2 | autotools | www.sqlite.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sqlite_orm](ports/sqlite_orm/README.md) | Header-only SQLite ORM library for modern C++ | 1.9 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [sscep](ports/sscep/README.md) | Simple SCEP client for certificate enrollment | 0.10.0 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sshpass](ports/sshpass/README.md) | Noninteractive ssh password auth | 1.10 | autotools | downloads.sourceforge.net | 2 | 🟢 standalone | — | ✅ | ✅ |
| [stable-diffusion.cpp](ports/stable-diffusion.cpp/README.md) | Stable Diffusion and FLUX image generation inference in C/C++ | 0_git20260601 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [stringencoders](ports/stringencoders/README.md) | Fast C string encoders and decoders (base64, base85, URL, JSON, ...) | 3.10.3-git-20180306 | autotools | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [strongswan](ports/strongswan/README.md) | IPsec-based VPN solution | 6.0.6 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [sudo](ports/sudo/README.md) | Runs commands with the privileges of another user | 1.9.17p2 | autotools | www.sudo.ws | 3 | 🔴 dependent | 3 | ✅ | ✅ |
| [suitesparse](ports/suitesparse/README.md) | Suite of sparse matrix algorithms and factorization libraries | 7.11.0 | cmake | github.com | 1 | 🔴 dependent | 1 | ✅ | ✅ |
| [svt-av1](ports/svt-av1/README.md) | Scalable Video Technology AV1 encoder library and tool | 2.3.0 | cmake | gitlab.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [swaybg](ports/swaybg/README.md) | Wallpaper utility for wlroots-based Wayland compositors | 1.2.2 | meson | github.com | 1 | 🔴 dependent | 4 | ✅ | ✅ |
| [swig](ports/swig/README.md) | Simplified Wrapper and Interface Generator: C/C++ bindings for Python, Perl, Lua, Tcl and more | 4.3.1 | autotools | downloads.sourceforge.net | — | 🔴 dependent | 1 | ✅ | ✅ |
| [sysbench](ports/sysbench/README.md) | Scriptable multi-threaded benchmark tool for CPU, memory, I/O and threads | 1.0.20-git-20250309 | autotools | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [taglib](ports/taglib/README.md) | Library for reading and editing audio file metadata tags | 2.0.2 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [tar](ports/tar/README.md) | GNU tar archiving utility | 1.35 | autotools | ftp.gnu.org | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [tcl](ports/tcl/README.md) | Tcl scripting language: tclsh, the embeddable libtcl and the bundled packages | 8.6.17 | autotools | downloads.sourceforge.net | — | 🔴 dependent | 1 | ✅ | ✅ |
| [tcpdump](ports/tcpdump/README.md) | A tool for network monitoring and data acquisition | 4.99.5 | autotools | www.tcpdump.org | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [tdb](ports/tdb/README.md) | Samba's trivial database: a small multi-process key/value store | 1.4.15 | custom | download.samba.org | 1 | 🟢 standalone | — | ✅ | ✅ |
| [tensorflow-lite](ports/tensorflow-lite/README.md) | TensorFlow Lite C/C++ runtime for machine learning inference | 2.16.1 | custom | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [texinfo](ports/texinfo/README.md) | GNU documentation system: makeinfo/texi2any, the info reader, install-info, texindex | 7.2 | autotools | ftp.gnu.org | — | 🔴 dependent | 1 | ✅ | ✅ |
| [tftp-hpa](ports/tftp-hpa/README.md) | TFTP client and server | 5.2 | custom | git.kernel.org | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [thorvg](ports/thorvg/README.md) | Lightweight library for rendering vector graphics, SVG and Lottie | 0.15.8 | meson | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [thunar](ports/thunar/README.md) | Thunar, the Xfce file manager, and its extension library | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 18 | ✅ | ✅ |
| [tinyxml](ports/tinyxml/README.md) | Simple, small, C++ XML parser (the original TinyXML) | 2.6.2 | custom | downloads.sourceforge.net | 2 | 🟢 standalone | — | ✅ | ✅ |
| [tinyxml2](ports/tinyxml2/README.md) | Small C++ XML parser | 10.0.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tl-expected](ports/tl-expected/README.md) | Header-only C++ implementation of std::expected with monadic extensions | 1.3.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tllist](ports/tllist/README.md) | Typed linked list C header library | 1.1.0 | meson | codeberg.org | — | 🟢 standalone | — | ✅ | ✅ |
| [tmux](ports/tmux/README.md) | Terminal multiplexer | 3.5a | autotools | github.com | 3 | 🔴 dependent | 2 | ✅ | ✅ |
| [tomlplusplus](ports/tomlplusplus/README.md) | Header-only TOML configuration file parser and serializer for C++17 | 3.4.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [toybox](ports/toybox/README.md) | Many common Linux command line utilities in a single BSD-licensed binary | 0.8.15 | custom | landley.net | 6 | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-bash](ports/tree-sitter-bash/README.md) | Bash grammar for tree-sitter | 0.25.0 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-c](ports/tree-sitter-c/README.md) | C grammar for tree-sitter | 0.24.1 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-cmake](ports/tree-sitter-cmake/README.md) | CMake grammar for tree-sitter | 0.7.2 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [tree-sitter-cpp](ports/tree-sitter-cpp/README.md) | tree-sitter grammar for C++ (parser library for editors and code tools) | 0.23.4 | custom | github.com | — | 🟢 standalone | — | ✅ | ✅ |
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
| [tzdata](ports/tzdata/README.md) | IANA time zone database (zoneinfo TZif files) with zic and zdump | 2026f | custom | data.iana.org | — | 🟢 standalone | — | ✅ | ✅ |
| [unbound](ports/unbound/README.md) | Validating, recursive and caching DNS resolver and library | 1.24.1 | autotools | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [unibilium](ports/unibilium/README.md) | Basic terminfo library for reading terminfo files and format strings | 2.1.2 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [unifdef](ports/unifdef/README.md) | Removes preprocessor conditionals from C/C++ source for given symbols | 2.12 | custom | dotat.at | — | 🟢 standalone | — | ✅ | ✅ |
| [up-core-api](ports/up-core-api/README.md) | uProtocol data model and core service APIs (protobuf, compiled for C++) | 1.6.0-alpha2 | cmake | github.com | 1 | 🔴 dependent | 2 | ✅ | ✅ |
| [up-cpp](ports/up-cpp/README.md) | Eclipse uProtocol C++ library: data model and communication APIs | 1.0.1 | cmake | github.com | 1 | 🔴 dependent | 3 | ✅ | ✅ |
| [up-transport-zenoh-cpp](ports/up-transport-zenoh-cpp/README.md) | uProtocol transport over Eclipse Zenoh for up-cpp (zenoh-cpp on zenoh-c) | 1.0.0-git-20250319 | cmake | github.com | 1 | 🔴 dependent | 7 | ✅ | ✅ |
| [urdfdom](ports/urdfdom/README.md) | Unified Robot Description Format (URDF) parser | 4.0.3 | cmake | github.com | — | 🔴 dependent | 2 | ✅ | ✅ |
| [urdfdom_headers](ports/urdfdom_headers/README.md) | Header-only C++ data structures for the Unified Robot Description Format | 1.1.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [uriparser](ports/uriparser/README.md) | Strictly RFC 3986 compliant URI parsing and handling library | 0.9.8 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [utf8proc](ports/utf8proc/README.md) | C library for Unicode normalization, case-folding and UTF-8 processing | 2.11.3 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [util-macros](ports/util-macros/README.md) | X.Org autoconf macros (xorg-macros.m4) | 1.20.2 | autotools | www.x.org | — | 🟢 standalone | — | ✅ | ✅ |
| [uvgrtp](ports/uvgrtp/README.md) | Open-source RTP library written in C++ (Ultra Video Group) | 4.0.1 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [vala](ports/vala/README.md) | Vala compiler (valac), translating Vala to C over GLib/GObject | 0.56.18 | autotools | download.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [valkey](ports/valkey/README.md) | High-performance key/value data store forked from Redis (server, client, benchmark) | 9.1.3 | custom | github.com | 7 | 🔴 dependent | 1 | ✅ | ✅ |
| [vcstool](ports/vcstool/README.md) | Command-line tool for working with many version-controlled repositories | 0.3.0 | custom | files.pythonhosted.org | — | 🟡 build-deps only | — | ✅ | ✅ |
| [vidstab](ports/vidstab/README.md) | Video stabilization library for FFmpeg and Transcode | 1.1.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [vim](ports/vim/README.md) | Vi IMproved, a highly configurable text editor | 9.1.0900 | autotools | github.com | — | 🔴 dependent | 3 | ✅ | ✅ |
| [virtualgl](ports/virtualgl/README.md) | VirtualGL client (vglclient) for displaying remotely rendered OpenGL | 3.1.5 | cmake | github.com | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [vlc](ports/vlc/README.md) | VLC media player and libvlc multimedia engine | 3.0.21 | custom | code.videolan.org | 2 | 🔴 dependent | 6 | ✅ | ✅ |
| [vsomeip](ports/vsomeip/README.md) | COVESA SOME/IP protocol stack for automotive communication | 3.5.6 | cmake | github.com | 4 | 🔴 dependent | 1 | ✅ | ✅ |
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
| [xfce4-dev-tools](ports/xfce4-dev-tools/README.md) | Xfce build tools: XDT autoconf macros, xdt-autogen, xdt-csource | 4.21.0 | autotools | archive.xfce.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [xfce4-panel](ports/xfce4-panel/README.md) | Xfce panel with launchers, menus and workspace switcher | 4.20.0 | autotools | archive.xfce.org | — | 🔴 dependent | 17 | ✅ | ✅ |
| [xfce4-session](ports/xfce4-session/README.md) | Xfce session manager | 4.20.2 | autotools | archive.xfce.org | 2 | 🔴 dependent | 11 | ✅ | ✅ |
| [xfce4-settings](ports/xfce4-settings/README.md) | Xfce settings daemon, manager and editor | 4.20.1 | autotools | archive.xfce.org | — | 🔴 dependent | 16 | ✅ | ✅ |
| [xfce4-terminal](ports/xfce4-terminal/README.md) | Xfce terminal emulator | 1.1.5 | meson | archive.xfce.org | 2 | 🔴 dependent | 12 | ✅ | ✅ |
| [xfconf](ports/xfconf/README.md) | Hierarchical configuration storage system for Xfce | 4.20.0 | autotools | archive.xfce.org | 2 | 🔴 dependent | 3 | ✅ | ✅ |
| [xkbcomp](ports/xkbcomp/README.md) | XKB keymap compiler | 1.5.0 | meson | xorg.freedesktop.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [xkeyboard-config](ports/xkeyboard-config/README.md) | XKB keyboard configuration data: rules, layouts, symbols | 2.43 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xmlto](ports/xmlto/README.md) | Tool for converting DocBook XML files to man pages, HTML and other formats | 0.0.29 | autotools | pagure.io | 2 | 🟢 standalone | — | ✅ | ✅ |
| [xnnpack](ports/xnnpack/README.md) | Optimized neural network inference operators for ARM and x86 | 0-git-20250209 | cmake | github.com | 2 | 🔴 dependent | 1 | ✅ | ✅ |
| [xorgproto](ports/xorgproto/README.md) | X Window System core and extension protocol headers | 2024.1 | meson | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xsimd](ports/xsimd/README.md) | Header-only C++ wrappers for SIMD intrinsics and vectorised math | 14.3.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xsltproc](ports/xsltproc/README.md) | Command-line XSLT processor from libxslt | 1.1.42 | cmake | gitlab.gnome.org | — | 🔴 dependent | 2 | ✅ | ✅ |
| [xtensor](ports/xtensor/README.md) | Header-only C++ multi-dimensional arrays with NumPy-style broadcasting | 0.27.1 | cmake | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [xtl](ports/xtl/README.md) | Basic tools, containers and algorithms for C++ (header-only, xtensor) | 0.8.2 | cmake | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [xtrans](ports/xtrans/README.md) | Header-only X network transport layer code shared by X libraries | 1.5.2 | autotools | gitlab.freedesktop.org | — | 🟢 standalone | — | ✅ | ✅ |
| [xvidcore](ports/xvidcore/README.md) | Xvid MPEG-4 Part 2 video codec library | 1.3.7 | custom | downloads.xvid.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xwayland](ports/xwayland/README.md) | X server running as a Wayland client (Xwayland) | 24.1.14 | meson | gitlab.freedesktop.org | 5 | 🔴 dependent | 6 | ✅ | ✅ |
| [xxhash](ports/xxhash/README.md) | Fast non-cryptographic hash algorithm library | 0.8.2 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [xz](ports/xz/README.md) | XZ Utils, LZMA compression library and tools | 5.6.3 | cmake | github.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [yaml-cpp](ports/yaml-cpp/README.md) | YAML parser and emitter for C++ | 0.8.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [yubico-c-client](ports/yubico-c-client/README.md) | YubiKey one-time password validation client library (libykclient) | 2.15 | autotools | developers.yubico.com | — | 🔴 dependent | 1 | ✅ | ✅ |
| [zenoh-c](ports/zenoh-c/README.md) | C bindings for Eclipse Zenoh pub/sub, storage and query protocol | 1.2.1 | custom | github.com | 2 | 🟡 build-deps only | — | ✅ | ✅ |
| [zenoh-cpp](ports/zenoh-cpp/README.md) | C++ bindings for Eclipse Zenoh (header-only, over zenoh-c or zenoh-pico) | 1.2.1 | cmake | github.com | — | 🟡 build-deps only | — | ✅ | ✅ |
| [zenoh-pico](ports/zenoh-pico/README.md) | Eclipse Zenoh pub/sub, store/query and compute protocol for microcontrollers | 1.2.1 | cmake | github.com | 3 | 🟢 standalone | — | ✅ | ✅ |
| [zenoh-router](ports/zenoh-router/README.md) | zenohd, the Eclipse Zenoh router (pub/sub, queries, liveliness across TCP/UDP/QUIC/TLS/WS) | 1.2.1 | custom | github.com | 1 | 🟡 build-deps only | — | ✅ | ✅ |
| [zerotier-one](ports/zerotier-one/README.md) | ZeroTier network virtualization service | 1.16.1 | custom | github.com | 2 | 🟢 standalone | — | ✅ | ✅ |
| [zimg](ports/zimg/README.md) | Library for image scaling, colorspace conversion and dithering | 3.0.5 | autotools | github.com | 1 | 🟢 standalone | — | ✅ | ✅ |
| [zip](ports/zip/README.md) | Info-ZIP zip: creates and updates .zip archives (zip, zipnote, zipsplit, zipcloak) | 3.0 | custom | downloads.sourceforge.net | — | 🟢 standalone | — | ✅ | ✅ |
| [zlib-ng](ports/zlib-ng/README.md) | zlib data compression library for next generation systems, native API | 2.2.5 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zlib](ports/zlib/README.md) | General purpose data compression library | 1.3.1 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zserio](ports/zserio/README.md) | C++ runtime library of zserio, a serialization framework for binary data | 2.20.0 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |
| [zsh](ports/zsh/README.md) | Z shell | 5.9 | autotools | downloads.sourceforge.net | 2 | 🔴 dependent | 2 | ✅ | ✅ |
| [zstd](ports/zstd/README.md) | Fast real-time lossless compression library (Zstandard) | 1.5.6 | cmake | github.com | — | 🟢 standalone | — | ✅ | ✅ |

