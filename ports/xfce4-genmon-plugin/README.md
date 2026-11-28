# xfce4-genmon-plugin 4.3.0

> Xfce panel plugin showing the output of a command or script, run periodically

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/panel-plugins/xfce4-genmon-plugin/4.3/xfce4-genmon-plugin-4.3.0.tar.xz |
| SHA-256 | `077197911d84e5ba22e7bb895ce6c038dbbd8e8e0067ed6f4e48502b7167a282` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`xfce4-panel`](../xfce4-panel/) |
| Run-time dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_genmon_plugin`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfce4-genmon-plugin` and `xfce4-genmon-plugin-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-genmon-plugin` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (66 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel
```

<details><summary>Full build chain (66)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/panel-plugins/xfce4-genmon-plugin/4.3/xfce4-genmon-plugin-4.3.0.tar.xz
mkdir -p dist src/xfce4-genmon-plugin
curl -fL -o dist/$(basename $URL) $URL
echo "077197911d84e5ba22e7bb895ce6c038dbbd8e8e0067ed6f4e48502b7167a282  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-genmon-plugin --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-genmon-plugin src/xfce4-genmon-plugin \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/xfce4-genmon-plugin
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-genmon-plugin install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-genmon-plugin: the generic monitor of the Xfce panel. It runs a command or script at a set interval and shows its output in the panel -- plain text, or text, an icon, a bar, a tooltip and a click command when the output uses the <txt>, <img>, <bar>, <tool>, <click> tags. The way to put a temperature, a voltage warning or any other reading in the panel with a few lines of shell.
- Release tarball, as for every Xfce package here: it carries the generated sources a git checkout makes with xfce4-dev-tools.
- The panel loads the plugin; its settings are in xfconf (channel xfce4-panel).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-genmon-plugin`.

### 5. What it installs

75 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/xfce4-genmon-plugin` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-genmon-plugin
ls dists/8.0/*/ | grep '^xfce4-genmon-plugin-'
```

This writes `xfce4-genmon-plugin-<ver>-r0.tar.gz` (run time) and `xfce4-genmon-plugin-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-genmon-plugin
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-genmon-plugin-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-genmon-plugin-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_genmon_plugin`](../../tests/src/) links xfce4-genmon-plugin the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-genmon-plugin`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-genmon-plugin
/usr/local/bin/qnx-port-tests/test_xfce4_genmon_plugin
```

```
== xfce4-genmon-plugin ==
  ok   ...
PASS xfce4-genmon-plugin
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_genmon_plugin` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_genmon_plugin`](../../tests/src/) -- a program linked against xfce4-genmon-plugin the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfce4-genmon-plugin`.

## Versions

This recipe builds **4.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-genmon-plugin=<version>`), sources in `ported/xfce4-genmon-plugin/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-genmon-plugin`; port one with `scripts/port-update xfce4-genmon-plugin`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-genmon-plugin: the generic monitor of the Xfce panel. It runs a
# command or script at a set interval and shows its output in the panel --
# plain text, or text, an icon, a bar, a tooltip and a click command when
# the output uses the <txt>, <img>, <bar>, <tool>, <click> tags. The way to
# put a temperature, a voltage warning or any other reading in the panel
# with a few lines of shell.
VERSION=4.3.0
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/panel-plugins/xfce4-genmon-plugin/4.3/xfce4-genmon-plugin-4.3.0.tar.xz
SHA256=077197911d84e5ba22e7bb895ce6c038dbbd8e8e0067ed6f4e48502b7167a282
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel"
# The panel loads the plugin; its settings are in xfconf (channel xfce4-panel).
RUNTIME_DEPS="xfce4-panel xfconf"
BUILDSYS=meson
# No options. The example scripts in share/xfce4/genmon/scripts are upstream's:
# most read Linux's /proc and /sys (monCPU, monBat, monWIFI, sysstat) and are
# examples of the output format rather than monitors for QNX; datetime and
# monTime work as they are.
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
