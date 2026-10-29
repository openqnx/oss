# restbed 4.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/Corvusoft/restbed.git (tag `4.8`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) [`asio`](../asio/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add restbed
apk add restbed-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install restbed`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild restbed` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl asio
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 4.8 --recurse-submodules --shallow-submodules \
    https://github.com/Corvusoft/restbed.git src/restbed
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/restbed < ports/restbed/patches/0001-qnx-sol-tcp.patch
```

**[`0001-qnx-sol-tcp.patch`](patches/0001-qnx-sol-tcp.patch)** — Use IPPROTO_TCP as the TCP socket-option level on QNX.

> SOL_TCP is a Linux alias for IPPROTO_TCP; QNX only has the POSIX name.

Touches: `source/corvusoft/restbed/detail/socket_impl.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/restbed -B build/$QNX_ARCH/restbed -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_CXX_FLAGS=-std=gnu++17 \
      -isystem \
      $STAGE$PREFIX/include \
      -DASIO_DISABLE_STD_EXPERIMENTAL_STRING_VIEW \
      -Wno-narrowing \
      -DBUILD_SSL=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBRARY=OFF \
      -DCMAKE_INSTALL_LIBDIR=lib
cmake --build build/$QNX_ARCH/restbed -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/restbed
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- restbed builds as C++14, where standalone Asio reaches for <experimental/string_view> -- a libstdc++ header QNX's libc++ does not ship. Tell Asio not to use it. (CMAKE_CXX_FLAGS replaces the toolchain's initial flags, so those are repeated.) -Wno-narrowing: uri.cpp puts -1 in a char table, which is a narrowing error where char is unsigned (aarch64).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH restbed`.

### 5. What it installs

26 files under `/usr/local`: 3 shared-library entries, 22 headers.

```
/usr/local/lib/librestbed.so
/usr/local/lib/librestbed.so.4
/usr/local/lib/librestbed.so.4.7
```

The complete list is `stage/<arch>/.manifest/restbed` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa restbed
ls dists/8.0/*/ | grep '^restbed-'
```

This writes `restbed-<ver>-r0.tar.gz` (run time) and `restbed-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add restbed
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/restbed-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/restbed-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | restbed 4.8-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **4.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add restbed=<version>`), sources in `ported/restbed/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates restbed`; port one with `scripts/port-update restbed`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.8; GIT=https://github.com/Corvusoft/restbed.git; REF=4.8
DEPS="openssl asio"
# restbed builds as C++14, where standalone Asio reaches for
# <experimental/string_view> -- a libstdc++ header QNX's libc++ does not ship.
# Tell Asio not to use it. (CMAKE_CXX_FLAGS replaces the toolchain's initial
# flags, so those are repeated.) -Wno-narrowing: uri.cpp puts -1 in a char
# table, which is a narrowing error where char is unsigned (aarch64).
CMAKE_ARGS_EXTRA=( "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -DASIO_DISABLE_STD_EXPERIMENTAL_STRING_VIEW -Wno-narrowing" -DBUILD_SSL=ON -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBRARY=OFF
                   -DCMAKE_INSTALL_LIBDIR=lib )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
