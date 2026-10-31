# g3log 2.4

> Asynchronous crash-safe C++ logging library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/KjellKod/g3log.git (tag `2.4`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_g3log`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add g3log
apk add g3log-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-g3log && /usr/local/bin/qnx-port-tests/test_g3log    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install g3log`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild g3log` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 2.4 --recurse-submodules --shallow-submodules \
    https://github.com/KjellKod/g3log.git src/g3log
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/g3log < ports/g3log/patches/0001-qnx-stackdump-libbacktrace.patch
patch -p1 -d src/g3log < ports/g3log/patches/0002-qnx-dlopen-libbacktrace.patch
```

**[`0001-qnx-stackdump-libbacktrace.patch`](patches/0001-qnx-stackdump-libbacktrace.patch)** — Produce stack dumps on QNX with QNX's libbacktrace.

> g3log's crash handler uses glibc's backtrace()/backtrace_symbols() from
> <execinfo.h>, and Build.cmake requires CMake's FindBacktrace, which finds
> nothing on QNX: configure stops with Backtrace_INCLUDE_DIR NOTFOUND. QNX has
> its own stack walker, libbacktrace (<backtrace.h>, bt_get_backtrace and
> bt_sprnf_addrs). Use it on QNX, so a fatal signal still logs one line per
> frame (address, object, offset), and link -lbacktrace instead of looking for
> execinfo.

Touches: `Build.cmake`, `src/crashhandler_unix.cpp` · Upstream status: QNX-specific

**[`0002-qnx-dlopen-libbacktrace.patch`](patches/0002-qnx-dlopen-libbacktrace.patch)** — Open QNX's libbacktrace at dump time instead of linking it.

> patches/0001 linked libg3log.so against libbacktrace.so.1, which needs
> libunwind-nto.so.0. That library is part of the SDP but missing from many
> QNX target images, where every program using g3log then failed to start
> ("Could not load library libunwind-nto.so.0") -- for a library used only to
> print a stack dump on a crash. dlopen() it when a dump is wanted; without
> it the dump is empty and logging works as before.

Touches: `Build.cmake`, `src/crashhandler_unix.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/g3log -B build/$QNX_ARCH/g3log -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DADD_G3LOG_UNIT_TEST=OFF \
      -DADD_FATAL_EXAMPLE=OFF \
      -DG3_SHARED_LIB=ON \
      -DCHANGE_G3LOG_DEBUG_TO_DBUG=OFF
cmake --build build/$QNX_ARCH/g3log -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/g3log
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH g3log`.

### 5. What it installs

25 files under `/usr/local`: 3 shared-library entries, 19 headers.

```
/usr/local/lib/libg3log.so
/usr/local/lib/libg3log.so.2
/usr/local/lib/libg3log.so.2.4-0
```

The complete list is `stage/<arch>/.manifest/g3log` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa g3log
ls dists/8.0/*/ | grep '^g3log-'
```

This writes `g3log-<ver>-r0.tar.gz` (run time) and `g3log-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add g3log
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/g3log-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/g3log-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_g3log`](../../tests/src/) links g3log the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-g3log`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-g3log
/usr/local/bin/qnx-port-tests/test_g3log
```

```
== g3log ==
  ok   ...
PASS g3log
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_g3log` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_g3log`](../../tests/src/) -- a program linked against g3log the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - logging initialised with two sinks
   - 103 records delivered (LOG_IF false suppressed)
   - stream message and level
   - file and line captured
   - printf-style LOGF
   - LOG_IF(true) logged at DEBUG
   - 100 messages arrive in order
   - file sink wrote the messages
   - suppressed message absent from the file

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | g3log 2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/g3log-2.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | g3log 2.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/g3log-2.4-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | g3log 2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/g3log-2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | g3log 2.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/g3log-2.4-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add g3log=<version>`), sources in `ported/g3log/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates g3log`; port one with `scripts/port-update g3log`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.4; GIT=https://github.com/KjellKod/g3log.git; REF=2.4
CMAKE_ARGS_EXTRA=( -DADD_G3LOG_UNIT_TEST=OFF -DADD_FATAL_EXAMPLE=OFF -DG3_SHARED_LIB=ON -DCHANGE_G3LOG_DEBUG_TO_DBUG=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
