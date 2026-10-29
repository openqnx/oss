# x265 3.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://bitbucket.org/multicoreware/x265_git.git (tag `3.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_x265`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add x265
apk add x265-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-x265 && /usr/local/bin/qnx-port-tests/test_x265    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install x265`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild x265` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.6 --recurse-submodules --shallow-submodules \
    https://bitbucket.org/multicoreware/x265_git.git src/x265
```

The project lives in the `source/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/x265 < ports/x265/patches/0001-cmake4-policies.patch
```

**[`0001-cmake4-policies.patch`](patches/0001-cmake4-policies.patch)** — x265: set CMP0025 and CMP0054 to NEW for CMake 4.

> CMake 4 removed the OLD behaviour of these policies and refuses to configure.
> CMP0025 only affects Apple's compiler name; NEW CMP0054 is the long-standing
> default.

Touches: `source/CMakeLists.txt` · Upstream status: Pending (not QNX-specific: CMake 4)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/x265/source -B build/$QNX_ARCH/x265 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_SHARED=ON \
      -DENABLE_CLI=OFF \
      -DENABLE_TESTS=OFF \
      -DENABLE_ASSEMBLY=OFF \
      # \
      nasm/yasm \
      absent \
      on \
      this \
      host \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/x265 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/x265
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH x265`.

### 5. What it installs

6 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libx265.so
/usr/local/lib/libx265.so.209
```

The complete list is `stage/<arch>/.manifest/x265` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa x265
ls dists/8.0/*/ | grep '^x265-'
```

This writes `x265-<ver>-r0.tar.gz` (run time) and `x265-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add x265
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/x265-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/x265-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_x265`](../../tests/src/) links x265 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-x265`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-x265
/usr/local/bin/qnx-port-tests/test_x265
```

```
== x265 ==
  ok   ...
PASS x265
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_x265` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_x265`](../../tests/src/) -- a program linked against x265 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - x265_api_get
   - param_alloc
   - encoder_open
   - all frames encoded
   - bitstream bytes produced

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add x265=<version>`), sources in `ported/x265/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates x265`; port one with `scripts/port-update x265`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.6; GIT=https://bitbucket.org/multicoreware/x265_git.git; REF=3.6
SUBDIR=source
CMAKE_ARGS_EXTRA=( -DENABLE_SHARED=ON -DENABLE_CLI=OFF -DENABLE_TESTS=OFF
                   -DENABLE_ASSEMBLY=OFF          # nasm/yasm absent on this host
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
