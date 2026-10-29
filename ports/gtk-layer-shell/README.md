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

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gtk-layer-shell
oss-get install gtk-layer-shell-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH gtk-layer-shell
ls repo/8.0/*/ | grep '^gtk-layer-shell-'
```

This writes `gtk-layer-shell-<ver>-r0.tar.gz` (run time) and `gtk-layer-shell-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gtk-layer-shell
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gtk-layer-shell-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gtk-layer-shell-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gtk_layer_shell`](../../tests/src/) links gtk-layer-shell the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk_layer_shell root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gtk_layer_shell'
```

```
== gtk-layer-shell ==
  ok   ...
PASS gtk-layer-shell
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
