# harfbuzz 10.1.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/harfbuzz/harfbuzz.git (tag `10.1.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`glib`](../glib/) [`icu`](../icu/) [`graphite2`](../graphite2/) |
| Run-time dependencies | [`freetype`](../freetype/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`icu`](../icu/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install harfbuzz
oss-get install harfbuzz-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH harfbuzz
ls repo/8.0/*/ | grep '^harfbuzz-'
```

This writes `harfbuzz-<ver>-r0.tar.gz` (run time) and `harfbuzz-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install harfbuzz
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/harfbuzz-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/harfbuzz-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
