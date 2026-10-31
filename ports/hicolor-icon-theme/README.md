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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `hicolor-icon-theme` (programs, shared libraries, run-time data) and `hicolor-icon-theme-dev` (headers, pkg-config/CMake files, static libraries; needs `hicolor-icon-theme`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`hicolor-icon-theme`](https://openqnx.github.io/oss/package/x86_64/hicolor-icon-theme.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hicolor-icon-theme.html)) · [`hicolor-icon-theme-dev`](https://openqnx.github.io/oss/package/x86_64/hicolor-icon-theme-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hicolor-icon-theme-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add hicolor-icon-theme` (dependencies come along); one version: `apk add hicolor-icon-theme=<version>` | `apk del hicolor-icon-theme` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install hicolor-icon-theme` (dependencies first) | `oss-get remove hicolor-icon-theme` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`hicolor-icon-theme-dev` the same way, by its own name (`apk add hicolor-icon-theme-dev`, `oss-get install hicolor-icon-theme-dev`, by hand with its own file and list `hicolor-icon-theme-dev.list`); remove it before `hicolor-icon-theme`, which it needs.

```sh
# on the QNX target, as root
apk add hicolor-icon-theme hicolor-icon-theme-dev
apk add qnx-test-hicolor-icon-theme && /usr/local/bin/qnx-port-tests/test_hicolor_icon_theme    # its on-target test
apk del hicolor-icon-theme hicolor-icon-theme-dev qnx-test-hicolor-icon-theme
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/hicolor-icon-theme.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/hicolor-icon-theme.list
rm var/lib/manual/hicolor-icon-theme.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hicolor-icon-theme 0.18-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/hicolor-icon-theme-0.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hicolor-icon-theme 0.18-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/hicolor-icon-theme-0.18-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hicolor-icon-theme 0.18-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/hicolor-icon-theme-0.18-r0.log) |

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
