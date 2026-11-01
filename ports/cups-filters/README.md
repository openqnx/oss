# cups-filters 1.28.17

> OpenPrinting CUPS filters

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/OpenPrinting/cups-filters/releases/download/1.28.17/cups-filters-1.28.17.tar.xz |
| SHA-256 | `270a3752a960368aa99d431fb5d34f4039b2ac943c576d840612d1d8185c9bb9` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`cups`](../cups/) [`poppler`](../poppler/) [`qpdf`](../qpdf/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`lcms2`](../lcms2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`zlib`](../zlib/) [`glib`](../glib/) |
| Run-time dependencies | [`avahi`](../avahi/) [`cups`](../cups/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`lcms2`](../lcms2/) [`libexif`](../libexif/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`poppler`](../poppler/) [`qpdf`](../qpdf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cups_filters`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `cups-filters` (programs, shared libraries, run-time data) and `cups-filters-dev` (headers, pkg-config/CMake files, static libraries; needs `cups-filters`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`cups-filters`](https://openqnx.github.io/oss/package/x86_64/cups-filters.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cups-filters.html)) · [`cups-filters-dev`](https://openqnx.github.io/oss/package/x86_64/cups-filters-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cups-filters-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add cups-filters` (dependencies come along); one version: `apk add cups-filters=<version>` | `apk del cups-filters` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install cups-filters` (dependencies first) | `oss-get remove cups-filters` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`cups-filters-dev` the same way, by its own name (`apk add cups-filters-dev`, `oss-get install cups-filters-dev`, by hand with its own file and list `cups-filters-dev.list`); remove it before `cups-filters`, which it needs.

```sh
# on the QNX target, as root
apk add cups-filters cups-filters-dev
apk add qnx-test-cups-filters && /usr/local/bin/qnx-port-tests/test_cups_filters    # its on-target test, from tests-8.0-<arch>
apk del cups-filters cups-filters-dev qnx-test-cups-filters
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/cups-filters.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/cups-filters.list
rm var/lib/manual/cups-filters.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cups-filters` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (43 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib
```

<details><summary>Full build chain (43)</summary>

[`avahi`](../avahi/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`cups`](../cups/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`lcms2`](../lcms2/) [`libdaemon`](../libdaemon/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcrypt`](../libxcrypt/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openjpeg`](../openjpeg/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`poppler`](../poppler/) [`pthread-stubs`](../pthread-stubs/) [`qpdf`](../qpdf/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/OpenPrinting/cups-filters/releases/download/1.28.17/cups-filters-1.28.17.tar.xz
mkdir -p dist src/cups-filters
curl -fL -o dist/$(basename $URL) $URL
echo "270a3752a960368aa99d431fb5d34f4039b2ac943c576d840612d1d8185c9bb9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/cups-filters --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cups-filters < ports/cups-filters/patches/0001-gdbus-codegen-srcdir.patch
patch -p1 -d src/cups-filters < ports/cups-filters/patches/0002-pdftoraster-cxx17-operator-new.patch
```

**[`0001-gdbus-codegen-srcdir.patch`](patches/0001-gdbus-codegen-srcdir.patch)** — Generate cups-notifier from the source directory's XML file.

> The rule that generates cups-notifier.[ch] with gdbus-codegen names its
> input as utils/org.cups.cupsd.Notifier.xml, relative to the current
> directory. In an out-of-tree (VPATH) build that is the build directory and
> gdbus-codegen fails with "No such file or directory". Take the file from
> $(srcdir) (Makefile.am and the generated Makefile.in alike).

Touches: `Makefile.am`, `Makefile.in` · Upstream status: Pending

**[`0002-pdftoraster-cxx17-operator-new.patch`](patches/0002-pdftoraster-cxx17-operator-new.patch)** — pdftoraster: no dynamic exception specification from C++17 on.

> pdftoraster replaces operator new/delete with throw(std::bad_alloc)
> specifications, spelled _GLIBCXX_THROW(std::bad_alloc) where libstdc++
> defines that macro, plain "throw" otherwise. QNX's C++ library is libc++,
> which has no _GLIBCXX_THROW, and C++17 (the compiler default here) removed
> dynamic exception specifications: "ISO C++17 does not allow dynamic
> exception specifications". From C++17 on, define the macro empty, as
> libstdc++ itself does.

Touches: `filter/pdftoraster.cxx` · Upstream status: Pending (affects any libc++ build)

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/cups-filters && cd build/$QNX_ARCH/cups-filters
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/cups-filters/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-static \
    --enable-shared \
    --disable-ghostscript \
    --disable-mutool \
    --disable-foomatic \
    --disable-braille \
    --disable-ldap \
    --disable-dbus \
    --disable-avahi \
    --disable-nls \
    --enable-poppler \
    --enable-pclm \
    --enable-driverless \
    --with-pdftops=pdftops \
    --with-pdftops-path="$PREFIX/bin/pdftops" \
    --with-ippfind-path="$PREFIX/bin/ippfind" \
    --with-cups-config="$STAGE$PREFIX/bin/cups-config" \
    --with-fontdir="$PREFIX/share/fonts" \
    --without-rcdir \
    --with-test-font-path=/dev/null
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- cups-filters: the CUPS filters for everything but macOS -- PDF as the print job format: pdftopdf (page handling, on qpdf), pdftoraster and pdftops (Poppler), texttopdf (fontconfig), imagetopdf/imagetoraster, rastertopdf, rastertopclx, the PWG/Apple raster and PCLm filters and driverless (PPDs for IPP Everywhere printers), with libcupsfilters and libfontembed; plus cups-browsed (legacy CUPS browsing only, see below).
- Off: Ghostscript and MuPDF (not ported -- the Poppler-based pdftops and pdftoraster take their place), Foomatic, Braille, LDAP, D-Bus colour management, Avahi (cups-browsed then browses only CUPS's own protocol), NLS. pdftops runs Poppler's pdftops program. cups-config: the staged one (it names the target paths: the filters go to CUPS's server directory, $PREFIX/lib/cups/filter).
- cupsd starts filters with a clean environment and passes no LD_* variable on (see ports/cups): DT_RPATH $PREFIX/lib so the filters find libcups, libqpdf, libpoppler. -z stack-size: 8 MiB main stack like ports/cups (QNX's default main-thread stack is small).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cups-filters`.

### 5. What it installs

91 files under `/usr/local`: 4 shared-library entries, 3 programs, 13 headers.

```
/usr/local/lib/libcupsfilters.so
/usr/local/lib/libcupsfilters.so.1
/usr/local/lib/libfontembed.so
/usr/local/lib/libfontembed.so.1
/usr/local/bin/driverless
/usr/local/bin/driverless-fax
/usr/local/sbin/cups-browsed
```

The complete list is `stage/<arch>/.manifest/cups-filters` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cups-filters
ls dists/8.0/*/ | grep '^cups-filters-'
```

This writes `cups-filters-<ver>-r0.tar.gz` (run time) and `cups-filters-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cups-filters
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cups-filters-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cups-filters-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cups_filters`](../../tests/src/) links cups-filters the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cups-filters`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cups-filters
/usr/local/bin/qnx-port-tests/test_cups_filters
```

```
== cups-filters ==
  ok   ...
PASS cups-filters
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cups_filters` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cups_filters`](../../tests/src/) -- a program linked against cups-filters the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cups-filters 1.28.17-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/cups-filters-1.28.17-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cups-filters 1.28.17-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/cups-filters-1.28.17-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.28.17**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cups-filters=<version>`), sources in `ported/cups-filters/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cups-filters`; port one with `scripts/port-update cups-filters`.

<details><summary>recipe.sh</summary>

```bash
# cups-filters: the CUPS filters for everything but macOS -- PDF as the
# print job format: pdftopdf (page handling, on qpdf), pdftoraster and
# pdftops (Poppler), texttopdf (fontconfig), imagetopdf/imagetoraster,
# rastertopdf, rastertopclx, the PWG/Apple raster and PCLm filters and
# driverless (PPDs for IPP Everywhere printers), with libcupsfilters and
# libfontembed; plus cups-browsed (legacy CUPS browsing only, see below).
VERSION=1.28.17
TARBALL=https://github.com/OpenPrinting/cups-filters/releases/download/1.28.17/cups-filters-1.28.17.tar.xz
SHA256=270a3752a960368aa99d431fb5d34f4039b2ac943c576d840612d1d8185c9bb9
BUILDSYS=autotools
DEPS="cups poppler qpdf fontconfig freetype lcms2 libjpeg-turbo libpng libtiff zlib glib"
RUNTIME_DEPS="cups"
# Off: Ghostscript and MuPDF (not ported -- the Poppler-based pdftops and
# pdftoraster take their place), Foomatic, Braille, LDAP, D-Bus colour
# management, Avahi (cups-browsed then browses only CUPS's own protocol),
# NLS. pdftops runs Poppler's pdftops program.
# cups-config: the staged one (it names the target paths: the filters go
# to CUPS's server directory, $PREFIX/lib/cups/filter).
CONF_ARGS_EXTRA=( --disable-static --enable-shared --disable-ghostscript --disable-mutool
                  --disable-foomatic --disable-braille --disable-ldap --disable-dbus
                  --disable-avahi --disable-nls --enable-poppler --enable-pclm --enable-driverless
                  --with-pdftops=pdftops --with-pdftops-path="$PREFIX/bin/pdftops"
                  --with-ippfind-path="$PREFIX/bin/ippfind"
                  --with-cups-config="$STAGE$PREFIX/bin/cups-config"
                  --with-fontdir="$PREFIX/share/fonts" --without-rcdir --with-test-font-path=/dev/null )
# cupsd starts filters with a clean environment and passes no LD_* variable
# on (see ports/cups): DT_RPATH $PREFIX/lib so the filters find libcups,
# libqpdf, libpoppler. -z stack-size: 8 MiB main stack like ports/cups
# (QNX's default main-thread stack is small).
LDFLAGS_APPEND="-Wl,-rpath,$PREFIX/lib -Wl,--disable-new-dtags -Wl,-z,stack-size=8388608 -Wl,-z,noexecstack"
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
