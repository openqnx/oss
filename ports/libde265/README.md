# libde265 1.0.15

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/strukturag/libde265.git (tag `v1.0.15`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libde265
apk add libde265-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libde265`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libde265` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.0.15 --recurse-submodules --shallow-submodules \
    https://github.com/strukturag/libde265.git src/libde265
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libde265 -B build/$QNX_ARCH/libde265 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_SDL=OFF \
      -DENABLE_DECODER=ON \
      -DENABLE_ENCODER=OFF
cmake --build build/$QNX_ARCH/libde265 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libde265
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The other half of the HEVC pair: x265 (already staged) encodes, libde265 decodes.  No SDL/Qt example programs -- they pull in a display stack the library itself does not need.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libde265`.

### 5. What it installs

9 files under `/usr/local`: 1 shared-library entries, 1 programs, 3 headers.

```
/usr/local/lib/libde265.so
/usr/local/bin/dec265
```

The complete list is `stage/<arch>/.manifest/libde265` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libde265
ls dists/8.0/*/ | grep '^libde265-'
```

This writes `libde265-<ver>-r0.tar.gz` (run time) and `libde265-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libde265
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libde265-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libde265-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libde265 1.0.15-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.0.15**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libde265=<version>`), sources in `ported/libde265/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libde265`; port one with `scripts/port-update libde265`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.15; GIT=https://github.com/strukturag/libde265.git; REF=v1.0.15
BUILDSYS=cmake
# The other half of the HEVC pair: x265 (already staged) encodes, libde265
# decodes.  No SDL/Qt example programs -- they pull in a display stack the
# library itself does not need.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_SDL=OFF
                   -DENABLE_DECODER=ON -DENABLE_ENCODER=OFF )

# Note on the soname: upstream's CMake build sets no VERSION/SOVERSION on the
# target, so it installs an unversioned libde265.so with SONAME "libde265.so"
# -- where its autotools build produces libde265.so.0.  That is upstream's
# behaviour, not something this recipe chose, and it is left alone rather than
# invented here: a soname this tree makes up would differ from every other
# distribution's.  scripts/check-devlinks only scans lib*.so.* so it does not
# flag this; the library exports 773 symbols and links correctly.
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
