# c-ares 1.34.3

> Asynchronous DNS resolver library in C

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/c-ares/c-ares.git (tag `v1.34.3`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_c_ares`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add c-ares
apk add c-ares-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-c-ares && /usr/local/bin/qnx-port-tests/test_c_ares    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install c-ares`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild c-ares` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.34.3 --recurse-submodules --shallow-submodules \
    https://github.com/c-ares/c-ares.git src/c-ares
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/c-ares < ports/c-ares/patches/0001-qnx-no-epollrdhup.patch
```

**[`0001-qnx-no-epollrdhup.patch`](patches/0001-qnx-no-epollrdhup.patch)** — c-ares: build the epoll backend without EPOLLRDHUP.

> QNX 8.0's epoll (libepoll) has no EPOLLRDHUP. Defined as 0 the bit is never
> requested; c-ares still sees a closed connection through EPOLLHUP and a read
> of 0 bytes.

Touches: `src/lib/event/ares_event_epoll.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/c-ares -B build/$QNX_ARCH/c-ares -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCARES_SHARED=ON \
      -DCARES_STATIC=ON \
      -DCARES_BUILD_TESTS=OFF \
      -DCARES_BUILD_TOOLS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/c-ares -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/c-ares
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH c-ares`.

### 5. What it installs

166 files under `/usr/local`: 3 shared-library entries, 6 headers.

```
/usr/local/lib/libcares.so
/usr/local/lib/libcares.so.2
/usr/local/lib/libcares.so.2.19.2
```

The complete list is `stage/<arch>/.manifest/c-ares` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa c-ares
ls dists/8.0/*/ | grep '^c-ares-'
```

This writes `c-ares-<ver>-r0.tar.gz` (run time) and `c-ares-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add c-ares
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/c-ares-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/c-ares-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_c_ares`](../../tests/src/) links c-ares the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-c-ares`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-c-ares
/usr/local/bin/qnx-port-tests/test_c_ares
```

```
== c-ares ==
  ok   ...
PASS c-ares
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_c_ares` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_c_ares`](../../tests/src/) -- a program linked against c-ares the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ares_library_init
   - ares_init_options
   - ares_parse_a_reply
   - question name parsed
   - A record is 10.0.0.1
   - ares_strerror

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | c-ares 1.34.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/c-ares-1.34.3-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | c-ares 1.34.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/c-ares-1.34.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.34.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add c-ares=<version>`), sources in `ported/c-ares/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates c-ares`; port one with `scripts/port-update c-ares`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.34.3; GIT=https://github.com/c-ares/c-ares.git; REF=v1.34.3
CMAKE_ARGS_EXTRA=( -DCARES_SHARED=ON -DCARES_STATIC=ON -DCARES_BUILD_TESTS=OFF
                   -DCARES_BUILD_TOOLS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
