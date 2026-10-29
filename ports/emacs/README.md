# emacs 30.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/emacs/emacs-30.2.tar.xz |
| SHA-256 | `b3f36f18a6dd2715713370166257de2fae01f9d38cfe878ced9b1e6ded5befd9` |
| Build system | custom (see recipe) |
| QNX patches | 4 |
| Build dependencies | [`ncurses`](../ncurses/) [`gnutls`](../gnutls/) [`libxml2`](../libxml2/) [`zlib`](../zlib/) [`dbus`](../dbus/) [`lcms2`](../lcms2/) [`sqlite`](../sqlite/) [`tree-sitter`](../tree-sitter/) |
| Run-time dependencies | [`dbus`](../dbus/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`lcms2`](../lcms2/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`sqlite`](../sqlite/) [`tree-sitter`](../tree-sitter/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install emacs
oss-get install emacs-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild emacs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter
```

<details><summary>Full build chain (19)</summary>

[`brotli`](../brotli/) [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`lcms2`](../lcms2/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`sqlite`](../sqlite/) [`tree-sitter`](../tree-sitter/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/emacs/emacs-30.2.tar.xz
mkdir -p dist src/emacs
curl -fL -o dist/$(basename $URL) $URL
echo "b3f36f18a6dd2715713370166257de2fae01f9d38cfe878ced9b1e6ded5befd9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/emacs --strip-components=1
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/emacs < ports/emacs/patches/0001-aports-fix-qnx-configure.patch
patch -p1 -d src/emacs < ports/emacs/patches/0002-aports-fix-qnx.patch
patch -p1 -d src/emacs < ports/emacs/patches/0003-aports-fix-qnx-pty-shell.patch
patch -p1 -d src/emacs < ports/emacs/patches/0004-boot-time-sys-time-h.patch
```

**[`0001-aports-fix-qnx-configure.patch`](patches/0001-aports-fix-qnx-configure.patch)** — QNX build fix from QNX's aports (extra/emacs/fix-qnx-configure.patch).

> Taken from qnx-ports/aports extra/emacs, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `emacs-30.2/configure.ac` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx.patch`](patches/0002-aports-fix-qnx.patch)** — QNX build fix from QNX's aports (extra/emacs/fix-qnx.patch).

> Taken from qnx-ports/aports extra/emacs, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `emacs-30.2/lib-src/emacsclient.c` · Upstream status: QNX-specific

**[`0003-aports-fix-qnx-pty-shell.patch`](patches/0003-aports-fix-qnx-pty-shell.patch)** — QNX build fix from QNX's aports (extra/emacs/fix-qnx-pty-shell.patch).

> Taken from qnx-ports/aports extra/emacs, the APKBUILD the QNX Everywhere image's
> package is built from.

Touches: `emacs-30.2/src/callproc.c` · Upstream status: QNX-specific

**[`0004-boot-time-sys-time-h.patch`](patches/0004-boot-time-sys-time-h.patch)** — Include <sys/time.h> for the sysctl boot-time fallback.

> gnulib's get_bsd_boot_time_final_fallback() reads KERN_BOOTTIME into a
> struct timeval, which boot-time.c relies on <sys/sysctl.h> to declare. QNX
> has KERN_BOOTTIME but its <sys/sysctl.h> does not bring in <sys/time.h>:
> storage size of 'result' isn't known.

Touches: `lib/boot-time.c` · Upstream status: Pending (gnulib lib/boot-time.c)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/emacs; B=build/$QNX_ARCH/emacs; mkdir -p $B
```

```bash
do_configure() {
  local H="$BUILD_DIR/host/emacs-$VERSION"
  if [ ! -x "$H/src/emacs" ]; then
    mkdir -p "$H" && ( cd "$H" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
      "$S/configure" --without-x --without-sound --with-native-compilation=no --without-gnutls --without-xml2 \
      --without-all --with-modules=no && make -j"$(nproc)" ) || return 1
  fi
  cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  "$S/configure" --host="$QNX_TRIPLE" --build=x86_64-pc-linux-gnu --prefix="$PREFIX" --without-x --without-sound \
    --without-file-notification --with-native-compilation=no --with-dumping=none --with-pdumper=no \
    --with-gnutls --with-xml2 --with-modules=no
}
```

```bash
do_build()   { _emake actual-all; }
```

```bash
do_install() { _emake DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU Emacs 30.2, terminal build (no X/GTK), with the QNX changes of QNX's aports (patches/). Version as in the QNX Everywhere 8.0.4r1 image. Emacs byte-compiles its Lisp and dumps its own image while building, which needs a running Emacs: a native build for the build host is made first and used for that (EMACS / BOOTSTRAPEMACS), then the target build reuses its compiled Lisp.
- Also linked at run time (found by scripts/gen-dependency-index): dbus lcms2 sqlite tree-sitter
- Emacs runs what it builds: lib-src's make-docfile (the DOC file, globals.h) and Emacs itself (byte-compiling, generated Lisp). Those come from the build host's Emacs made above -- libsrc= for the lib-src tools, EMACS= for Emacs; with --with-dumping=none there is no dump to make with the QNX binary.
- timerfd_create() is in QNX's libtimerfd, not libc. configure finds it (via other libraries) but temacs is linked with LIBS_SYSTEM, which the aports configure patch sets to -lsocket alone: "DSO missing from command line". "make all" is actual-all followed by a sanity check that runs the new emacs -- a QNX binary. Build actual-all; the check belongs on the target. install depends on actual-all too, so it gets the same host tools. EMACS means two things: the program lisp/ runs, and at the top level the name to install the binary as. Pass the top level's own value ("emacs") and let PATH resolve it to the host build -- an absolute path there became the name of the installed symlink.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH emacs`.

### 5. What it installs

4369 files under `/usr/local`, 6 programs, 1 headers.

```
/usr/local/bin/ctags
/usr/local/bin/ebrowse
/usr/local/bin/emacs
/usr/local/bin/emacs-30.2
/usr/local/bin/emacsclient
/usr/local/bin/etags
```

The complete list is `stage/<arch>/.manifest/emacs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH emacs
ls repo/8.0/*/ | grep '^emacs-'
```

This writes `emacs-<ver>-r0.tar.gz` (run time) and `emacs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install emacs
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/emacs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/emacs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# GNU Emacs 30.2, terminal build (no X/GTK), with the QNX changes of QNX's
# aports (patches/). Version as in the QNX Everywhere 8.0.4r1 image.
#
# Emacs byte-compiles its Lisp and dumps its own image while building, which
# needs a running Emacs: a native build for the build host is made first and
# used for that (EMACS / BOOTSTRAPEMACS), then the target build reuses its
# compiled Lisp.
VERSION=30.2
TARBALL=https://ftp.gnu.org/gnu/emacs/emacs-30.2.tar.xz
SHA256=b3f36f18a6dd2715713370166257de2fae01f9d38cfe878ced9b1e6ded5befd9
# Also linked at run time (found by scripts/gen-dependency-index): dbus lcms2 sqlite tree-sitter
DEPS="ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter"
BUILDSYS=custom
do_configure() {
  local H="$BUILD_DIR/host/emacs-$VERSION"
  if [ ! -x "$H/src/emacs" ]; then
    mkdir -p "$H" && ( cd "$H" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
      "$S/configure" --without-x --without-sound --with-native-compilation=no --without-gnutls --without-xml2 \
      --without-all --with-modules=no && make -j"$(nproc)" ) || return 1
  fi
  cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  "$S/configure" --host="$QNX_TRIPLE" --build=x86_64-pc-linux-gnu --prefix="$PREFIX" --without-x --without-sound \
    --without-file-notification --with-native-compilation=no --with-dumping=none --with-pdumper=no \
    --with-gnutls --with-xml2 --with-modules=no
}
# Emacs runs what it builds: lib-src's make-docfile (the DOC file, globals.h)
# and Emacs itself (byte-compiling, generated Lisp). Those come from the build
# host's Emacs made above -- libsrc= for the lib-src tools, EMACS= for Emacs;
# with --with-dumping=none there is no dump to make with the QNX binary.
_hostemacs() { echo "$BUILD_DIR/host/emacs-$VERSION"; }
# timerfd_create() is in QNX's libtimerfd, not libc. configure finds it (via
# other libraries) but temacs is linked with LIBS_SYSTEM, which the aports
# configure patch sets to -lsocket alone: "DSO missing from command line".
# "make all" is actual-all followed by a sanity check that runs the new
# emacs -- a QNX binary. Build actual-all; the check belongs on the target.
# install depends on actual-all too, so it gets the same host tools. EMACS
# means two things: the program lisp/ runs, and at the top level the name to
# install the binary as. Pass the top level's own value ("emacs") and let PATH
# resolve it to the host build -- an absolute path there became the name of
# the installed symlink.
_emake() { local H; H=$(_hostemacs)
  mkdir -p "$B/hostbin" && ln -sf "$H/src/emacs" "$B/hostbin/emacs" && \
  PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)" libsrc="$H/lib-src" LIBS_SYSTEM="-lsocket -ltimerfd" \
    EMACS=emacs BOOTSTRAPEMACS="$H/src/bootstrap-emacs" "$@"; }
do_build()   { _emake actual-all; }
do_install() { _emake DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
