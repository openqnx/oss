# Putting the components on a QNX 8.0 target

Every component reaches a target one of two ways, and is tested there the same way:

| | How | Where it comes from |
|---|---|---|
| **A. Install the built package** | `apk add <name>` | `dists/` -- served from this machine (`scripts/serve-dists`) or the GitHub releases |
| **B. Build it yourself, then deploy** | original source + QNX patches, configure, build, package | `ported/<name>/<version>/` |
| **Test** (either way) | `apk add qnx-test-<name>`, run it | `test_suites/` |

## The three directories

| Directory | What is in it | Made by |
|---|---|---|
| `dists/8.0/<arch>/` | the packages: `<name>-<ver>-r<N>.apk` + `APKINDEX.tar.gz` (apk), `.tar.gz` + `INDEX` (oss-get), signed; **every version kept** | `scripts/qnxpkg --key keys/private/<key>.rsa --all` |
| `ported/<name>/<version>/` | `SOURCE.md` (where the original comes from and how to get it), `patches/` (the QNX patches, each with its reason), `<name>-<ver>-orig.tar.gz` (the original, untouched), `<name>-<ver>-qnx.tar.gz` (patched: what is built) | `scripts/gen-ported` |
| `test_suites/8.0/<arch>/` | the tests as packages (`qnx-test-<name>`, `qnx-port-tests` = the runner), `bin/` (the plain programs), `qnx-port-tests-<arch>.tar.gz` (every test with every library it loads, for a target without apk) | `scripts/qnxpkg --tests` |

`<arch>` is `x86_64` or `aarch64`. None of the three is tracked by git; `scripts/publish-github-release` puts `dists/` and `test_suites/` on GitHub releases.

## 1. Prepare the target (once)

Any QNX 8.0 image with a network, `sshd` and a writable `/data` will do -- the
packages are installed under `/data/oss` and need nothing else from the image.

Step by step, with every option explained (QNX SDP 8.0 installation,
host network, the exact `mkqnximage` options of the recorded results, what a
stock image lacks): [docs/qemu-test-vms.md](qemu-test-vms.md). In short:

### x86_64 -- QEMU (vanilla mkqnximage image)

```bash
source ~/qnx800/qnxsdp-env.sh
mkdir -p ~/vm/oss-vanilla-x86_64 && cd ~/vm/oss-vanilla-x86_64
mkqnximage --type=qemu --arch=x86_64 --hostname=vanilla-x86 \
           --ssh-ident=$HOME/.ssh/qnx-target.pub \
           --data-size=4000 --data-inodes=80000 --ram=2G --cpu=4 \
           --run=-h                                           # the recorded results' options
mkqnximage --getip                                             # the target's address
```

### aarch64 -- QEMU (vanilla mkqnximage image)

The same with `--arch=aarch64le`. QEMU emulates Arm on an x86 host (no KVM):
correct, several times slower -- fine for testing, give it time.

### aarch64 -- Raspberry Pi 5 (QNX 8.0 BSP image)

Write the QNX 8.0 Raspberry Pi 5 BSP image to an SD card as its BSP
documentation describes, boot, and make sure of three things:
network up (`ifconfig`), `sshd` running with your public key in
`/root/.ssh/authorized_keys` (or a password), and a writable `/data` with a
few hundred MB free (`df -h /data`). Not tested here on hardware: everything
below was checked on the two QEMU targets; the Pi is the same `aarch64le`
architecture and the same packages.

### Then, from the build host (for every target)

```bash
./scripts/serve-dists &                                               # local dists/ + test_suites/, port 8000
./scripts/target-bootstrap <target> -r http://<host>:8000/dists/8.0   # local packages
./scripts/target-bootstrap <target>                                   # or: the GitHub releases
```

`target-bootstrap` copies over SSH what a vanilla image lacks for the first
download -- `oss-get`, `apk-setup`, the signing key, a CA bundle -- and the
QNX run-time libraries the packages use that the image does not carry (a
vanilla mkqnximage image lacks `libeventfd`, `libepoll`, `libsignalfd`,
`libtimerfd`, `libasound`, `libEGL`, `libGLESv2`, `libscreen`,
`libbacktrace`; they come from your SDP, into `/data/oss/qnx-runtime/lib`).
Then apk installs itself into `/data/oss`, with this repository and its
tests repository configured. On the target, once per shell:

```sh
. /data/oss/etc/oss-env.sh     # PATH, LD_LIBRARY_PATH
```

## 2A. Install and test a package

```sh
apk add <name>                               # + everything it needs at run time
apk add qnx-test-<test> qnx-port-tests       # its test (names in the tables below)
/usr/local/bin/qnx-port-tests/test_<test>    # one test: ok/FAIL per check, PASS/FAIL at the end
/usr/local/bin/qnx-port-tests/run-on-target.sh   # every installed test
apk add <name>=<version>                     # a specific version (all are kept)
apk del <name>
```

From the build host, the same for one component, a kind, or what changed --
installed with apk from what `serve-dists` serves, tested, and recorded in
`tests/results/<arch>.tsv`:

```bash
./scripts/verify-on-target -t "QEMU x86_64, vanilla mkqnximage image" <target> zstd mosquitto
./scripts/verify-on-target -t "Raspberry Pi 5, QNX 8.0 BSP" <target> --standalone
./scripts/verify-on-target -t "<target>" <target> --dependent
./scripts/verify-on-target -t "<target>" <target> --changed    # versions not yet verified there
```

Only what passed there is published with `scripts/publish-github-release --verified --upload`
(`--changed` uploads only files the release does not have yet; `--only <name>...` a few components).

## 2B. Build it yourself from `ported/`, then test on the target

```bash
tar xf ported/<name>/<ver>/<name>-<ver>-qnx.tar.gz -C src/   # original + QNX patches
#   or: the original (-orig.tar.gz, or the URL in SOURCE.md) + patch -p1 < ported/<name>/<ver>/patches/*.patch
source ./env.sh x86_64          # or aarch64le
```

then configure, build and install into the staging tree exactly as
`ports/<name>/README.md` shows (step 4; `./scripts/qnxbuild <name>` does the
same in one command). Deploy what you built the same way as 2A -- package it
and let the target install it:

```bash
./scripts/qnxpkg --key keys/private/<key>.rsa <name>      # -> dists/ (a new release if it changed)
./scripts/serve-dists &
./scripts/verify-on-target -t "<target>" <target> <name>   # apk add + its test + record
```

or copy the files yourself (`scp` the `dists/8.0/<arch>/<name>-*.tar.gz` and
unpack it under `/data/oss`, or `test_suites/8.0/<arch>/bin/test_<test>`).

## Standalone and dependent


**271 standalone** components build with the QNX SDP alone and run on a vanilla image with nothing else: `apk add <name>` installs one package. **404 dependent** components need others: apk installs those first by itself (the *needs at run time* column); to build one yourself, build its *build* dependencies first (`ports/<name>/README.md` lists them in order).

Verified: `x86✅` / `a64✅` passed on x86_64 / aarch64 (QEMU) with the version in `dists/`, `ᵛ` on a vanilla image installed with apk; details per component in its README's *How it was verified*.

### Standalone (271)

