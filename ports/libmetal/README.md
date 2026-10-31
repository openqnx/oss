# libmetal 2024.10.0

> Abstraction library for device access, interrupts and shared memory

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/OpenAMP/libmetal.git (tag `v2024.10.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libmetal`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libmetal
apk add libmetal-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libmetal && /usr/local/bin/qnx-port-tests/test_libmetal    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libmetal`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libmetal` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2024.10.0 --recurse-submodules --shallow-submodules \
    https://github.com/OpenAMP/libmetal.git src/libmetal
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libmetal < ports/libmetal/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/libmetal (f8a0e51aee8e...qnx-v2024.10.0); original authorship kept.
>
>
> Following API additions are made to support libmetal on QNX systems
>
> allocate and free memory
> assert statements
> caching support
> condition variables
> physical devices like static shared memory
> i/o configurations
> interrupt requests
> logs
> mutex
> dynamic shared memory (under /dev/shmem) with precise page-wise controls
> sleep
> time
> system and misc utilities
>
> Signed-off-by: Deep Chordia <dchordia@blackberry.com>
> ---
>  lib/CMakeLists.txt                  |   7 +-
>  lib/system/qnx/CMakeLists.txt       |  21 +++
>  lib/system/qnx/alloc.h              |  39 +++++
>  lib/system/qnx/assert.h             |  27 +++
>  lib/system/qnx/cache.h              |  45 +++++
>  lib/system/qnx/condition.c          |  19 +++
>  lib/system/qnx/condition.h          |  59 +++++++
>  lib/system/qnx/device.c             |  29 ++++
>  lib/system/qnx/init.c               |  77 +++++++++
>  lib/system/qnx/io.c                 |  45 +++++
>  lib/system/qnx/io.h                 |  43 +++++
>  lib/system/qnx/irq.c                | 256 ++++++++++++++++++++++++++++
>  lib/system/qnx/irq.h                |  45 +++++
>  lib/system/qnx/log.h                |  27 +++
>  lib/system/qnx/mutex.h              |  74 ++++++++
>  lib/system/qnx/shmem.c              | 103 +++++++++++
>  lib/system/qnx/sleep.h              |  40 +++++
>  lib/system/qnx/sys.h                |  86 ++++++++++
>  lib/system/qnx/time.c               |  30 ++++
>  lib/system/qnx/utilities.c          |  98 +++++++++++
>  test/CMakeLists.txt                 |   6 +-
>  test/system/qnx/CMakeLists.txt      |  13 ++
>  test/system/qnx/alloc.c             |  28 +++
>  test/system/qnx/atomic.c            |  47 +++++
>  test/system/qnx/condition.c         | 101 +++++++++++
>  test/system/qnx/irq.c               |  68 ++++++++
>  test/system/qnx/main.c              |  16 ++
>  test/system/qnx/mutex.c             |  44 +++++
>  test/system/qnx/rpi4/CMakeLists.txt |   1 +
>  test/system/qnx/rpi4/device.c       |  83 +++++++++
>  test/system/qnx/shmem.c             |  58 +++++++
>  test/system/qnx/spinlock.c          |  50 ++++++
>  test/system/qnx/threads.c           |  61 +++++++
>  33 files changed, 1744 insertions(+), 2 deletions(-)
>  create mode 100644 lib/system/qnx/CMakeLists.txt
>  create mode 100644 lib/system/qnx/alloc.h
>  create mode 100644 lib/system/qnx/assert.h
>  create mode 100644 lib/system/qnx/cache.h
>  create mode 100644 lib/system/qnx/condition.c
>  create mode 100644 lib/system/qnx/condition.h
>  create mode 100644 lib/system/qnx/device.c
>  create mode 100644 lib/system/qnx/init.c
>  create mode 100644 lib/system/qnx/io.c
>  create mode 100644 lib/system/qnx/io.h
>  create mode 100644 lib/system/qnx/irq.c
>  create mode 100644 lib/system/qnx/irq.h
>  create mode 100644 lib/system/qnx/log.h
>  create mode 100644 lib/system/qnx/mutex.h
>  create mode 100644 lib/system/qnx/shmem.c
>  create mode 100644 lib/system/qnx/sleep.h
>  create mode 100644 lib/system/qnx/sys.h
>  create mode 100644 lib/system/qnx/time.c
>  create mode 100644 lib/system/qnx/utilities.c
>  create mode 100644 test/system/qnx/CMakeLists.txt
>  create mode 100644 test/system/qnx/alloc.c
>  create mode 100644 test/system/qnx/atomic.c
>  create mode 100644 test/system/qnx/condition.c
>  create mode 100644 test/system/qnx/irq.c
>  create mode 100644 test/system/qnx/main.c
>  create mode 100644 test/system/qnx/mutex.c
>  create mode 100644 test/system/qnx/rpi4/CMakeLists.txt
>  create mode 100644 test/system/qnx/rpi4/device.c
>  create mode 100644 test/system/qnx/shmem.c
>  create mode 100644 test/system/qnx/spinlock.c
>  create mode 100644 test/system/qnx/threads.c

Touches: `lib/CMakeLists.txt`, `lib/system/qnx/CMakeLists.txt`, `lib/system/qnx/alloc.h`, `lib/system/qnx/assert.h`, `lib/system/qnx/cache.h`, `lib/system/qnx/condition.c` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libmetal -B build/$QNX_ARCH/libmetal -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_C_FLAGS=-isystem \
      $STAGE$PREFIX/include \
      -Wno-error=unused-parameter \
      -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_INSTALL_BINDIR=bin \
      -DWITH_DOC=OFF \
      -DWITH_EXAMPLES=OFF \
      -DWITH_TESTS=OFF \
      -DWITH_DEFAULT_LOGGER=ON
cmake --build build/$QNX_ARCH/libmetal -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libmetal
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libmetal builds with -Werror, and the QNX mutex stubs from the QNX patch leave parameters unused.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libmetal`.

### 5. What it installs

43 files under `/usr/local`, 42 headers.

The complete list is `stage/<arch>/.manifest/libmetal` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libmetal
ls dists/8.0/*/ | grep '^libmetal-'
```

This writes `libmetal-<ver>-r0.tar.gz` (run time) and `libmetal-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libmetal
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libmetal-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libmetal-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libmetal`](../../tests/src/) links libmetal the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libmetal`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libmetal
/usr/local/bin/qnx-port-tests/test_libmetal
```

