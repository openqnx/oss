# geographiclib 2.7

> Small C++ library for geodesic and other geographic conversions

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/geographiclib/geographiclib.git (tag `v2.7`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_geographiclib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `geographiclib` (programs, shared libraries, run-time data) and `geographiclib-dev` (headers, pkg-config/CMake files, static libraries; needs `geographiclib`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`geographiclib`](https://openqnx.github.io/oss/package/x86_64/geographiclib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/geographiclib.html)) · [`geographiclib-dev`](https://openqnx.github.io/oss/package/x86_64/geographiclib-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/geographiclib-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add geographiclib` (dependencies come along); one version: `apk add geographiclib=<version>` | `apk del geographiclib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install geographiclib` (dependencies first) | `oss-get remove geographiclib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`geographiclib-dev` the same way, by its own name (`apk add geographiclib-dev`, `oss-get install geographiclib-dev`, by hand with its own file and list `geographiclib-dev.list`); remove it before `geographiclib`, which it needs.

```sh
# on the QNX target, as root
apk add geographiclib geographiclib-dev
apk add qnx-test-geographiclib && /usr/local/bin/qnx-port-tests/test_geographiclib    # its on-target test, from tests-8.0-<arch>
apk del geographiclib geographiclib-dev qnx-test-geographiclib
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/geographiclib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/geographiclib.list
rm var/lib/manual/geographiclib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild geographiclib` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.7 --recurse-submodules --shallow-submodules \
    https://github.com/geographiclib/geographiclib.git src/geographiclib
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/geographiclib -B build/$QNX_ARCH/geographiclib -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBS=OFF \
      -DBUILD_DOCUMENTATION=OFF \
      -DBUILD_MANPAGES=OFF \
      -DEXAMPLEDIR=
cmake --build build/$QNX_ARCH/geographiclib -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/geographiclib
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The C++ library and its command-line tools (GeoConvert, GeodSolve, ...). No documentation (needs doxygen) and no tests (they run the tools on the build machine). The data directory for geoid/gravity/magnetic models is compiled in as $PREFIX/share/GeographicLib; the models themselves are separate downloads.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH geographiclib`.

### 5. What it installs

102 files under `/usr/local`: 3 shared-library entries, 18 programs, 55 headers.

```
/usr/local/lib/libGeographicLib.so
/usr/local/lib/libGeographicLib.so.26
/usr/local/lib/libGeographicLib.so.26.2.1
/usr/local/bin/Cart3Convert
/usr/local/bin/CartConvert
/usr/local/bin/Conformal3Proj
/usr/local/bin/ConicProj
/usr/local/bin/GeoConvert
/usr/local/bin/Geod3Solve
/usr/local/bin/GeodSolve
/usr/local/bin/GeodesicProj
/usr/local/bin/GeoidEval
/usr/local/bin/Gravity
/usr/local/bin/IntersectTool
/usr/local/bin/MagneticField
/usr/local/bin/Planimeter
...
```

The complete list is `stage/<arch>/.manifest/geographiclib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa geographiclib
ls dists/8.0/*/ | grep '^geographiclib-'
```

This writes `geographiclib-<ver>-r0.tar.gz` (run time) and `geographiclib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add geographiclib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/geographiclib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/geographiclib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_geographiclib`](../../tests/src/) links geographiclib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-geographiclib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-geographiclib
/usr/local/bin/qnx-port-tests/test_geographiclib
```

```
== geographiclib ==
  ok   ...
PASS geographiclib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_geographiclib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_geographiclib`](../../tests/src/) -- a program linked against geographiclib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - DMS::Decode(40:38:23N)
   - Inverse JFK -> SIN: 15347628 m
   - azimuths 003:18:29.9 and 177:29:09.2
   - Direct from JFK with that azimuth and length reaches SIN
   - midpoint 70:20:29.2N 097:01:29.4E (the GeodSolve -F waypoint example)
   - UTM of 33.3 44.4 = 38n 444140.54 3684706.36
   - MGRS = 38SMB4414084706
   - MGRS::Reverse(38SMB4484) = centre of the 1 km square
   - Planimeter 18SWK: 400139.532959 m, 10007388597.2 m^2
   - Geocentric forward + reverse
   - WGS84 a = 6378137 m

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | geographiclib 2.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/geographiclib-2.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | geographiclib 2.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/geographiclib-2.7-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add geographiclib=<version>`), sources in `ported/geographiclib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates geographiclib`; port one with `scripts/port-update geographiclib`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.7; GIT=https://github.com/geographiclib/geographiclib.git; REF=v2.7
# The C++ library and its command-line tools (GeoConvert, GeodSolve, ...).
# No documentation (needs doxygen) and no tests (they run the tools on the
# build machine). The data directory for geoid/gravity/magnetic models is
# compiled in as $PREFIX/share/GeographicLib; the models themselves are
# separate downloads.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_DOCUMENTATION=OFF
                   -DBUILD_MANPAGES=OFF -DEXAMPLEDIR= )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
