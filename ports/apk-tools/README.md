# apk-tools 3.0.0_rc5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/alpinelinux/apk-tools.git (tag `v3.0.0_rc5`) |
| Build system | Meson |
| QNX patches | 3 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add apk-tools
apk add apk-tools-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install apk-tools`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0001-aports-qnx-compat.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0002-portability-optional-sys-random-h.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0003-qnx-openat-layer-dir-o-directory.patch
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

**[`0003-qnx-openat-layer-dir-o-directory.patch`](patches/0003-qnx-openat-layer-dir-o-directory.patch)** — Open the database layer directory with O_DIRECTORY.

> apk_db_read_layer() opens lib/apk/db with plain O_RDONLY and then reads
> installed, triggers and scripts.tar from it with openat(). On QNX 8 a
> directory descriptor is only accepted as an openat() base if it was opened
> with O_DIRECTORY; otherwise every openat() under it fails with ENOTDIR
> (verified on the target). The first "apk --initdb add" works because the
> directory does not exist yet, but every later apk command on that root stops
> with "Unable to read database: Not a directory". Open the layer directory
> with O_DIRECTORY, as apk already does for the root and cache directories.

Touches: `src/database.c` · Upstream status: QNX-specific

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
- Not linked, but needed to fetch from an https:// repository (the GitHub releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa apk-tools
ls dists/8.0/*/ | grep '^apk-tools-'
```

This writes `apk-tools-<ver>-r0.tar.gz` (run time) and `apk-tools-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add apk-tools
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/apk-tools-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/apk-tools-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.0.0_rc5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add apk-tools=<version>`), sources in `ported/apk-tools/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates apk-tools`; port one with `scripts/port-update apk-tools`.

<details><summary>recipe.sh</summary>

```bash
# apk-tools, the Alpine package manager the QNX Everywhere image (and
# repo.oss.qnx.com) uses -- with it a target can install this repository's APK
# packages. Version as in the image.
VERSION=3.0.0_rc5; GIT=https://github.com/alpinelinux/apk-tools.git; REF=v3.0.0_rc5
DEPS="openssl zlib zstd"
# Not linked, but needed to fetch from an https:// repository (the GitHub
# releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.
RUNTIME_DEPS="ca-certificates"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dhelp=disabled -Dlua=disabled -Dpython=disabled -Dtests=disabled -Dcrypto_backend=openssl -Dzstd=true -Dc_args=-D_QNX_SOURCE -Dc_link_args=-lsocket )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
