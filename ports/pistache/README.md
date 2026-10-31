# pistache 0.4.26

> C++ REST framework for HTTP servers and clients

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pistacheio/pistache.git (tag `v0.4.26`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`libevent`](../libevent/) [`rapidjson`](../rapidjson/) [`openssl`](../openssl/) |
| Run-time dependencies | [`libevent`](../libevent/) [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pistache`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pistache
apk add pistache-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pistache && /usr/local/bin/qnx-port-tests/test_pistache    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pistache`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pistache` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libevent rapidjson openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.4.26 --recurse-submodules --shallow-submodules \
    https://github.com/pistacheio/pistache.git src/pistache
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pistache < ports/pistache/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — pistache on QNX, from QNX's port (github.com/qnx-ports/pistache, branch

> qnx-v0.4.26): pistache's event loop runs on libevent where there is no
> Linux epoll/eventfd/timerfd interface of its own to use (the PISTACHE_USE_
> CONTENT_LIBEVENT path, extended to QNX), and QNX branches for errno values,
> strerror_r and sendfile.

Touches: `include/pistache/emosandlibevdefs.h`, `include/pistache/pist_strerror_r.h`, `include/pistache/ps_sendfile.h`, `include/pistache/pst_errno.h`, `include/pistache/winornix.h`, `meson.build` … · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/pistache src/pistache \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -DPISTACHE_BUILD_TESTS=false \
      -DPISTACHE_BUILD_EXAMPLES=false \
      -DPISTACHE_BUILD_DOCS=false \
      -DPISTACHE_USE_SSL=true \
      -DPISTACHE_USE_RAPIDJSON=true
ninja -C build/$QNX_ARCH/pistache
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/pistache install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pistache: a C++17 REST/HTTP server and client framework, with QNX's port (patches/0001, from github.com/qnx-ports/pistache: its event loop on libevent).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pistache`.

### 5. What it installs

64 files under `/usr/local`: 3 shared-library entries, 59 headers.

```
/usr/local/lib/libpistache.so
/usr/local/lib/libpistache.so.0.4
/usr/local/lib/libpistache.so.0.4.26
```

The complete list is `stage/<arch>/.manifest/pistache` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pistache
ls dists/8.0/*/ | grep '^pistache-'
```

This writes `pistache-<ver>-r0.tar.gz` (run time) and `pistache-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pistache
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pistache-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pistache-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pistache`](../../tests/src/) links pistache the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pistache`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pistache
/usr/local/bin/qnx-port-tests/test_pistache
```

```
== pistache ==
  ok   ...
PASS pistache
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pistache` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pistache`](../../tests/src/) -- a program linked against pistache the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - endpoint serving on the loopback
   - GET /hello: 200
   - body: world
   - GET /qnx: 404 with the resource
   - endpoint shut down

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pistache 0.4.26-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/pistache-0.4.26-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pistache 0.4.26-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/pistache-0.4.26-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.4.26**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pistache=<version>`), sources in `ported/pistache/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pistache`; port one with `scripts/port-update pistache`.

<details><summary>recipe.sh</summary>

```bash
# pistache: a C++17 REST/HTTP server and client framework, with QNX's port
# (patches/0001, from github.com/qnx-ports/pistache: its event loop on
# libevent).
VERSION=0.4.26; GIT=https://github.com/pistacheio/pistache.git; REF=v0.4.26
DEPS="libevent rapidjson openssl"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -DPISTACHE_BUILD_TESTS=false -DPISTACHE_BUILD_EXAMPLES=false
                   -DPISTACHE_BUILD_DOCS=false -DPISTACHE_USE_SSL=true -DPISTACHE_USE_RAPIDJSON=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
