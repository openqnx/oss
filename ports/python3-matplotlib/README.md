# python3-matplotlib 3.11.2

> Comprehensive library for static and animated visualizations in Python

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/e7/c8/9aa712a0afb882649424dd8de8ad9aa6235e796e84c6052e8f6dc1598d0d/matplotlib-3.11.2.tar.gz |
| SHA-256 | `cec596316640f2b394b8f0daa0ea61a8eae82d017b620b9f202befb972a59ea4` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`numpy`](../numpy/) [`freetype`](../freetype/) [`libraqm`](../libraqm/) [`qhull`](../qhull/) [`python3-contourpy`](../python3-contourpy/) [`python3-cycler`](../python3-cycler/) [`python3-fonttools`](../python3-fonttools/) [`python3-kiwisolver`](../python3-kiwisolver/) [`python3-packaging`](../python3-packaging/) [`python3-pillow`](../python3-pillow/) [`python3-pyparsing`](../python3-pyparsing/) [`python3-dateutil`](../python3-dateutil/) |
| Run-time dependencies | [`freetype`](../freetype/) [`libraqm`](../libraqm/) [`python3`](../python3/) [`qhull`](../qhull/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_matplotlib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-matplotlib`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-matplotlib`](https://openqnx.github.io/oss/package/x86_64/python3-matplotlib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-matplotlib.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-matplotlib` (dependencies come along); one version: `apk add python3-matplotlib=<version>` | `apk del python3-matplotlib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-matplotlib` (dependencies first) | `oss-get remove python3-matplotlib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-matplotlib && /usr/local/bin/qnx-port-tests/test_python3_matplotlib    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-matplotlib qnx-test-python3-matplotlib
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-matplotlib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-matplotlib.list
rm var/lib/manual/python3-matplotlib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-matplotlib` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil
```

<details><summary>Full build chain (43)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libraqm`](../libraqm/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openjpeg`](../openjpeg/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`python3-contourpy`](../python3-contourpy/) [`python3-cycler`](../python3-cycler/) [`python3-dateutil`](../python3-dateutil/) [`python3-fonttools`](../python3-fonttools/) [`python3-kiwisolver`](../python3-kiwisolver/) [`python3-packaging`](../python3-packaging/) [`python3-pillow`](../python3-pillow/) [`python3-pyparsing`](../python3-pyparsing/) [`python3-six`](../python3-six/) [`qhull`](../qhull/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/e7/c8/9aa712a0afb882649424dd8de8ad9aa6235e796e84c6052e8f6dc1598d0d/matplotlib-3.11.2.tar.gz
mkdir -p dist src/python3-matplotlib
curl -fL -o dist/$(basename $URL) $URL
echo "cec596316640f2b394b8f0daa0ea61a8eae82d017b620b9f202befb972a59ea4  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-matplotlib --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-matplotlib; B=build/$QNX_ARCH/python3-matplotlib; mkdir -p $B
```

```bash
do_configure() {
  qnx_py_venv "meson-python>=0.18" "meson>=1.2" "pybind11>=2.13.2,!=2.13.3" "setuptools_scm>=7,<10" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile && echo "pybind11-config = '$B/venv/bin/pybind11-config'" >> "$B/qnx-cross.ini" || return 1
  # FreeType's .pc requires zlib's, which zlib installs in share/pkgconfig:
  # that one file is added (all of share/pkgconfig would also bring a
  # pybind11.pc, which Meson would prefer to the venv's pybind11).
  mkdir -p "$B/pc" && ln -sfn "$STAGE$PREFIX/share/pkgconfig/zlib.pc" "$B/pc/zlib.pc" || return 1
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$B/pc']#" "$B/qnx-cross.ini"
}
```

```bash
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dsystem-freetype=true" -Csetup-args="-Dsystem-libraqm=true" \
    -Csetup-args="-Dsystem-qhull=true" -Csetup-args="-Dmacosx=false" \
    -Csetup-args="--wrap-mode=nodownload" -Csetup-args="-Db_lto=false"
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Matplotlib: 2D plotting for Python. Its C++ extensions (Agg rasterizer, FreeType/libraqm text, path and image operations, Qhull triangulation; pybind11) are cross compiled like numpy's (meson-python with a cross file, scripts/lib/pywheel.sh), against the ported FreeType, libraqm and Qhull (system-*=true): Matplotlib otherwise downloads and builds its own copies of FreeType, HarfBuzz, SheenBidi, libraqm and Qhull during the build (--wrap-mode=nodownload makes a missing one an error instead). b_lto=false: pyproject.toml turns LTO on, and the cross binutils' ar has no LTO plugin, so the bundled Agg static library got no symbol index and _path/_backend_agg linked with its symbols unresolved ("Unresolved symbols" at import on the target). Backends that work on QNX: Agg (PNG), PDF, PS, SVG -- the default is Agg (no GUI toolkit binding is ported).
- pybind11 (headers only) comes from the build venv; Meson finds it through its config tool, which a cross build looks up in the cross file.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-matplotlib`.

### 5. What it installs

601 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-matplotlib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-matplotlib
ls dists/8.0/*/ | grep '^python3-matplotlib-'
```

This writes `python3-matplotlib-<ver>-r0.tar.gz` (run time) and `python3-matplotlib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-matplotlib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-matplotlib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-matplotlib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_matplotlib`](../../tests/src/) links python3-matplotlib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-matplotlib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-matplotlib
/usr/local/bin/qnx-port-tests/test_python3_matplotlib
```

```
== python3-matplotlib ==
  ok   ...
