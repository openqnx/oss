# bullet3 3.25

> Bullet real-time collision detection and multi-physics simulation library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/bulletphysics/bullet3.git (tag `3.25`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_bullet3`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `bullet3` (programs, shared libraries, run-time data) and `bullet3-dev` (headers, pkg-config/CMake files, static libraries; needs `bullet3`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`bullet3`](https://openqnx.github.io/oss/package/x86_64/bullet3.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/bullet3.html)) · [`bullet3-dev`](https://openqnx.github.io/oss/package/x86_64/bullet3-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/bullet3-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add bullet3` (dependencies come along); one version: `apk add bullet3=<version>` | `apk del bullet3` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install bullet3` (dependencies first) | `oss-get remove bullet3` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`bullet3-dev` the same way, by its own name (`apk add bullet3-dev`, `oss-get install bullet3-dev`, by hand with its own file and list `bullet3-dev.list`); remove it before `bullet3`, which it needs.

```sh
# on the QNX target, as root
apk add bullet3 bullet3-dev
apk add qnx-test-bullet3 && /usr/local/bin/qnx-port-tests/test_bullet3    # its on-target test, from tests-8.0-<arch>
apk del bullet3 bullet3-dev qnx-test-bullet3
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/bullet3.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/bullet3.list
rm var/lib/manual/bullet3.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild bullet3` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.25 --recurse-submodules --shallow-submodules \
    https://github.com/bulletphysics/bullet3.git src/bullet3
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/bullet3 -B build/$QNX_ARCH/bullet3 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DINSTALL_LIBS=ON \
      -DBUILD_BULLET2_DEMOS=OFF \
      -DBUILD_CPU_DEMOS=OFF \
      -DBUILD_OPENGL3_DEMOS=OFF \
      -DBUILD_EGL=OFF \
      -DBUILD_ENET=OFF \
      -DBUILD_CLSOCKET=OFF \
      -DBUILD_UNIT_TESTS=OFF \
      -DBUILD_PYBULLET=OFF \
      -DBUILD_EXTRAS=ON \
      -DINSTALL_EXTRA_LIBS=ON \
      -DBUILD_BULLET3=ON \
      -DBUILD_BULLET_ROBOTICS_EXTRA=OFF \
      -DBUILD_BULLET_ROBOTICS_GUI_EXTRA=OFF \
      -DBUILD_OBJ2SDF_EXTRA=OFF \
      -DENABLE_VHACD=OFF \
      -DUSE_GLUT=OFF \
      -DUSE_GRAPHICAL_BENCHMARK=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/bullet3 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/bullet3
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Bullet Physics SDK: the core libraries only -- LinearMath, collision detection, rigid/soft body dynamics, Bullet3 (the GPU pipeline's common, geometry, collision and dynamics parts, and clew, which only looks for an OpenCL library at run time) and the Extras (world importers, inverse dynamics, convex decomposition, GImpact utilities). Not BulletRobotics, BulletRoboticsGUI and obj2sdf: they are built from the examples tree (the pybullet shared-memory and GUI client code). No examples, demos, GUI, OpenGL or pybullet: the target has no display.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH bullet3`.

### 5. What it installs

498 files under `/usr/local`: 36 shared-library entries, 459 headers.

```
/usr/local/lib/libBullet2FileLoader.so
/usr/local/lib/libBullet2FileLoader.so.3.25
/usr/local/lib/libBullet3Collision.so
/usr/local/lib/libBullet3Collision.so.3.25
/usr/local/lib/libBullet3Common.so
/usr/local/lib/libBullet3Common.so.3.25
/usr/local/lib/libBullet3Dynamics.so
/usr/local/lib/libBullet3Dynamics.so.3.25
/usr/local/lib/libBullet3Geometry.so
/usr/local/lib/libBullet3Geometry.so.3.25
/usr/local/lib/libBullet3OpenCL_clew.so
/usr/local/lib/libBullet3OpenCL_clew.so.3.25
/usr/local/lib/libBulletCollision.so
/usr/local/lib/libBulletCollision.so.3.25
/usr/local/lib/libBulletDynamics.so
/usr/local/lib/libBulletDynamics.so.3.25
...
```

The complete list is `stage/<arch>/.manifest/bullet3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa bullet3
ls dists/8.0/*/ | grep '^bullet3-'
```

This writes `bullet3-<ver>-r0.tar.gz` (run time) and `bullet3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add bullet3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/bullet3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/bullet3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_bullet3`](../../tests/src/) links bullet3 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-bullet3`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-bullet3
/usr/local/bin/qnx-port-tests/test_bullet3
```

```
== bullet3 ==
  ok   ...
PASS bullet3
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_bullet3` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_bullet3`](../../tests/src/) -- a program linked against bullet3 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - quaternion: 90 deg about z maps x to y
   - transform inverse undoes the transform
   - dot and cross products
   - sphere inertia is 2/5 m r^2
   - free fall follows 1/2 g t^2
   - velocity after 0.5 s is about -g t
   - the sphere rests on the plane at its radius (0.5)
   - the resting sphere is put to sleep
   - and is not moving
   - a vertical drop stays vertical
   - ray cast hits the ground at y = 0 with normal +y
   - a ray from above hits the resting sphere's top (y = 1)
   - contactPairTest: sphere and overlapping box are in contact
   - contactPairTest: separated objects have no contact
   - GJK: distance between spheres 3 apart is 1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | bullet3 3.25-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/bullet3-3.25-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | bullet3 3.25-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/bullet3-3.25-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.25**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add bullet3=<version>`), sources in `ported/bullet3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates bullet3`; port one with `scripts/port-update bullet3`.

<details><summary>recipe.sh</summary>

```bash
# Bullet Physics SDK: the core libraries only -- LinearMath, collision
# detection, rigid/soft body dynamics, Bullet3 (the GPU pipeline's common,
# geometry, collision and dynamics parts, and clew, which only looks for an
# OpenCL library at run time) and the Extras (world importers, inverse dynamics, convex decomposition,
# GImpact utilities). Not BulletRobotics, BulletRoboticsGUI and obj2sdf:
# they are built from the examples tree (the pybullet shared-memory and
# GUI client code). No examples, demos, GUI, OpenGL or pybullet: the
# target has no display.
VERSION=3.25; GIT=https://github.com/bulletphysics/bullet3.git; REF=3.25
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DINSTALL_LIBS=ON
                   -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_OPENGL3_DEMOS=OFF
                   -DBUILD_EGL=OFF -DBUILD_ENET=OFF -DBUILD_CLSOCKET=OFF
                   -DBUILD_UNIT_TESTS=OFF -DBUILD_PYBULLET=OFF
                   -DBUILD_EXTRAS=ON -DINSTALL_EXTRA_LIBS=ON -DBUILD_BULLET3=ON
                   -DBUILD_BULLET_ROBOTICS_EXTRA=OFF -DBUILD_BULLET_ROBOTICS_GUI_EXTRA=OFF
                   -DBUILD_OBJ2SDF_EXTRA=OFF -DENABLE_VHACD=OFF -DUSE_GLUT=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
