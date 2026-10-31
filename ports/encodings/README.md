# encodings 1.1.0

> X.Org font encoding tables for libfontenc

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.x.org/releases/individual/font/encodings-1.1.0.tar.xz |
| SHA-256 | `9ff13c621756cfa12e95f32ba48a5b23839e8f577d0048beda66c67dab4de975` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_encodings`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add encodings
apk add encodings-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-encodings && /usr/local/bin/qnx-port-tests/test_encodings    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install encodings`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild encodings` performs exactly the same steps (2–5) in one command.

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
URL=https://www.x.org/releases/individual/font/encodings-1.1.0.tar.xz
mkdir -p dist src/encodings
curl -fL -o dist/$(basename $URL) $URL
echo "9ff13c621756cfa12e95f32ba48a5b23839e8f577d0048beda66c67dab4de975  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/encodings --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/encodings && cd build/$QNX_ARCH/encodings
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/encodings/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- X.Org font encoding files (data). Their index is generated with a native mkfontscale on the build host. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH encodings`.

### 5. What it installs

52 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/encodings` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa encodings
ls dists/8.0/*/ | grep '^encodings-'
```

This writes `encodings-<ver>-r0.tar.gz` (run time) and `encodings-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add encodings
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/encodings-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/encodings-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_encodings`](../../tests/src/) links encodings the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-encodings`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-encodings
/usr/local/bin/qnx-port-tests/test_encodings
```

```
== encodings ==
  ok   ...
PASS encodings
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_encodings` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_encodings`](../../tests/src/) -- a program linked against encodings the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libfontenc's encodings.dir is installed
   - encodings.dir: 60+ entries, every file present
   - iso8859-13: 0xC0 -> U+0104 (A with ogonek)
   - iso8859-13: 0xA1 -> U+201D, ASCII unchanged
   - alias microsoft-ansi -> microsoft-cp1252: 0x80 -> U+20AC (euro)
   - cp866 0x80, cp1251 0xC0 -> U+0410 (Cyrillic A)
   - koi8-r (built in): 0xC1 -> U+0430
   - large/gbk-0: 0x8140 -> U+4E02, 0xB0A1 -> U+554A
   - large/jisx0208: 0x3021 -> U+4E9C, 0x2422 -> U+3042 (hiragana a)
   - reverse cp1252: U+20AC -> 0x80, U+2122 -> 0x99
   - encoding from an XLFD name
   - FontEncIdentify(large/gbk-0.enc.gz) -> gbk-0

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | encodings 1.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/encodings-1.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | encodings 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/encodings-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | encodings 1.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/encodings-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | encodings 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/encodings-1.1.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add encodings=<version>`), sources in `ported/encodings/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates encodings`; port one with `scripts/port-update encodings`.

<details><summary>recipe.sh</summary>

```bash
# X.Org font encoding files (data). Their index is generated with a native mkfontscale on the build host.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.1.0
TARBALL=https://www.x.org/releases/individual/font/encodings-1.1.0.tar.xz
SHA256=9ff13c621756cfa12e95f32ba48a5b23839e8f577d0048beda66c67dab4de975
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
