# assimp 6.0.2

> Open Asset Import Library: loads 3D model formats (OBJ, STL, COLLADA, glTF, FBX, ...)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/assimp/assimp.git (tag `v6.0.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_assimp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `assimp` (programs, shared libraries, run-time data) and `assimp-dev` (headers, pkg-config/CMake files, static libraries; needs `assimp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`assimp`](https://openqnx.github.io/oss/package/x86_64/assimp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/assimp.html)) · [`assimp-dev`](https://openqnx.github.io/oss/package/x86_64/assimp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/assimp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add assimp` (dependencies come along); one version: `apk add assimp=<version>` | `apk del assimp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install assimp` (dependencies first) | `oss-get remove assimp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`assimp-dev` the same way, by its own name (`apk add assimp-dev`, `oss-get install assimp-dev`, by hand with its own file and list `assimp-dev.list`); remove it before `assimp`, which it needs.

```sh
# on the QNX target, as root
apk add assimp assimp-dev
apk add qnx-test-assimp && /usr/local/bin/qnx-port-tests/test_assimp    # its on-target test, from tests-8.0-<arch>
apk del assimp assimp-dev qnx-test-assimp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/assimp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/assimp.list
rm var/lib/manual/assimp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild assimp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v6.0.2 --recurse-submodules --shallow-submodules \
    https://github.com/assimp/assimp.git src/assimp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/assimp < ports/assimp/patches/0001-qnx-minizip-no-lfs64.patch
```

**[`0001-qnx-minizip-no-lfs64.patch`](patches/0001-qnx-minizip-no-lfs64.patch)** — Use fopen/ftello/fseeko on QNX: there is no LFS64 API

> QNX has no fopen64/ftello64/fseeko64 (the LFS64 transitional API): off_t is
> 64 bits on both QNX 8 targets, and the plain fopen/ftello/fseeko already take
> large files. The library linked, but with undefined references to the *64
> names. Use the plain calls on QNX, as minizip does on macOS and Haiku.

Touches: `contrib/unzip/ioapi.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/assimp -B build/$QNX_ARCH/assimp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DASSIMP_BUILD_TESTS=OFF \
      -DASSIMP_BUILD_SAMPLES=OFF \
      -DASSIMP_BUILD_ASSIMP_TOOLS=ON \
      -DASSIMP_BUILD_ZLIB=OFF \
      -DASSIMP_WARNINGS_AS_ERRORS=OFF \
      -DASSIMP_INSTALL_PDB=OFF \
      -DCMAKE_INSTALL_LIBDIR=lib
cmake --build build/$QNX_ARCH/assimp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/assimp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- assimp: Open Asset Import Library -- loads 3D model formats (COLLADA, STL, OBJ, glTF, FBX, ...) into one scene structure. Needed by RViz (ros2-rviz) for robot meshes. Built against this tree's zlib (ASSIMP_BUILD_ZLIB off) rather than its copy. ASSIMP_WARNINGS_AS_ERRORS off: GCC 12's -Wstringop-truncation fires on assimp's own strncpy calls (EmbedTexturesProcess.cpp), which are bounded correctly. The `assimp` command-line tool is built too (info, export).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH assimp`.

### 5. What it installs

98 files under `/usr/local`: 3 shared-library entries, 1 programs, 89 headers.

```
/usr/local/lib/libassimp.so
/usr/local/lib/libassimp.so.6
/usr/local/lib/libassimp.so.6.0.2
/usr/local/bin/assimp
```

The complete list is `stage/<arch>/.manifest/assimp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa assimp
ls dists/8.0/*/ | grep '^assimp-'
```

This writes `assimp-<ver>-r0.tar.gz` (run time) and `assimp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add assimp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/assimp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/assimp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_assimp`](../../tests/src/) links assimp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-assimp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-assimp
/usr/local/bin/qnx-port-tests/test_assimp
```

```
== assimp ==
  ok   ...
PASS assimp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_assimp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_assimp`](../../tests/src/) -- a program linked against assimp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - OBJ cube imports
   - OBJ: one mesh
   - OBJ: 6 quads triangulated to 12 faces
   - OBJ: 8 vertices after joining identical ones
   - OBJ: x spans [-1, 1]
   - STL tetrahedron imports
   - STL: 4 triangles
   - STL: 12 vertices (corners differ by normal)
   - STL: first facet normal is -z
   - export the cube to ASCII STL
   - ASCII STL has 12 facets
   - export the cube to binary STL
   - binary STL is 84 + 12 x 50 bytes

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> assimp`.

## Versions

This recipe builds **6.0.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add assimp=<version>`), sources in `ported/assimp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates assimp`; port one with `scripts/port-update assimp`.

<details><summary>recipe.sh</summary>

```bash
# assimp: Open Asset Import Library -- loads 3D model formats (COLLADA, STL,
# OBJ, glTF, FBX, ...) into one scene structure. Needed by RViz (ros2-rviz)
# for robot meshes.
# Built against this tree's zlib (ASSIMP_BUILD_ZLIB off) rather than its copy.
# ASSIMP_WARNINGS_AS_ERRORS off: GCC 12's -Wstringop-truncation fires on
# assimp's own strncpy calls (EmbedTexturesProcess.cpp), which are bounded
# correctly. The `assimp` command-line tool is built too (info, export).
VERSION=6.0.2; GIT=https://github.com/assimp/assimp.git; REF=v6.0.2
DEPS="zlib"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_SAMPLES=OFF
                   -DASSIMP_BUILD_ASSIMP_TOOLS=ON -DASSIMP_BUILD_ZLIB=OFF
                   -DASSIMP_WARNINGS_AS_ERRORS=OFF -DASSIMP_INSTALL_PDB=OFF
                   -DCMAKE_INSTALL_LIBDIR=lib )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
