# gloo 0_git20261104

> Gloo collective communications library (allreduce, broadcast, barrier) with the TCP transport

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/facebookincubator/gloo.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gloo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gloo` (programs, shared libraries, run-time data) and `gloo-dev` (headers, pkg-config/CMake files, static libraries; needs `gloo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gloo`](https://openqnx.github.io/oss/package/x86_64/gloo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gloo.html)) · [`gloo-dev`](https://openqnx.github.io/oss/package/x86_64/gloo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gloo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gloo` (dependencies come along); one version: `apk add gloo=<version>` | `apk del gloo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gloo` (dependencies first) | `oss-get remove gloo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gloo-dev` the same way, by its own name (`apk add gloo-dev`, `oss-get install gloo-dev`, by hand with its own file and list `gloo-dev.list`); remove it before `gloo`, which it needs.

```sh
# on the QNX target, as root
apk add gloo gloo-dev
apk add qnx-test-gloo && /usr/local/bin/qnx-port-tests/test_gloo    # its on-target test, from tests-8.0-<arch>
apk del gloo gloo-dev qnx-test-gloo
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gloo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gloo.list
rm var/lib/manual/gloo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gloo` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/facebookincubator/gloo.git src/gloo
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gloo < ports/gloo/patches/0001-qnx-tcp-transport.patch
```

**[`0001-qnx-tcp-transport.patch`](patches/0001-qnx-tcp-transport.patch)** — Build the TCP transport on QNX (epoll in libepoll, EPOLLONESHOT emulated)

> Gloo compiles its TCP transport on Linux only, for epoll(2). QNX 8 has
> epoll in libepoll (and timerfd, pipe2), so build it there too and link
> libepoll and libsocket. What QNX's epoll lacks is EPOLLONESHOT, which the
> transport's one-shot read/write/connect helpers register with: emulate it
> in Loop (a one-shot descriptor is taken out of the epoll set when its event
> is reported, before its handler runs; unregistering one that already fired
> is not an error). Without that, level-triggered events would keep calling a
> helper that has finished. Also: HOST_NAME_MAX is _POSIX_HOST_NAME_MAX
> there, the debug trace prints pthread_self() for the gettid syscall, and
> thread names are set with pthread_setname_np. After qnx-ports/gloo's
> qnx-mediapipe branch, which defined EPOLLONESHOT as 0 instead.
> QNX's <sys/timerfd.h> does not declare timerfd_create/settime extern "C"
> (the timer referenced C++-mangled names): include it inside extern "C" and
> link libtimerfd. The interface's link speed and PCI bus ID are read from
> Linux's ethtool ioctl and sysfs (common/linux.cc, not built here): unknown
> (-1, "") on QNX.

Touches: `gloo/CMakeLists.txt`, `gloo/common/utils.cc`, `gloo/transport/tcp/buffer.cc`, `gloo/transport/tcp/CMakeLists.txt`, `gloo/transport/tcp/device.cc`, `gloo/transport/tcp/loop.cc` … · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/gloo -B build/$QNX_ARCH/gloo -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TEST=OFF \
      -DBUILD_BENCHMARK=OFF \
      -DUSE_REDIS=OFF \
      -DUSE_IBVERBS=OFF \
      -DUSE_MPI=OFF \
      -DUSE_LIBUV=OFF
cmake --build build/$QNX_ARCH/gloo -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/gloo
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Gloo: collective communications for machine learning (allreduce, broadcast, barrier, allgather, ... across processes), PyTorch's CPU distributed backend. Built with its TCP transport (epoll; QNX has it in libepoll -- QNX changes in patches/, after qnx-ports' qnx-mediapipe branch) and the file/prefix/hash rendezvous stores; no CUDA, ROCm, ibverbs, Redis or MPI. Upstream has no releases: pinned to a commit of main.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gloo`.

### 5. What it installs

65 files under `/usr/local`: 1 shared-library entries, 60 headers.

```
/usr/local/lib/libgloo.so
```

The complete list is `stage/<arch>/.manifest/gloo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gloo
ls dists/8.0/*/ | grep '^gloo-'
```

This writes `gloo-<ver>-r0.tar.gz` (run time) and `gloo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gloo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gloo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gloo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gloo`](../../tests/src/) links gloo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gloo`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gloo
/usr/local/bin/qnx-port-tests/test_gloo
```

```
== gloo ==
  ok   ...
PASS gloo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gloo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gloo`](../../tests/src/) -- a program linked against gloo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - all 4 ranks passed the barrier

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gloo 0_git20261104-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/gloo-0_git20261104-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gloo 0_git20261104-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/gloo-0_git20261104-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_git20261104**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gloo=<version>`), sources in `ported/gloo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gloo`; port one with `scripts/port-update gloo`.

<details><summary>recipe.sh</summary>

```bash
# Gloo: collective communications for machine learning (allreduce,
# broadcast, barrier, allgather, ... across processes), PyTorch's CPU
# distributed backend. Built with its TCP transport (epoll; QNX has it in
# libepoll -- QNX changes in patches/, after qnx-ports' qnx-mediapipe branch)
# and the file/prefix/hash rendezvous stores; no CUDA, ROCm, ibverbs, Redis
# or MPI. Upstream has no releases: pinned to a commit of main.
VERSION=0_git20261104; GIT=https://github.com/facebookincubator/gloo.git; COMMIT=8d8f8645ed5ef28aeca751bf17119efab5de2244
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_TEST=OFF -DBUILD_BENCHMARK=OFF -DUSE_REDIS=OFF -DUSE_IBVERBS=OFF -DUSE_MPI=OFF -DUSE_LIBUV=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
