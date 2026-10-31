# libwebsockets 4.3.5

> Lightweight C library for HTTP, WebSocket and MQTT clients and servers

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/warmcat/libwebsockets.git (tag `v4.3.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libwebsockets`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libwebsockets
apk add libwebsockets-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libwebsockets && /usr/local/bin/qnx-port-tests/test_libwebsockets    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libwebsockets`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libwebsockets` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v4.3.5 --recurse-submodules --shallow-submodules \
    https://github.com/warmcat/libwebsockets.git src/libwebsockets
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libwebsockets < ports/libwebsockets/patches/0001-qnx8-standard-tcp-keepalive.patch
```

**[`0001-qnx8-standard-tcp-keepalive.patch`](patches/0001-qnx8-standard-tcp-keepalive.patch)** — libwebsockets: standard TCP keepalive options on QNX 8.0.

> QNX 8.0 defines TCP_KEEPIDLE/TCP_KEEPINTVL/TCP_KEEPCNT in <netinet/tcp.h> and
> no longer the TCPCTL_* names older QNX had; apply the remap only before 8.0
> (qcc predefines __QNX__ as 700, 710 or 800).

Touches: `lib/plat/unix/private-lib-plat-unix.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libwebsockets -B build/$QNX_ARCH/libwebsockets -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DLWS_WITH_SHARED=ON \
      -DLWS_WITH_STATIC=ON \
      -DLWS_WITHOUT_TESTAPPS=ON \
      -DLWS_WITHOUT_TEST_SERVER=ON \
      -DLWS_WITH_MINIMAL_EXAMPLES=OFF \
      -DLWS_OPENSSL_LIBRARIES="$STAGE$PREFIX/lib/libssl.so;$STAGE$PREFIX/lib/libcrypto.so" \
      -DLWS_OPENSSL_INCLUDE_DIRS="$STAGE$PREFIX/include"
cmake --build build/$QNX_ARCH/libwebsockets -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libwebsockets
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libwebsockets`.

### 5. What it installs

94 files under `/usr/local`: 1 shared-library entries, 85 headers.

```
/usr/local/lib/libwebsockets.so
```

The complete list is `stage/<arch>/.manifest/libwebsockets` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libwebsockets
ls dists/8.0/*/ | grep '^libwebsockets-'
```

This writes `libwebsockets-<ver>-r0.tar.gz` (run time) and `libwebsockets-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libwebsockets
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libwebsockets-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libwebsockets-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libwebsockets`](../../tests/src/) links libwebsockets the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libwebsockets`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libwebsockets
/usr/local/bin/qnx-port-tests/test_libwebsockets
```

```
== libwebsockets ==
  ok   ...
PASS libwebsockets
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libwebsockets` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libwebsockets`](../../tests/src/) -- a program linked against libwebsockets the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - lws_create_context
   - lws_service pass
   - lws_context_destroy

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libwebsockets 4.3.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libwebsockets-4.3.5-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libwebsockets 4.3.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libwebsockets-4.3.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.3.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libwebsockets=<version>`), sources in `ported/libwebsockets/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libwebsockets`; port one with `scripts/port-update libwebsockets`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.3.5; GIT=https://github.com/warmcat/libwebsockets.git; REF=v4.3.5
DEPS="openssl zlib"
CMAKE_ARGS_EXTRA=( -DLWS_WITH_SHARED=ON -DLWS_WITH_STATIC=ON
                   -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITHOUT_TEST_SERVER=ON
                   -DLWS_WITH_MINIMAL_EXAMPLES=OFF
                   -DLWS_OPENSSL_LIBRARIES="$STAGE$PREFIX/lib/libssl.so;$STAGE$PREFIX/lib/libcrypto.so"
                   -DLWS_OPENSSL_INCLUDE_DIRS="$STAGE$PREFIX/include" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
