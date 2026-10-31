# linux-headers-input-codes 6.12

> Linux input event codes header (input-event-codes.h), header only

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/include/uapi/linux/input-event-codes.h?h=v6.12 |
| SHA-256 | `f9889a73d7d4f7e303d2d346a3622657169c5e8d7d354cb7a030ff47a7fdfdee` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_linux_headers_input_codes`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add linux-headers-input-codes
apk add linux-headers-input-codes-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-linux-headers-input-codes && /usr/local/bin/qnx-port-tests/test_linux_headers_input_codes    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install linux-headers-input-codes`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild linux-headers-input-codes` performs exactly the same steps (2–5) in one command.

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
URL=https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/include/uapi/linux/input-event-codes.h?h=v6.12
mkdir -p dist src/linux-headers-input-codes
curl -fL -o dist/$(basename $URL) $URL
echo "f9889a73d7d4f7e303d2d346a3622657169c5e8d7d354cb7a030ff47a7fdfdee  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/linux-headers-input-codes --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/linux-headers-input-codes; B=build/$QNX_ARCH/linux-headers-input-codes; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() { install -Dm644 "$S/input-event-codes.h" "$STAGE$PREFIX/include/linux/input-event-codes.h"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- linux-headers-input-codes: the Linux input event codes (KEY_*, BTN_*, ...) as one header, linux/input-event-codes.h -- defines only, the shared vocabulary of Wayland input (wlroots' QNX Screen backend maps Screen events to them, Wayland clients read them). QNX ships it under this name too. GPL-2.0 WITH Linux-syscall-note: usable from any program.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH linux-headers-input-codes`.

### 5. What it installs

1 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/linux-headers-input-codes` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa linux-headers-input-codes
ls dists/8.0/*/ | grep '^linux-headers-input-codes-'
```

This writes `linux-headers-input-codes-<ver>-r0.tar.gz` (run time) and `linux-headers-input-codes-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add linux-headers-input-codes
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/linux-headers-input-codes-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/linux-headers-input-codes-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_linux_headers_input_codes`](../../tests/src/) links linux-headers-input-codes the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-linux-headers-input-codes`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-linux-headers-input-codes
/usr/local/bin/qnx-port-tests/test_linux_headers_input_codes
```

```
== linux-headers-input-codes ==
  ok   ...
PASS linux-headers-input-codes
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_linux_headers_input_codes` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_linux_headers_input_codes`](../../tests/src/) -- a program linked against linux-headers-input-codes the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - event types SYN KEY REL ABS, EV_MAX
   - KEY_ESC 1, KEY_A 30, KEY_Z 44, ENTER 28, SPACE 57
   - shift, F1, F12, KEY_MAX 0x2ff
   - BTN_LEFT 0x110, BTN_TOUCH 0x14a, BTN_SOUTH = BTN_GAMEPAD
   - REL_WHEEL 8, multitouch ABS_MT_* codes
   - SYN, INPUT_PROP, SW, LED codes
   - US keymap compiled
   - KEY_A + 8 -> keysym a
   - KEY_ENTER + 8 -> Return
   - KEY_1 + 8 -> 1
   - with KEY_LEFTSHIFT held: KEY_A -> A

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | linux-headers-input-codes 6.12-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/linux-headers-input-codes-6.12-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | linux-headers-input-codes 6.12-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/linux-headers-input-codes-6.12-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | linux-headers-input-codes 6.12-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/linux-headers-input-codes-6.12-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | linux-headers-input-codes 6.12-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/linux-headers-input-codes-6.12-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.12**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add linux-headers-input-codes=<version>`), sources in `ported/linux-headers-input-codes/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates linux-headers-input-codes`; port one with `scripts/port-update linux-headers-input-codes`.

<details><summary>recipe.sh</summary>

```bash
# linux-headers-input-codes: the Linux input event codes (KEY_*, BTN_*, ...)
# as one header, linux/input-event-codes.h -- defines only, the shared
# vocabulary of Wayland input (wlroots' QNX Screen backend maps Screen events
# to them, Wayland clients read them). QNX ships it under this name too.
# GPL-2.0 WITH Linux-syscall-note: usable from any program.
VERSION=6.12
TARBALL="https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/include/uapi/linux/input-event-codes.h?h=v6.12"
FETCH_NAME=input-event-codes-6.12.h
SHA256=f9889a73d7d4f7e303d2d346a3622657169c5e8d7d354cb7a030ff47a7fdfdee
DEPS=""
BUILDSYS=custom
do_fetch() {
  local a="$WS/dist/$FETCH_NAME"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$a" ] || curl -fsSL -o "$a" "$TARBALL" || return 1
  echo "$SHA256  $a" | sha256sum -c --status || { echo "checksum mismatch: $a"; rm -f "$a"; return 1; }
  cp "$a" "$SRCROOT/input-event-codes.h"
}
do_configure() { :; }
do_build() { :; }
do_install() { install -Dm644 "$S/input-event-codes.h" "$STAGE$PREFIX/include/linux/input-event-codes.h"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
