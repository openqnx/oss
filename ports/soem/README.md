# soem 2.0.1

> Simple Open EtherCAT Master library and tools

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/OpenEtherCATsociety/SOEM.git (tag `v2.0.1`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`libpcap`](../libpcap/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_soem`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `soem` (programs, shared libraries, run-time data) and `soem-dev` (headers, pkg-config/CMake files, static libraries; needs `soem`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`soem`](https://openqnx.github.io/oss/package/x86_64/soem.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/soem.html)) · [`soem-dev`](https://openqnx.github.io/oss/package/x86_64/soem-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/soem-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add soem` (dependencies come along); one version: `apk add soem=<version>` | `apk del soem` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install soem` (dependencies first) | `oss-get remove soem` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`soem-dev` the same way, by its own name (`apk add soem-dev`, `oss-get install soem-dev`, by hand with its own file and list `soem-dev.list`); remove it before `soem`, which it needs.

```sh
# on the QNX target, as root
apk add soem soem-dev
apk add qnx-test-soem && /usr/local/bin/qnx-port-tests/test_soem    # its on-target test, from tests-8.0-<arch>-m-z
apk del soem soem-dev qnx-test-soem
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/soem.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/soem.list
rm var/lib/manual/soem.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild soem` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libpcap
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.0.1 --recurse-submodules --shallow-submodules \
    https://github.com/OpenEtherCATsociety/SOEM.git src/soem
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/soem < ports/soem/patches/0001-qnx-os-layer.patch
patch -p1 -d src/soem < ports/soem/patches/0002-install-dirs.patch
```

**[`0001-qnx-os-layer.patch`](patches/0001-qnx-os-layer.patch)** — Add a QNX OS layer: POSIX osal, NIC driver on BPF through libpcap.

> SOEM picks its OS layer from cmake/${CMAKE_SYSTEM_NAME}.cmake, and there is
> none for QNX. This adds one: the osal of the Linux port, which is plain
> POSIX (pthreads, clock_gettime/clock_nanosleep on CLOCK_MONOTONIC) and is
> used unchanged, and oshw/qnx, derived from oshw/linux.
>
> The Linux NIC driver is built on an AF_PACKET raw socket bound to the
> EtherCAT ethertype; QNX has no AF_PACKET. QNX's raw link-layer access is
> BPF, used here through libpcap, as SOEM's Windows driver uses WinPcap/Npcap
> (and its former macOS port used libpcap): one pcap handle per NIC,
> promiscuous, immediate mode, non-blocking, with an "ether proto 0x88a4"
> filter in place of the bind(); frames go out with pcap_inject() and come in
> with pcap_next_ex(), and the poll in ecx_waitinframe_red() waits on
> pcap_get_selectable_fd(). The frame index, buffer and redundancy logic is
> the Linux driver's, unchanged. ecx_setupnic() fails (returns 0) when the NIC
> cannot be opened, as on Linux.
>
> oshw.c (if_nameindex adapter list, htons/ntohs) is the Linux one verbatim.

Touches: `cmake/QNX.cmake`, `oshw/qnx/nicdrv.c`, `oshw/qnx/nicdrv.h`, `oshw/qnx/oshw.c`, `oshw/qnx/oshw.h` · Upstream status: QNX-specific

**[`0002-install-dirs.patch`](patches/0002-install-dirs.patch)** — Install the CMake package, script and documents in standard places.

> The CMake package files went to <prefix>/cmake, eniconv.py to
> <prefix>/scripts and README.md/LICENSE.md to the prefix itself, i.e.
> /usr/local/README.md on a target. They go to lib/cmake/soem (where
> find_package(soem) looks), share/soem/scripts and share/doc/soem;
> AddENI.cmake also looks for eniconv.py in the new place.

Touches: `CMakeLists.txt`, `cmake/AddENI.cmake` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/soem -B build/$QNX_ARCH/soem -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=OFF \
      -DSOEM_BUILD_SAMPLES=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Python3=ON
cmake --build build/$QNX_ARCH/soem -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/soem
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- SOEM: Simple Open EtherCAT Master -- the EtherCAT master stack (slave enumeration and configuration, process data, CoE/SoE/FoE/EoE mailboxes, distributed clocks) and its sample tools (slaveinfo, eepromtool, firm_update, simple_ng, ec_sample).
- SOEM's OS layer comes per CMAKE_SYSTEM_NAME (cmake/<system>.cmake): Linux, Windows and rt-kernel. QNX gets its own (patch 0001): the POSIX osal of the Linux port unchanged, and an oshw whose NIC driver sends and receives the raw EtherCAT frames (ethertype 0x88a4) through BPF, by way of libpcap -- QNX has no AF_PACKET sockets, which the Linux driver is built on. That is the approach SOEM's Windows port (WinPcap/Npcap) and its former macOS port took. Opening a NIC needs root (BPF), as raw sockets do on Linux.
- A static library, as upstream builds it by default: the libpcap port installs only a static, non-PIC libpcap.a, which cannot go into a shared libsoem.so. eni_test runs the build machine's python on an ENI XML file at build time; it is a test program, not a tool, and is left out with eoe_test (Linux only upstream: it needs a TAP device).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH soem`.

### 5. What it installs

27 files under `/usr/local`, 5 programs, 15 headers.

```
/usr/local/bin/ec_sample
/usr/local/bin/eepromtool
/usr/local/bin/firm_update
/usr/local/bin/simple_ng
/usr/local/bin/slaveinfo
```

The complete list is `stage/<arch>/.manifest/soem` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa soem
ls dists/8.0/*/ | grep '^soem-'
```

This writes `soem-<ver>-r0.tar.gz` (run time) and `soem-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add soem
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/soem-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/soem-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_soem`](../../tests/src/) links soem the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-soem`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-soem
/usr/local/bin/qnx-port-tests/test_soem
```

```
== soem ==
  ok   ...
PASS soem
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_soem` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_soem`](../../tests/src/) -- a program linked against soem the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ecx_init on a nonexistent interface fails
   - ecx_close after the failed init
   - ec_find_adapters lists the interfaces, loopback among them
   - slaveinfo on a nonexistent interface: No socket connection
   - slaveinfo without arguments lists the adapters
   - osal_usleep(20 ms) returns 0
   - monotonic clock: at least 20 ms elapsed
   - time difference normalised
   - osal_time_diff borrows across seconds (1.2 s)
   - a 50 ms timer has not expired at once
   - the timer has expired after 70 ms
   - osal_monotonic_sleep to an absolute time
   - woke at or after the requested time
   - osal_current_time is wall-clock time
   - osal_thread_create (two threads)
   - osal mutex: 2 x 10000 increments, none lost
   - osal_malloc
   - oshw_htons/oshw_ntohs
   - ec_setupheader: ethertype 0x88A4
   - ec_setupheader: broadcast destination
   - AL status code 0x0011 text
   - SDO abort code 0x06020000 text

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | soem 2.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/soem-2.0.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | soem 2.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/soem-2.0.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add soem=<version>`), sources in `ported/soem/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates soem`; port one with `scripts/port-update soem`.

<details><summary>recipe.sh</summary>

```bash
# SOEM: Simple Open EtherCAT Master -- the EtherCAT master stack (slave
# enumeration and configuration, process data, CoE/SoE/FoE/EoE mailboxes,
# distributed clocks) and its sample tools (slaveinfo, eepromtool,
# firm_update, simple_ng, ec_sample).
VERSION=2.0.1; GIT=https://github.com/OpenEtherCATsociety/SOEM.git; REF=v2.0.1
# SOEM's OS layer comes per CMAKE_SYSTEM_NAME (cmake/<system>.cmake): Linux,
# Windows and rt-kernel. QNX gets its own (patch 0001): the POSIX osal of the
# Linux port unchanged, and an oshw whose NIC driver sends and receives the
# raw EtherCAT frames (ethertype 0x88a4) through BPF, by way of libpcap --
# QNX has no AF_PACKET sockets, which the Linux driver is built on. That is
# the approach SOEM's Windows port (WinPcap/Npcap) and its former macOS port
# took. Opening a NIC needs root (BPF), as raw sockets do on Linux.
DEPS="libpcap"
# A static library, as upstream builds it by default: the libpcap port
# installs only a static, non-PIC libpcap.a, which cannot go into a shared
# libsoem.so.
# eni_test runs the build machine's python on an ENI XML file at build time;
# it is a test program, not a tool, and is left out with eoe_test (Linux only
# upstream: it needs a TAP device).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=OFF -DSOEM_BUILD_SAMPLES=ON -DCMAKE_DISABLE_FIND_PACKAGE_Python3=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
