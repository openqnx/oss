# gdk-pixbuf 2.42.12

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gdk-pixbuf.git (tag `2.42.12`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gdk_pixbuf`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gdk-pixbuf
oss-get install gdk-pixbuf-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gdk-pixbuf` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libpng libjpeg-turbo
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.42.12 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gdk-pixbuf.git src/gdk-pixbuf
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gdk-pixbuf src/gdk-pixbuf \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dinstalled_tests=false \
      -Dman=false \
      -Dgtk_doc=false \
      -Dintrospection=disabled \
      -Dpng=enabled \
      -Djpeg=enabled \
      -Dtiff=disabled \
      -Dothers=enabled \
      -Dbuiltin_loaders=all \
      -Drelocatable=false \
      # \
      gio_sniffing \
      pulls \
      in \
      shared-mime-info, \
      a \
      desktop \
      data \
      # \
      package; \
      gdk-pixbuf's \
      own \
      image \
      sniffing \
      is \
      what \
      an \
      # \
      embedded \
      target \
      wants \
      anyway. \
      -Dgio_sniffing=false
ninja -C build/$QNX_ARCH/gdk-pixbuf
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gdk-pixbuf install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gdk-pixbuf`.

### 5. What it installs

129 files under `/usr/local`: 3 shared-library entries, 3 programs, 13 headers.

```
/usr/local/lib/libgdk_pixbuf-2.0.so
/usr/local/lib/libgdk_pixbuf-2.0.so.0
/usr/local/lib/libgdk_pixbuf-2.0.so.0.4200.12
/usr/local/bin/gdk-pixbuf-csource
/usr/local/bin/gdk-pixbuf-pixdata
/usr/local/bin/gdk-pixbuf-query-loaders
```

The complete list is `stage/<arch>/.manifest/gdk-pixbuf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gdk-pixbuf
ls repo/8.0/*/ | grep '^gdk-pixbuf-'
```

This writes `gdk-pixbuf-<ver>-r0.tar.gz` (run time) and `gdk-pixbuf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gdk-pixbuf
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gdk-pixbuf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gdk-pixbuf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gdk_pixbuf`](../../tests/src/) links gdk-pixbuf the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gdk_pixbuf root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gdk_pixbuf'
```

```
== gdk-pixbuf ==
  ok   ...
PASS gdk-pixbuf
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.42.12; GIT=https://gitlab.gnome.org/GNOME/gdk-pixbuf.git; REF=2.42.12
DEPS="glib libpng libjpeg-turbo"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dinstalled_tests=false -Dman=false
                   -Dgtk_doc=false -Dintrospection=disabled
                   -Dpng=enabled -Djpeg=enabled -Dtiff=disabled
                   -Dothers=enabled -Dbuiltin_loaders=all
                   -Drelocatable=false
                   # gio_sniffing pulls in shared-mime-info, a desktop data
                   # package; gdk-pixbuf's own image sniffing is what an
                   # embedded target wants anyway.
                   -Dgio_sniffing=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
