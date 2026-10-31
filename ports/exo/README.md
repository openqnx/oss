# exo 4.20.0

> Extension library and helper applications for the Xfce desktop

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/exo/4.20/exo-4.20.0.tar.bz2 |
| SHA-256 | `4277f799245f1efde01cd917fd538ba6b12cf91c9f8a73fe2035fd5456ec078d` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_exo`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add exo
apk add exo-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-exo && /usr/local/bin/qnx-port-tests/test_exo    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install exo`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild exo` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui
```

<details><summary>Full build chain (55)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/exo/4.20/exo-4.20.0.tar.bz2
mkdir -p dist src/exo
curl -fL -o dist/$(basename $URL) $URL
echo "4277f799245f1efde01cd917fd538ba6b12cf91c9f8a73fe2035fd5456ec078d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/exo --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/exo && cd build/$QNX_ARCH/exo
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/exo/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH exo`.

### 5. What it installs

96 files under `/usr/local`: 2 shared-library entries, 2 programs, 19 headers.

```
/usr/local/lib/libexo-2.so
/usr/local/lib/libexo-2.so.1
/usr/local/bin/exo-desktop-item-edit
/usr/local/bin/exo-open
```

The complete list is `stage/<arch>/.manifest/exo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa exo
ls dists/8.0/*/ | grep '^exo-'
```

This writes `exo-<ver>-r0.tar.gz` (run time) and `exo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add exo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/exo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/exo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_exo`](../../tests/src/) links exo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-exo`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-exo
/usr/local/bin/qnx-port-tests/test_exo
```

```
== exo ==
  ok   ...
PASS exo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_exo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_exo`](../../tests/src/) -- a program linked against exo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - exo 4.20.0
   - exo_check_version (upstream stub: always NULL)
   - exo_str_elide_underscores (\"__\" keeps one)
   - exo_str_replace
   - exo_str_is_equal (NULL-safe)
   - exo_str_looks_like_an_uri
   - exo_str_is_flag
   - exo_strdup_strftime
   - exo_strndupv (first 2 of 3)
   - exo_binding: follows; with negation: inverts
   - bindings track a second change
   - exo_mutual_binding: target -> source
   - exo_mutual_binding: source -> target
   - scale_ratio 100x50 -> 20x10
   - scale_down 100x50 into 40x40 -> 40x20
   - scale_down: already small enough -> the same pixbuf
   - lucent 50%: colour kept, alpha 255 -> 127
   - lucent 20% on RGB: alpha channel added, 51
   - scratch directory
   - exo-open --version: exo-open 4.20.0
   - exo-open --help lists --launch TYPE
   - without a display exo-open stops at the display probe

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | exo 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/exo-4.20.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | exo 4.20.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/exo-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | exo 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/exo-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | exo 4.20.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/exo-4.20.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add exo=<version>`), sources in `ported/exo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates exo`; port one with `scripts/port-update exo`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/exo/4.20/exo-4.20.0.tar.bz2
SHA256=4277f799245f1efde01cd917fd538ba6b12cf91c9f8a73fe2035fd5456ec078d
DEPS="glib gtk3 libxfce4util libxfce4ui"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
