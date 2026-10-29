# gtsam 4.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/borglab/gtsam.git (tag `4.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`boost`](../boost/) [`eigen`](../eigen/) |
| Run-time dependencies | [`boost`](../boost/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gtsam
apk add gtsam-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gtsam`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtsam` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild boost eigen
```

<details><summary>Full build chain (9)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`eigen`](../eigen/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 4.2 --recurse-submodules --shallow-submodules \
    https://github.com/borglab/gtsam.git src/gtsam
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/gtsam -B build/$QNX_ARCH/gtsam -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DGTSAM_BUILD_TESTS=OFF \
      -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF \
      -DGTSAM_BUILD_PYTHON=OFF \
      -DGTSAM_WITH_TBB=OFF \
      -DGTSAM_USE_SYSTEM_EIGEN=ON \
      -DGTSAM_BUILD_UNSTABLE=OFF \
      -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF \
      -DBoost_NO_BOOST_CMAKE=OFF \
      -DBOOST_ROOT="$STAGE$PREFIX"
cmake --build build/$QNX_ARCH/gtsam -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/gtsam
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtsam`.

### 5. What it installs

360 files under `/usr/local`: 4 shared-library entries, 341 headers.

```
/usr/local/lib/libgtsam.so
/usr/local/lib/libgtsam.so.4
/usr/local/lib/libgtsam.so.4.2.0
/usr/local/lib/libmetis-gtsam.so
```

The complete list is `stage/<arch>/.manifest/gtsam` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtsam
ls dists/8.0/*/ | grep '^gtsam-'
```

This writes `gtsam-<ver>-r0.tar.gz` (run time) and `gtsam-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtsam
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtsam-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtsam-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **4.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtsam=<version>`), sources in `ported/gtsam/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtsam`; port one with `scripts/port-update gtsam`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.2; GIT=https://github.com/borglab/gtsam.git; REF=4.2
DEPS="boost eigen"
CMAKE_ARGS_EXTRA=( -DGTSAM_BUILD_TESTS=OFF -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF -DGTSAM_BUILD_PYTHON=OFF
                   -DGTSAM_WITH_TBB=OFF -DGTSAM_USE_SYSTEM_EIGEN=ON -DGTSAM_BUILD_UNSTABLE=OFF
                   -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF -DBoost_NO_BOOST_CMAKE=OFF -DBOOST_ROOT="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
