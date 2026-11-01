# libraqm 0.10.5

> Library for complex text layout (bidirectional text, shaping)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/HOST-Oman/libraqm.git (tag `v0.10.5`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`fribidi`](../fribidi/) |
| Run-time dependencies | [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`harfbuzz`](../harfbuzz/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libraqm`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libraqm` (programs, shared libraries, run-time data) and `libraqm-dev` (headers, pkg-config/CMake files, static libraries; needs `libraqm`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libraqm`](https://openqnx.github.io/oss/package/x86_64/libraqm.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libraqm.html)) · [`libraqm-dev`](https://openqnx.github.io/oss/package/x86_64/libraqm-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libraqm-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libraqm` (dependencies come along); one version: `apk add libraqm=<version>` | `apk del libraqm` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libraqm` (dependencies first) | `oss-get remove libraqm` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libraqm-dev` the same way, by its own name (`apk add libraqm-dev`, `oss-get install libraqm-dev`, by hand with its own file and list `libraqm-dev.list`); remove it before `libraqm`, which it needs.

```sh
# on the QNX target, as root
apk add libraqm libraqm-dev
apk add qnx-test-libraqm && /usr/local/bin/qnx-port-tests/test_libraqm    # its on-target test, from tests-8.0-<arch>
apk del libraqm libraqm-dev qnx-test-libraqm
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libraqm.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libraqm.list
rm var/lib/manual/libraqm.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libraqm` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (16 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype harfbuzz fribidi
```

<details><summary>Full build chain (16)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.10.5 --recurse-submodules --shallow-submodules \
    https://github.com/HOST-Oman/libraqm.git src/libraqm
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libraqm src/libraqm \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=false \
      -Dtests=false \
      -Dsheenbidi=false
ninja -C build/$QNX_ARCH/libraqm
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libraqm install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libraqm: complex text layout (bidirectional text, shaping, script itemization) on FreeType, HarfBuzz and FriBidi -- what matplotlib 3.11 lays text out with.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libraqm`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libraqm.so
/usr/local/lib/libraqm.so.0
/usr/local/lib/libraqm.so.0.10.5
```

The complete list is `stage/<arch>/.manifest/libraqm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libraqm
ls dists/8.0/*/ | grep '^libraqm-'
```

This writes `libraqm-<ver>-r0.tar.gz` (run time) and `libraqm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libraqm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libraqm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libraqm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libraqm`](../../tests/src/) links libraqm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libraqm`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libraqm
/usr/local/bin/qnx-port-tests/test_libraqm
```

```
== libraqm ==
  ok   ...
PASS libraqm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libraqm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libraqm`](../../tests/src/) -- a program linked against libraqm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - raqm_version_atleast(0.10)
   - FreeType initialised
   - DejaVu Sans opened
   - \"Hello\": 5 glyphs
   - logical order, clusters 0..4, positive advances
   - both 'l' map to the same glyph
   - resolved direction LTR
   - raqm_index_to_position: positions increase along the text
   - raqm_position_to_index: x=0 -> 0, past the end -> 5
   - Hebrew word: 4 glyphs
   - right-to-left: visual order (clusters 6,4,2,0)
   - mixed text: LTR paragraph, 8 glyphs
   - the Hebrew run inside is reversed
   - OpenType liga: \"fi\" becomes one glyph

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libraqm 0.10.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libraqm-0.10.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libraqm 0.10.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libraqm-0.10.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.10.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libraqm=<version>`), sources in `ported/libraqm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libraqm`; port one with `scripts/port-update libraqm`.

<details><summary>recipe.sh</summary>

```bash
# libraqm: complex text layout (bidirectional text, shaping, script
# itemization) on FreeType, HarfBuzz and FriBidi -- what matplotlib 3.11
# lays text out with.
VERSION=0.10.5; GIT=https://github.com/HOST-Oman/libraqm.git; REF=v0.10.5
DEPS="freetype harfbuzz fribidi"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=false -Dtests=false -Dsheenbidi=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
