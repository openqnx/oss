# qhull 8.0.2

> Computational geometry: convex hulls, Delaunay triangulation, Voronoi

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/qhull/qhull.git (tag `v8.0.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qhull`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qhull` (programs, shared libraries, run-time data) and `qhull-dev` (headers, pkg-config/CMake files, static libraries; needs `qhull`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qhull`](https://openqnx.github.io/oss/package/x86_64/qhull.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qhull.html)) · [`qhull-dev`](https://openqnx.github.io/oss/package/x86_64/qhull-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qhull-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qhull` (dependencies come along); one version: `apk add qhull=<version>` | `apk del qhull` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qhull` (dependencies first) | `oss-get remove qhull` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qhull-dev` the same way, by its own name (`apk add qhull-dev`, `oss-get install qhull-dev`, by hand with its own file and list `qhull-dev.list`); remove it before `qhull`, which it needs.

```sh
# on the QNX target, as root
apk add qhull qhull-dev
apk add qnx-test-qhull && /usr/local/bin/qnx-port-tests/test_qhull    # its on-target test, from tests-8.0-<arch>-m-z
apk del qhull qhull-dev qnx-test-qhull
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qhull.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qhull.list
rm var/lib/manual/qhull.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qhull` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v8.0.2 --recurse-submodules --shallow-submodules \
    https://github.com/qhull/qhull.git src/qhull
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/qhull -B build/$QNX_ARCH/qhull -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBS=ON \
      -DLINK_APPS_SHARED=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/qhull -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/qhull
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Qhull: convex hulls, Delaunay triangulations, Voronoi diagrams and halfspace intersections (libqhull_r, the reentrant library matplotlib's triangulation uses; also the qhull, qconvex, qdelaunay ... programs and libqhullcpp).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qhull`.

### 5. What it installs

112 files under `/usr/local`: 3 shared-library entries, 6 programs, 49 headers.

```
/usr/local/lib/libqhull_r.so
/usr/local/lib/libqhull_r.so.8.0
/usr/local/lib/libqhull_r.so.8.0.2
/usr/local/bin/qconvex
/usr/local/bin/qdelaunay
/usr/local/bin/qhalf
/usr/local/bin/qhull
/usr/local/bin/qvoronoi
/usr/local/bin/rbox
```

The complete list is `stage/<arch>/.manifest/qhull` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qhull
ls dists/8.0/*/ | grep '^qhull-'
```

This writes `qhull-<ver>-r0.tar.gz` (run time) and `qhull-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qhull
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qhull-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qhull-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qhull`](../../tests/src/) links qhull the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qhull`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qhull
/usr/local/bin/qnx-port-tests/test_qhull
```

```
== qhull ==
  ok   ...
PASS qhull
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qhull` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qhull`](../../tests/src/) -- a program linked against qhull the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - 2-d hull computed
   - square + 3 interior points: 4 vertices, 4 edges
   - area 1, perimeter 4
   - 3-d hull computed
   - cube + centre: 8 vertices, 12 triangles
   - volume 1, area 6
   - Delaunay triangulation computed
   - square + centre: 4 Delaunay triangles
   - qhull Fx: hull points 0..3 of the square, not the centre

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qhull 8.0.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/qhull-8.0.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qhull 8.0.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/qhull-8.0.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.0.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qhull=<version>`), sources in `ported/qhull/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qhull`; port one with `scripts/port-update qhull`.

<details><summary>recipe.sh</summary>

```bash
# Qhull: convex hulls, Delaunay triangulations, Voronoi diagrams and halfspace
# intersections (libqhull_r, the reentrant library matplotlib's triangulation
# uses; also the qhull, qconvex, qdelaunay ... programs and libqhullcpp).
VERSION=8.0.2; GIT=https://github.com/qhull/qhull.git; REF=v8.0.2
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON -DLINK_APPS_SHARED=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
