# fdk-aac 2.0.3

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/mstorsjo/fdk-aac.git (tag `v2.0.3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fdk_aac`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fdk-aac
apk add fdk-aac-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-fdk-aac && /usr/local/bin/qnx-port-tests/test_fdk_aac    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fdk-aac`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fdk-aac` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.0.3 --recurse-submodules --shallow-submodules \
    https://github.com/mstorsjo/fdk-aac.git src/fdk-aac
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fdk-aac -B build/$QNX_ARCH/fdk-aac -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_PROGRAMS=OFF
cmake --build build/$QNX_ARCH/fdk-aac -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fdk-aac
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fdk-aac`.

### 5. What it installs

14 files under `/usr/local`: 3 shared-library entries, 6 headers.

```
/usr/local/lib/libfdk-aac.so
/usr/local/lib/libfdk-aac.so.2
/usr/local/lib/libfdk-aac.so.2.0.3
```

The complete list is `stage/<arch>/.manifest/fdk-aac` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fdk-aac
ls dists/8.0/*/ | grep '^fdk-aac-'
```

This writes `fdk-aac-<ver>-r0.tar.gz` (run time) and `fdk-aac-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fdk-aac
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fdk-aac-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fdk-aac-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fdk_aac`](../../tests/src/) links fdk-aac the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fdk-aac`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fdk-aac
/usr/local/bin/qnx-port-tests/test_fdk_aac
```

```
== fdk-aac ==
  ok   ...
PASS fdk-aac
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fdk_aac` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fdk_aac`](../../tests/src/) -- a program linked against fdk-aac the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - aacEncOpen
   - set AOT
   - encoder init pass
   - aacEncInfo
   - aacDecoder_Open
   - ADTS packets produced
   - at least one frame decoded back
   - decoded stream reports 44100 Hz

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fdk-aac 2.0.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/fdk-aac-2.0.3-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fdk-aac 2.0.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/fdk-aac-2.0.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fdk-aac=<version>`), sources in `ported/fdk-aac/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fdk-aac`; port one with `scripts/port-update fdk-aac`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.0.3; GIT=https://github.com/mstorsjo/fdk-aac.git; REF=v2.0.3
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_PROGRAMS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
