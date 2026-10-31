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

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add harfbuzz
apk add harfbuzz-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-harfbuzz && /usr/local/bin/qnx-port-tests/test_harfbuzz    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install harfbuzz`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | harfbuzz 10.1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/harfbuzz-10.1.0-r0.log) |

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
