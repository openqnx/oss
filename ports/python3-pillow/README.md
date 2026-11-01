# python3-pillow 13.0.0

> Python Imaging Library (fork): image processing and file formats

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/ce/a4/9cc30690a394a8305e8be4c9036aa16d0dfddadd09079860b582f562496d/pillow-13.0.0.tar.gz |
| SHA-256 | `cdcd1ba258178f7356d74170c9a81ba4eab6a5d55748bc402a5a2a6fe6be3510` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`zlib`](../zlib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`freetype`](../freetype/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`lcms2`](../lcms2/) [`openjpeg`](../openjpeg/) [`libraqm`](../libraqm/) |
| Run-time dependencies | [`freetype`](../freetype/) [`lcms2`](../lcms2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libraqm`](../libraqm/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`openjpeg`](../openjpeg/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_pillow`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-pillow`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-pillow`](https://openqnx.github.io/oss/package/x86_64/python3-pillow.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-pillow.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-pillow` (dependencies come along); one version: `apk add python3-pillow=<version>` | `apk del python3-pillow` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-pillow` (dependencies first) | `oss-get remove python3-pillow` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-pillow && /usr/local/bin/qnx-port-tests/test_python3_pillow    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-pillow qnx-test-python3-pillow
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-pillow.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-pillow.list
rm var/lib/manual/python3-pillow.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-pillow` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (31 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm
```

<details><summary>Full build chain (31)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libraqm`](../libraqm/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`openjpeg`](../openjpeg/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/ce/a4/9cc30690a394a8305e8be4c9036aa16d0dfddadd09079860b582f562496d/pillow-13.0.0.tar.gz
mkdir -p dist src/python3-pillow
curl -fL -o dist/$(basename $URL) $URL
echo "cdcd1ba258178f7356d74170c9a81ba4eab6a5d55748bc402a5a2a6fe6be3510  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-pillow --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-pillow; B=build/$QNX_ARCH/python3-pillow; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77" "pybind11"; }
```

```bash
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  LDCXXSHARED="q++ -V${QCC_TARGET}_cxx -shared" \
  qnx_py_wheel -Cplatform-guessing=disable -Czlib=enable -Cjpeg=enable -Cfreetype=enable \
    -Ctiff=enable -Cwebp=enable -Clcms=enable -Cjpeg2000=enable -Craqm=enable \
    -Cimagequant=disable -Cavif=disable -Cxcb=disable
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Pillow: the Python imaging library (image file formats, drawing, filters, fonts). A setuptools C extension, cross compiled like numpy (scripts/lib/pywheel.sh). platform-guessing=disable: Pillow's setup.py otherwise searches the build machine's /usr/lib and /usr/include; with it off, libraries are found through pkg-config (the staging tree) and the target's sysconfig flags. Required (build fails without): zlib, JPEG, FreeType; also TIFF, WebP, LCMS2, OpenJPEG and libraqm (complex text layout) from the ported libraries. Not built: imagequant, AVIF, XCB.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-pillow`.

### 5. What it installs

118 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-pillow` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-pillow
ls dists/8.0/*/ | grep '^python3-pillow-'
```

This writes `python3-pillow-<ver>-r0.tar.gz` (run time) and `python3-pillow-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-pillow
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-pillow-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-pillow-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_pillow`](../../tests/src/) links python3-pillow the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-pillow`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-pillow
/usr/local/bin/qnx-port-tests/test_python3_pillow
```

```
== python3-pillow ==
  ok   ...
PASS python3-pillow
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_pillow` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_pillow`](../../tests/src/) -- a program linked against python3-pillow the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pillow 13.0.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pillow-13.0.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pillow 13.0.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pillow-13.0.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **13.0.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-pillow=<version>`), sources in `ported/python3-pillow/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-pillow`; port one with `scripts/port-update python3-pillow`.

<details><summary>recipe.sh</summary>

```bash
# Pillow: the Python imaging library (image file formats, drawing, filters,
# fonts). A setuptools C extension, cross compiled like numpy
# (scripts/lib/pywheel.sh). platform-guessing=disable: Pillow's setup.py
# otherwise searches the build machine's /usr/lib and /usr/include; with it
# off, libraries are found through pkg-config (the staging tree) and the
# target's sysconfig flags. Required (build fails without): zlib, JPEG,
# FreeType; also TIFF, WebP, LCMS2, OpenJPEG and libraqm (complex text
# layout) from the ported libraries. Not built: imagequant, AVIF, XCB.
VERSION=13.0.0
TARBALL=https://files.pythonhosted.org/packages/ce/a4/9cc30690a394a8305e8be4c9036aa16d0dfddadd09079860b582f562496d/pillow-13.0.0.tar.gz
SHA256=cdcd1ba258178f7356d74170c9a81ba4eab6a5d55748bc402a5a2a6fe6be3510
DEPS="python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77" "pybind11"; }
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  LDCXXSHARED="q++ -V${QCC_TARGET}_cxx -shared" \
  qnx_py_wheel -Cplatform-guessing=disable -Czlib=enable -Cjpeg=enable -Cfreetype=enable \
    -Ctiff=enable -Cwebp=enable -Clcms=enable -Cjpeg2000=enable -Craqm=enable \
    -Cimagequant=disable -Cavif=disable -Cxcb=disable
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
