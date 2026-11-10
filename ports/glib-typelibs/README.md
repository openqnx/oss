# glib-typelibs 2.82.4

> GObject introspection data (typelibs and GIR files) of the ported GLib, for PyGObject

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/glib.git (tag `2.82.4`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_gobject3`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `glib-typelibs` (programs, shared libraries, run-time data) and `glib-typelibs-dev` (headers, pkg-config/CMake files, static libraries; needs `glib-typelibs`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`glib-typelibs`](https://openqnx.github.io/oss/package/x86_64/glib-typelibs.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib-typelibs.html)) · [`glib-typelibs-dev`](https://openqnx.github.io/oss/package/x86_64/glib-typelibs-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib-typelibs-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add glib-typelibs` (dependencies come along); one version: `apk add glib-typelibs=<version>` | `apk del glib-typelibs` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install glib-typelibs` (dependencies first) | `oss-get remove glib-typelibs` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`glib-typelibs-dev` the same way, by its own name (`apk add glib-typelibs-dev`, `oss-get install glib-typelibs-dev`, by hand with its own file and list `glib-typelibs-dev.list`); remove it before `glib-typelibs`, which it needs.

```sh
# on the QNX target, as root
apk add glib-typelibs glib-typelibs-dev
apk add qnx-test-python3-gobject3 && /usr/local/bin/qnx-port-tests/test_python3_gobject3    # its on-target test, from tests-8.0-<arch>-m-z
apk del glib-typelibs glib-typelibs-dev qnx-test-python3-gobject3
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/glib-typelibs.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/glib-typelibs.list
rm var/lib/manual/glib-typelibs.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild glib-typelibs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.82.4 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/glib.git src/glib-typelibs
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/glib-typelibs; B=build/$QNX_ARCH/glib-typelibs; mkdir -p $B
```

```bash
do_build() {
  _native || return 1
  rm -rf "$B/gir" "$B/typelib" && mkdir -p "$B/gir" "$B/typelib" || return 1
  find "$H/glib2" -name '*.gir' -exec cp {} "$B/gir/" \; || return 1
  ls "$B/gir"
  /usr/bin/python3 "$WS/ports/glib-typelibs/fix-constants.py" "$STAGE$PREFIX/lib/glib-2.0/include/glibconfig.h" "$SRCROOT/glib" "$B/gir"/*.gir || return 1
  local g
  for g in "$B/gir"/*.gir; do
    _host gi-compile-repository --includedir="$B/gir" -o "$B/typelib/$(basename "${g%.gir}").typelib" "$g" || return 1
  done
}
```

```bash
do_install() {
  mkdir -p "$STAGE$PREFIX/lib/girepository-1.0" "$STAGE$PREFIX/share/gir-1.0"
  cp "$B"/typelib/*.typelib "$STAGE$PREFIX/lib/girepository-1.0/"
  cp "$B"/gir/*.gir "$STAGE$PREFIX/share/gir-1.0/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- glib-typelibs: the GObject introspection data of the ported GLib -- GLib-2.0, GObject-2.0, Gio-2.0, GModule-2.0, GLibUnix-2.0, GioUnix-2.0 and GIRepository-3.0 (.typelib for lib/girepository-1.0, .gir for share/gir-1.0) -- which PyGObject (python3-gobject3) loads to reach GLib from Python. The glib port cannot make them (-Dintrospection=disabled): g-ir-scanner runs a "dumper" program linked against the library it describes, and a QNX program does not run on the build machine. So the same GLib release is built a second time natively, on the build machine, only to describe it: 1. GLib (no introspection), 2. gobject-introspection's g-ir-scanner against it, 3. GLib again with -Dintrospection=enabled, all in build/host/glib-typelibs, once for both architectures. The API description is the same on QNX (same sources, same version); both QNX targets are LP64 little endian like the build machine, so the sizes and offsets g-ir-compiler computes match. What does differ are the values glibconfig.h gives the GLIB_SYSDEF_* constants (AF_INET6, MSG_*): those are taken from the target's staged glibconfig.h before the .typelibs are compiled (fix-constants.py).
- The native builds: the build machine's compiler, pkg-config and libraries (libffi, pcre2, zlib, libmount), nothing of the cross environment.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH glib-typelibs`.

### 5. What it installs

14 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/glib-typelibs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa glib-typelibs
ls dists/8.0/*/ | grep '^glib-typelibs-'
```

This writes `glib-typelibs-<ver>-r0.tar.gz` (run time) and `glib-typelibs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add glib-typelibs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/glib-typelibs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/glib-typelibs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_gobject3`](../../tests/src/) links glib-typelibs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-gobject3`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-gobject3
/usr/local/bin/qnx-port-tests/test_python3_gobject3
```

```
== glib-typelibs ==
  ok   ...
PASS glib-typelibs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_gobject3` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_gobject3`](../../tests/src/) -- a program linked against glib-typelibs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glib-typelibs 2.82.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/glib-typelibs-2.82.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-gobject3 3.58.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-gobject3-3.58.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glib-typelibs 2.82.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/glib-typelibs-2.82.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-gobject3 3.58.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-gobject3-3.58.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.82.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add glib-typelibs=<version>`), sources in `ported/glib-typelibs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates glib-typelibs`; port one with `scripts/port-update glib-typelibs`.

<details><summary>recipe.sh</summary>

```bash
# glib-typelibs: the GObject introspection data of the ported GLib --
# GLib-2.0, GObject-2.0, Gio-2.0, GModule-2.0, GLibUnix-2.0, GioUnix-2.0 and
# GIRepository-3.0 (.typelib for lib/girepository-1.0, .gir for share/gir-1.0)
# -- which PyGObject (python3-gobject3) loads to reach GLib from Python.
#
# The glib port cannot make them (-Dintrospection=disabled): g-ir-scanner
# runs a "dumper" program linked against the library it describes, and a QNX
# program does not run on the build machine. So the same GLib release is
# built a second time natively, on the build machine, only to describe it:
#   1. GLib (no introspection), 2. gobject-introspection's g-ir-scanner
#   against it, 3. GLib again with -Dintrospection=enabled,
# all in build/host/glib-typelibs, once for both architectures.
# The API description is the same on QNX (same sources, same version); both
# QNX targets are LP64 little endian like the build machine, so the sizes
# and offsets g-ir-compiler computes match. What does differ are the values
# glibconfig.h gives the GLIB_SYSDEF_* constants (AF_INET6, MSG_*): those are
# taken from the target's staged glibconfig.h before the .typelibs are
# compiled (fix-constants.py).
VERSION=2.82.4
GIT=https://gitlab.gnome.org/GNOME/glib.git; REF=2.82.4
GI_GIT=https://gitlab.gnome.org/GNOME/gobject-introspection.git; GI_REF=1.86.0
DEPS="glib"
RUNTIME_DEPS=""
BUILDSYS=custom
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT/glib" &&
  git clone -q --depth 1 --branch "$GI_REF" "$GI_GIT" "$SRCROOT/gobject-introspection" || { rm -rf "$SRCROOT"; return 1; }
}
# The native builds: the build machine's compiler, pkg-config and libraries
# (libffi, pcre2, zlib, libmount), nothing of the cross environment.
_host() {
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u PKG_CONFIG -u PKG_CONFIG_LIBDIR \
      -u PKG_CONFIG_SYSROOT_DIR -u QNX_TARGET -u SYSROOT \
      PATH="$H/root/bin:$H/venv/bin:/usr/bin:/bin" PKG_CONFIG_PATH="$H/root/lib/pkgconfig" \
      LD_LIBRARY_PATH="$H/root/lib" "$@"
}
_native() {
  H="$BUILD_DIR/host/glib-typelibs"
  [ -f "$H/done-$VERSION-$GI_REF" ] && return 0
  rm -rf "$H" && mkdir -p "$H" || return 1
  /usr/bin/python3 -m venv "$H/venv" && "$H/venv/bin/pip" install -q "meson>=1.6,<1.9" setuptools packaging || return 1
  local common=( --prefix="$H/root" --libdir=lib --buildtype=release --wrap-mode=nodownload )
  local gopts=( -Dtests=false -Dman-pages=disabled -Ddocumentation=false -Dnls=disabled -Dselinux=disabled
                -Dsysprof=disabled -Dlibmount=disabled -Dxattr=false -Ddtrace=disabled -Dsystemtap=disabled )
  _host meson setup "$H/glib1" "$SRCROOT/glib" "${common[@]}" "${gopts[@]}" -Dintrospection=disabled &&
  _host ninja -C "$H/glib1" install || return 1
  _host meson setup "$H/gi" "$SRCROOT/gobject-introspection" "${common[@]}" -Dbuild_introspection_data=false \
      -Dgtk_doc=false -Ddoctool=disabled -Dcairo=disabled -Dtests=false -Dpython="$H/venv/bin/python" &&
  _host ninja -C "$H/gi" install || return 1
  _host meson setup "$H/glib2" "$SRCROOT/glib" "${common[@]}" "${gopts[@]}" -Dintrospection=enabled &&
  _host ninja -C "$H/glib2" || return 1
  touch "$H/done-$VERSION-$GI_REF"
}
do_build() {
  _native || return 1
  rm -rf "$B/gir" "$B/typelib" && mkdir -p "$B/gir" "$B/typelib" || return 1
  find "$H/glib2" -name '*.gir' -exec cp {} "$B/gir/" \; || return 1
  ls "$B/gir"
  /usr/bin/python3 "$WS/ports/glib-typelibs/fix-constants.py" "$STAGE$PREFIX/lib/glib-2.0/include/glibconfig.h" "$SRCROOT/glib" "$B/gir"/*.gir || return 1
  local g
  for g in "$B/gir"/*.gir; do
    _host gi-compile-repository --includedir="$B/gir" -o "$B/typelib/$(basename "${g%.gir}").typelib" "$g" || return 1
  done
}
do_install() {
  mkdir -p "$STAGE$PREFIX/lib/girepository-1.0" "$STAGE$PREFIX/share/gir-1.0"
  cp "$B"/typelib/*.typelib "$STAGE$PREFIX/lib/girepository-1.0/"
  cp "$B"/gir/*.gir "$STAGE$PREFIX/share/gir-1.0/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
