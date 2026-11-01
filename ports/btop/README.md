# btop 1.4.6

> Terminal resource monitor for CPU, memory, disks, network and processes

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/aristocratos/btop.git (tag `v1.4.6`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_monitors`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `btop`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`btop`](https://openqnx.github.io/oss/package/x86_64/btop.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/btop.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add btop` (dependencies come along); one version: `apk add btop=<version>` | `apk del btop` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install btop` (dependencies first) | `oss-get remove btop` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-monitors && /usr/local/bin/qnx-port-tests/test_monitors    # its on-target test, from tests-8.0-<arch>-m-z
apk del btop qnx-test-monitors
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/btop.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/btop.list
rm var/lib/manual/btop.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild btop` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.4.6 --recurse-submodules --shallow-submodules \
    https://github.com/aristocratos/btop.git src/btop
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/btop < ports/btop/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/btop < ports/btop/patches/0002-xdg-dirs-not-constexpr.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — btop on QNX, from QNX's port (github.com/qnx-ports/btop, branch qnx-1.4.6):

> a QNX collector (CPU, memory, processes, network from QNX's interfaces)
> and the build changes that select it.

Touches: `CMakeLists.txt`, `Makefile`, `src/btop.cpp`, `src/btop_config.cpp`, `src/btop_draw.cpp`, `src/btop_shared.hpp` … · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-xdg-dirs-not-constexpr.patch`](patches/0002-xdg-dirs-not-constexpr.patch)** — Do not declare the XDG state directory lookup constexpr

> get_xdg_state_dir() is declared constexpr but calls std::getenv(), so it can
> never be evaluated at compile time. C++23 allows
> such a declaration; GCC 12 -- QNX SDP 8.0's compiler -- rejects it ("call to
> non-'constexpr' function 'char* getenv(const char*)'"). It is only ever
> called at run time: a plain function.

Touches: `src/btop_config.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/btop -B build/$QNX_ARCH/btop -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBTOP_GPU=OFF \
      -DBTOP_LTO=OFF
cmake --build build/$QNX_ARCH/btop -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/btop
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- btop: resource monitor (CPU, memory, disks, network, processes), with QNX's QNX collector (patches/0001, from github.com/qnx-ports/btop).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH btop`.

### 5. What it installs

40 files under `/usr/local`, 1 programs.

```
/usr/local/bin/btop
```

The complete list is `stage/<arch>/.manifest/btop` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa btop
ls dists/8.0/*/ | grep '^btop-'
```

This writes `btop-<ver>-r0.tar.gz` (run time) and `btop-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add btop
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/btop-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/btop-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_monitors`](../../tests/src/) links btop the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-monitors`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-monitors
/usr/local/bin/qnx-port-tests/test_monitors
```

```
== btop ==
  ok   ...
PASS btop
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_monitors` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_monitors`](../../tests/src/) -- a program linked against btop the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - htop drew a screen
   - htop: header and meters
   - htop: lists sshd (QNX process table)
   - htop: lists io-sock
   - htop exits on q
   - btop drew a screen
   - btop: cpu, mem, proc boxes
   - btop: lists this target's processes
   - btop exits on q

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | btop 1.4.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/btop-1.4.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | htop 3.5.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/htop-3.5.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | btop 1.4.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/btop-1.4.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | htop 3.5.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/htop-3.5.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.4.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add btop=<version>`), sources in `ported/btop/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates btop`; port one with `scripts/port-update btop`.

<details><summary>recipe.sh</summary>

```bash
# btop: resource monitor (CPU, memory, disks, network, processes), with QNX's
# QNX collector (patches/0001, from github.com/qnx-ports/btop).
VERSION=1.4.6; GIT=https://github.com/aristocratos/btop.git; REF=v1.4.6
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBTOP_GPU=OFF -DBTOP_LTO=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
