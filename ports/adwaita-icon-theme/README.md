# adwaita-icon-theme 51.0

> GNOME's Adwaita icon theme and default cursors

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/adwaita-icon-theme/51/adwaita-icon-theme-51.0.tar.xz |
| SHA-256 | `ba561cf3c96305a47179fa5605856ad695c9238ea07f7093aebd9ba80c4e323b` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`hicolor-icon-theme`](../hicolor-icon-theme/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_adwaita`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `adwaita-icon-theme` (programs, shared libraries, run-time data) and `adwaita-icon-theme-dev` (headers, pkg-config/CMake files, static libraries; needs `adwaita-icon-theme`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`adwaita-icon-theme`](https://openqnx.github.io/oss/package/x86_64/adwaita-icon-theme.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/adwaita-icon-theme.html)) · [`adwaita-icon-theme-dev`](https://openqnx.github.io/oss/package/x86_64/adwaita-icon-theme-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/adwaita-icon-theme-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add adwaita-icon-theme` (dependencies come along); one version: `apk add adwaita-icon-theme=<version>` | `apk del adwaita-icon-theme` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install adwaita-icon-theme` (dependencies first) | `oss-get remove adwaita-icon-theme` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`adwaita-icon-theme-dev` the same way, by its own name (`apk add adwaita-icon-theme-dev`, `oss-get install adwaita-icon-theme-dev`, by hand with its own file and list `adwaita-icon-theme-dev.list`); remove it before `adwaita-icon-theme`, which it needs.

```sh
# on the QNX target, as root
apk add adwaita-icon-theme adwaita-icon-theme-dev
apk add qnx-test-adwaita && /usr/local/bin/qnx-port-tests/test_adwaita    # its on-target test, from tests-8.0-<arch>
apk del adwaita-icon-theme adwaita-icon-theme-dev qnx-test-adwaita
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/adwaita-icon-theme.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/adwaita-icon-theme.list
rm var/lib/manual/adwaita-icon-theme.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild adwaita-icon-theme` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild hicolor-icon-theme
```

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/adwaita-icon-theme/51/adwaita-icon-theme-51.0.tar.xz
mkdir -p dist src/adwaita-icon-theme
curl -fL -o dist/$(basename $URL) $URL
echo "ba561cf3c96305a47179fa5605856ad695c9238ea07f7093aebd9ba80c4e323b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/adwaita-icon-theme --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/adwaita-icon-theme src/adwaita-icon-theme \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/adwaita-icon-theme
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/adwaita-icon-theme install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- adwaita-icon-theme: GNOME's icon theme (and default cursors), the icons GTK applications ask for by name. Data only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH adwaita-icon-theme`.

### 5. What it installs

836 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/adwaita-icon-theme` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa adwaita-icon-theme
ls dists/8.0/*/ | grep '^adwaita-icon-theme-'
```

This writes `adwaita-icon-theme-<ver>-r0.tar.gz` (run time) and `adwaita-icon-theme-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add adwaita-icon-theme
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/adwaita-icon-theme-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/adwaita-icon-theme-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_adwaita`](../../tests/src/) links adwaita-icon-theme the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-adwaita`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-adwaita
/usr/local/bin/qnx-port-tests/test_adwaita
```

```
== adwaita-icon-theme ==
  ok   ...
PASS adwaita-icon-theme
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_adwaita` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_adwaita`](../../tests/src/) -- a program linked against adwaita-icon-theme the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | adwaita-icon-theme 51.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/adwaita-icon-theme-51.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | adwaita-icon-theme 51.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/adwaita-icon-theme-51.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **51.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add adwaita-icon-theme=<version>`), sources in `ported/adwaita-icon-theme/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates adwaita-icon-theme`; port one with `scripts/port-update adwaita-icon-theme`.

<details><summary>recipe.sh</summary>

```bash
# adwaita-icon-theme: GNOME's icon theme (and default cursors), the icons GTK
# applications ask for by name. Data only.
VERSION=51.0
TARBALL=https://download.gnome.org/sources/adwaita-icon-theme/51/adwaita-icon-theme-51.0.tar.xz
SHA256=ba561cf3c96305a47179fa5605856ad695c9238ea07f7093aebd9ba80c4e323b
DEPS="hicolor-icon-theme"
RUNTIME_DEPS="hicolor-icon-theme"
BUILDSYS=meson
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
