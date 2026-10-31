# hicolor-icon-theme 0.18

> Freedesktop.org default fallback icon theme

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/default-icon-theme.git (tag `v0.18`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_hicolor_icon_theme`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add hicolor-icon-theme
apk add hicolor-icon-theme-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-hicolor-icon-theme && /usr/local/bin/qnx-port-tests/test_hicolor_icon_theme    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install hicolor-icon-theme`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hicolor-icon-theme` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.18 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xdg/default-icon-theme.git src/hicolor-icon-theme
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/hicolor-icon-theme src/hicolor-icon-theme \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/hicolor-icon-theme
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/hicolor-icon-theme install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The freedesktop.org fallback icon theme (directory structure; data only).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hicolor-icon-theme`.

### 5. What it installs

2 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/hicolor-icon-theme` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hicolor-icon-theme
ls dists/8.0/*/ | grep '^hicolor-icon-theme-'
```

This writes `hicolor-icon-theme-<ver>-r0.tar.gz` (run time) and `hicolor-icon-theme-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hicolor-icon-theme
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hicolor-icon-theme-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hicolor-icon-theme-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_hicolor_icon_theme`](../../tests/src/) links hicolor-icon-theme the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-hicolor-icon-theme`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-hicolor-icon-theme
/usr/local/bin/qnx-port-tests/test_hicolor_icon_theme
```

```
== hicolor-icon-theme ==
  ok   ...
PASS hicolor-icon-theme
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_hicolor_icon_theme` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_hicolor_icon_theme`](../../tests/src/) -- a program linked against hicolor-icon-theme the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - share/icons/hicolor/index.theme installed
   - index.theme parses as a key file
   - Name=Hicolor, Hidden=true
   - 300+ directories, each with its section
   - every section has a Context and a Size
   - @2 directories have Scale=2 and the base size
   - scalable/* directories: Type=Scalable, MaxSize >= 256
   - 48x48/apps, 16x16/mimetypes, 256x256/places, symbolic/apps as expected

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hicolor-icon-theme 0.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/hicolor-icon-theme-0.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hicolor-icon-theme 0.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/hicolor-icon-theme-0.18-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hicolor-icon-theme=<version>`), sources in `ported/hicolor-icon-theme/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hicolor-icon-theme`; port one with `scripts/port-update hicolor-icon-theme`.

<details><summary>recipe.sh</summary>

```bash
# The freedesktop.org fallback icon theme (directory structure; data only).
VERSION=0.18; GIT=https://gitlab.freedesktop.org/xdg/default-icon-theme.git; REF=v0.18
BUILDSYS=meson
MESON_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
