# libwebsockets 4.3.5

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

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libwebsockets
oss-get install libwebsockets-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH libwebsockets
ls repo/8.0/*/ | grep '^libwebsockets-'
```

This writes `libwebsockets-<ver>-r0.tar.gz` (run time) and `libwebsockets-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libwebsockets
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libwebsockets-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libwebsockets-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libwebsockets`](../../tests/src/) links libwebsockets the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libwebsockets root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libwebsockets'
```

```
== libwebsockets ==
  ok   ...
PASS libwebsockets
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
