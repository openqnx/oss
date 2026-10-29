# libcoap 4.3.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/obgm/libcoap.git (tag `v4.3.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libcoap`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libcoap
oss-get install libcoap-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libcoap` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v4.3.5 --recurse-submodules --shallow-submodules \
    https://github.com/obgm/libcoap.git src/libcoap
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libcoap < ports/libcoap/patches/0001-qnx-epollrdhup.patch
```

**[`0001-qnx-epollrdhup.patch`](patches/0001-qnx-epollrdhup.patch)** — Define EPOLLRDHUP as 0 where the epoll shim lacks it.

> QNX 8 provides epoll through libepoll, whose <sys/epoll.h> has no EPOLLRDHUP.
> libcoap only ORs it into masks next to EPOLLHUP/EPOLLIN, which still report
> a peer that closed its end, so a zero value keeps the behaviour.

Touches: `src/coap_net.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libcoap -B build/$QNX_ARCH/libcoap -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_DTLS=ON \
      -DDTLS_BACKEND=openssl \
      -DENABLE_DOCS=OFF \
      -DENABLE_EXAMPLES=ON \
      -DENABLE_TESTS=OFF
cmake --build build/$QNX_ARCH/libcoap -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libcoap
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libcoap`.

### 5. What it installs

43 files under `/usr/local`: 3 shared-library entries, 6 programs, 30 headers.

```
/usr/local/lib/libcoap-3.so
/usr/local/lib/libcoap-3.so.3
/usr/local/lib/libcoap-3.so.3.2.0
/usr/local/bin/coap-client
/usr/local/bin/coap-rd
/usr/local/bin/coap-server
/usr/local/bin/etsi_iot_01
/usr/local/bin/oscore-interop-server
/usr/local/bin/tiny
```

The complete list is `stage/<arch>/.manifest/libcoap` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libcoap
ls repo/8.0/*/ | grep '^libcoap-'
```

This writes `libcoap-<ver>-r0.tar.gz` (run time) and `libcoap-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libcoap
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libcoap-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libcoap-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libcoap`](../../tests/src/) links libcoap the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libcoap root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libcoap'
```

```
== libcoap ==
  ok   ...
PASS libcoap
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.3.5; GIT=https://github.com/obgm/libcoap.git; REF=v4.3.5
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_DTLS=ON -DDTLS_BACKEND=openssl -DENABLE_DOCS=OFF
                   -DENABLE_EXAMPLES=ON -DENABLE_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
