# gobject-introspection 1.86.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gobject-introspection.git (tag `1.86.0`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libffi`](../libffi/) [`python3`](../python3/) |
| Run-time dependencies | [`glib`](../glib/) [`libffi`](../libffi/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gobject-introspection
apk add gobject-introspection-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gobject-introspection`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gobject-introspection` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libffi python3
```

<details><summary>Full build chain (17)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.86.0 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gobject-introspection.git src/gobject-introspection
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gobject-introspection; B=build/$QNX_ARCH/gobject-introspection; mkdir -p $B
```

```bash
do_configure() {
  local v="$BUILD_DIR/host/meson-venv"
  [ -x "$v/bin/meson" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q "meson>=1.6,<1.9"; } || return 1
  local g="$BUILD_DIR/host/gi-venv"
  [ -x "$g/bin/python" ] || { "$WS/hosttools/bin/python$QNX_PYV" -m venv "$g" && "$g/bin/pip" install -q setuptools; } || return 1
  "$v/bin/meson" setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$QNX_ARCH.ini" --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release "${MESON_ARGS_EXTRA[@]}"
}
```

```bash
do_build() { "$BUILD_DIR/host/meson-venv/bin/meson" compile -C "$B"; }
```

```bash
do_install() { DESTDIR="$STAGE" "$BUILD_DIR/host/meson-venv/bin/meson" install -C "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GObject Introspection: libgirepository and the g-ir tools for the target. The introspection data (.gir/.typelib) of GLib itself cannot be generated in a cross build without running target binaries, so it is not built here (build_introspection_data=false). Version as in the QNX Everywhere image.
- The giscanner extension module is built against the target's Python (the python3 port); the build host's Python of the same version describes it.
- gobject-introspection 1.86 needs Meson >= 1.4; it gets its own from a venv of the build host's Python, so the host's Meson is left as it is.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gobject-introspection`.

### 5. What it installs

133 files under `/usr/local`: 3 shared-library entries, 5 programs, 22 headers.

```
/usr/local/lib/libgirepository-1.0.so
/usr/local/lib/libgirepository-1.0.so.1
/usr/local/lib/libgirepository-1.0.so.1.0.0
/usr/local/bin/g-ir-annotation-tool
/usr/local/bin/g-ir-compiler
/usr/local/bin/g-ir-generate
/usr/local/bin/g-ir-inspect
/usr/local/bin/g-ir-scanner
```

The complete list is `stage/<arch>/.manifest/gobject-introspection` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gobject-introspection
ls dists/8.0/*/ | grep '^gobject-introspection-'
```

This writes `gobject-introspection-<ver>-r0.tar.gz` (run time) and `gobject-introspection-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gobject-introspection
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gobject-introspection-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gobject-introspection-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gobject-introspection 1.86.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gobject-introspection-1.86.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gobject-introspection 1.86.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gobject-introspection-1.86.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.86.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gobject-introspection=<version>`), sources in `ported/gobject-introspection/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gobject-introspection`; port one with `scripts/port-update gobject-introspection`.

<details><summary>recipe.sh</summary>

```bash
# GObject Introspection: libgirepository and the g-ir tools for the target.
# The introspection data (.gir/.typelib) of GLib itself cannot be generated
# in a cross build without running target binaries, so it is not built here
# (build_introspection_data=false). Version as in the QNX Everywhere image.
VERSION=1.86.0; GIT=https://gitlab.gnome.org/GNOME/gobject-introspection.git; REF=1.86.0
DEPS="glib libffi python3"
BUILDSYS=meson
QNX_PYV=3.14
# The giscanner extension module is built against the target's Python (the
# python3 port); the build host's Python of the same version describes it.
MESON_ARGS_EXTRA=( -Dbuild_introspection_data=false -Dgtk_doc=false -Ddoctool=disabled -Dcairo=disabled -Dpython=$BUILD_DIR/host/gi-venv/bin/python -Dc_args=-I$STAGE$PREFIX/include/python$QNX_PYV )
# gobject-introspection 1.86 needs Meson >= 1.4; it gets its own from a venv of
# the build host's Python, so the host's Meson is left as it is.
BUILDSYS=custom
do_configure() {
  local v="$BUILD_DIR/host/meson-venv"
  [ -x "$v/bin/meson" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q "meson>=1.6,<1.9"; } || return 1
  local g="$BUILD_DIR/host/gi-venv"
  [ -x "$g/bin/python" ] || { "$WS/hosttools/bin/python$QNX_PYV" -m venv "$g" && "$g/bin/pip" install -q setuptools; } || return 1
  "$v/bin/meson" setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$QNX_ARCH.ini" --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release "${MESON_ARGS_EXTRA[@]}"
}
do_build() { "$BUILD_DIR/host/meson-venv/bin/meson" compile -C "$B"; }
do_install() { DESTDIR="$STAGE" "$BUILD_DIR/host/meson-venv/bin/meson" install -C "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
