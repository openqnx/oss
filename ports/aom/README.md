# aom 3.12.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://aomedia.googlesource.com/aom (tag `v3.12.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aom`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aom
apk add aom-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aom && /usr/local/bin/qnx-port-tests/test_aom    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aom`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aom` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v3.12.1 --recurse-submodules --shallow-submodules \
    https://aomedia.googlesource.com/aom src/aom
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aom -B build/$QNX_ARCH/aom -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_TESTS=OFF \
      -DENABLE_EXAMPLES=OFF \
      -DENABLE_TOOLS=OFF \
      -DENABLE_DOCS=OFF \
      -DCONFIG_PIC=1 \
      -DENABLE_NASM=ON \
      -DCONFIG_RUNTIME_CPU_DETECT=0 \
      -DAOM_TARGET_CPU=$( \
      [ \
      $QNX_ARCH \
      = \
      aarch64le \
      ] \
      && \
      echo \
      arm64 \
      || \
      echo \
      x86_64 \
      )
cmake --build build/$QNX_ARCH/aom -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aom
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Without runtime CPU detection (aom has none for QNX) every Arm extension that is compiled in is called unconditionally; keep to ARMv8.0 + NEON, which every aarch64 part has. With them, encoding died with SIGILL on a Cortex-A57.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aom`.

### 5. What it installs

15 files under `/usr/local`: 3 shared-library entries, 10 headers.

```
/usr/local/lib/libaom.so
/usr/local/lib/libaom.so.3
/usr/local/lib/libaom.so.3.12.1
```

The complete list is `stage/<arch>/.manifest/aom` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aom
ls dists/8.0/*/ | grep '^aom-'
```

This writes `aom-<ver>-r0.tar.gz` (run time) and `aom-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aom
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aom-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aom-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aom`](../../tests/src/) links aom the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aom`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aom
/usr/local/bin/qnx-port-tests/test_aom
```

```
== aom ==
  ok   ...
PASS aom
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aom` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aom`](../../tests/src/) -- a program linked against aom the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - enc_config_default
   - aom_codec_enc_init
   - aom_codec_dec_init
   - aom_img_alloc
   - compressed packets produced
   - at least one frame decoded back at the right size

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aom 3.12.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aom 3.12.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.12.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aom=<version>`), sources in `ported/aom/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aom`; port one with `scripts/port-update aom`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.12.1; GIT=https://aomedia.googlesource.com/aom; REF=v3.12.1
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF
                   -DENABLE_TOOLS=OFF -DENABLE_DOCS=OFF -DCONFIG_PIC=1
                   -DENABLE_NASM=ON -DCONFIG_RUNTIME_CPU_DETECT=0 -DAOM_TARGET_CPU=$( [ "$QNX_ARCH" = aarch64le ] && echo arm64 || echo x86_64 ) )
# Without runtime CPU detection (aom has none for QNX) every Arm extension that
# is compiled in is called unconditionally; keep to ARMv8.0 + NEON, which every
# aarch64 part has. With them, encoding died with SIGILL on a Cortex-A57.
[ "$QNX_ARCH" = aarch64le ] && CMAKE_ARGS_EXTRA+=( -DENABLE_NEON_DOTPROD=0 -DENABLE_NEON_I8MM=0 -DENABLE_SVE=0 -DENABLE_SVE2=0 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
