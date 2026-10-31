# icu 76.1

> International Components for Unicode: Unicode and locale support libraries

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-src.tgz |
| SHA-256 | `dfacb46bfe4747410472ce3e1144bf28a102feeaa4e3875bac9b4c6cf30f4f3e` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_icu`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `icu` (programs, shared libraries, run-time data) and `icu-dev` (headers, pkg-config/CMake files, static libraries; needs `icu`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`icu`](https://openqnx.github.io/oss/package/x86_64/icu.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/icu.html)) · [`icu-dev`](https://openqnx.github.io/oss/package/x86_64/icu-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/icu-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add icu` (dependencies come along); one version: `apk add icu=<version>` | `apk del icu` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install icu` (dependencies first) | `oss-get remove icu` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`icu-dev` the same way, by its own name (`apk add icu-dev`, `oss-get install icu-dev`, by hand with its own file and list `icu-dev.list`); remove it before `icu`, which it needs.

```sh
# on the QNX target, as root
apk add icu icu-dev
apk add qnx-test-icu && /usr/local/bin/qnx-port-tests/test_icu    # its on-target test
apk del icu icu-dev qnx-test-icu
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/icu.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/icu.list
rm var/lib/manual/icu.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild icu` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-src.tgz
mkdir -p dist src/icu
curl -fL -o dist/$(basename $URL) $URL
echo "dfacb46bfe4747410472ce3e1144bf28a102feeaa4e3875bac9b4c6cf30f4f3e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/icu --strip-components=1
```

The project lives in the `source/` subdirectory of the repository.

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/icu < ports/icu/patches/0001-qnx-pic-compile-flags.patch
patch -p1 -d src/icu < ports/icu/patches/0002-qnx-default-library-names.patch
```

**[`0001-qnx-pic-compile-flags.patch`](patches/0001-qnx-pic-compile-flags.patch)** — ICU: compile shared-library objects for QNX with -fPIC.

> config/mh-qnx dates from QNX 6 and puts -shared, a link flag, in the
> SHAREDLIB*FLAGS compile flags, leaving the objects non-PIC; the aarch64 link
> then fails ("relocation R_AARCH64_ADR_PREL_PG_HI21 ... recompile with
> -fPIC"). Use -fPIC like the other mh-* fragments.

Touches: `source/config/mh-qnx` · Upstream status: Pending

**[`0002-qnx-default-library-names.patch`](patches/0002-qnx-default-library-names.patch)** — Use ICU's standard library names on QNX.

> mh-qnx names each library's real file after its soname (libicuuc.so.76),
> for QNX's old native package format, by redefining FINAL_SO_TARGET in terms
> of SO_TARGET_VERSION_MAJOR. pkgdata, which builds libicudata, learns the
> file-name suffix from FINAL_SO_TARGET evaluated where SO_TARGET_VERSION_MAJOR
> is not set: it got an empty suffix and produced "libicudataso.76.1" (soname
> libicudataso.76). Installed, libicudata.so.76 was then the 7 KB stub, and
> every ICU user on the target had no data. Take the default rules, as Linux
> does: libicuuc.so.76.1, soname libicuuc.so.76 -- the same soname as before.

Touches: `source/config/mh-qnx` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/icu/source; B=build/$QNX_ARCH/icu; mkdir -p $B
```

```bash
do_configure() {
  local HB="$BUILD_DIR/host/icu"
  if [ ! -f "$HB/config/icucross.mk" ]; then
    rm -rf "$HB"; mkdir -p "$HB"
    ( cd "$HB" && unset CC CXX CPP CFLAGS CXXFLAGS CPPFLAGS LDFLAGS \
      && "$S/configure" --enable-static --disable-shared --disable-tests \
           --disable-samples --disable-extras \
      && make -j"$(nproc)" ) >> "$LOG_DIR/icu-host.log" 2>&1 \
      || { echo "native ICU build failed -- see $LOG_DIR/icu-host.log"; return 1; }
  fi
  cd "$B"
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" \
  CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" \
    --with-cross-build="$HB" \
    --enable-shared --disable-static \
    --disable-tests --disable-samples --disable-extras
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag: only the tarball carries the prebuilt data archive (source/data/in/icudt76l.dat). From git the build quietly produced the 7 KB stub libicudata, and every ICU user on the target had no locale data (Node.js: "Icu error" from Intl).
- ICU's cross build needs a *native* ICU build first: its data and message tooling (pkgdata, genrb, ...) runs on the build machine, and configure wants that tree handed to it via --with-cross-build.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH icu`.

