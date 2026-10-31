# enca 1.19

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/nijel/enca.git (tag `1.19`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_misc_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add enca
apk add enca-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-misc-libs && /usr/local/bin/qnx-port-tests/test_misc_libs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install enca`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild enca` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.19 --recurse-submodules --shallow-submodules \
    https://github.com/nijel/enca.git src/enca
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/enca; B=build/$QNX_ARCH/enca; mkdir -p $B
```

```bash
do_patch() {
  cd "$SRCROOT" && autoreconf -fi
}
```

```bash
do_build() {
  cc -O1 -o "$B/make_hash_native" "$S/tools/make_hash.c" -lm
  make -C "$B" -j"$(nproc)" || true
  # Replace the cross-built generator with the native one and resume.
  install -m 755 "$B/make_hash_native" "$B/tools/make_hash"
  make -C "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tools/make_hash is compiled and then *run* to generate encodings.h. Built for the target it is a QNX binary, and the build dies in the binfmt handler ("Could not open /usr/lib/ldqnx-64.so.2"). Build a native copy first and make the target one a wrapper around it -- the generated header is architecture-independent.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH enca`.

### 5. What it installs

21 files under `/usr/local`: 2 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libenca.so
/usr/local/lib/libenca.so.5
/usr/local/bin/enca
/usr/local/bin/enconv
```

The complete list is `stage/<arch>/.manifest/enca` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa enca
ls dists/8.0/*/ | grep '^enca-'
```

This writes `enca-<ver>-r0.tar.gz` (run time) and `enca-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add enca
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/enca-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/enca-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links enca the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-misc-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-misc-libs
/usr/local/bin/qnx-port-tests/test_misc_libs
```

```
== enca ==
  ok   ...
PASS enca
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
   Built again against an *empty* staging tree (`scripts/verify-standalone`): aarch64le passed, x86_64 passed -- it cannot have picked anything up from other components.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_misc_libs`](../../tests/src/) -- a program linked against enca the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | enca 1.19-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/enca-1.19-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gsm 1.0.22-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gsm-1.0.22-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hyphen 2.8.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/hyphen-2.8.8-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | enca 1.19-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/enca-1.19-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gsm 1.0.22-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gsm-1.0.22-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hyphen 2.8.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/hyphen-2.8.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.19**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add enca=<version>`), sources in `ported/enca/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates enca`; port one with `scripts/port-update enca`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.19; GIT=https://github.com/nijel/enca.git; REF=1.19
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-external )
# tools/make_hash is compiled and then *run* to generate encodings.h. Built for
# the target it is a QNX binary, and the build dies in the binfmt handler
# ("Could not open /usr/lib/ldqnx-64.so.2"). Build a native copy first and make
# the target one a wrapper around it -- the generated header is
# architecture-independent.
do_patch() {
  cd "$SRCROOT" && autoreconf -fi
}
do_build() {
  cc -O1 -o "$B/make_hash_native" "$S/tools/make_hash.c" -lm
  make -C "$B" -j"$(nproc)" || true
  # Replace the cross-built generator with the native one and resume.
  install -m 755 "$B/make_hash_native" "$B/tools/make_hash"
  make -C "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
