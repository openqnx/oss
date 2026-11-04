# hunspell 1.7.2

> Hunspell spell checker and morphological analyzer library and program

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/hunspell/hunspell/releases/download/v1.7.2/hunspell-1.7.2.tar.gz |
| SHA-256 | `11ddfa39afe28c28539fe65fc4f1592d410c1e9b6dd7d8a91ca25d85e9ec65b8` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_hunspell`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `hunspell` (programs, shared libraries, run-time data) and `hunspell-dev` (headers, pkg-config/CMake files, static libraries; needs `hunspell`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`hunspell`](https://openqnx.github.io/oss/package/x86_64/hunspell.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hunspell.html)) · [`hunspell-dev`](https://openqnx.github.io/oss/package/x86_64/hunspell-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hunspell-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add hunspell` (dependencies come along); one version: `apk add hunspell=<version>` | `apk del hunspell` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install hunspell` (dependencies first) | `oss-get remove hunspell` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`hunspell-dev` the same way, by its own name (`apk add hunspell-dev`, `oss-get install hunspell-dev`, by hand with its own file and list `hunspell-dev.list`); remove it before `hunspell`, which it needs.

```sh
# on the QNX target, as root
apk add hunspell hunspell-dev
apk add qnx-test-hunspell && /usr/local/bin/qnx-port-tests/test_hunspell    # its on-target test, from tests-8.0-<arch>
apk del hunspell hunspell-dev qnx-test-hunspell
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/hunspell.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/hunspell.list
rm var/lib/manual/hunspell.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hunspell` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/hunspell/hunspell/releases/download/v1.7.2/hunspell-1.7.2.tar.gz
mkdir -p dist src/hunspell
curl -fL -o dist/$(basename $URL) $URL
echo "11ddfa39afe28c28539fe65fc4f1592d410c1e9b6dd7d8a91ca25d85e9ec65b8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/hunspell --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/hunspell && cd build/$QNX_ARCH/hunspell
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/hunspell/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --without-ui \
    --without-readline \
    --disable-nls
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Hunspell: the spell checker and morphological analyser (LibreOffice's, Firefox's, enchant's): libhunspell-1.7 and the hunspell program (-l, -a ispell pipe mode, -m, -s), plus affixcompress/analyze/chmorph/munch/ unmunch for dictionary work. No dictionaries are included: they are per-language data (share/hunspell/<lang>.aff + .dic). Version as in QNX's own package list (also the latest release).
- iconv (GNU libiconv: QNX libc has none) converts dictionaries in other encodings than UTF-8.
- --without-ui: the curses interactive front end is not built (the pipe and list modes, which programs and scripts use, are). No readline, no translations (libintl).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hunspell`.

### 5. What it installs

27 files under `/usr/local`: 2 shared-library entries, 12 programs, 5 headers.

```
/usr/local/lib/libhunspell-1.7.so
/usr/local/lib/libhunspell-1.7.so.0
/usr/local/bin/affixcompress
/usr/local/bin/analyze
/usr/local/bin/chmorph
/usr/local/bin/hunspell
/usr/local/bin/hunzip
/usr/local/bin/hzip
/usr/local/bin/ispellaff2myspell
/usr/local/bin/makealias
/usr/local/bin/munch
/usr/local/bin/unmunch
/usr/local/bin/wordforms
/usr/local/bin/wordlist2hunspell
```

The complete list is `stage/<arch>/.manifest/hunspell` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hunspell
ls dists/8.0/*/ | grep '^hunspell-'
```

This writes `hunspell-<ver>-r0.tar.gz` (run time) and `hunspell-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hunspell
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hunspell-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hunspell-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_hunspell`](../../tests/src/) links hunspell the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-hunspell`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-hunspell
/usr/local/bin/qnx-port-tests/test_hunspell
```

```
== hunspell ==
  ok   ...
PASS hunspell
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_hunspell` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_hunspell`](../../tests/src/) -- a program linked against hunspell the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - dictionary qx.aff / qx.dic written
   - Hunspell_create
   - dictionary encoding UTF-8
   - \"qnx\" unknown before Hunspell_add
   - after Hunspell_add_with_affix(qnx like cat): qnx, qnxs
   - hunspell -s stems

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> hunspell`.

## Versions

This recipe builds **1.7.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hunspell=<version>`), sources in `ported/hunspell/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hunspell`; port one with `scripts/port-update hunspell`.

<details><summary>recipe.sh</summary>

```bash
# Hunspell: the spell checker and morphological analyser (LibreOffice's,
# Firefox's, enchant's): libhunspell-1.7 and the hunspell program (-l, -a
# ispell pipe mode, -m, -s), plus affixcompress/analyze/chmorph/munch/
# unmunch for dictionary work. No dictionaries are included: they are
# per-language data (share/hunspell/<lang>.aff + .dic). Version as in QNX's
# own package list (also the latest release).
VERSION=1.7.2
TARBALL=https://github.com/hunspell/hunspell/releases/download/v1.7.2/hunspell-1.7.2.tar.gz
SHA256=11ddfa39afe28c28539fe65fc4f1592d410c1e9b6dd7d8a91ca25d85e9ec65b8
BUILDSYS=autotools
# iconv (GNU libiconv: QNX libc has none) converts dictionaries in other
# encodings than UTF-8.
DEPS="libiconv"
# --without-ui: the curses interactive front end is not built (the pipe and
# list modes, which programs and scripts use, are). No readline, no
# translations (libintl).
CONF_ARGS_EXTRA=( --without-ui --without-readline --disable-nls )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