### 5. What it installs

264 files under `/usr/local`: 18 shared-library entries, 17 programs, 203 headers.

```
/usr/local/lib/libicudata.so
/usr/local/lib/libicudata.so.76
/usr/local/lib/libicudata.so.76.1
/usr/local/lib/libicui18n.so
/usr/local/lib/libicui18n.so.76
/usr/local/lib/libicui18n.so.76.1
/usr/local/lib/libicuio.so
/usr/local/lib/libicuio.so.76
/usr/local/lib/libicuio.so.76.1
/usr/local/lib/libicutest.so
/usr/local/lib/libicutest.so.76
/usr/local/lib/libicutest.so.76.1
/usr/local/lib/libicutu.so
/usr/local/lib/libicutu.so.76
/usr/local/lib/libicutu.so.76.1
/usr/local/lib/libicuuc.so
...
```

The complete list is `stage/<arch>/.manifest/icu` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa icu
ls dists/8.0/*/ | grep '^icu-'
```

This writes `icu-<ver>-r0.tar.gz` (run time) and `icu-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add icu
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/icu-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/icu-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_icu`](../../tests/src/) links icu the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-icu`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-icu
/usr/local/bin/qnx-port-tests/test_icu
```

```
== icu ==
  ok   ...
PASS icu
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_icu` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_icu`](../../tests/src/) -- a program linked against icu the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - Collator::createInstance(de_DE)
   - \"Bär\" sorts before \"Baz\" in German
   - BreakIterator::createWordInstance
   - four words found
   - NumberFormat::createInstance(en_US)
   - en_US grouping and decimal separator
   - DateFormat::createDateInstance
   - Transliterator Any-Latin; Latin-ASCII
   - transliteration produced ASCII
   - ucnv_open(ISO-8859-1)
   - converter registry is populated
   - Normalizer2::getNFCInstance
   - NFC composes u + diaeresis to U+00FC

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | icu 76.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/icu-76.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | icu 76.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/icu-76.1-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | icu 76.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/icu-76.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | icu 76.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/icu-76.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **76.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add icu=<version>`), sources in `ported/icu/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates icu`; port one with `scripts/port-update icu`.

<details><summary>recipe.sh</summary>

```bash
VERSION=76.1
# The release tarball, not the git tag: only the tarball carries the prebuilt
# data archive (source/data/in/icudt76l.dat). From git the build quietly
# produced the 7 KB stub libicudata, and every ICU user on the target had no
# locale data (Node.js: "Icu error" from Intl).
TARBALL=https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-src.tgz
SHA256=dfacb46bfe4747410472ce3e1144bf28a102feeaa4e3875bac9b4c6cf30f4f3e
SUBDIR=source
BUILDSYS=custom
# ICU's cross build needs a *native* ICU build first: its data and message
# tooling (pkgdata, genrb, ...) runs on the build machine, and configure wants
# that tree handed to it via --with-cross-build.
do_configure() {
  local HB="$BUILD_DIR/host/icu"
  if [ ! -f "$HB/config/icucross.mk" ]; then
    rm -rf "$HB"; mkdir -p "$HB"
    ( cd "$HB" && unset CC CXX CPP CFLAGS CXXFLAGS CPPFLAGS LDFLAGS \
      && "$S/configure" --enable-static --disable-shared --disable-tests \
           --disable-samples --disable-extras \
      && make -j"$(nproc)" ) >> "$LOG_DIR/icu-host.log" 2>&1 \
      || { echo "native ICU build failed -- see $LOG_DIR/icu-host.log"; return 1; }
  fi
  cd "$B"
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" \
  CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" \
    --with-cross-build="$HB" \
    --enable-shared --disable-static \
    --disable-tests --disable-samples --disable-extras
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
