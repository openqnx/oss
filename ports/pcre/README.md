# pcre 8.45

> Perl compatible regular expressions library (legacy PCRE1 API)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/pcre/pcre/8.45/pcre-8.45.tar.bz2 |
| SHA-256 | `4dae6fdcd2bb0bb6c37b5f97c33c2be954da743985369cddac3546e3218bffb8` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pcre`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pcre` (programs, shared libraries, run-time data) and `pcre-dev` (headers, pkg-config/CMake files, static libraries; needs `pcre`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pcre`](https://openqnx.github.io/oss/package/x86_64/pcre.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pcre.html)) · [`pcre-dev`](https://openqnx.github.io/oss/package/x86_64/pcre-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pcre-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pcre` (dependencies come along); one version: `apk add pcre=<version>` | `apk del pcre` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pcre` (dependencies first) | `oss-get remove pcre` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pcre-dev` the same way, by its own name (`apk add pcre-dev`, `oss-get install pcre-dev`, by hand with its own file and list `pcre-dev.list`); remove it before `pcre`, which it needs.

```sh
# on the QNX target, as root
apk add pcre pcre-dev
apk add qnx-test-pcre && /usr/local/bin/qnx-port-tests/test_pcre    # its on-target test, from tests-8.0-<arch>-m-z
apk del pcre pcre-dev qnx-test-pcre
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pcre.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pcre.list
rm var/lib/manual/pcre.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pcre` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/project/pcre/pcre/8.45/pcre-8.45.tar.bz2
mkdir -p dist src/pcre
curl -fL -o dist/$(basename $URL) $URL
echo "4dae6fdcd2bb0bb6c37b5f97c33c2be954da743985369cddac3546e3218bffb8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pcre --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/pcre && cd build/$QNX_ARCH/pcre
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/pcre/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-unicode-properties \
    --enable-pcre16 \
    --enable-pcre32 \
    --enable-jit
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- PCRE 1 (the legacy API; pcre2 is a separate component). Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pcre`.

### 5. What it installs

196 files under `/usr/local`: 10 shared-library entries, 3 programs, 6 headers.

```
/usr/local/lib/libpcre.so
/usr/local/lib/libpcre.so.3
/usr/local/lib/libpcre16.so
/usr/local/lib/libpcre16.so.2
/usr/local/lib/libpcre32.so
/usr/local/lib/libpcre32.so.0
/usr/local/lib/libpcrecpp.so
/usr/local/lib/libpcrecpp.so.0
/usr/local/lib/libpcreposix.so
/usr/local/lib/libpcreposix.so.0
/usr/local/bin/pcre-config
/usr/local/bin/pcregrep
/usr/local/bin/pcretest
```

The complete list is `stage/<arch>/.manifest/pcre` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pcre
ls dists/8.0/*/ | grep '^pcre-'
```

This writes `pcre-<ver>-r0.tar.gz` (run time) and `pcre-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pcre
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pcre-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pcre-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pcre`](../../tests/src/) links pcre the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pcre`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pcre
/usr/local/bin/qnx-port-tests/test_pcre
```

```
== pcre ==
  ok   ...
PASS pcre
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pcre` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pcre`](../../tests/src/) -- a program linked against pcre the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pcre_version() 8.45
   - built with UTF-8 and Unicode properties
   - pcre_compile date pattern
   - pcre_exec: match at offset 12..22, 3 groups
   - group 2 == \"06\"
   - pcre_get_substring
   - named group host == qnx.example.org
   - named group match
   - UTF-8 + \\p{Lu}\\p{Ll}+ \\p{Greek}+ matches
   - caseless UTF-8 match of e-acute vs E-acute
   - lookahead: 'a' before 'b' found at 5
   - pcre_study(JIT) compiled the pattern
   - studied/JIT pattern matches exactly abc1def2 and x9
   - pcre16: b+ in \"abbbc\" at 1..4
   - pcreposix regcomp with \\d and lookahead
   - pcreposix regexec: \"34\" at 20..22
   - pcrecpp FullMatch extracts word and int
   - pcrecpp GlobalReplace
   - mkdtemp
   - pcregrep -o '\\d+' prints 12, 345, 6
   - pcregrep -c -v counts 1 line
   - pcretest: 0: aac / 1: aa / No match

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pcre 8.45-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pcre-8.45-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pcre 8.45-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/pcre-8.45-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pcre 8.45-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pcre-8.45-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pcre 8.45-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/pcre-8.45-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.45**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pcre=<version>`), sources in `ported/pcre/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pcre`; port one with `scripts/port-update pcre`.

<details><summary>recipe.sh</summary>

```bash
# PCRE 1 (the legacy API; pcre2 is a separate component). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=8.45
TARBALL=https://downloads.sourceforge.net/project/pcre/pcre/8.45/pcre-8.45.tar.bz2
SHA256=4dae6fdcd2bb0bb6c37b5f97c33c2be954da743985369cddac3546e3218bffb8
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-unicode-properties --enable-pcre16 --enable-pcre32 --enable-jit )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
