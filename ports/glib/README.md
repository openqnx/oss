# glib 2.82.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/glib.git (tag `2.82.4`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`libffi`](../libffi/) [`pcre2`](../pcre2/) [`gettext`](../gettext/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libffi`](../libffi/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glib`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install glib
oss-get install glib-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild glib` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libffi pcre2 gettext zlib
```

<details><summary>Full build chain (8)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.82.4 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/glib.git src/glib
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/glib < ports/glib/patches/0001-qnx-fstab-mount-points.patch
```

**[`0001-qnx-fstab-mount-points.patch`](patches/0001-qnx-fstab-mount-points.patch)** — GIO: use the fstab code for mount points on QNX.

> QNX provides setfsent/getfsent/endfsent and struct fstab in <fstab.h>, but the
> fstab branch of gunixmounts.c is chosen only with getfsstat/getvfsstat, which
> QNX lacks (its mount table already has its own HAVE_QNX branch). Without this,
> the file ends at "#error No g_get_mount_table() implementation".

Touches: `gio/gunixmounts.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/glib src/glib \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dglib_debug=disabled \
      -Dman-pages=disabled \
      -Ddocumentation=false \
      -Dintrospection=disabled \
      # \
      needs \
      a \
      working \
      gobject-introspection \
      scanner \
      on \
      target \
      -Dlibmount=disabled \
      # \
      Linux \
      util-linux \
      only \
      -Dselinux=disabled \
      # \
      Linux \
      only \
      -Dxattr=false \
      # \
      QNX \
      has \
      no \
      <sys/xattr.h> \
      -Dnls=enabled \
      -Dtests=false \
      -Dsysprof=disabled \
      -Ddtrace=false \
      -Dsystemtap=false
ninja -C build/$QNX_ARCH/glib
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/glib install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH glib`.

### 5. What it installs

487 files under `/usr/local`: 18 shared-library entries, 18 programs, 305 headers.

```
/usr/local/lib/libgio-2.0.so
/usr/local/lib/libgio-2.0.so.0
/usr/local/lib/libgio-2.0.so.0.8200.4
/usr/local/lib/libgirepository-2.0.so
/usr/local/lib/libgirepository-2.0.so.0
/usr/local/lib/libgirepository-2.0.so.0.8200.4
/usr/local/lib/libglib-2.0.so
/usr/local/lib/libglib-2.0.so.0
/usr/local/lib/libglib-2.0.so.0.8200.4
/usr/local/lib/libgmodule-2.0.so
/usr/local/lib/libgmodule-2.0.so.0
/usr/local/lib/libgmodule-2.0.so.0.8200.4
/usr/local/lib/libgobject-2.0.so
/usr/local/lib/libgobject-2.0.so.0
/usr/local/lib/libgobject-2.0.so.0.8200.4
/usr/local/lib/libgthread-2.0.so
...
```

The complete list is `stage/<arch>/.manifest/glib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH glib
ls repo/8.0/*/ | grep '^glib-'
```

This writes `glib-<ver>-r0.tar.gz` (run time) and `glib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install glib
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/glib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/glib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_glib`](../../tests/src/) links glib the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glib root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_glib'
```

```
== glib ==
  ok   ...
PASS glib
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.82.4; GIT=https://gitlab.gnome.org/GNOME/glib.git; REF=2.82.4
DEPS="libffi pcre2 gettext zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=false
  -Dglib_debug=disabled
  -Dman-pages=disabled
  -Ddocumentation=false
  -Dintrospection=disabled          # needs a working gobject-introspection scanner on target
  -Dlibmount=disabled               # Linux util-linux only
  -Dselinux=disabled                # Linux only
  -Dxattr=false                     # QNX has no <sys/xattr.h>
  -Dnls=enabled
  -Dtests=false
  -Dsysprof=disabled
  -Ddtrace=false
  -Dsystemtap=false
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
