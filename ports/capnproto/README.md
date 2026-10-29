# capnproto 1.0.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/capnproto/capnproto.git (tag `v1.0.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install capnproto
oss-get install capnproto-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH capnproto
ls repo/8.0/*/ | grep '^capnproto-'
```

This writes `capnproto-<ver>-r0.tar.gz` (run time) and `capnproto-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install capnproto
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/capnproto-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/capnproto-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
