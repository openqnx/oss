# capicxx-core-runtime 3.2.4

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/COVESA/capicxx-core-runtime.git (tag `3.2.4`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add capicxx-core-runtime
apk add capicxx-core-runtime-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install capicxx-core-runtime`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capicxx-core-runtime` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.2.4 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/capicxx-core-runtime.git src/capicxx-core-runtime
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/capicxx-core-runtime < ports/capicxx-core-runtime/patches/0001-qnx-no-use-after-free-at-exit.patch
```

**[`0001-qnx-no-use-after-free-at-exit.patch`](patches/0001-qnx-no-use-after-free-at-exit.patch)** — CommonAPI: do not reset the runtime from a static destructor.

> The deinitializer resets and deletes the runtime at exit, when static objects
> it still refers to may already be destroyed -- a use-after-free during exit.
> Leave the runtime to process teardown.

Touches: `src/CommonAPI/Runtime.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capicxx-core-runtime -B build/$QNX_ARCH/capicxx-core-runtime -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/capicxx-core-runtime -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capicxx-core-runtime
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capicxx-core-runtime`.

### 5. What it installs

39 files under `/usr/local`: 2 shared-library entries, 32 headers.

```
/usr/local/lib/libCommonAPI.so
/usr/local/lib/libCommonAPI.so.3.2.4
```

The complete list is `stage/<arch>/.manifest/capicxx-core-runtime` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa capicxx-core-runtime
ls dists/8.0/*/ | grep '^capicxx-core-runtime-'
```

This writes `capicxx-core-runtime-<ver>-r0.tar.gz` (run time) and `capicxx-core-runtime-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add capicxx-core-runtime
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/capicxx-core-runtime-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/capicxx-core-runtime-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **3.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add capicxx-core-runtime=<version>`), sources in `ported/capicxx-core-runtime/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates capicxx-core-runtime`; port one with `scripts/port-update capicxx-core-runtime`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.2.4; GIT=https://github.com/COVESA/capicxx-core-runtime.git; REF=3.2.4
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
