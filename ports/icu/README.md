# icu 76.1

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

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install icu
oss-get install icu-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH icu
ls repo/8.0/*/ | grep '^icu-'
```

This writes `icu-<ver>-r0.tar.gz` (run time) and `icu-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install icu
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/icu-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/icu-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_icu`](../../tests/src/) links icu the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_icu root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_icu'
```

```
== icu ==
  ok   ...
PASS icu
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
