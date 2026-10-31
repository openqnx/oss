# capnproto 1.0.2

> Cap'n Proto data serialization format and capability-based RPC system

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/capnproto/capnproto.git (tag `v1.0.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_capnproto`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add capnproto
apk add capnproto-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-capnproto && /usr/local/bin/qnx-port-tests/test_capnproto    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install capnproto`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capnproto` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.0.2 --recurse-submodules --shallow-submodules \
    https://github.com/capnproto/capnproto.git src/capnproto
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/capnproto < ports/capnproto/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add INSTALL_TESTS option

> Imported from qnx-ports/capnproto (v1.0.2...qnx-1.0.2).
>
> ---
>  c++/CMakeLists.txt           | 1 +
>  c++/src/capnp/CMakeLists.txt | 5 +++++
>  c++/src/kj/CMakeLists.txt    | 7 +++++++
>  3 files changed, 13 insertions(+)

Touches: `c++/CMakeLists.txt`, `c++/src/capnp/CMakeLists.txt`, `c++/src/kj/CMakeLists.txt`, `c++/CMakeLists.txt`, `c++/src/CMakeLists.txt`, `c++/src/kj/CMakeLists.txt` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capnproto -B build/$QNX_ARCH/capnproto -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_PROJECT_INCLUDE="$RECIPE_DIR/capnproto/files/qnx-cxx-source-language.cmake" \
      -DBUILD_SHARED_LIBS=ON \
      -DWITH_OPENSSL=ON \
      -DWITH_ZLIB=ON \
      -DWITH_FIBERS=OFF
cmake --build build/$QNX_ARCH/capnproto -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capnproto
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Cap'n Proto names its C++ sources *.c++. qcc/q++ do not recognise that extension and silently produce no object at all (the link then fails with "cannot find .../array.c++.o"). files/qnx-cxx-source-language.cmake makes the language explicit in the compile rule only, so link lines are unchanged.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capnproto`.

### 5. What it installs

134 files under `/usr/local`: 22 shared-library entries, 4 programs, 92 headers.

```
/usr/local/lib/libcapnp-json.so
/usr/local/lib/libcapnp-json.so.1.0.2
/usr/local/lib/libcapnp-rpc.so
/usr/local/lib/libcapnp-rpc.so.1.0.2
/usr/local/lib/libcapnp-websocket.so
/usr/local/lib/libcapnp-websocket.so.1.0.2
/usr/local/lib/libcapnp.so
/usr/local/lib/libcapnp.so.1.0.2
/usr/local/lib/libcapnpc.so
/usr/local/lib/libcapnpc.so.1.0.2
/usr/local/lib/libkj-async.so
/usr/local/lib/libkj-async.so.1.0.2
/usr/local/lib/libkj-gzip.so
/usr/local/lib/libkj-gzip.so.1.0.2
/usr/local/lib/libkj-http.so
/usr/local/lib/libkj-http.so.1.0.2
...
```

The complete list is `stage/<arch>/.manifest/capnproto` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa capnproto
ls dists/8.0/*/ | grep '^capnproto-'
```

This writes `capnproto-<ver>-r0.tar.gz` (run time) and `capnproto-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add capnproto
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/capnproto-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/capnproto-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_capnproto`](../../tests/src/) links capnproto the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-capnproto`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-capnproto
/usr/local/bin/qnx-port-tests/test_capnproto
```

```
== capnproto ==
  ok   ...
PASS capnproto
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_capnproto` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_capnproto`](../../tests/src/) -- a program linked against capnproto the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capnproto 1.0.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/capnproto-1.0.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | capnproto 1.0.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/capnproto-1.0.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capnproto 1.0.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/capnproto-1.0.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | capnproto 1.0.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/capnproto-1.0.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add capnproto=<version>`), sources in `ported/capnproto/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates capnproto`; port one with `scripts/port-update capnproto`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.2; GIT=https://github.com/capnproto/capnproto.git; REF=v1.0.2
DEPS="zlib openssl"
# Cap'n Proto names its C++ sources *.c++. qcc/q++ do not recognise that
# extension and silently produce no object at all (the link then fails with
# "cannot find .../array.c++.o"). files/qnx-cxx-source-language.cmake makes
# the language explicit in the compile rule only, so link lines are unchanged.
CMAKE_ARGS_EXTRA=( -DCMAKE_PROJECT_INCLUDE="$RECIPE_DIR/capnproto/files/qnx-cxx-source-language.cmake" -DBUILD_SHARED_LIBS=ON -DWITH_OPENSSL=ON -DWITH_ZLIB=ON -DWITH_FIBERS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
