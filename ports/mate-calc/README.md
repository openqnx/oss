# mate-calc 1.28.0

> MATE desktop calculator

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz |
| SHA-256 | `804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxml2`](../libxml2/) [`gmp`](../gmp/) [`mpfr`](../mpfr/) [`mpc`](../mpc/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxml2`](../libxml2/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mate_calc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add mate-calc
apk add mate-calc-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-mate-calc && /usr/local/bin/qnx-port-tests/test_mate_calc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install mate-calc`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mate-calc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (55 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxml2 gmp mpfr mpc
```

<details><summary>Full build chain (55)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz
mkdir -p dist src/mate-calc
curl -fL -o dist/$(basename $URL) $URL
echo "804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mate-calc --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0001-aports-fix-angle-parsing-qnx.patch
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0002-aports-fix-radix-qnx.patch
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0003-aports-fix-test-mp-equation-qnx.patch
```

**[`0001-aports-fix-angle-parsing-qnx.patch`](patches/0001-aports-fix-angle-parsing-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-angle-parsing-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/mp-convert.c` · Upstream status: QNX-specific

**[`0002-aports-fix-radix-qnx.patch`](patches/0002-aports-fix-radix-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-radix-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/mp-serializer.c` · Upstream status: QNX-specific

**[`0003-aports-fix-test-mp-equation-qnx.patch`](patches/0003-aports-fix-test-mp-equation-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-test-mp-equation-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/test-mp-equation.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mate-calc && cd build/$QNX_ARCH/mate-calc
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ITSTOOL=/bin/true \
$OLDPWD/src/mate-calc/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MATE Calculator (GTK3). Version as in the QNX Everywhere 8.0.4r1 image.
- itstool only builds the help pages (it would have to run on the build host).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mate-calc`.

### 5. What it installs

36 files under `/usr/local`, 3 programs.

```
/usr/local/bin/mate-calc
/usr/local/bin/mate-calc-cmd
/usr/local/bin/mate-calculator
```

The complete list is `stage/<arch>/.manifest/mate-calc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mate-calc
ls dists/8.0/*/ | grep '^mate-calc-'
```

This writes `mate-calc-<ver>-r0.tar.gz` (run time) and `mate-calc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mate-calc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mate-calc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mate-calc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mate_calc`](../../tests/src/) links mate-calc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mate-calc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mate-calc
/usr/local/bin/qnx-port-tests/test_mate_calc
```

```
== mate-calc ==
  ok   ...
PASS mate-calc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mate_calc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mate_calc`](../../tests/src/) -- a program linked against mate-calc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mate-calc 1.28.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/mate-calc-1.28.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mate-calc 1.28.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mate-calc-1.28.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mate-calc 1.28.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/mate-calc-1.28.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mate-calc 1.28.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mate-calc-1.28.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.28.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mate-calc=<version>`), sources in `ported/mate-calc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mate-calc`; port one with `scripts/port-update mate-calc`.

<details><summary>recipe.sh</summary>

```bash
# MATE Calculator (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.28.0
TARBALL=https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz
SHA256=804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5
DEPS="glib gtk3 libxml2 gmp mpfr mpc"
BUILDSYS=autotools
# itstool only builds the help pages (it would have to run on the build host).
CONF_ENV=( ITSTOOL=/bin/true )
CONF_ARGS_EXTRA=( --disable-nls )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
