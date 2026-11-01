# glib 2.82.4

> Low-level core C library of GTK and GNOME: data types, main loop, GIO

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/glib.git (tag `2.82.4`) |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`libffi`](../libffi/) [`pcre2`](../pcre2/) [`gettext`](../gettext/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `glib` (programs, shared libraries, run-time data) and `glib-dev` (headers, pkg-config/CMake files, static libraries; needs `glib`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`glib`](https://openqnx.github.io/oss/package/x86_64/glib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib.html)) · [`glib-dev`](https://openqnx.github.io/oss/package/x86_64/glib-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add glib` (dependencies come along); one version: `apk add glib=<version>` | `apk del glib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install glib` (dependencies first) | `oss-get remove glib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`glib-dev` the same way, by its own name (`apk add glib-dev`, `oss-get install glib-dev`, by hand with its own file and list `glib-dev.list`); remove it before `glib`, which it needs.

```sh
# on the QNX target, as root
apk add glib glib-dev
apk add qnx-test-glib && /usr/local/bin/qnx-port-tests/test_glib    # its on-target test, from tests-8.0-<arch>
apk del glib glib-dev qnx-test-glib
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/glib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/glib.list
rm var/lib/manual/glib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/glib < ports/glib/patches/0001-qnx-fstab-mount-points.patch
patch -p1 -d src/glib < ports/glib/patches/0002-qnx-credentials-cmsgcred.patch
```

**[`0001-qnx-fstab-mount-points.patch`](patches/0001-qnx-fstab-mount-points.patch)** — GIO: use the fstab code for mount points on QNX.

> QNX provides setfsent/getfsent/endfsent and struct fstab in <fstab.h>, but the
> fstab branch of gunixmounts.c is chosen only with getfsstat/getvfsstat, which
> QNX lacks (its mount table already has its own HAVE_QNX branch). Without this,
> the file ends at "#error No g_get_mount_table() implementation".

Touches: `gio/gunixmounts.c` · Upstream status: QNX-specific

**[`0002-qnx-credentials-cmsgcred.patch`](patches/0002-qnx-credentials-cmsgcred.patch)** — QNX: GCredentials via SCM_CREDS/struct cmsgcred, as on FreeBSD (GDBus EXTERNAL)

Touches: `gio/gcredentialsprivate.h` · Upstream status: QNX-specific

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

[`test_glib`](../../tests/src/) links glib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-glib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

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

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glib 2.82.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/glib-2.82.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glib 2.82.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/glib-2.82.4-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glib 2.82.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/glib-2.82.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glib 2.82.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/glib-2.82.4-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

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