| Component | What it is | Version | Install | Test package | Verified |
|---|---|---|---|---|---|
| [`7zip`](../ports/7zip/) | File archiver with a high compression ratio (7-Zip command line, 7zz) | 25.01 | `apk add 7zip` | `qnx-test-7zip` | x86✅ᵛ a64✅ᵛ |
| [`abseil`](../ports/abseil/) | Collection of C++ libraries augmenting the C++ standard library | 20240722.0 | `apk add abseil` | `qnx-test-abseil` | x86✅ᵛ a64✅ᵛ |
| [`ada`](../ports/ada/) | Fast WHATWG-compliant URL parser library for C++ | 3.4.4 | `apk add ada` | `qnx-test-ada` | x86✅ᵛ a64✅ᵛ |
| [`adwaita-fonts`](../ports/adwaita-fonts/) | Adwaita Sans and Adwaita Mono, the GNOME fonts | 49.0 | `apk add adwaita-fonts` | `qnx-test-adwaita-fonts` | x86✅ᵛ a64✅ᵛ |
| [`aom`](../ports/aom/) | AV1 video codec library from the Alliance for Open Media | 3.12.1 | `apk add aom` | `qnx-test-aom` | x86✅ᵛ a64✅ᵛ |
| [`apr`](../ports/apr/) | Apache Portable Runtime, a portable C API for system services | 1.7.6 | `apk add apr` | `qnx-test-apr` | x86✅ᵛ a64✅ᵛ |
| [`asio`](../ports/asio/) | Header-only C++ library for asynchronous network and low-level I/O | 1.30.2 | `apk add asio` | `qnx-test-asio` | x86✅ᵛ a64✅ᵛ |
| [`autoconf`](../ports/autoconf/) | GNU tool for generating configure scripts from M4 macros | 2.72 | `apk add autoconf` | `qnx-test-autoconf` | x86✅ᵛ a64✅ᵛ |
| [`automake`](../ports/automake/) | GNU tool for generating portable Makefile.in files | 1.18 | `apk add automake` | `qnx-test-automake` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-common`](../ports/aws-c-common/) | Core C library of the AWS SDK: primitives, data structures, errors | 0.12.6 | `apk add aws-c-common` | `qnx-test-aws-c-common` | x86✅ᵛ a64✅ᵛ |
| [`benchmark`](../ports/benchmark/) | Google's C++ library for benchmarking code snippets | 1.9.0 | `apk add benchmark` | `qnx-test-benchmark` | x86✅ᵛ a64✅ᵛ |
| [`bison`](../ports/bison/) | The GNU general-purpose parser generator | 3.8.2 | `apk add bison` | `qnx-test-bison` | x86✅ᵛ a64✅ᵛ |
| [`boringssl`](../ports/boringssl/) | Fork of OpenSSL designed to meet Google's needs (libcrypto, libssl, bssl) | 0.20260929.0 | `apk add boringssl` | `qnx-test-boringssl` | x86✅ᵛ a64✅ᵛ |
| [`brotli`](../ports/brotli/) | Generic lossless compression library and tool using the Brotli format | 1.1.0 | `apk add brotli` | `qnx-test-brotli` | x86✅ᵛ a64✅ᵛ |
| [`btop`](../ports/btop/) | Terminal resource monitor for CPU, memory, disks, network and processes | 1.4.6 | `apk add btop` | `qnx-test-monitors` | x86✅ᵛ a64✅ᵛ |
| [`bullet3`](../ports/bullet3/) | Bullet real-time collision detection and multi-physics simulation library | 3.25 | `apk add bullet3` | `qnx-test-bullet3` | x86✅ᵛ a64✅ᵛ |
| [`busybox`](../ports/busybox/) | Tiny versions of many common UNIX utilities in a single executable | 1.36.1 | `apk add busybox` | `qnx-test-busybox` | x86✅ᵛ a64✅ᵛ |
| [`bzip2`](../ports/bzip2/) | Block-sorting file compressor and libbz2 library | 1.0.8 | `apk add bzip2` | `qnx-test-bzip2` | x86✅ᵛ a64✅ᵛ |
| [`c-ares`](../ports/c-ares/) | Asynchronous DNS resolver library in C | 1.34.3 | `apk add c-ares` | `qnx-test-c-ares` | x86✅ᵛ a64✅ᵛ |
| [`ca-certificates`](../ports/ca-certificates/) | Mozilla's CA certificate bundle for TLS verification | 20261020 | `apk add ca-certificates` | `qnx-test-ca-certificates` | x86✅ᵛ a64✅ᵛ |
| [`can-utils`](../ports/can-utils/) | CAN log converters (asc2log, log2asc, log2long), can-calc-bit-timing | 2025.01 | `apk add can-utils` | `qnx-test-can-utils` | x86✅ᵛ a64✅ᵛ |
| [`candb`](../ports/candb/) | Library for parsing CAN bus database description formats | 0.1-git-20210625 | `apk add candb` | `qnx-test-candb` | – |
| [`capicxx-core-runtime`](../ports/capicxx-core-runtime/) | COVESA CommonAPI C++ core runtime for IPC interfaces | 3.2.4 | `apk add capicxx-core-runtime` | `qnx-test-capicxx-core-runtime` | x86✅ᵛ a64✅ᵛ |
| [`capstone`](../ports/capstone/) | Multi-architecture disassembly framework | 5.0.9 | `apk add capstone` | `qnx-test-capstone` | x86✅ᵛ a64✅ᵛ |
| [`catch2`](../ports/catch2/) | C++ unit testing framework with micro-benchmarking and BDD macros | 3.7.1 | `apk add catch2` | `qnx-test-catch2` | x86✅ᵛ a64✅ᵛ |
| [`cereal`](../ports/cereal/) | Header-only C++11 library for serialization | 1.3.2 | `apk add cereal` | `qnx-test-cereal` | x86✅ᵛ a64✅ᵛ |
| [`check`](../ports/check/) | Unit testing framework for C | 0.15.2 | `apk add check` | `qnx-test-check` | x86✅ᵛ a64✅ᵛ |
| [`chrony`](../ports/chrony/) | NTP client and server for synchronizing the system clock | 4.8-git-20260226 | `apk add chrony` | `qnx-test-chrony` | – |
| [`cjson`](../ports/cjson/) | Lightweight JSON parser in ANSI C | 1.7.18 | `apk add cjson` | `qnx-test-cjson` | x86✅ᵛ a64✅ᵛ |
| [`clapack`](../ports/clapack/) | LAPACK linear algebra routines translated to C with f2c | 3.2.1 | `apk add clapack` | `qnx-test-clapack` | x86✅ᵛ a64✅ᵛ |
| [`cli11`](../ports/cli11/) | Header-only command line parser for C++11 and later | 2.4.2 | `apk add cli11` | `qnx-test-cli11` | x86✅ᵛ a64✅ᵛ |
| [`compiler-rt`](../ports/compiler-rt/) | LLVM compiler runtime: builtins and profiling libraries | 22.1.8 | `apk add compiler-rt` | `qnx-test-compiler-rt` | x86✅ᵛ a64✅ᵛ |
| [`computelibrary`](../ports/computelibrary/) | Arm Compute Library of optimized NEON/SVE kernels for Arm CPUs | 53.0.0 | `apk add computelibrary` | `qnx-test-computelibrary` | a64✅ᵛ |
| [`console_bridge`](../ports/console_bridge/) | ROS-independent C++ logging library mirroring rosconsole calls | 1.0.2 | `apk add console_bridge` | `qnx-test-console-bridge` | x86✅ᵛ a64✅ᵛ |
| [`cpuinfo`](../ports/cpuinfo/) | C library for detecting CPU features relevant to performance tuning | 0-git-20240417 | `apk add cpuinfo` | `qnx-test-cpuinfo` | – |
| [`csmith`](../ports/csmith/) | Random generator of C programs for testing compilers | 2.3.0 | `apk add csmith` | `qnx-test-csmith` | x86✅ᵛ a64✅ᵛ |
| [`cunit`](../ports/cunit/) | Unit testing framework for C | 2.1-3 | `apk add cunit` | `qnx-test-cunit` | – |
| [`cyclonedds`](../ports/cyclonedds/) | Eclipse Cyclone DDS, an implementation of the OMG DDS standard | 0.10.5 | `apk add cyclonedds` | `qnx-test-cyclonedds` | x86✅ᵛ a64✅ᵛ |
| [`dav1d`](../ports/dav1d/) | Fast cross-platform AV1 video decoder library | 1.5.0 | `apk add dav1d` | `qnx-test-dav1d` | x86✅ᵛ a64✅ᵛ |
| [`davici`](../ports/davici/) | Decoupled Asynchronous VICI client library for strongSwan | 1.6 | `apk add davici` | `qnx-test-davici` | x86✅ᵛ a64✅ᵛ |
| [`dhcpcd`](../ports/dhcpcd/) | DHCP, DHCPv6 and IPv4LL (ZeroConf) client | 10.5.2 | `apk add dhcpcd` | `qnx-test-dhcpcd` | x86✅ᵛ a64✅ᵛ |
| [`dnsmasq`](../ports/dnsmasq/) | Lightweight DNS forwarder and DHCP server | 2.93test4 | `apk add dnsmasq` | `qnx-test-dnsmasq` | – |
| [`docbook-xsl`](../ports/docbook-xsl/) | DocBook XSL stylesheets for converting DocBook XML to HTML and man pages | 1.79.2 | `apk add docbook-xsl` | `qnx-test-docbook-xsl` | x86✅ᵛ a64✅ᵛ |
| [`double-conversion`](../ports/double-conversion/) | C++ routines for binary-decimal conversion of IEEE doubles | 3.3.0 | `apk add double-conversion` | `qnx-test-double-conversion` | x86✅ᵛ a64✅ᵛ |
| [`duktape`](../ports/duktape/) | Small embeddable ECMAScript (JavaScript) engine with a C API | 2.7.0 | `apk add duktape` | `qnx-test-duktape` | x86✅ᵛ a64✅ᵛ |
| [`ed`](../ports/ed/) | GNU ed, the POSIX line-oriented text editor | 1.22.5 | `apk add ed` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`eigen`](../ports/eigen/) | Header-only C++ template library for linear algebra | 3.4.0 | `apk add eigen` | `qnx-test-eigen` | x86✅ᵛ a64✅ᵛ |
| [`enca`](../ports/enca/) | Detects and converts the character encoding of text files | 1.19 | `apk add enca` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`encodings`](../ports/encodings/) | X.Org font encoding tables for libfontenc | 1.1.0 | `apk add encodings` | `qnx-test-encodings` | x86✅ᵛ a64✅ᵛ |
| [`expat`](../ports/expat/) | Stream-oriented XML parser library in C | 2.6.4 | `apk add expat` | `qnx-test-expat` | x86✅ᵛ a64✅ᵛ |
| [`fakeroot`](../ports/fakeroot/) | Runs a command in an environment faking root privileges for packaging | 1.37.1.1 | `apk add fakeroot` | `qnx-test-fakeroot` | x86✅ᵛ a64✅ᵛ |
| [`farmhash`](../ports/farmhash/) | Google's FarmHash family of hash functions for strings | 1.1-git-20190513 | `apk add farmhash` | `qnx-test-farmhash` | – |
| [`fast-float`](../ports/fast-float/) | Fast and exact implementation of from_chars for numbers | 8.0.2 | `apk add fast-float` | `qnx-test-fast-float` | – |
| [`fastcdr`](../ports/fastcdr/) | eProsima Fast CDR, C++ serialization library for the CDR format | 2.2.6 | `apk add fastcdr` | `qnx-test-fastcdr` | x86✅ᵛ a64✅ᵛ |
| [`fastfetch`](../ports/fastfetch/) | Like neofetch, but much faster because written in C | 2.66.1 | `apk add fastfetch` | `qnx-test-fastfetch` | x86✅ᵛ a64✅ᵛ |
| [`fdk-aac`](../ports/fdk-aac/) | Fraunhofer FDK AAC audio codec library | 2.0.3 | `apk add fdk-aac` | `qnx-test-fdk-aac` | x86✅ᵛ a64✅ᵛ |
| [`fft2d`](../ports/fft2d/) | General purpose 1D/2D/3D FFT, DCT and DST package (Ooura) | 2006.12.28 | `apk add fft2d` | `qnx-test-fft2d` | x86✅ᵛ a64✅ᵛ |
| [`flatbuffers`](../ports/flatbuffers/) | Memory-efficient serialization library and flatc schema compiler | 24.3.25 | `apk add flatbuffers` | `qnx-test-flatbuffers` | x86✅ᵛ a64✅ᵛ |
| [`fmt`](../ports/fmt/) | Fast and safe C++ formatting library | 11.2.0 | `apk add fmt` | `qnx-test-fmt` | x86✅ᵛ a64✅ᵛ |
| [`font-cantarell`](../ports/font-cantarell/) | Cantarell, the GNOME user interface font | 0.303.1 | `apk add font-cantarell` | `qnx-test-font-cantarell` | x86✅ᵛ a64✅ᵛ |
| [`font-dejavu`](../ports/font-dejavu/) | DejaVu font family (sans, serif, mono) | 2.37 | `apk add font-dejavu` | `qnx-test-font-dejavu` | x86✅ᵛ a64✅ᵛ |
| [`font-hack`](../ports/font-hack/) | Hack, a monospace font for source code | 3.003 | `apk add font-hack` | `qnx-test-font-hack` | x86✅ᵛ a64✅ᵛ |
| [`font-inter`](../ports/font-inter/) | Inter, a sans-serif font family for user interfaces | 4.1 | `apk add font-inter` | `qnx-test-font-inter` | x86✅ᵛ a64✅ᵛ |
| [`font-liberation`](../ports/font-liberation/) | Liberation fonts, metric-compatible with Arial, Times New Roman, Courier | 2.1.5 | `apk add font-liberation` | `qnx-test-font-liberation` | x86✅ᵛ a64✅ᵛ |
| [`font-noto-emoji`](../ports/font-noto-emoji/) | Noto Color Emoji font | 2.048 | `apk add font-noto-emoji` | `qnx-test-font-noto-emoji` | x86✅ᵛ a64✅ᵛ |
| [`font-noto-sans-cjk`](../ports/font-noto-sans-cjk/) | Noto Sans CJK fonts for Chinese, Japanese and Korean | 2.004 | `apk add font-noto-sans-cjk` | `qnx-test-font-noto-sans-cjk` | x86✅ᵛ a64✅ᵛ |
| [`font-noto-serif-cjk`](../ports/font-noto-serif-cjk/) | Noto Serif CJK fonts for Chinese, Japanese and Korean | 2.003 | `apk add font-noto-serif-cjk` | `qnx-test-font-noto-serif-cjk` | x86✅ᵛ a64✅ᵛ |
| [`font-util`](../ports/font-util/) | X.Org font utilities and autoconf macros for building font packages | 1.4.1 | `apk add font-util` | `qnx-test-font-util` | x86✅ᵛ a64✅ᵛ |
| [`foonathan_memory`](../ports/foonathan_memory/) | C++ memory allocator library improving on the STL allocator model | 0.7-3 | `apk add foonathan_memory` | `qnx-test-foonathan-memory` | x86✅ a64✅ |
| [`fp16`](../ports/fp16/) | Header-only C library for converting to and from half-precision floats | 0-git-20260620 | `apk add fp16` | `qnx-test-fp16` | – |
| [`freeimage`](../ports/freeimage/) | Library for loading and saving popular image formats (PNG, JPEG, TIFF...) | 3.18.0 | `apk add freeimage` | `qnx-test-freeimage` | x86✅ᵛ a64✅ᵛ |
| [`fribidi`](../ports/fribidi/) | Implementation of the Unicode Bidirectional Algorithm | 1.0.16 | `apk add fribidi` | `qnx-test-fribidi` | x86✅ᵛ a64✅ᵛ |
| [`fxdiv`](../ports/fxdiv/) | Header-only C library for integer division via fixed-point multiplication | 0-git-20200417 | `apk add fxdiv` | `qnx-test-fxdiv` | – |
| [`g3log`](../ports/g3log/) | Asynchronous crash-safe C++ logging library | 2.4 | `apk add g3log` | `qnx-test-g3log` | x86✅ᵛ a64✅ᵛ |
| [`gdbm`](../ports/gdbm/) | GNU dbm key/value database library | 1.24 | `apk add gdbm` | `qnx-test-gdbm` | x86✅ᵛ a64✅ᵛ |
| [`gemmlowp`](../ports/gemmlowp/) | Small self-contained low-precision (8-bit) GEMM library | 0-git-20231103 | `apk add gemmlowp` | `qnx-test-gemmlowp` | – |
| [`geographiclib`](../ports/geographiclib/) | Small C++ library for geodesic and other geographic conversions | 2.7 | `apk add geographiclib` | `qnx-test-geographiclib` | x86✅ᵛ a64✅ᵛ |
| [`gflags`](../ports/gflags/) | C++ library for command line flags processing | 2.2.2 | `apk add gflags` | `qnx-test-gflags` | x86✅ᵛ a64✅ᵛ |
| [`giflib`](../ports/giflib/) | Library and tools for reading and writing GIF images | 5.2.2 | `apk add giflib` | `qnx-test-giflib` | x86✅ᵛ a64✅ᵛ |
| [`gklib`](../ports/gklib/) | Helper routines library used by METIS and related software | 5.1.1-git-20230326 | `apk add gklib` | `qnx-test-gklib` | – |
| [`glm`](../ports/glm/) | Header-only C++ mathematics library for graphics based on GLSL | 1.0.1 | `apk add glm` | `qnx-test-glm` | x86✅ᵛ a64✅ᵛ |
| [`gmp`](../ports/gmp/) | GNU Multiple Precision arithmetic library | 6.3.0 | `apk add gmp` | `qnx-test-gmp` | x86✅ᵛ a64✅ᵛ |
| [`googletest`](../ports/googletest/) | Google's C++ testing and mocking framework (GoogleTest, GoogleMock) | 1.17.0 | `apk add googletest` | `qnx-test-googletest` | x86✅ᵛ a64✅ᵛ |
| [`gperf`](../ports/gperf/) | GNU perfect hash function generator | 3.1 | `apk add gperf` | `qnx-test-gperf` | x86✅ᵛ a64✅ᵛ |
| [`graphite2`](../ports/graphite2/) | Font rendering engine for complex scripts (SIL Graphite smart fonts) | 1.3.14 | `apk add graphite2` | `qnx-test-graphite2` | x86✅ᵛ a64✅ᵛ |
| [`gsm`](../ports/gsm/) | GSM 06.10 lossy speech compression library | 1.0.22 | `apk add gsm` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`hailort`](../ports/hailort/) | Light-weight, high-performance inference framework for Hailo devices | 4.14.0 | `apk add hailort` | `qnx-test-hailort` | x86✅ᵛ a64✅ᵛ |
| [`hicolor-icon-theme`](../ports/hicolor-icon-theme/) | Freedesktop.org default fallback icon theme | 0.18 | `apk add hicolor-icon-theme` | `qnx-test-hicolor-icon-theme` | x86✅ᵛ a64✅ᵛ |
| [`highway`](../ports/highway/) | Performance-portable C++ SIMD/vector intrinsics library | 1.2.0 | `apk add highway` | `qnx-test-highway` | x86✅ᵛ a64✅ᵛ |
| [`highwayhash`](../ports/highwayhash/) | Fast strong hash functions: SipHash/HighwayHash | 0_git20240418 | `apk add highwayhash` | `qnx-test-highwayhash` | x86✅ᵛ a64✅ᵛ |
| [`hyphen`](../ports/hyphen/) | Hyphenation library using converted TeX hyphenation patterns | 2.8.8 | `apk add hyphen` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`iceoryx`](../ports/iceoryx/) | Eclipse iceoryx, true zero-copy inter-process communication middleware | 2.0.6 | `apk add iceoryx` | `qnx-test-iceoryx` | x86✅ᵛ a64✅ᵛ |
| [`icu`](../ports/icu/) | International Components for Unicode: Unicode and locale support libraries | 76.1 | `apk add icu` | `qnx-test-icu` | x86✅ᵛ a64✅ᵛ |
| [`iperf2`](../ports/iperf2/) | Tool for measuring network bandwidth, TCP and UDP performance (iperf 2) | 2.2.1 | `apk add iperf2` | `qnx-test-iperf2` | x86✅ᵛ a64✅ᵛ |
| [`iso-codes`](../ports/iso-codes/) | Lists of ISO country, language, script and currency codes with translations | 4.17.0 | `apk add iso-codes` | `qnx-test-iso-codes` | x86✅ᵛ a64✅ᵛ |
| [`jansson`](../ports/jansson/) | C library for encoding, decoding and manipulating JSON data | 2.14 | `apk add jansson` | `qnx-test-jansson` | x86✅ᵛ a64✅ᵛ |
| [`json-c`](../ports/json-c/) | JSON implementation in C | 0.18 | `apk add json-c` | `qnx-test-json-c` | x86✅ᵛ a64✅ᵛ |
| [`jsoncpp`](../ports/jsoncpp/) | C++ library for reading and writing JSON | 1.9.6 | `apk add jsoncpp` | `qnx-test-jsoncpp` | x86✅ᵛ a64✅ᵛ |
| [`kineto`](../ports/kineto/) | PyTorch profiler library (libkineto) for CPU performance tracing | 0-git-20250214 | `apk add kineto` | `qnx-test-kineto` | – |
| [`lame`](../ports/lame/) | LAME MP3 audio encoder and library | 3.100 | `apk add lame` | `qnx-test-lame` | x86✅ᵛ a64✅ᵛ |
| [`lely-core`](../ports/lely-core/) | Lely C/C++ libraries for CANopen, CAN, event loop and I/O | 2.3.4 | `apk add lely-core` | `qnx-test-lely-core` | x86✅ᵛ a64✅ᵛ |
| [`libaec`](../ports/libaec/) | Adaptive Entropy Coding library for lossless compression, SZIP compatible | 1.1.7 | `apk add libaec` | `qnx-test-libaec` | x86✅ᵛ a64✅ᵛ |
| [`libcbor`](../ports/libcbor/) | C library for parsing and generating CBOR binary data | 0.11.0 | `apk add libcbor` | `qnx-test-libcbor` | x86✅ᵛ a64✅ᵛ |
| [`libccd`](../ports/libccd/) | Library for collision detection between convex shapes | 2.1 | `apk add libccd` | `qnx-test-fcl` | x86✅ᵛ a64✅ᵛ |
| [`libconfig`](../ports/libconfig/) | C/C++ library for processing structured configuration files | 1.8.1 | `apk add libconfig` | `qnx-test-libconfig` | x86✅ᵛ a64✅ᵛ |
| [`libdaemon`](../ports/libdaemon/) | Lightweight C library that eases the writing of UNIX daemons | 0.14 | `apk add libdaemon` | `qnx-test-libdaemon` | x86✅ᵛ a64✅ᵛ |
| [`libdatrie`](../ports/libdatrie/) | Double-array trie data structure library | 0.2.13 | `apk add libdatrie` | `qnx-test-libdatrie` | x86✅ᵛ a64✅ᵛ |
| [`libde265`](../ports/libde265/) | H.265/HEVC video decoder library | 1.0.15 | `apk add libde265` | `qnx-test-libde265` | x86✅ᵛ a64✅ᵛ |
| [`libdeflate`](../ports/libdeflate/) | Fast library for DEFLATE, zlib and gzip compression and decompression | 1.22 | `apk add libdeflate` | `qnx-test-libdeflate` | x86✅ᵛ a64✅ᵛ |
| [`libdisplay-info`](../ports/libdisplay-info/) | EDID and DisplayID parsing library | 0.3.0 | `apk add libdisplay-info` | `qnx-test-libdisplay-info` | x86✅ᵛ a64✅ᵛ |
| [`libdrm-headers`](../ports/libdrm-headers/) | Headers for the DRM kernel interface and the libdrm API (headers only) | 2.4.129 | `apk add libdrm-headers` | `qnx-test-libdrm-headers` | x86✅ᵛ a64✅ᵛ |
| [`libev`](../ports/libev/) | Full-featured event loop library for C | 4.33 | `apk add libev` | `qnx-test-libev` | x86✅ᵛ a64✅ᵛ |
| [`libexif`](../ports/libexif/) | Library for parsing, editing and saving EXIF data | 0.6.24 | `apk add libexif` | `qnx-test-libexif` | x86✅ᵛ a64✅ᵛ |
| [`libfdt`](../ports/libfdt/) | Device Tree Compiler (dtc) and libfdt for flat device trees | 1.7.2 | `apk add libfdt` | `qnx-test-libfdt` | x86✅ᵛ a64✅ᵛ |
| [`libffi`](../ports/libffi/) | Portable foreign function interface library | 3.4.6 | `apk add libffi` | `qnx-test-libffi` | x86✅ᵛ a64✅ᵛ |
| [`libhevc`](../ports/libhevc/) | Android's HEVC (H.265) video decoder and encoder libraries | 1.8.5 | `apk add libhevc` | `qnx-test-libhevc` | x86✅ᵛ a64✅ᵛ |
| [`libiconv`](../ports/libiconv/) | GNU character set conversion library (iconv) | 1.18 | `apk add libiconv` | `qnx-test-libiconv` | x86✅ᵛ a64✅ᵛ |
| [`libinih`](../ports/libinih/) | Simple INI file parser library for C and C++ | 58 | `apk add libinih` | `qnx-test-libinih` | x86✅ᵛ a64✅ᵛ |
| [`libjpeg-turbo`](../ports/libjpeg-turbo/) | SIMD-accelerated JPEG codec library and tools (libjpeg, TurboJPEG APIs) | 3.0.4 | `apk add libjpeg-turbo` | `qnx-test-libjpeg-turbo` | x86✅ᵛ a64✅ᵛ |
| [`libjson`](../ports/libjson/) | Small streaming JSON parser and printer in C | 0.8-git-20260512 | `apk add libjson` | `qnx-test-libjson` | – |
| [`libmd`](../ports/libmd/) | Message digest functions from BSD systems (MD5, SHA-1, SHA-2, RIPEMD) | 1.1.0 | `apk add libmd` | `qnx-test-libmd` | x86✅ᵛ a64✅ᵛ |
| [`libmetal`](../ports/libmetal/) | Abstraction library for device access, interrupts and shared memory | 2024.10.0 | `apk add libmetal` | `qnx-test-libmetal` | x86✅ᵛ a64✅ᵛ |
| [`libmicrohttpd`](../ports/libmicrohttpd/) | GNU C library for embedding an HTTP server | 1.0.1 | `apk add libmicrohttpd` | `qnx-test-libmicrohttpd` | x86✅ᵛ a64✅ᵛ |
| [`libmodbus`](../ports/libmodbus/) | Library for communicating with Modbus devices over serial or TCP | 3.1.11 | `apk add libmodbus` | `qnx-test-libmodbus` | x86✅ᵛ a64✅ᵛ |
| [`libogg`](../ports/libogg/) | Library for the Ogg multimedia bitstream container format | 1.3.5 | `apk add libogg` | `qnx-test-libogg` | x86✅ᵛ a64✅ᵛ |
| [`libpcap`](../ports/libpcap/) | Portable network traffic capture library | 1.10.5 | `apk add libpcap` | `qnx-test-libpcap` | x86✅ᵛ a64✅ᵛ |
| [`libpipeline`](../ports/libpipeline/) | C library for setting up and running pipelines of processes | 1.5.8 | `apk add libpipeline` | `qnx-test-libpipeline` | x86✅ᵛ a64✅ᵛ |
| [`libretro-2048`](../ports/libretro-2048/) | 2048 puzzle game as a libretro core for RetroArch | 0-git-20241227 | `apk add libretro-2048` | `qnx-test-libretro-2048` | – |
| [`libretro-fake08`](../ports/libretro-fake08/) | fake-08 PICO-8 emulator as a libretro core for RetroArch | 0-git-20260613 | `apk add libretro-fake08` | `qnx-test-libretro-fake08` | – |
| [`libretro-mrboom`](../ports/libretro-mrboom/) | Mr.Boom, an 8-player Bomberman clone, as a libretro core | 0-git-20260625 | `apk add libretro-mrboom` | `qnx-test-libretro-mrboom` | – |
| [`libretro-retro8`](../ports/libretro-retro8/) | retro8 PICO-8 fantasy console emulator as a libretro core | 0-git-20260912 | `apk add libretro-retro8` | `qnx-test-libretro-retro8` | – |
| [`libretro-samples`](../ports/libretro-samples/) | libretro sample cores: test patterns and input/audio tests | 0-git-20221110 | `apk add libretro-samples` | `qnx-test-libretro-samples` | – |
| [`libsamplerate`](../ports/libsamplerate/) | Audio sample rate conversion library (Secret Rabbit Code) | 0.2.2 | `apk add libsamplerate` | `qnx-test-libsamplerate` | x86✅ᵛ a64✅ᵛ |
| [`libsfdo`](../ports/libsfdo/) | Libraries implementing freedesktop.org desktop entry, icon and basedir specs | 0.1.4 | `apk add libsfdo` | `qnx-test-libsfdo` | x86✅ᵛ a64✅ᵛ |
| [`libsodium`](../ports/libsodium/) | Easy-to-use library for encryption, signatures and password hashing | 1.0.20 | `apk add libsodium` | `qnx-test-libsodium` | x86✅ᵛ a64✅ᵛ |
| [`libtasn1`](../ports/libtasn1/) | GNU library for ASN.1 and DER manipulation | 4.19.0 | `apk add libtasn1` | `qnx-test-libtasn1` | x86✅ᵛ a64✅ᵛ |
| [`libtommath`](../ports/libtommath/) | Multiple-precision integer library written in portable C | 1.3.0 | `apk add libtommath` | `qnx-test-libtommath` | x86✅ᵛ a64✅ᵛ |
| [`libtool`](../ports/libtool/) | GNU libltdl, a portable dynamic module loading library | 2.5.4 | `apk add libtool` | `qnx-test-libtool` | x86✅ᵛ a64✅ᵛ |
| [`libunibreak`](../ports/libunibreak/) | Implementation of Unicode line, word and grapheme breaking algorithms | 6.1 | `apk add libunibreak` | `qnx-test-libunibreak` | x86✅ᵛ a64✅ᵛ |
| [`libunistring`](../ports/libunistring/) | GNU library for manipulating Unicode strings | 1.3 | `apk add libunistring` | `qnx-test-libunistring` | x86✅ᵛ a64✅ᵛ |
| [`libunwind`](../ports/libunwind/) | Library for determining and unwinding the call chain of a program | 1.8.3 | `apk add libunwind` | `qnx-test-libunwind` | x86✅ᵛ a64✅ᵛ |
| [`libusb`](../ports/libusb/) | C library for accessing USB devices from user space | 1.0.27 | `apk add libusb` | `qnx-test-libusb` | x86✅ᵛ a64✅ᵛ |
| [`libutempter`](../ports/libutempter/) | Privileged helper and library for utmp/wtmp updates by terminal emulators | 1.2.3 | `apk add libutempter` | `qnx-test-libutempter` | x86✅ᵛ a64✅ᵛ |
| [`libuuid`](../ports/libuuid/) | Library for generating and handling UUIDs (from util-linux) | 2.42.4 | `apk add libuuid` | `qnx-test-libuuid` | x86✅ᵛ a64✅ᵛ |
| [`libuv`](../ports/libuv/) | Multi-platform support library for asynchronous I/O | 1.51.0 | `apk add libuv` | `qnx-test-libuv` | x86✅ᵛ a64✅ᵛ |
| [`libvpx`](../ports/libvpx/) | WebM VP8/VP9 video codec library and tools | 1.15.0 | `apk add libvpx` | `qnx-test-libvpx` | x86✅ᵛ a64✅ᵛ |
| [`libxcrypt`](../ports/libxcrypt/) | Extended crypt library for one-way password hashing | 4.4.38 | `apk add libxcrypt` | `qnx-test-libxcrypt` | x86✅ᵛ a64✅ᵛ |
| [`libxcvt`](../ports/libxcvt/) | Library and tool for generating VESA CVT standard timing modelines | 0.1.3 | `apk add libxcvt` | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxo`](../ports/libxo/) | Library for generating text, XML, JSON, and HTML output | 2.3.0 | `apk add libxo` | `qnx-test-libxo` | x86✅ᵛ a64✅ᵛ |
| [`libyaml`](../ports/libyaml/) | C library for parsing and emitting YAML | 0.2.5 | `apk add libyaml` | `qnx-test-libyaml` | x86✅ᵛ a64✅ᵛ |
| [`linux-headers-input-codes`](../ports/linux-headers-input-codes/) | Linux input event codes header (input-event-codes.h), header only | 6.12 | `apk add linux-headers-input-codes` | `qnx-test-linux-headers-input-codes` | x86✅ᵛ a64✅ᵛ |
| [`lmbench`](../ports/lmbench/) | Suite of micro-benchmarks for operating system and hardware performance | 3.0-git-20260929 | `apk add lmbench` | `qnx-test-lmbench` | – |
| [`lua`](../ports/lua/) | Lua 5.4 scripting language interpreter and library | 5.4.7 | `apk add lua` | `qnx-test-lua` | x86✅ᵛ a64✅ᵛ |
| [`lua5.1`](../ports/lua5.1/) | Lua 5.1 scripting language interpreter and library | 5.1.5 | `apk add lua5.1` | `qnx-test-lua5-1` | x86✅ᵛ a64✅ᵛ |
| [`lua5.3`](../ports/lua5.3/) | Lua 5.3 scripting language interpreter and library | 5.3.6 | `apk add lua5.3` | `qnx-test-lua5-3` | x86✅ᵛ a64✅ᵛ |
| [`luajit`](../ports/luajit/) | Just-in-time compiler for the Lua programming language | 2.1.c6ffc141 | `apk add luajit` | `qnx-test-luajit` | x86✅ a64✅ |
| [`lxde-icon-theme`](../ports/lxde-icon-theme/) | LXDE (nuoveXT) icon theme | 0.5.2 | `apk add lxde-icon-theme` | `qnx-test-lxde-icon-theme` | x86✅ᵛ a64✅ᵛ |
| [`lz4`](../ports/lz4/) | Fast lossless compression library and tool (LZ4) | 1.10.0 | `apk add lz4` | `qnx-test-lz4` | x86✅ᵛ a64✅ᵛ |
| [`lzip`](../ports/lzip/) | Lossless data compressor based on LZMA, with integrity checking | 1.26 | `apk add lzip` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`lzo`](../ports/lzo/) | LZO real-time data compression library | 2.10 | `apk add lzo` | `qnx-test-lzo` | x86✅ᵛ a64✅ᵛ |
| [`magic_enum`](../ports/magic_enum/) | Header-only C++17 library for static reflection of enums | 0.9.6 | `apk add magic_enum` | `qnx-test-magic-enum` | x86✅ᵛ a64✅ᵛ |
| [`make`](../ports/make/) | GNU make build automation tool | 4.4.1 | `apk add make` | `qnx-test-make` | x86✅ᵛ a64✅ᵛ |
| [`mbedtls`](../ports/mbedtls/) | Small C library for cryptography, X.509 and TLS/DTLS | 3.6.2 | `apk add mbedtls` | `qnx-test-mbedtls` | x86✅ᵛ a64✅ᵛ |
| [`mimalloc`](../ports/mimalloc/) | Compact general-purpose memory allocator from Microsoft | 3.1.5 | `apk add mimalloc` | `qnx-test-mimalloc` | x86✅ᵛ a64✅ᵛ |
| [`miniz`](../ports/miniz/) | Single-file zlib/Deflate compression library with ZIP archive support | 3.1.2 | `apk add miniz` | `qnx-test-miniz` | x86✅ᵛ a64✅ᵛ |
| [`mnn`](../ports/mnn/) | Lightweight deep learning inference engine from Alibaba | 3.6.2 | `apk add mnn` | `qnx-test-mnn` | x86✅ᵛ a64✅ᵛ |
| [`mpg123`](../ports/mpg123/) | MPEG audio (MP3) decoder library and signal synthesis library | 1.32.9 | `apk add mpg123` | `qnx-test-mpg123` | x86✅ᵛ a64✅ᵛ |
| [`msgpack-c`](../ports/msgpack-c/) | MessagePack binary object serialization library for C | 6.1.0 | `apk add msgpack-c` | `qnx-test-msgpack` | x86✅ᵛ a64✅ᵛ |
| [`mujoco`](../ports/mujoco/) | MuJoCo physics engine for multi-joint dynamics with contact | 3.10.0 | `apk add mujoco` | `qnx-test-mujoco` | x86✅ᵛ a64✅ᵛ |
| [`muslflt`](../ports/muslflt/) | musl's libm and float conversion routines as a separate library | 1.0-git-20240802 | `apk add muslflt` | `qnx-test-muslflt` | – |
| [`nanomsg`](../ports/nanomsg/) | Socket library that provides several common communication patterns | 1.3.0 | `apk add nanomsg` | `qnx-test-nanomsg` | x86✅ᵛ a64✅ᵛ |
| [`nanosvg`](../ports/nanosvg/) | Header-only SVG parser and rasterizer | 0-git-20260709 | `apk add nanosvg` | `qnx-test-nanosvg` | – |
| [`ncnn`](../ports/ncnn/) | Neural network inference framework for mobile and embedded devices | 20260526 | `apk add ncnn` | `qnx-test-ncnn` | x86✅ᵛ a64✅ᵛ |
| [`ncurses`](../ports/ncurses/) | Terminal handling and text user interface library | 6.5 | `apk add ncurses` | `qnx-test-ncurses` | x86✅ᵛ a64✅ᵛ |
| [`netmap`](../ports/netmap/) | netmap apps (pkt-gen, bridge, lb, nmreplay, tlem) for io-sock's netmap | 14.0-git-20261027 | `apk add netmap` | `qnx-test-netmap` | – |
| [`ninja`](../ports/ninja/) | Small build system with a focus on speed | 1.13.1 | `apk add ninja` | `qnx-test-ninja` | x86✅ᵛ a64✅ᵛ |
| [`nlohmann_json`](../ports/nlohmann_json/) | Header-only JSON library for modern C++ | 3.12.0 | `apk add nlohmann_json` | `qnx-test-nlohmann-json` | x86✅ᵛ a64✅ᵛ |
| [`nng`](../ports/nng/) | nanomsg-next-gen lightweight messaging library (pub/sub, req/rep and more) | 1.9.0 | `apk add nng` | `qnx-test-nng` | x86✅ᵛ a64✅ᵛ |
| [`npth`](../ports/npth/) | New GNU Portable Threads library | 1.8 | `apk add npth` | `qnx-test-npth` | x86✅ᵛ a64✅ᵛ |
| [`nspr`](../ports/nspr/) | Netscape Portable Runtime | 4.39 | `apk add nspr` | `qnx-test-nspr` | x86✅ᵛ a64✅ᵛ |
| [`nsync`](../ports/nsync/) | C library of synchronization primitives (locks, condition variables) | 1.30.0 | `apk add nsync` | `qnx-test-nsync` | x86✅ᵛ a64✅ᵛ |
| [`ntttcp-for-linux`](../ports/ntttcp-for-linux/) | Multi-threaded Linux network throughput benchmark (NTTTCP for Linux) | 1.4.0-git-20230613 | `apk add ntttcp-for-linux` | `qnx-test-ntttcp-for-linux` | – |
| [`octomap`](../ports/octomap/) | Probabilistic 3D mapping framework based on octrees | 1.10.0 | `apk add octomap` | `qnx-test-octomap` | x86✅ᵛ a64✅ᵛ |
| [`onednn`](../ports/onednn/) | oneAPI Deep Neural Network Library | 3.14.1 | `apk add onednn` | `qnx-test-onednn` | x86✅ᵛ a64✅ᵛ |
| [`onetbb`](../ports/onetbb/) | oneAPI Threading Building Blocks, C++ library for parallel programming | 2022.0.0 | `apk add onetbb` | `qnx-test-onetbb` | x86✅ᵛ a64✅ᵛ |
| [`oniguruma`](../ports/oniguruma/) | Regular expression library | 6.9.9 | `apk add oniguruma` | `qnx-test-oniguruma` | x86✅ᵛ a64✅ᵛ |
| [`openal-soft`](../ports/openal-soft/) | Cross-platform 3D audio library implementing the OpenAL API | 1.24.1 | `apk add openal-soft` | `qnx-test-openal-soft` | x86✅ᵛ a64✅ᵛ |
| [`openblas`](../ports/openblas/) | Optimized BLAS linear algebra library based on GotoBLAS2 | 0.3.28 | `apk add openblas` | `qnx-test-openblas` | x86✅ᵛ a64✅ᵛ |
| [`openh264`](../ports/openh264/) | H.264 video encoding and decoding library for real-time use | 2.5.0 | `apk add openh264` | `qnx-test-openh264` | x86✅ᵛ a64✅ᵛ |
| [`openmp`](../ports/openmp/) | LLVM OpenMP runtime library (libomp) | 22.1.8 | `apk add openmp` | `qnx-test-openmp` | x86✅ᵛ a64✅ᵛ |
| [`openpam`](../ports/openpam/) | BSD-licensed implementation of the Pluggable Authentication Modules API | 20250531 | `apk add openpam` | `qnx-test-openpam` | x86✅ᵛ a64✅ᵛ |
| [`openssl`](../ports/openssl/) | Toolkit and libraries for TLS, DTLS, QUIC and general cryptography | 3.5.1 | `apk add openssl` | `qnx-test-openssl` | x86✅ᵛ a64✅ᵛ |
| [`openttd-opengfx`](../ports/openttd-opengfx/) | OpenGFX, the free base graphics set for OpenTTD | 8.0 | `apk add openttd-opengfx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`openttd-openmsx`](../ports/openttd-openmsx/) | OpenMSX, the free base music set for OpenTTD | 0.4.2 | `apk add openttd-openmsx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`openttd-opensfx`](../ports/openttd-opensfx/) | OpenSFX, the free base sound set for OpenTTD | 1.0.3 | `apk add openttd-opensfx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`opus`](../ports/opus/) | Opus interactive speech and audio codec library | 1.5.2 | `apk add opus` | `qnx-test-opus` | x86✅ᵛ a64✅ᵛ |
| [`orc`](../ports/orc/) | Oil Runtime Compiler library for optimized inner loops (SIMD) | 0.4.40 | `apk add orc` | `qnx-test-orc` | x86✅ᵛ a64✅ᵛ |
| [`patch`](../ports/patch/) | GNU patch, applies diff files to original files | 2.8 | `apk add patch` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`pax-utils`](../ports/pax-utils/) | ELF security and QA utilities (scanelf, lddtree) | 1.3.8 | `apk add pax-utils` | `qnx-test-pax-utils` | x86✅ᵛ a64✅ᵛ |
| [`pcre`](../ports/pcre/) | Perl compatible regular expressions library (legacy PCRE1 API) | 8.45 | `apk add pcre` | `qnx-test-pcre` | x86✅ᵛ a64✅ᵛ |
| [`pdfium`](../ports/pdfium/) | PDFium, Chromium's PDF rendering library | 0-git-84a8011 | `apk add pdfium` | `qnx-test-pdfium` | – |
| [`picocom`](../ports/picocom/) | Minimal dumb-terminal emulator for serial ports | 3.1 | `apk add picocom` | `qnx-test-picocom` | x86✅ᵛ a64✅ᵛ |
| [`pkgconf`](../ports/pkgconf/) | Tool and library for managing compiler and linker flags (pkg-config) | 2.3.0 | `apk add pkgconf` | `qnx-test-pkgconf` | x86✅ᵛ a64✅ᵛ |
| [`protobuf-c`](../ports/protobuf-c/) | Protocol Buffers implementation in C (runtime library) | 1.5.0 | `apk add protobuf-c` | `qnx-test-protobuf-c` | x86✅ᵛ a64✅ᵛ |
| [`pthread-stubs`](../ports/pthread-stubs/) | pkg-config file for pthread symbols needed by X libraries | 0.5 | `apk add pthread-stubs` | `qnx-test-pthread-stubs` | x86✅ᵛ a64✅ᵛ |
| [`ptpd`](../ports/ptpd/) | Precision Time Protocol (IEEE 1588) daemon | 2.3.1 | `apk add ptpd` | `qnx-test-ptpd` | x86✅ᵛ a64✅ᵛ |
| [`pugixml`](../ports/pugixml/) | Lightweight XML parser for C++ with XPath support | 1.14 | `apk add pugixml` | `qnx-test-pugixml` | x86✅ᵛ a64✅ᵛ |
| [`pybind11`](../ports/pybind11/) | Seamless operability between C++11 and Python (header-only) | 3.1.0 | `apk add pybind11` | `qnx-test-pybind11` | – |
| [`qhull`](../ports/qhull/) | Computational geometry: convex hulls, Delaunay triangulation, Voronoi | 8.0.2 | `apk add qhull` | `qnx-test-qhull` | x86✅ᵛ a64✅ᵛ |
| [`quickjs-ng`](../ports/quickjs-ng/) | Small and embeddable JavaScript engine | 0.8.0 | `apk add quickjs-ng` | `qnx-test-quickjs` | x86✅ᵛ a64✅ᵛ |
| [`rapidjson`](../ports/rapidjson/) | Header-only fast JSON parser and generator for C++ (SAX and DOM APIs) | 1.1.0-24b5e7a | `apk add rapidjson` | `qnx-test-rapidjson` | x86✅ a64✅ |
| [`rust`](../ports/rust/) | Rust cross toolchain: rustc and cargo for the host, std for the targets | 1.90.0 | `apk add rust` | `qnx-test-rust` | x86✅ᵛ a64✅ᵛ |
| [`silkit`](../ports/silkit/) | Open-source library for connecting software-in-the-loop simulations | 5.0.8 | `apk add silkit` | `qnx-test-silkit` | x86✅ᵛ a64✅ᵛ |
| [`simdjson`](../ports/simdjson/) | SIMD-accelerated JSON parsing library for C++ | 3.11.3 | `apk add simdjson` | `qnx-test-simdjson` | x86✅ᵛ a64✅ᵛ |
| [`simdutf`](../ports/simdutf/) | Fast SIMD Unicode validation and transcoding library | 6.4.2 | `apk add simdutf` | `qnx-test-simdutf` | x86✅ᵛ a64✅ᵛ |
| [`skalibs`](../ports/skalibs/) | Set of general-purpose C programming libraries for skarnet.org software | 2.14.5.1 | `apk add skalibs` | `qnx-test-skalibs` | x86✅ᵛ a64✅ᵛ |
| [`sleef`](../ports/sleef/) | SIMD library of vectorized elementary math functions and DFT | 3.8-git-20250214 | `apk add sleef` | `qnx-test-sleef` | – |
| [`snappy`](../ports/snappy/) | Fast compression and decompression library | 1.2.1 | `apk add snappy` | `qnx-test-snappy` | x86✅ᵛ a64✅ᵛ |
| [`soxr`](../ports/soxr/) | SoX Resampler, high quality one-dimensional sample-rate conversion library | 0.1.3 | `apk add soxr` | `qnx-test-soxr` | x86✅ᵛ a64✅ᵛ |
| [`speexdsp`](../ports/speexdsp/) | Speech processing DSP library for the Speex codec | 1.2.1 | `apk add speexdsp` | `qnx-test-speexdsp` | x86✅ᵛ a64✅ᵛ |
| [`sshpass`](../ports/sshpass/) | Noninteractive ssh password auth | 1.10 | `apk add sshpass` | `qnx-test-sshpass` | x86✅ᵛ a64✅ᵛ |
| [`stable-diffusion.cpp`](../ports/stable-diffusion.cpp/) | Stable Diffusion and FLUX image generation inference in C/C++ | 0_git20260601 | `apk add stable-diffusion.cpp` | `qnx-test-sdcpp` | x86✅ᵛ a64✅ᵛ |
| [`stringencoders`](../ports/stringencoders/) | Fast C string encoders and decoders (base64, base85, URL, JSON, ...) | 3.10.3-git-20180306 | `apk add stringencoders` | `qnx-test-stringencoders` | – |
| [`svt-av1`](../ports/svt-av1/) | Scalable Video Technology AV1 encoder library and tool | 2.3.0 | `apk add svt-av1` | `qnx-test-svt-av1` | x86✅ᵛ a64✅ᵛ |
| [`tensorflow-lite`](../ports/tensorflow-lite/) | TensorFlow Lite C/C++ runtime for machine learning inference | 2.16.1 | `apk add tensorflow-lite` | `qnx-test-tflite` | x86✅ᵛ a64✅ᵛ |
| [`tinyxml`](../ports/tinyxml/) | Simple, small, C++ XML parser (the original TinyXML) | 2.6.2 | `apk add tinyxml` | `qnx-test-tinyxml` | x86✅ᵛ a64✅ᵛ |
| [`tinyxml2`](../ports/tinyxml2/) | Small C++ XML parser | 10.0.0 | `apk add tinyxml2` | `qnx-test-tinyxml2` | x86✅ᵛ a64✅ᵛ |
| [`tomlplusplus`](../ports/tomlplusplus/) | Header-only TOML configuration file parser and serializer for C++17 | 3.4.0 | `apk add tomlplusplus` | `qnx-test-tomlplusplus` | x86✅ᵛ a64✅ᵛ |
| [`toybox`](../ports/toybox/) | Many common Linux command line utilities in a single BSD-licensed binary | 0.8.15 | `apk add toybox` | `qnx-test-toybox` | x86✅ᵛ a64✅ᵛ |
| [`tree`](../ports/tree/) | Displays directory contents as an indented tree | 2.2.1 | `apk add tree` | `qnx-test-tree-sitter` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter`](../ports/tree-sitter/) | Incremental parsing library for programming tools | 0.25.9 | `apk add tree-sitter` | `qnx-test-tree-sitter` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-bash`](../ports/tree-sitter-bash/) | Bash grammar for tree-sitter | 0.25.0 | `apk add tree-sitter-bash` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-c`](../ports/tree-sitter-c/) | C grammar for tree-sitter | 0.24.1 | `apk add tree-sitter-c` | `qnx-test-tree-sitter-c` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-cmake`](../ports/tree-sitter-cmake/) | CMake grammar for tree-sitter | 0.7.2 | `apk add tree-sitter-cmake` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-lua`](../ports/tree-sitter-lua/) | Lua grammar for tree-sitter | 0.4.0 | `apk add tree-sitter-lua` | `qnx-test-tree-sitter-lua` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-make`](../ports/tree-sitter-make/) | Makefile grammar for tree-sitter | 1.1.1 | `apk add tree-sitter-make` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-markdown`](../ports/tree-sitter-markdown/) | Markdown and inline Markdown grammars for tree-sitter | 0.5.1 | `apk add tree-sitter-markdown` | `qnx-test-tree-sitter-markdown` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-python`](../ports/tree-sitter-python/) | Python grammar for tree-sitter | 0.23.6 | `apk add tree-sitter-python` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-query`](../ports/tree-sitter-query/) | Tree-sitter query file grammar for tree-sitter | 0.7.0 | `apk add tree-sitter-query` | `qnx-test-tree-sitter-query` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-vim`](../ports/tree-sitter-vim/) | Vimscript grammar for tree-sitter | 0.7.0 | `apk add tree-sitter-vim` | `qnx-test-tree-sitter-vim` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-vimdoc`](../ports/tree-sitter-vimdoc/) | Vim help file grammar for tree-sitter | 4.0.0 | `apk add tree-sitter-vimdoc` | `qnx-test-tree-sitter-vimdoc` | x86✅ᵛ a64✅ᵛ |
| [`trompeloeil`](../ports/trompeloeil/) | Header-only C++14 mocking framework | 48 | `apk add trompeloeil` | `qnx-test-trompeloeil` | x86✅ᵛ a64✅ᵛ |
| [`unibilium`](../ports/unibilium/) | Basic terminfo library for reading terminfo files and format strings | 2.1.2 | `apk add unibilium` | `qnx-test-unibilium` | x86✅ᵛ a64✅ᵛ |
| [`urdfdom_headers`](../ports/urdfdom_headers/) | Header-only C++ data structures for the Unified Robot Description Format | 1.1.2 | `apk add urdfdom_headers` | `qnx-test-urdfdom-headers` | x86✅ᵛ a64✅ᵛ |
| [`uriparser`](../ports/uriparser/) | Strictly RFC 3986 compliant URI parsing and handling library | 0.9.8 | `apk add uriparser` | `qnx-test-uriparser` | x86✅ᵛ a64✅ᵛ |
| [`utf8proc`](../ports/utf8proc/) | C library for Unicode normalization, case-folding and UTF-8 processing | 2.11.3 | `apk add utf8proc` | `qnx-test-utf8proc` | x86✅ᵛ a64✅ᵛ |
| [`uvgrtp`](../ports/uvgrtp/) | Open-source RTP library written in C++ (Ultra Video Group) | 4.0.1 | `apk add uvgrtp` | `qnx-test-uvgrtp` | x86✅ᵛ a64✅ᵛ |
| [`vidstab`](../ports/vidstab/) | Video stabilization library for FFmpeg and Transcode | 1.1.1 | `apk add vidstab` | `qnx-test-vidstab` | x86✅ᵛ a64✅ᵛ |
| [`webrtc`](../ports/webrtc/) | WebRTC native C++ library for real-time audio, video and data | m132-afaf497 | `apk add webrtc` | `qnx-test-webrtc` | – |
| [`whisper.cpp`](../ports/whisper.cpp/) | OpenAI Whisper speech recognition inference in C/C++ | 0_git20260728 | `apk add whisper.cpp` | `qnx-test-whisper` | x86✅ᵛ a64✅ᵛ |
| [`wlr-protocols`](../ports/wlr-protocols/) | Wayland protocol extensions from wlroots (XML files) | 0-git-20250702 | `apk add wlr-protocols` | `qnx-test-wlr-protocols` | – |
| [`wolfssl`](../ports/wolfssl/) | Lightweight embedded SSL/TLS library in C | 5.7.6 | `apk add wolfssl` | `qnx-test-wolfssl` | x86✅ᵛ a64✅ᵛ |
| [`x264`](../ports/x264/) | H.264/MPEG-4 AVC video encoder library | stable | `apk add x264` | `qnx-test-x264` | x86✅ a64✅ |
| [`x265`](../ports/x265/) | H.265/HEVC video encoder library | 3.6 | `apk add x265` | `qnx-test-x265` | x86✅ᵛ a64✅ᵛ |
| [`xcb-proto`](../ports/xcb-proto/) | XML-XCB protocol descriptions used to generate libxcb | 1.17.0 | `apk add xcb-proto` | `qnx-test-xcb-proto` | x86✅ᵛ a64✅ᵛ |
| [`xdg-utils`](../ports/xdg-utils/) | Desktop integration scripts such as xdg-open | 1.2.1 | `apk add xdg-utils` | `qnx-test-xdg-utils` | x86✅ᵛ a64✅ᵛ |
| [`xkeyboard-config`](../ports/xkeyboard-config/) | XKB keyboard configuration data: rules, layouts, symbols | 2.43 | `apk add xkeyboard-config` | `qnx-test-xkeyboard-config` | x86✅ᵛ a64✅ᵛ |
| [`xmlto`](../ports/xmlto/) | Tool for converting DocBook XML files to man pages, HTML and other formats | 0.0.29 | `apk add xmlto` | `qnx-test-xmlto` | x86✅ᵛ a64✅ᵛ |
| [`xorgproto`](../ports/xorgproto/) | X Window System core and extension protocol headers | 2024.1 | `apk add xorgproto` | `qnx-test-xorgproto` | x86✅ᵛ a64✅ᵛ |
| [`xtl`](../ports/xtl/) | Basic tools, containers and algorithms for C++ (header-only, xtensor) | 0.8.2 | `apk add xtl` | `qnx-test-xtl` | – |
| [`xtrans`](../ports/xtrans/) | Header-only X network transport layer code shared by X libraries | 1.5.2 | `apk add xtrans` | `qnx-test-xtrans` | x86✅ᵛ a64✅ᵛ |
| [`xvidcore`](../ports/xvidcore/) | Xvid MPEG-4 Part 2 video codec library | 1.3.7 | `apk add xvidcore` | `qnx-test-xvidcore` | x86✅ᵛ a64✅ᵛ |
| [`xxhash`](../ports/xxhash/) | Fast non-cryptographic hash algorithm library | 0.8.2 | `apk add xxhash` | `qnx-test-xxhash` | x86✅ᵛ a64✅ᵛ |
| [`yaml-cpp`](../ports/yaml-cpp/) | YAML parser and emitter for C++ | 0.8.0 | `apk add yaml-cpp` | `qnx-test-yaml-cpp` | x86✅ᵛ a64✅ᵛ |
| [`zenoh-pico`](../ports/zenoh-pico/) | Eclipse Zenoh pub/sub, store/query and compute protocol for microcontrollers | 1.2.1 | `apk add zenoh-pico` | `qnx-test-zenoh-pico` | x86✅ᵛ a64✅ᵛ |
| [`zerotier-one`](../ports/zerotier-one/) | ZeroTier network virtualization service | 1.16.1 | `apk add zerotier-one` | `qnx-test-zerotier-one` | x86✅ᵛ a64✅ᵛ |
| [`zimg`](../ports/zimg/) | Library for image scaling, colorspace conversion and dithering | 3.0.5 | `apk add zimg` | `qnx-test-zimg` | x86✅ᵛ a64✅ᵛ |
| [`zlib`](../ports/zlib/) | General purpose data compression library | 1.3.1 | `apk add zlib` | `qnx-test-zlib` | x86✅ᵛ a64✅ᵛ |
| [`zlib-ng`](../ports/zlib-ng/) | zlib data compression library for next generation systems, native API | 2.2.5 | `apk add zlib-ng` | `qnx-test-zlib-ng` | x86✅ᵛ a64✅ᵛ |
| [`zserio`](../ports/zserio/) | C++ runtime library of zserio, a serialization framework for binary data | 2.20.0 | `apk add zserio` | `qnx-test-zserio` | – |
| [`zstd`](../ports/zstd/) | Fast real-time lossless compression library (Zstandard) | 1.5.6 | `apk add zstd` | `qnx-test-zstd` | x86✅ᵛ a64✅ᵛ |

