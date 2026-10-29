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

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install git
oss-get install git-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH git
ls repo/8.0/*/ | grep '^git-'
```

This writes `git-<ver>-r0.tar.gz` (run time) and `git-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install git
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/git-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/git-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
