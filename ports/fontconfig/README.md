# fontconfig 2.15.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/fontconfig/fontconfig.git (tag `2.15.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) [`freetype`](../freetype/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fontconfig`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fontconfig
apk add fontconfig-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-fontconfig && /usr/local/bin/qnx-port-tests/test_fontconfig    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fontconfig`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fontconfig` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype expat
```

<details><summary>Full build chain (6)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.15.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/fontconfig/fontconfig.git src/fontconfig
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/fontconfig src/fontconfig \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddoc=disabled \
      -Dtests=disabled \
      -Dtools=disabled \
      -Dnls=enabled \
      -Dcache-build=disabled \
      -Diconv=disabled
ninja -C build/$QNX_ARCH/fontconfig
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/fontconfig install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fontconfig`.

### 5. What it installs

79 files under `/usr/local`: 3 shared-library entries, 3 headers.

```
/usr/local/lib/libfontconfig.so
/usr/local/lib/libfontconfig.so.1
/usr/local/lib/libfontconfig.so.1.14.0
```

The complete list is `stage/<arch>/.manifest/fontconfig` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fontconfig
ls dists/8.0/*/ | grep '^fontconfig-'
```

This writes `fontconfig-<ver>-r0.tar.gz` (run time) and `fontconfig-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fontconfig
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fontconfig-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fontconfig-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fontconfig`](../../tests/src/) links fontconfig the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fontconfig`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fontconfig
/usr/local/bin/qnx-port-tests/test_fontconfig
```

```
== fontconfig ==
  ok   ...
PASS fontconfig
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fontconfig` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fontconfig`](../../tests/src/) -- a program linked against fontconfig the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - FcInit
   - FcConfigGetCurrent
   - at least one font directory is configured
   - FcNameParse
   - match names a font file
   - no-match is reported cleanly

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fontconfig 2.15.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fontconfig 2.15.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.15.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fontconfig=<version>`), sources in `ported/fontconfig/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fontconfig`; port one with `scripts/port-update fontconfig`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.15.0; GIT=https://gitlab.freedesktop.org/fontconfig/fontconfig.git; REF=2.15.0
DEPS="freetype expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled -Dtools=disabled
                   -Dnls=enabled -Dcache-build=disabled -Diconv=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
