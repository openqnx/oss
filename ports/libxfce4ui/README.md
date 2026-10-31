# libxfce4ui 4.20.0

> Shared widgets library for Xfce applications

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/libxfce4ui/4.20/libxfce4ui-4.20.0.tar.bz2 |
| SHA-256 | `75e8996984f20375aadecd5c16f5147c211ed0bd26d7861ab0257561eb76eaee` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`xfconf`](../xfconf/) [`libepoxy`](../libepoxy/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`xfconf`](../xfconf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxfce4ui`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libxfce4ui` (programs, shared libraries, run-time data) and `libxfce4ui-dev` (headers, pkg-config/CMake files, static libraries; needs `libxfce4ui`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libxfce4ui`](https://openqnx.github.io/oss/package/x86_64/libxfce4ui.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxfce4ui.html)) · [`libxfce4ui-dev`](https://openqnx.github.io/oss/package/x86_64/libxfce4ui-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxfce4ui-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libxfce4ui` (dependencies come along); one version: `apk add libxfce4ui=<version>` | `apk del libxfce4ui` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libxfce4ui` (dependencies first) | `oss-get remove libxfce4ui` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libxfce4ui-dev` the same way, by its own name (`apk add libxfce4ui-dev`, `oss-get install libxfce4ui-dev`, by hand with its own file and list `libxfce4ui-dev.list`); remove it before `libxfce4ui`, which it needs.

```sh
# on the QNX target, as root
apk add libxfce4ui libxfce4ui-dev
apk add qnx-test-libxfce4ui && /usr/local/bin/qnx-port-tests/test_libxfce4ui    # its on-target test
apk del libxfce4ui libxfce4ui-dev qnx-test-libxfce4ui
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libxfce4ui.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libxfce4ui.list
rm var/lib/manual/libxfce4ui.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxfce4ui` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (54 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util xfconf libepoxy
```

<details><summary>Full build chain (54)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/libxfce4ui/4.20/libxfce4ui-4.20.0.tar.bz2
mkdir -p dist src/libxfce4ui
curl -fL -o dist/$(basename $URL) $URL
echo "75e8996984f20375aadecd5c16f5147c211ed0bd26d7861ab0257561eb76eaee  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxfce4ui --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libxfce4ui && cd build/$QNX_ARCH/libxfce4ui
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxfce4ui/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-linker-opts \
    --disable-x11 \
    --enable-wayland
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag: Xfce keeps its generated sources (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
- hicolor-icon-theme at run time: libxfce4ui installs its icons (org.xfce.about and friends) under share/icons/hicolor, but GTK finds nothing there without hicolor's index.theme, and no DT_NEEDED shows that dependency.  On a target without it xfce_gicon_from_name() returned NULL for every themed icon.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxfce4ui`.

### 5. What it installs

100 files under `/usr/local`: 4 shared-library entries, 1 programs, 17 headers.

```
/usr/local/lib/libxfce4kbd-private-3.so
/usr/local/lib/libxfce4kbd-private-3.so.0
/usr/local/lib/libxfce4ui-2.so
/usr/local/lib/libxfce4ui-2.so.0
/usr/local/bin/xfce4-about
```

The complete list is `stage/<arch>/.manifest/libxfce4ui` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxfce4ui
ls dists/8.0/*/ | grep '^libxfce4ui-'
```

This writes `libxfce4ui-<ver>-r0.tar.gz` (run time) and `libxfce4ui-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxfce4ui
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxfce4ui-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxfce4ui-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxfce4ui`](../../tests/src/) links libxfce4ui the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxfce4ui`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxfce4ui
/usr/local/bin/qnx-port-tests/test_libxfce4ui
```

```
== libxfce4ui ==
  ok   ...
PASS libxfce4ui
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxfce4ui` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxfce4ui`](../../tests/src/) -- a program linked against libxfce4ui the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - xfce_gicon_from_name: themed icon for a named icon
   - themed icon names start with org.xfce.about
   - desktop file with an absolute Icon path -> GFileIcon
   - GFileIcon points at that path
   - scratch directory
   - libxfce4ui 4.20.0
   - libxfce4ui_check_version
   - xfce_icon_name_from_desktop_id(xfce4-about) = org.xfce.about
   - xfce_gtk_get_action_entry_by_id finds the entry (labels kept in the C locale)
   - xfce4-about --version: 4.20.0
   - gtk_init_check on the headless display
   - XfceTitledDialog with mixed buttons
   - subtitle set and read back
   - dialog title
   - OK response button is \"_Save\"
   - xfce_gtk_button_new_mixed
   - xfce_gtk_frame_box_new returns a frame and its container
   - menu item built from the action entry and appended
   - activating the menu item runs the entry's callback
   - xfce_spawn_command_line ran the child
   - without a display xfce4-about stops at the display probe

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfce4ui 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxfce4ui-4.20.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfce4ui 4.20.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxfce4ui-4.20.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfce4ui 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxfce4ui-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfce4ui 4.20.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxfce4ui-4.20.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxfce4ui=<version>`), sources in `ported/libxfce4ui/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxfce4ui`; port one with `scripts/port-update libxfce4ui`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4ui/4.20/libxfce4ui-4.20.0.tar.bz2
SHA256=75e8996984f20375aadecd5c16f5147c211ed0bd26d7861ab0257561eb76eaee
DEPS="glib gtk3 libxfce4util xfconf libepoxy"
# hicolor-icon-theme at run time: libxfce4ui installs its icons (org.xfce.about
# and friends) under share/icons/hicolor, but GTK finds nothing there without
# hicolor's index.theme, and no DT_NEEDED shows that dependency.  On a target
# without it xfce_gicon_from_name() returned NULL for every themed icon.
RUNTIME_DEPS="hicolor-icon-theme"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts --disable-x11 --enable-wayland )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
