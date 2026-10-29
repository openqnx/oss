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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gst-plugins-ugly
apk add gst-plugins-ugly-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gst-plugins-ugly`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gst-plugins-ugly
ls dists/8.0/*/ | grep '^gst-plugins-ugly-'
```

This writes `gst-plugins-ugly-<ver>-r0.tar.gz` (run time) and `gst-plugins-ugly-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gst-plugins-ugly
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gst-plugins-ugly-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gst-plugins-ugly-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gst-plugins-ugly=<version>`), sources in `ported/gst-plugins-ugly/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gst-plugins-ugly`; port one with `scripts/port-update gst-plugins-ugly`.

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
