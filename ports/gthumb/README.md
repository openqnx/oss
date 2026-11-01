# gthumb 3.12.8.2

> Image viewer and organizer for GNOME

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/gthumb/3.12/gthumb-3.12.8.2.tar.xz |
| SHA-256 | `abc57b1103165dd691535796f7dbdba7686217c7d069e974ed0f3d800fe88796` |
| Build system | Meson |
| QNX patches | 4 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gdk-pixbuf`](../gdk-pixbuf/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`lcms2`](../lcms2/) [`exiv2`](../exiv2/) [`libsecret`](../libsecret/) [`json-glib`](../json-glib/) |
| Run-time dependencies | [`cairo`](../cairo/) [`exiv2`](../exiv2/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`lcms2`](../lcms2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gthumb`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gthumb` (programs, shared libraries, run-time data) and `gthumb-dev` (headers, pkg-config/CMake files, static libraries; needs `gthumb`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gthumb`](https://openqnx.github.io/oss/package/x86_64/gthumb.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gthumb.html)) · [`gthumb-dev`](https://openqnx.github.io/oss/package/x86_64/gthumb-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gthumb-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gthumb` (dependencies come along); one version: `apk add gthumb=<version>` | `apk del gthumb` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gthumb` (dependencies first) | `oss-get remove gthumb` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gthumb-dev` the same way, by its own name (`apk add gthumb-dev`, `oss-get install gthumb-dev`, by hand with its own file and list `gthumb-dev.list`); remove it before `gthumb`, which it needs.

```sh
# on the QNX target, as root
apk add gthumb gthumb-dev
apk add qnx-test-gthumb && /usr/local/bin/qnx-port-tests/test_gthumb    # its on-target test, from tests-8.0-<arch>
apk del gthumb gthumb-dev qnx-test-gthumb
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gthumb.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gthumb.list
rm var/lib/manual/gthumb.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gthumb` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (68 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib
```

<details><summary>Full build chain (68)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exiv2`](../exiv2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`json-glib`](../json-glib/) [`lcms2`](../lcms2/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libinih`](../libinih/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/gthumb/3.12/gthumb-3.12.8.2.tar.xz
mkdir -p dist src/gthumb
curl -fL -o dist/$(basename $URL) $URL
echo "abc57b1103165dd691535796f7dbdba7686217c7d069e974ed0f3d800fe88796  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gthumb --strip-components=1
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gthumb < ports/gthumb/patches/0001-aports-001-fix-time_t-type.patch
patch -p1 -d src/gthumb < ports/gthumb/patches/0002-aports-002-fix-xopen-source.patch
patch -p1 -d src/gthumb < ports/gthumb/patches/0003-x11-check-compile-only.patch
patch -p1 -d src/gthumb < ports/gthumb/patches/0004-help-only-with-itstool.patch
```

**[`0001-aports-001-fix-time_t-type.patch`](patches/0001-aports-001-fix-time_t-type.patch)** — QNX change from QNX's aports (extra/gthumb/001-fix-time_t-type.patch).

> Taken from qnx-ports/aports extra/gthumb -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `gthumb/main-migrate-catalogs.c` · Upstream status: QNX-specific

**[`0002-aports-002-fix-xopen-source.patch`](patches/0002-aports-002-fix-xopen-source.patch)** — QNX change from QNX's aports (extra/gthumb/002-fix-xopen-source.patch).

> Taken from qnx-ports/aports extra/gthumb -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `gthumb/gth-time-selector.c` · Upstream status: QNX-specific

**[`0003-x11-check-compile-only.patch`](patches/0003-x11-check-compile-only.patch)** — Detect GDK's X11 backend at compile time, not by running a program.

> meson.build compiles and *runs* a test program to see whether GDK was built
> with X11, which a cross build cannot do ("Can not run test applications in
> this cross environment"). The answer is a preprocessor condition, so a
> compile check that fails without GDK_WINDOWING_X11 gives the same result.

Touches: `meson.build` · Upstream status: QNX-specific

**[`0004-help-only-with-itstool.patch`](patches/0004-help-only-with-itstool.patch)** — Build the help pages only when itstool is available.

> The user manual is translated with itstool at build time; without it on the
> build machine, configuration stops. Skip the help in that case.

Touches: `meson.build` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gthumb src/gthumb \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dlibtiff=true \
      -Dlibwebp=true \
      -Dlcms2=true \
      -Dlibraw=false \
      -Dlibheif=false \
      -Dlibjxl=false \
      -Dcolord=false \
      -Dgstreamer=false \
      -Dlibchamplain=false \
      -Dlibbrasero=false \
      -Dclutter=false \
      -Dlibrsvg=false
ninja -C build/$QNX_ARCH/gthumb
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gthumb install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gThumb image viewer and organiser. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gthumb`.

### 5. What it installs

444 files under `/usr/local`, 1 programs, 118 headers.

```
/usr/local/bin/gthumb
```

The complete list is `stage/<arch>/.manifest/gthumb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gthumb
ls dists/8.0/*/ | grep '^gthumb-'
```

This writes `gthumb-<ver>-r0.tar.gz` (run time) and `gthumb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gthumb
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gthumb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gthumb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gthumb`](../../tests/src/) links gthumb the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gthumb`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gthumb
/usr/local/bin/qnx-port-tests/test_gthumb
```

```
== gthumb ==
  ok   ...
PASS gthumb
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gthumb` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gthumb`](../../tests/src/) -- a program linked against gthumb the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - gthumb --help lists --fullscreen, --slideshow, --import-photos
   - gthumb --version: gthumb 3.12.8.2
   - gthumb --version exits 0 with a display
   - without a display gthumb reports the failed GTK option parse (exit 1)
   - extensions directory
   - at least 20 extensions
   - every extension: key file parses, [Loader] API=3.12, Type=module, lib<name>.so present
   - desktop entry runs gthumb

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gthumb 3.12.8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gthumb-3.12.8.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gthumb 3.12.8.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gthumb-3.12.8.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gthumb 3.12.8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gthumb-3.12.8.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gthumb 3.12.8.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gthumb-3.12.8.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.12.8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gthumb=<version>`), sources in `ported/gthumb/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gthumb`; port one with `scripts/port-update gthumb`.

<details><summary>recipe.sh</summary>

```bash
# gThumb image viewer and organiser.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.12.8.2
TARBALL=https://download.gnome.org/sources/gthumb/3.12/gthumb-3.12.8.2.tar.xz
SHA256=abc57b1103165dd691535796f7dbdba7686217c7d069e974ed0f3d800fe88796
DEPS="glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlibtiff=true -Dlibwebp=true -Dlcms2=true -Dlibraw=false -Dlibheif=false -Dlibjxl=false -Dcolord=false -Dgstreamer=false -Dlibchamplain=false -Dlibbrasero=false -Dclutter=false -Dlibrsvg=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
