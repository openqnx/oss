# fcft 3.3.2

> Font loading and glyph rasterization library on FontConfig, FreeType and pixman

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://codeberg.org/dnkl/fcft.git (tag `3.3.2`) |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`tllist`](../tllist/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`utf8proc`](../utf8proc/) [`pixman`](../pixman/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`pixman`](../pixman/) [`utf8proc`](../utf8proc/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fcft`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `fcft` (programs, shared libraries, run-time data) and `fcft-dev` (headers, pkg-config/CMake files, static libraries; needs `fcft`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`fcft`](https://openqnx.github.io/oss/package/x86_64/fcft.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fcft.html)) · [`fcft-dev`](https://openqnx.github.io/oss/package/x86_64/fcft-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/fcft-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add fcft` (dependencies come along); one version: `apk add fcft=<version>` | `apk del fcft` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install fcft` (dependencies first) | `oss-get remove fcft` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`fcft-dev` the same way, by its own name (`apk add fcft-dev`, `oss-get install fcft-dev`, by hand with its own file and list `fcft-dev.list`); remove it before `fcft`, which it needs.

```sh
# on the QNX target, as root
apk add fcft fcft-dev
apk add qnx-test-fcft && /usr/local/bin/qnx-port-tests/test_fcft    # its on-target test, from tests-8.0-<arch>
apk del fcft fcft-dev qnx-test-fcft
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/fcft.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/fcft.list
rm var/lib/manual/fcft.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fcft` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild tllist fontconfig freetype harfbuzz utf8proc pixman
```

<details><summary>Full build chain (20)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`tllist`](../tllist/) [`utf8proc`](../utf8proc/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.3.2 --recurse-submodules --shallow-submodules \
    https://codeberg.org/dnkl/fcft.git src/fcft
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fcft < ports/fcft/patches/0001-qnx-stdlib-min-max.patch
patch -p1 -d src/fcft < ports/fcft/patches/0002-qnx-no-xlocale.patch
```

**[`0001-qnx-stdlib-min-max.patch`](patches/0001-qnx-stdlib-min-max.patch)** — #undef QNX's <stdlib.h> min()/max() before defining fcft's own

> QNX's <stdlib.h> defines min() and max() macros (for C, under the default
> _QNX_SOURCE extensions); fcft.c and svg-backend-nanosvg.c define their own and the redefinition is an
> error under the project's -Werror. #undef them first; the definitions are the
> same expressions.

Touches: `fcft.c`, `svg-backend-nanosvg.c` · Upstream status: QNX-specific

**[`0002-qnx-no-xlocale.patch`](patches/0002-qnx-no-xlocale.patch)** — No temporary C numeric locale on QNX (no xlocale; only the C locale)

> QNX has no xlocale: no locale_t, newlocale(), uselocale() or freelocale().
> fcft switches LC_NUMERIC to "C" around FcNameParse() so fontconfig parses
> floating point sizes ("monospace:size=10.5") in any locale. QNX only has the
> "C" locale, so the numeric locale is always "C" already: skip the switch.

Touches: `fcft.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/fcft src/fcft \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=disabled \
      -Dgrapheme-shaping=enabled \
      -Drun-shaping=enabled \
      -Dsvg-backend=nanosvg \
      -Dtest-text-shaping=false
ninja -C build/$QNX_ARCH/fcft
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/fcft install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- fcft: font loading and glyph rasterization on fontconfig, FreeType and pixman, with HarfBuzz/utf8proc text shaping -- the font library of the foot terminal.
- - grapheme and run shaping on (foot uses both; HarfBuzz and utf8proc are here); - SVG glyphs with the bundled nanosvg (upstream's default); - no man pages: they need scdoc on the build machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fcft`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libfcft.so
/usr/local/lib/libfcft.so.4
/usr/local/lib/libfcft.so.4.3.2
```

The complete list is `stage/<arch>/.manifest/fcft` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fcft
ls dists/8.0/*/ | grep '^fcft-'
```

This writes `fcft-<ver>-r0.tar.gz` (run time) and `fcft-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fcft
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fcft-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fcft-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fcft`](../../tests/src/) links fcft the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fcft`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fcft
/usr/local/bin/qnx-port-tests/test_fcft
```

```
== fcft ==
  ok   ...
PASS fcft
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fcft` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fcft`](../../tests/src/) -- a program linked against fcft the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - fcft_init
   - built with grapheme and text-run shaping (HarfBuzz, utf8proc)
   - built with SVG glyphs (nanosvg)
   - fcft_from_name('DejaVu Sans Mono:pixelsize=20')
   - fontconfig resolved it to DejaVu Sans Mono
   - monospace advance 12 px (0.602 em at 20 px)
   - line height 20-26 px
   - 'A' rasterized: about 11x15 px
   - 'A' has ink in its pixman image
   - 'i' and 'W' advance alike, 'W' is wider ink
   - precompose e + U+0301 = U+00E9
   - text run 'QNX 8' shaped: 5 glyphs, 60 px

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> fcft`.

## Versions

This recipe builds **3.3.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fcft=<version>`), sources in `ported/fcft/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fcft`; port one with `scripts/port-update fcft`.

<details><summary>recipe.sh</summary>

```bash
# fcft: font loading and glyph rasterization on fontconfig, FreeType and
# pixman, with HarfBuzz/utf8proc text shaping -- the font library of the foot
# terminal.
VERSION=3.3.2; GIT=https://codeberg.org/dnkl/fcft.git; REF=3.3.2
DEPS="tllist fontconfig freetype harfbuzz utf8proc pixman"
BUILDSYS=meson
# - grapheme and run shaping on (foot uses both; HarfBuzz and utf8proc are here);
# - SVG glyphs with the bundled nanosvg (upstream's default);
# - no man pages: they need scdoc on the build machine.
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dgrapheme-shaping=enabled -Drun-shaping=enabled
                   -Dsvg-backend=nanosvg -Dtest-text-shaping=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
