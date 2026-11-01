# fastfetch 2.66.1

> Like neofetch, but much faster because written in C

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/fastfetch-cli/fastfetch.git (tag `2.66.1`) |
| Build system | CMake |
| QNX patches | 6 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fastfetch`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fastfetch` (programs, shared libraries, run-time data) and `fastfetch-dev` (headers, pkg-config/CMake files, static libraries; needs `fastfetch`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fastfetch`](https://openqnx.github.io/oss/package/x86_64/fastfetch.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fastfetch.html)) · [`fastfetch-dev`](https://openqnx.github.io/oss/package/x86_64/fastfetch-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fastfetch-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fastfetch` (dependencies come along); one version: `apk add fastfetch=<version>` | `apk del fastfetch` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fastfetch` (dependencies first) | `oss-get remove fastfetch` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fastfetch-dev` the same way, by its own name (`apk add fastfetch-dev`, `oss-get install fastfetch-dev`, by hand with its own file and list `fastfetch-dev.list`); remove it before `fastfetch`, which it needs.

```sh
# on the QNX target, as root
apk add fastfetch fastfetch-dev
apk add qnx-test-fastfetch && /usr/local/bin/qnx-port-tests/test_fastfetch    # its on-target test, from tests-8.0-<arch>
apk del fastfetch fastfetch-dev qnx-test-fastfetch
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fastfetch.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fastfetch.list
rm var/lib/manual/fastfetch.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fastfetch` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.66.1 --recurse-submodules --shallow-submodules \
    https://github.com/fastfetch-cli/fastfetch.git src/fastfetch
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0001-qnx-platform.patch
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0002-qnx-process-info.patch
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0003-qnx-no-dirent-d-type.patch
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0004-qnx-bsd-network-code.patch
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0005-qnx-colors-min-max.patch
patch -p1 -d src/fastfetch < ports/fastfetch/patches/0006-nosupport-signatures.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — Add QNX as a platform: build list and QNX detection code

> fastfetch knows each operating system by name and builds a source list of
> its own for it; QNX stopped at "Unsupported platform: QNX". This adds
> QNX (CMAKE_SYSTEM_NAME QNX) with
> - os_qnx.c: name, release and build from uname() (no os-release file);
> - cpu_qnx.c: CPU count and speed from the system page (cpuinfo), the
>   name from cpuid on x86 or the system page's string table;
> - memory_qnx.c: RAM from the system page's "ram" address-space entries
>   (sysconf(_SC_PHYS_PAGES) is not implemented on QNX), free memory as the
>   size procnto reports for /proc;
> - uptime_qnx.c: the boot time from the system page (qtime->boot_time);
> - processes_qnx.c: the numeric entries of /proc;
> - host_qnx.c: the board name (uname -m) and the architecture;
> - disk_qnx.c: QNX has no mount table, so "/" and every top-level
>   directory on a file system of its own (st_dev), with statvfs() and the
>   device from DCMD_FSYS_MOUNTED_ON;
> - displayserver_qnx.c: QNX Screen when /dev/screen exists;
> and the generic Unix parts (terminal, shell, locale, DNS, network
> interfaces through the PF_ROUTE socket) shared with the other ports;
> _QNX_SOURCE, and libm/libsocket for the link. Everything else is the
> existing *_nosupport.c.

Touches: `CMakeLists.txt`, `src/detection/cpu/cpu_qnx.c`, `src/detection/disk/disk_qnx.c`, `src/detection/displayserver/displayserver_qnx.c`, `src/detection/host/host_qnx.c`, `src/detection/memory/memory_qnx.c` … · Upstream status: QNX-specific

**[`0002-qnx-process-info.patch`](patches/0002-qnx-process-info.patch)** — QNX: executable path, process names and parents from /proc

> QNX's procfs is not Linux's: /proc/<pid>/exefile holds a process's
> executable path as text (there is no exe link or stat file), and the
> parent comes from devctl(DCMD_PROC_INFO) on /proc/<pid>/as. Used for
> fastfetch's own path (data files next to it) and for the shell and
> terminal detection, which walks up the process tree. procfs_info has no
> controlling terminal: tty is reported as -1.

Touches: `src/common/impl/FFPlatform_unix.c`, `src/common/impl/processing_linux.c` · Upstream status: QNX-specific

**[`0003-qnx-no-dirent-d-type.patch`](patches/0003-qnx-no-dirent-d-type.patch)** — QNX: no d_type in struct dirent

> QNX's struct dirent has no d_type (nor DT_* constants): use the stat()
> path the code already has for Solaris and Haiku.

Touches: `src/common/impl/io_unix.c`, `src/detection/packages/packages.c` · Upstream status: QNX-specific

**[`0004-qnx-bsd-network-code.patch`](patches/0004-qnx-bsd-network-code.patch)** — QNX: use the BSD routing-socket and interface code

> QNX 8's network stack (io-sock) is FreeBSD's: the default route comes
> from the PF_ROUTE sysctl with FreeBSD's address alignment, and link-layer
> addresses are AF_LINK with <net/if_dl.h> and <net/if_media.h> -- there is
> no <netpacket/packet.h>. Take the FreeBSD branches on QNX.

Touches: `src/common/impl/netif_bsd.c`, `src/detection/localip/localip_linux.c` · Upstream status: QNX-specific

**[`0005-qnx-colors-min-max.patch`](patches/0005-qnx-colors-min-max.patch)** — colors: QNX's <stdlib.h> defines min() and max() macros

> With _QNX_SOURCE, QNX's <stdlib.h> defines min(a,b) and max(a,b) as
> macros, which turned colors.c's static inline min()/max() definitions into
> syntax errors. #undef them first.

Touches: `src/modules/colors/colors.c` · Upstream status: QNX-specific

**[`0006-nosupport-signatures.patch`](patches/0006-nosupport-signatures.patch)** — sound, swap: update the *_nosupport.c stubs to the current prototypes

> ffDetectSound() takes the module options and ffDetectSwap() a list since
> earlier releases, but the stubs for platforms without support still had
> the old signatures and did not compile (no platform in the tree used them
> until now).

Touches: `src/detection/sound/sound_nosupport.c`, `src/detection/swap/swap_nosupport.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fastfetch -B build/$QNX_ARCH/fastfetch -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_SYSTEM_YYJSON=OFF \
      -DENABLE_ZLIB=OFF \
      -DENABLE_LUA=OFF \
      -DENABLE_LIBZFS=OFF \
      -DENABLE_THREADS=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_FLASHFETCH=OFF \
      -DENABLE_LTO=OFF
cmake --build build/$QNX_ARCH/fastfetch -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fastfetch
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fastfetch: system information at a glance (OS, kernel, CPU, memory, disks, uptime, shell, terminal, network addresses ...), as text or JSON. 2.66.1, the last release in C11: from 2.67.0 fastfetch requires C23 (nullptr, enums with a fixed type, auto) and GCC 13, and QNX SDP 8.0's compiler is GCC 12.2. qnx-ports' fastfetch branch (qnx-2.68.1) carries no QNX changes; QNX support is patches/0001-0005 here: a QNX platform with its own detection code (system page, /proc, statvfs; 0001-0002), no d_type (0003), the FreeBSD network code QNX's io-sock shares (0004), QNX's min/max macros (0005), and two stub files fixed upstream-style (0006).
- Built-in yyjson; no Lua format scripting, libzfs or zlib (image logos). Everything optional fastfetch would dlopen on Linux (X11, Wayland, Vulkan, OpenCL, D-Bus ...) is off for QNX by the build's own platform switches. LTO off: not tried with the QNX toolchain.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fastfetch`.

### 5. What it installs

43 files under `/usr/local`, 1 programs.

```
/usr/local/bin/fastfetch
```

The complete list is `stage/<arch>/.manifest/fastfetch` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fastfetch
ls dists/8.0/*/ | grep '^fastfetch-'
```

This writes `fastfetch-<ver>-r0.tar.gz` (run time) and `fastfetch-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fastfetch
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fastfetch-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fastfetch-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fastfetch`](../../tests/src/) links fastfetch the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fastfetch`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fastfetch
/usr/local/bin/qnx-port-tests/test_fastfetch
```

```
== fastfetch ==
  ok   ...
PASS fastfetch
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fastfetch` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fastfetch`](../../tests/src/) -- a program linked against fastfetch the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fastfetch 2.66.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/fastfetch-2.66.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fastfetch 2.66.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/fastfetch-2.66.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.66.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fastfetch=<version>`), sources in `ported/fastfetch/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fastfetch`; port one with `scripts/port-update fastfetch`.

<details><summary>recipe.sh</summary>

```bash
# fastfetch: system information at a glance (OS, kernel, CPU, memory,
# disks, uptime, shell, terminal, network addresses ...), as text or JSON.
# 2.66.1, the last release in C11: from 2.67.0 fastfetch requires C23
# (nullptr, enums with a fixed type, auto) and GCC 13, and QNX SDP 8.0's
# compiler is GCC 12.2. qnx-ports' fastfetch branch (qnx-2.68.1) carries no
# QNX changes; QNX support is patches/0001-0005 here: a QNX platform with
# its own detection code (system page, /proc, statvfs; 0001-0002), no
# d_type (0003), the FreeBSD network code QNX's io-sock shares (0004),
# QNX's min/max macros (0005), and two stub files fixed upstream-style
# (0006).
VERSION=2.66.1; GIT=https://github.com/fastfetch-cli/fastfetch.git; REF=2.66.1
# Built-in yyjson; no Lua format scripting, libzfs or zlib (image logos).
# Everything optional fastfetch would dlopen on Linux (X11, Wayland,
# Vulkan, OpenCL, D-Bus ...) is off for QNX by the build's own platform
# switches. LTO off: not tried with the QNX toolchain.
CMAKE_ARGS_EXTRA=( -DENABLE_SYSTEM_YYJSON=OFF -DENABLE_ZLIB=OFF -DENABLE_LUA=OFF -DENABLE_LIBZFS=OFF
                   -DENABLE_THREADS=ON -DBUILD_TESTS=OFF -DBUILD_FLASHFETCH=OFF -DENABLE_LTO=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
