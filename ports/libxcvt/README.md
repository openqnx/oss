# libxcvt 0.1.3

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/lib/libxcvt-0.1.3.tar.xz |
| SHA-256 | `a929998a8767de7dfa36d6da4751cdbeef34ed630714f2f4a767b351f2442e01` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xorg_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxcvt
apk add libxcvt-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xorg-libs && /usr/local/bin/qnx-port-tests/test_xorg_libs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxcvt`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxcvt` performs exactly the same steps (2–5) in one command.

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
URL=https://xorg.freedesktop.org/releases/individual/lib/libxcvt-0.1.3.tar.xz
mkdir -p dist src/libxcvt
curl -fL -o dist/$(basename $URL) $URL
echo "a929998a8767de7dfa36d6da4751cdbeef34ed630714f2f4a767b351f2442e01  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxcvt --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libxcvt src/libxcvt \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/libxcvt
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libxcvt install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libxcvt: VESA CVT standard timing modelines (the cvt program and the library X servers and Wayland compositors compute display modes with).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxcvt`.

### 5. What it installs

8 files under `/usr/local`: 3 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libxcvt.so
/usr/local/lib/libxcvt.so.0
/usr/local/lib/libxcvt.so.0.1.3
/usr/local/bin/cvt
```

The complete list is `stage/<arch>/.manifest/libxcvt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxcvt
ls dists/8.0/*/ | grep '^libxcvt-'
```

This writes `libxcvt-<ver>-r0.tar.gz` (run time) and `libxcvt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxcvt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxcvt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxcvt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xorg_libs`](../../tests/src/) links libxcvt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xorg-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xorg-libs
/usr/local/bin/qnx-port-tests/test_xorg_libs
```

```
== libxcvt ==
  ok   ...
PASS libxcvt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xorg_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xorg_libs`](../../tests/src/) -- a program linked against libxcvt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libxcvt_gen_mode_info
   - horizontal timing = cvt's (173.00 MHz, 2048 2248 2576)
   - vertical timing = cvt's (1083 1088 1120)
   - xshmfence_alloc_shm
   - xshmfence_map_shm
   - new fence not triggered
   - triggered
   - await on a triggered fence returns
   - reset
   - xcb_event_get_label(KEY_PRESS) = KeyPress
   - XmuCopyISOLatin1Lowered (incl. E-acute)
   - XmuCompareISOLatin1 ignores case
   - IceListenForConnections
   - network id list
   - no session manager: the documented error
   - XtCreateApplicationContext
   - XtNewString
   - XkbRF_LoadRules
   - XkbRF_GetComponents
   - keycodes = evdev
   - symbols = pc+us
   - geometry = pc(pc105)
   - xfont2_make_font_pattern_cache
   - cached pattern found
   - other pattern not found

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.1.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxcvt=<version>`), sources in `ported/libxcvt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxcvt`; port one with `scripts/port-update libxcvt`.

<details><summary>recipe.sh</summary>

```bash
# libxcvt: VESA CVT standard timing modelines (the cvt program and the library
# X servers and Wayland compositors compute display modes with).
VERSION=0.1.3
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libxcvt-0.1.3.tar.xz
SHA256=a929998a8767de7dfa36d6da4751cdbeef34ed630714f2f4a767b351f2442e01
DEPS=""
BUILDSYS=meson
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
