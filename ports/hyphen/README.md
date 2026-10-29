# hyphen 2.8.8

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz |
| SHA-256 | `304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_misc_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add hyphen
apk add hyphen-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-misc-libs && /usr/local/bin/qnx-port-tests/test_misc_libs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install hyphen`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hyphen` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz
mkdir -p dist src/hyphen
curl -fL -o dist/$(basename $URL) $URL
echo "304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/hyphen --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/hyphen && cd build/$QNX_ARCH/hyphen
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/hyphen/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hyphen`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libhyphen.so
/usr/local/lib/libhyphen.so.3
/usr/local/bin/substrings.pl
```

The complete list is `stage/<arch>/.manifest/hyphen` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hyphen
ls dists/8.0/*/ | grep '^hyphen-'
```

This writes `hyphen-<ver>-r0.tar.gz` (run time) and `hyphen-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hyphen
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hyphen-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hyphen-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links hyphen the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-misc-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-misc-libs
/usr/local/bin/qnx-port-tests/test_misc_libs
```

```
== hyphen ==
  ok   ...
PASS hyphen
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_misc_libs`](../../tests/src/) -- a program linked against hyphen the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - enca reports supported languages
   - enca_analyser_alloc
   - enca identified the encoding
   - gsf_output_memory_new
   - gsf_output_write
   - gsf_output_size
   - bytes came back
   - gsm_create
   - gsm encode/decode round trip
   - decoded frame carries signal energy
   - hnj_hyphen_hyphenate2
   - missing dictionary reports NULL

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.8.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hyphen=<version>`), sources in `ported/hyphen/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hyphen`; port one with `scripts/port-update hyphen`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.8.8
TARBALL=https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz
SHA256=304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
