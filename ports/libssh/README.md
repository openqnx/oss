# libssh 0.11.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.libssh.org/files/0.11/libssh-0.11.1.tar.xz |
| SHA-256 | `14b7dcc72e91e08151c58b981a7b570ab2663f630e7d2837645d5a9c612c1b79` |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libssh`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libssh
oss-get install libssh-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libssh` performs exactly the same steps (2–5) in one command.

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
URL=https://www.libssh.org/files/0.11/libssh-0.11.1.tar.xz
mkdir -p dist src/libssh
curl -fL -o dist/$(basename $URL) $URL
echo "14b7dcc72e91e08151c58b981a7b570ab2663f630e7d2837645d5a9c612c1b79  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libssh --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libssh < ports/libssh/patches/0001-ttyopts-b230400-optional.patch
```

**[`0001-ttyopts-b230400-optional.patch`](patches/0001-ttyopts-b230400-optional.patch)** — Treat B230400 as optional.

> The termios speed constants above B115200 are not POSIX. QNX's <termios.h>
> stops at B115200, so the case label for 230400 baud has to be conditional.

Touches: `src/ttyopts.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libssh -B build/$QNX_ARCH/libssh -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DWITH_EXAMPLES=OFF \
      -DUNIT_TESTING=OFF \
      -DWITH_GSSAPI=OFF \
      -DWITH_SERVER=ON \
      -DWITH_ZLIB=ON
cmake --build build/$QNX_ARCH/libssh -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libssh
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libssh`.

### 5. What it installs

16 files under `/usr/local`: 3 shared-library entries, 9 headers.

```
/usr/local/lib/libssh.so
/usr/local/lib/libssh.so.4
/usr/local/lib/libssh.so.4.10.1
```

The complete list is `stage/<arch>/.manifest/libssh` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libssh
ls repo/8.0/*/ | grep '^libssh-'
```

This writes `libssh-<ver>-r0.tar.gz` (run time) and `libssh-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libssh
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libssh-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libssh-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libssh`](../../tests/src/) links libssh the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libssh root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libssh'
```

```
== libssh ==
  ok   ...
PASS libssh
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.11.1
TARBALL=https://www.libssh.org/files/0.11/libssh-0.11.1.tar.xz
SHA256=14b7dcc72e91e08151c58b981a7b570ab2663f630e7d2837645d5a9c612c1b79
DEPS="openssl zlib"
CMAKE_ARGS_EXTRA=( -DWITH_EXAMPLES=OFF -DUNIT_TESTING=OFF -DWITH_GSSAPI=OFF -DWITH_SERVER=ON -DWITH_ZLIB=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
