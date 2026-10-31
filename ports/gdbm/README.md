# gdbm 1.24

> GNU dbm key/value database library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gdbm/gdbm-1.24.tar.gz |
| SHA-256 | `695e9827fdf763513f133910bc7e6cfdb9187943a4fec943e57449723d2b8dbf` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gdbm`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gdbm
apk add gdbm-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gdbm && /usr/local/bin/qnx-port-tests/test_gdbm    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gdbm`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gdbm` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/gdbm/gdbm-1.24.tar.gz
mkdir -p dist src/gdbm
curl -fL -o dist/$(basename $URL) $URL
echo "695e9827fdf763513f133910bc7e6cfdb9187943a4fec943e57449723d2b8dbf  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gdbm --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gdbm && cd build/$QNX_ARCH/gdbm
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/gdbm/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-nls \
    --without-readline \
    --enable-libgdbm-compat
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gdbm`.

### 5. What it installs

15 files under `/usr/local`: 4 shared-library entries, 3 programs, 3 headers.

```
/usr/local/lib/libgdbm.so
/usr/local/lib/libgdbm.so.6
/usr/local/lib/libgdbm_compat.so
/usr/local/lib/libgdbm_compat.so.4
/usr/local/bin/gdbm_dump
/usr/local/bin/gdbm_load
/usr/local/bin/gdbmtool
```

The complete list is `stage/<arch>/.manifest/gdbm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gdbm
ls dists/8.0/*/ | grep '^gdbm-'
```

This writes `gdbm-<ver>-r0.tar.gz` (run time) and `gdbm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gdbm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gdbm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gdbm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gdbm`](../../tests/src/) links gdbm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gdbm`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gdbm
/usr/local/bin/qnx-port-tests/test_gdbm
```

```
== gdbm ==
  ok   ...
PASS gdbm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gdbm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gdbm`](../../tests/src/) -- a program linked against gdbm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - create temp file
   - gdbm_open
   - gdbm_store
   - duplicate insert is refused
   - replace works
   - gdbm_fetch returns the new value
   - gdbm_exists
   - gdbm_delete
   - key is gone after delete

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gdbm 1.24-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gdbm-1.24-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gdbm 1.24-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gdbm-1.24-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gdbm=<version>`), sources in `ported/gdbm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gdbm`; port one with `scripts/port-update gdbm`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24; TARBALL=https://ftp.gnu.org/gnu/gdbm/gdbm-1.24.tar.gz
SHA256=695e9827fdf763513f133910bc7e6cfdb9187943a4fec943e57449723d2b8dbf
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls
                  --without-readline --enable-libgdbm-compat )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
