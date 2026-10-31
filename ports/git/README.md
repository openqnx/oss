# git 2.51.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.51.0.tar.xz |
| SHA-256 | `60a7c2251cc2e588d5cd87bae567260617c6de0c22dca9cdbfc4c7d2b8990b62` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) [`curl`](../curl/) [`expat`](../expat/) [`pcre2`](../pcre2/) [`perl`](../perl/) [`perl-error`](../perl-error/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`curl`](../curl/) [`expat`](../expat/) [`libiconv`](../libiconv/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add git
apk add git-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install git`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild git` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (14 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib openssl curl expat pcre2 perl perl-error libiconv
```

<details><summary>Full build chain (14)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`expat`](../expat/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`perl`](../perl/) [`perl-error`](../perl-error/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.51.0.tar.xz
mkdir -p dist src/git
curl -fL -o dist/$(basename $URL) $URL
echo "60a7c2251cc2e588d5cd87bae567260617c6de0c22dca9cdbfc4c7d2b8990b62  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/git --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/git < ports/git/patches/0001-aports-qnx-build.patch
```

**[`0001-aports-qnx-build.patch`](patches/0001-aports-qnx-build.patch)** — QNX change from QNX's aports (extra/git/qnx-build.patch).

> Taken from qnx-ports/aports extra/git -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `git-2.51.0/config.mak.uname`, `git-2.51.0/t/helper/test-rot13-filter.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/git; B=build/$QNX_ARCH/git; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _gitmake all; }
```

```bash
do_install() { _gitmake DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Git for the target. Version as in the QNX Everywhere 8.0.4r1 image.
- Git's Makefile is configured by variables rather than a configure script. uname_S=QNX selects its QNX settings; the rest names the target's paths and the features that are cross compiled here. git builds its own link list (EXTLIBS: -lz, -lcrypto, ...); setting EXTLIBS here replaced it ("undefined reference to crc32"). NEEDS_SOCKET adds QNX's -lsocket to it, and NO_REGEX builds git's bundled regex, so no -lregex. The staged GNU libiconv's <iconv.h> renames iconv() to libiconv(), which QNX's libc lacks: NEEDS_LIBICONV links -liconv.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH git`.

### 5. What it installs

246 files under `/usr/local`, 7 programs.

```
/usr/local/bin/git
/usr/local/bin/git-cvsserver
/usr/local/bin/git-receive-pack
/usr/local/bin/git-shell
/usr/local/bin/git-upload-archive
/usr/local/bin/git-upload-pack
/usr/local/bin/scalar
```

The complete list is `stage/<arch>/.manifest/git` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa git
ls dists/8.0/*/ | grep '^git-'
```

This writes `git-<ver>-r0.tar.gz` (run time) and `git-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add git
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/git-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/git-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | git 2.51.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/git-2.51.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | git 2.51.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/git-2.51.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.51.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add git=<version>`), sources in `ported/git/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates git`; port one with `scripts/port-update git`.

<details><summary>recipe.sh</summary>

```bash
# Git for the target. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.51.0
TARBALL=https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.51.0.tar.xz
SHA256=60a7c2251cc2e588d5cd87bae567260617c6de0c22dca9cdbfc4c7d2b8990b62
DEPS="zlib openssl curl expat pcre2 perl perl-error libiconv"
BUILDSYS=custom
# Git's Makefile is configured by variables rather than a configure script.
# uname_S=QNX selects its QNX settings; the rest names the target's paths and
# the features that are cross compiled here.
# git builds its own link list (EXTLIBS: -lz, -lcrypto, ...); setting EXTLIBS
# here replaced it ("undefined reference to crc32"). NEEDS_SOCKET adds QNX's
# -lsocket to it, and NO_REGEX builds git's bundled regex, so no -lregex.
# The staged GNU libiconv's <iconv.h> renames iconv() to libiconv(), which
# QNX's libc lacks: NEEDS_LIBICONV links -liconv.
_gitmake() { make -C "$B" -j"$(nproc)" prefix="$PREFIX" CC="qcc -V$QCC_TARGET" AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" \
  uname_S=QNX uname_M="$QNX_CPU" CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE_SYSROOT$PREFIX/include" LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib" \
  NO_GETTEXT=1 NO_TCLTK=1 NO_PYTHON=1 USE_LIBPCRE2=1 CURL_CONFIG=false CURL_LDFLAGS="-lcurl" \
  PERL_PATH=/usr/local/bin/perl SHELL_PATH=/bin/sh NO_REGEX=NeedsStartEnd HOST_CPU="$QNX_CPU" \
  NEEDS_SOCKET=YesPlease NEEDS_LIBICONV=YesPlease "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _gitmake all; }
do_install() { _gitmake DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
