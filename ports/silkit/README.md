# silkit 5.0.8

> Open-source library for connecting software-in-the-loop simulations

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/vectorgrp/sil-kit.git (tag `v5.0.8`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_silkit`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `silkit` (programs, shared libraries, run-time data) and `silkit-dev` (headers, pkg-config/CMake files, static libraries; needs `silkit`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`silkit`](https://openqnx.github.io/oss/package/x86_64/silkit.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/silkit.html)) · [`silkit-dev`](https://openqnx.github.io/oss/package/x86_64/silkit-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/silkit-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add silkit` (dependencies come along); one version: `apk add silkit=<version>` | `apk del silkit` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install silkit` (dependencies first) | `oss-get remove silkit` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`silkit-dev` the same way, by its own name (`apk add silkit-dev`, `oss-get install silkit-dev`, by hand with its own file and list `silkit-dev.list`); remove it before `silkit`, which it needs.

```sh
# on the QNX target, as root
apk add silkit silkit-dev
apk add qnx-test-silkit && /usr/local/bin/qnx-port-tests/test_silkit    # its on-target test, from tests-8.0-<arch>-m-z
apk del silkit silkit-dev qnx-test-silkit
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/silkit.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/silkit.list
rm var/lib/manual/silkit.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild silkit` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v5.0.8 --recurse-submodules --shallow-submodules \
    https://github.com/vectorgrp/sil-kit.git src/silkit
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/silkit < ports/silkit/patches/0001-qnx-current-path-getcwd.patch
```

**[`0001-qnx-current-path-getcwd.patch`](patches/0001-qnx-current-path-getcwd.patch)** — Do not use std::filesystem::current_path() on QNX.

> libc++'s current_path() sizes its getcwd() buffer with pathconf(".",
> _PC_PATH_MAX) and throws when that fails. On QNX 8.0 pathconf() fails
> with EINVAL when the working directory is "/" (the image filesystem), so
> every participant creation threw "filesystem error: in current_path: call
> to pathconf failed: Invalid argument" in a process started from "/"
> (VAsioConnection hashes the working directory into its local socket
> name). util/CurrentPath.hpp calls getcwd() with a PATH_MAX buffer on QNX
> and current_path() elsewhere; the four library uses go through it
> (std::filesystem::absolute() of a relative path calls current_path() too).

Touches: `SilKit/source/config/ParticipantConfigurationFromXImpl.cpp`, `SilKit/source/core/vasio/VAsioConnection.cpp`, `SilKit/source/extensions/SilKitExtensions.cpp`, `SilKit/source/util/CurrentPath.hpp`, `SilKit/source/util/Uri.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/silkit -B build/$QNX_ARCH/silkit -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSILKIT_BUILD_DEMOS=OFF \
      -DSILKIT_BUILD_TESTS=OFF \
      -DSILKIT_BUILD_UTILITIES=ON \
      -DSILKIT_BUILD_DOCS=OFF \
      -DSILKIT_BUILD_DASHBOARD=OFF \
      -DSILKIT_BUILD_STATIC=OFF \
      -DSILKIT_USE_SYSTEM_LIBRARIES=OFF \
      -DSILKIT_WARNINGS_AS_ERRORS=OFF \
      -DSILKIT_BUILD_REPRODUCIBLE=ON
cmake --build build/$QNX_ARCH/silkit -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/silkit
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Vector SIL Kit: an open-source library for connecting software-in-the-loop simulations -- participants exchange CAN, LIN, FlexRay, Ethernet and publish/subscribe or RPC data through a registry, with virtual time synchronisation. Upstream supports QNX (it ships QNX toolchain presets). Built: libSilKit and the utilities (sil-kit-registry, sil-kit-monitor, sil-kit-system-controller). Not built: demos, tests, documentation, the dashboard client. asio, fmt, spdlog and the YAML parser are SIL Kit's own bundled copies (git submodules), linked into libSilKit and not exported. patches/0001: std::filesystem::current_path() throws on QNX when the working directory is "/" (pathconf EINVAL); getcwd() instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH silkit`.

### 5. What it installs

135 files under `/usr/local`: 1 shared-library entries, 3 programs, 127 headers.

```
/usr/local/lib/libSilKit.so
/usr/local/bin/sil-kit-monitor
/usr/local/bin/sil-kit-registry
/usr/local/bin/sil-kit-system-controller
```

The complete list is `stage/<arch>/.manifest/silkit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa silkit
ls dists/8.0/*/ | grep '^silkit-'
```

This writes `silkit-<ver>-r0.tar.gz` (run time) and `silkit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add silkit
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/silkit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/silkit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_silkit`](../../tests/src/) links silkit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-silkit`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-silkit
/usr/local/bin/qnx-port-tests/test_silkit
```

```
== silkit ==
  ok   ...
PASS silkit
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_silkit` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_silkit`](../../tests/src/) -- a program linked against silkit the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary directory
   - two participants joined through the registry
   - a late subscriber gets the publisher's history (the last value)
   - 50 messages arrive in order, byte for byte
   - a subscriber with a different mandatory label gets nothing
   - RPC: the server adds 40 + 2 and the client gets 42
   - SIL Kit calls do not throw

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | silkit 5.0.8-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/silkit-5.0.8-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | silkit 5.0.8-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/silkit-5.0.8-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.0.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add silkit=<version>`), sources in `ported/silkit/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates silkit`; port one with `scripts/port-update silkit`.

<details><summary>recipe.sh</summary>

```bash
# Vector SIL Kit: an open-source library for connecting software-in-the-loop
# simulations -- participants exchange CAN, LIN, FlexRay, Ethernet and
# publish/subscribe or RPC data through a registry, with virtual time
# synchronisation. Upstream supports QNX (it ships QNX toolchain presets).
# Built: libSilKit and the utilities (sil-kit-registry, sil-kit-monitor,
# sil-kit-system-controller). Not built: demos, tests, documentation, the
# dashboard client. asio, fmt, spdlog and the YAML parser are SIL Kit's own
# bundled copies (git submodules), linked into libSilKit and not exported.
# patches/0001: std::filesystem::current_path() throws on QNX when the
# working directory is "/" (pathconf EINVAL); getcwd() instead.
VERSION=5.0.8; GIT=https://github.com/vectorgrp/sil-kit.git; REF=v5.0.8
CMAKE_ARGS_EXTRA=( -DSILKIT_BUILD_DEMOS=OFF -DSILKIT_BUILD_TESTS=OFF -DSILKIT_BUILD_UTILITIES=ON
                   -DSILKIT_BUILD_DOCS=OFF -DSILKIT_BUILD_DASHBOARD=OFF -DSILKIT_BUILD_STATIC=OFF
                   -DSILKIT_USE_SYSTEM_LIBRARIES=OFF -DSILKIT_WARNINGS_AS_ERRORS=OFF
                   -DSILKIT_BUILD_REPRODUCIBLE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