### Dependent (404)

| Component | What it is | Version | Install | Needs at run time | Needs to build | Test package | Verified |
|---|---|---|---|---|---|---|---|
| [`abuild`](../ports/abuild/) | Alpine's tools for building APK packages | 3.16.0 | `apk add abuild` | openssl zlib | openssl zlib apk-tools | `qnx-test-abuild` | x86✅ᵛ a64✅ᵛ |
| [`adwaita-icon-theme`](../ports/adwaita-icon-theme/) | GNOME's Adwaita icon theme and default cursors | 51.0 | `apk add adwaita-icon-theme` | – | hicolor-icon-theme | `qnx-test-adwaita` | x86✅ᵛ a64✅ᵛ |
| [`amqp-cpp`](../ports/amqp-cpp/) | C++ library for communicating with a RabbitMQ message broker | 4.3.27 | `apk add amqp-cpp` | – | openssl | `qnx-test-amqp-cpp` | x86✅ᵛ a64✅ᵛ |
| [`apk-tools`](../ports/apk-tools/) | Alpine Package Keeper (apk), the Alpine package manager | 3.0.0_rc5 | `apk add apk-tools` | openssl zlib zstd | openssl zlib zstd | `qnx-test-apk-tools` | x86✅ᵛ a64✅ᵛ |
| [`appstream`](../ports/appstream/) | Tools and libraries to work with AppStream metadata | 1.0.6 | `apk add appstream` | curl gettext glib libxml2 libxmlb libyaml zstd | glib curl libxml2 libyaml libxmlb zstd | `qnx-test-appstream` | x86✅ᵛ a64✅ᵛ |
| [`apr-util`](../ports/apr-util/) | Apache Portable Runtime utility library | 1.6.5 | `apk add apr-util` | apr expat libiconv libxcrypt openssl sqlite | apr expat openssl sqlite libiconv libxcrypt | `qnx-test-apr-util` | x86✅ᵛ a64✅ᵛ |
| [`asciidoc`](../ports/asciidoc/) | Text document format for short documents, articles, books and UNIX man pages | 10.2.1 | `apk add asciidoc` | – | python3 | `qnx-test-asciidoc` | x86✅ᵛ a64✅ᵛ |
| [`aspell`](../ports/aspell/) | GNU Aspell spell checker and library | 0.60.8.1 | `apk add aspell` | ncurses | ncurses | `qnx-test-aspell` | x86✅ᵛ a64✅ᵛ |
| [`at-spi2-core`](../ports/at-spi2-core/) | Accessibility infrastructure: AT-SPI D-Bus interfaces, libatspi and ATK | 2.54.0 | `apk add at-spi2-core` | dbus gettext glib libx11 libxi libxtst | glib dbus libxml2 libx11 libxtst | `qnx-test-at-spi2-core` | x86✅ᵛ a64✅ᵛ |
| [`avahi`](../ports/avahi/) | Multicast/unicast DNS-SD framework (mDNS daemon, client library and tools) | 0.9-rc3 | `apk add avahi` | dbus expat libdaemon | dbus expat libdaemon | `qnx-test-avahi` | – |
| [`avro-cpp`](../ports/avro-cpp/) | C++ library for Apache Avro, a data serialization system | 1.12.2 | `apk add avro-cpp` | snappy zlib zstd | fmt zlib snappy zstd | `qnx-test-avro-cpp` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-auth`](../ports/aws-c-auth/) | C library for AWS client-side authentication: credentials and signing | 0.10.1 | `apk add aws-c-auth` | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils openssl s2n-tls | aws-c-http aws-c-sdkutils | `qnx-test-aws-c-auth` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-cal`](../ports/aws-c-cal/) | AWS crypto abstraction layer, C wrapper for cryptography primitives | 0.9.13 | `apk add aws-c-cal` | aws-c-common openssl | aws-c-common openssl | `qnx-test-aws-c-cal` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-compression`](../ports/aws-c-compression/) | C library of compression algorithms for the AWS SDK (Huffman coding) | 0.3.2 | `apk add aws-c-compression` | aws-c-common | aws-c-common | `qnx-test-aws-c-compression` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-event-stream`](../ports/aws-c-event-stream/) | C implementation of the AWS event-stream message encoding | 0.7.0 | `apk add aws-c-event-stream` | aws-c-cal aws-c-common aws-c-io aws-checksums openssl s2n-tls | aws-c-io aws-checksums | `qnx-test-aws-c-event-stream` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-http`](../ports/aws-c-http/) | C implementation of HTTP/1.1 and HTTP/2 for the AWS SDK | 0.10.14 | `apk add aws-c-http` | aws-c-cal aws-c-common aws-c-compression aws-c-io openssl s2n-tls | aws-c-io aws-c-compression | `qnx-test-aws-c-http` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-io`](../ports/aws-c-io/) | Event-driven I/O and TLS library for the AWS SDK for C | 0.26.1 | `apk add aws-c-io` | aws-c-cal aws-c-common openssl s2n-tls | aws-c-common aws-c-cal s2n-tls | `qnx-test-aws-c-io` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-mqtt`](../ports/aws-c-mqtt/) | C implementation of the MQTT 3.1.1 and MQTT 5 protocols for AWS | 0.15.2 | `apk add aws-c-mqtt` | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io openssl s2n-tls | aws-c-http | `qnx-test-aws-c-mqtt` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-s3`](../ports/aws-c-s3/) | Asynchronous high-throughput AWS S3 client library in C | 0.12.2 | `apk add aws-c-s3` | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils aws-checksums openssl s2n-tls | aws-c-auth aws-checksums | `qnx-test-aws-c-s3` | x86✅ᵛ a64✅ᵛ |
| [`aws-c-sdkutils`](../ports/aws-c-sdkutils/) | C library of AWS SDK utilities: ARN parsing, AWS profile reading | 0.2.4 | `apk add aws-c-sdkutils` | aws-c-common | aws-c-common | `qnx-test-aws-c-sdkutils` | x86✅ᵛ a64✅ᵛ |
| [`aws-checksums`](../ports/aws-checksums/) | Hardware-accelerated CRC32 and CRC32C checksum library for the AWS SDK | 0.2.10 | `apk add aws-checksums` | aws-c-common | aws-c-common | `qnx-test-aws-checksums` | x86✅ᵛ a64✅ᵛ |
| [`aws-crt-cpp`](../ports/aws-crt-cpp/) | C++ wrapper around the AWS Common Runtime (aws-c-*) libraries | 0.38.5 | `apk add aws-crt-cpp` | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-event-stream aws-c-http aws-c-io aws-c-mqtt aws-c-s3 aws-c-sdkutils aws-checksums openssl s2n-tls | aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3 | `qnx-test-aws-crt-cpp` | x86✅ᵛ a64✅ᵛ |
| [`azure-iot-sdk-c`](../ports/azure-iot-sdk-c/) | C SDK for connecting devices to Azure IoT Hub and Device Provisioning | LTS_08_2024 | `apk add azure-iot-sdk-c` | curl openssl | openssl curl | `qnx-test-azure-iot-sdk-c` | – |
| [`bash`](../ports/bash/) | GNU Bourne Again shell | 5.2.37 | `apk add bash` | libiconv ncurses readline | ncurses readline libiconv | `qnx-test-bash` | x86✅ᵛ a64✅ᵛ |
| [`bc`](../ports/bc/) | GNU bc, an arbitrary-precision calculator language | 1.08.2 | `apk add bc` | ncurses readline | readline | `qnx-test-bc` | x86✅ᵛ a64✅ᵛ |
| [`binutils`](../ports/binutils/) | GNU binary tools: assembler, linker, objdump, readelf and others | 2.43 | `apk add binutils` | zlib zstd | zlib zstd | `qnx-test-binutils` | x86✅ᵛ a64✅ᵛ |
| [`boost`](../ports/boost/) | Boost peer-reviewed portable C++ source libraries | 1.86.0 | `apk add boost` | bzip2 xz zlib | zlib bzip2 xz | `qnx-test-boost` | x86✅ᵛ a64✅ᵛ |
| [`c-blosc`](../ports/c-blosc/) | Blocking, shuffling and lossless compression library | 1.21.6 | `apk add c-blosc` | lz4 zlib zstd | lz4 zlib zstd | `qnx-test-c-blosc` | x86✅ᵛ a64✅ᵛ |
| [`cairo`](../ports/cairo/) | 2D vector graphics library with multiple output backends | 1.18.2 | `apk add cairo` | fontconfig freetype glib libpng libx11 libxext libxrender lzo pixman zlib | pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo | `qnx-test-cairo` | x86✅ᵛ a64✅ᵛ |
| [`capicxx-someip-runtime`](../ports/capicxx-someip-runtime/) | COVESA CommonAPI C++ SOME/IP binding runtime | 3.2.4 | `apk add capicxx-someip-runtime` | capicxx-core-runtime vsomeip | capicxx-core-runtime vsomeip | `qnx-test-capicxx-someip-runtime` | x86✅ᵛ a64✅ᵛ |
| [`capnproto`](../ports/capnproto/) | Cap'n Proto data serialization format and capability-based RPC system | 1.0.2 | `apk add capnproto` | openssl zlib | zlib openssl | `qnx-test-capnproto` | x86✅ᵛ a64✅ᵛ |
| [`cartographer`](../ports/cartographer/) | Real-time 2D and 3D SLAM library across multiple platforms and sensors | 2.0.0 | `apk add cartographer` | abseil boost cairo ceres-solver gflags glog lua5.3 protobuf | abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo | `qnx-test-cartographer` | x86✅ᵛ a64✅ᵛ |
| [`ceres-solver`](../ports/ceres-solver/) | C++ library for modeling and solving large optimization problems | 2.2.0 | `apk add ceres-solver` | gflags glog | eigen glog gflags | `qnx-test-ceres-solver` | x86✅ᵛ a64✅ᵛ |
| [`civetweb`](../ports/civetweb/) | Embeddable C/C++ web server with optional Lua and WebSocket support | 1.16 | `apk add civetweb` | openssl | openssl | `qnx-test-civetweb` | x86✅ᵛ a64✅ᵛ |
| [`cmake`](../ports/cmake/) | Cross-platform build system generator | 4.1.1 | `apk add cmake` | curl libuv openssl zlib | openssl zlib curl libuv | `qnx-test-cmake` | x86✅ᵛ a64✅ᵛ |
| [`cppzmq`](../ports/cppzmq/) | Header-only C++ binding for the ZeroMQ messaging library | 4.10.0 | `apk add cppzmq` | – | libzmq | `qnx-test-libzmq` | x86✅ᵛ a64✅ᵛ |
| [`cups`](../ports/cups/) | Standards-based printing system: scheduler, libcups and command-line tools | 2.4.16 | `apk add cups` | avahi libiconv libxcrypt openssl zlib | openssl zlib avahi libiconv libxcrypt | `qnx-test-cups` | x86✅ᵛ a64✅ᵛ |
| [`cups-filters`](../ports/cups-filters/) | OpenPrinting CUPS filters | 1.28.17 | `apk add cups-filters` | avahi cups fontconfig freetype gettext glib lcms2 libexif libiconv libjpeg-turbo libpng libtiff libxcrypt openssl poppler qpdf zlib | cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib | `qnx-test-cups-filters` | x86✅ᵛ a64✅ᵛ |
| [`curl`](../ports/curl/) | Command line tool and library for transferring data with URLs | 8.11.1 | `apk add curl` | brotli c-ares nghttp2 openssl zlib zstd | openssl zlib nghttp2 c-ares brotli zstd | `qnx-test-curl` | x86✅ᵛ a64✅ᵛ |
| [`dbus`](../ports/dbus/) | D-Bus message bus daemon and library for interprocess communication | 1.16.0 | `apk add dbus` | expat | expat | `qnx-test-dbus` | x86✅ᵛ a64✅ᵛ |
| [`desktop-file-utils`](../ports/desktop-file-utils/) | Command line utilities for validating and installing .desktop files | 0.28 | `apk add desktop-file-utils` | gettext glib | glib | `qnx-test-desktop-file-utils` | x86✅ᵛ a64✅ᵛ |
| [`dlt-daemon`](../ports/dlt-daemon/) | COVESA Diagnostic Log and Trace (DLT) daemon, library and tools | 2.18.10 | `apk add dlt-daemon` | libjson | zlib libjson | `qnx-test-dlt-daemon` | x86✅ᵛ a64✅ᵛ |
| [`doxygen`](../ports/doxygen/) | Documentation generator for C, C++ and other languages | 1.17.0 | `apk add doxygen` | libiconv | libiconv | `qnx-test-doxygen` | x86✅ᵛ a64✅ᵛ |
| [`dump1090`](../ports/dump1090/) | Mode S (ADS-B) aircraft transponder decoder for RTL-SDR devices | 0-git-20240121 | `apk add dump1090` | rtl-sdr | rtl-sdr | `qnx-test-dump1090` | – |
| [`e2fsprogs`](../ports/e2fsprogs/) | Ext2/3/4 filesystem utilities | 1.47.4 | `apk add e2fsprogs` | libuuid | libuuid | `qnx-test-e2fsprogs` | x86✅ᵛ a64✅ᵛ |
| [`elfutils`](../ports/elfutils/) | Library to read and write ELF files (libelf from elfutils) | 0.193 | `apk add elfutils` | zlib | zlib | `qnx-test-elfutils` | x86✅ᵛ a64✅ᵛ |
| [`elvis`](../ports/elvis/) | Clone of the ex/vi text editor | 2.2.1-git-20240226 | `apk add elvis` | ncurses | ncurses | `qnx-test-elvis` | – |
| [`emacs`](../ports/emacs/) | GNU Emacs extensible text editor (terminal build) | 30.2 | `apk add emacs` | dbus gmp gnutls lcms2 libxml2 ncurses sqlite tree-sitter zlib | ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter | `qnx-test-emacs` | x86✅ᵛ a64✅ᵛ |
| [`emulationstation`](../ports/emulationstation/) | Graphical front-end for browsing and launching emulated games | 2.11-git-20241212 | `apk add emulationstation` | curl freeimage freetype pugixml sdl2 vlc | sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc | `qnx-test-emulationstation` | – |
| [`enchant`](../ports/enchant/) | Generic spell checking library wrapping several spell checker backends | 2.8.2 | `apk add enchant` | aspell gettext glib | glib aspell | `qnx-test-enchant` | x86✅ᵛ a64✅ᵛ |
| [`epiphany`](../ports/epiphany/) | GNOME Web, a WebKitGTK-based web browser | 43.1 | `apk add epiphany` | at-spi2-core cairo gcr gdk-pixbuf gettext glib gmp gtk3 json-glib libarchive libdazzle libhandy libportal libsecret libsoup libxml2 nettle pango sqlite webkitgtk | webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle | `qnx-test-epiphany` | x86❌ᵛ a64❌ᵛ |
| [`exiv2`](../ports/exiv2/) | C++ library and tool for reading and writing Exif, IPTC and XMP metadata | 0.28.5 | `apk add exiv2` | brotli expat libinih zlib | zlib expat brotli libinih | `qnx-test-exiv2` | x86✅ᵛ a64✅ᵛ |
| [`exo`](../ports/exo/) | Extension library and helper applications for the Xfce desktop | 4.20.0 | `apk add exo` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | glib gtk3 libxfce4util libxfce4ui | `qnx-test-exo` | x86✅ᵛ a64✅ᵛ |
| [`fastdds`](../ports/fastdds/) | eProsima Fast DDS, C++ implementation of the OMG DDS standard | 2.14.4 | `apk add fastdds` | fastcdr foonathan_memory openssl tinyxml2 | fastcdr foonathan_memory tinyxml2 openssl | `qnx-test-fastdds` | x86✅ᵛ a64✅ᵛ |
| [`fcl`](../ports/fcl/) | Flexible Collision Library for collision and proximity queries | 0.7.0 | `apk add fcl` | libccd octomap | eigen libccd octomap | `qnx-test-fcl` | x86✅ᵛ a64✅ᵛ |
| [`ffmpeg`](../ports/ffmpeg/) | Libraries and tools for decoding, encoding and processing audio and video | 7.1.1 | `apk add ffmpeg` | bzip2 dav1d lame libtheora libvorbis libvpx openh264 opus x264 x265 xz zlib | zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame | `qnx-test-ffmpeg` | x86✅ᵛ a64✅ᵛ |
| [`ffmpegthumbnailer`](../ports/ffmpegthumbnailer/) | Lightweight video thumbnailer library and tool using FFmpeg | 2.2.3 | `apk add ffmpegthumbnailer` | ffmpeg libjpeg-turbo libpng zlib | ffmpeg libpng libjpeg-turbo | `qnx-test-ffmpegthumbnailer` | x86✅ᵛ a64✅ᵛ |
| [`file`](../ports/file/) | File type identification tool and libmagic library | 5.46 | `apk add file` | bzip2 xz zlib zstd | zlib bzip2 xz zstd | `qnx-test-file` | x86✅ᵛ a64✅ᵛ |
| [`flac`](../ports/flac/) | Free Lossless Audio Codec library and tools | 1.4.3 | `apk add flac` | libogg | libogg | `qnx-test-flac` | x86✅ᵛ a64✅ᵛ |
| [`flann`](../ports/flann/) | Fast Library for Approximate Nearest Neighbors search | 1.9.2 | `apk add flann` | lz4 | lz4 | `qnx-test-flann` | x86✅ᵛ a64✅ᵛ |
| [`folly`](../ports/folly/) | Open-source C++ library developed and used at Facebook | 2026.10.26.00 | `apk add folly` | boost bzip2 fmt gflags glog libevent libsodium lz4 openssl xz zlib zstd | boost fast-float fmt gflags glog libevent openssl zlib zstd lz4 xz bzip2 libsodium | `qnx-test-folly` | x86✅ᵛ a64✅ᵛ |
| [`fontconfig`](../ports/fontconfig/) | Library for font configuration, discovery and customization | 2.15.0 | `apk add fontconfig` | expat freetype | freetype expat | `qnx-test-fontconfig` | x86✅ᵛ a64✅ᵛ |
| [`fontenc`](../ports/fontenc/) | X.Org font encoding library | 1.1.8 | `apk add fontenc` | zlib | xorgproto zlib | `qnx-test-fontenc` | x86✅ᵛ a64✅ᵛ |
| [`freetype`](../ports/freetype/) | Portable font rendering engine library | 2.13.3 | `apk add freetype` | brotli bzip2 libpng zlib | zlib bzip2 libpng brotli | `qnx-test-freetype` | x86✅ᵛ a64✅ᵛ |
| [`garcon`](../ports/garcon/) | Freedesktop.org compliant menu library for Xfce | 4.20.0 | `apk add garcon` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | glib gtk3 libxfce4util libxfce4ui | `qnx-test-garcon` | x86✅ᵛ a64✅ᵛ |
| [`gawk`](../ports/gawk/) | GNU awk pattern scanning and text processing language | 5.3.2 | `apk add gawk` | ncurses readline | readline ncurses | `qnx-test-gawk` | x86✅ᵛ a64✅ᵛ |
| [`gcr`](../ports/gcr/) | GNOME library for displaying certificates and accessing key stores | 3.41.2 | `apk add gcr` | cairo gdk-pixbuf gettext glib gtk3 libgcrypt p11-kit pango | glib libgcrypt p11-kit libsecret gtk3 | `qnx-test-gcr` | x86✅ᵛ a64✅ᵛ |
| [`gdb`](../ports/gdb/) | GNU debugger | 14.2 | `apk add gdb` | expat gmp libiconv mpfr ncurses readline xxhash xz zlib zstd | gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd | `qnx-test-gdb` | x86✅ᵛ a64✅ᵛ |
| [`gdk-pixbuf`](../ports/gdk-pixbuf/) | GTK library for loading and scaling images | 2.42.12 | `apk add gdk-pixbuf` | gettext glib libjpeg-turbo libpng | glib libpng libjpeg-turbo | `qnx-test-gdk-pixbuf` | x86✅ᵛ a64✅ᵛ |
| [`geany`](../ports/geany/) | Fast and lightweight IDE using GTK | 2.1 | `apk add geany` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango zlib | glib gtk3 | `qnx-test-geany` | x86✅ᵛ a64✅ᵛ |
| [`geany-plugins`](../ports/geany-plugins/) | Collection of plugins for the Geany IDE | 2.1 | `apk add geany-plugins` | at-spi2-core cairo gdk-pixbuf geany gettext glib gtk3 harfbuzz pango zlib | geany | `qnx-test-geany-plugins` | x86✅ᵛ a64✅ᵛ |
| [`gettext`](../ports/gettext/) | GNU internationalization tools and libraries for translated messages | 0.22.5 | `apk add gettext` | libiconv libunistring ncurses | ncurses libiconv libunistring | `qnx-test-gettext` | x86✅ᵛ a64✅ᵛ |
| [`git`](../ports/git/) | Fast distributed version control system | 2.51.0 | `apk add git` | curl expat libiconv openssl pcre2 zlib | zlib openssl curl expat pcre2 perl perl-error libiconv | `qnx-test-git` | x86✅ᵛ a64✅ᵛ |
| [`glfw`](../ports/glfw/) | Multi-platform library for OpenGL, OpenGL ES and Vulkan windowing | 3.5.1 | `apk add glfw` | – | wayland libxkbcommon linux-headers-input-codes | `qnx-test-glfw` | x86✅ᵛ a64✅ᵛ |
| [`glib`](../ports/glib/) | Low-level core C library of GTK and GNOME: data types, main loop, GIO | 2.82.4 | `apk add glib` | gettext libffi libiconv pcre2 zlib | libffi pcre2 gettext zlib | `qnx-test-glib` | x86✅ᵛ a64✅ᵛ |
| [`glib-networking`](../ports/glib-networking/) | TLS (GnuTLS) and proxy support modules for GLib's GIO | 2.80.1 | `apk add glib-networking` | gettext glib gnutls libproxy | glib gnutls libproxy | `qnx-test-glib-networking` | x86✅ᵛ a64✅ᵛ |
| [`glog`](../ports/glog/) | Google's C++ application-level logging library | 0.7.1 | `apk add glog` | gflags | gflags | `qnx-test-glog` | x86✅ᵛ a64✅ᵛ |
| [`gnupg`](../ports/gnupg/) | GNU Privacy Guard 2: OpenPGP and S/MIME encryption and signing tools | 2.4.9 | `apk add gnupg` | bzip2 gnutls libassuan libgcrypt libgpg-error libiconv libksba ncurses npth readline sqlite zlib | libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv | `qnx-test-gnupg` | x86✅ᵛ a64✅ᵛ |
| [`gnutls`](../ports/gnutls/) | GNU TLS/SSL protocol implementation library | 3.8.8 | `apk add gnutls` | brotli gmp libidn2 libtasn1 libunistring nettle zlib zstd | nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd | `qnx-test-gnutls` | x86✅ᵛ a64✅ᵛ |
| [`gobject-introspection`](../ports/gobject-introspection/) | GObject Introspection library and tools for API metadata | 1.86.0 | `apk add gobject-introspection` | glib libffi | glib libffi python3 | `qnx-test-gobject-introspection` | x86✅ᵛ a64✅ᵛ |
| [`godot`](../ports/godot/) | Godot game engine runtime (export templates) for 2D and 3D games | 4.4.1 | `apk add godot` | brotli bzip2 freetype gettext glib graphite2 harfbuzz libiconv libogg libpng libtheora libvorbis libwebp libxkbcommon pcre2 zlib zstd | freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon | `qnx-test-godot` | x86✅ᵛ a64✅ᵛ |
| [`gpgme`](../ports/gpgme/) | GnuPG Made Easy | 2.2.0 | `apk add gpgme` | libassuan libgpg-error | libgpg-error libassuan | `qnx-test-gpgme` | x86✅ᵛ a64✅ᵛ |
| [`graphene`](../ports/graphene/) | Thin layer of math types (vectors, matrices, points) for graphics libraries | 1.10.8 | `apk add graphene` | glib | glib | `qnx-test-graphene` | x86✅ᵛ a64✅ᵛ |
| [`graphicsmagick`](../ports/graphicsmagick/) | Image processing tools and libraries (gm, Magick++) | 1.3.46 | `apk add graphicsmagick` | bzip2 freetype lcms2 libjpeg-turbo libjxl libpng libtiff libwebp libxml2 libzip xz zlib zstd | libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2 | `qnx-test-graphicsmagick` | x86✅ᵛ a64✅ᵛ |
| [`grep`](../ports/grep/) | GNU grep, searches files for lines matching a pattern | 3.11 | `apk add grep` | libiconv pcre2 | pcre2 libiconv | `qnx-test-grep` | x86✅ᵛ a64✅ᵛ |
| [`groff`](../ports/groff/) | GNU troff document formatting system | 1.23.0 | `apk add groff` | libiconv | libiconv | `qnx-test-groff` | x86✅ᵛ a64✅ᵛ |
| [`grpc`](../ports/grpc/) | Remote procedure call (RPC) framework from Google | 1.69.0 | `apk add grpc` | abseil c-ares openssl protobuf re2 zlib | zlib openssl c-ares abseil protobuf re2 | `qnx-test-grpc` | x86✅ᵛ a64✅ᵛ |
| [`gsettings-desktop-schemas`](../ports/gsettings-desktop-schemas/) | GSettings schemas for settings shared by desktop components | 49.0 | `apk add gsettings-desktop-schemas` | – | glib | `qnx-test-gsettings-desktop-schemas` | x86✅ᵛ a64✅ᵛ |
| [`gst-libav`](../ports/gst-libav/) | GStreamer plugin for FFmpeg (libav) codecs and formats | 1.24.10 | `apk add gst-libav` | ffmpeg glib gst-plugins-base gstreamer | gst-plugins-base ffmpeg | `qnx-test-gst-libav` | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-bad`](../ports/gst-plugins-bad/) | GStreamer plugins of lesser quality or maturity (bad set) | 1.24.10 | `apk add gst-plugins-bad` | aom cairo fdk-aac glib gst-plugins-base gstreamer libwebp openh264 pango svt-av1 x265 | gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo | `qnx-test-gst-plugins-bad` | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-base`](../ports/gst-plugins-base/) | Essential GStreamer plugins and helper libraries (base set) | 1.24.10 | `apk add gst-plugins-base` | gettext glib gstreamer libogg libtheora libvorbis opus zlib | gstreamer orc libogg libvorbis opus libtheora zlib | `qnx-test-gst-plugins-base` | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-good`](../ports/gst-plugins-good/) | Well-supported GStreamer plugins under preferred licenses (good set) | 1.24.10 | `apk add gst-plugins-good` | bzip2 cairo flac gdk-pixbuf gettext glib gst-plugins-base gstreamer lame libjpeg-turbo libpng libsoup libvpx zlib | gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2 | `qnx-test-gst-plugins-good` | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-ugly`](../ports/gst-plugins-ugly/) | Quality GStreamer plugins with possible licensing issues (ugly set) | 1.24.10 | `apk add gst-plugins-ugly` | gettext glib gst-plugins-base gstreamer x264 | gst-plugins-base x264 | `qnx-test-gst-plugins-ugly` | x86✅ᵛ a64✅ᵛ |
| [`gstreamer`](../ports/gstreamer/) | GStreamer streaming media framework core | 1.24.10 | `apk add gstreamer` | gettext glib | glib libxml2 | `qnx-test-gstreamer` | x86✅ᵛ a64✅ᵛ |
| [`gthumb`](../ports/gthumb/) | Image viewer and organizer for GNOME | 3.12.8.2 | `apk add gthumb` | cairo exiv2 gdk-pixbuf gettext glib gtk3 lcms2 libjpeg-turbo libpng libtiff libwebp pango zlib | glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib | `qnx-test-gthumb` | x86✅ᵛ a64✅ᵛ |
| [`gtk-layer-shell`](../ports/gtk-layer-shell/) | Library for using the Wayland layer shell protocol with GTK 3 | 0.9.0 | `apk add gtk-layer-shell` | cairo glib gtk3 wayland | glib gtk3 wayland wayland-protocols | `qnx-test-gtk-layer-shell` | x86✅ᵛ a64✅ᵛ |
| [`gtk-vnc`](../ports/gtk-vnc/) | VNC client library and GTK 3 widget, with the gvncviewer viewer | 1.3.1 | `apk add gtk-vnc` | cairo gdk-pixbuf gettext glib gnutls gtk3 libgcrypt libx11 zlib | glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11 | `qnx-test-gtk-vnc` | x86✅ᵛ a64✅ᵛ |
| [`gtk3`](../ports/gtk3/) | GTK 3 multi-platform toolkit for graphical user interfaces | 3.24.43 | `apk add gtk3` | at-spi2-core cairo fontconfig fribidi gdk-pixbuf gettext glib harfbuzz libepoxy libx11 libxcomposite libxcursor libxdamage libxext libxfixes libxi libxinerama libxkbcommon libxrandr pango wayland | glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama | `qnx-test-gtk3` | x86✅ᵛ a64✅ᵛ |
| [`gtk4`](../ports/gtk4/) | GTK 4 multi-platform toolkit for graphical user interfaces | 4.16.7 | `apk add gtk4` | cairo fontconfig fribidi gdk-pixbuf gettext glib graphene harfbuzz libepoxy libjpeg-turbo libpng libtiff libxkbcommon pango wayland | glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff | `qnx-test-gtk4` | x86✅ᵛ a64✅ᵛ |
| [`gtsam`](../ports/gtsam/) | Georgia Tech Smoothing and Mapping library for robotics and SLAM | 4.2 | `apk add gtsam` | boost | boost eigen | `qnx-test-gtsam` | x86✅ᵛ a64✅ᵛ |
| [`harfbuzz`](../ports/harfbuzz/) | OpenType text shaping engine | 10.1.0 | `apk add harfbuzz` | freetype glib graphite2 icu | freetype glib icu graphite2 | `qnx-test-harfbuzz` | x86✅ᵛ a64✅ᵛ |
| [`htop`](../ports/htop/) | Interactive process viewer | 3.5.0 | `apk add htop` | ncurses | ncurses | `qnx-test-monitors` | x86✅ᵛ a64✅ᵛ |
| [`inetutils`](../ports/inetutils/) | A collection of common network programs (GNU inetutils) | 2.6 | `apk add inetutils` | libxcrypt ncurses readline | ncurses readline libxcrypt | `qnx-test-inetutils` | x86✅ᵛ a64✅ᵛ |
| [`iperf3`](../ports/iperf3/) | TCP, UDP and SCTP network bandwidth measurement tool | 3.17.1 | `apk add iperf3` | openssl | openssl | `qnx-test-iperf3` | x86✅ᵛ a64✅ᵛ |
| [`ippusbxd`](../ports/ippusbxd/) | Daemon for IPP over USB printer support | 1.34 | `apk add ippusbxd` | avahi cups libusb libxml2 | libusb avahi libxml2 cups | `qnx-test-ippusbxd` | x86✅ᵛ a64✅ᵛ |
| [`itstool`](../ports/itstool/) | Translates XML documents with PO files using W3C ITS rules | 2.0.7 | `apk add itstool` | – | python3 libxml2 | `qnx-test-itstool` | x86✅ᵛ a64✅ᵛ |
| [`javascriptcore`](../ports/javascriptcore/) | WebKit's JavaScriptCore JavaScript engine (standalone JSCOnly port) | 2.44.4 | `apk add javascriptcore` | icu | icu | `qnx-test-javascriptcore` | x86✅ᵛ a64✅ᵛ |
| [`jq`](../ports/jq/) | Command line JSON processor and libjq library | 1.7.1 | `apk add jq` | oniguruma | oniguruma | `qnx-test-jq` | x86✅ᵛ a64✅ᵛ |
| [`json-glib`](../ports/json-glib/) | JSON parser and generator library for GLib | 1.10.6 | `apk add json-glib` | gettext glib | glib | `qnx-test-json-glib` | x86✅ᵛ a64✅ᵛ |
| [`labwc`](../ports/labwc/) | Stacking Wayland compositor (window manager) based on wlroots | 0.20.2 | `apk add labwc` | cairo glib libpng libsfdo libxkbcommon libxml2 pango pixman wayland wlroots | wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`lcm`](../ports/lcm/) | Lightweight Communications and Marshalling: message passing for real time | 1.5.2 | `apk add lcm` | glib | glib | `qnx-test-lcm` | x86✅ᵛ a64✅ᵛ |
| [`lcms2`](../ports/lcms2/) | Little CMS, small-footprint ICC color management engine | 2.16 | `apk add lcms2` | – | zlib | `qnx-test-lcms2` | x86✅ᵛ a64✅ᵛ |
| [`less`](../ports/less/) | Terminal pager for viewing text files one screen at a time | 668 | `apk add less` | ncurses | ncurses | `qnx-test-less` | x86✅ᵛ a64✅ᵛ |
| [`leveldb`](../ports/leveldb/) | Fast ordered key-value storage library from Google | 1.23 | `apk add leveldb` | snappy | snappy | `qnx-test-leveldb` | x86✅ᵛ a64✅ᵛ |
| [`libadwaita`](../ports/libadwaita/) | Building blocks for modern GNOME applications | 1.6.10 | `apk add libadwaita` | appstream fribidi gettext glib graphene gtk4 pango | glib gtk4 fribidi appstream | `qnx-test-libadwaita` | x86✅ᵛ a64✅ᵛ |
| [`libarchive`](../ports/libarchive/) | Library and tools (bsdtar, bsdcpio) for reading and writing archives | 3.7.7 | `apk add libarchive` | bzip2 expat lz4 openssl xz zlib zstd | zlib bzip2 xz zstd openssl libxml2 expat lz4 | `qnx-test-libarchive` | x86✅ᵛ a64✅ᵛ |
| [`libass`](../ports/libass/) | Subtitle renderer library for the ASS/SSA subtitle format | 0.17.3 | `apk add libass` | fontconfig freetype fribidi harfbuzz | freetype fribidi harfbuzz fontconfig | `qnx-test-libass` | x86✅ᵛ a64✅ᵛ |
| [`libassuan`](../ports/libassuan/) | IPC library used by GnuPG and related software | 3.0.2 | `apk add libassuan` | libgpg-error | libgpg-error | `qnx-test-libassuan` | x86✅ᵛ a64✅ᵛ |
| [`libavif`](../ports/libavif/) | Library for encoding and decoding AVIF images | 1.1.1 | `apk add libavif` | aom dav1d | aom dav1d libyuv | `qnx-test-libavif` | x86✅ᵛ a64✅ᵛ |
| [`libcamera`](../ports/libcamera/) | Raspberry Pi camera image processing algorithms from libcamera | 0.5.1+rpt20250722 | `apk add libcamera` | libyaml | libyaml | `qnx-test-libcamera` | – |
| [`libcoap`](../ports/libcoap/) | C implementation of the Constrained Application Protocol (CoAP) | 4.3.5 | `apk add libcoap` | openssl | openssl | `qnx-test-libcoap` | x86✅ᵛ a64✅ᵛ |
| [`libdazzle`](../ports/libdazzle/) | Companion library of extra widgets and utilities for GObject and GTK 3 | 3.44.0 | `apk add libdazzle` | cairo gdk-pixbuf gettext glib gtk3 pango | glib gtk3 | `qnx-test-libdazzle` | x86✅ᵛ a64✅ᵛ |
| [`libedit`](../ports/libedit/) | Command line editor library: line editing, history and tokenization | 20240808-3.1 | `apk add libedit` | ncurses | ncurses | `qnx-test-libedit` | x86✅ a64✅ |
| [`libepoxy`](../ports/libepoxy/) | Library for managing OpenGL, EGL and GLX function pointers | 1.5.10 | `apk add libepoxy` | – | libx11 | `qnx-test-libepoxy` | x86✅ᵛ a64✅ᵛ |
| [`libevent`](../ports/libevent/) | Asynchronous event notification library | 2.1.12 | `apk add libevent` | openssl | openssl | `qnx-test-libevent` | x86✅ᵛ a64✅ᵛ |
| [`libgcrypt`](../ports/libgcrypt/) | GNU general purpose cryptographic library | 1.11.0 | `apk add libgcrypt` | libgpg-error | libgpg-error | `qnx-test-libgcrypt` | x86✅ᵛ a64✅ᵛ |
| [`libgit2`](../ports/libgit2/) | Portable C library implementing the Git core methods | 1.8.4 | `apk add libgit2` | openssl pcre2 zlib | openssl zlib pcre2 | `qnx-test-libgit2` | x86✅ᵛ a64✅ᵛ |
| [`libgpg-error`](../ports/libgpg-error/) | Common error values and runtime library for GnuPG components | 1.50 | `apk add libgpg-error` | gettext | gettext | `qnx-test-libgpg-error` | x86✅ᵛ a64✅ᵛ |
| [`libgsf`](../ports/libgsf/) | Library for reading and writing structured files (MS OLE, Zip) | 1.14.53 | `apk add libgsf` | bzip2 gettext glib libxml2 zlib | glib libxml2 zlib bzip2 | `qnx-test-libgsf` | x86✅ᵛ a64✅ᵛ |
| [`libhandy`](../ports/libhandy/) | GTK 3 widgets for adaptive and mobile user interfaces | 1.8.3 | `apk add libhandy` | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gtk3 pango | glib gtk3 | `qnx-test-libhandy` | x86✅ᵛ a64✅ᵛ |
| [`libice`](../ports/libice/) | X Inter-Client Exchange (ICE) protocol library | 1.1.2 | `apk add libice` | – | xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libidn2`](../ports/libidn2/) | Library for internationalized domain names (IDNA2008, Punycode, TR46) | 2.3.7 | `apk add libidn2` | gettext libunistring | libunistring gettext | `qnx-test-libidn2` | x86✅ᵛ a64✅ᵛ |
| [`libjxl`](../ports/libjxl/) | JPEG XL image format reference library and tools | 0.11.1 | `apk add libjxl` | brotli highway lcms2 | brotli highway lcms2 libpng giflib libjpeg-turbo | `qnx-test-libjxl` | x86✅ᵛ a64✅ᵛ |
| [`libksba`](../ports/libksba/) | Library to access X.509 certificates and CMS data | 1.8.1 | `apk add libksba` | libgpg-error | libgpg-error | `qnx-test-libksba` | x86✅ᵛ a64✅ᵛ |
| [`libnotify`](../ports/libnotify/) | Library for sending desktop notifications | 0.8.3 | `apk add libnotify` | gdk-pixbuf glib | glib gdk-pixbuf | `qnx-test-libnotify` | x86✅ᵛ a64✅ᵛ |
| [`libpanel`](../ports/libpanel/) | Dock/panel library for GTK 4 | 1.10.4 | `apk add libpanel` | cairo gettext glib graphene gtk4 libadwaita | glib gtk4 libadwaita | `qnx-test-libpanel` | x86✅ᵛ a64✅ᵛ |
| [`libpng`](../ports/libpng/) | C library for reading and writing PNG images | 1.6.44 | `apk add libpng` | zlib | zlib | `qnx-test-libpng` | x86✅ᵛ a64✅ᵛ |
| [`libportal`](../ports/libportal/) | GIO-style client library for XDG desktop portals | 0.8.1 | `apk add libportal` | glib gtk3 | glib gtk3 | `qnx-test-libportal` | x86✅ᵛ a64✅ᵛ |
| [`libproxy`](../ports/libproxy/) | Library for automatic proxy configuration management | 0.5.9 | `apk add libproxy` | curl glib | glib curl | `qnx-test-libproxy` | x86✅ᵛ a64✅ᵛ |
| [`libpsl`](../ports/libpsl/) | C library for handling the Public Suffix List | 0.21.5 | `apk add libpsl` | libiconv libidn2 libunistring | libidn2 libunistring libiconv | `qnx-test-libpsl` | x86✅ᵛ a64✅ᵛ |
| [`libraqm`](../ports/libraqm/) | Library for complex text layout (bidirectional text, shaping) | 0.10.5 | `apk add libraqm` | freetype fribidi harfbuzz | freetype harfbuzz fribidi | `qnx-test-libraqm` | x86✅ᵛ a64✅ᵛ |
| [`libsecret`](../ports/libsecret/) | GObject library for storing and retrieving secrets via Secret Service | 0.21.4 | `apk add libsecret` | glib libgcrypt | glib libgcrypt | `qnx-test-libsecret` | x86✅ᵛ a64✅ᵛ |
| [`libsm`](../ports/libsm/) | X Session Management library | 1.2.6 | `apk add libsm` | libice | libice xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libsndfile`](../ports/libsndfile/) | C library for reading and writing sampled audio files | 1.2.2 | `apk add libsndfile` | flac libogg libvorbis opus | libogg libvorbis flac opus | `qnx-test-libsndfile` | x86✅ᵛ a64✅ᵛ |
| [`libsoup`](../ports/libsoup/) | GLib-based HTTP client and server library | 3.6.1 | `apk add libsoup` | brotli gettext glib libpsl nghttp2 sqlite zlib | glib libxml2 sqlite nghttp2 libpsl brotli | `qnx-test-libsoup` | x86✅ᵛ a64✅ᵛ |
| [`libssh`](../ports/libssh/) | C library implementing the SSH client and server protocols | 0.11.1 | `apk add libssh` | openssl zlib | openssl zlib | `qnx-test-libssh` | x86✅ᵛ a64✅ᵛ |
| [`libssh2`](../ports/libssh2/) | Client-side C library implementing the SSH2 protocol | 1.11.1 | `apk add libssh2` | openssl zlib | openssl zlib | `qnx-test-libssh2` | x86✅ᵛ a64✅ᵛ |
| [`libthai`](../ports/libthai/) | Thai language support library (word breaking, input, sorting) | 0.1.29 | `apk add libthai` | libdatrie | libdatrie | `qnx-test-libthai` | x86✅ᵛ a64✅ᵛ |
| [`libtheora`](../ports/libtheora/) | Theora video codec library | 1.1.1 | `apk add libtheora` | libogg | libogg libvorbis | `qnx-test-libtheora` | x86✅ᵛ a64✅ᵛ |
| [`libtiff`](../ports/libtiff/) | Library for reading and writing TIFF images | 4.7.0 | `apk add libtiff` | libjpeg-turbo libwebp xz zlib zstd | zlib xz zstd libjpeg-turbo libwebp | `qnx-test-libtiff` | x86✅ᵛ a64✅ᵛ |
| [`libtomcrypt`](../ports/libtomcrypt/) | Public-domain modular cryptographic toolkit written in C | 1.18.2 | `apk add libtomcrypt` | libtommath | libtommath | `qnx-test-libtomcrypt` | x86✅ᵛ a64✅ᵛ |
| [`libvncserver`](../ports/libvncserver/) | C libraries for implementing VNC servers and clients | 0.9.15 | `apk add libvncserver` | gnutls libjpeg-turbo libpng lzo openssl sdl2 zlib | zlib lzo libjpeg-turbo libpng gnutls openssl sdl2 | `qnx-test-libvncserver` | x86✅ᵛ a64✅ᵛ |
| [`libvorbis`](../ports/libvorbis/) | Ogg Vorbis audio codec libraries | 1.3.7 | `apk add libvorbis` | libogg | libogg | `qnx-test-libvorbis` | x86✅ᵛ a64✅ᵛ |
| [`libwebp`](../ports/libwebp/) | Library and tools for the WebP image format | 1.4.0 | `apk add libwebp` | – | libpng libjpeg-turbo giflib | `qnx-test-libwebp` | x86✅ᵛ a64✅ᵛ |
| [`libwebsockets`](../ports/libwebsockets/) | Lightweight C library for HTTP, WebSocket and MQTT clients and servers | 4.3.5 | `apk add libwebsockets` | openssl | openssl zlib | `qnx-test-libwebsockets` | x86✅ᵛ a64✅ᵛ |
| [`libx11`](../ports/libx11/) | Core X11 protocol client library (Xlib) | 1.8.10 | `apk add libx11` | libxcb | libxcb xorgproto xtrans | `qnx-test-libx11` | x86✅ᵛ a64✅ᵛ |
| [`libxau`](../ports/libxau/) | X authorization file management library | 1.0.11 | `apk add libxau` | – | xorgproto | `qnx-test-libxau` | x86✅ᵛ a64✅ᵛ |
| [`libxcb`](../ports/libxcb/) | X protocol C-language binding (XCB) libraries | 1.17.0 | `apk add libxcb` | libxau libxdmcp | xcb-proto libxau libxdmcp xtrans pthread-stubs | `qnx-test-libxcb` | x86✅ᵛ a64✅ᵛ |
| [`libxcomposite`](../ports/libxcomposite/) | X Composite extension client library | 0.4.6 | `apk add libxcomposite` | libx11 | libx11 libxfixes xorgproto | `qnx-test-libxcomposite` | x86✅ᵛ a64✅ᵛ |
| [`libxcursor`](../ports/libxcursor/) | X cursor management library | 1.2.2 | `apk add libxcursor` | libx11 libxfixes libxrender | libx11 libxrender libxfixes xorgproto | `qnx-test-libxcursor` | x86✅ᵛ a64✅ᵛ |
| [`libxdamage`](../ports/libxdamage/) | X Damage extension client library | 1.1.6 | `apk add libxdamage` | libx11 libxfixes | libx11 libxfixes xorgproto | `qnx-test-libxdamage` | x86✅ᵛ a64✅ᵛ |
| [`libxdmcp`](../ports/libxdmcp/) | X Display Manager Control Protocol library | 1.1.5 | `apk add libxdmcp` | – | xorgproto | `qnx-test-libxdmcp` | x86✅ᵛ a64✅ᵛ |
| [`libxext`](../ports/libxext/) | X11 library for common protocol extensions | 1.3.6 | `apk add libxext` | libx11 | libx11 xorgproto | `qnx-test-libxext` | x86✅ᵛ a64✅ᵛ |
| [`libxfce4ui`](../ports/libxfce4ui/) | Shared widgets library for Xfce applications | 4.20.0 | `apk add libxfce4ui` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4util pango xfconf zlib | glib gtk3 libxfce4util xfconf libepoxy | `qnx-test-libxfce4ui` | x86✅ᵛ a64✅ᵛ |
| [`libxfce4util`](../ports/libxfce4util/) | Non-GUI utility library for the Xfce desktop | 4.20.0 | `apk add libxfce4util` | gettext glib | glib | `qnx-test-libxfce4util` | x86✅ᵛ a64✅ᵛ |
| [`libxfce4windowing`](../ports/libxfce4windowing/) | X11/Wayland windowing abstraction library for Xfce | 4.20.7 | `apk add libxfce4windowing` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango wayland zlib | glib gtk3 wayland wayland-protocols | `qnx-test-libxfce4windowing` | x86✅ᵛ a64✅ᵛ |
| [`libxfixes`](../ports/libxfixes/) | X Fixes extension client library | 6.0.1 | `apk add libxfixes` | libx11 | libx11 xorgproto | `qnx-test-libxfixes` | x86✅ᵛ a64✅ᵛ |
| [`libxfont2`](../ports/libxfont2/) | X font handling library for X servers and utilities | 2.0.7 | `apk add libxfont2` | fontenc freetype zlib | freetype zlib fontenc xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxft`](../ports/libxft/) | X FreeType library for anti-aliased font rendering in X clients | 2.3.8 | `apk add libxft` | fontconfig freetype libx11 libxrender | libx11 libxrender freetype fontconfig xorgproto | `qnx-test-libxft` | x86✅ᵛ a64✅ᵛ |
| [`libxi`](../ports/libxi/) | X Input extension client library | 1.8.2 | `apk add libxi` | libx11 libxext | libx11 libxext libxfixes xorgproto | `qnx-test-libxi` | x86✅ᵛ a64✅ᵛ |
| [`libxinerama`](../ports/libxinerama/) | Xinerama extension client library for multi-monitor X displays | 1.1.5 | `apk add libxinerama` | libx11 libxext | libx11 libxext xorgproto | `qnx-test-libxinerama` | x86✅ᵛ a64✅ᵛ |
| [`libxkbcommon`](../ports/libxkbcommon/) | Keyboard keymap compiler and support library (XKB) | 1.8.1 | `apk add libxkbcommon` | libxml2 | libxml2 | `qnx-test-libxkbcommon` | x86✅ᵛ a64✅ᵛ |
| [`libxkbfile`](../ports/libxkbfile/) | Library for reading, writing and compiling XKB keyboard descriptions | 1.2.0 | `apk add libxkbfile` | libx11 | libx11 xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxml2`](../ports/libxml2/) | XML parser and toolkit library in C | 2.13.5 | `apk add libxml2` | xz zlib | zlib xz python3 | `qnx-test-libxml2` | x86✅ᵛ a64✅ᵛ |
| [`libxml2-utils`](../ports/libxml2-utils/) | xmllint and xmlcatalog, the command-line tools of libxml2 | 2.13.5 | `apk add libxml2-utils` | libxml2 | libxml2 zlib xz | `qnx-test-libxml2-utils` | x86✅ᵛ a64✅ᵛ |
| [`libxmlb`](../ports/libxmlb/) | Library to help create and query binary XML blobs | 0.3.24 | `apk add libxmlb` | glib xz zstd | glib xz zstd | `qnx-test-libxmlb` | x86✅ᵛ a64✅ᵛ |
| [`libxmu`](../ports/libxmu/) | X miscellaneous utility routines library | 1.3.1 | `apk add libxmu` | libx11 libxext libxt | libx11 libxext libxt xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxrandr`](../ports/libxrandr/) | X Resize, Rotate and Reflect (RandR) extension client library | 1.5.4 | `apk add libxrandr` | libx11 libxext libxrender | libx11 libxext libxrender xorgproto | `qnx-test-libxrandr` | x86✅ᵛ a64✅ᵛ |
| [`libxrender`](../ports/libxrender/) | X Render extension client library | 0.9.11 | `apk add libxrender` | libx11 | libx11 xorgproto | `qnx-test-libxrender` | x86✅ᵛ a64✅ᵛ |
| [`libxscrnsaver`](../ports/libxscrnsaver/) | X11 Screen Saver (MIT-SCREEN-SAVER) extension client library | 1.2.4 | `apk add libxscrnsaver` | libx11 libxext | libx11 libxext xorgproto | `qnx-test-libxscrnsaver` | x86✅ᵛ a64✅ᵛ |
| [`libxshmfence`](../ports/libxshmfence/) | Shared memory fence synchronization library for X | 1.3.3 | `apk add libxshmfence` | – | xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxslt`](../ports/libxslt/) | XSLT processor library based on libxml2, with EXSLT extensions | 1.1.42 | `apk add libxslt` | libxml2 | libxml2 zlib | `qnx-test-libxslt` | x86✅ᵛ a64✅ᵛ |
| [`libxt`](../ports/libxt/) | X Toolkit Intrinsics library (Xt) | 1.3.1 | `apk add libxt` | libice libsm libx11 | libx11 libsm libice xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxtst`](../ports/libxtst/) | X XTEST and RECORD extensions client library | 1.2.5 | `apk add libxtst` | libx11 libxext libxi | libx11 libxext libxfixes libxi xorgproto | `qnx-test-libxtst` | x86✅ᵛ a64✅ᵛ |
| [`libyuv`](../ports/libyuv/) | Library for YUV scaling and color space conversion | stable | `apk add libyuv` | libjpeg-turbo | libjpeg-turbo | `qnx-test-libyuv` | x86✅ a64✅ |
| [`libzip`](../ports/libzip/) | C library for reading, creating and modifying zip archives | 1.11.1 | `apk add libzip` | bzip2 openssl xz zlib zstd | zlib bzip2 xz zstd openssl | `qnx-test-libzip` | x86✅ᵛ a64✅ᵛ |
| [`libzmq`](../ports/libzmq/) | ZeroMQ lightweight messaging library | 4.3.5 | `apk add libzmq` | libsodium | libsodium | `qnx-test-libzmq` | x86✅ᵛ a64✅ᵛ |
| [`lighttpd`](../ports/lighttpd/) | Lightweight, flexible web server | 1.4.77 | `apk add lighttpd` | libxcrypt openssl pcre2 zlib | openssl pcre2 zlib | `qnx-test-lighttpd` | x86✅ᵛ a64✅ᵛ |
| [`llama.cpp`](../ports/llama.cpp/) | LLM inference in C/C++ | 0_b9006 | `apk add llama.cpp` | openssl | – | `qnx-test-llama-cpp` | – |
| [`lldpd`](../ports/lldpd/) | Implementation of IEEE 802.1ab (LLDP) | 1.0.22 | `apk add lldpd` | libevent | libevent | `qnx-test-lldpd` | x86✅ᵛ a64✅ᵛ |
| [`llvm`](../ports/llvm/) | LLVM compiler infrastructure with the Clang compiler and LLD linker | 22.1.8 | `apk add llvm` | libxml2 zlib zstd | zlib zstd libffi libxml2 | `qnx-test-llvm` | x86✅ᵛ a64✅ᵛ |
| [`log4cxx`](../ports/log4cxx/) | C++ logging framework modeled after Apache log4j | 1.8.0 | `apk add log4cxx` | apr apr-util expat libiconv | apr apr-util expat libiconv | `qnx-test-log4cxx` | x86✅ᵛ a64✅ᵛ |
| [`lua-lpeg`](../ports/lua-lpeg/) | Parsing Expression Grammars (LPeg) module for Lua | 1.1.0 | `apk add lua-lpeg` | – | luajit | `qnx-test-lua-lpeg` | x86✅ᵛ a64✅ᵛ |
| [`luv`](../ports/luv/) | libuv bindings for Lua and LuaJIT | 1.51.0-1 | `apk add luv` | libuv | libuv luajit | `qnx-test-luv` | – |
| [`m4`](../ports/m4/) | GNU implementation of the Unix m4 macro processor | 1.4.19 | `apk add m4` | libiconv | libiconv | `qnx-test-m4` | x86✅ᵛ a64✅ᵛ |
| [`mame`](../ports/mame/) | MAME multi-purpose arcade and computer emulator | 0.281 | `apk add mame` | fontconfig freetype sdl2 sdl2-ttf zlib | sdl2 sdl2-ttf fontconfig freetype zlib | `qnx-test-mame` | x86✅ᵛ a64✅ᵛ |
| [`man-db`](../ports/man-db/) | Manual page reader suite (man, apropos, whatis) | 2.13.1 | `apk add man-db` | gdbm libiconv libpipeline zlib | libpipeline gdbm groff libiconv zlib | `qnx-test-man-db` | x86✅ᵛ a64✅ᵛ |
| [`mate-calc`](../ports/mate-calc/) | MATE desktop calculator | 1.28.0 | `apk add mate-calc` | at-spi2-core cairo gdk-pixbuf gettext glib gmp gtk3 harfbuzz libxml2 mpc mpfr pango zlib | glib gtk3 libxml2 gmp mpfr mpc | `qnx-test-mate-calc` | x86✅ᵛ a64✅ᵛ |
| [`mcap-cpp`](../ports/mcap-cpp/) | Header-only C++ library for the MCAP robotics message log format | 2.1.3 | `apk add mcap-cpp` | – | lz4 zstd | `qnx-test-mcap` | x86✅ᵛ a64✅ᵛ |
| [`mesa`](../ports/mesa/) | Mesa 3D: software OpenGL, GLES, EGL (llvmpipe) and Vulkan (lavapipe) | 26.2.6 | `apk add mesa` | expat llvm wayland zlib zstd | llvm wayland wayland-protocols zlib zstd expat | `qnx-test-mesa-wayland` | x86✅ᵛ a64✅ᵛ |
| [`meson`](../ports/meson/) | Meson build system | 1.10.0 | `apk add meson` | – | python3 | `qnx-test-meson` | x86✅ᵛ a64✅ᵛ |
| [`metis`](../ports/metis/) | Programs and library for partitioning graphs and finite element meshes | 5.2.1 | `apk add metis` | gklib | gklib | `qnx-test-metis` | x86✅ᵛ a64✅ᵛ |
| [`minicom`](../ports/minicom/) | Menu-driven serial communication program and terminal emulator | 2.9 | `apk add minicom` | libiconv ncurses | ncurses libiconv | `qnx-test-minicom` | x86✅ᵛ a64✅ᵛ |
| [`mkfontscale`](../ports/mkfontscale/) | Creates fonts.scale and fonts.dir index files for X11 font directories | 1.2.3 | `apk add mkfontscale` | fontenc freetype zlib | freetype fontenc zlib | `qnx-test-mkfontscale` | x86✅ᵛ a64✅ᵛ |
| [`mosh`](../ports/mosh/) | Mobile shell that supports roaming and intelligent local echo | 1.4.0 | `apk add mosh` | abseil ncurses openssl protobuf zlib | protobuf ncurses openssl zlib | `qnx-test-mosh` | x86✅ᵛ a64✅ᵛ |
| [`mosquitto`](../ports/mosquitto/) | Eclipse Mosquitto MQTT broker, client libraries and pub/sub utilities | 2.0.20 | `apk add mosquitto` | cjson openssl | openssl cjson | `qnx-test-mosquitto` | x86✅ᵛ a64✅ᵛ |
| [`mpc`](../ports/mpc/) | GNU MPC library for complex number arithmetic with arbitrary precision | 1.3.1 | `apk add mpc` | gmp mpfr | gmp mpfr | `qnx-test-mpc` | x86✅ᵛ a64✅ᵛ |
| [`mpfr`](../ports/mpfr/) | C library for multiple-precision floating-point computations | 4.2.1 | `apk add mpfr` | gmp | gmp | `qnx-test-mpfr` | x86✅ᵛ a64✅ᵛ |
| [`nano`](../ports/nano/) | GNU nano, a small and simple terminal text editor | 8.2 | `apk add nano` | file ncurses zlib | ncurses zlib file | `qnx-test-nano` | x86✅ᵛ a64✅ᵛ |
| [`neon`](../ports/neon/) | HTTP and WebDAV client library, with a C interface | 0.37.1 | `apk add neon` | expat openssl zlib | openssl expat zlib | `qnx-test-neon` | x86✅ᵛ a64✅ᵛ |
| [`neovim`](../ports/neovim/) | Vim-based extensible text editor | 0.12.3 | `apk add neovim` | gettext libiconv luv tree-sitter unibilium utf8proc | luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext | `qnx-test-neovim` | x86✅ᵛ a64✅ᵛ |
| [`nettle`](../ports/nettle/) | Low-level cryptographic library | 3.10.1 | `apk add nettle` | gmp | gmp | `qnx-test-nettle` | x86✅ᵛ a64✅ᵛ |
| [`nghttp2`](../ports/nghttp2/) | HTTP/2 protocol implementation library in C | 1.64.0 | `apk add nghttp2` | – | zlib openssl | `qnx-test-nghttp2` | x86✅ᵛ a64✅ᵛ |
| [`nginx`](../ports/nginx/) | HTTP web server, reverse proxy, load balancer and TCP/UDP/mail proxy | 1.30.6 | `apk add nginx` | libxcrypt openssl pcre2 zlib | openssl pcre2 zlib libxcrypt | `qnx-test-nginx` | x86✅ᵛ a64✅ᵛ |
| [`nodejs`](../ports/nodejs/) | Node.js cross-platform JavaScript runtime environment | 24.14.1 | `apk add nodejs` | ada brotli c-ares icu libuv nghttp2 openssl simdjson simdutf sqlite zlib zstd | libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu | `qnx-test-nodejs` | x86✅ᵛ a64✅ᵛ |
| [`npm`](../ports/npm/) | Package manager for JavaScript and Node.js | 11.11.0 | `apk add npm` | – | nodejs | `qnx-test-npm` | x86✅ᵛ a64✅ᵛ |
| [`nss`](../ports/nss/) | Mozilla Network Security Services | 3.131.1 | `apk add nss` | nspr sqlite zlib | nspr sqlite zlib | `qnx-test-nss` | x86✅ᵛ a64✅ᵛ |
| [`ntp`](../ports/ntp/) | Network Time Protocol reference implementation: ntpd, ntpq, ntpdate | 4.2.8p18 | `apk add ntp` | libedit libevent | libevent libedit | `qnx-test-ntp` | – |
| [`ntpsec`](../ports/ntpsec/) | Hardened implementation of the Network Time Protocol | 1.2.4 | `apk add ntpsec` | openssl | openssl | `qnx-test-ntpsec` | x86✅ᵛ a64✅ᵛ |
| [`numpy`](../ports/numpy/) | Fundamental package for array computing in Python | 2.3.4 | `apk add numpy` | openblas python3 | python3 openblas | `qnx-test-numpy` | x86✅ᵛ a64✅ᵛ |
| [`onnx`](../ports/onnx/) | Open Neural Network Exchange (ONNX) format library for AI models | 1.17.0 | `apk add onnx` | abseil protobuf | protobuf | `qnx-test-onnx` | x86✅ᵛ a64✅ᵛ |
| [`onnxruntime`](../ports/onnxruntime/) | Cross-platform machine learning inference engine for ONNX models | 1.20.1 | `apk add onnxruntime` | zlib | protobuf | `qnx-test-onnxruntime` | x86✅ᵛ a64✅ᵛ |
| [`open-amp`](../ports/open-amp/) | OpenAMP framework for asymmetric multiprocessing (remoteproc, RPMsg) | 2024.10.0 | `apk add open-amp` | – | libmetal | `qnx-test-open-amp` | x86✅ᵛ a64✅ᵛ |
| [`open62541`](../ports/open62541/) | C implementation of OPC UA (IEC 62541) clients and servers | 1.4.8 | `apk add open62541` | openssl | openssl | `qnx-test-open62541` | x86✅ᵛ a64✅ᵛ |
| [`opencv`](../ports/opencv/) | Open Source Computer Vision library | 4.10.0 | `apk add opencv` | zlib | zlib eigen | `qnx-test-opencv` | x86✅ᵛ a64✅ᵛ |
| [`openjpeg`](../ports/openjpeg/) | JPEG 2000 image codec library and tools | 2.5.3 | `apk add openjpeg` | – | zlib libpng libtiff lcms2 | `qnx-test-openjpeg` | x86✅ᵛ a64✅ᵛ |
| [`openssh`](../ports/openssh/) | OpenSSH client and server for secure remote login and file transfer | 10.2p1 | `apk add openssh` | libxcrypt openssl zlib | openssl zlib libxcrypt | `qnx-test-openssh` | – |
| [`openttd`](../ports/openttd/) | OpenTTD, an open source simulation game based on Transport Tycoon Deluxe | 15.0-git-20250501 | `apk add openttd` | curl freetype libpng lzo sdl2 xz zlib | sdl2 zlib xz libpng freetype fontconfig curl lzo | – | – |
| [`opusfile`](../ports/opusfile/) | High-level library for decoding and seeking in Ogg Opus files | 0.12 | `apk add opusfile` | libogg openssl opus | opus libogg openssl | `qnx-test-opusfile` | x86✅ᵛ a64✅ᵛ |
| [`orocos-kdl`](../ports/orocos-kdl/) | Orocos Kinematics and Dynamics Library for robot kinematic chains | 1.5.1 | `apk add orocos-kdl` | – | eigen | `qnx-test-orocos-kdl` | x86✅ᵛ a64✅ᵛ |
| [`oss-desktop`](../ports/oss-desktop/) | Desktop session with labwc, Xfce panel, terminal and file manager | 1.0 | `apk add oss-desktop` | – | labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`p11-kit`](../ports/p11-kit/) | Library and proxy module for loading and sharing PKCS#11 modules | 0.25.5 | `apk add p11-kit` | libffi | libffi libtasn1 | `qnx-test-p11kit` | x86✅ᵛ a64✅ᵛ |
| [`paho.mqtt.c`](../ports/paho.mqtt.c/) | Eclipse Paho MQTT client library for C | 1.3.14 | `apk add paho.mqtt.c` | openssl | openssl | `qnx-test-paho-mqtt-c` | x86✅ᵛ a64✅ᵛ |
| [`paho.mqtt.cpp`](../ports/paho.mqtt.cpp/) | Eclipse Paho MQTT client library for C++ | 1.5.2 | `apk add paho.mqtt.cpp` | openssl paho.mqtt.c | paho.mqtt.c | `qnx-test-paho-mqtt-cpp` | x86✅ᵛ a64✅ᵛ |
| [`pandas`](../ports/pandas/) | Python data analysis library with labeled data structures | 2.3.3 | `apk add pandas` | python3 | numpy python3-dateutil python3-pytz python3-tzdata | `qnx-test-pandas` | x86✅ᵛ a64✅ᵛ |
| [`pango`](../ports/pango/) | Library for layout and rendering of internationalized text | 1.54.0 | `apk add pango` | cairo fontconfig freetype fribidi glib harfbuzz libx11 libxft libxrender | glib harfbuzz fribidi cairo fontconfig freetype libxft | `qnx-test-pango` | x86✅ᵛ a64✅ᵛ |
| [`pcl`](../ports/pcl/) | Point Cloud Library for 2D/3D point cloud processing | 1.14.1 | `apk add pcl` | boost flann lz4 | boost eigen flann | `qnx-test-pcl` | x86✅ᵛ a64✅ᵛ |
| [`pcre2`](../ports/pcre2/) | Perl compatible regular expressions library (PCRE2 API) | 10.44 | `apk add pcre2` | – | zlib bzip2 | `qnx-test-pcre2` | x86✅ᵛ a64✅ᵛ |
| [`perl`](../ports/perl/) | Perl 5 programming language interpreter | 5.40.2 | `apk add perl` | libxcrypt | libxcrypt | – | x86✅ᵛ a64✅ᵛ |
| [`perl-error`](../ports/perl-error/) | Error.pm, try/catch style exception handling for Perl | 0.17030 | `apk add perl-error` | – | perl | `qnx-test-perl-error` | x86✅ᵛ a64✅ᵛ |
| [`perl-io-tty`](../ports/perl-io-tty/) | Perl IO::Tty and IO::Pty: pseudo-terminals and terminal constants | 1.31 | `apk add perl-io-tty` | – | perl | `qnx-test-perl-io-tty` | x86✅ᵛ a64✅ᵛ |
| [`perl-mozilla-ca`](../ports/perl-mozilla-ca/) | Mozilla's CA certificate bundle for Perl (Mozilla::CA) | 20261020 | `apk add perl-mozilla-ca` | – | perl | `qnx-test-perl-mozilla-ca` | x86✅ᵛ a64✅ᵛ |
| [`pistache`](../ports/pistache/) | C++ REST framework for HTTP servers and clients | 0.4.26 | `apk add pistache` | libevent openssl | libevent rapidjson openssl | `qnx-test-pistache` | x86✅ᵛ a64✅ᵛ |
| [`pixman`](../ports/pixman/) | Low-level pixel manipulation library (compositing, rasterization) | 0.46.4 | `apk add pixman` | – | libpng | `qnx-test-pixman` | x86✅ᵛ a64✅ᵛ |
| [`poco`](../ports/poco/) | Cross-platform C++ libraries for network-based applications | 1.15.4 | `apk add poco` | expat openssl pcre2 utf8proc zlib | openssl zlib pcre2 utf8proc expat | `qnx-test-poco` | x86✅ᵛ a64✅ᵛ |
| [`poppler`](../ports/poppler/) | PDF rendering library and command line tools | 25.12.0 | `apk add poppler` | cairo fontconfig freetype gettext glib lcms2 libiconv libjpeg-turbo libpng libtiff openjpeg zlib | freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib | `qnx-test-poppler` | x86✅ᵛ a64✅ᵛ |
| [`pqiv`](../ports/pqiv/) | Minimal command line image viewer based on GTK 3 | 2.13.2 | `apk add pqiv` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libx11 pango zlib | glib gtk3 gdk-pixbuf cairo | `qnx-test-pqiv` | x86✅ᵛ a64✅ᵛ |
| [`protobuf`](../ports/protobuf/) | Google's Protocol Buffers data serialization library and compiler | 29.3 | `apk add protobuf` | abseil zlib | zlib abseil | `qnx-test-protobuf` | x86✅ᵛ a64✅ᵛ |
| [`psutil`](../ports/psutil/) | Cross-platform process and system utilities module for Python | 7.2.2 | `apk add psutil` | – | python3 | `qnx-test-psutil` | x86✅ᵛ a64✅ᵛ |
| [`pthreadpool`](../ports/pthreadpool/) | Portable thread pool library in C | 0-git-20250207 | `apk add pthreadpool` | – | fxdiv | `qnx-test-pthreadpool` | – |
| [`pygame`](../ports/pygame/) | Python game development library on SDL2 | 2.6.1_git20251005 | `apk add pygame` | freetype libjpeg-turbo libpng sdl2 sdl2-image sdl2-mixer sdl2-ttf | python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo | `qnx-test-pygame` | x86✅ᵛ a64✅ᵛ |
| [`python3`](../ports/python3/) | Python 3 programming language interpreter and standard library | 3.14.0 | `apk add python3` | bzip2 expat gdbm gettext libffi ncurses openssl readline sqlite xz zlib zstd | zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd | `qnx-test-python` | x86✅ᵛ a64✅ᵛ |
| [`python3-argcomplete`](../ports/python3-argcomplete/) | Bash tab completion for argparse | 3.7.2 | `apk add python3-argcomplete` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-astor`](../ports/python3-astor/) | Python module for reading, rewriting and writing Python ASTs as source | 0.8.1 | `apk add python3-astor` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-async-generator`](../ports/python3-async-generator/) | Python async generators and async context managers backport | 1.10 | `apk add python3-async-generator` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-boto3`](../ports/python3-boto3/) | The AWS SDK for Python | 1.43.123 | `apk add python3-boto3` | – | python3 python3-botocore python3-s3transfer python3-jmespath | `qnx-test-python3-boto3` | x86✅ᵛ a64✅ᵛ |
| [`python3-botocore`](../ports/python3-botocore/) | Low-level, data-driven core of boto 3 | 1.43.123 | `apk add python3-botocore` | – | python3 python3-jmespath python3-dateutil python3-urllib3 | `qnx-test-python3-boto3` | x86✅ᵛ a64✅ᵛ |
| [`python3-catkin-pkg`](../ports/python3-catkin-pkg/) | Library for catkin packages (package.xml parsing) | 1.1.1 | `apk add python3-catkin-pkg` | – | python3 python3-dateutil python3-docutils python3-packaging python3-pyparsing | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-click`](../ports/python3-click/) | Python package for creating composable command line interfaces | 8.5.0 | `apk add python3-click` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-argcomplete`](../ports/python3-colcon-argcomplete/) | Completion for colcon command lines using argcomplete | 0.3.3 | `apk add python3-colcon-argcomplete` | – | python3 python3-argcomplete python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-bash`](../ports/python3-colcon-bash/) | Extension for colcon to provide Bash scripts | 0.5.0 | `apk add python3-colcon-bash` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-cd`](../ports/python3-colcon-cd/) | Shell function for colcon to change to a package's directory | 0.1.1 | `apk add python3-colcon-cd` | – | python3 python3-colcon-core python3-colcon-package-information | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-cmake`](../ports/python3-colcon-cmake/) | Extension for colcon to support CMake packages | 0.2.30 | `apk add python3-colcon-cmake` | – | python3 python3-colcon-core python3-colcon-library-path python3-colcon-test-result python3-packaging | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-common-extensions`](../ports/python3-colcon-common-extensions/) | Meta package aggregating colcon-core and common extensions | 0.3.0 | `apk add python3-colcon-common-extensions` | – | python3 python3-colcon-argcomplete python3-colcon-bash python3-colcon-cd python3-colcon-cmake python3-colcon-core python3-colcon-defaults python3-colcon-devtools python3-colcon-library-path python3-colcon-metadata python3-colcon-notification python3-colcon-output python3-colcon-package-information python3-colcon-package-selection python3-colcon-parallel-executor python3-colcon-powershell python3-colcon-python-setup-py python3-colcon-recursive-crawl python3-colcon-ros python3-colcon-test-result python3-colcon-zsh | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-core`](../ports/python3-colcon-core/) | Command line tool to build sets of software packages | 0.21.3 | `apk add python3-colcon-core` | – | python3 python3-distlib python3-empy python3-packaging python3-pytest python3-pytest-cov python3-pytest-repeat python3-pytest-rerunfailures python3-setuptools | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-defaults`](../ports/python3-colcon-defaults/) | Extension for colcon to read defaults from a config file | 0.2.9 | `apk add python3-colcon-defaults` | – | python3 python3-colcon-core python3-pyyaml | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-devtools`](../ports/python3-colcon-devtools/) | Extension for colcon to list its extension points and extensions | 0.3.0 | `apk add python3-colcon-devtools` | – | python3 python3-colcon-core python3-packaging | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-library-path`](../ports/python3-colcon-library-path/) | Extension for colcon adding an environment variable to find libraries | 0.2.1 | `apk add python3-colcon-library-path` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-metadata`](../ports/python3-colcon-metadata/) | Extension for colcon to read package metadata from files | 0.3.1 | `apk add python3-colcon-metadata` | – | python3 python3-colcon-core python3-pyyaml | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-notification`](../ports/python3-colcon-notification/) | Extension for colcon to provide status notifications | 0.3.3 | `apk add python3-colcon-notification` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-output`](../ports/python3-colcon-output/) | Extension for colcon to customize the output in various ways | 0.2.15 | `apk add python3-colcon-output` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-package-information`](../ports/python3-colcon-package-information/) | Extension for colcon to output package information | 0.4.1 | `apk add python3-colcon-package-information` | – | python3 python3-colcon-core python3-packaging | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-package-selection`](../ports/python3-colcon-package-selection/) | Extension for colcon to select the packages to process | 0.2.10 | `apk add python3-colcon-package-selection` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-parallel-executor`](../ports/python3-colcon-parallel-executor/) | Extension for colcon to process packages in parallel | 0.4.0 | `apk add python3-colcon-parallel-executor` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-pkg-config`](../ports/python3-colcon-pkg-config/) | Extension for colcon adding an environment variable for pkg-config files | 0.1.0 | `apk add python3-colcon-pkg-config` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-powershell`](../ports/python3-colcon-powershell/) | Extension for colcon to provide PowerShell scripts | 0.5.0 | `apk add python3-colcon-powershell` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-python-setup-py`](../ports/python3-colcon-python-setup-py/) | Extension for colcon to support Python packages described by setup.py | 0.2.9 | `apk add python3-colcon-python-setup-py` | – | python3 python3-colcon-core python3-setuptools | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-recursive-crawl`](../ports/python3-colcon-recursive-crawl/) | Extension for colcon to recursively crawl for packages | 0.2.3 | `apk add python3-colcon-recursive-crawl` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-ros`](../ports/python3-colcon-ros/) | Extension for colcon to support ROS packages | 0.5.0 | `apk add python3-colcon-ros` | – | python3 python3-catkin-pkg python3-colcon-cmake python3-colcon-core python3-colcon-pkg-config python3-colcon-python-setup-py python3-colcon-recursive-crawl | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-test-result`](../ports/python3-colcon-test-result/) | Extension for colcon to provide information about the test results | 0.3.8 | `apk add python3-colcon-test-result` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-colcon-zsh`](../ports/python3-colcon-zsh/) | Extension for colcon to provide Z shell scripts | 0.5.0 | `apk add python3-colcon-zsh` | – | python3 python3-colcon-core | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-contourpy`](../ports/python3-contourpy/) | Python library for calculating contours of 2D quadrilateral grids | 1.4.0 | `apk add python3-contourpy` | python3 | numpy | `qnx-test-python3-matplotlib` | x86✅ᵛ a64✅ᵛ |
| [`python3-coverage`](../ports/python3-coverage/) | Code coverage measurement for Python | 7.17.0 | `apk add python3-coverage` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-curio`](../ports/python3-curio/) | Coroutine-based Python library for concurrent systems programming | 1.6_git20251221 | `apk add python3-curio` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-cycler`](../ports/python3-cycler/) | Composable style cycles for Python (matplotlib's property cycler) | 0.12.1 | `apk add python3-cycler` | – | python3 | `qnx-test-python3-matplotlib` | x86✅ᵛ a64✅ᵛ |
| [`python3-dateutil`](../ports/python3-dateutil/) | Python extensions to the standard datetime module | 2.9.0.post0 | `apk add python3-dateutil` | – | python3 python3-six | `qnx-test-python3-dateutil` | – |
| [`python3-distlib`](../ports/python3-distlib/) | Distribution utilities | 0.4.4 | `apk add python3-distlib` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-docutils`](../ports/python3-docutils/) | Python documentation utilities (reStructuredText processing) | 0.23 | `apk add python3-docutils` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-empy`](../ports/python3-empy/) | Templating system for Python | 4.2.1 | `apk add python3-empy` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-fonttools`](../ports/python3-fonttools/) | Library to manipulate font files from Python | 4.68.0 | `apk add python3-fonttools` | – | python3 | `qnx-test-python3-matplotlib` | x86✅ᵛ a64✅ᵛ |
| [`python3-idna`](../ports/python3-idna/) | Python support for internationalized domain names (IDNA 2008, UTS 46) | 3.20 | `apk add python3-idna` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-iniconfig`](../ports/python3-iniconfig/) | Minimal INI file parsing module for Python | 2.3.1 | `apk add python3-iniconfig` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-jmespath`](../ports/python3-jmespath/) | JSON Matching Expressions | 1.1.0 | `apk add python3-jmespath` | – | python3 | `qnx-test-python3-boto3` | x86✅ᵛ a64✅ᵛ |
| [`python3-kiwisolver`](../ports/python3-kiwisolver/) | Fast implementation of the Cassowary constraint solver | 1.5.1 | `apk add python3-kiwisolver` | – | python3 | `qnx-test-python3-matplotlib` | x86✅ᵛ a64✅ᵛ |
| [`python3-lxml`](../ports/python3-lxml/) | Pythonic XML processing library combining libxml2/libxslt with ElementTree | 6.1.3 | `apk add python3-lxml` | libxml2 libxslt zlib | python3 libxml2 libxslt zlib | `qnx-test-python3-lxml` | x86✅ᵛ a64✅ᵛ |
| [`python3-matplotlib`](../ports/python3-matplotlib/) | Comprehensive library for static and animated visualizations in Python | 3.11.2 | `apk add python3-matplotlib` | freetype libraqm python3 qhull | numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil | `qnx-test-python3-matplotlib` | x86✅ᵛ a64✅ᵛ |
| [`python3-ml-dtypes`](../ports/python3-ml-dtypes/) | NumPy dtype extensions for machine learning (bfloat16, float8, int4) | 0.6.0 | `apk add python3-ml-dtypes` | – | numpy | `qnx-test-python3-ml-dtypes` | x86✅ᵛ a64✅ᵛ |
| [`python3-mypy-extensions`](../ports/python3-mypy-extensions/) | Type system extensions for programs checked with mypy | 1.1.0 | `apk add python3-mypy-extensions` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-netifaces`](../ports/python3-netifaces/) | Portable network interface information for Python | 0.11.0 | `apk add python3-netifaces` | – | python3 | `qnx-test-python3-netifaces` | x86✅ᵛ a64✅ᵛ |
| [`python3-packaging`](../ports/python3-packaging/) | Core utilities for Python packages: versions, specifiers, markers | 25.0 | `apk add python3-packaging` | – | python3 | `qnx-test-python3-packaging` | x86✅ᵛ a64✅ᵛ |
| [`python3-pillow`](../ports/python3-pillow/) | Python Imaging Library (fork): image processing and file formats | 13.0.0 | `apk add python3-pillow` | freetype lcms2 libjpeg-turbo libraqm libtiff libwebp openjpeg zlib | python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm | `qnx-test-python3-pillow` | x86✅ᵛ a64✅ᵛ |
| [`python3-pip`](../ports/python3-pip/) | Python package installer | 25.1.1 | `apk add python3-pip` | – | python3 | `qnx-test-python3-pip` | x86✅ᵛ a64✅ᵛ |
| [`python3-pluggy`](../ports/python3-pluggy/) | Plugin and hook calling mechanism for Python (used by pytest) | 1.7.0 | `apk add python3-pluggy` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pretend`](../ports/python3-pretend/) | Python library for creating test stubs | 1.0.9 | `apk add python3-pretend` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pygments`](../ports/python3-pygments/) | Python syntax highlighter for many languages | 2.21.0 | `apk add python3-pygments` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pyparsing`](../ports/python3-pyparsing/) | Classes and methods to define and execute parsing grammars | 3.3.3 | `apk add python3-pyparsing` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest`](../ports/python3-pytest/) | Python testing framework | 9.2.0 | `apk add python3-pytest` | – | python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-cov`](../ports/python3-pytest-cov/) | pytest plugin for measuring code coverage | 7.1.0 | `apk add python3-pytest-cov` | – | python3 python3-pytest python3-coverage python3-pluggy | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-repeat`](../ports/python3-pytest-repeat/) | pytest plugin for repeating tests | 0.9.4 | `apk add python3-pytest-repeat` | – | python3 python3-pytest | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-rerunfailures`](../ports/python3-pytest-rerunfailures/) | pytest plugin to re-run tests to eliminate flaky failures | 16.7 | `apk add python3-pytest-rerunfailures` | – | python3 python3-packaging python3-pytest | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-tornasync`](../ports/python3-pytest-tornasync/) | pytest plugin for testing tornado coroutines | 0.6.0.post2 | `apk add python3-pytest-tornasync` | – | python3 python3-pytest python3-tornado | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytokens`](../ports/python3-pytokens/) | Fast spec-compliant Python 3.12+ tokenizer | 0.4.1 | `apk add python3-pytokens` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytz`](../ports/python3-pytz/) | World timezone definitions for Python | 2025.2 | `apk add python3-pytz` | – | python3 | `qnx-test-python3-pytz` | x86✅ᵛ a64✅ᵛ |
| [`python3-pyyaml`](../ports/python3-pyyaml/) | YAML parser and emitter for Python | 6.0.3 | `apk add python3-pyyaml` | – | python3 | `qnx-test-python3-colcon-common-extensions` | x86✅ᵛ a64✅ᵛ |
| [`python3-s3transfer`](../ports/python3-s3transfer/) | An Amazon S3 Transfer Manager | 0.19.3 | `apk add python3-s3transfer` | – | python3 python3-botocore | `qnx-test-python3-boto3` | x86✅ᵛ a64✅ᵛ |
| [`python3-setuptools`](../ports/python3-setuptools/) | Python library and tools for building and distributing packages | 80.8.0 | `apk add python3-setuptools` | – | python3 | `qnx-test-python3-setuptools` | x86✅ᵛ a64✅ᵛ |
| [`python3-six`](../ports/python3-six/) | Python 2 and 3 compatibility library | 1.17.0 | `apk add python3-six` | – | python3 | `qnx-test-python3-six` | x86✅ᵛ a64✅ᵛ |
| [`python3-sniffio`](../ports/python3-sniffio/) | Detects which async library a Python program is running under | 1.3.1 | `apk add python3-sniffio` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-sortedcontainers`](../ports/python3-sortedcontainers/) | Sorted list, dict and set types in pure Python | 2.4.0 | `apk add python3-sortedcontainers` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-tornado`](../ports/python3-tornado/) | Python web framework and asynchronous networking library | 6.6 | `apk add python3-tornado` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-traitlets`](../ports/python3-traitlets/) | Typed, validated and observable attributes for Python configuration | 5.16.1 | `apk add python3-traitlets` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-tzdata`](../ports/python3-tzdata/) | IANA time zone database as a Python package | 2025.2 | `apk add python3-tzdata` | – | python3 | `qnx-test-python3-tzdata` | x86✅ᵛ a64✅ᵛ |
| [`python3-urllib3`](../ports/python3-urllib3/) | HTTP library with thread-safe connection pooling, file post, and more | 2.8.0 | `apk add python3-urllib3` | – | python3 | `qnx-test-python3-boto3` | x86✅ᵛ a64✅ᵛ |
| [`python3-versioneer`](../ports/python3-versioneer/) | Python tool for deriving version strings from version-control tags | 0.29 | `apk add python3-versioneer` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-wheel`](../ports/python3-wheel/) | Command line tool and library for the Python wheel package format | 0.46.1 | `apk add python3-wheel` | – | python3 python3-packaging | `qnx-test-python3-wheel` | x86✅ᵛ a64✅ᵛ |
| [`pytorch`](../ports/pytorch/) | PyTorch tensor computation and deep learning library | 2.3.1 | `apk add pytorch` | abseil openblas protobuf | python3 protobuf openblas | `qnx-test-torch` | x86✅ᵛ a64✅ᵛ |
| [`qdirstat`](../ports/qdirstat/) | Qt-based directory statistics: disk usage with a treemap | 2.0 | `apk add qdirstat` | gettext qt6 zlib | qt6 zlib gettext | `qnx-test-qdirstat` | x86✅ᵛ a64✅ᵛ |
| [`qpdf`](../ports/qpdf/) | PDF Transformation System | 11.10.1 | `apk add qpdf` | libjpeg-turbo openssl zlib | zlib libjpeg-turbo openssl | `qnx-test-qpdf` | x86✅ᵛ a64✅ᵛ |
| [`qt6`](../ports/qt6/) | Qt 6 cross-platform application and UI framework | 6.8.1 | `apk add qt6` | brotli double-conversion ffmpeg fontconfig freetype glib gst-plugins-bad gst-plugins-base gstreamer harfbuzz icu libjpeg-turbo libpng libtiff libwebp openssl pcre2 zlib zstd | zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd | `qnx-test-qt6` | x86✅ᵛ a64✅ᵛ |
| [`quake3`](../ports/quake3/) | ioquake3, the maintained open-source Quake III Arena engine | 1.36-git-20261012 | `apk add quake3` | sdl2 | sdl2 | `qnx-test-quake3` | – |
| [`raylib`](../ports/raylib/) | Simple and easy-to-use library to enjoy videogames programming | 6.2 | `apk add raylib` | sdl2 | sdl2 | `qnx-test-raylib` | x86✅ᵛ a64✅ᵛ |
| [`re2`](../ports/re2/) | Fast, safe, thread-friendly regular expression library | 2024-07-02 | `apk add re2` | abseil | abseil | `qnx-test-re2` | x86✅ a64✅ |
| [`readline`](../ports/readline/) | GNU Readline library for command line editing and history | 8.2 | `apk add readline` | – | ncurses | `qnx-test-readline` | x86✅ᵛ a64✅ᵛ |
| [`redis`](../ports/redis/) | Advanced key-value store (server and command-line client) | 8.10.3 | `apk add redis` | openssl | openssl | `qnx-test-redis` | x86✅ᵛ a64✅ᵛ |
| [`restbed`](../ports/restbed/) | C++ framework for building HTTP and HTTPS client and server applications | 4.8 | `apk add restbed` | openssl | openssl asio | `qnx-test-restbed` | x86✅ᵛ a64✅ᵛ |
| [`retroarch`](../ports/retroarch/) | RetroArch, the libretro frontend for emulators and games | 1.15-git-20230408 | `apk add retroarch` | fontconfig freetype fribidi libass libxkbcommon openssl zlib | sdl2 freetype zlib fontconfig fribidi libass openssl | `qnx-test-retroarch` | – |
| [`retropie`](../ports/retropie/) | RetroPie retro gaming stack: RetroArch, EmulationStation, cores, configs | 0.3 | `apk add retropie` | – | retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08 | `qnx-test-retropie` | x86✅ᵛ a64✅ᵛ |
| [`riegeli`](../ports/riegeli/) | File format for storing a sequence of records, typically protocol buffers | 0_git20231212 | `apk add riegeli` | abseil brotli bzip2 highwayhash lz4 protobuf snappy xz zlib zstd | abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash | `qnx-test-riegeli` | x86✅ᵛ a64✅ᵛ |
| [`rocksdb`](../ports/rocksdb/) | Embeddable, persistent key-value store for fast storage | 11.8.1 | `apk add rocksdb` | lz4 zlib zstd | lz4 zlib zstd | `qnx-test-rocksdb` | x86✅ᵛ a64✅ᵛ |
| [`ros2`](../ports/ros2/) | ROS 2 Jazzy core: robotics middleware, client libraries and tools | jazzy | `apk add ros2` | benchmark console_bridge curl cyclonedds fastcdr fastdds libyaml lz4 opencv python3 sqlite tinyxml2 yaml-cpp zstd | python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2 | `qnx-test-ros2` | x86✅ᵛ a64✅ᵛ |
| [`rsync`](../ports/rsync/) | Fast local and remote file copying tool using delta transfer | 3.3.0 | `apk add rsync` | lz4 openssl xxhash zstd | zstd lz4 xxhash openssl | `qnx-test-rsync` | x86✅ᵛ a64✅ᵛ |
| [`rtl-sdr`](../ports/rtl-sdr/) | Library and tools turning RTL2832 DVB dongles into SDR receivers | 2.0.2 | `apk add rtl-sdr` | libusb | libusb | `qnx-test-rtl-sdr` | x86✅ᵛ a64✅ᵛ |
| [`ruy`](../ports/ruy/) | Matrix multiplication library for neural network inference | 0-git-20240321 | `apk add ruy` | – | cpuinfo | `qnx-test-ruy` | – |
| [`s2n-tls`](../ports/s2n-tls/) | Small, fast C99 implementation of the TLS/SSL protocols from AWS | 1.7.0 | `apk add s2n-tls` | openssl | openssl | `qnx-test-s2n-tls` | x86✅ᵛ a64✅ᵛ |
| [`sdl2`](../ports/sdl2/) | Simple DirectMedia Layer 2, cross-platform multimedia and input library | 2.32.10 | `apk add sdl2` | – | wayland libxkbcommon | `qnx-test-sdl2` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-compat`](../ports/sdl2-compat/) | SDL2 compatibility layer implemented on top of SDL3 | 2.32.60 | `apk add sdl2-compat` | – | sdl3 | `qnx-test-sdl2-compat` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-image`](../ports/sdl2-image/) | Image loading library for SDL2 | 2.8.8 | `apk add sdl2-image` | libavif libtiff libwebp sdl2 | sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl | `qnx-test-sdl2-image` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-mixer`](../ports/sdl2-mixer/) | Audio mixer library for SDL2 (WAV, FLAC, MP3, Ogg Vorbis, Opus, MIDI) | 2.8.2 | `apk add sdl2-mixer` | opusfile sdl2 | sdl2 opusfile | `qnx-test-sdl2-mixer` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-net`](../ports/sdl2-net/) | Portable network library for SDL2 | 2.2.0 | `apk add sdl2-net` | sdl2 | sdl2 | `qnx-test-sdl2-net` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-ttf`](../ports/sdl2-ttf/) | TrueType font rendering library for SDL2 using FreeType and HarfBuzz | 2.24.0 | `apk add sdl2-ttf` | freetype harfbuzz sdl2 | sdl2 freetype harfbuzz | `qnx-test-sdl2-ttf` | x86✅ᵛ a64✅ᵛ |
| [`sdl3`](../ports/sdl3/) | Simple DirectMedia Layer 3, cross-platform multimedia and input library | 3.4.18 | `apk add sdl3` | libiconv libx11 libxcursor libxext libxfixes libxi libxrandr libxrender libxscrnsaver libxtst | libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst | `qnx-test-sdl3` | x86✅ᵛ a64✅ᵛ |
| [`sentencepiece`](../ports/sentencepiece/) | Unsupervised text tokenizer for neural network-based text generation | 0.2.1 | `apk add sentencepiece` | abseil protobuf | protobuf abseil | `qnx-test-sentencepiece` | x86✅ᵛ a64✅ᵛ |
| [`shared-mime-info`](../ports/shared-mime-info/) | Freedesktop.org shared MIME type database and update-mime-database | 2.4 | `apk add shared-mime-info` | glib libxml2 | glib libxml2 | `qnx-test-shared-mime-info` | x86✅ᵛ a64✅ᵛ |
| [`silkit-adapters-tap`](../ports/silkit-adapters-tap/) | SIL Kit adapter connecting TAP network devices to SIL Kit Ethernet networks | 1.2.0 | `apk add silkit-adapters-tap` | silkit | silkit | `qnx-test-silkit-adapters-tap` | x86✅ᵛ a64✅ᵛ |
| [`socat`](../ports/socat/) | Multipurpose relay for binary protocols (SOcket CAT) | 1.8.0.3 | `apk add socat` | ncurses openssl readline | openssl readline | `qnx-test-socat` | x86✅ᵛ a64✅ᵛ |
| [`soem`](../ports/soem/) | Simple Open EtherCAT Master library and tools | 2.0.1 | `apk add soem` | – | libpcap | `qnx-test-soem` | x86✅ᵛ a64✅ᵛ |
| [`spdlog`](../ports/spdlog/) | Fast C++ logging library | 1.15.3 | `apk add spdlog` | fmt | fmt | `qnx-test-spdlog` | x86✅ᵛ a64✅ᵛ |
| [`sqlite`](../ports/sqlite/) | SQLite embedded SQL database engine and command line shell | 3.47.2 | `apk add sqlite` | zlib | zlib | `qnx-test-sqlite` | x86✅ᵛ a64✅ᵛ |
| [`sqlite_orm`](../ports/sqlite_orm/) | Header-only SQLite ORM library for modern C++ | 1.9 | `apk add sqlite_orm` | – | sqlite | `qnx-test-sqlite-orm` | x86✅ᵛ a64✅ᵛ |
| [`sscep`](../ports/sscep/) | Simple SCEP client for certificate enrollment | 0.10.0 | `apk add sscep` | openssl | openssl | `qnx-test-sscep` | x86✅ᵛ a64✅ᵛ |
| [`strongswan`](../ports/strongswan/) | IPsec-based VPN solution | 6.0.6 | `apk add strongswan` | openssl | openssl | `qnx-test-strongswan` | x86✅ᵛ a64❌ᵛ |
| [`sudo`](../ports/sudo/) | Runs commands with the privileges of another user | 1.9.17p2 | `apk add sudo` | libxcrypt openssl zlib | libxcrypt openssl zlib | `qnx-test-sudo` | – |
| [`suitesparse`](../ports/suitesparse/) | Suite of sparse matrix algorithms and factorization libraries | 7.11.0 | `apk add suitesparse` | openblas | openblas | `qnx-test-suitesparse` | x86✅ᵛ a64✅ᵛ |
| [`swaybg`](../ports/swaybg/) | Wallpaper utility for wlroots-based Wayland compositors | 1.2.2 | `apk add swaybg` | cairo gdk-pixbuf glib wayland | wayland wayland-protocols cairo gdk-pixbuf glib | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`sysbench`](../ports/sysbench/) | Scriptable multi-threaded benchmark tool for CPU, memory, I/O and threads | 1.0.20-git-20250309 | `apk add sysbench` | luajit | luajit | `qnx-test-sysbench` | – |
| [`taglib`](../ports/taglib/) | Library for reading and editing audio file metadata tags | 2.0.2 | `apk add taglib` | zlib | zlib | `qnx-test-taglib` | x86✅ᵛ a64✅ᵛ |
| [`tar`](../ports/tar/) | GNU tar archiving utility | 1.35 | `apk add tar` | libiconv | libiconv | `qnx-test-tar` | x86✅ᵛ a64✅ᵛ |
| [`tcpdump`](../ports/tcpdump/) | A tool for network monitoring and data acquisition | 4.99.5 | `apk add tcpdump` | – | libpcap | `qnx-test-tcpdump` | x86✅ᵛ a64✅ᵛ |
| [`tftp-hpa`](../ports/tftp-hpa/) | TFTP client and server | 5.2 | `apk add tftp-hpa` | ncurses readline | readline ncurses | `qnx-test-tftp-hpa` | x86✅ᵛ a64✅ᵛ |
| [`thorvg`](../ports/thorvg/) | Lightweight library for rendering vector graphics, SVG and Lottie | 0.15.8 | `apk add thorvg` | libjpeg-turbo libpng | libjpeg-turbo libpng zlib | `qnx-test-thorvg` | x86✅ᵛ a64✅ᵛ |
| [`thunar`](../ports/thunar/) | Thunar, the Xfce file manager, and its extension library | 4.20.0 | `apk add thunar` | at-spi2-core cairo exo gdk-pixbuf gettext glib gtk3 harfbuzz libexif libnotify libx11 libxfce4ui libxfce4util pango pcre2 xfce4-panel xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel | `qnx-test-thunar` | x86✅ᵛ a64✅ᵛ |
| [`tmux`](../ports/tmux/) | Terminal multiplexer | 3.5a | `apk add tmux` | libevent ncurses | libevent ncurses | `qnx-test-tmux` | x86✅ᵛ a64✅ᵛ |
| [`tumbler`](../ports/tumbler/) | D-Bus thumbnailing service for Xfce applications | 4.20.0 | `apk add tumbler` | curl ffmpegthumbnailer freetype gdk-pixbuf gettext glib gst-plugins-base gstreamer libgsf libjpeg-turbo libpng libxfce4util libxml2 | glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer | `qnx-test-tumbler` | x86✅ᵛ a64✅ᵛ |
| [`unbound`](../ports/unbound/) | Validating, recursive and caching DNS resolver and library | 1.24.1 | `apk add unbound` | expat openssl | openssl expat | `qnx-test-unbound` | x86✅ᵛ a64✅ᵛ |
| [`up-core-api`](../ports/up-core-api/) | uProtocol data model and core service APIs (protobuf, compiled for C++) | 1.6.0-alpha2 | `apk add up-core-api` | abseil protobuf | protobuf | `qnx-test-up-core-api` | – |
| [`up-cpp`](../ports/up-cpp/) | Eclipse uProtocol C++ library: data model and communication APIs | 1.0.1 | `apk add up-cpp` | abseil protobuf up-core-api | protobuf up-core-api | `qnx-test-up-cpp` | x86✅ᵛ a64✅ᵛ |
| [`up-transport-zenoh-cpp`](../ports/up-transport-zenoh-cpp/) | uProtocol transport over Eclipse Zenoh for up-cpp (zenoh-cpp on zenoh-c) | 1.0.0-git-20250319 | `apk add up-transport-zenoh-cpp` | abseil fmt protobuf spdlog up-core-api up-cpp zenoh-c | zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog | `qnx-test-up-transport-zenoh-cpp` | – |
| [`urdfdom`](../ports/urdfdom/) | Unified Robot Description Format (URDF) parser | 4.0.3 | `apk add urdfdom` | console_bridge tinyxml2 | urdfdom_headers console_bridge tinyxml2 | `qnx-test-urdfdom` | x86✅ᵛ a64✅ᵛ |
| [`vim`](../ports/vim/) | Vi IMproved, a highly configurable text editor | 9.1.0900 | `apk add vim` | libiconv libsodium ncurses | ncurses libiconv libsodium | `qnx-test-vim` | x86✅ᵛ a64✅ᵛ |
| [`virtualgl`](../ports/virtualgl/) | VirtualGL client (vglclient) for displaying remotely rendered OpenGL | 3.1.5 | `apk add virtualgl` | libjpeg-turbo libx11 libxext | libx11 libxext libjpeg-turbo | `qnx-test-virtualgl` | x86✅ᵛ a64✅ᵛ |
| [`vlc`](../ports/vlc/) | VLC media player and libvlc multimedia engine | 3.0.21 | `apk add vlc` | bzip2 libiconv lz4 openssl xz zstd | bzip2 libiconv lz4 openssl xz zstd | `qnx-test-vlc` | x86✅ᵛ a64✅ᵛ |
| [`vsomeip`](../ports/vsomeip/) | COVESA SOME/IP protocol stack for automotive communication | 3.5.6 | `apk add vsomeip` | boost | boost | `qnx-test-vsomeip` | x86✅ᵛ a64✅ᵛ |
| [`vte`](../ports/vte/) | Virtual terminal widget library for GTK | 0.78.2 | `apk add vte` | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gnutls graphene gtk3 gtk4 icu lz4 pango pcre2 | gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4 | `qnx-test-vte` | x86✅ᵛ a64✅ᵛ |
| [`wayland`](../ports/wayland/) | Wayland display protocol client and server libraries | 1.24.0 | `apk add wayland` | libffi | libffi expat libxml2 | `qnx-test-wayland` | x86✅ᵛ a64✅ᵛ |
| [`wayland-protocols`](../ports/wayland-protocols/) | Extension protocol definitions for Wayland (XML files) | 1.47 | `apk add wayland-protocols` | – | wayland | `qnx-test-wayland-protocols` | x86✅ᵛ a64✅ᵛ |
| [`webkitgtk`](../ports/webkitgtk/) | WebKitGTK web content engine for GTK applications | 2.44.4 | `apk add webkitgtk` | at-spi2-core cairo fontconfig freetype gdk-pixbuf gettext glib gst-plugins-bad gst-plugins-base gstreamer gtk3 harfbuzz hyphen icu lcms2 libavif libepoxy libgcrypt libgpg-error libjpeg-turbo libjxl libpng libsecret libsoup libtasn1 libwebp libxml2 libxslt pango sqlite woff2 zlib | gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret | `qnx-test-webkitgtk` | x86✅ᵛ a64✅ᵛ |
| [`wget`](../ports/wget/) | GNU Wget, non-interactive HTTP, HTTPS and FTP downloader | 1.25.0 | `apk add wget` | gnutls libiconv libpsl libunistring nettle pcre2 zlib | gnutls libidn2 zlib pcre2 libpsl | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`wlroots`](../ports/wlroots/) | Modular library for building Wayland compositors | 0.20.2 | `apk add wlroots` | lcms2 libxkbcommon pixman wayland | wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes | `qnx-test-wlroots` | x86✅ᵛ a64✅ᵛ |
| [`woff2`](../ports/woff2/) | Library and tools for WOFF2 font compression and decompression | 1.0.2 | `apk add woff2` | brotli | brotli | `qnx-test-woff2` | x86✅ᵛ a64✅ᵛ |
| [`xcb-util`](../ports/xcb-util/) | XCB utility libraries: atom cache, events and convenience functions | 0.4.1 | `apk add xcb-util` | libxcb | libxcb | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-appfinder`](../ports/xfce4-appfinder/) | Xfce application finder and launcher | 4.20.0 | `apk add xfce4-appfinder` | cairo garcon gdk-pixbuf glib gtk3 libxfce4ui libxfce4util xfconf | glib gtk3 libxfce4util libxfce4ui xfconf garcon | `qnx-test-xfce4-appfinder` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-panel`](../ports/xfce4-panel/) | Xfce panel with launchers, menus and workspace switcher | 4.20.0 | `apk add xfce4-panel` | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util libxfce4windowing pango wayland xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols | `qnx-test-xfce4-panel` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-session`](../ports/xfce4-session/) | Xfce session manager | 4.20.2 | `apk add xfce4-session` | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util libxfce4windowing pango xfconf | glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell | `qnx-test-xfce4-session` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-settings`](../ports/xfce4-settings/) | Xfce settings daemon, manager and editor | 4.20.1 | `apk add xfce4-settings` | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util pango wayland xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell | `qnx-test-xfce4-settings` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-terminal`](../ports/xfce4-terminal/) | Xfce terminal emulator | 1.1.5 | `apk add xfce4-terminal` | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util pango pcre2 vte xfconf | glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell | `qnx-test-xfce4-terminal` | x86✅ᵛ a64✅ᵛ |
| [`xfconf`](../ports/xfconf/) | Hierarchical configuration storage system for Xfce | 4.20.0 | `apk add xfconf` | gettext glib libxfce4util | glib libxfce4util dbus | `qnx-test-xfconf` | x86✅ᵛ a64✅ᵛ |
| [`xkbcomp`](../ports/xkbcomp/) | XKB keymap compiler | 1.5.0 | `apk add xkbcomp` | libx11 libxkbfile | libx11 libxkbfile xorgproto | `qnx-test-xkbcomp` | x86✅ᵛ a64✅ᵛ |
| [`xnnpack`](../ports/xnnpack/) | Optimized neural network inference operators for ARM and x86 | 0-git-20250209 | `apk add xnnpack` | pthreadpool | cpuinfo pthreadpool fxdiv | `qnx-test-xnnpack` | – |
| [`xsltproc`](../ports/xsltproc/) | Command-line XSLT processor from libxslt | 1.1.42 | `apk add xsltproc` | libxml2 libxslt | libxslt libxml2 | `qnx-test-xsltproc` | x86✅ᵛ a64✅ᵛ |
| [`xwayland`](../ports/xwayland/) | X server running as a Wayland client (Xwayland) | 24.1.14 | `apk add xwayland` | libmd libxau libxcvt libxfont2 pixman wayland | wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util | `qnx-test-virtualgl` | x86✅ᵛ a64✅ᵛ |
| [`xz`](../ports/xz/) | XZ Utils, LZMA compression library and tools | 5.6.3 | `apk add xz` | gettext | gettext | `qnx-test-xz` | x86✅ᵛ a64✅ᵛ |
| [`zenoh-c`](../ports/zenoh-c/) | C bindings for Eclipse Zenoh pub/sub, storage and query protocol | 1.2.1 | `apk add zenoh-c` | – | rust | `qnx-test-zenoh-c` | x86✅ᵛ a64✅ᵛ |
| [`zenoh-cpp`](../ports/zenoh-cpp/) | C++ bindings for Eclipse Zenoh (header-only, over zenoh-c or zenoh-pico) | 1.2.1 | `apk add zenoh-cpp` | – | zenoh-c zenoh-pico | `qnx-test-zenoh-cpp` | – |
| [`zsh`](../ports/zsh/) | Z shell | 5.9 | `apk add zsh` | libiconv ncurses | ncurses pcre2 libiconv | `qnx-test-zsh` | x86✅ᵛ a64✅ᵛ |

---
<sub>Generated by <code>scripts/gen-target-guide</code>.</sub>
