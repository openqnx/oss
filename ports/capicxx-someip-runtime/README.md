# capicxx-someip-runtime 3.2.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/capicxx-someip-runtime.git (tag `3.2.4`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`capicxx-core-runtime`](../capicxx-core-runtime/) [`vsomeip`](../vsomeip/) |
| Run-time dependencies | [`capicxx-core-runtime`](../capicxx-core-runtime/) [`vsomeip`](../vsomeip/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install capicxx-someip-runtime
oss-get install capicxx-someip-runtime-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild capicxx-someip-runtime` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (10 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild capicxx-core-runtime vsomeip
```

<details><summary>Full build chain (10)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`capicxx-core-runtime`](../capicxx-core-runtime/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`vsomeip`](../vsomeip/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.2.4 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/capicxx-someip-runtime.git src/capicxx-someip-runtime
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/capicxx-someip-runtime < ports/capicxx-someip-runtime/patches/0001-qnx-bsd-endian-macros.patch
patch -p1 -d src/capicxx-someip-runtime < ports/capicxx-someip-runtime/patches/0002-qnx-signed-uid-narrowing.patch
```

**[`0001-qnx-bsd-endian-macros.patch`](patches/0001-qnx-bsd-endian-macros.patch)** — CommonAPI SOME/IP: byte-order macros on QNX.

> The streams test glibc's __BYTE_ORDER/__LITTLE_ENDIAN; QNX has only the BSD
> names from <sys/param.h>. Alias them on QNX.

Touches: `include/CommonAPI/SomeIP/OutputStream.hpp`, `include/CommonAPI/SomeIP/InputStream.hpp`, `src/CommonAPI/SomeIP/StringEncoder.cpp` · Upstream status: QNX-specific

**[`0002-qnx-signed-uid-narrowing.patch`](patches/0002-qnx-signed-uid-narrowing.patch)** — CommonAPI SOME/IP: write "no uid/gid" as (uid_t)-1.

> uid_t and gid_t are signed on QNX, so the brace-initialisers 0xffffffff are
> narrowing conversions, which C++ rejects. (uid_t)-1 is the same value where
> the type is unsigned.

Touches: `src/CommonAPI/SomeIP/ClientId.cpp` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/capicxx-someip-runtime -B build/$QNX_ARCH/capicxx-someip-runtime -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DUSE_INSTALLED_COMMONAPI=ON \
      -DCommonAPI_DIR="$STAGE$PREFIX/lib/cmake/CommonAPI" \
      -Dvsomeip3_DIR="$STAGE$PREFIX/lib/cmake/vsomeip3"
cmake --build build/$QNX_ARCH/capicxx-someip-runtime -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/capicxx-someip-runtime
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH capicxx-someip-runtime`.

### 5. What it installs

41 files under `/usr/local`: 2 shared-library entries, 34 headers.

```
/usr/local/lib/libCommonAPI-SomeIP.so
/usr/local/lib/libCommonAPI-SomeIP.so.3.2.4
```

The complete list is `stage/<arch>/.manifest/capicxx-someip-runtime` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH capicxx-someip-runtime
ls repo/8.0/*/ | grep '^capicxx-someip-runtime-'
```

This writes `capicxx-someip-runtime-<ver>-r0.tar.gz` (run time) and `capicxx-someip-runtime-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install capicxx-someip-runtime
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/capicxx-someip-runtime-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/capicxx-someip-runtime-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.2.4; GIT=https://github.com/COVESA/capicxx-someip-runtime.git; REF=3.2.4
DEPS="capicxx-core-runtime vsomeip"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DUSE_INSTALLED_COMMONAPI=ON
                   -DCommonAPI_DIR="$STAGE$PREFIX/lib/cmake/CommonAPI"
                   -Dvsomeip3_DIR="$STAGE$PREFIX/lib/cmake/vsomeip3" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
