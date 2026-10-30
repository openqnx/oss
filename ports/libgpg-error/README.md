# libgpg-error 1.50

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2 |
| SHA-256 | `69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libgpg-error
apk add libgpg-error-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libgpg-error`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libgpg-error` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gettext
```

<details><summary>Full build chain (4)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2
mkdir -p dist src/libgpg-error
curl -fL -o dist/$(basename $URL) $URL
echo "69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libgpg-error --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libgpg-error && cd build/$QNX_ARCH/libgpg-error
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libgpg-error/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-tests \
    --disable-languages
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): gettext
- libgpg-error needs a syscfg/lock-obj-pub.<triplet>.h describing the target's pthread_mutex_t layout, and ships none for QNX. The two under files/ were produced by building the upstream generator (src/gen-posix-lock-obj.c) for QNX and running it on the real targets -- a Raspberry Pi 5 for aarch64le and the QEMU VM for x86_64. QNX's pthread_mutex_t is 8 bytes on both and the two generated headers are byte-identical, so a CPU-less lock-obj-pub.nto-qnx.h is shipped as well: mkheader falls back to that spelling for the x86_64 triplet.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libgpg-error`.

### 5. What it installs

30 files under `/usr/local`: 2 shared-library entries, 2 programs, 2 headers.

```
/usr/local/lib/libgpg-error.so
/usr/local/lib/libgpg-error.so.37
/usr/local/bin/gpg-error
/usr/local/bin/gpgrt-config
```

The complete list is `stage/<arch>/.manifest/libgpg-error` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libgpg-error
ls dists/8.0/*/ | grep '^libgpg-error-'
```

This writes `libgpg-error-<ver>-r0.tar.gz` (run time) and `libgpg-error-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libgpg-error
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libgpg-error-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libgpg-error-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.50**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libgpg-error=<version>`), sources in `ported/libgpg-error/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libgpg-error`; port one with `scripts/port-update libgpg-error`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.50
# Also linked at run time (found by scripts/gen-dependency-index): gettext
DEPS="gettext"
TARBALL=https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2
SHA256=69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a
BUILDSYS=autotools
# libgpg-error needs a syscfg/lock-obj-pub.<triplet>.h describing the target's
# pthread_mutex_t layout, and ships none for QNX. The two under files/ were
# produced by building the upstream generator (src/gen-posix-lock-obj.c) for QNX
# and running it on the real targets -- a Raspberry Pi 5 for aarch64le and the
# QEMU VM for x86_64. QNX's pthread_mutex_t is 8 bytes on both and the two
# generated headers are byte-identical, so a CPU-less lock-obj-pub.nto-qnx.h is
# shipped as well: mkheader falls back to that spelling for the x86_64 triplet.
do_patch() {
  cp "$RECIPE_DIR/libgpg-error/files/lock-obj-pub."*.h "$SRCROOT/src/syscfg/"
  ls "$SRCROOT/src/syscfg/" | grep nto-qnx
}

CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-languages )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
