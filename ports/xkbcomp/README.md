# xkbcomp 1.5.0

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

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xkbcomp
apk add xkbcomp-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xkbcomp && /usr/local/bin/qnx-port-tests/test_xkbcomp    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xkbcomp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

[`test_xkbcomp`](../../tests/src/) links xkbcomp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xkbcomp`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

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

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xkbcomp 1.5.0-r0 | ✅ PASS | 2026-10-30 |

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
