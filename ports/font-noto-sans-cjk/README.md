# font-noto-sans-cjk 2.004

> Noto Sans CJK fonts for Chinese, Japanese and Korean

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/notofonts/noto-cjk/releases/download/Sans2.004/03_NotoSansCJK-OTC.zip |
| SHA-256 | `528f4e1b25ff3badb0321b38d015d954c4c0de926c7830ef50e4a1948f6a3eed` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_font_noto_sans_cjk`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add font-noto-sans-cjk
apk add font-noto-sans-cjk-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-font-noto-sans-cjk && /usr/local/bin/qnx-port-tests/test_font_noto_sans_cjk    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install font-noto-sans-cjk`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild font-noto-sans-cjk` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/notofonts/noto-cjk/releases/download/Sans2.004/03_NotoSansCJK-OTC.zip
mkdir -p dist src/font-noto-sans-cjk
curl -fL -o dist/$(basename $URL) $URL
echo "528f4e1b25ff3badb0321b38d015d954c4c0de926c7830ef50e4a1948f6a3eed  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/font-noto-sans-cjk --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/font-noto-sans-cjk; B=build/$QNX_ARCH/font-noto-sans-cjk; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_fonts_install font-noto-sans-cjk; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Noto Sans CJK (Chinese, Japanese, Korean). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH font-noto-sans-cjk`.

### 5. What it installs

7 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/font-noto-sans-cjk` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa font-noto-sans-cjk
ls dists/8.0/*/ | grep '^font-noto-sans-cjk-'
```

This writes `font-noto-sans-cjk-<ver>-r0.tar.gz` (run time) and `font-noto-sans-cjk-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add font-noto-sans-cjk
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/font-noto-sans-cjk-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/font-noto-sans-cjk-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_font_noto_sans_cjk`](../../tests/src/) links font-noto-sans-cjk the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-font-noto-sans-cjk`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-font-noto-sans-cjk
/usr/local/bin/qnx-port-tests/test_font_noto_sans_cjk
```

```
== font-noto-sans-cjk ==
  ok   ...
PASS font-noto-sans-cjk
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_font_noto_sans_cjk` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_font_noto_sans_cjk`](../../tests/src/) -- a program linked against font-noto-sans-cjk the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | font-noto-sans-cjk 2.004-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/font-noto-sans-cjk-2.004-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | font-noto-sans-cjk 2.004-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/font-noto-sans-cjk-2.004-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | font-noto-sans-cjk 2.004-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/font-noto-sans-cjk-2.004-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | font-noto-sans-cjk 2.004-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/font-noto-sans-cjk-2.004-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.004**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add font-noto-sans-cjk=<version>`), sources in `ported/font-noto-sans-cjk/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates font-noto-sans-cjk`; port one with `scripts/port-update font-noto-sans-cjk`.

<details><summary>recipe.sh</summary>

```bash
# Noto Sans CJK (Chinese, Japanese, Korean). Font files only. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.004
TARBALL=https://github.com/notofonts/noto-cjk/releases/download/Sans2.004/03_NotoSansCJK-OTC.zip
SHA256=528f4e1b25ff3badb0321b38d015d954c4c0de926c7830ef50e4a1948f6a3eed
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_build() { :; }
do_install() { qnx_fonts_install font-noto-sans-cjk; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
