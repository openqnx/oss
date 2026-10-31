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

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pcre
apk add pcre-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pcre && /usr/local/bin/qnx-port-tests/test_pcre    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pcre`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

[`test_pcre`](../../tests/src/) links pcre the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pcre`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

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
