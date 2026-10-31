# orc 0.4.40

> Oil Runtime Compiler library for optimized inner loops (SIMD)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/gstreamer/orc.git (tag `0.4.40`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_orc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `orc` (programs, shared libraries, run-time data) and `orc-dev` (headers, pkg-config/CMake files, static libraries; needs `orc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`orc`](https://openqnx.github.io/oss/package/x86_64/orc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/orc.html)) · [`orc-dev`](https://openqnx.github.io/oss/package/x86_64/orc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/orc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add orc` (dependencies come along); one version: `apk add orc=<version>` | `apk del orc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install orc` (dependencies first) | `oss-get remove orc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`orc-dev` the same way, by its own name (`apk add orc-dev`, `oss-get install orc-dev`, by hand with its own file and list `orc-dev.list`); remove it before `orc`, which it needs.

```sh
# on the QNX target, as root
apk add orc orc-dev
apk add qnx-test-orc && /usr/local/bin/qnx-port-tests/test_orc    # its on-target test
apk del orc orc-dev qnx-test-orc
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/orc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/orc.list
rm var/lib/manual/orc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild orc` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.4.40 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/gstreamer/orc.git src/orc
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/orc < ports/orc/patches/0001-qnx-aarch64-neon-is-mandatory.patch
```

**[`0001-qnx-aarch64-neon-is-mandatory.patch`](patches/0001-qnx-aarch64-neon-is-mandatory.patch)** — orc: NEON is always present on aarch64 QNX.

> QNX has no /proc/cpuinfo or auxv to probe, so orc found no NEON, marked its
> NEON backend unusable and interpreted every program. Advanced SIMD is
> mandatory in ARMv8-A, the reasoning orc already uses for Windows and macOS.

Touches: `orc/orccpu-arm.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/orc src/orc \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dorc-test=disabled \
      -Dbenchmarks=disabled \
      -Dexamples=disabled \
      -Dtests=disabled \
      -Dtools=disabled \
      -Dgtk_doc=disabled
ninja -C build/$QNX_ARCH/orc
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/orc install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH orc`.

### 5. What it installs

37 files under `/usr/local`: 3 shared-library entries, 32 headers.

```
/usr/local/lib/liborc-0.4.so
/usr/local/lib/liborc-0.4.so.0
/usr/local/lib/liborc-0.4.so.0.40.0
```

The complete list is `stage/<arch>/.manifest/orc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa orc
ls dists/8.0/*/ | grep '^orc-'
```

This writes `orc-<ver>-r0.tar.gz` (run time) and `orc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add orc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/orc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/orc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_orc`](../../tests/src/) links orc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-orc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-orc
/usr/local/bin/qnx-port-tests/test_orc
```

```
== orc ==
  ok   ...
PASS orc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_orc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_orc`](../../tests/src/) -- a program linked against orc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - orc_program_new
   - orc_program_compile
   - the generated program computed s1 + s2 for every element

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | orc 0.4.40-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/orc-0.4.40-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | orc 0.4.40-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/orc-0.4.40-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.4.40**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add orc=<version>`), sources in `ported/orc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates orc`; port one with `scripts/port-update orc`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.4.40; GIT=https://gitlab.freedesktop.org/gstreamer/orc.git; REF=0.4.40
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dorc-test=disabled -Dbenchmarks=disabled -Dexamples=disabled
                   -Dtests=disabled -Dtools=disabled -Dgtk_doc=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
