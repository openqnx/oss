# garcon 4.20.0

> Freedesktop.org compliant menu library for Xfce

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/garcon/4.20/garcon-4.20.0.tar.bz2 |
| SHA-256 | `7fb8517c12309ca4ddf8b42c34bc0c315e38ea077b5442bfcc4509415feada8f` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_garcon`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `garcon` (programs, shared libraries, run-time data) and `garcon-dev` (headers, pkg-config/CMake files, static libraries; needs `garcon`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`garcon`](https://openqnx.github.io/oss/package/x86_64/garcon.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/garcon.html)) · [`garcon-dev`](https://openqnx.github.io/oss/package/x86_64/garcon-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/garcon-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add garcon` (dependencies come along); one version: `apk add garcon=<version>` | `apk del garcon` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install garcon` (dependencies first) | `oss-get remove garcon` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`garcon-dev` the same way, by its own name (`apk add garcon-dev`, `oss-get install garcon-dev`, by hand with its own file and list `garcon-dev.list`); remove it before `garcon`, which it needs.

```sh
# on the QNX target, as root
apk add garcon garcon-dev
apk add qnx-test-garcon && /usr/local/bin/qnx-port-tests/test_garcon    # its on-target test, from tests-8.0-<arch>
apk del garcon garcon-dev qnx-test-garcon
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/garcon.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/garcon.list
rm var/lib/manual/garcon.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild garcon` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (61 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui
```

<details><summary>Full build chain (61)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/garcon/4.20/garcon-4.20.0.tar.bz2
mkdir -p dist src/garcon
curl -fL -o dist/$(basename $URL) $URL
echo "7fb8517c12309ca4ddf8b42c34bc0c315e38ea077b5442bfcc4509415feada8f  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/garcon --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/garcon && cd build/$QNX_ARCH/garcon
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/garcon/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-linker-opts
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag: Xfce keeps its generated sources (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which xfce4-dev-tools 4.20 no longer ships. The tarball has them all.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH garcon`.

### 5. What it installs

110 files under `/usr/local`: 6 shared-library entries, 18 headers.

```
/usr/local/lib/libgarcon-1.so
/usr/local/lib/libgarcon-1.so.0
/usr/local/lib/libgarcon-1.so.0.0.0
/usr/local/lib/libgarcon-gtk3-1.so
/usr/local/lib/libgarcon-gtk3-1.so.0
/usr/local/lib/libgarcon-gtk3-1.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/garcon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa garcon
ls dists/8.0/*/ | grep '^garcon-'
```

This writes `garcon-<ver>-r0.tar.gz` (run time) and `garcon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add garcon
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/garcon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/garcon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_garcon`](../../tests/src/) links garcon the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-garcon`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-garcon
/usr/local/bin/qnx-port-tests/test_garcon
```

```
== garcon ==
  ok   ...
PASS garcon
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_garcon` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_garcon`](../../tests/src/) -- a program linked against garcon the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - desktop file parsed
   - Name / GenericName
   - Exec / Icon
   - Categories
   - Keywords
   - Terminal=false, displayed
   - desktop action parsed
   - NoDisplay=true honoured
   - menu file loaded
   - root menu holds the Development item only
   - one sub-menu named Games
   - Games holds the Game item

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | garcon 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/garcon-4.20.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | garcon 4.20.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/garcon-4.20.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | garcon 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/garcon-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | garcon 4.20.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/garcon-4.20.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add garcon=<version>`), sources in `ported/garcon/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates garcon`; port one with `scripts/port-update garcon`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/garcon/4.20/garcon-4.20.0.tar.bz2
SHA256=7fb8517c12309ca4ddf8b42c34bc0c315e38ea077b5442bfcc4509415feada8f
DEPS="glib gtk3 libxfce4util libxfce4ui"   # garcon-gtk: PKG_CHECK_MODULES libxfce4ui-2
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
