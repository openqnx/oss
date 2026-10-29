# apk-tools 3.0.0_rc5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/alpinelinux/apk-tools.git (tag `v3.0.0_rc5`) |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install apk-tools
oss-get install apk-tools-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild apk-tools` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib zstd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v3.0.0_rc5 --recurse-submodules --shallow-submodules \
    https://github.com/alpinelinux/apk-tools.git src/apk-tools
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0001-aports-qnx-compat.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0002-portability-optional-sys-random-h.patch
```

**[`0001-aports-qnx-compat.patch`](patches/0001-aports-qnx-compat.patch)** — QNX compatibility for apk-tools (from QNX's aports core/apk-tools).

> Taken from qnx-ports/aports core/apk-tools/0001-qnx-compat.patch, the patch
> the QNX Everywhere image's apk is built with.
>
>
> qnx: compat
>
> ---
>  libfetch/common.c | 4 ++++
>  src/apk_xattr.h   | 8 ++++++++
>  src/io.c          | 6 +++---
>  3 files changed, 15 insertions(+), 3 deletions(-)

Touches: `libfetch/common.c`, `src/apk_xattr.h`, `src/io.c` · Upstream status: QNX-specific

**[`0002-portability-optional-sys-random-h.patch`](patches/0002-portability-optional-sys-random-h.patch)** — Do not require a system <sys/random.h>.

> The portability wrapper always does #include_next <sys/random.h>, but its
> whole purpose is systems without getrandom(): QNX 8 has neither the header
> nor the function, and the build stops before the bundled getrandom.c (which
> reads /dev/urandom) is used. Include the system header only if it exists.

Touches: `portability/sys/random.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/apk-tools src/apk-tools \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=disabled \
      -Dhelp=disabled \
      -Dlua=disabled \
      -Dpython=disabled \
      -Dtests=disabled \
      -Dcrypto_backend=openssl \
      -Dzstd=true \
      -Dc_args=-D_QNX_SOURCE \
      -Dc_link_args=-lsocket
ninja -C build/$QNX_ARCH/apk-tools
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/apk-tools install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- apk-tools, the Alpine package manager the QNX Everywhere image (and repo.oss.qnx.com) uses -- with it a target can install this repository's APK packages. Version as in the image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH apk-tools`.

### 5. What it installs

31 files under `/usr/local`: 2 shared-library entries, 1 programs, 26 headers.

```
/usr/local/lib/libapk.so
/usr/local/lib/libapk.so.3.0.0_rc5
/usr/local/bin/apk
```

The complete list is `stage/<arch>/.manifest/apk-tools` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH apk-tools
ls repo/8.0/*/ | grep '^apk-tools-'
```

This writes `apk-tools-<ver>-r0.tar.gz` (run time) and `apk-tools-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install apk-tools
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/apk-tools-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/apk-tools-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# apk-tools, the Alpine package manager the QNX Everywhere image (and
# repo.oss.qnx.com) uses -- with it a target can install this repository's APK
# packages. Version as in the image.
VERSION=3.0.0_rc5; GIT=https://github.com/alpinelinux/apk-tools.git; REF=v3.0.0_rc5
DEPS="openssl zlib zstd"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dhelp=disabled -Dlua=disabled -Dpython=disabled -Dtests=disabled -Dcrypto_backend=openssl -Dzstd=true -Dc_args=-D_QNX_SOURCE -Dc_link_args=-lsocket )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
