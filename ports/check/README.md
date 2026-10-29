# check 0.15.2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libcheck/check.git (tag `0.15.2`) |
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
apk add check
apk add check-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install check`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild check` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.15.2 --recurse-submodules --shallow-submodules \
    https://github.com/libcheck/check.git src/check
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/check -B build/$QNX_ARCH/check -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCHECK_ENABLE_TESTS=OFF \
      -DCHECK_ENABLE_TIMEOUT_TESTS=OFF
cmake --build build/$QNX_ARCH/check -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/check
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH check`.

### 5. What it installs

13 files under `/usr/local`: 3 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libcheck.so
/usr/local/lib/libcheck.so.0
/usr/local/lib/libcheck.so.0.15.2
/usr/local/bin/checkmk
```

The complete list is `stage/<arch>/.manifest/check` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa check
ls dists/8.0/*/ | grep '^check-'
```

This writes `check-<ver>-r0.tar.gz` (run time) and `check-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add check
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/check-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/check-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **0.15.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add check=<version>`), sources in `ported/check/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates check`; port one with `scripts/port-update check`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.15.2; GIT=https://github.com/libcheck/check.git; REF=0.15.2
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCHECK_ENABLE_TESTS=OFF -DCHECK_ENABLE_TIMEOUT_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
