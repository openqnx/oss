# mimalloc 3.1.5

> Compact general-purpose memory allocator from Microsoft

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/microsoft/mimalloc.git (tag `v3.1.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mimalloc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add mimalloc
apk add mimalloc-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-mimalloc && /usr/local/bin/qnx-port-tests/test_mimalloc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install mimalloc`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mimalloc` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v3.1.5 --recurse-submodules --shallow-submodules \
    https://github.com/microsoft/mimalloc.git src/mimalloc
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mimalloc < ports/mimalloc/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — mimalloc on QNX, from QNX's port (github.com/qnx-ports/mimalloc, branch

> qnx-v3.1.5): the thread-pointer TLS slot fast path only on QNX 8 (older QNX
> falls back to the TLS slot assembly), and cfree() with the int return type
> QNX before 8.0 declares.

Touches: `include/mimalloc/prim.h`, `src/alloc-override.c` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/mimalloc -B build/$QNX_ARCH/mimalloc -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DMI_BUILD_TESTS=OFF \
      -DMI_BUILD_OBJECT=OFF \
      -DMI_OVERRIDE=OFF
cmake --build build/$QNX_ARCH/mimalloc -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/mimalloc
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mimalloc: Microsoft's compact general-purpose allocator (link it, or preload it to replace malloc), with QNX's changes (patches/0001, from github.com/qnx-ports/mimalloc).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mimalloc`.

### 5. What it installs

13 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/libmimalloc.so
/usr/local/lib/libmimalloc.so.3
/usr/local/lib/libmimalloc.so.3.1
```

The complete list is `stage/<arch>/.manifest/mimalloc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mimalloc
ls dists/8.0/*/ | grep '^mimalloc-'
```

This writes `mimalloc-<ver>-r0.tar.gz` (run time) and `mimalloc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mimalloc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mimalloc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mimalloc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mimalloc`](../../tests/src/) links mimalloc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mimalloc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mimalloc
/usr/local/bin/qnx-port-tests/test_mimalloc
```

```
== mimalloc ==
  ok   ...
PASS mimalloc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mimalloc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mimalloc`](../../tests/src/) -- a program linked against mimalloc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mimalloc 3.1.5
   - mi_malloc 1 B .. 4 MB, usable size, written, freed
   - mi_malloc_aligned(4096)
   - mi_heap_new
   - mi_heap_destroy frees 10000 blocks at once
   - mi_strdup

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mimalloc 3.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/mimalloc-3.1.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mimalloc 3.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/mimalloc-3.1.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.1.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mimalloc=<version>`), sources in `ported/mimalloc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mimalloc`; port one with `scripts/port-update mimalloc`.

<details><summary>recipe.sh</summary>

```bash
# mimalloc: Microsoft's compact general-purpose allocator (link it, or preload
# it to replace malloc), with QNX's changes (patches/0001, from
# github.com/qnx-ports/mimalloc).
VERSION=3.1.5; GIT=https://github.com/microsoft/mimalloc.git; REF=v3.1.5
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DMI_BUILD_TESTS=OFF -DMI_BUILD_OBJECT=OFF -DMI_OVERRIDE=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
