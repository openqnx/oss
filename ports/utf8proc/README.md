# utf8proc 2.11.3

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/JuliaStrings/utf8proc.git (tag `v2.11.3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_utf8proc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add utf8proc
apk add utf8proc-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-utf8proc && /usr/local/bin/qnx-port-tests/test_utf8proc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install utf8proc`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild utf8proc` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.11.3 --recurse-submodules --shallow-submodules \
    https://github.com/JuliaStrings/utf8proc.git src/utf8proc
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/utf8proc -B build/$QNX_ARCH/utf8proc -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DUTF8PROC_ENABLE_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/utf8proc -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/utf8proc
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- 2.11 for Neovim 0.12, which uses ambiguous_width (the version Neovim bundles).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH utf8proc`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libutf8proc.so
/usr/local/lib/libutf8proc.so.3
/usr/local/lib/libutf8proc.so.3.2.3
```

The complete list is `stage/<arch>/.manifest/utf8proc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa utf8proc
ls dists/8.0/*/ | grep '^utf8proc-'
```

This writes `utf8proc-<ver>-r0.tar.gz` (run time) and `utf8proc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add utf8proc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/utf8proc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/utf8proc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_utf8proc`](../../tests/src/) links utf8proc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-utf8proc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-utf8proc
/usr/local/bin/qnx-port-tests/test_utf8proc
```

```
== utf8proc ==
  ok   ...
PASS utf8proc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_utf8proc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_utf8proc`](../../tests/src/) -- a program linked against utf8proc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version string
   - decode first code point
   - NFC normalisation
   - NFC composed the diaeresis
   - sharp s is a lowercase letter
   - CJK ideograph is double width

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.11.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add utf8proc=<version>`), sources in `ported/utf8proc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates utf8proc`; port one with `scripts/port-update utf8proc`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.11.3; GIT=https://github.com/JuliaStrings/utf8proc.git; REF=v2.11.3
# 2.11 for Neovim 0.12, which uses ambiguous_width (the version Neovim bundles).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUTF8PROC_ENABLE_TESTING=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
