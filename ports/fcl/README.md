# fcl 0.7.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/flexible-collision-library/fcl.git (tag `0.7.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`eigen`](../eigen/) [`libccd`](../libccd/) [`octomap`](../octomap/) |
| Run-time dependencies | [`libccd`](../libccd/) [`octomap`](../octomap/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fcl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fcl
apk add fcl-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-fcl && /usr/local/bin/qnx-port-tests/test_fcl    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fcl`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fcl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild eigen libccd octomap
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.7.0 --recurse-submodules --shallow-submodules \
    https://github.com/flexible-collision-library/fcl.git src/fcl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fcl -B build/$QNX_ARCH/fcl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DFCL_WITH_OCTOMAP=ON \
      -DFCL_USE_X64_SSE=OFF \
      -DFCL_STATIC_LIBRARY=OFF
cmake --build build/$QNX_ARCH/fcl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fcl
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fcl`.

### 5. What it installs

323 files under `/usr/local`: 3 shared-library entries, 314 headers.

```
/usr/local/lib/libfcl.so
/usr/local/lib/libfcl.so.0.7
/usr/local/lib/libfcl.so.0.7.0
```

The complete list is `stage/<arch>/.manifest/fcl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fcl
ls dists/8.0/*/ | grep '^fcl-'
```

This writes `fcl-<ver>-r0.tar.gz` (run time) and `fcl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fcl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fcl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fcl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fcl`](../../tests/src/) links fcl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fcl`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fcl
/usr/local/bin/qnx-port-tests/test_fcl
```

```
== fcl ==
  ok   ...
PASS fcl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fcl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fcl`](../../tests/src/) -- a program linked against fcl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - overlapping boxes collide
   - distance is 2

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fcl 0.7.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/fcl-0.7.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libccd 2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libccd-2.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fcl 0.7.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/fcl-0.7.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libccd 2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libccd-2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fcl=<version>`), sources in `ported/fcl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fcl`; port one with `scripts/port-update fcl`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.7.0; GIT=https://github.com/flexible-collision-library/fcl.git; REF=0.7.0
DEPS="eigen libccd octomap"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DFCL_WITH_OCTOMAP=ON
                   -DFCL_USE_X64_SSE=OFF -DFCL_STATIC_LIBRARY=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
