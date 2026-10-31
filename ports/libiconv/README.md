# libiconv 1.18

> GNU character set conversion library (iconv)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/libiconv/libiconv-1.18.tar.gz |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libiconv`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libiconv
apk add libiconv-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libiconv && /usr/local/bin/qnx-port-tests/test_libiconv    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libiconv`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libiconv` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/libiconv/libiconv-1.18.tar.gz
mkdir -p dist src/libiconv
curl -fL -o dist/$(basename $URL) $URL
tar xf dist/$(basename $URL) -C src/libiconv --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libiconv && cd build/$QNX_ARCH/libiconv
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libiconv/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-nls \
    --enable-extra-encodings
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libiconv`.

### 5. What it installs

22 files under `/usr/local`: 6 shared-library entries, 1 programs, 3 headers.

```
/usr/local/lib/libcharset.so
/usr/local/lib/libcharset.so.1
/usr/local/lib/libcharset.so.1.0.0
/usr/local/lib/libiconv.so
/usr/local/lib/libiconv.so.2
/usr/local/lib/libiconv.so.2.7.0
/usr/local/bin/iconv
```

The complete list is `stage/<arch>/.manifest/libiconv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libiconv
ls dists/8.0/*/ | grep '^libiconv-'
```

This writes `libiconv-<ver>-r0.tar.gz` (run time) and `libiconv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libiconv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libiconv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libiconv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libiconv`](../../tests/src/) links libiconv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libiconv`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libiconv
/usr/local/bin/qnx-port-tests/test_libiconv
```

```
== libiconv ==
  ok   ...
PASS libiconv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libiconv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libiconv`](../../tests/src/) -- a program linked against libiconv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - _libiconv_version is 1.18
   - locale_charset() returns a name
   - UTF-8 -> ISO-8859-1: exact bytes
   - ISO-8859-1 -> UTF-8: round trip identical
   - UTF-8 -> UTF-16BE: A, e-acute, euro
   - UTF-16 surrogate pair -> U+1F600 (UTF-32LE)
   - euro sign to ISO-8859-1 fails with EILSEQ
   - //TRANSLIT turns the euro sign into EUR
   - ASCII//TRANSLIT of cafe-acute
   - CP1252 0x80 0x93 -> euro, left quote
   - KOI8-R -> UTF-8 (Cyrillic word)
   - unknown charset: EINVAL
   - mkdtemp
   - iconv -f UTF-8 -t ISO-8859-1 file
   - iconv back to UTF-8 (stdin) identical
   - iconv -l lists ISO-8859-1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libiconv 1.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libiconv-1.18-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libiconv 1.18-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libiconv-1.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libiconv 1.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libiconv-1.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libiconv 1.18-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libiconv-1.18-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libiconv=<version>`), sources in `ported/libiconv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libiconv`; port one with `scripts/port-update libiconv`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.18
TARBALL=https://ftp.gnu.org/gnu/libiconv/libiconv-1.18.tar.gz
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls --enable-extra-encodings )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
