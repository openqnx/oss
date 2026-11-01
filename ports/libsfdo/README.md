# libsfdo 0.1.4

> Libraries implementing freedesktop.org desktop entry, icon and basedir specs

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/vyivel/libsfdo.git (tag `v0.1.4`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libsfdo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libsfdo` (programs, shared libraries, run-time data) and `libsfdo-dev` (headers, pkg-config/CMake files, static libraries; needs `libsfdo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libsfdo`](https://openqnx.github.io/oss/package/x86_64/libsfdo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsfdo.html)) · [`libsfdo-dev`](https://openqnx.github.io/oss/package/x86_64/libsfdo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsfdo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libsfdo` (dependencies come along); one version: `apk add libsfdo=<version>` | `apk del libsfdo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libsfdo` (dependencies first) | `oss-get remove libsfdo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libsfdo-dev` the same way, by its own name (`apk add libsfdo-dev`, `oss-get install libsfdo-dev`, by hand with its own file and list `libsfdo-dev.list`); remove it before `libsfdo`, which it needs.

```sh
# on the QNX target, as root
apk add libsfdo libsfdo-dev
apk add qnx-test-libsfdo && /usr/local/bin/qnx-port-tests/test_libsfdo    # its on-target test, from tests-8.0-<arch>
apk del libsfdo libsfdo-dev qnx-test-libsfdo
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libsfdo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libsfdo.list
rm var/lib/manual/libsfdo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libsfdo` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.1.4 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/vyivel/libsfdo.git src/libsfdo
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libsfdo src/libsfdo \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dexamples=false \
      -Dtests=false
ninja -C build/$QNX_ARCH/libsfdo
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libsfdo install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libsfdo: freedesktop.org desktop entry, icon theme and base directory specifications (labwc's menus and icons).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libsfdo`.

### 5. What it installs

22 files under `/usr/local`: 8 shared-library entries, 6 headers.

```
/usr/local/lib/libsfdo-basedir.so
/usr/local/lib/libsfdo-basedir.so.0
/usr/local/lib/libsfdo-desktop-file.so
/usr/local/lib/libsfdo-desktop-file.so.0
/usr/local/lib/libsfdo-desktop.so
/usr/local/lib/libsfdo-desktop.so.0
/usr/local/lib/libsfdo-icon.so
/usr/local/lib/libsfdo-icon.so.0
```

The complete list is `stage/<arch>/.manifest/libsfdo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libsfdo
ls dists/8.0/*/ | grep '^libsfdo-'
```

This writes `libsfdo-<ver>-r0.tar.gz` (run time) and `libsfdo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libsfdo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libsfdo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libsfdo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libsfdo`](../../tests/src/) links libsfdo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libsfdo`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libsfdo
/usr/local/bin/qnx-port-tests/test_libsfdo
```

```
== libsfdo ==
  ok   ...
PASS libsfdo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libsfdo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libsfdo`](../../tests/src/) -- a program linked against libsfdo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary data directory
   - sfdo_basedir_ctx_create
   - sfdo_desktop_ctx_create
   - sfdo_desktop_db_load_from
   - two entries found
   - entry qnx-demo by id
   - localized name (de)
   - type Application
   - Exec parsed
   - Icon
   - two categories
   - NoDisplay entry flagged

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsfdo 0.1.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libsfdo-0.1.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsfdo 0.1.4-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/x86_64/libsfdo-0.1.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsfdo 0.1.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libsfdo-0.1.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.1.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libsfdo=<version>`), sources in `ported/libsfdo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libsfdo`; port one with `scripts/port-update libsfdo`.

<details><summary>recipe.sh</summary>

```bash
# libsfdo: freedesktop.org desktop entry, icon theme and base directory
# specifications (labwc's menus and icons).
VERSION=0.1.4; GIT=https://gitlab.freedesktop.org/vyivel/libsfdo.git; REF=v0.1.4
DEPS=""
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dexamples=false -Dtests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
