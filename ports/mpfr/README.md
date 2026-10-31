# mpfr 4.2.1

> C library for multiple-precision floating-point computations

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz |
| SHA-256 | `277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gmp`](../gmp/) |
| Run-time dependencies | [`gmp`](../gmp/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mpfr`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add mpfr
apk add mpfr-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-mpfr && /usr/local/bin/qnx-port-tests/test_mpfr    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install mpfr`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mpfr` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gmp
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz
mkdir -p dist src/mpfr
curl -fL -o dist/$(basename $URL) $URL
echo "277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mpfr --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mpfr && cd build/$QNX_ARCH/mpfr
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/mpfr/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-gmp="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mpfr`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libmpfr.so
/usr/local/lib/libmpfr.so.8
```

The complete list is `stage/<arch>/.manifest/mpfr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mpfr
ls dists/8.0/*/ | grep '^mpfr-'
```

This writes `mpfr-<ver>-r0.tar.gz` (run time) and `mpfr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mpfr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mpfr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mpfr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mpfr`](../../tests/src/) links mpfr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mpfr`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mpfr
/usr/local/bin/qnx-port-tests/test_mpfr
```

```
== mpfr ==
  ok   ...
PASS mpfr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mpfr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mpfr`](../../tests/src/) -- a program linked against mpfr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pi to 28 decimals matches the known value
   - e to 20 decimals
   - pi + e exceeds both
   - precision is honoured
   - 1/3 at 10 bits is close to 0.3333

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mpfr 4.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mpfr-4.2.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mpfr 4.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mpfr-4.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mpfr=<version>`), sources in `ported/mpfr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mpfr`; port one with `scripts/port-update mpfr`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.2.1; TARBALL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz
SHA256=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
DEPS="gmp"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gmp="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
