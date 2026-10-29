# libevent 2.1.12

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libevent/libevent.git (tag `release-2.1.12-stable`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libevent`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libevent
oss-get install libevent-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libevent` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b release-2.1.12-stable --recurse-submodules --shallow-submodules \
    https://github.com/libevent/libevent.git src/libevent
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libevent < ports/libevent/patches/0001-qnx-sa-restart-and-epollet.patch
```

**[`0001-qnx-sa-restart-and-epollet.patch`](patches/0001-qnx-sa-restart-and-epollet.patch)** — libevent: QNX's epoll has no EPOLLET.

> QNX 8.0's libepoll is level-triggered only. Keep the epoll backend but stop
> advertising EV_FEATURE_ET, so a caller that needs edge-triggered behaviour
> gets another backend instead of silently level-triggered events.

Touches: `epoll.c`, `signal.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libevent -B build/$QNX_ARCH/libevent -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DEVENT__LIBRARY_TYPE=SHARED \
      -DEVENT__DISABLE_BENCHMARK=ON \
      -DEVENT__DISABLE_TESTS=ON \
      -DEVENT__DISABLE_SAMPLES=ON \
      -DEVENT__DISABLE_REGRESS=ON
cmake --build build/$QNX_ARCH/libevent -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libevent
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libevent`.

### 5. What it installs

61 files under `/usr/local`: 20 shared-library entries, 1 programs, 31 headers.

```
/usr/local/lib/libevent-2.1.so
/usr/local/lib/libevent-2.1.so.7
/usr/local/lib/libevent-2.1.so.7.0.1
/usr/local/lib/libevent.so
/usr/local/lib/libevent_core-2.1.so
/usr/local/lib/libevent_core-2.1.so.7
/usr/local/lib/libevent_core-2.1.so.7.0.1
/usr/local/lib/libevent_core.so
/usr/local/lib/libevent_extra-2.1.so
/usr/local/lib/libevent_extra-2.1.so.7
/usr/local/lib/libevent_extra-2.1.so.7.0.1
/usr/local/lib/libevent_extra.so
/usr/local/lib/libevent_openssl-2.1.so
/usr/local/lib/libevent_openssl-2.1.so.7
/usr/local/lib/libevent_openssl-2.1.so.7.0.1
/usr/local/lib/libevent_openssl.so
...
```

The complete list is `stage/<arch>/.manifest/libevent` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libevent
ls repo/8.0/*/ | grep '^libevent-'
```

This writes `libevent-<ver>-r0.tar.gz` (run time) and `libevent-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libevent
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libevent-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libevent-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libevent`](../../tests/src/) links libevent the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libevent root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libevent'
```

```
== libevent ==
  ok   ...
PASS libevent
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.1.12; GIT=https://github.com/libevent/libevent.git; REF=release-2.1.12-stable
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DEVENT__LIBRARY_TYPE=SHARED -DEVENT__DISABLE_BENCHMARK=ON
                   -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_SAMPLES=ON
                   -DEVENT__DISABLE_REGRESS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
