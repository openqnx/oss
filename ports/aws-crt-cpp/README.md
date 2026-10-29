# aws-crt-cpp 0.38.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/awslabs/aws-crt-cpp.git (tag `v0.38.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-s3`](../aws-c-s3/) |
| Run-time dependencies | [`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-s3`](../aws-c-s3/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install aws-crt-cpp
oss-get install aws-crt-cpp-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aws-crt-cpp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (13 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3
```

<details><summary>Full build chain (13)</summary>

[`aws-c-auth`](../aws-c-auth/) [`aws-c-cal`](../aws-c-cal/) [`aws-c-common`](../aws-c-common/) [`aws-c-compression`](../aws-c-compression/) [`aws-c-event-stream`](../aws-c-event-stream/) [`aws-c-http`](../aws-c-http/) [`aws-c-io`](../aws-c-io/) [`aws-c-mqtt`](../aws-c-mqtt/) [`aws-c-s3`](../aws-c-s3/) [`aws-c-sdkutils`](../aws-c-sdkutils/) [`aws-checksums`](../aws-checksums/) [`openssl`](../openssl/) [`s2n-tls`](../s2n-tls/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.38.5 --recurse-submodules --shallow-submodules \
    https://github.com/awslabs/aws-crt-cpp.git src/aws-crt-cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/aws-crt-cpp < ports/aws-crt-cpp/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/aws-crt-cpp (v0.30.1...qnx-0.30.1).
>
> ---
>  CMakeLists.txt                    | 10 +++++++++-
>  bin/elasticurl_cpp/CMakeLists.txt |  2 ++
>  bin/mqtt5_app/CMakeLists.txt      |  2 ++
>  bin/mqtt5_canary/CMakeLists.txt   |  2 ++
>  4 files changed, 15 insertions(+), 1 deletion(-)

Touches: `CMakeLists.txt`, `bin/elasticurl_cpp/CMakeLists.txt`, `bin/mqtt5_app/CMakeLists.txt`, `bin/mqtt5_canary/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/aws-crt-cpp -B build/$QNX_ARCH/aws-crt-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_DEPS=OFF \
      -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" \
      -DAWS_WARNINGS_ARE_ERRORS=OFF
cmake --build build/$QNX_ARCH/aws-crt-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/aws-crt-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- BUILD_DEPS=OFF: build against the separately packaged aws-c-* components instead of the copies in crt/ (whose submodules are therefore not needed).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aws-crt-cpp`.

### 5. What it installs

58 files under `/usr/local`: 1 shared-library entries, 53 headers.

```
/usr/local/lib/libaws-crt-cpp.so
```

The complete list is `stage/<arch>/.manifest/aws-crt-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH aws-crt-cpp
ls repo/8.0/*/ | grep '^aws-crt-cpp-'
```

This writes `aws-crt-cpp-<ver>-r0.tar.gz` (run time) and `aws-crt-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install aws-crt-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/aws-crt-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/aws-crt-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.38.5; GIT=https://github.com/awslabs/aws-crt-cpp.git; REF=v0.38.5
DEPS="aws-c-auth aws-c-mqtt aws-c-event-stream aws-c-s3"
# BUILD_DEPS=OFF: build against the separately packaged aws-c-* components
# instead of the copies in crt/ (whose submodules are therefore not needed).
do_fetch() { git -c advice.detachedHead=false clone -q --depth 1 -b "$REF" "$GIT" "$SRCROOT"; }
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_DEPS=OFF -DCMAKE_PREFIX_PATH="$STAGE$PREFIX" -DAWS_WARNINGS_ARE_ERRORS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
