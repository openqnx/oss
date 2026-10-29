# gtk-layer-shell 0.9.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/wmww/gtk-layer-shell.git (tag `v0.9.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) |
| Run-time dependencies | [`cairo`](../cairo/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gtk_layer_shell`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gtk-layer-shell
apk add gtk-layer-shell-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gtk-layer-shell && /usr/local/bin/qnx-port-tests/test_gtk_layer_shell    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gtk-layer-shell`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtk-layer-shell` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (52 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 wayland wayland-protocols
```

<details><summary>Full build chain (52)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.9.0 --recurse-submodules --shallow-submodules \
    https://github.com/wmww/gtk-layer-shell.git src/gtk-layer-shell
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gtk-layer-shell src/gtk-layer-shell \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dexamples=false \
      -Ddocs=false \
      -Dintrospection=false \
      -Dvapi=false
ninja -C build/$QNX_ARCH/gtk-layer-shell
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gtk-layer-shell install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtk-layer-shell`.

### 5. What it installs

6 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libgtk-layer-shell.so
/usr/local/lib/libgtk-layer-shell.so.0
/usr/local/lib/libgtk-layer-shell.so.0.9.0
```

The complete list is `stage/<arch>/.manifest/gtk-layer-shell` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtk-layer-shell
ls dists/8.0/*/ | grep '^gtk-layer-shell-'
```

This writes `gtk-layer-shell-<ver>-r0.tar.gz` (run time) and `gtk-layer-shell-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtk-layer-shell
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtk-layer-shell-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtk-layer-shell-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gtk_layer_shell`](../../tests/src/) links gtk-layer-shell the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gtk-layer-shell`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gtk-layer-shell
/usr/local/bin/qnx-port-tests/test_gtk_layer_shell
```

```
== gtk-layer-shell ==
  ok   ...
PASS gtk-layer-shell
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk_layer_shell` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gtk_layer_shell`](../../tests/src/) -- a program linked against gtk-layer-shell the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version entry points
   - library loaded and linked

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.9.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtk-layer-shell=<version>`), sources in `ported/gtk-layer-shell/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtk-layer-shell`; port one with `scripts/port-update gtk-layer-shell`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.9.0; GIT=https://github.com/wmww/gtk-layer-shell.git; REF=v0.9.0
DEPS="glib gtk3 wayland wayland-protocols"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dexamples=false -Ddocs=false
                   -Dintrospection=false -Dvapi=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