PASS python3-matplotlib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_matplotlib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_matplotlib`](../../tests/src/) -- a program linked against python3-matplotlib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-contourpy 1.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-contourpy-1.4.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-cycler 0.12.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-cycler-0.12.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-fonttools 4.68.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-fonttools-4.68.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-kiwisolver 1.5.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-kiwisolver-1.5.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-matplotlib 3.11.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-matplotlib-3.11.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-contourpy 1.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-contourpy-1.4.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-cycler 0.12.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-cycler-0.12.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-fonttools 4.68.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-fonttools-4.68.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-kiwisolver 1.5.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-kiwisolver-1.5.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-matplotlib 3.11.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-matplotlib-3.11.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.11.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-matplotlib=<version>`), sources in `ported/python3-matplotlib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-matplotlib`; port one with `scripts/port-update python3-matplotlib`.

<details><summary>recipe.sh</summary>

```bash
# Matplotlib: 2D plotting for Python. Its C++ extensions (Agg rasterizer,
# FreeType/libraqm text, path and image operations, Qhull triangulation;
# pybind11) are cross compiled like numpy's (meson-python with a cross file,
# scripts/lib/pywheel.sh), against the ported FreeType, libraqm and Qhull
# (system-*=true): Matplotlib otherwise downloads and builds its own copies
# of FreeType, HarfBuzz, SheenBidi, libraqm and Qhull during the build
# (--wrap-mode=nodownload makes a missing one an error instead).
# b_lto=false: pyproject.toml turns LTO on, and the cross binutils' ar has no
# LTO plugin, so the bundled Agg static library got no symbol index and
# _path/_backend_agg linked with its symbols unresolved ("Unresolved
# symbols" at import on the target).
# Backends that work on QNX: Agg (PNG), PDF, PS, SVG -- the default is Agg
# (no GUI toolkit binding is ported).
VERSION=3.11.2
TARBALL=https://files.pythonhosted.org/packages/e7/c8/9aa712a0afb882649424dd8de8ad9aa6235e796e84c6052e8f6dc1598d0d/matplotlib-3.11.2.tar.gz
SHA256=cec596316640f2b394b8f0daa0ea61a8eae82d017b620b9f202befb972a59ea4
DEPS="numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil"
RUNTIME_DEPS="numpy python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# pybind11 (headers only) comes from the build venv; Meson finds it through
# its config tool, which a cross build looks up in the cross file.
do_configure() {
  qnx_py_venv "meson-python>=0.18" "meson>=1.2" "pybind11>=2.13.2,!=2.13.3" "setuptools_scm>=7,<10" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile && echo "pybind11-config = '$B/venv/bin/pybind11-config'" >> "$B/qnx-cross.ini" || return 1
  # FreeType's .pc requires zlib's, which zlib installs in share/pkgconfig:
  # that one file is added (all of share/pkgconfig would also bring a
  # pybind11.pc, which Meson would prefer to the venv's pybind11).
  mkdir -p "$B/pc" && ln -sfn "$STAGE$PREFIX/share/pkgconfig/zlib.pc" "$B/pc/zlib.pc" || return 1
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$B/pc']#" "$B/qnx-cross.ini"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dsystem-freetype=true" -Csetup-args="-Dsystem-libraqm=true" \
    -Csetup-args="-Dsystem-qhull=true" -Csetup-args="-Dmacosx=false" \
    -Csetup-args="--wrap-mode=nodownload" -Csetup-args="-Db_lto=false"
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
