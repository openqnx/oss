# glib 2.82.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/glib.git (tag `2.82.4`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`libffi`](../libffi/) [`pcre2`](../pcre2/) [`gettext`](../gettext/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glib`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add glib
apk add glib-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-glib && /usr/local/bin/qnx-port-tests/test_glib    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install glib`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa glib
ls dists/8.0/*/ | grep '^glib-'
```

This writes `glib-<ver>-r0.tar.gz` (run time) and `glib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add glib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/glib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/glib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_glib`](../../tests/src/) links glib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-glib`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-glib
/usr/local/bin/qnx-port-tests/test_glib
```

```
== glib ==
  ok   ...
PASS glib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_glib`](../../tests/src/) -- a program linked against glib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - GLib 2.x
   - default main context
   - idle source ran
   - timeout source ran and quit the loop
   - g_thread_new
   - thread returned its value
   - thread ran
   - g_object_new
   - G_IS_OBJECT
   - g_type_from_name
   - g_utf8_strup
   - GString formatting
   - GHashTable insert
   - GHashTable lookup
   - g_file_new_for_path
   - /tmp is reported as a directory
   - g_file_test

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.82.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add glib=<version>`), sources in `ported/glib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates glib`; port one with `scripts/port-update glib`.

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
