# libvpx 1.15.0

> WebM VP8/VP9 video codec library and tools

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/webm/libvpx (tag `v1.15.0`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvpx`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libvpx` (programs, shared libraries, run-time data) and `libvpx-dev` (headers, pkg-config/CMake files, static libraries; needs `libvpx`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libvpx`](https://openqnx.github.io/oss/package/x86_64/libvpx.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvpx.html)) · [`libvpx-dev`](https://openqnx.github.io/oss/package/x86_64/libvpx-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvpx-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libvpx` (dependencies come along); one version: `apk add libvpx=<version>` | `apk del libvpx` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libvpx` (dependencies first) | `oss-get remove libvpx` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libvpx-dev` the same way, by its own name (`apk add libvpx-dev`, `oss-get install libvpx-dev`, by hand with its own file and list `libvpx-dev.list`); remove it before `libvpx`, which it needs.

```sh
# on the QNX target, as root
apk add libvpx libvpx-dev
apk add qnx-test-libvpx && /usr/local/bin/qnx-port-tests/test_libvpx    # its on-target test, from tests-8.0-<arch>
apk del libvpx libvpx-dev qnx-test-libvpx
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libvpx.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libvpx.list
rm var/lib/manual/libvpx.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvpx` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.15.0 --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/webm/libvpx src/libvpx
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libvpx; B=build/$QNX_ARCH/libvpx; mkdir -p $B
```

```bash
do_configure() {
  local tgt extra=()
  case "$QNX_ARCH" in
    aarch64le) tgt=arm64-linux-gcc      # generic ELF/aarch64 profile; CROSS gives the real toolchain
               # libvpx's aarch64 runtime CPU detection reads Linux hwcaps
               # (getauxval); there is no QNX method, and the file says so:
               # "Rerun configure with --disable-runtime-cpu-detect". Without
               # detection every extension that is compiled in is called
               # unconditionally, so keep to what all ARMv8.0 parts have: the
               # dotprod/i8mm/SVE paths died with SIGILL on a Cortex-A57.
               extra+=( --disable-runtime-cpu-detect --disable-neon_dotprod --disable-neon_i8mm
                        --disable-sve --disable-sve2 ) ;;
    x86_64)    tgt=x86_64-linux-gcc ;;  # x86 detection is cpuid, which works anywhere
  esac
  cd "$B"
  CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
  CFLAGS="-D_QNX_SOURCE" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --prefix="$PREFIX" --target="$tgt" \
    --enable-pic --enable-shared --disable-static \
    --disable-examples --disable-tools --disable-docs --disable-unit-tests \
    --enable-vp8 --enable-vp9 --disable-webm-io \
    "${extra[@]}"
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DIST_DIR="$STAGE$PREFIX" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libvpx has a hand-written configure driven by a --target triple of its own.
- libvpx installs into DIST_DIR, not DESTDIR: passing DESTDIR alone silently produces no files in the staging tree.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvpx`.

### 5. What it installs

16 files under `/usr/local`: 4 shared-library entries, 11 headers.

```
/usr/local/lib/libvpx.so
/usr/local/lib/libvpx.so.9
/usr/local/lib/libvpx.so.9.1
/usr/local/lib/libvpx.so.9.1.0
```

The complete list is `stage/<arch>/.manifest/libvpx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libvpx
ls dists/8.0/*/ | grep '^libvpx-'
```

This writes `libvpx-<ver>-r0.tar.gz` (run time) and `libvpx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libvpx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libvpx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libvpx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libvpx`](../../tests/src/) links libvpx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libvpx`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libvpx
/usr/local/bin/qnx-port-tests/test_libvpx
```

```
== libvpx ==
  ok   ...
PASS libvpx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvpx` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libvpx`](../../tests/src/) -- a program linked against libvpx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - enc_config_default
   - vpx_codec_enc_init
   - vpx_codec_dec_init
   - vpx_img_alloc
   - compressed packets produced
   - frames decoded back at the right size

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libvpx 1.15.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libvpx-1.15.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libvpx 1.15.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libvpx-1.15.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.15.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libvpx=<version>`), sources in `ported/libvpx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libvpx`; port one with `scripts/port-update libvpx`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.15.0; GIT=https://chromium.googlesource.com/webm/libvpx; REF=v1.15.0
BUILDSYS=custom
# libvpx has a hand-written configure driven by a --target triple of its own.
do_configure() {
  local tgt extra=()
  case "$QNX_ARCH" in
    aarch64le) tgt=arm64-linux-gcc      # generic ELF/aarch64 profile; CROSS gives the real toolchain
               # libvpx's aarch64 runtime CPU detection reads Linux hwcaps
               # (getauxval); there is no QNX method, and the file says so:
               # "Rerun configure with --disable-runtime-cpu-detect". Without
               # detection every extension that is compiled in is called
               # unconditionally, so keep to what all ARMv8.0 parts have: the
               # dotprod/i8mm/SVE paths died with SIGILL on a Cortex-A57.
               extra+=( --disable-runtime-cpu-detect --disable-neon_dotprod --disable-neon_i8mm
                        --disable-sve --disable-sve2 ) ;;
    x86_64)    tgt=x86_64-linux-gcc ;;  # x86 detection is cpuid, which works anywhere
  esac
  cd "$B"
  CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
  CFLAGS="-D_QNX_SOURCE" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --prefix="$PREFIX" --target="$tgt" \
    --enable-pic --enable-shared --disable-static \
    --disable-examples --disable-tools --disable-docs --disable-unit-tests \
    --enable-vp8 --enable-vp9 --disable-webm-io \
    "${extra[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
# libvpx installs into DIST_DIR, not DESTDIR: passing DESTDIR alone silently
# produces no files in the staging tree.
do_install() { make -C "$B" DIST_DIR="$STAGE$PREFIX" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
