# ruckig 0.17.3

> Ruckig: time-optimal, jerk-limited online trajectory generation for robots

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pantor/ruckig.git (tag `v0.17.3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ruckig`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ruckig` (programs, shared libraries, run-time data) and `ruckig-dev` (headers, pkg-config/CMake files, static libraries; needs `ruckig`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ruckig`](https://openqnx.github.io/oss/package/x86_64/ruckig.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ruckig.html)) · [`ruckig-dev`](https://openqnx.github.io/oss/package/x86_64/ruckig-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ruckig-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ruckig` (dependencies come along); one version: `apk add ruckig=<version>` | `apk del ruckig` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ruckig` (dependencies first) | `oss-get remove ruckig` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ruckig-dev` the same way, by its own name (`apk add ruckig-dev`, `oss-get install ruckig-dev`, by hand with its own file and list `ruckig-dev.list`); remove it before `ruckig`, which it needs.

```sh
# on the QNX target, as root
apk add ruckig ruckig-dev
apk add qnx-test-ruckig && /usr/local/bin/qnx-port-tests/test_ruckig    # its on-target test, from tests-8.0-<arch>-m-z
apk del ruckig ruckig-dev qnx-test-ruckig
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ruckig.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ruckig.list
rm var/lib/manual/ruckig.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ruckig` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.17.3 --recurse-submodules --shallow-submodules \
    https://github.com/pantor/ruckig.git src/ruckig
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ruckig -B build/$QNX_ARCH/ruckig -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_BENCHMARK=OFF \
      -DBUILD_PYTHON_MODULE=OFF \
      -DBUILD_CLOUD_CLIENT=OFF \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/ruckig -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ruckig
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Ruckig: instantaneous, time-optimal and jerk-limited trajectory generation for robots and machines (the Community Version: online trajectories for any number of degrees of freedom, with velocity, acceleration and jerk limits). MoveIt 2 smooths its trajectories with it. The library only: its tests, examples and benchmark (doctest, nlohmann/json bundled for them) and the Python module are not built; nor is the cloud client (BUILD_CLOUD_CLIENT, which computes Ruckig Pro trajectories on a remote server over HTTP): it is compiled with Ruckig's bundled nlohmann/json and exports functions with that JSON's types, while consumers' <nlohmann/json.hpp> is the nlohmann_json component's (another ABI tag): MoveIt's libmoveit_trajectory_processing then failed to load ("unknown symbol: ruckig::CloudClient::post(nlohmann::json_abi_v3_12_0::...)").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ruckig`.

### 5. What it installs

22 files under `/usr/local`: 1 shared-library entries, 16 headers.

```
/usr/local/lib/libruckig.so
```

The complete list is `stage/<arch>/.manifest/ruckig` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ruckig
ls dists/8.0/*/ | grep '^ruckig-'
```

This writes `ruckig-<ver>-r0.tar.gz` (run time) and `ruckig-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ruckig
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ruckig-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ruckig-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ruckig`](../../tests/src/) links ruckig the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ruckig`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ruckig
/usr/local/bin/qnx-port-tests/test_ruckig
```

```
== ruckig ==
  ok   ...
PASS ruckig
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ruckig` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ruckig`](../../tests/src/) -- a program linked against ruckig the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - calculate() succeeds
   - ends at 1 m, at rest
   - velocity and acceleration stay within their limits
   - online update() loop reaches the 3-DoF target

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ruckig 0.17.3-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/ruckig-0.17.3-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ruckig 0.17.3-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/ruckig-0.17.3-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.17.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ruckig=<version>`), sources in `ported/ruckig/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ruckig`; port one with `scripts/port-update ruckig`.

<details><summary>recipe.sh</summary>

```bash
# Ruckig: instantaneous, time-optimal and jerk-limited trajectory generation
# for robots and machines (the Community Version: online trajectories for any
# number of degrees of freedom, with velocity, acceleration and jerk limits).
# MoveIt 2 smooths its trajectories with it. The library only: its tests,
# examples and benchmark (doctest, nlohmann/json bundled for them) and the
# Python module are not built; nor is the cloud client (BUILD_CLOUD_CLIENT,
# which computes Ruckig Pro trajectories on a remote server over HTTP): it
# is compiled with Ruckig's bundled nlohmann/json and exports functions with
# that JSON's types, while consumers' <nlohmann/json.hpp> is the
# nlohmann_json component's (another ABI tag): MoveIt's
# libmoveit_trajectory_processing then failed to load ("unknown symbol:
# ruckig::CloudClient::post(nlohmann::json_abi_v3_12_0::...)").
VERSION=0.17.3; GIT=https://github.com/pantor/ruckig.git; REF=v0.17.3
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARK=OFF -DBUILD_PYTHON_MODULE=OFF -DBUILD_CLOUD_CLIENT=OFF -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
