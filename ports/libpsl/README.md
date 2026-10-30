# libpsl 0.21.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/rockdaboot/libpsl.git (tag `0.21.5`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libpsl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libpsl
apk add libpsl-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libpsl && /usr/local/bin/qnx-port-tests/test_libpsl    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libpsl`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libpsl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libidn2 libunistring libiconv
```

<details><summary>Full build chain (5)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.21.5 --recurse-submodules --shallow-submodules \
    https://github.com/rockdaboot/libpsl.git src/libpsl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libpsl src/libpsl \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Druntime=libidn2 \
      -Dbuiltin=true \
      -Dtests=false \
      -Ddocs=false
ninja -C build/$QNX_ARCH/libpsl
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libpsl install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libiconv

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libpsl`.

### 5. What it installs

10 files under `/usr/local`: 3 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libpsl.so
/usr/local/lib/libpsl.so.5
/usr/local/lib/libpsl.so.5.3.5
/usr/local/bin/psl
/usr/local/bin/psl-make-dafsa
```

The complete list is `stage/<arch>/.manifest/libpsl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libpsl
ls dists/8.0/*/ | grep '^libpsl-'
```

This writes `libpsl-<ver>-r0.tar.gz` (run time) and `libpsl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libpsl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libpsl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libpsl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libpsl`](../../tests/src/) links libpsl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libpsl`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libpsl
/usr/local/bin/qnx-port-tests/test_libpsl
```

```
== libpsl ==
  ok   ...
PASS libpsl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libpsl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libpsl`](../../tests/src/) -- a program linked against libpsl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - psl_builtin list is compiled in
   - \"com\" is a public suffix
   - \"co.uk\" is a public suffix
   - \"example.com\" is not
   - registrable domain of www.foo.co.uk
   - suffix list is populated
   - a cookie for co.uk is refused

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libpsl 0.21.5-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libpsl 0.21.5-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.21.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libpsl=<version>`), sources in `ported/libpsl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libpsl`; port one with `scripts/port-update libpsl`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.21.5; GIT=https://github.com/rockdaboot/libpsl.git; REF=0.21.5
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libidn2 libunistring libiconv"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Druntime=libidn2 -Dbuiltin=true -Dtests=false -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
