# freetype 2.13.3

> Portable font rendering engine library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/freetype/freetype.git (tag `VER-2-13-3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`libpng`](../libpng/) [`brotli`](../brotli/) |
| Run-time dependencies | [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`libpng`](../libpng/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_freetype`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `freetype` (programs, shared libraries, run-time data) and `freetype-dev` (headers, pkg-config/CMake files, static libraries; needs `freetype`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`freetype`](https://openqnx.github.io/oss/package/x86_64/freetype.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/freetype.html)) · [`freetype-dev`](https://openqnx.github.io/oss/package/x86_64/freetype-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/freetype-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add freetype` (dependencies come along); one version: `apk add freetype=<version>` | `apk del freetype` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install freetype` (dependencies first) | `oss-get remove freetype` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`freetype-dev` the same way, by its own name (`apk add freetype-dev`, `oss-get install freetype-dev`, by hand with its own file and list `freetype-dev.list`); remove it before `freetype`, which it needs.

```sh
# on the QNX target, as root
apk add freetype freetype-dev
apk add qnx-test-freetype && /usr/local/bin/qnx-port-tests/test_freetype    # its on-target test
apk del freetype freetype-dev qnx-test-freetype
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/freetype.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/freetype.list
rm var/lib/manual/freetype.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild freetype` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 libpng brotli
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b VER-2-13-3 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/freetype/freetype.git src/freetype
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/freetype -B build/$QNX_ARCH/freetype -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DFT_DISABLE_HARFBUZZ=ON \
      -DFT_REQUIRE_ZLIB=ON \
      -DFT_REQUIRE_PNG=ON \
      -DFT_REQUIRE_BZIP2=ON \
      -DZLIB_ROOT="$STAGE$PREFIX" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/freetype -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/freetype
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH freetype`.

### 5. What it installs

62 files under `/usr/local`: 3 shared-library entries, 55 headers.

```
/usr/local/lib/libfreetype.so
/usr/local/lib/libfreetype.so.6
/usr/local/lib/libfreetype.so.6.20.2
```

The complete list is `stage/<arch>/.manifest/freetype` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa freetype
ls dists/8.0/*/ | grep '^freetype-'
```

This writes `freetype-<ver>-r0.tar.gz` (run time) and `freetype-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add freetype
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/freetype-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/freetype-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_freetype`](../../tests/src/) links freetype the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-freetype`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-freetype
/usr/local/bin/qnx-port-tests/test_freetype
```

```
== freetype ==
  ok   ...
PASS freetype
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_freetype` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_freetype`](../../tests/src/) -- a program linked against freetype the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - FT_Init_FreeType
   - version >= 2
   - missing font file is reported as an error
   - face has a real glyph set
   - FT_Set_Pixel_Sizes
   - glyph index for 'A'
   - FT_Load_Glyph
   - FT_Render_Glyph
   - rasterised bitmap is non-empty
   - glyph has inked pixels

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | freetype 2.13.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/freetype-2.13.3-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | freetype 2.13.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/freetype-2.13.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add freetype=<version>`), sources in `ported/freetype/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates freetype`; port one with `scripts/port-update freetype`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.13.3; GIT=https://gitlab.freedesktop.org/freetype/freetype.git; REF=VER-2-13-3
DEPS="zlib bzip2 libpng brotli"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DFT_DISABLE_HARFBUZZ=ON
                   -DFT_REQUIRE_ZLIB=ON -DFT_REQUIRE_PNG=ON -DFT_REQUIRE_BZIP2=ON
                   -DZLIB_ROOT="$STAGE$PREFIX" -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
