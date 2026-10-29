# gst-plugins-ugly 1.24.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`x264`](../x264/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`x264`](../x264/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gst-plugins-ugly
oss-get install gst-plugins-ugly-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-ugly` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gst-plugins-base x264
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`x264`](../x264/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-ugly/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-ugly src/gst-plugins-ugly/subprojects/gst-plugins-ugly \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      -Dgpl=enabled \
      # \
      x264 \
      is \
      GPL; \
      the \
      plugin \
      set \
      refuses \
      to \
      build \
      it \
      otherwise \
      -Dx264=enabled \
      # \
      not \
      ported: \
      the \
      DVD/legacy-codec \
      corner \
      of \
      this \
      plugin \
      set \
      -Da52dec=disabled \
      -Dcdio=disabled \
      -Ddvdread=disabled \
      -Dmpeg2dec=disabled \
      -Dsidplay=disabled
ninja -C build/$QNX_ARCH/gst-plugins-ugly
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-ugly install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-ugly`.

### 5. What it installs

61 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/gst-plugins-ugly` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gst-plugins-ugly
ls repo/8.0/*/ | grep '^gst-plugins-ugly-'
```

This writes `gst-plugins-ugly-<ver>-r0.tar.gz` (run time) and `gst-plugins-ugly-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gst-plugins-ugly
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gst-plugins-ugly-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gst-plugins-ugly-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-ugly
DEPS="gst-plugins-base x264"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Ddoc=disabled -Dnls=enabled
  -Dgpl=enabled                 # x264 is GPL; the plugin set refuses to build it otherwise
  -Dx264=enabled
  # not ported: the DVD/legacy-codec corner of this plugin set
  -Da52dec=disabled -Dcdio=disabled
  -Ddvdread=disabled -Dmpeg2dec=disabled -Dsidplay=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
