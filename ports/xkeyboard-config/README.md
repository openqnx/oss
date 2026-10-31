# xkeyboard-config 2.43

> XKB keyboard configuration data: rules, layouts, symbols

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config.git (tag `xkeyboard-config-2.43`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xkeyboard_config`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xkeyboard-config
apk add xkeyboard-config-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xkeyboard-config && /usr/local/bin/qnx-port-tests/test_xkeyboard_config    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xkeyboard-config`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xkeyboard-config` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b xkeyboard-config-2.43 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config.git src/xkeyboard-config
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xkeyboard-config src/xkeyboard-config \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dxorg-rules-symlinks=true \
      -Dcompat-rules=true
ninja -C build/$QNX_ARCH/xkeyboard-config
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xkeyboard-config install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Pure data: the XKB rules, symbols and geometry files libxkbcommon reads at runtime. Nothing is compiled, so the cross file only decides where it lands.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xkeyboard-config`.

### 5. What it installs

343 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/xkeyboard-config` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xkeyboard-config
ls dists/8.0/*/ | grep '^xkeyboard-config-'
```

This writes `xkeyboard-config-<ver>-r0.tar.gz` (run time) and `xkeyboard-config-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xkeyboard-config
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xkeyboard-config-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xkeyboard-config-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xkeyboard_config`](../../tests/src/) links xkeyboard-config the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xkeyboard-config`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xkeyboard-config
/usr/local/bin/qnx-port-tests/test_xkeyboard_config
```

```
== xkeyboard-config ==
  ok   ...
PASS xkeyboard-config
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xkeyboard_config` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xkeyboard_config`](../../tests/src/) -- a program linked against xkeyboard-config the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - XKB data (rules/evdev)
   - xkb context with only this data
   - keymaps us, de, fr, ru compiled
   - us: a, A, semicolon
   - us: shift+1 -> exclam
   - us: one layout, Shift modifier
   - de: QWERTZ (Y key -> z, Z key -> y)
   - de: odiaeresis, dead_acute
   - fr: AZERTY (Q key -> a, 1 -> ampersand)
   - ru: Cyrillic ef, SHORTI
   - us,de keymap: 2 layouts
   - group 2 (de): Y key -> z
   - keymap serialised to text
   - and compiled back
   - rules/evdev.xml lists us, de, fr

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xkeyboard-config 2.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xkeyboard-config-2.43-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xkeyboard-config 2.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xkeyboard-config-2.43-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.43**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xkeyboard-config=<version>`), sources in `ported/xkeyboard-config/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xkeyboard-config`; port one with `scripts/port-update xkeyboard-config`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.43
GIT=https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config.git; REF=xkeyboard-config-2.43
BUILDSYS=meson
# Pure data: the XKB rules, symbols and geometry files libxkbcommon reads at
# runtime. Nothing is compiled, so the cross file only decides where it lands.
MESON_ARGS_EXTRA=( -Dxorg-rules-symlinks=true -Dcompat-rules=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
