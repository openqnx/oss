# libffi 3.4.6

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libffi/libffi/releases/download/v3.4.6/libffi-3.4.6.tar.gz |
| SHA-256 | `b0dea9df23c863a7a50e825440f3ebffabd65df1497108e5d437747843895a4e` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libffi`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libffi
apk add libffi-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libffi && /usr/local/bin/qnx-port-tests/test_libffi    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libffi`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libffi` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/libffi/libffi/releases/download/v3.4.6/libffi-3.4.6.tar.gz
mkdir -p dist src/libffi
curl -fL -o dist/$(basename $URL) $URL
echo "b0dea9df23c863a7a50e825440f3ebffabd65df1497108e5d437747843895a4e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libffi --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libffi < ports/libffi/patches/0001-qnx-executable-closures.patch
```

**[`0001-qnx-executable-closures.patch`](patches/0001-qnx-executable-closures.patch)** — libffi: allocate closures from executable mappings on QNX.

> QNX never makes heap pages executable, so closures allocated from malloc()
> faulted when called. Set FFI_MMAP_EXEC_WRIT so dlmmap() uses an anonymous
> PROT_EXEC|PROT_WRITE mapping, which QNX allows.

Touches: `src/closures.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libffi && cd build/$QNX_ARCH/libffi
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libffi/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-docs \
    --disable-multi-os-directory
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libffi`.

### 5. What it installs

9 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libffi.so
/usr/local/lib/libffi.so.9
```

The complete list is `stage/<arch>/.manifest/libffi` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libffi
ls dists/8.0/*/ | grep '^libffi-'
```

This writes `libffi-<ver>-r0.tar.gz` (run time) and `libffi-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libffi
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libffi-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libffi-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libffi`](../../tests/src/) links libffi the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libffi`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libffi
/usr/local/bin/qnx-port-tests/test_libffi
```

```
== libffi ==
  ok   ...
PASS libffi
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libffi` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libffi`](../../tests/src/) -- a program linked against libffi the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ffi_prep_cif for int(int,int,int)
   - ffi_call returned 42
   - ffi_prep_cif for double(double,float)
   - float/double call returned 10.0
   - ffi_closure_alloc
   - ffi_prep_cif for the closure
   - ffi_prep_closure_loc
   - calling through the closure returned 42

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libffi 3.4.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libffi-3.4.6-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libffi 3.4.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libffi-3.4.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.4.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libffi=<version>`), sources in `ported/libffi/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libffi`; port one with `scripts/port-update libffi`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.4.6
TARBALL=https://github.com/libffi/libffi/releases/download/v3.4.6/libffi-3.4.6.tar.gz
SHA256=b0dea9df23c863a7a50e825440f3ebffabd65df1497108e5d437747843895a4e
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-docs --disable-multi-os-directory )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
