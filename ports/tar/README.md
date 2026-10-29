# tar 1.35

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz |
| SHA-256 | `4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tar
apk add tar-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tar`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tar` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libiconv
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz
mkdir -p dist src/tar
curl -fL -o dist/$(basename $URL) $URL
echo "4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tar --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tar < ports/tar/patches/0001-aports-dir_open.patch
patch -p1 -d src/tar < ports/tar/patches/0002-aports-fix-qnx-makedev.patch
```

**[`0001-aports-dir_open.patch`](patches/0001-aports-dir_open.patch)** — QNX change from QNX's aports (core/tar/dir_open.patch).

> Taken from qnx-ports/aports core/tar -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/create.c` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx-makedev.patch`](patches/0002-aports-fix-qnx-makedev.patch)** — QNX change from QNX's aports (core/tar/fix-qnx-makedev.patch).

> Taken from qnx-ports/aports core/tar -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/list.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/tar && cd build/$QNX_ARCH/tar
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
LIBS="-liconv" \
$OLDPWD/src/tar/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU tar. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv
- The libiconv component's <iconv.h> renames iconv_open to libiconv_open, so link that library explicitly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tar`.

### 5. What it installs

6 files under `/usr/local`, 1 programs.

```
/usr/local/bin/tar
```

The complete list is `stage/<arch>/.manifest/tar` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tar
ls dists/8.0/*/ | grep '^tar-'
```

This writes `tar-<ver>-r0.tar.gz` (run time) and `tar-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tar
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tar-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tar-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tar 1.35-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.35**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tar=<version>`), sources in `ported/tar/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tar`; port one with `scripts/port-update tar`.

<details><summary>recipe.sh</summary>

```bash
# GNU tar. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.35
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/tar/tar-1.35.tar.xz
SHA256=4d62ff37342ec7aed748535323930c7cf94acf71c3591882b26a7ea50f3edc16
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls )
# The libiconv component's <iconv.h> renames iconv_open to libiconv_open, so
# link that library explicitly.
CONF_ENV=( LIBS="-liconv" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
