# fakeroot 1.37.1.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz |
| SHA-256 | `86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fakeroot
apk add fakeroot-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fakeroot`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fakeroot` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
mkdir -p dist src/fakeroot
curl -fL -o dist/$(basename $URL) $URL
echo "86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/fakeroot --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0001-aports-fix-chown.patch
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0002-aports-fix-qnx.patch
patch -p1 -d src/fakeroot < ports/fakeroot/patches/0003-qnx-byte-order-macros.patch
```

**[`0001-aports-fix-chown.patch`](patches/0001-aports-fix-chown.patch)** — QNX change from QNX's aports (core/fakeroot/fix-chown.patch).

> Taken from qnx-ports/aports core/fakeroot -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `fakeroot-1.38.1/libfakeroot.c` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx.patch`](patches/0002-aports-fix-qnx.patch)** — QNX change from QNX's aports (core/fakeroot/fix-qnx.patch).

> Taken from qnx-ports/aports core/fakeroot -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
> Changed here: its message.h hunk (an endian-header case that only exists in
> 1.38) is replaced by 0003 for this release.

Touches: `fakeroot-1.38.1/communicate.c`, `fakeroot-1.38.1/faked.c`, `fakeroot-1.38.1/libfakeroot.c` · Upstream status: QNX-specific

**[`0003-qnx-byte-order-macros.patch`](patches/0003-qnx-byte-order-macros.patch)** — Give message.h the glibc byte-order macro names on QNX.

> message.h picks its 64-bit network byte-order helpers by testing
> __BYTE_ORDER against __BIG_ENDIAN/__LITTLE_ENDIAN, glibc's names. QNX only
> has the BSD spellings in <sys/param.h>; map the glibc names onto the GCC
> predefined macros, which every QNX compiler has.

Touches: `message.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/fakeroot && cd build/$QNX_ARCH/fakeroot
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/fakeroot/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-static \
    --with-ipc=tcp
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
- configure cannot test which IPC works when cross compiling; fakeroot uses its TCP transport on QNX.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fakeroot`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 2 programs.

```
/usr/local/lib/libfakeroot-0.so
/usr/local/lib/libfakeroot.so
/usr/local/bin/faked
/usr/local/bin/fakeroot
```

The complete list is `stage/<arch>/.manifest/fakeroot` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fakeroot
ls dists/8.0/*/ | grep '^fakeroot-'
```

This writes `fakeroot-<ver>-r0.tar.gz` (run time) and `fakeroot-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fakeroot
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fakeroot-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fakeroot-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.37.1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fakeroot=<version>`), sources in `ported/fakeroot/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fakeroot`; port one with `scripts/port-update fakeroot`.

<details><summary>recipe.sh</summary>

```bash
# fakeroot (run as fake root for packaging). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.37.1.1
TARBALL=https://deb.debian.org/debian/pool/main/f/fakeroot/fakeroot_1.37.1.1.orig.tar.gz
SHA256=86b0b75bf319ca42e525c098675b6ed10a06b76e69ec9ccf20ef5e03883b3a14
BUILDSYS=autotools
# configure cannot test which IPC works when cross compiling; fakeroot uses
# its TCP transport on QNX.
CONF_ARGS_EXTRA=( --disable-static --with-ipc=tcp )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
