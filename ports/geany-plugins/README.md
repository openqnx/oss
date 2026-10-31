# geany-plugins 2.1

> Collection of plugins for the Geany IDE

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2 |
| SHA-256 | `813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`geany`](../geany/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_geany_plugins`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add geany-plugins
apk add geany-plugins-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-geany-plugins && /usr/local/bin/qnx-port-tests/test_geany_plugins    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install geany-plugins`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild geany-plugins` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (53 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild geany
```

<details><summary>Full build chain (53)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2
mkdir -p dist src/geany-plugins
curl -fL -o dist/$(basename $URL) $URL
echo "813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/geany-plugins --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/geany-plugins < ports/geany-plugins/patches/0001-aports-fix-qnx.patch
```

**[`0001-aports-fix-qnx.patch`](patches/0001-aports-fix-qnx.patch)** — QNX change from QNX's aports (extra/geany-plugins/fix-qnx.patch).

> Taken from qnx-ports/aports extra/geany-plugins -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `geany-plugins-2.1/build//scope.m4`, `geany-plugins-2.1/scope/src/conterm.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/geany-plugins && cd build/$QNX_ARCH/geany-plugins
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/geany-plugins/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --with-geany-libdir=$PREFIX/lib \
    --disable-all-plugins \
    --enable-addons \
    --enable-autoclose \
    --enable-automark \
    --enable-codenav \
    --enable-commander \
    --enable-defineformat \
    --enable-filebrowser \
    --enable-keyrecord \
    --enable-lineoperations \
    --enable-overview \
    --enable-pairtaghighlighter \
    --enable-projectorganizer \
    --enable-sendmail \
    --enable-shiftcolumn \
    --enable-treebrowser \
    --enable-xmlsnippets
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Geany plugin collection (the plugins without extra library dependencies). Version as in the QNX Everywhere 8.0.4r1 image.
- The plugin directory: geany.pc's libdir, which pkg-config reports with the sysroot (the staging tree) in front -- the plugins were installed, and packaged, under <stage>/usr/local/lib/geany, where geany never looks.
- The QNX patch touches the autotools inputs; regenerate the build files.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH geany-plugins`.

### 5. What it installs

116 files under `/usr/local`: 2 shared-library entries.

```
/usr/local/lib/libgeanypluginutils.so
/usr/local/lib/libgeanypluginutils.so.0
```

The complete list is `stage/<arch>/.manifest/geany-plugins` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa geany-plugins
ls dists/8.0/*/ | grep '^geany-plugins-'
```

This writes `geany-plugins-<ver>-r0.tar.gz` (run time) and `geany-plugins-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add geany-plugins
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/geany-plugins-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/geany-plugins-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_geany_plugins`](../../tests/src/) links geany-plugins the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-geany-plugins`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-geany-plugins
/usr/local/bin/qnx-port-tests/test_geany_plugins
```

```
== geany-plugins ==
  ok   ...
PASS geany-plugins
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_geany_plugins` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_geany_plugins`](../../tests/src/) -- a program linked against geany-plugins the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - three matching files found
   - main.c, main.h and sub/util.c listed
   - ignored directory, ignored file and non-matching file left out
   - ADD_DIRS lists directories too; NULL pattern matches every file
   - single-path pattern check (whole-path patterns)
   - every plugin in /usr/local/lib/geany loads and exports a plugin entry point

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r0 | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/geany-plugins-2.1-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/geany-plugins-2.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r0 | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/geany-plugins-2.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | geany-plugins 2.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/geany-plugins-2.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add geany-plugins=<version>`), sources in `ported/geany-plugins/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates geany-plugins`; port one with `scripts/port-update geany-plugins`.

<details><summary>recipe.sh</summary>

```bash
# The Geany plugin collection (the plugins without extra library dependencies).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.1
TARBALL=https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2
SHA256=813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70
DEPS="geany"
BUILDSYS=autotools
# The plugin directory: geany.pc's libdir, which pkg-config reports with the
# sysroot (the staging tree) in front -- the plugins were installed, and
# packaged, under <stage>/usr/local/lib/geany, where geany never looks.
CONF_ARGS_EXTRA=( --with-geany-libdir=$PREFIX/lib --disable-all-plugins --enable-addons --enable-autoclose --enable-automark --enable-codenav --enable-commander --enable-defineformat --enable-filebrowser --enable-keyrecord --enable-lineoperations --enable-overview --enable-pairtaghighlighter --enable-projectorganizer --enable-sendmail --enable-shiftcolumn --enable-treebrowser --enable-xmlsnippets )
# The QNX patch touches the autotools inputs; regenerate the build files.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
