# graphviz 13.1.2

> Graph visualization tools: dot, neato and other layout engines

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.com/graphviz/graphviz.git (tag `13.1.2`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`gd`](../gd/) [`cairo`](../cairo/) [`pango`](../pango/) [`expat`](../expat/) [`zlib`](../zlib/) [`libtool`](../libtool/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`libwebp`](../libwebp/) [`poppler`](../poppler/) [`glib`](../glib/) [`libx11`](../libx11/) [`libxrender`](../libxrender/) |
| Run-time dependencies | [`cairo`](../cairo/) [`expat`](../expat/) [`gd`](../gd/) [`gettext`](../gettext/) [`glib`](../glib/) [`harfbuzz`](../harfbuzz/) [`libtool`](../libtool/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxrender`](../libxrender/) [`pango`](../pango/) [`poppler`](../poppler/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_graphviz`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `graphviz` (programs, shared libraries, run-time data) and `graphviz-dev` (headers, pkg-config/CMake files, static libraries; needs `graphviz`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`graphviz`](https://openqnx.github.io/oss/package/x86_64/graphviz.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/graphviz.html)) · [`graphviz-dev`](https://openqnx.github.io/oss/package/x86_64/graphviz-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/graphviz-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add graphviz` (dependencies come along); one version: `apk add graphviz=<version>` | `apk del graphviz` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install graphviz` (dependencies first) | `oss-get remove graphviz` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`graphviz-dev` the same way, by its own name (`apk add graphviz-dev`, `oss-get install graphviz-dev`, by hand with its own file and list `graphviz-dev.list`); remove it before `graphviz`, which it needs.

```sh
# on the QNX target, as root
apk add graphviz graphviz-dev
apk add qnx-test-graphviz && /usr/local/bin/qnx-port-tests/test_graphviz    # its on-target test, from tests-8.0-<arch>
apk del graphviz graphviz-dev qnx-test-graphviz
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/graphviz.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/graphviz.list
rm var/lib/manual/graphviz.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild graphviz` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gd cairo pango expat zlib libtool freetype fontconfig libwebp poppler glib libx11 libxrender
```

<details><summary>Full build chain (53)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gd`](../gd/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libde265`](../libde265/) [`libffi`](../libffi/) [`libheif`](../libheif/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libtool`](../libtool/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxrender`](../libxrender/) [`libyuv`](../libyuv/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openjph`](../openjph/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`poppler`](../poppler/) [`pthread-stubs`](../pthread-stubs/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 13.1.2 --recurse-submodules --shallow-submodules \
    https://gitlab.com/graphviz/graphviz.git src/graphviz
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/graphviz < ports/graphviz/patches/0001-qnx-no-sysexits.patch
patch -p1 -d src/graphviz < ports/graphviz/patches/0002-qnx-dl-iterate-phdr.patch
```

**[`0001-qnx-no-sysexits.patch`](patches/0001-qnx-no-sysexits.patch)** — diffimg: define EX_* on QNX (no <sysexits.h>)

> QNX has no <sysexits.h>. diffimg already defines the five EX_* codes it
> uses for Windows, with the BSD values; use them on QNX too.

Touches: `contrib/diffimg/diffimg.c` · Upstream status: QNX-specific

**[`0002-qnx-dl-iterate-phdr.patch`](patches/0002-qnx-dl-iterate-phdr.patch)** — gvconfig: dl_iterate_phdr() from <sys/link.h> on QNX

> QNX declares dl_iterate_phdr() in <sys/link.h> (there is no <link.h>), and
> its callback type takes a const struct dl_phdr_info *. gvconfig.c uses it to
> find the plugin directory next to libgvc.so. Include the QNX header and
> declare the callback's parameter const there.

Touches: `lib/gvc/gvconfig.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/graphviz; B=build/$QNX_ARCH/graphviz; mkdir -p $B
```

```bash
do_configure() {
  local rl="" d
  for d in "$S"/lib/*/; do d=${d%/}; rl="$rl${rl:+:}$B/lib/${d##*/}"; done
  LDFLAGS="-Wl,-rpath-link,$rl" cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "${CMAKE_ARGS_EXTRA[@]}"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- graphviz: graph layout and drawing -- dot, neato, circo, fdp, sfdp, twopi, osage, patchwork and the graph tools (gvpr, gc, acyclic, tred, ccomps ...), with libcgraph/libgvc for programs (doxygen draws its diagrams with dot).
- Renderers: cairo/pango (PNG, SVG, PDF, PS), gd (GIF, JPEG, ...), webp, PDF input (poppler), the X11 window output (-Txlib, Xwayland), and the core ones (SVG, dot, json, plain). Plugins are loaded with libltdl (libtool) from lib/graphviz, listed in its config file: `dot -c` writes that file on the target -- the apk trigger (apk.trigger) does it whenever a package changes lib/graphviz; after an oss-get install, run `dot -c` once. No GUIs (smyrna, gvedit: GTK2/Qt+OpenGL), no GDK plugin, no language bindings (SWIG, Tcl ...). Python runs the build's generator scripts.
- libgvc reaches libpathplan and libxdot through a static helper library, so CMake does not pass their build directories to the linker as -rpath-link, and the cross linker (which does not follow libgvc's DT_RPATH, as a native one would) cannot resolve libgvc's references when it links dot, gvpr ...: "libgvc.so: undefined reference to `Pshortestpath'". Name every library directory of the build tree for it (LDFLAGS from the environment goes in front of the toolchain's linker flags). Otherwise qnxbuild's own configure.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH graphviz`.

### 5. What it installs

261 files under `/usr/local`: 18 shared-library entries, 38 programs, 25 headers.

```
/usr/local/lib/libcdt.so
/usr/local/lib/libcdt.so.6
/usr/local/lib/libcdt.so.6.0.0
/usr/local/lib/libcgraph.so
/usr/local/lib/libcgraph.so.7
/usr/local/lib/libcgraph.so.7.0.0
/usr/local/lib/libgvc.so
/usr/local/lib/libgvc.so.7
/usr/local/lib/libgvc.so.7.0.0
/usr/local/lib/libgvpr.so
/usr/local/lib/libgvpr.so.2
/usr/local/lib/libgvpr.so.2.0.0
/usr/local/lib/libpathplan.so
/usr/local/lib/libpathplan.so.4
/usr/local/lib/libpathplan.so.4.0.0
/usr/local/lib/libxdot.so
...
```

The complete list is `stage/<arch>/.manifest/graphviz` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa graphviz
ls dists/8.0/*/ | grep '^graphviz-'
```

This writes `graphviz-<ver>-r0.tar.gz` (run time) and `graphviz-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add graphviz
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/graphviz-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/graphviz-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_graphviz`](../../tests/src/) links graphviz the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-graphviz`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-graphviz
/usr/local/bin/qnx-port-tests/test_graphviz
```

```
== graphviz ==
  ok   ...
PASS graphviz
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_graphviz` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_graphviz`](../../tests/src/) -- a program linked against graphviz the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> graphviz`.

## Versions

This recipe builds **13.1.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add graphviz=<version>`), sources in `ported/graphviz/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates graphviz`; port one with `scripts/port-update graphviz`.

<details><summary>recipe.sh</summary>

```bash
# graphviz: graph layout and drawing -- dot, neato, circo, fdp, sfdp, twopi,
# osage, patchwork and the graph tools (gvpr, gc, acyclic, tred, ccomps ...),
# with libcgraph/libgvc for programs (doxygen draws its diagrams with dot).
VERSION=13.1.2; GIT=https://gitlab.com/graphviz/graphviz.git; REF=13.1.2
DEPS="gd cairo pango expat zlib libtool freetype fontconfig libwebp poppler glib libx11 libxrender"
BUILDSYS=cmake
# Renderers: cairo/pango (PNG, SVG, PDF, PS), gd (GIF, JPEG, ...), webp, PDF
# input (poppler), the X11 window output (-Txlib, Xwayland), and
# the core ones (SVG, dot, json, plain). Plugins are loaded with libltdl
# (libtool) from lib/graphviz, listed in its config file: `dot -c` writes that
# file on the target -- the apk trigger (apk.trigger) does it whenever a
# package changes lib/graphviz; after an oss-get install, run `dot -c` once.
# No GUIs (smyrna, gvedit: GTK2/Qt+OpenGL), no GDK plugin, no language
# bindings (SWIG, Tcl ...). Python runs the build's generator scripts.
APK_TRIGGERS="/usr/local/lib/graphviz"
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DENABLE_LTDL=ON -DWITH_EXPAT=ON -DWITH_ZLIB=ON
                   -DWITH_GVEDIT=OFF -DWITH_SMYRNA=OFF -DWITH_GDK=OFF -Dwith_cxx_api=OFF
                   -DENABLE_TCL=OFF -DENABLE_SWIG=OFF -DENABLE_SHARP=OFF -DENABLE_D=OFF -DENABLE_GO=OFF
                   -DENABLE_GUILE=OFF -DENABLE_JAVA=OFF -DENABLE_JAVASCRIPT=OFF -DENABLE_LUA=OFF
                   -DENABLE_PERL=OFF -DENABLE_PHP=OFF -DENABLE_PYTHON=OFF -DENABLE_R=OFF -DENABLE_RUBY=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3
                   -DCMAKE_DISABLE_FIND_PACKAGE_GTK2=ON -DCMAKE_DISABLE_FIND_PACKAGE_GLUT=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_GTS=ON -DCMAKE_DISABLE_FIND_PACKAGE_ANN=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_DevIL=ON -DCMAKE_DISABLE_FIND_PACKAGE_AA=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_GS=ON )

# libgvc reaches libpathplan and libxdot through a static helper library, so
# CMake does not pass their build directories to the linker as -rpath-link,
# and the cross linker (which does not follow libgvc's DT_RPATH, as a native
# one would) cannot resolve libgvc's references when it links dot, gvpr ...:
# "libgvc.so: undefined reference to `Pshortestpath'". Name every library
# directory of the build tree for it (LDFLAGS from the environment goes in
# front of the toolchain's linker flags). Otherwise qnxbuild's own configure.
do_configure() {
  local rl="" d
  for d in "$S"/lib/*/; do d=${d%/}; rl="$rl${rl:+:}$B/lib/${d##*/}"; done
  LDFLAGS="-Wl,-rpath-link,$rl" cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "${CMAKE_ARGS_EXTRA[@]}"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
