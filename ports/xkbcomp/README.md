# xkbcomp 1.5.0

> XKB keymap compiler

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/app/xkbcomp-1.5.0.tar.xz |
| SHA-256 | `2ac31f26600776db6d9cd79b3fcd272263faebac7eb85fb2f33c7141b8486060` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`libxkbfile`](../libxkbfile/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`libx11`](../libx11/) [`libxkbfile`](../libxkbfile/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xkbcomp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `xkbcomp` (programs, shared libraries, run-time data) and `xkbcomp-dev` (headers, pkg-config/CMake files, static libraries; needs `xkbcomp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xkbcomp`](https://openqnx.github.io/oss/package/x86_64/xkbcomp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xkbcomp.html)) · [`xkbcomp-dev`](https://openqnx.github.io/oss/package/x86_64/xkbcomp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xkbcomp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xkbcomp` (dependencies come along); one version: `apk add xkbcomp=<version>` | `apk del xkbcomp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xkbcomp` (dependencies first) | `oss-get remove xkbcomp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`xkbcomp-dev` the same way, by its own name (`apk add xkbcomp-dev`, `oss-get install xkbcomp-dev`, by hand with its own file and list `xkbcomp-dev.list`); remove it before `xkbcomp`, which it needs.

```sh
# on the QNX target, as root
apk add xkbcomp xkbcomp-dev
apk add qnx-test-xkbcomp && /usr/local/bin/qnx-port-tests/test_xkbcomp    # its on-target test, from tests-8.0-<arch>-m-z
apk del xkbcomp xkbcomp-dev qnx-test-xkbcomp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xkbcomp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xkbcomp.list
rm var/lib/manual/xkbcomp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xkbcomp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11 libxkbfile xorgproto
```

<details><summary>Full build chain (9)</summary>

[`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxkbfile`](../libxkbfile/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
URL=https://xorg.freedesktop.org/releases/individual/app/xkbcomp-1.5.0.tar.xz
mkdir -p dist src/xkbcomp
curl -fL -o dist/$(basename $URL) $URL
echo "2ac31f26600776db6d9cd79b3fcd272263faebac7eb85fb2f33c7141b8486060  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xkbcomp --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xkbcomp src/xkbcomp \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/xkbcomp
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xkbcomp install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xkbcomp: the XKB keymap compiler (keymap source -> compiled keymap; X servers run it).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xkbcomp`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xkbcomp
```

The complete list is `stage/<arch>/.manifest/xkbcomp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xkbcomp
ls dists/8.0/*/ | grep '^xkbcomp-'
```

This writes `xkbcomp-<ver>-r0.tar.gz` (run time) and `xkbcomp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xkbcomp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xkbcomp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xkbcomp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xkbcomp`](../../tests/src/) links xkbcomp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xkbcomp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xkbcomp
/usr/local/bin/qnx-port-tests/test_xkbcomp
```

```
== xkbcomp ==
  ok   ...
PASS xkbcomp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xkbcomp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xkbcomp`](../../tests/src/) -- a program linked against xkbcomp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xkbcomp-1.5.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xkbcomp-1.5.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xkbcomp-1.5.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xkbcomp-1.5.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.5.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xkbcomp=<version>`), sources in `ported/xkbcomp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xkbcomp`; port one with `scripts/port-update xkbcomp`.

<details><summary>recipe.sh</summary>

```bash
# xkbcomp: the XKB keymap compiler (keymap source -> compiled keymap; X servers run it).
VERSION=1.5.0
TARBALL=https://xorg.freedesktop.org/releases/individual/app/xkbcomp-1.5.0.tar.xz
SHA256=2ac31f26600776db6d9cd79b3fcd272263faebac7eb85fb2f33c7141b8486060
DEPS="libx11 libxkbfile xorgproto"
BUILDSYS=meson
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
