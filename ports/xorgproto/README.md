# xorgproto 2024.1

> X Window System core and extension protocol headers

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/proto/xorgproto.git (tag `xorgproto-2024.1`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xorgproto`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xorgproto
apk add xorgproto-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xorgproto && /usr/local/bin/qnx-port-tests/test_xorgproto    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xorgproto`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xorgproto` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b xorgproto-2024.1 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/proto/xorgproto.git src/xorgproto
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xorgproto src/xorgproto \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dlegacy=false
ninja -C build/$QNX_ARCH/xorgproto
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xorgproto install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xorgproto`.

### 5. What it installs

169 files under `/usr/local`, 129 headers.

The complete list is `stage/<arch>/.manifest/xorgproto` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xorgproto
ls dists/8.0/*/ | grep '^xorgproto-'
```

This writes `xorgproto-<ver>-r0.tar.gz` (run time) and `xorgproto-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xorgproto
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xorgproto-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xorgproto-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xorgproto`](../../tests/src/) links xorgproto the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xorgproto`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xorgproto
/usr/local/bin/qnx-port-tests/test_xorgproto
```

```
== xorgproto ==
  ok   ...
PASS xorgproto
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xorgproto` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xorgproto`](../../tests/src/) -- a program linked against xorgproto the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - X protocol 11.0
   - request opcodes
   - xReq 4 bytes, xCreateWindowReq 32 = sz_xCreateWindowReq
   - events and errors are 32 bytes, connection prefix
   - InternAtom request, GetProperty reply
   - event codes
   - event masks
   - core keysyms
   - Latin-1, Greek, Euro keysyms
   - XF86 media keysyms
   - XKB constants
   - RandR rotations
   - Render ops and picture types
   - XInput 2 events
   - Shape kinds
   - CreateWindow request: opcode byte 1, length 8 words

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xorgproto 2024.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xorgproto-2024.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xorgproto 2024.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xorgproto-2024.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2024.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xorgproto=<version>`), sources in `ported/xorgproto/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xorgproto`; port one with `scripts/port-update xorgproto`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2024.1; GIT=https://gitlab.freedesktop.org/xorg/proto/xorgproto.git; REF=xorgproto-2024.1
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlegacy=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
