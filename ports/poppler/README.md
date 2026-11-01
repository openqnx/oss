# poppler 25.12.0

> PDF rendering library and command line tools

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/poppler/poppler.git (tag `poppler-25.12.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`cairo`](../cairo/) [`glib`](../glib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`openjpeg`](../openjpeg/) [`lcms2`](../lcms2/) [`zlib`](../zlib/) |
| Run-time dependencies | [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`lcms2`](../lcms2/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`openjpeg`](../openjpeg/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_poppler`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `poppler` (programs, shared libraries, run-time data) and `poppler-dev` (headers, pkg-config/CMake files, static libraries; needs `poppler`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`poppler`](https://openqnx.github.io/oss/package/x86_64/poppler.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/poppler.html)) · [`poppler-dev`](https://openqnx.github.io/oss/package/x86_64/poppler-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/poppler-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add poppler` (dependencies come along); one version: `apk add poppler=<version>` | `apk del poppler` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install poppler` (dependencies first) | `oss-get remove poppler` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`poppler-dev` the same way, by its own name (`apk add poppler-dev`, `oss-get install poppler-dev`, by hand with its own file and list `poppler-dev.list`); remove it before `poppler`, which it needs.

```sh
# on the QNX target, as root
apk add poppler poppler-dev
apk add qnx-test-poppler && /usr/local/bin/qnx-port-tests/test_poppler    # its on-target test, from tests-8.0-<arch>-m-z
apk del poppler poppler-dev qnx-test-poppler
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/poppler.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/poppler.list
rm var/lib/manual/poppler.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild poppler` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (35 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib
```

<details><summary>Full build chain (35)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`lcms2`](../lcms2/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openjpeg`](../openjpeg/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b poppler-25.12.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/poppler/poppler.git src/poppler
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/poppler -B build/$QNX_ARCH/poppler -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DENABLE_NSS3=OFF \
      -DENABLE_GPGME=OFF \
      -DENABLE_QT5=OFF \
      -DENABLE_QT6=OFF \
      -DENABLE_LIBCURL=OFF \
      -DBUILD_GTK_TESTS=OFF \
      -DBUILD_QT5_TESTS=OFF \
      -DBUILD_QT6_TESTS=OFF \
      -DBUILD_CPP_TESTS=OFF \
      -DBUILD_MANUAL_TESTS=OFF \
      -DENABLE_GLIB=ON \
      -DENABLE_UTILS=ON \
      -DENABLE_CPP=ON \
      -DENABLE_UNSTABLE_API_ABI_HEADERS=OFF \
      -DENABLE_GOBJECT_INTROSPECTION=OFF \
      -DENABLE_LCMS=ON \
      -DENABLE_DCTDECODER=libjpeg \
      -DENABLE_LIBOPENJPEG=openjpeg2 \
      -DENABLE_BOOST=OFF \
      -DPython3_EXECUTABLE=/usr/bin/python3 \
      -DGLIB2_MKENUMS_PYTHON=/usr/bin/python3
cmake --build build/$QNX_ARCH/poppler -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/poppler
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- poppler: PDF rendering library (xpdf-based) with its command-line tools (pdftotext, pdfinfo, pdftoppm, pdftocairo, ...); C++, GLib and cairo front ends. No NSS (signature verification: NSS is not ported), no Qt front ends, no GPGME.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH poppler`.

### 5. What it installs

65 files under `/usr/local`: 9 shared-library entries, 12 programs, 29 headers.

```
/usr/local/lib/libpoppler-cpp.so
/usr/local/lib/libpoppler-cpp.so.2
/usr/local/lib/libpoppler-cpp.so.2.2.0
/usr/local/lib/libpoppler-glib.so
/usr/local/lib/libpoppler-glib.so.8
/usr/local/lib/libpoppler-glib.so.8.31.0
/usr/local/lib/libpoppler.so
/usr/local/lib/libpoppler.so.155
/usr/local/lib/libpoppler.so.155.0.0
/usr/local/bin/pdfattach
/usr/local/bin/pdfdetach
/usr/local/bin/pdffonts
/usr/local/bin/pdfimages
/usr/local/bin/pdfinfo
/usr/local/bin/pdfseparate
/usr/local/bin/pdftocairo
...
```

The complete list is `stage/<arch>/.manifest/poppler` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa poppler
ls dists/8.0/*/ | grep '^poppler-'
```

This writes `poppler-<ver>-r0.tar.gz` (run time) and `poppler-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add poppler
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/poppler-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/poppler-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_poppler`](../../tests/src/) links poppler the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-poppler`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-poppler
/usr/local/bin/qnx-port-tests/test_poppler
```

```
== poppler ==
  ok   ...
PASS poppler
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_poppler` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_poppler`](../../tests/src/) -- a program linked against poppler the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - poppler_document_new_from_file
   - one page
   - page size 298 x 420 pt (A6)
   - text read back
   - find_text QNX
   - found where it was drawn

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | poppler 25.12.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/poppler-25.12.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | poppler 25.12.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/poppler-25.12.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **25.12.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add poppler=<version>`), sources in `ported/poppler/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates poppler`; port one with `scripts/port-update poppler`.

<details><summary>recipe.sh</summary>

```bash
# poppler: PDF rendering library (xpdf-based) with its command-line tools (pdftotext,
# pdfinfo, pdftoppm, pdftocairo, ...); C++, GLib and cairo front ends. No NSS
# (signature verification: NSS is not ported), no Qt front ends, no GPGME.
VERSION=25.12.0; GIT=https://gitlab.freedesktop.org/poppler/poppler.git; REF=poppler-25.12.0
DEPS="freetype fontconfig cairo glib libjpeg-turbo libpng libtiff openjpeg lcms2 zlib"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DENABLE_NSS3=OFF -DENABLE_GPGME=OFF -DENABLE_QT5=OFF -DENABLE_QT6=OFF -DENABLE_LIBCURL=OFF -DBUILD_GTK_TESTS=OFF -DBUILD_QT5_TESTS=OFF -DBUILD_QT6_TESTS=OFF -DBUILD_CPP_TESTS=OFF -DBUILD_MANUAL_TESTS=OFF -DENABLE_GLIB=ON -DENABLE_UTILS=ON -DENABLE_CPP=ON -DENABLE_UNSTABLE_API_ABI_HEADERS=OFF -DENABLE_GOBJECT_INTROSPECTION=OFF -DENABLE_LCMS=ON -DENABLE_DCTDECODER=libjpeg -DENABLE_LIBOPENJPEG=openjpeg2 -DENABLE_BOOST=OFF
                   -DPython3_EXECUTABLE=/usr/bin/python3 -DGLIB2_MKENUMS_PYTHON=/usr/bin/python3 )
# glib-mkenums runs on the build machine: its Python, not the QNX one CMake
# finds in the sysroot.
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
