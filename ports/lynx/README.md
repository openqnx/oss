# lynx 2.9.2

> Text-mode web browser

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://invisible-mirror.net/archives/lynx/tarballs/lynx2.9.2.tar.bz2 |
| SHA-256 | `7374b89936d991669e101f4e97f2c9592036e1e8cdaa7bafc259a77ab6fb07ce` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`libidn2`](../libidn2/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lynx`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lynx` (programs, shared libraries, run-time data) and `lynx-dev` (headers, pkg-config/CMake files, static libraries; needs `lynx`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lynx`](https://openqnx.github.io/oss/package/x86_64/lynx.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lynx.html)) · [`lynx-dev`](https://openqnx.github.io/oss/package/x86_64/lynx-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lynx-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lynx` (dependencies come along); one version: `apk add lynx=<version>` | `apk del lynx` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lynx` (dependencies first) | `oss-get remove lynx` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lynx-dev` the same way, by its own name (`apk add lynx-dev`, `oss-get install lynx-dev`, by hand with its own file and list `lynx-dev.list`); remove it before `lynx`, which it needs.

```sh
# on the QNX target, as root
apk add lynx lynx-dev
apk add qnx-test-lynx && /usr/local/bin/qnx-port-tests/test_lynx    # its on-target test, from tests-8.0-<arch>
apk del lynx lynx-dev qnx-test-lynx
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lynx.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lynx.list
rm var/lib/manual/lynx.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lynx` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses openssl zlib brotli bzip2 libidn2 libiconv
```

<details><summary>Full build chain (9)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://invisible-mirror.net/archives/lynx/tarballs/lynx2.9.2.tar.bz2
mkdir -p dist src/lynx
curl -fL -o dist/$(basename $URL) $URL
echo "7374b89936d991669e101f4e97f2c9592036e1e8cdaa7bafc259a77ab6fb07ce  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lynx --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lynx < ports/lynx/patches/0001-include-pwd-h.patch
```

**[`0001-include-pwd-h.patch`](patches/0001-include-pwd-h.patch)** — Include <pwd.h> for getpwnam() whenever it exists.

> LYUtils.c expands "~user/" paths with getpwnam() and reads pw->pw_dir, but
> includes <pwd.h> only inside its HAVE_UTMP block. On glibc another header
> drags <pwd.h> in; on QNX nothing does and the build stops with "invalid use
> of undefined type 'struct passwd'" (configure finds no utmp there). Include
> it whenever configure found it (HAVE_PWD_H) -- the same fix the QNX ports
> carry.

Touches: `src/LYUtils.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/lynx && cd build/$QNX_ARCH/lynx
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/lynx/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --sysconfdir=/usr/local/etc \
    --with-build-cc=gcc \
    --with-screen=ncursesw \
    --with-ssl="$STAGE$PREFIX" \
    --with-zlib \
    --enable-ipv6 \
    --disable-nls \
    --enable-default-colors \
    --disable-full-paths \
    --enable-externs
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Lynx: the text-mode web browser (HTTP/HTTPS with OpenSSL, FTP, local files), with -dump/-source for scripted page rendering. Screen library: ncursesw (UTF-8).
- configure also finds and uses brotli and bzip2 (compressed transfer encodings and .bz2 files), libidn2 (international domain names) and libiconv (charset conversion).
- lynx.cfg and lynx.lss go to the package tree's own etc (hazard 124). --with-build-cc: the character-set tables are generated at build time by makeuctb, which has to run on the build machine. --disable-full-paths: helper programs (gzip, telnet, ...) are looked up on PATH at run time, not wherever configure found them on the build machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lynx`.

### 5. What it installs

4 files under `/usr/local`, 1 programs.

```
/usr/local/bin/lynx
```

The complete list is `stage/<arch>/.manifest/lynx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lynx
ls dists/8.0/*/ | grep '^lynx-'
```

This writes `lynx-<ver>-r0.tar.gz` (run time) and `lynx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lynx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lynx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lynx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lynx`](../../tests/src/) links lynx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lynx`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lynx
/usr/local/bin/qnx-port-tests/test_lynx
```

```
== lynx ==
  ok   ...
PASS lynx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lynx` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lynx`](../../tests/src/) -- a program linked against lynx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - lynx -version: 2.9.2
   - -version names OpenSSL
   - -dump: the <h1> heading
   - paragraph wrapped within 60 columns
   - list items as \"* item\"
   - entities: &eacute; -> UTF-8 \\u00e9, &amp; -> &
   - link [1] and the References list
   - table cells rendered
   - -nolist: no References section
   - -source: the file byte for byte
   - -dump http://127.0.0.1:<port>/page.html renders the served page
   - the server saw GET /page.html with User-Agent Lynx/2.9.2

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> lynx`.

## Versions

This recipe builds **2.9.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lynx=<version>`), sources in `ported/lynx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lynx`; port one with `scripts/port-update lynx`.

<details><summary>recipe.sh</summary>

```bash
# Lynx: the text-mode web browser (HTTP/HTTPS with OpenSSL, FTP, local
# files), with -dump/-source for scripted page rendering. Screen library:
# ncursesw (UTF-8).
VERSION=2.9.2
TARBALL=https://invisible-mirror.net/archives/lynx/tarballs/lynx2.9.2.tar.bz2
SHA256=7374b89936d991669e101f4e97f2c9592036e1e8cdaa7bafc259a77ab6fb07ce
# configure also finds and uses brotli and bzip2 (compressed transfer
# encodings and .bz2 files), libidn2 (international domain names) and
# libiconv (charset conversion).
DEPS="ncurses openssl zlib brotli bzip2 libidn2 libiconv"
BUILDSYS=autotools
# lynx.cfg and lynx.lss go to the package tree's own etc (hazard 124).
# --with-build-cc: the character-set tables are generated at build time by
# makeuctb, which has to run on the build machine. --disable-full-paths:
# helper programs (gzip, telnet, ...) are looked up on PATH at run time, not
# wherever configure found them on the build machine.
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc --with-build-cc=gcc
                  --with-screen=ncursesw --with-ssl="$STAGE$PREFIX" --with-zlib
                  --enable-ipv6 --disable-nls --enable-default-colors
                  --disable-full-paths --enable-externs )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
