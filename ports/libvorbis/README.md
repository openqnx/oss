# libvorbis 1.3.7

> Ogg Vorbis audio codec libraries

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/xiph/vorbis.git (tag `v1.3.7`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libogg`](../libogg/) |
| Run-time dependencies | [`libogg`](../libogg/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvorbis`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libvorbis` (programs, shared libraries, run-time data) and `libvorbis-dev` (headers, pkg-config/CMake files, static libraries; needs `libvorbis`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libvorbis`](https://openqnx.github.io/oss/package/x86_64/libvorbis.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvorbis.html)) · [`libvorbis-dev`](https://openqnx.github.io/oss/package/x86_64/libvorbis-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libvorbis-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libvorbis` (dependencies come along); one version: `apk add libvorbis=<version>` | `apk del libvorbis` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libvorbis` (dependencies first) | `oss-get remove libvorbis` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libvorbis-dev` the same way, by its own name (`apk add libvorbis-dev`, `oss-get install libvorbis-dev`, by hand with its own file and list `libvorbis-dev.list`); remove it before `libvorbis`, which it needs.

```sh
# on the QNX target, as root
apk add libvorbis libvorbis-dev
apk add qnx-test-libvorbis && /usr/local/bin/qnx-port-tests/test_libvorbis    # its on-target test, from tests-8.0-<arch>
apk del libvorbis libvorbis-dev qnx-test-libvorbis
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libvorbis.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libvorbis.list
rm var/lib/manual/libvorbis.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvorbis` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libogg
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.3.7 --recurse-submodules --shallow-submodules \
    https://github.com/xiph/vorbis.git src/libvorbis
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libvorbis -B build/$QNX_ARCH/libvorbis -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DOGG_ROOT="$STAGE$PREFIX" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libvorbis -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libvorbis
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvorbis`.

### 5. What it installs

16 files under `/usr/local`: 6 shared-library entries, 3 headers.

```
/usr/local/lib/libvorbis.so
/usr/local/lib/libvorbis.so.0.4.9
/usr/local/lib/libvorbisenc.so
/usr/local/lib/libvorbisenc.so.2.0.12
/usr/local/lib/libvorbisfile.so
/usr/local/lib/libvorbisfile.so.3.3.8
```

The complete list is `stage/<arch>/.manifest/libvorbis` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libvorbis
ls dists/8.0/*/ | grep '^libvorbis-'
```

This writes `libvorbis-<ver>-r0.tar.gz` (run time) and `libvorbis-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libvorbis
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libvorbis-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libvorbis-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libvorbis`](../../tests/src/) links libvorbis the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libvorbis`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libvorbis
/usr/local/bin/qnx-port-tests/test_libvorbis
```

```
== libvorbis ==
  ok   ...
PASS libvorbis
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvorbis` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libvorbis`](../../tests/src/) -- a program linked against libvorbis the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - open output file in /tmp
   - vorbis_encode_init_vbr 2ch 44.1k q0.4
   - Ogg Vorbis file written (plausible size)
   - ov_fopen
   - stream info: 2 channels, 44100 Hz
   - TITLE comment survives
   - ov_pcm_total equals the encoded frame count
   - decoded frame count matches
   - decoded level close to the input
   - decoded tone is 1 kHz
   - right channel is the inverted left one
   - ov_pcm_seek to 1 s

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libvorbis 1.3.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libvorbis-1.3.7-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libvorbis 1.3.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libvorbis-1.3.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libvorbis 1.3.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libvorbis-1.3.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libvorbis 1.3.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libvorbis-1.3.7-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libvorbis=<version>`), sources in `ported/libvorbis/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libvorbis`; port one with `scripts/port-update libvorbis`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.7; GIT=https://github.com/xiph/vorbis.git; REF=v1.3.7
DEPS="libogg"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DOGG_ROOT="$STAGE$PREFIX" -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