```
== libmetal ==
  ok   ...
PASS libmetal
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libmetal` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libmetal`](../../tests/src/) -- a program linked against libmetal the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - metal_init (IRQ channel, cache control, generic bus)
   - I/O region of 4096 bytes
   - typed writes read back (8/16/32/64 bit)
   - a 32-bit write landed in memory
   - block_write
   - block_read gives it back
   - block_set fills exactly 32 bytes
   - offset <-> virtual address
   - offset <-> physical address
   - physical <-> virtual
   - out of range -> NULL / BAD_OFFSET
   - metal_shmem_open twice, 8 KiB
   - written through one mapping, read through the other
   - new list is empty
   - list: add_tail 1..4, delete 2 -> 1,3,4
   - list_find_node
   - bitmap set/test
   - bitmap next set/clear bit
   - log2, align_up, div_round_up
   - mutex and spinlock: 4 threads x 20000 increments each
   - condition variable woke the waiter for all 4
   - try_acquire on a free mutex
   - is_acquired while held
   - not is_acquired once released
   - is_acquired left a free mutex free (try_acquire succeeds)
   - ... and 3 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libmetal 2024.10.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libmetal-2024.10.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libmetal 2024.10.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libmetal-2024.10.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2024.10.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libmetal=<version>`), sources in `ported/libmetal/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libmetal`; port one with `scripts/port-update libmetal`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2024.10.0; GIT=https://github.com/OpenAMP/libmetal.git; REF=v2024.10.0
# libmetal builds with -Werror, and the QNX mutex stubs from the QNX patch
# leave parameters unused.
CMAKE_ARGS_EXTRA=( "-DCMAKE_C_FLAGS=-isystem $STAGE$PREFIX/include -Wno-error=unused-parameter" -DCMAKE_INSTALL_INCLUDEDIR=include -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_BINDIR=bin -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
