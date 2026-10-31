# harfbuzz 10.1.0

> OpenType text shaping engine

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/harfbuzz/harfbuzz.git (tag `10.1.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`glib`](../glib/) [`icu`](../icu/) [`graphite2`](../graphite2/) |
| Run-time dependencies | [`freetype`](../freetype/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`icu`](../icu/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_harfbuzz`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `harfbuzz` (programs, shared libraries, run-time data) and `harfbuzz-dev` (headers, pkg-config/CMake files, static libraries; needs `harfbuzz`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`harfbuzz`](https://openqnx.github.io/oss/package/x86_64/harfbuzz.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/harfbuzz.html)) · [`harfbuzz-dev`](https://openqnx.github.io/oss/package/x86_64/harfbuzz-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/harfbuzz-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add harfbuzz` (dependencies come along); one version: `apk add harfbuzz=<version>` | `apk del harfbuzz` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install harfbuzz` (dependencies first) | `oss-get remove harfbuzz` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`harfbuzz-dev` the same way, by its own name (`apk add harfbuzz-dev`, `oss-get install harfbuzz-dev`, by hand with its own file and list `harfbuzz-dev.list`); remove it before `harfbuzz`, which it needs.

```sh
# on the QNX target, as root
apk add harfbuzz harfbuzz-dev
apk add qnx-test-harfbuzz && /usr/local/bin/qnx-port-tests/test_harfbuzz    # its on-target test
apk del harfbuzz harfbuzz-dev qnx-test-harfbuzz
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/harfbuzz.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/harfbuzz.list
rm var/lib/manual/harfbuzz.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild harfbuzz` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (14 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype glib icu graphite2
```

<details><summary>Full build chain (14)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 10.1.0 --recurse-submodules --shallow-submodules \
    https://github.com/harfbuzz/harfbuzz.git src/harfbuzz
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/harfbuzz src/harfbuzz \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Ddocs=disabled \
      -Dbenchmark=disabled \
      -Dutilities=disabled \
      -Dintrospection=disabled \
      -Dfreetype=enabled \
      -Dglib=enabled \
      -Dicu=enabled \
      -Dgraphite2=enabled \
      -Dcairo=disabled \
      -Dchafa=disabled
ninja -C build/$QNX_ARCH/harfbuzz
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/harfbuzz install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH harfbuzz`.

### 5. What it installs

61 files under `/usr/local`: 12 shared-library entries, 40 headers.

```
/usr/local/lib/libharfbuzz-gobject.so
/usr/local/lib/libharfbuzz-gobject.so.0
/usr/local/lib/libharfbuzz-gobject.so.0.61010.0
/usr/local/lib/libharfbuzz-icu.so
/usr/local/lib/libharfbuzz-icu.so.0
/usr/local/lib/libharfbuzz-icu.so.0.61010.0
/usr/local/lib/libharfbuzz-subset.so
/usr/local/lib/libharfbuzz-subset.so.0
/usr/local/lib/libharfbuzz-subset.so.0.61010.0
/usr/local/lib/libharfbuzz.so
/usr/local/lib/libharfbuzz.so.0
/usr/local/lib/libharfbuzz.so.0.61010.0
```

The complete list is `stage/<arch>/.manifest/harfbuzz` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa harfbuzz
ls dists/8.0/*/ | grep '^harfbuzz-'
```

This writes `harfbuzz-<ver>-r0.tar.gz` (run time) and `harfbuzz-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add harfbuzz
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/harfbuzz-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/harfbuzz-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_harfbuzz`](../../tests/src/) links harfbuzz the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-harfbuzz`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-harfbuzz
/usr/local/bin/qnx-port-tests/test_harfbuzz
```

```
== harfbuzz ==
  ok   ...
PASS harfbuzz
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_harfbuzz` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_harfbuzz`](../../tests/src/) -- a program linked against harfbuzz the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - general categories
   - mirroring and scripts
   - hb_set range/delete
   - language tags normalised
   - DejaVuSans.ttf loaded
   - glyph count 6253, 2048 units/em
   - cmap: A -> 36, V -> 57
   - unmapped private-use code point
   - GSUB and GPOS present
   - \"office\": ffi ligature (glyph 5044) spanning clusters 1-3
   - LTR Latin guessed
   - \"office\" with -liga keeps f f i
   - GPOS kerning tightens A-V
   - Hebrew shaped RTL, clusters in visual order
   - hb-subset produced a face
   - subset: .notdef + A + V, B dropped

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | harfbuzz 10.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/harfbuzz-10.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | harfbuzz 10.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/harfbuzz-10.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | harfbuzz 10.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/harfbuzz-10.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | harfbuzz 10.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/harfbuzz-10.1.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **10.1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add harfbuzz=<version>`), sources in `ported/harfbuzz/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates harfbuzz`; port one with `scripts/port-update harfbuzz`.

<details><summary>recipe.sh</summary>

```bash
VERSION=10.1.0; GIT=https://github.com/harfbuzz/harfbuzz.git; REF=10.1.0
DEPS="freetype glib icu graphite2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Ddocs=disabled -Dbenchmark=disabled
                   -Dutilities=disabled -Dintrospection=disabled
                   -Dfreetype=enabled -Dglib=enabled -Dicu=enabled
                   -Dgraphite2=enabled -Dcairo=disabled -Dchafa=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
