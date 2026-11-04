# flex 2.6.4

> Fast lexical analyser generator (flex) and its libfl

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz |
| SHA-256 | `e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_flex`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `flex` (programs, shared libraries, run-time data) and `flex-dev` (headers, pkg-config/CMake files, static libraries; needs `flex`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`flex`](https://openqnx.github.io/oss/package/x86_64/flex.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/flex.html)) · [`flex-dev`](https://openqnx.github.io/oss/package/x86_64/flex-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/flex-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add flex` (dependencies come along); one version: `apk add flex=<version>` | `apk del flex` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install flex` (dependencies first) | `oss-get remove flex` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`flex-dev` the same way, by its own name (`apk add flex-dev`, `oss-get install flex-dev`, by hand with its own file and list `flex-dev.list`); remove it before `flex`, which it needs.

```sh
# on the QNX target, as root
apk add flex flex-dev
apk add qnx-test-flex && /usr/local/bin/qnx-port-tests/test_flex    # its on-target test, from tests-8.0-<arch>
apk del flex flex-dev qnx-test-flex
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/flex.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/flex.list
rm var/lib/manual/flex.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild flex` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
mkdir -p dist src/flex
curl -fL -o dist/$(basename $URL) $URL
echo "e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/flex --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/flex && cd build/$QNX_ARCH/flex
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
M4=/usr/local/bin/m4 \
ac_cv_path_M4=/usr/local/bin/m4 \
ac_cv_func_malloc_0_nonnull=yes \
ac_cv_func_realloc_0_nonnull=yes \
$OLDPWD/src/flex/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --disable-rpath \
    --without-libiconv-prefix \
    --without-libintl-prefix
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- flex, the fast lexical analyser generator (scanners from .l files), and libfl (its yywrap/main helpers). Version as in QNX's own package list.
- flex calls m4 at run time to expand its skeleton: the m4 port's /usr/local/bin/m4 (QNX itself ships none), not the build machine's path.
- regcomp/regexec are in QNX's libregex, not in libc: on LDFLAGS, not LIBS -- the build also compiles a stage1flex for the build machine (with $(LIBS)), which has no libregex. malloc(0)/realloc(NULL, 0): a cross configure cannot run its test and guesses "no", which replaces them with rpl_malloc/rpl_realloc that flex never defines; QNX's return a valid pointer.
- --disable-nls: no gettext/libiconv (the C locale needs no catalogue).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH flex`.

### 5. What it installs

15 files under `/usr/local`: 2 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libfl.so
/usr/local/lib/libfl.so.2
/usr/local/bin/flex
/usr/local/bin/flex++
```

The complete list is `stage/<arch>/.manifest/flex` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa flex
ls dists/8.0/*/ | grep '^flex-'
```

This writes `flex-<ver>-r0.tar.gz` (run time) and `flex-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add flex
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/flex-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/flex-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_flex`](../../tests/src/) links flex the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-flex`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-flex
/usr/local/bin/qnx-port-tests/test_flex
```

```
== flex ==
  ok   ...
PASS flex
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_flex` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_flex`](../../tests/src/) -- a program linked against flex the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> flex`.

## Versions

This recipe builds **2.6.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add flex=<version>`), sources in `ported/flex/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates flex`; port one with `scripts/port-update flex`.

<details><summary>recipe.sh</summary>

```bash
# flex, the fast lexical analyser generator (scanners from .l files), and
# libfl (its yywrap/main helpers). Version as in QNX's own package list.
VERSION=2.6.4
TARBALL=https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz
SHA256=e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995
BUILDSYS=autotools
# flex calls m4 at run time to expand its skeleton: the m4 port's
# /usr/local/bin/m4 (QNX itself ships none), not the build machine's path.
RUNTIME_DEPS="m4"
# regcomp/regexec are in QNX's libregex, not in libc: on LDFLAGS, not LIBS --
# the build also compiles a stage1flex for the build machine (with $(LIBS)),
# which has no libregex.
# malloc(0)/realloc(NULL, 0): a cross configure cannot run its test and
# guesses "no", which replaces them with rpl_malloc/rpl_realloc that flex
# never defines; QNX's return a valid pointer.
LDFLAGS_APPEND="-lregex"
CONF_ENV=( M4=/usr/local/bin/m4 ac_cv_path_M4=/usr/local/bin/m4
           ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes )
# --disable-nls: no gettext/libiconv (the C locale needs no catalogue).
CONF_ARGS_EXTRA=( --disable-nls --disable-rpath --without-libiconv-prefix --without-libintl-prefix )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
