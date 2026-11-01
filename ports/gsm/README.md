# gsm 1.0.22

> GSM 06.10 lossy speech compression library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.quut.com/gsm/gsm-1.0.22.tar.gz |
| SHA-256 | `f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_misc_libs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gsm` (programs, shared libraries, run-time data) and `gsm-dev` (headers, pkg-config/CMake files, static libraries; needs `gsm`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gsm`](https://openqnx.github.io/oss/package/x86_64/gsm.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gsm.html)) · [`gsm-dev`](https://openqnx.github.io/oss/package/x86_64/gsm-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gsm-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gsm` (dependencies come along); one version: `apk add gsm=<version>` | `apk del gsm` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gsm` (dependencies first) | `oss-get remove gsm` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gsm-dev` the same way, by its own name (`apk add gsm-dev`, `oss-get install gsm-dev`, by hand with its own file and list `gsm-dev.list`); remove it before `gsm`, which it needs.

```sh
# on the QNX target, as root
apk add gsm gsm-dev
apk add qnx-test-misc-libs && /usr/local/bin/qnx-port-tests/test_misc_libs    # its on-target test, from tests-8.0-<arch>-m-z
apk del gsm gsm-dev qnx-test-misc-libs
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gsm.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gsm.list
rm var/lib/manual/gsm.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gsm` performs exactly the same steps (2–5) in one command.

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
URL=https://www.quut.com/gsm/gsm-1.0.22.tar.gz
mkdir -p dist src/gsm
curl -fL -o dist/$(basename $URL) $URL
echo "f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gsm --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gsm; B=build/$QNX_ARCH/gsm; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B/o"
  for f in "$S"/src/*.c; do
    "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
      -DSASR -DWAV49 -DNeedFunctionPrototypes=1 -I"$S/inc" \
      -c "$f" -o "$B/o/$(basename "${f%.c}").o"
  done
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libgsm.so.1 \
    -o "$B/libgsm.so.1.0.22" "$B"/o/*.o
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/gsm"
  install -m 755 "$B/libgsm.so.1.0.22" "$STAGE$PREFIX/lib/"
  ln -sfn libgsm.so.1.0.22 "$STAGE$PREFIX/lib/libgsm.so.1"
  ln -sfn libgsm.so.1      "$STAGE$PREFIX/lib/libgsm.so"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/gsm/"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The upstream Makefile hardcodes the host compiler and installs nowhere useful; the library is eight objects, so build it directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gsm`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libgsm.so
/usr/local/lib/libgsm.so.1
/usr/local/lib/libgsm.so.1.0.22
```

The complete list is `stage/<arch>/.manifest/gsm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gsm
ls dists/8.0/*/ | grep '^gsm-'
```

This writes `gsm-<ver>-r0.tar.gz` (run time) and `gsm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gsm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gsm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gsm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links gsm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-misc-libs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-misc-libs
/usr/local/bin/qnx-port-tests/test_misc_libs
```

```
== gsm ==
  ok   ...
PASS gsm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_misc_libs`](../../tests/src/) -- a program linked against gsm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - enca reports supported languages
   - enca_analyser_alloc
   - enca identified the encoding
   - gsf_output_memory_new
   - gsf_output_write
   - gsf_output_size
   - bytes came back
   - gsm_create
   - gsm encode/decode round trip
   - decoded frame carries signal energy
   - hnj_hyphen_hyphenate2
   - missing dictionary reports NULL

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | enca 1.19-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/enca-1.19-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gsm 1.0.22-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gsm-1.0.22-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hyphen 2.8.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/hyphen-2.8.8-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | enca 1.19-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/enca-1.19-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gsm 1.0.22-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gsm-1.0.22-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hyphen 2.8.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/hyphen-2.8.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.22**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gsm=<version>`), sources in `ported/gsm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gsm`; port one with `scripts/port-update gsm`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.22
TARBALL=https://www.quut.com/gsm/gsm-1.0.22.tar.gz
SHA256=f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac
BUILDSYS=custom
# The upstream Makefile hardcodes the host compiler and installs nowhere useful;
# the library is eight objects, so build it directly.
do_build() {
  mkdir -p "$B/o"
  for f in "$S"/src/*.c; do
    "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
      -DSASR -DWAV49 -DNeedFunctionPrototypes=1 -I"$S/inc" \
      -c "$f" -o "$B/o/$(basename "${f%.c}").o"
  done
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libgsm.so.1 \
    -o "$B/libgsm.so.1.0.22" "$B"/o/*.o
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/gsm"
  install -m 755 "$B/libgsm.so.1.0.22" "$STAGE$PREFIX/lib/"
  ln -sfn libgsm.so.1.0.22 "$STAGE$PREFIX/lib/libgsm.so.1"
  ln -sfn libgsm.so.1      "$STAGE$PREFIX/lib/libgsm.so"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/gsm/"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
