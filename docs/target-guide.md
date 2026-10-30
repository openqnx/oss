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

All of this in one command -- image, apk, every component installed and
tested, or the desktop with graphics: [`vanilla-vm/`](../vanilla-vm/README.md)
(`vanilla-vm/run-all both`, `vanilla-vm/desktop x86_64`).

### x86_64 -- QEMU (vanilla mkqnximage image)

```bash
source ~/qnx800/qnxsdp-env.sh
mkdir -p ~/vm/qnx-x86_64 && cd ~/vm/qnx-x86_64
mkqnximage --type=qemu --arch=x86_64 --ssh-ident=$HOME/.ssh/id_ed25519.pub \
           --data-size=3000 --data-inodes=60000 --run=-h      # 3 GB /data for the packages
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


**223 standalone** components build with the QNX SDP alone and run on a vanilla image with nothing else: `apk add <name>` installs one package. **301 dependent** components need others: apk installs those first by itself (the *needs at run time* column); to build one yourself, build its *build* dependencies first (`ports/<name>/README.md` lists them in order).

Verified: `x86✅` / `a64✅` passed on x86_64 / aarch64 (QEMU) with the version in `dists/`, `ᵛ` on a vanilla image installed with apk; details per component in its README's *How it was verified*.

### Standalone (223)

| Component | Version | Install | Test package | Verified |
|---|---|---|---|---|
| [`abseil`](../ports/abseil/) | 20240722.0 | `apk add abseil` | – | x86✅ᵛ a64✅ᵛ |
| [`ada`](../ports/ada/) | 3.4.4 | `apk add ada` | – | x86✅ᵛ a64✅ᵛ |
| [`adwaita-fonts`](../ports/adwaita-fonts/) | 49.0 | `apk add adwaita-fonts` | – | x86✅ᵛ a64✅ᵛ |
| [`aom`](../ports/aom/) | 3.12.1 | `apk add aom` | `qnx-test-aom` | x86✅ᵛ a64✅ᵛ |
| [`asio`](../ports/asio/) | 1.30.2 | `apk add asio` | – | x86✅ᵛ a64✅ᵛ |
| [`autoconf`](../ports/autoconf/) | 2.72 | `apk add autoconf` | – | x86✅ᵛ a64✅ᵛ |
| [`automake`](../ports/automake/) | 1.18 | `apk add automake` | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-common`](../ports/aws-c-common/) | 0.12.6 | `apk add aws-c-common` | – | x86✅ᵛ a64✅ᵛ |
| [`benchmark`](../ports/benchmark/) | 1.9.0 | `apk add benchmark` | `qnx-test-benchmark` | x86✅ᵛ a64✅ᵛ |
| [`brotli`](../ports/brotli/) | 1.1.0 | `apk add brotli` | `qnx-test-brotli` | x86✅ᵛ a64✅ᵛ |
| [`btop`](../ports/btop/) | 1.4.6 | `apk add btop` | `qnx-test-monitors` | x86✅ᵛ a64✅ᵛ |
| [`busybox`](../ports/busybox/) | 1.36.1 | `apk add busybox` | – | x86✅ᵛ a64✅ᵛ |
| [`bzip2`](../ports/bzip2/) | 1.0.8 | `apk add bzip2` | `qnx-test-bzip2` | x86✅ᵛ a64✅ᵛ |
| [`c-ares`](../ports/c-ares/) | 1.34.3 | `apk add c-ares` | `qnx-test-c-ares` | x86✅ᵛ a64✅ᵛ |
| [`ca-certificates`](../ports/ca-certificates/) | 20261020 | `apk add ca-certificates` | – | x86✅ᵛ a64✅ᵛ |
| [`candb`](../ports/candb/) | 0.1-git-20210625 | `apk add candb` | – | x86✅ᵛ a64✅ᵛ |
| [`capicxx-core-runtime`](../ports/capicxx-core-runtime/) | 3.2.4 | `apk add capicxx-core-runtime` | – | x86✅ᵛ a64✅ᵛ |
| [`capstone`](../ports/capstone/) | 5.0.9 | `apk add capstone` | `qnx-test-capstone` | x86✅ᵛ a64✅ᵛ |
| [`catch2`](../ports/catch2/) | 3.7.1 | `apk add catch2` | `qnx-test-catch2` | x86✅ᵛ a64✅ᵛ |
| [`cereal`](../ports/cereal/) | 1.3.2 | `apk add cereal` | `qnx-test-cereal` | x86✅ᵛ a64✅ᵛ |
| [`check`](../ports/check/) | 0.15.2 | `apk add check` | – | x86✅ᵛ a64✅ᵛ |
| [`chrony`](../ports/chrony/) | 4.8-git-20260226 | `apk add chrony` | – | x86✅ᵛ a64✅ᵛ |
| [`cjson`](../ports/cjson/) | 1.7.18 | `apk add cjson` | `qnx-test-cjson` | x86✅ᵛ a64✅ᵛ |
| [`clapack`](../ports/clapack/) | 3.2.1 | `apk add clapack` | – | x86✅ᵛ a64✅ᵛ |
| [`cli11`](../ports/cli11/) | 2.4.2 | `apk add cli11` | `qnx-test-cli11` | x86✅ᵛ a64✅ᵛ |
| [`computelibrary`](../ports/computelibrary/) | 53.0.0 | `apk add computelibrary` | – | a64✅ᵛ |
| [`console_bridge`](../ports/console_bridge/) | 1.0.2 | `apk add console_bridge` | `qnx-test-console-bridge` | x86✅ᵛ a64✅ᵛ |
| [`cpuinfo`](../ports/cpuinfo/) | 0-git-20240417 | `apk add cpuinfo` | – | x86✅ᵛ a64✅ᵛ |
| [`csmith`](../ports/csmith/) | 2.3.0 | `apk add csmith` | – | x86✅ᵛ a64✅ᵛ |
| [`cunit`](../ports/cunit/) | 2.1-3 | `apk add cunit` | – | x86✅ᵛ a64✅ᵛ |
| [`cyclonedds`](../ports/cyclonedds/) | 0.10.5 | `apk add cyclonedds` | `qnx-test-cyclonedds` | x86✅ᵛ a64✅ᵛ |
| [`dav1d`](../ports/dav1d/) | 1.5.0 | `apk add dav1d` | `qnx-test-dav1d` | x86✅ᵛ a64✅ᵛ |
| [`dnsmasq`](../ports/dnsmasq/) | 2.93test4 | `apk add dnsmasq` | – | x86✅ᵛ a64✅ᵛ |
| [`docbook-xsl`](../ports/docbook-xsl/) | 1.79.2 | `apk add docbook-xsl` | `qnx-test-docbook-xsl` | x86✅ᵛ a64✅ᵛ |
| [`double-conversion`](../ports/double-conversion/) | 3.3.0 | `apk add double-conversion` | `qnx-test-double-conversion` | x86✅ᵛ a64✅ᵛ |
| [`duktape`](../ports/duktape/) | 2.7.0 | `apk add duktape` | `qnx-test-duktape` | x86✅ᵛ a64✅ᵛ |
| [`ed`](../ports/ed/) | 1.22.5 | `apk add ed` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`eigen`](../ports/eigen/) | 3.4.0 | `apk add eigen` | `qnx-test-eigen` | x86✅ᵛ a64✅ᵛ |
| [`enca`](../ports/enca/) | 1.19 | `apk add enca` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`encodings`](../ports/encodings/) | 1.1.0 | `apk add encodings` | – | x86✅ᵛ a64✅ᵛ |
| [`expat`](../ports/expat/) | 2.6.4 | `apk add expat` | `qnx-test-expat` | x86✅ᵛ a64✅ᵛ |
| [`fakeroot`](../ports/fakeroot/) | 1.37.1.1 | `apk add fakeroot` | – | x86✅ᵛ a64✅ᵛ |
| [`farmhash`](../ports/farmhash/) | 1.1-git-20190513 | `apk add farmhash` | – | x86✅ᵛ a64✅ᵛ |
| [`fastcdr`](../ports/fastcdr/) | 2.2.6 | `apk add fastcdr` | `qnx-test-fastcdr` | x86✅ᵛ a64✅ᵛ |
| [`fdk-aac`](../ports/fdk-aac/) | 2.0.3 | `apk add fdk-aac` | `qnx-test-fdk-aac` | x86✅ᵛ a64✅ᵛ |
| [`flatbuffers`](../ports/flatbuffers/) | 24.3.25 | `apk add flatbuffers` | `qnx-test-flatbuffers` | x86✅ᵛ a64✅ᵛ |
| [`fmt`](../ports/fmt/) | 11.2.0 | `apk add fmt` | `qnx-test-fmt` | x86✅ᵛ a64✅ᵛ |
| [`font-cantarell`](../ports/font-cantarell/) | 0.303.1 | `apk add font-cantarell` | – | x86✅ᵛ a64✅ᵛ |
| [`font-dejavu`](../ports/font-dejavu/) | 2.37 | `apk add font-dejavu` | – | x86✅ᵛ a64✅ᵛ |
| [`font-hack`](../ports/font-hack/) | 3.003 | `apk add font-hack` | – | x86✅ᵛ a64✅ᵛ |
| [`font-inter`](../ports/font-inter/) | 4.1 | `apk add font-inter` | – | x86✅ᵛ a64✅ᵛ |
| [`font-liberation`](../ports/font-liberation/) | 2.1.5 | `apk add font-liberation` | – | x86✅ᵛ a64✅ᵛ |
| [`font-noto-emoji`](../ports/font-noto-emoji/) | 2.048 | `apk add font-noto-emoji` | – | x86✅ᵛ a64✅ᵛ |
| [`font-noto-sans-cjk`](../ports/font-noto-sans-cjk/) | 2.004 | `apk add font-noto-sans-cjk` | – | x86✅ᵛ a64✅ᵛ |
| [`font-noto-serif-cjk`](../ports/font-noto-serif-cjk/) | 2.003 | `apk add font-noto-serif-cjk` | – | x86✅ᵛ a64✅ᵛ |
| [`font-util`](../ports/font-util/) | 1.4.1 | `apk add font-util` | – | x86✅ᵛ a64✅ᵛ |
| [`foonathan_memory`](../ports/foonathan_memory/) | 0.7-3 | `apk add foonathan_memory` | `qnx-test-foonathan-memory` | x86✅ᵛ a64✅ᵛ |
| [`fp16`](../ports/fp16/) | 0-git-20260620 | `apk add fp16` | – | x86✅ᵛ a64✅ᵛ |
| [`freeimage`](../ports/freeimage/) | 3.18.0 | `apk add freeimage` | – | x86✅ᵛ a64✅ᵛ |
| [`fribidi`](../ports/fribidi/) | 1.0.16 | `apk add fribidi` | – | x86✅ᵛ a64✅ᵛ |
| [`fxdiv`](../ports/fxdiv/) | 0-git-20200417 | `apk add fxdiv` | – | x86✅ᵛ a64✅ᵛ |
| [`g3log`](../ports/g3log/) | 2.4 | `apk add g3log` | – | x86✅ᵛ a64✅ᵛ |
| [`gdbm`](../ports/gdbm/) | 1.24 | `apk add gdbm` | `qnx-test-gdbm` | x86✅ᵛ a64✅ᵛ |
| [`gflags`](../ports/gflags/) | 2.2.2 | `apk add gflags` | `qnx-test-gflags` | x86✅ᵛ a64✅ᵛ |
| [`giflib`](../ports/giflib/) | 5.2.2 | `apk add giflib` | `qnx-test-giflib` | x86✅ᵛ a64✅ᵛ |
| [`gklib`](../ports/gklib/) | 5.1.1-git-20230326 | `apk add gklib` | – | x86✅ᵛ a64✅ᵛ |
| [`glm`](../ports/glm/) | 1.0.1 | `apk add glm` | `qnx-test-glm` | x86✅ᵛ a64✅ᵛ |
| [`gmp`](../ports/gmp/) | 6.3.0 | `apk add gmp` | `qnx-test-gmp` | x86✅ᵛ a64✅ᵛ |
| [`googletest`](../ports/googletest/) | 1.17.0 | `apk add googletest` | `qnx-test-googletest` | x86✅ᵛ a64✅ᵛ |
| [`gperf`](../ports/gperf/) | 3.1 | `apk add gperf` | – | x86✅ᵛ a64✅ᵛ |
| [`graphite2`](../ports/graphite2/) | 1.3.14 | `apk add graphite2` | `qnx-test-graphite2` | x86✅ᵛ a64✅ᵛ |
| [`gsm`](../ports/gsm/) | 1.0.22 | `apk add gsm` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`hicolor-icon-theme`](../ports/hicolor-icon-theme/) | 0.18 | `apk add hicolor-icon-theme` | – | x86✅ᵛ a64✅ᵛ |
| [`highway`](../ports/highway/) | 1.2.0 | `apk add highway` | `qnx-test-highway` | x86✅ᵛ a64✅ᵛ |
| [`hyphen`](../ports/hyphen/) | 2.8.8 | `apk add hyphen` | `qnx-test-misc-libs` | x86✅ᵛ a64✅ᵛ |
| [`iceoryx`](../ports/iceoryx/) | 2.0.6 | `apk add iceoryx` | `qnx-test-iceoryx` | x86✅ᵛ a64✅ᵛ |
| [`icu`](../ports/icu/) | 76.1 | `apk add icu` | `qnx-test-icu` | x86✅ᵛ a64✅ᵛ |
| [`iso-codes`](../ports/iso-codes/) | 4.17.0 | `apk add iso-codes` | – | x86✅ᵛ a64✅ᵛ |
| [`jansson`](../ports/jansson/) | 2.14 | `apk add jansson` | `qnx-test-jansson` | x86✅ᵛ a64✅ᵛ |
| [`json-c`](../ports/json-c/) | 0.18 | `apk add json-c` | `qnx-test-json-c` | x86✅ᵛ a64✅ᵛ |
| [`jsoncpp`](../ports/jsoncpp/) | 1.9.6 | `apk add jsoncpp` | `qnx-test-jsoncpp` | x86✅ᵛ a64✅ᵛ |
| [`kineto`](../ports/kineto/) | 0-git-20250214 | `apk add kineto` | – | x86✅ᵛ a64✅ᵛ |
| [`lame`](../ports/lame/) | 3.100 | `apk add lame` | `qnx-test-lame` | x86✅ᵛ a64✅ᵛ |
| [`lely-core`](../ports/lely-core/) | 2.3.4 | `apk add lely-core` | `qnx-test-lely-core` | x86✅ᵛ a64✅ᵛ |
| [`libaec`](../ports/libaec/) | 1.1.7 | `apk add libaec` | `qnx-test-libaec` | x86✅ᵛ a64✅ᵛ |
| [`libcbor`](../ports/libcbor/) | 0.11.0 | `apk add libcbor` | `qnx-test-libcbor` | x86✅ᵛ a64✅ᵛ |
| [`libccd`](../ports/libccd/) | 2.1 | `apk add libccd` | `qnx-test-fcl` | x86✅ᵛ a64✅ᵛ |
| [`libconfig`](../ports/libconfig/) | 1.8.1 | `apk add libconfig` | `qnx-test-libconfig` | x86✅ᵛ a64✅ᵛ |
| [`libdatrie`](../ports/libdatrie/) | 0.2.13 | `apk add libdatrie` | `qnx-test-libdatrie` | x86✅ᵛ a64✅ᵛ |
| [`libde265`](../ports/libde265/) | 1.0.15 | `apk add libde265` | – | x86✅ᵛ a64✅ᵛ |
| [`libdeflate`](../ports/libdeflate/) | 1.22 | `apk add libdeflate` | `qnx-test-libdeflate` | x86✅ᵛ a64✅ᵛ |
| [`libdisplay-info`](../ports/libdisplay-info/) | 0.3.0 | `apk add libdisplay-info` | – | x86✅ᵛ a64✅ᵛ |
| [`libdrm-headers`](../ports/libdrm-headers/) | 2.4.129 | `apk add libdrm-headers` | – | x86✅ᵛ a64✅ᵛ |
| [`libev`](../ports/libev/) | 4.33 | `apk add libev` | `qnx-test-libev` | x86✅ᵛ a64✅ᵛ |
| [`libexif`](../ports/libexif/) | 0.6.24 | `apk add libexif` | `qnx-test-libexif` | x86✅ᵛ a64✅ᵛ |
| [`libfdt`](../ports/libfdt/) | 1.7.2 | `apk add libfdt` | `qnx-test-libfdt` | x86✅ᵛ a64✅ᵛ |
| [`libffi`](../ports/libffi/) | 3.4.6 | `apk add libffi` | `qnx-test-libffi` | x86✅ᵛ a64✅ᵛ |
| [`libiconv`](../ports/libiconv/) | 1.18 | `apk add libiconv` | – | x86✅ᵛ a64✅ᵛ |
| [`libinih`](../ports/libinih/) | 58 | `apk add libinih` | `qnx-test-libinih` | x86✅ᵛ a64✅ᵛ |
| [`libjpeg-turbo`](../ports/libjpeg-turbo/) | 3.0.4 | `apk add libjpeg-turbo` | – | x86✅ᵛ a64✅ᵛ |
| [`libjson`](../ports/libjson/) | 0.8-git-20260512 | `apk add libjson` | – | x86✅ᵛ a64✅ᵛ |
| [`libmd`](../ports/libmd/) | 1.1.0 | `apk add libmd` | `qnx-test-libmd` | x86✅ᵛ a64✅ᵛ |
| [`libmetal`](../ports/libmetal/) | 2024.10.0 | `apk add libmetal` | – | x86✅ᵛ a64✅ᵛ |
| [`libmicrohttpd`](../ports/libmicrohttpd/) | 1.0.1 | `apk add libmicrohttpd` | `qnx-test-libmicrohttpd` | x86✅ᵛ a64✅ᵛ |
| [`libmodbus`](../ports/libmodbus/) | 3.1.11 | `apk add libmodbus` | `qnx-test-libmodbus` | x86✅ᵛ a64✅ᵛ |
| [`libogg`](../ports/libogg/) | 1.3.5 | `apk add libogg` | – | x86✅ᵛ a64✅ᵛ |
| [`libpcap`](../ports/libpcap/) | 1.10.5 | `apk add libpcap` | `qnx-test-libpcap` | x86✅ᵛ a64✅ᵛ |
| [`libpipeline`](../ports/libpipeline/) | 1.5.8 | `apk add libpipeline` | – | x86✅ᵛ a64✅ᵛ |
| [`libretro-2048`](../ports/libretro-2048/) | 0-git-20241227 | `apk add libretro-2048` | – | x86✅ᵛ a64✅ᵛ |
| [`libretro-fake08`](../ports/libretro-fake08/) | 0-git-20260613 | `apk add libretro-fake08` | – | x86✅ᵛ a64✅ᵛ |
| [`libretro-mrboom`](../ports/libretro-mrboom/) | 0-git-20260625 | `apk add libretro-mrboom` | – | x86✅ᵛ a64✅ᵛ |
| [`libretro-retro8`](../ports/libretro-retro8/) | 0-git-20260912 | `apk add libretro-retro8` | – | x86✅ᵛ a64✅ᵛ |
| [`libretro-samples`](../ports/libretro-samples/) | 0-git-20221110 | `apk add libretro-samples` | – | x86✅ᵛ a64✅ᵛ |
| [`libsamplerate`](../ports/libsamplerate/) | 0.2.2 | `apk add libsamplerate` | `qnx-test-libsamplerate` | x86✅ᵛ a64✅ᵛ |
| [`libsfdo`](../ports/libsfdo/) | 0.1.4 | `apk add libsfdo` | `qnx-test-libsfdo` | x86✅ᵛ a64✅ᵛ |
| [`libsodium`](../ports/libsodium/) | 1.0.20 | `apk add libsodium` | `qnx-test-libsodium` | x86✅ᵛ a64✅ᵛ |
| [`libtasn1`](../ports/libtasn1/) | 4.19.0 | `apk add libtasn1` | `qnx-test-libtasn1` | x86✅ᵛ a64✅ᵛ |
| [`libtool`](../ports/libtool/) | 2.5.4 | `apk add libtool` | – | x86✅ᵛ a64✅ᵛ |
| [`libunibreak`](../ports/libunibreak/) | 6.1 | `apk add libunibreak` | `qnx-test-libunibreak` | x86✅ᵛ a64✅ᵛ |
| [`libunistring`](../ports/libunistring/) | 1.3 | `apk add libunistring` | `qnx-test-libunistring` | x86✅ᵛ a64✅ᵛ |
| [`libunwind`](../ports/libunwind/) | 1.8.3 | `apk add libunwind` | `qnx-test-libunwind` | x86✅ᵛ a64✅ᵛ |
| [`libusb`](../ports/libusb/) | 1.0.27 | `apk add libusb` | `qnx-test-libusb` | x86✅ᵛ a64✅ᵛ |
| [`libuv`](../ports/libuv/) | 1.51.0 | `apk add libuv` | `qnx-test-libuv` | x86✅ᵛ a64✅ᵛ |
| [`libvpx`](../ports/libvpx/) | 1.15.0 | `apk add libvpx` | `qnx-test-libvpx` | x86✅ᵛ a64✅ᵛ |
| [`libxcrypt`](../ports/libxcrypt/) | 4.4.38 | `apk add libxcrypt` | – | x86✅ᵛ a64✅ᵛ |
| [`libxcvt`](../ports/libxcvt/) | 0.1.3 | `apk add libxcvt` | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libyaml`](../ports/libyaml/) | 0.2.5 | `apk add libyaml` | `qnx-test-libyaml` | x86✅ᵛ a64✅ᵛ |
| [`linux-headers-input-codes`](../ports/linux-headers-input-codes/) | 6.12 | `apk add linux-headers-input-codes` | – | x86✅ᵛ a64✅ᵛ |
| [`lmbench`](../ports/lmbench/) | 3.0-git-20260929 | `apk add lmbench` | – | x86✅ᵛ a64✅ᵛ |
| [`lua`](../ports/lua/) | 5.4.7 | `apk add lua` | `qnx-test-lua` | x86✅ᵛ a64✅ᵛ |
| [`lua5.1`](../ports/lua5.1/) | 5.1.5 | `apk add lua5.1` | – | x86✅ᵛ a64✅ᵛ |
| [`lua5.3`](../ports/lua5.3/) | 5.3.6 | `apk add lua5.3` | – | x86✅ᵛ a64✅ᵛ |
| [`luajit`](../ports/luajit/) | 2.1.c6ffc141 | `apk add luajit` | `qnx-test-luajit` | x86✅ᵛ a64✅ᵛ |
| [`lxde-icon-theme`](../ports/lxde-icon-theme/) | 0.5.2 | `apk add lxde-icon-theme` | – | x86✅ᵛ a64✅ᵛ |
| [`lz4`](../ports/lz4/) | 1.10.0 | `apk add lz4` | `qnx-test-lz4` | x86✅ᵛ a64✅ᵛ |
| [`lzip`](../ports/lzip/) | 1.26 | `apk add lzip` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`lzo`](../ports/lzo/) | 2.10 | `apk add lzo` | `qnx-test-lzo` | x86✅ᵛ a64✅ᵛ |
| [`magic_enum`](../ports/magic_enum/) | 0.9.6 | `apk add magic_enum` | `qnx-test-magic-enum` | x86✅ᵛ a64✅ᵛ |
| [`make`](../ports/make/) | 4.4.1 | `apk add make` | – | x86✅ᵛ a64✅ᵛ |
| [`mbedtls`](../ports/mbedtls/) | 3.6.2 | `apk add mbedtls` | `qnx-test-mbedtls` | x86✅ᵛ a64✅ᵛ |
| [`mimalloc`](../ports/mimalloc/) | 3.1.5 | `apk add mimalloc` | `qnx-test-mimalloc` | x86✅ᵛ a64✅ᵛ |
| [`miniz`](../ports/miniz/) | 3.1.2 | `apk add miniz` | `qnx-test-miniz` | x86✅ᵛ a64✅ᵛ |
| [`mpg123`](../ports/mpg123/) | 1.32.9 | `apk add mpg123` | `qnx-test-mpg123` | x86✅ᵛ a64✅ᵛ |
| [`msgpack-c`](../ports/msgpack-c/) | 6.1.0 | `apk add msgpack-c` | `qnx-test-msgpack` | x86✅ᵛ a64✅ᵛ |
| [`mujoco`](../ports/mujoco/) | 3.10.0 | `apk add mujoco` | `qnx-test-mujoco` | x86✅ᵛ a64✅ᵛ |
| [`muslflt`](../ports/muslflt/) | 1.0-git-20240802 | `apk add muslflt` | – | x86✅ᵛ a64✅ᵛ |
| [`nanosvg`](../ports/nanosvg/) | 0-git-20260709 | `apk add nanosvg` | – | x86✅ᵛ a64✅ᵛ |
| [`ncnn`](../ports/ncnn/) | 20260526 | `apk add ncnn` | `qnx-test-ncnn` | x86✅ᵛ a64✅ᵛ |
| [`ncurses`](../ports/ncurses/) | 6.5 | `apk add ncurses` | `qnx-test-ncurses` | x86✅ᵛ a64✅ᵛ |
| [`ninja`](../ports/ninja/) | 1.13.1 | `apk add ninja` | – | x86✅ᵛ a64✅ᵛ |
| [`nlohmann_json`](../ports/nlohmann_json/) | 3.12.0 | `apk add nlohmann_json` | `qnx-test-nlohmann-json` | x86✅ᵛ a64✅ᵛ |
| [`nng`](../ports/nng/) | 1.9.0 | `apk add nng` | `qnx-test-nng` | x86✅ᵛ a64✅ᵛ |
| [`octomap`](../ports/octomap/) | 1.10.0 | `apk add octomap` | `qnx-test-octomap` | x86✅ᵛ a64✅ᵛ |
| [`onetbb`](../ports/onetbb/) | 2022.0.0 | `apk add onetbb` | – | x86✅ᵛ a64✅ᵛ |
| [`oniguruma`](../ports/oniguruma/) | 6.9.9 | `apk add oniguruma` | `qnx-test-oniguruma` | x86✅ᵛ a64✅ᵛ |
| [`openal-soft`](../ports/openal-soft/) | 1.24.1 | `apk add openal-soft` | – | x86✅ᵛ a64✅ᵛ |
| [`openblas`](../ports/openblas/) | 0.3.28 | `apk add openblas` | – | x86✅ᵛ a64✅ᵛ |
| [`openh264`](../ports/openh264/) | 2.5.0 | `apk add openh264` | `qnx-test-openh264` | x86✅ᵛ a64✅ᵛ |
| [`openssl`](../ports/openssl/) | 3.5.1 | `apk add openssl` | `qnx-test-openssl` | x86✅ᵛ a64✅ᵛ |
| [`openttd-opengfx`](../ports/openttd-opengfx/) | 8.0 | `apk add openttd-opengfx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`openttd-openmsx`](../ports/openttd-openmsx/) | 0.4.2 | `apk add openttd-openmsx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`openttd-opensfx`](../ports/openttd-opensfx/) | 1.0.3 | `apk add openttd-opensfx` | `qnx-test-openttd-sets` | x86✅ᵛ a64✅ᵛ |
| [`opus`](../ports/opus/) | 1.5.2 | `apk add opus` | `qnx-test-opus` | x86✅ᵛ a64✅ᵛ |
| [`orc`](../ports/orc/) | 0.4.40 | `apk add orc` | `qnx-test-orc` | x86✅ᵛ a64✅ᵛ |
| [`patch`](../ports/patch/) | 2.8 | `apk add patch` | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`pax-utils`](../ports/pax-utils/) | 1.3.8 | `apk add pax-utils` | – | x86✅ᵛ a64✅ᵛ |
| [`pcre`](../ports/pcre/) | 8.45 | `apk add pcre` | – | x86✅ᵛ a64✅ᵛ |
| [`pdfium`](../ports/pdfium/) | 0-git-84a8011 | `apk add pdfium` | – | x86✅ᵛ a64✅ᵛ |
| [`picocom`](../ports/picocom/) | 3.1 | `apk add picocom` | – | x86✅ᵛ a64✅ᵛ |
| [`pkgconf`](../ports/pkgconf/) | 2.3.0 | `apk add pkgconf` | – | x86✅ᵛ a64✅ᵛ |
| [`protobuf-c`](../ports/protobuf-c/) | 1.5.0 | `apk add protobuf-c` | `qnx-test-protobuf-c` | x86✅ᵛ a64✅ᵛ |
| [`pthread-stubs`](../ports/pthread-stubs/) | 0.5 | `apk add pthread-stubs` | – | x86✅ᵛ a64✅ᵛ |
| [`pugixml`](../ports/pugixml/) | 1.14 | `apk add pugixml` | `qnx-test-pugixml` | x86✅ᵛ a64✅ᵛ |
| [`quickjs-ng`](../ports/quickjs-ng/) | 0.8.0 | `apk add quickjs-ng` | `qnx-test-quickjs` | x86✅ᵛ a64✅ᵛ |
| [`rapidjson`](../ports/rapidjson/) | 1.1.0-24b5e7a | `apk add rapidjson` | `qnx-test-rapidjson` | x86✅ᵛ a64✅ᵛ |
| [`rust`](../ports/rust/) | 1.90.0 | `apk add rust` | – | x86✅ᵛ a64✅ᵛ |
| [`simdjson`](../ports/simdjson/) | 3.11.3 | `apk add simdjson` | `qnx-test-simdjson` | x86✅ᵛ a64✅ᵛ |
| [`simdutf`](../ports/simdutf/) | 6.4.2 | `apk add simdutf` | `qnx-test-simdutf` | x86✅ᵛ a64✅ᵛ |
| [`sleef`](../ports/sleef/) | 3.8-git-20250214 | `apk add sleef` | – | x86✅ᵛ a64✅ᵛ |
| [`snappy`](../ports/snappy/) | 1.2.1 | `apk add snappy` | `qnx-test-snappy` | x86✅ᵛ a64✅ᵛ |
| [`soxr`](../ports/soxr/) | 0.1.3 | `apk add soxr` | `qnx-test-soxr` | x86✅ᵛ a64✅ᵛ |
| [`speexdsp`](../ports/speexdsp/) | 1.2.1 | `apk add speexdsp` | `qnx-test-speexdsp` | x86✅ᵛ a64✅ᵛ |
| [`stable-diffusion.cpp`](../ports/stable-diffusion.cpp/) | 0_git20260601 | `apk add stable-diffusion.cpp` | `qnx-test-sdcpp` | x86✅ᵛ a64✅ᵛ |
| [`svt-av1`](../ports/svt-av1/) | 2.3.0 | `apk add svt-av1` | `qnx-test-svt-av1` | x86✅ᵛ a64✅ᵛ |
| [`tensorflow-lite`](../ports/tensorflow-lite/) | 2.16.1 | `apk add tensorflow-lite` | `qnx-test-tflite` | x86✅ᵛ a64✅ᵛ |
| [`tinyxml2`](../ports/tinyxml2/) | 10.0.0 | `apk add tinyxml2` | `qnx-test-tinyxml2` | x86✅ᵛ a64✅ᵛ |
| [`tomlplusplus`](../ports/tomlplusplus/) | 3.4.0 | `apk add tomlplusplus` | `qnx-test-tomlplusplus` | x86✅ᵛ a64✅ᵛ |
| [`tree`](../ports/tree/) | 2.2.1 | `apk add tree` | `qnx-test-tree-sitter` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter`](../ports/tree-sitter/) | 0.25.9 | `apk add tree-sitter` | `qnx-test-tree-sitter` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-bash`](../ports/tree-sitter-bash/) | 0.25.0 | `apk add tree-sitter-bash` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-c`](../ports/tree-sitter-c/) | 0.24.1 | `apk add tree-sitter-c` | – | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-cmake`](../ports/tree-sitter-cmake/) | 0.7.2 | `apk add tree-sitter-cmake` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-lua`](../ports/tree-sitter-lua/) | 0.4.0 | `apk add tree-sitter-lua` | – | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-make`](../ports/tree-sitter-make/) | 1.1.1 | `apk add tree-sitter-make` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-markdown`](../ports/tree-sitter-markdown/) | 0.5.1 | `apk add tree-sitter-markdown` | – | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-python`](../ports/tree-sitter-python/) | 0.23.6 | `apk add tree-sitter-python` | `qnx-test-ts-grammars` | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-query`](../ports/tree-sitter-query/) | 0.7.0 | `apk add tree-sitter-query` | – | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-vim`](../ports/tree-sitter-vim/) | 0.7.0 | `apk add tree-sitter-vim` | – | x86✅ᵛ a64✅ᵛ |
| [`tree-sitter-vimdoc`](../ports/tree-sitter-vimdoc/) | 4.0.0 | `apk add tree-sitter-vimdoc` | – | x86✅ᵛ a64✅ᵛ |
| [`trompeloeil`](../ports/trompeloeil/) | 48 | `apk add trompeloeil` | – | x86✅ᵛ a64✅ᵛ |
| [`unibilium`](../ports/unibilium/) | 2.1.2 | `apk add unibilium` | `qnx-test-unibilium` | x86✅ᵛ a64✅ᵛ |
| [`urdfdom_headers`](../ports/urdfdom_headers/) | 1.1.2 | `apk add urdfdom_headers` | – | x86✅ᵛ a64✅ᵛ |
| [`uriparser`](../ports/uriparser/) | 0.9.8 | `apk add uriparser` | `qnx-test-uriparser` | x86✅ᵛ a64✅ᵛ |
| [`utf8proc`](../ports/utf8proc/) | 2.11.3 | `apk add utf8proc` | `qnx-test-utf8proc` | x86✅ᵛ a64✅ᵛ |
| [`vidstab`](../ports/vidstab/) | 1.1.1 | `apk add vidstab` | `qnx-test-vidstab` | x86✅ᵛ a64✅ᵛ |
| [`webrtc`](../ports/webrtc/) | m132-afaf497 | `apk add webrtc` | – | x86✅ᵛ a64✅ᵛ |
| [`whisper.cpp`](../ports/whisper.cpp/) | 0_git20260728 | `apk add whisper.cpp` | `qnx-test-whisper` | x86✅ᵛ a64✅ᵛ |
| [`wlr-protocols`](../ports/wlr-protocols/) | 0-git-20250702 | `apk add wlr-protocols` | – | x86✅ᵛ a64✅ᵛ |
| [`wolfssl`](../ports/wolfssl/) | 5.7.6 | `apk add wolfssl` | `qnx-test-wolfssl` | x86✅ᵛ a64✅ᵛ |
| [`x264`](../ports/x264/) | stable | `apk add x264` | `qnx-test-x264` | x86✅ᵛ a64✅ᵛ |
| [`x265`](../ports/x265/) | 3.6 | `apk add x265` | `qnx-test-x265` | x86✅ᵛ a64✅ᵛ |
| [`xcb-proto`](../ports/xcb-proto/) | 1.17.0 | `apk add xcb-proto` | – | x86✅ᵛ a64✅ᵛ |
| [`xdg-utils`](../ports/xdg-utils/) | 1.2.1 | `apk add xdg-utils` | – | x86✅ᵛ a64✅ᵛ |
| [`xkeyboard-config`](../ports/xkeyboard-config/) | 2.43 | `apk add xkeyboard-config` | – | x86✅ᵛ a64✅ᵛ |
| [`xorgproto`](../ports/xorgproto/) | 2024.1 | `apk add xorgproto` | – | x86✅ᵛ a64✅ᵛ |
| [`xtrans`](../ports/xtrans/) | 1.5.2 | `apk add xtrans` | – | x86✅ᵛ a64✅ᵛ |
| [`xvidcore`](../ports/xvidcore/) | 1.3.7 | `apk add xvidcore` | `qnx-test-xvidcore` | x86✅ᵛ a64✅ᵛ |
| [`xxhash`](../ports/xxhash/) | 0.8.2 | `apk add xxhash` | `qnx-test-xxhash` | x86✅ᵛ a64✅ᵛ |
| [`yaml-cpp`](../ports/yaml-cpp/) | 0.8.0 | `apk add yaml-cpp` | `qnx-test-yaml-cpp` | x86✅ᵛ a64✅ᵛ |
| [`zimg`](../ports/zimg/) | 3.0.5 | `apk add zimg` | `qnx-test-zimg` | x86✅ᵛ a64✅ᵛ |
| [`zlib`](../ports/zlib/) | 1.3.1 | `apk add zlib` | `qnx-test-zlib` | x86✅ᵛ a64✅ᵛ |
| [`zlib-ng`](../ports/zlib-ng/) | 2.2.5 | `apk add zlib-ng` | – | x86✅ᵛ a64✅ᵛ |
| [`zstd`](../ports/zstd/) | 1.5.6 | `apk add zstd` | `qnx-test-zstd` | x86✅ᵛ a64✅ᵛ |

### Dependent (301)

| Component | Version | Install | Needs at run time | Needs to build | Test package | Verified |
|---|---|---|---|---|---|---|
| [`abuild`](../ports/abuild/) | 3.16.0 | `apk add abuild` | openssl zlib | openssl zlib apk-tools | – | x86✅ᵛ a64✅ᵛ |
| [`adwaita-icon-theme`](../ports/adwaita-icon-theme/) | 51.0 | `apk add adwaita-icon-theme` | – | hicolor-icon-theme | `qnx-test-adwaita` | x86✅ᵛ a64✅ᵛ |
| [`amqp-cpp`](../ports/amqp-cpp/) | 4.3.27 | `apk add amqp-cpp` | – | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`apk-tools`](../ports/apk-tools/) | 3.0.0_rc5 | `apk add apk-tools` | openssl zlib zstd | openssl zlib zstd | – | x86✅ᵛ a64✅ᵛ |
| [`aspell`](../ports/aspell/) | 0.60.8.1 | `apk add aspell` | ncurses | ncurses | – | x86✅ᵛ a64✅ᵛ |
| [`at-spi2-core`](../ports/at-spi2-core/) | 2.54.0 | `apk add at-spi2-core` | dbus gettext glib libx11 libxi libxtst | glib dbus libxml2 libx11 libxtst | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-auth`](../ports/aws-c-auth/) | 0.10.1 | `apk add aws-c-auth` | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils openssl s2n-tls | aws-c-http aws-c-sdkutils | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-cal`](../ports/aws-c-cal/) | 0.9.13 | `apk add aws-c-cal` | aws-c-common openssl | aws-c-common openssl | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-compression`](../ports/aws-c-compression/) | 0.3.2 | `apk add aws-c-compression` | aws-c-common | aws-c-common | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-event-stream`](../ports/aws-c-event-stream/) | 0.7.0 | `apk add aws-c-event-stream` | aws-c-cal aws-c-common aws-c-io aws-checksums openssl s2n-tls | aws-c-io aws-checksums | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-http`](../ports/aws-c-http/) | 0.10.14 | `apk add aws-c-http` | aws-c-cal aws-c-common aws-c-compression aws-c-io openssl s2n-tls | aws-c-io aws-c-compression | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-io`](../ports/aws-c-io/) | 0.26.1 | `apk add aws-c-io` | aws-c-cal aws-c-common openssl s2n-tls | aws-c-common aws-c-cal s2n-tls | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-mqtt`](../ports/aws-c-mqtt/) | 0.15.2 | `apk add aws-c-mqtt` | aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io openssl s2n-tls | aws-c-http | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-s3`](../ports/aws-c-s3/) | 0.12.2 | `apk add aws-c-s3` | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-http aws-c-io aws-c-sdkutils aws-checksums openssl s2n-tls | aws-c-auth aws-checksums | – | x86✅ᵛ a64✅ᵛ |
| [`aws-c-sdkutils`](../ports/aws-c-sdkutils/) | 0.2.4 | `apk add aws-c-sdkutils` | aws-c-common | aws-c-common | – | x86✅ᵛ a64✅ᵛ |
| [`aws-checksums`](../ports/aws-checksums/) | 0.2.10 | `apk add aws-checksums` | aws-c-common | aws-c-common | – | x86✅ᵛ a64✅ᵛ |
| [`aws-crt-cpp`](../ports/aws-crt-cpp/) | 0.38.5 | `apk add aws-crt-cpp` | aws-c-auth aws-c-cal aws-c-common aws-c-compression aws-c-event-stream aws-c-http aws-c-io aws-c-mqtt aws-c-s3 aws-c-sdkutils aws-checksums openssl s2n-tls | aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3 | – | x86✅ᵛ a64✅ᵛ |
| [`azure-iot-sdk-c`](../ports/azure-iot-sdk-c/) | LTS_08_2024 | `apk add azure-iot-sdk-c` | curl openssl | openssl curl | – | x86✅ᵛ a64✅ᵛ |
| [`bash`](../ports/bash/) | 5.2.37 | `apk add bash` | libiconv ncurses readline | ncurses readline libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`bc`](../ports/bc/) | 1.08.2 | `apk add bc` | ncurses readline | readline | – | x86✅ᵛ a64✅ᵛ |
| [`binutils`](../ports/binutils/) | 2.43 | `apk add binutils` | zlib zstd | zlib zstd | – | x86✅ᵛ a64✅ᵛ |
| [`boost`](../ports/boost/) | 1.86.0 | `apk add boost` | bzip2 xz zlib | zlib bzip2 xz | `qnx-test-boost` | x86✅ᵛ a64✅ᵛ |
| [`cairo`](../ports/cairo/) | 1.18.2 | `apk add cairo` | fontconfig freetype glib libpng libx11 libxext libxrender lzo pixman zlib | pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo | – | x86✅ᵛ a64✅ᵛ |
| [`capicxx-someip-runtime`](../ports/capicxx-someip-runtime/) | 3.2.4 | `apk add capicxx-someip-runtime` | capicxx-core-runtime vsomeip | capicxx-core-runtime vsomeip | – | x86✅ᵛ a64✅ᵛ |
| [`capnproto`](../ports/capnproto/) | 1.0.2 | `apk add capnproto` | openssl zlib | zlib openssl | – | x86✅ᵛ a64✅ᵛ |
| [`ceres-solver`](../ports/ceres-solver/) | 2.2.0 | `apk add ceres-solver` | gflags glog | eigen glog gflags | – | x86✅ᵛ a64✅ᵛ |
| [`civetweb`](../ports/civetweb/) | 1.16 | `apk add civetweb` | openssl | openssl | `qnx-test-civetweb` | x86✅ᵛ a64✅ᵛ |
| [`cmake`](../ports/cmake/) | 4.1.1 | `apk add cmake` | curl libuv openssl zlib | openssl zlib curl libuv | – | x86✅ᵛ a64✅ᵛ |
| [`cppzmq`](../ports/cppzmq/) | 4.10.0 | `apk add cppzmq` | – | libzmq | `qnx-test-libzmq` | x86✅ᵛ a64✅ᵛ |
| [`curl`](../ports/curl/) | 8.11.1 | `apk add curl` | brotli c-ares nghttp2 openssl zlib zstd | openssl zlib nghttp2 c-ares brotli zstd | `qnx-test-curl` | x86✅ᵛ a64✅ᵛ |
| [`dbus`](../ports/dbus/) | 1.16.0 | `apk add dbus` | expat | expat | `qnx-test-dbus` | x86✅ᵛ a64✅ᵛ |
| [`desktop-file-utils`](../ports/desktop-file-utils/) | 0.28 | `apk add desktop-file-utils` | gettext glib | glib | – | x86✅ᵛ a64✅ᵛ |
| [`dlt-daemon`](../ports/dlt-daemon/) | 2.18.10 | `apk add dlt-daemon` | libjson | zlib libjson | – | x86✅ᵛ a64✅ᵛ |
| [`doxygen`](../ports/doxygen/) | 1.17.0 | `apk add doxygen` | libiconv | libiconv | `qnx-test-doxygen` | x86✅ᵛ a64✅ᵛ |
| [`dump1090`](../ports/dump1090/) | 0-git-20240121 | `apk add dump1090` | rtl-sdr | rtl-sdr | – | x86✅ᵛ a64✅ᵛ |
| [`emacs`](../ports/emacs/) | 30.2 | `apk add emacs` | dbus gmp gnutls lcms2 libxml2 ncurses sqlite tree-sitter zlib | ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter | – | x86✅ᵛ a64✅ᵛ |
| [`emulationstation`](../ports/emulationstation/) | 2.11-git-20241212 | `apk add emulationstation` | curl freeimage freetype pugixml sdl2 vlc | sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc | – | x86✅ᵛ a64✅ᵛ |
| [`enchant`](../ports/enchant/) | 2.8.2 | `apk add enchant` | aspell gettext glib | glib aspell | `qnx-test-enchant` | x86✅ᵛ a64✅ᵛ |
| [`epiphany`](../ports/epiphany/) | 43.1 | `apk add epiphany` | at-spi2-core cairo gcr gdk-pixbuf gettext glib gmp gtk3 json-glib libarchive libdazzle libhandy libportal libsecret libsoup libxml2 nettle pango sqlite webkitgtk | webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle | – | x86✅ᵛ a64✅ᵛ |
| [`exiv2`](../ports/exiv2/) | 0.28.5 | `apk add exiv2` | brotli expat libinih zlib | zlib expat brotli libinih | `qnx-test-exiv2` | x86✅ᵛ a64✅ᵛ |
| [`exo`](../ports/exo/) | 4.20.0 | `apk add exo` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | glib gtk3 libxfce4util libxfce4ui | – | x86✅ᵛ a64✅ᵛ |
| [`fastdds`](../ports/fastdds/) | 2.14.4 | `apk add fastdds` | fastcdr foonathan_memory openssl tinyxml2 | fastcdr foonathan_memory tinyxml2 openssl | `qnx-test-fastdds` | x86✅ᵛ a64✅ᵛ |
| [`fcl`](../ports/fcl/) | 0.7.0 | `apk add fcl` | libccd octomap | eigen libccd octomap | `qnx-test-fcl` | x86✅ᵛ a64✅ᵛ |
| [`ffmpeg`](../ports/ffmpeg/) | 7.1.1 | `apk add ffmpeg` | bzip2 dav1d lame libtheora libvorbis libvpx openh264 opus x264 x265 xz zlib | zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame | `qnx-test-ffmpeg` | x86✅ᵛ a64✅ᵛ |
| [`ffmpegthumbnailer`](../ports/ffmpegthumbnailer/) | 2.2.3 | `apk add ffmpegthumbnailer` | ffmpeg libjpeg-turbo libpng zlib | ffmpeg libpng libjpeg-turbo | – | x86✅ᵛ a64✅ᵛ |
| [`file`](../ports/file/) | 5.46 | `apk add file` | bzip2 xz zlib zstd | zlib bzip2 xz zstd | – | x86✅ᵛ a64✅ᵛ |
| [`flac`](../ports/flac/) | 1.4.3 | `apk add flac` | libogg | libogg | `qnx-test-flac` | x86✅ᵛ a64✅ᵛ |
| [`flann`](../ports/flann/) | 1.9.2 | `apk add flann` | lz4 | lz4 | `qnx-test-flann` | x86✅ᵛ a64✅ᵛ |
| [`fontconfig`](../ports/fontconfig/) | 2.15.0 | `apk add fontconfig` | expat freetype | freetype expat | `qnx-test-fontconfig` | x86✅ᵛ a64✅ᵛ |
| [`fontenc`](../ports/fontenc/) | 1.1.8 | `apk add fontenc` | zlib | xorgproto zlib | – | x86✅ᵛ a64✅ᵛ |
| [`freetype`](../ports/freetype/) | 2.13.3 | `apk add freetype` | brotli bzip2 libpng zlib | zlib bzip2 libpng brotli | `qnx-test-freetype` | x86✅ᵛ a64✅ᵛ |
| [`garcon`](../ports/garcon/) | 4.20.0 | `apk add garcon` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4ui libxfce4util pango zlib | glib gtk3 libxfce4util libxfce4ui | – | x86✅ᵛ a64✅ᵛ |
| [`gawk`](../ports/gawk/) | 5.3.2 | `apk add gawk` | ncurses readline | readline ncurses | – | x86✅ᵛ a64✅ᵛ |
| [`gcr`](../ports/gcr/) | 3.41.2 | `apk add gcr` | cairo gdk-pixbuf gettext glib gtk3 libgcrypt p11-kit pango | glib libgcrypt p11-kit libsecret gtk3 | – | x86✅ᵛ a64✅ᵛ |
| [`gdb`](../ports/gdb/) | 14.2 | `apk add gdb` | expat gmp libiconv mpfr ncurses readline xxhash xz zlib zstd | gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd | – | x86✅ᵛ a64✅ᵛ |
| [`gdk-pixbuf`](../ports/gdk-pixbuf/) | 2.42.12 | `apk add gdk-pixbuf` | gettext glib libjpeg-turbo libpng | glib libpng libjpeg-turbo | `qnx-test-gdk-pixbuf` | x86✅ᵛ a64✅ᵛ |
| [`geany`](../ports/geany/) | 2.1 | `apk add geany` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango zlib | glib gtk3 | – | x86✅ᵛ a64✅ᵛ |
| [`geany-plugins`](../ports/geany-plugins/) | 2.1 | `apk add geany-plugins` | at-spi2-core cairo gdk-pixbuf geany gettext glib gtk3 harfbuzz pango zlib | geany | – | x86✅ᵛ a64✅ᵛ |
| [`gettext`](../ports/gettext/) | 0.22.5 | `apk add gettext` | libiconv libunistring ncurses | ncurses libiconv libunistring | – | x86✅ᵛ a64✅ᵛ |
| [`git`](../ports/git/) | 2.51.0 | `apk add git` | curl expat libiconv openssl pcre2 zlib | zlib openssl curl expat pcre2 perl perl-error libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`glib`](../ports/glib/) | 2.82.4 | `apk add glib` | gettext libffi libiconv pcre2 zlib | libffi pcre2 gettext zlib | `qnx-test-glib` | x86✅ᵛ a64✅ᵛ |
| [`glib-networking`](../ports/glib-networking/) | 2.80.1 | `apk add glib-networking` | gettext glib gnutls libproxy | glib gnutls libproxy | – | x86✅ᵛ a64✅ᵛ |
| [`glog`](../ports/glog/) | 0.7.1 | `apk add glog` | gflags | gflags | `qnx-test-glog` | x86✅ᵛ a64✅ᵛ |
| [`gnutls`](../ports/gnutls/) | 3.8.8 | `apk add gnutls` | brotli gmp libidn2 libtasn1 libunistring nettle zlib zstd | nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd | `qnx-test-gnutls` | x86✅ᵛ a64✅ᵛ |
| [`gobject-introspection`](../ports/gobject-introspection/) | 1.86.0 | `apk add gobject-introspection` | glib libffi | glib libffi python3 | – | x86✅ᵛ a64✅ᵛ |
| [`godot`](../ports/godot/) | 4.4.1 | `apk add godot` | brotli bzip2 freetype gettext glib graphite2 harfbuzz libiconv libogg libpng libtheora libvorbis libwebp libxkbcommon pcre2 zlib zstd | freetype harfbuzz graphite2 libogg libtheora libpng libvorbis libwebp opus pcre2 zlib zstd brotli libxkbcommon | – | x86✅ᵛ a64✅ᵛ |
| [`graphene`](../ports/graphene/) | 1.10.8 | `apk add graphene` | glib | glib | `qnx-test-graphene` | x86✅ᵛ a64✅ᵛ |
| [`graphicsmagick`](../ports/graphicsmagick/) | 1.3.46 | `apk add graphicsmagick` | bzip2 freetype lcms2 libjpeg-turbo libjxl libpng libtiff libwebp libxml2 libzip xz zlib zstd | libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2 | `qnx-test-graphicsmagick` | x86✅ᵛ a64✅ᵛ |
| [`grep`](../ports/grep/) | 3.11 | `apk add grep` | libiconv pcre2 | pcre2 libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`groff`](../ports/groff/) | 1.23.0 | `apk add groff` | libiconv | libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`grpc`](../ports/grpc/) | 1.69.0 | `apk add grpc` | abseil c-ares openssl protobuf re2 zlib | zlib openssl c-ares abseil protobuf re2 | `qnx-test-grpc` | x86✅ᵛ a64✅ᵛ |
| [`gsettings-desktop-schemas`](../ports/gsettings-desktop-schemas/) | 49.0 | `apk add gsettings-desktop-schemas` | – | glib | – | x86✅ᵛ a64✅ᵛ |
| [`gst-libav`](../ports/gst-libav/) | 1.24.10 | `apk add gst-libav` | ffmpeg glib gst-plugins-base gstreamer | gst-plugins-base ffmpeg | – | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-bad`](../ports/gst-plugins-bad/) | 1.24.10 | `apk add gst-plugins-bad` | aom cairo fdk-aac glib gst-plugins-base gstreamer libwebp openh264 pango svt-av1 x265 | gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo | – | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-base`](../ports/gst-plugins-base/) | 1.24.10 | `apk add gst-plugins-base` | gettext glib gstreamer libogg libtheora libvorbis opus zlib | gstreamer orc libogg libvorbis opus libtheora zlib | – | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-good`](../ports/gst-plugins-good/) | 1.24.10 | `apk add gst-plugins-good` | bzip2 cairo flac gdk-pixbuf gettext glib gst-plugins-base gstreamer lame libjpeg-turbo libpng libsoup libvpx zlib | gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2 | `qnx-test-gst-plugins-good` | x86✅ᵛ a64✅ᵛ |
| [`gst-plugins-ugly`](../ports/gst-plugins-ugly/) | 1.24.10 | `apk add gst-plugins-ugly` | gettext glib gst-plugins-base gstreamer x264 | gst-plugins-base x264 | – | x86✅ᵛ a64✅ᵛ |
| [`gstreamer`](../ports/gstreamer/) | 1.24.10 | `apk add gstreamer` | gettext glib | glib libxml2 | `qnx-test-gstreamer` | x86✅ᵛ a64✅ᵛ |
| [`gthumb`](../ports/gthumb/) | 3.12.8.2 | `apk add gthumb` | cairo exiv2 gdk-pixbuf gettext glib gtk3 lcms2 libjpeg-turbo libpng libtiff libwebp pango zlib | glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib | – | x86✅ᵛ a64✅ᵛ |
| [`gtk-layer-shell`](../ports/gtk-layer-shell/) | 0.9.0 | `apk add gtk-layer-shell` | cairo glib gtk3 wayland | glib gtk3 wayland wayland-protocols | `qnx-test-gtk-layer-shell` | x86✅ᵛ a64✅ᵛ |
| [`gtk-vnc`](../ports/gtk-vnc/) | 1.3.1 | `apk add gtk-vnc` | cairo gdk-pixbuf gettext glib gnutls gtk3 libgcrypt libx11 zlib | glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11 | `qnx-test-gtk-vnc` | x86✅ᵛ a64✅ᵛ |
| [`gtk3`](../ports/gtk3/) | 3.24.43 | `apk add gtk3` | at-spi2-core cairo fontconfig fribidi gdk-pixbuf gettext glib harfbuzz libepoxy libx11 libxcomposite libxcursor libxdamage libxext libxfixes libxi libxinerama libxkbcommon libxrandr pango wayland | glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama | `qnx-test-gtk3` | x86✅ᵛ a64✅ᵛ |
| [`gtk4`](../ports/gtk4/) | 4.16.7 | `apk add gtk4` | cairo fontconfig fribidi gdk-pixbuf gettext glib graphene harfbuzz libepoxy libjpeg-turbo libpng libtiff libxkbcommon pango wayland | glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff | `qnx-test-gtk4` | x86✅ᵛ a64✅ᵛ |
| [`gtsam`](../ports/gtsam/) | 4.2 | `apk add gtsam` | boost | boost eigen | – | x86✅ᵛ a64✅ᵛ |
| [`harfbuzz`](../ports/harfbuzz/) | 10.1.0 | `apk add harfbuzz` | freetype glib graphite2 icu | freetype glib icu graphite2 | – | x86✅ᵛ a64✅ᵛ |
| [`htop`](../ports/htop/) | 3.5.0 | `apk add htop` | ncurses | ncurses | `qnx-test-monitors` | x86✅ᵛ a64✅ᵛ |
| [`iperf3`](../ports/iperf3/) | 3.17.1 | `apk add iperf3` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`itstool`](../ports/itstool/) | 2.0.7 | `apk add itstool` | – | python3 libxml2 | – | x86✅ᵛ a64✅ᵛ |
| [`javascriptcore`](../ports/javascriptcore/) | 2.44.4 | `apk add javascriptcore` | icu | icu | `qnx-test-javascriptcore` | x86✅ᵛ a64✅ᵛ |
| [`jq`](../ports/jq/) | 1.7.1 | `apk add jq` | oniguruma | oniguruma | `qnx-test-jq` | x86✅ᵛ a64✅ᵛ |
| [`json-glib`](../ports/json-glib/) | 1.10.6 | `apk add json-glib` | gettext glib | glib | `qnx-test-json-glib` | x86✅ᵛ a64✅ᵛ |
| [`labwc`](../ports/labwc/) | 0.20.2 | `apk add labwc` | cairo glib libpng libsfdo libxkbcommon libxml2 pango pixman wayland wlroots | wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`lcm`](../ports/lcm/) | 1.5.2 | `apk add lcm` | glib | glib | `qnx-test-lcm` | x86✅ᵛ a64✅ᵛ |
| [`lcms2`](../ports/lcms2/) | 2.16 | `apk add lcms2` | – | zlib | `qnx-test-lcms2` | x86✅ᵛ a64✅ᵛ |
| [`less`](../ports/less/) | 668 | `apk add less` | ncurses | ncurses | – | x86✅ᵛ a64✅ᵛ |
| [`leveldb`](../ports/leveldb/) | 1.23 | `apk add leveldb` | snappy | snappy | `qnx-test-leveldb` | x86✅ᵛ a64✅ᵛ |
| [`libarchive`](../ports/libarchive/) | 3.7.7 | `apk add libarchive` | bzip2 expat lz4 openssl xz zlib zstd | zlib bzip2 xz zstd openssl libxml2 expat lz4 | `qnx-test-libarchive` | x86✅ᵛ a64✅ᵛ |
| [`libass`](../ports/libass/) | 0.17.3 | `apk add libass` | fontconfig freetype fribidi harfbuzz | freetype fribidi harfbuzz fontconfig | `qnx-test-libass` | x86✅ᵛ a64✅ᵛ |
| [`libavif`](../ports/libavif/) | 1.1.1 | `apk add libavif` | aom dav1d | aom dav1d libyuv | `qnx-test-libavif` | x86✅ᵛ a64✅ᵛ |
| [`libcamera`](../ports/libcamera/) | 0.5.1+rpt20250722 | `apk add libcamera` | – | libyaml | – | a64✅ᵛ |
| [`libcoap`](../ports/libcoap/) | 4.3.5 | `apk add libcoap` | openssl | openssl | `qnx-test-libcoap` | x86✅ᵛ a64✅ᵛ |
| [`libdazzle`](../ports/libdazzle/) | 3.44.0 | `apk add libdazzle` | cairo gdk-pixbuf gettext glib gtk3 pango | glib gtk3 | – | x86✅ᵛ a64✅ᵛ |
| [`libedit`](../ports/libedit/) | 20240808-3.1 | `apk add libedit` | ncurses | ncurses | `qnx-test-libedit` | x86✅ᵛ a64✅ᵛ |
| [`libepoxy`](../ports/libepoxy/) | 1.5.10 | `apk add libepoxy` | – | libx11 | `qnx-test-libepoxy` | x86✅ᵛ a64✅ᵛ |
| [`libevent`](../ports/libevent/) | 2.1.12 | `apk add libevent` | openssl | openssl | `qnx-test-libevent` | x86✅ᵛ a64✅ᵛ |
| [`libgcrypt`](../ports/libgcrypt/) | 1.11.0 | `apk add libgcrypt` | libgpg-error | libgpg-error | `qnx-test-libgcrypt` | x86✅ᵛ a64✅ᵛ |
| [`libgit2`](../ports/libgit2/) | 1.8.4 | `apk add libgit2` | openssl pcre2 zlib | openssl zlib pcre2 | `qnx-test-libgit2` | x86✅ᵛ a64✅ᵛ |
| [`libgpg-error`](../ports/libgpg-error/) | 1.50 | `apk add libgpg-error` | gettext | gettext | – | x86✅ᵛ a64✅ᵛ |
| [`libgsf`](../ports/libgsf/) | 1.14.53 | `apk add libgsf` | bzip2 gettext glib libxml2 zlib | glib libxml2 zlib bzip2 | – | x86✅ᵛ a64✅ᵛ |
| [`libhandy`](../ports/libhandy/) | 1.8.3 | `apk add libhandy` | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gtk3 pango | glib gtk3 | `qnx-test-libhandy` | x86✅ᵛ a64✅ᵛ |
| [`libice`](../ports/libice/) | 1.1.2 | `apk add libice` | – | xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libidn2`](../ports/libidn2/) | 2.3.7 | `apk add libidn2` | gettext libunistring | libunistring gettext | `qnx-test-libidn2` | x86✅ᵛ a64✅ᵛ |
| [`libjxl`](../ports/libjxl/) | 0.11.1 | `apk add libjxl` | brotli highway lcms2 | brotli highway lcms2 libpng giflib libjpeg-turbo | `qnx-test-libjxl` | x86✅ᵛ a64✅ᵛ |
| [`libnotify`](../ports/libnotify/) | 0.8.3 | `apk add libnotify` | gdk-pixbuf glib | glib gdk-pixbuf | `qnx-test-libnotify` | x86✅ᵛ a64✅ᵛ |
| [`libpng`](../ports/libpng/) | 1.6.44 | `apk add libpng` | zlib | zlib | `qnx-test-libpng` | x86✅ᵛ a64✅ᵛ |
| [`libportal`](../ports/libportal/) | 0.8.1 | `apk add libportal` | glib gtk3 | glib gtk3 | – | x86✅ᵛ a64✅ᵛ |
| [`libproxy`](../ports/libproxy/) | 0.5.9 | `apk add libproxy` | curl glib | glib curl | – | x86✅ᵛ a64✅ᵛ |
| [`libpsl`](../ports/libpsl/) | 0.21.5 | `apk add libpsl` | libiconv libidn2 libunistring | libidn2 libunistring libiconv | `qnx-test-libpsl` | x86✅ᵛ a64✅ᵛ |
| [`libsecret`](../ports/libsecret/) | 0.21.4 | `apk add libsecret` | glib libgcrypt | glib libgcrypt | `qnx-test-libsecret` | x86✅ᵛ a64✅ᵛ |
| [`libsm`](../ports/libsm/) | 1.2.6 | `apk add libsm` | libice | libice xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libsndfile`](../ports/libsndfile/) | 1.2.2 | `apk add libsndfile` | flac libogg libvorbis opus | libogg libvorbis flac opus | `qnx-test-libsndfile` | x86✅ᵛ a64✅ᵛ |
| [`libsoup`](../ports/libsoup/) | 3.6.1 | `apk add libsoup` | brotli gettext glib libpsl nghttp2 sqlite zlib | glib libxml2 sqlite nghttp2 libpsl brotli | `qnx-test-libsoup` | x86✅ᵛ a64✅ᵛ |
| [`libssh`](../ports/libssh/) | 0.11.1 | `apk add libssh` | openssl zlib | openssl zlib | `qnx-test-libssh` | x86✅ᵛ a64✅ᵛ |
| [`libssh2`](../ports/libssh2/) | 1.11.1 | `apk add libssh2` | openssl zlib | openssl zlib | `qnx-test-libssh2` | x86✅ᵛ a64✅ᵛ |
| [`libthai`](../ports/libthai/) | 0.1.29 | `apk add libthai` | libdatrie | libdatrie | `qnx-test-libthai` | x86✅ᵛ a64✅ᵛ |
| [`libtheora`](../ports/libtheora/) | 1.1.1 | `apk add libtheora` | libogg | libogg libvorbis | – | x86✅ᵛ a64✅ᵛ |
| [`libtiff`](../ports/libtiff/) | 4.7.0 | `apk add libtiff` | libjpeg-turbo libwebp xz zlib zstd | zlib xz zstd libjpeg-turbo libwebp | `qnx-test-libtiff` | x86✅ᵛ a64✅ᵛ |
| [`libvncserver`](../ports/libvncserver/) | 0.9.15 | `apk add libvncserver` | gnutls libjpeg-turbo libpng lzo openssl sdl2 zlib | zlib lzo libjpeg-turbo libpng gnutls openssl sdl2 | `qnx-test-libvncserver` | x86✅ᵛ a64✅ᵛ |
| [`libvorbis`](../ports/libvorbis/) | 1.3.7 | `apk add libvorbis` | libogg | libogg | – | x86✅ᵛ a64✅ᵛ |
| [`libwebp`](../ports/libwebp/) | 1.4.0 | `apk add libwebp` | – | libpng libjpeg-turbo giflib | `qnx-test-libwebp` | x86✅ᵛ a64✅ᵛ |
| [`libwebsockets`](../ports/libwebsockets/) | 4.3.5 | `apk add libwebsockets` | openssl | openssl zlib | `qnx-test-libwebsockets` | x86✅ᵛ a64✅ᵛ |
| [`libx11`](../ports/libx11/) | 1.8.10 | `apk add libx11` | libxcb | libxcb xorgproto xtrans | – | x86✅ᵛ a64✅ᵛ |
| [`libxau`](../ports/libxau/) | 1.0.11 | `apk add libxau` | – | xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxcb`](../ports/libxcb/) | 1.17.0 | `apk add libxcb` | libxau libxdmcp | xcb-proto libxau libxdmcp xtrans pthread-stubs | – | x86✅ᵛ a64✅ᵛ |
| [`libxcomposite`](../ports/libxcomposite/) | 0.4.6 | `apk add libxcomposite` | libx11 | libx11 libxfixes xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxcursor`](../ports/libxcursor/) | 1.2.2 | `apk add libxcursor` | libx11 libxfixes libxrender | libx11 libxrender libxfixes xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxdamage`](../ports/libxdamage/) | 1.1.6 | `apk add libxdamage` | libx11 libxfixes | libx11 libxfixes xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxdmcp`](../ports/libxdmcp/) | 1.1.5 | `apk add libxdmcp` | – | xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxext`](../ports/libxext/) | 1.3.6 | `apk add libxext` | libx11 | libx11 xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxfce4ui`](../ports/libxfce4ui/) | 4.20.0 | `apk add libxfce4ui` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libxfce4util pango xfconf zlib | glib gtk3 libxfce4util xfconf libepoxy | – | x86✅ᵛ a64✅ᵛ |
| [`libxfce4util`](../ports/libxfce4util/) | 4.20.0 | `apk add libxfce4util` | gettext glib | glib | – | x86✅ᵛ a64✅ᵛ |
| [`libxfce4windowing`](../ports/libxfce4windowing/) | 4.20.0 | `apk add libxfce4windowing` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz pango wayland zlib | glib gtk3 wayland wayland-protocols | – | x86✅ᵛ a64✅ᵛ |
| [`libxfixes`](../ports/libxfixes/) | 6.0.1 | `apk add libxfixes` | libx11 | libx11 xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxfont2`](../ports/libxfont2/) | 2.0.7 | `apk add libxfont2` | fontenc freetype zlib | freetype zlib fontenc xorgproto xtrans | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxft`](../ports/libxft/) | 2.3.8 | `apk add libxft` | fontconfig freetype libx11 libxrender | libx11 libxrender freetype fontconfig xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxi`](../ports/libxi/) | 1.8.2 | `apk add libxi` | libx11 libxext | libx11 libxext libxfixes xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxinerama`](../ports/libxinerama/) | 1.1.5 | `apk add libxinerama` | libx11 libxext | libx11 libxext xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxkbcommon`](../ports/libxkbcommon/) | 1.8.1 | `apk add libxkbcommon` | libxml2 | libxml2 | – | x86✅ᵛ a64✅ᵛ |
| [`libxkbfile`](../ports/libxkbfile/) | 1.2.0 | `apk add libxkbfile` | libx11 | libx11 xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxml2`](../ports/libxml2/) | 2.13.5 | `apk add libxml2` | xz zlib | zlib xz | `qnx-test-libxml2` | x86✅ᵛ a64✅ᵛ |
| [`libxmu`](../ports/libxmu/) | 1.3.1 | `apk add libxmu` | libx11 libxext libxt | libx11 libxext libxt xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxrandr`](../ports/libxrandr/) | 1.5.4 | `apk add libxrandr` | libx11 libxext libxrender | libx11 libxext libxrender xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxrender`](../ports/libxrender/) | 0.9.11 | `apk add libxrender` | libx11 | libx11 xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxscrnsaver`](../ports/libxscrnsaver/) | 1.2.4 | `apk add libxscrnsaver` | libx11 libxext | libx11 libxext xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libxshmfence`](../ports/libxshmfence/) | 1.3.3 | `apk add libxshmfence` | – | xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxslt`](../ports/libxslt/) | 1.1.42 | `apk add libxslt` | libxml2 | libxml2 zlib | `qnx-test-libxslt` | x86✅ᵛ a64✅ᵛ |
| [`libxt`](../ports/libxt/) | 1.3.1 | `apk add libxt` | libice libsm libx11 | libx11 libsm libice xorgproto | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`libxtst`](../ports/libxtst/) | 1.2.5 | `apk add libxtst` | libx11 libxext libxi | libx11 libxext libxfixes libxi xorgproto | – | x86✅ᵛ a64✅ᵛ |
| [`libyuv`](../ports/libyuv/) | stable | `apk add libyuv` | libjpeg-turbo | libjpeg-turbo | `qnx-test-libyuv` | x86✅ᵛ a64✅ᵛ |
| [`libzip`](../ports/libzip/) | 1.11.1 | `apk add libzip` | bzip2 openssl xz zlib zstd | zlib bzip2 xz zstd openssl | `qnx-test-libzip` | x86✅ᵛ a64✅ᵛ |
| [`libzmq`](../ports/libzmq/) | 4.3.5 | `apk add libzmq` | libsodium | libsodium | `qnx-test-libzmq` | x86✅ᵛ a64✅ᵛ |
| [`lighttpd`](../ports/lighttpd/) | 1.4.77 | `apk add lighttpd` | openssl pcre2 zlib | openssl pcre2 zlib | – | x86✅ᵛ a64✅ᵛ |
| [`llvm`](../ports/llvm/) | 22.1.8 | `apk add llvm` | libxml2 zlib zstd | zlib zstd libffi libxml2 | – | x86✅ᵛ a64✅ᵛ |
| [`lua-lpeg`](../ports/lua-lpeg/) | 1.1.0 | `apk add lua-lpeg` | – | luajit | – | x86✅ᵛ a64✅ᵛ |
| [`luv`](../ports/luv/) | 1.51.0-1 | `apk add luv` | libuv | libuv luajit | – | x86✅ᵛ a64✅ᵛ |
| [`m4`](../ports/m4/) | 1.4.19 | `apk add m4` | libiconv | libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`mame`](../ports/mame/) | 0.281 | `apk add mame` | fontconfig freetype sdl2 sdl2-ttf zlib | sdl2 sdl2-ttf fontconfig freetype zlib | – | x86✅ᵛ a64✅ᵛ |
| [`man-db`](../ports/man-db/) | 2.13.1 | `apk add man-db` | gdbm libiconv libpipeline zlib | libpipeline gdbm groff libiconv zlib | – | x86✅ᵛ a64✅ᵛ |
| [`mate-calc`](../ports/mate-calc/) | 1.28.0 | `apk add mate-calc` | at-spi2-core cairo gdk-pixbuf gettext glib gmp gtk3 harfbuzz libxml2 mpc mpfr pango zlib | glib gtk3 libxml2 gmp mpfr mpc | – | x86✅ᵛ a64✅ᵛ |
| [`mcap-cpp`](../ports/mcap-cpp/) | 2.1.3 | `apk add mcap-cpp` | – | lz4 zstd | `qnx-test-mcap` | x86✅ᵛ a64✅ᵛ |
| [`meson`](../ports/meson/) | 1.10.0 | `apk add meson` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`metis`](../ports/metis/) | 5.2.1 | `apk add metis` | gklib | gklib | – | x86✅ᵛ a64✅ᵛ |
| [`minicom`](../ports/minicom/) | 2.9 | `apk add minicom` | libiconv ncurses | ncurses libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`mkfontscale`](../ports/mkfontscale/) | 1.2.3 | `apk add mkfontscale` | fontenc freetype zlib | freetype fontenc zlib | – | x86✅ᵛ a64✅ᵛ |
| [`mosquitto`](../ports/mosquitto/) | 2.0.20 | `apk add mosquitto` | cjson openssl | openssl cjson | `qnx-test-mosquitto` | x86✅ᵛ a64✅ᵛ |
| [`mpc`](../ports/mpc/) | 1.3.1 | `apk add mpc` | gmp mpfr | gmp mpfr | `qnx-test-mpc` | x86✅ᵛ a64✅ᵛ |
| [`mpfr`](../ports/mpfr/) | 4.2.1 | `apk add mpfr` | gmp | gmp | `qnx-test-mpfr` | x86✅ᵛ a64✅ᵛ |
| [`nano`](../ports/nano/) | 8.2 | `apk add nano` | file ncurses zlib | ncurses zlib file | – | x86✅ᵛ a64✅ᵛ |
| [`neovim`](../ports/neovim/) | 0.12.3 | `apk add neovim` | gettext libiconv luv tree-sitter unibilium utf8proc | luajit libuv utf8proc tree-sitter unibilium lua-lpeg luv libiconv gettext | – | x86✅ᵛ a64✅ᵛ |
| [`nettle`](../ports/nettle/) | 3.10.1 | `apk add nettle` | gmp | gmp | `qnx-test-nettle` | x86✅ᵛ a64✅ᵛ |
| [`nghttp2`](../ports/nghttp2/) | 1.64.0 | `apk add nghttp2` | – | zlib openssl | `qnx-test-nghttp2` | x86✅ᵛ a64✅ᵛ |
| [`nodejs`](../ports/nodejs/) | 24.14.1 | `apk add nodejs` | ada brotli c-ares icu libuv nghttp2 openssl simdjson simdutf sqlite zlib zstd | libuv ada brotli zlib openssl c-ares nghttp2 simdjson simdutf sqlite zstd icu | – | x86✅ᵛ a64✅ᵛ |
| [`npm`](../ports/npm/) | 11.11.0 | `apk add npm` | – | nodejs | – | x86✅ᵛ a64✅ᵛ |
| [`ntpsec`](../ports/ntpsec/) | 1.2.4 | `apk add ntpsec` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`numpy`](../ports/numpy/) | 2.3.4 | `apk add numpy` | openblas python3 | python3 openblas | – | x86✅ᵛ a64✅ᵛ |
| [`onnx`](../ports/onnx/) | 1.17.0 | `apk add onnx` | abseil protobuf | protobuf | – | x86✅ᵛ a64✅ᵛ |
| [`onnxruntime`](../ports/onnxruntime/) | 1.20.1 | `apk add onnxruntime` | zlib | protobuf | `qnx-test-onnxruntime` | x86✅ᵛ a64✅ᵛ |
| [`open-amp`](../ports/open-amp/) | 2024.10.0 | `apk add open-amp` | – | libmetal | – | x86✅ᵛ a64✅ᵛ |
| [`open62541`](../ports/open62541/) | 1.4.8 | `apk add open62541` | openssl | openssl | `qnx-test-open62541` | x86✅ᵛ a64✅ᵛ |
| [`opencv`](../ports/opencv/) | 4.10.0 | `apk add opencv` | zlib | zlib eigen | `qnx-test-opencv` | x86✅ᵛ a64✅ᵛ |
| [`openjpeg`](../ports/openjpeg/) | 2.5.3 | `apk add openjpeg` | – | zlib libpng libtiff lcms2 | `qnx-test-openjpeg` | x86✅ᵛ a64✅ᵛ |
| [`openssh`](../ports/openssh/) | 10.2p1 | `apk add openssh` | libxcrypt openssl zlib | openssl zlib libxcrypt | – | x86✅ᵛ a64✅ᵛ |
| [`openttd`](../ports/openttd/) | 15.0-git-20250501 | `apk add openttd` | curl freetype libpng lzo sdl2 xz zlib | sdl2 zlib xz libpng freetype fontconfig curl lzo | – | x86✅ᵛ a64✅ᵛ |
| [`opusfile`](../ports/opusfile/) | 0.12 | `apk add opusfile` | libogg openssl opus | opus libogg openssl | `qnx-test-opusfile` | x86✅ᵛ a64✅ᵛ |
| [`orocos-kdl`](../ports/orocos-kdl/) | 1.5.1 | `apk add orocos-kdl` | – | eigen | `qnx-test-orocos-kdl` | x86✅ᵛ a64✅ᵛ |
| [`oss-desktop`](../ports/oss-desktop/) | 1.0 | `apk add oss-desktop` | – | labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`p11-kit`](../ports/p11-kit/) | 0.25.5 | `apk add p11-kit` | libffi | libffi libtasn1 | `qnx-test-p11kit` | x86✅ᵛ a64✅ᵛ |
| [`paho.mqtt.c`](../ports/paho.mqtt.c/) | 1.3.14 | `apk add paho.mqtt.c` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`paho.mqtt.cpp`](../ports/paho.mqtt.cpp/) | 1.5.2 | `apk add paho.mqtt.cpp` | openssl paho.mqtt.c | paho.mqtt.c | – | x86✅ᵛ a64✅ᵛ |
| [`pandas`](../ports/pandas/) | 2.3.3 | `apk add pandas` | python3 | numpy python3-dateutil python3-pytz python3-tzdata | – | x86✅ᵛ a64✅ᵛ |
| [`pango`](../ports/pango/) | 1.54.0 | `apk add pango` | cairo fontconfig freetype fribidi glib harfbuzz libx11 libxft libxrender | glib harfbuzz fribidi cairo fontconfig freetype libxft | – | x86✅ᵛ a64✅ᵛ |
| [`pcl`](../ports/pcl/) | 1.14.1 | `apk add pcl` | boost flann lz4 | boost eigen flann | `qnx-test-pcl` | x86✅ᵛ a64✅ᵛ |
| [`pcre2`](../ports/pcre2/) | 10.44 | `apk add pcre2` | – | zlib bzip2 | `qnx-test-pcre2` | x86✅ᵛ a64✅ᵛ |
| [`perl`](../ports/perl/) | 5.40.2 | `apk add perl` | libxcrypt | libxcrypt | – | x86✅ᵛ a64✅ᵛ |
| [`perl-error`](../ports/perl-error/) | 0.17030 | `apk add perl-error` | – | perl | – | x86✅ᵛ a64✅ᵛ |
| [`perl-mozilla-ca`](../ports/perl-mozilla-ca/) | 20261020 | `apk add perl-mozilla-ca` | – | perl | `qnx-test-perl-mozilla-ca` | x86✅ᵛ a64✅ᵛ |
| [`pistache`](../ports/pistache/) | 0.4.26 | `apk add pistache` | libevent openssl | libevent rapidjson openssl | `qnx-test-pistache` | x86✅ᵛ a64✅ᵛ |
| [`pixman`](../ports/pixman/) | 0.46.4 | `apk add pixman` | – | libpng | `qnx-test-pixman` | x86✅ᵛ a64✅ᵛ |
| [`poppler`](../ports/poppler/) | 25.12.0 | `apk add poppler` | cairo fontconfig freetype gettext glib lcms2 libiconv libjpeg-turbo libpng libtiff openjpeg zlib | freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib | `qnx-test-poppler` | x86✅ᵛ a64✅ᵛ |
| [`pqiv`](../ports/pqiv/) | 2.13.2 | `apk add pqiv` | at-spi2-core cairo gdk-pixbuf gettext glib gtk3 harfbuzz libx11 pango zlib | glib gtk3 gdk-pixbuf cairo | – | x86✅ᵛ a64✅ᵛ |
| [`protobuf`](../ports/protobuf/) | 29.3 | `apk add protobuf` | abseil zlib | zlib abseil | `qnx-test-protobuf` | x86✅ᵛ a64✅ᵛ |
| [`pthreadpool`](../ports/pthreadpool/) | 0-git-20250207 | `apk add pthreadpool` | – | fxdiv | – | x86✅ᵛ a64✅ᵛ |
| [`python3`](../ports/python3/) | 3.14.0 | `apk add python3` | bzip2 expat gdbm gettext libffi ncurses openssl sqlite xz zlib zstd | zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd | `qnx-test-python` | x86✅ᵛ a64✅ᵛ |
| [`python3-astor`](../ports/python3-astor/) | 0.8.1 | `apk add python3-astor` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-async-generator`](../ports/python3-async-generator/) | 1.10 | `apk add python3-async-generator` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-click`](../ports/python3-click/) | 8.5.0 | `apk add python3-click` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-coverage`](../ports/python3-coverage/) | 7.17.0 | `apk add python3-coverage` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-curio`](../ports/python3-curio/) | 1.6_git20251221 | `apk add python3-curio` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-dateutil`](../ports/python3-dateutil/) | 2.9.0.post0 | `apk add python3-dateutil` | – | python3 python3-six | – | x86✅ᵛ a64✅ᵛ |
| [`python3-idna`](../ports/python3-idna/) | 3.20 | `apk add python3-idna` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-iniconfig`](../ports/python3-iniconfig/) | 2.3.1 | `apk add python3-iniconfig` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-mypy-extensions`](../ports/python3-mypy-extensions/) | 1.1.0 | `apk add python3-mypy-extensions` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-packaging`](../ports/python3-packaging/) | 25.0 | `apk add python3-packaging` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-pip`](../ports/python3-pip/) | 25.1.1 | `apk add python3-pip` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-pluggy`](../ports/python3-pluggy/) | 1.7.0 | `apk add python3-pluggy` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pretend`](../ports/python3-pretend/) | 1.0.9 | `apk add python3-pretend` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pygments`](../ports/python3-pygments/) | 2.21.0 | `apk add python3-pygments` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest`](../ports/python3-pytest/) | 9.2.0 | `apk add python3-pytest` | – | python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-cov`](../ports/python3-pytest-cov/) | 7.1.0 | `apk add python3-pytest-cov` | – | python3 python3-pytest python3-coverage python3-pluggy | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytest-tornasync`](../ports/python3-pytest-tornasync/) | 0.6.0.post2 | `apk add python3-pytest-tornasync` | – | python3 python3-pytest python3-tornado | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytokens`](../ports/python3-pytokens/) | 0.4.1 | `apk add python3-pytokens` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-pytz`](../ports/python3-pytz/) | 2025.2 | `apk add python3-pytz` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-setuptools`](../ports/python3-setuptools/) | 80.8.0 | `apk add python3-setuptools` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-six`](../ports/python3-six/) | 1.17.0 | `apk add python3-six` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-sniffio`](../ports/python3-sniffio/) | 1.3.1 | `apk add python3-sniffio` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-sortedcontainers`](../ports/python3-sortedcontainers/) | 2.4.0 | `apk add python3-sortedcontainers` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-tornado`](../ports/python3-tornado/) | 6.6 | `apk add python3-tornado` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-traitlets`](../ports/python3-traitlets/) | 5.16.1 | `apk add python3-traitlets` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-tzdata`](../ports/python3-tzdata/) | 2025.2 | `apk add python3-tzdata` | – | python3 | – | x86✅ᵛ a64✅ᵛ |
| [`python3-versioneer`](../ports/python3-versioneer/) | 0.29 | `apk add python3-versioneer` | – | python3 | `qnx-test-pymods` | x86✅ᵛ a64✅ᵛ |
| [`python3-wheel`](../ports/python3-wheel/) | 0.46.1 | `apk add python3-wheel` | – | python3 python3-packaging | – | x86✅ᵛ a64✅ᵛ |
| [`pytorch`](../ports/pytorch/) | 2.3.1 | `apk add pytorch` | abseil openblas protobuf | python3 protobuf openblas | `qnx-test-torch` | x86✅ᵛ a64✅ᵛ |
| [`qt6`](../ports/qt6/) | 6.8.1 | `apk add qt6` | brotli double-conversion ffmpeg fontconfig freetype glib gst-plugins-bad gst-plugins-base gstreamer harfbuzz icu libjpeg-turbo libpng libtiff libwebp openssl pcre2 zlib zstd | zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd | `qnx-test-qt6` | x86✅ᵛ a64✅ᵛ |
| [`re2`](../ports/re2/) | 2024-07-02 | `apk add re2` | abseil | abseil | `qnx-test-re2` | x86✅ᵛ a64✅ᵛ |
| [`readline`](../ports/readline/) | 8.2 | `apk add readline` | – | ncurses | `qnx-test-readline` | x86✅ᵛ a64✅ᵛ |
| [`restbed`](../ports/restbed/) | 4.8 | `apk add restbed` | openssl | openssl asio | – | x86✅ᵛ a64✅ᵛ |
| [`retroarch`](../ports/retroarch/) | 1.15-git-20230408 | `apk add retroarch` | fontconfig freetype fribidi libass libxkbcommon openssl zlib | sdl2 freetype zlib fontconfig fribidi libass openssl | – | x86✅ᵛ a64✅ᵛ |
| [`retropie`](../ports/retropie/) | 0.3 | `apk add retropie` | – | retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08 | – | x86✅ᵛ a64✅ᵛ |
| [`ros2`](../ports/ros2/) | jazzy | `apk add ros2` | benchmark console_bridge curl cyclonedds fastcdr fastdds libyaml lz4 opencv python3 sqlite tinyxml2 yaml-cpp zstd | python3 numpy opencv curl lz4 fastcdr fastdds cyclonedds console_bridge libyaml yaml-cpp zstd benchmark tinyxml2 | `qnx-test-ros2` | x86✅ᵛ a64✅ᵛ |
| [`rsync`](../ports/rsync/) | 3.3.0 | `apk add rsync` | lz4 openssl xxhash zstd | zstd lz4 xxhash openssl | – | x86✅ᵛ a64✅ᵛ |
| [`rtl-sdr`](../ports/rtl-sdr/) | 2.0.2 | `apk add rtl-sdr` | libusb | libusb | – | x86✅ᵛ a64✅ᵛ |
| [`ruy`](../ports/ruy/) | 0-git-20240321 | `apk add ruy` | – | cpuinfo | – | x86✅ᵛ a64✅ᵛ |
| [`s2n-tls`](../ports/s2n-tls/) | 1.7.0 | `apk add s2n-tls` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`sdl2`](../ports/sdl2/) | 2.32.10 | `apk add sdl2` | – | wayland libxkbcommon | `qnx-test-sdl2` | x86✅ᵛ a64✅ᵛ |
| [`sdl2-compat`](../ports/sdl2-compat/) | 2.32.60 | `apk add sdl2-compat` | – | sdl3 | – | x86✅ᵛ a64✅ᵛ |
| [`sdl2-image`](../ports/sdl2-image/) | 2.8.8 | `apk add sdl2-image` | libavif libtiff libwebp sdl2 | sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl | – | x86✅ᵛ a64✅ᵛ |
| [`sdl2-net`](../ports/sdl2-net/) | 2.2.0 | `apk add sdl2-net` | sdl2 | sdl2 | – | x86✅ᵛ a64✅ᵛ |
| [`sdl2-ttf`](../ports/sdl2-ttf/) | 2.24.0 | `apk add sdl2-ttf` | freetype harfbuzz sdl2 | sdl2 freetype harfbuzz | – | x86✅ᵛ a64✅ᵛ |
| [`sdl3`](../ports/sdl3/) | 3.4.18 | `apk add sdl3` | libiconv libx11 libxcursor libxext libxfixes libxi libxrandr libxrender libxscrnsaver libxtst | libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst | – | x86✅ᵛ a64✅ᵛ |
| [`shared-mime-info`](../ports/shared-mime-info/) | 2.4 | `apk add shared-mime-info` | glib libxml2 | glib libxml2 | – | x86✅ᵛ a64✅ᵛ |
| [`spdlog`](../ports/spdlog/) | 1.15.3 | `apk add spdlog` | fmt | fmt | `qnx-test-spdlog` | x86✅ᵛ a64✅ᵛ |
| [`sqlite`](../ports/sqlite/) | 3.47.2 | `apk add sqlite` | zlib | zlib | `qnx-test-sqlite` | x86✅ᵛ a64✅ᵛ |
| [`sqlite_orm`](../ports/sqlite_orm/) | 1.9 | `apk add sqlite_orm` | – | sqlite | – | x86✅ᵛ a64✅ᵛ |
| [`sscep`](../ports/sscep/) | 0.10.0 | `apk add sscep` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`strongswan`](../ports/strongswan/) | 6.0.6 | `apk add strongswan` | openssl | openssl | – | x86✅ᵛ a64✅ᵛ |
| [`sudo`](../ports/sudo/) | 1.9.17p2 | `apk add sudo` | libxcrypt openssl zlib | libxcrypt openssl zlib | – | x86✅ᵛ a64✅ᵛ |
| [`suitesparse`](../ports/suitesparse/) | 7.11.0 | `apk add suitesparse` | openblas | openblas | – | x86✅ᵛ a64✅ᵛ |
| [`swaybg`](../ports/swaybg/) | 1.2.2 | `apk add swaybg` | cairo gdk-pixbuf glib wayland | wayland wayland-protocols cairo gdk-pixbuf glib | `qnx-test-labwc` | x86✅ᵛ a64✅ᵛ |
| [`sysbench`](../ports/sysbench/) | 1.0.20-git-20250309 | `apk add sysbench` | luajit | luajit | – | x86✅ᵛ a64✅ᵛ |
| [`taglib`](../ports/taglib/) | 2.0.2 | `apk add taglib` | zlib | zlib | `qnx-test-taglib` | x86✅ᵛ a64✅ᵛ |
| [`tar`](../ports/tar/) | 1.35 | `apk add tar` | libiconv | libiconv | – | x86✅ᵛ a64✅ᵛ |
| [`tftp-hpa`](../ports/tftp-hpa/) | 5.2 | `apk add tftp-hpa` | ncurses readline | readline ncurses | – | x86✅ᵛ a64✅ᵛ |
| [`thorvg`](../ports/thorvg/) | 0.15.8 | `apk add thorvg` | libjpeg-turbo libpng | libjpeg-turbo libpng zlib | `qnx-test-thorvg` | x86✅ᵛ a64✅ᵛ |
| [`thunar`](../ports/thunar/) | 4.20.0 | `apk add thunar` | at-spi2-core cairo exo gdk-pixbuf gettext glib gtk3 harfbuzz libexif libnotify libx11 libxfce4ui libxfce4util pango pcre2 xfce4-panel xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel | – | x86✅ᵛ a64✅ᵛ |
| [`tmux`](../ports/tmux/) | 3.5a | `apk add tmux` | libevent ncurses | libevent ncurses | – | x86✅ᵛ a64✅ᵛ |
| [`tumbler`](../ports/tumbler/) | 4.20.0 | `apk add tumbler` | curl ffmpegthumbnailer freetype gdk-pixbuf gettext glib gst-plugins-base gstreamer libgsf libjpeg-turbo libpng libxfce4util libxml2 | glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer | – | x86✅ᵛ a64✅ᵛ |
| [`unbound`](../ports/unbound/) | 1.24.1 | `apk add unbound` | expat openssl | openssl expat | – | x86✅ᵛ a64✅ᵛ |
| [`vim`](../ports/vim/) | 9.1.0900 | `apk add vim` | libiconv libsodium ncurses | ncurses libiconv libsodium | – | x86✅ᵛ a64✅ᵛ |
| [`virtualgl`](../ports/virtualgl/) | 3.1.5 | `apk add virtualgl` | libjpeg-turbo libx11 libxext | libx11 libxext libjpeg-turbo | `qnx-test-virtualgl` | x86✅ᵛ a64✅ᵛ |
| [`vlc`](../ports/vlc/) | 3.0.21 | `apk add vlc` | bzip2 libiconv lz4 openssl xz zstd | bzip2 libiconv lz4 openssl xz zstd | – | x86✅ᵛ a64✅ᵛ |
| [`vsomeip`](../ports/vsomeip/) | 3.5.6 | `apk add vsomeip` | boost | boost | `qnx-test-vsomeip` | x86✅ᵛ a64✅ᵛ |
| [`vte`](../ports/vte/) | 0.78.2 | `apk add vte` | at-spi2-core cairo fribidi gdk-pixbuf gettext glib gnutls graphene gtk3 gtk4 icu lz4 pango pcre2 | gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4 | `qnx-test-vte` | x86✅ᵛ a64✅ᵛ |
| [`wayland`](../ports/wayland/) | 1.24.0 | `apk add wayland` | libffi | libffi expat libxml2 | `qnx-test-wayland` | x86❌ᵛ a64❌ᵛ |
| [`wayland-protocols`](../ports/wayland-protocols/) | 1.47 | `apk add wayland-protocols` | – | wayland | – | x86✅ᵛ a64✅ᵛ |
| [`webkitgtk`](../ports/webkitgtk/) | 2.44.4 | `apk add webkitgtk` | at-spi2-core cairo fontconfig freetype gdk-pixbuf gettext glib gst-plugins-bad gst-plugins-base gstreamer gtk3 harfbuzz hyphen icu lcms2 libavif libepoxy libgcrypt libgpg-error libjpeg-turbo libjxl libpng libsecret libsoup libtasn1 libwebp libxml2 libxslt pango sqlite woff2 zlib | gtk3 glib cairo fontconfig freetype libgcrypt libtasn1 harfbuzz icu libjpeg-turbo libepoxy libxml2 libpng sqlite zlib libwebp libxslt libsoup gstreamer gst-plugins-base hyphen woff2 libx11 wayland wayland-protocols libxkbcommon at-spi2-core gst-plugins-bad lcms2 libavif libjxl libsecret | – | x86✅ᵛ a64✅ᵛ |
| [`wget`](../ports/wget/) | 1.25.0 | `apk add wget` | gnutls libiconv libpsl libunistring nettle pcre2 zlib | gnutls libidn2 zlib pcre2 libpsl | `qnx-test-cli-tools` | x86✅ᵛ a64✅ᵛ |
| [`wlroots`](../ports/wlroots/) | 0.20.2 | `apk add wlroots` | lcms2 libxkbcommon pixman wayland | wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes | `qnx-test-wlroots` | x86✅ᵛ a64✅ᵛ |
| [`woff2`](../ports/woff2/) | 1.0.2 | `apk add woff2` | brotli | brotli | `qnx-test-woff2` | x86✅ᵛ a64✅ᵛ |
| [`xcb-util`](../ports/xcb-util/) | 0.4.1 | `apk add xcb-util` | libxcb | libxcb | `qnx-test-xorg-libs` | x86✅ᵛ a64✅ᵛ |
| [`xfce4-appfinder`](../ports/xfce4-appfinder/) | 4.20.0 | `apk add xfce4-appfinder` | cairo garcon gdk-pixbuf glib gtk3 libxfce4ui libxfce4util xfconf | glib gtk3 libxfce4util libxfce4ui xfconf garcon | – | x86✅ᵛ a64✅ᵛ |
| [`xfce4-panel`](../ports/xfce4-panel/) | 4.20.0 | `apk add xfce4-panel` | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util libxfce4windowing pango wayland xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols | – | x86✅ᵛ a64✅ᵛ |
| [`xfce4-session`](../ports/xfce4-session/) | 4.20.2 | `apk add xfce4-session` | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util libxfce4windowing pango xfconf | glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell | – | x86✅ᵛ a64✅ᵛ |
| [`xfce4-settings`](../ports/xfce4-settings/) | 4.20.1 | `apk add xfce4-settings` | at-spi2-core cairo exo garcon gdk-pixbuf gettext glib gtk-layer-shell gtk3 harfbuzz libxfce4ui libxfce4util pango wayland xfconf zlib | glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell | – | x86✅ᵛ a64✅ᵛ |
| [`xfce4-terminal`](../ports/xfce4-terminal/) | 1.1.5 | `apk add xfce4-terminal` | at-spi2-core cairo gdk-pixbuf glib gtk-layer-shell gtk3 libxfce4ui libxfce4util pango pcre2 vte xfconf | glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell | – | x86✅ᵛ a64✅ᵛ |
| [`xfconf`](../ports/xfconf/) | 4.20.0 | `apk add xfconf` | gettext glib libxfce4util | glib libxfce4util dbus | – | x86✅ᵛ a64✅ᵛ |
| [`xkbcomp`](../ports/xkbcomp/) | 1.5.0 | `apk add xkbcomp` | libx11 libxkbfile | libx11 libxkbfile xorgproto | `qnx-test-xkbcomp` | x86✅ᵛ a64✅ᵛ |
| [`xnnpack`](../ports/xnnpack/) | 0-git-20250209 | `apk add xnnpack` | pthreadpool | cpuinfo pthreadpool fxdiv | – | x86✅ᵛ a64✅ᵛ |
| [`xwayland`](../ports/xwayland/) | 24.1.14 | `apk add xwayland` | libmd libxau libxcvt libxfont2 pixman wayland | wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util | `qnx-test-virtualgl` | x86✅ᵛ a64✅ᵛ |
| [`xz`](../ports/xz/) | 5.6.3 | `apk add xz` | gettext | gettext | `qnx-test-xz` | x86✅ᵛ a64✅ᵛ |
| [`zenoh-c`](../ports/zenoh-c/) | 1.2.1 | `apk add zenoh-c` | – | rust | `qnx-test-zenoh-c` | x86✅ᵛ a64✅ᵛ |
| [`zsh`](../ports/zsh/) | 5.9 | `apk add zsh` | libiconv ncurses | ncurses pcre2 libiconv | – | x86✅ᵛ a64✅ᵛ |

---
<sub>Generated by <code>scripts/gen-target-guide</code>.</sub>
