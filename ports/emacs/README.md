# emacs 30.2

> GNU Emacs extensible text editor (terminal build)

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
| On-target test | [`test_emacs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `emacs` (programs, shared libraries, run-time data) and `emacs-dev` (headers, pkg-config/CMake files, static libraries; needs `emacs`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`emacs`](https://openqnx.github.io/oss/package/x86_64/emacs.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/emacs.html)) · [`emacs-dev`](https://openqnx.github.io/oss/package/x86_64/emacs-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/emacs-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add emacs` (dependencies come along); one version: `apk add emacs=<version>` | `apk del emacs` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install emacs` (dependencies first) | `oss-get remove emacs` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`emacs-dev` the same way, by its own name (`apk add emacs-dev`, `oss-get install emacs-dev`, by hand with its own file and list `emacs-dev.list`); remove it before `emacs`, which it needs.

```sh
# on the QNX target, as root
apk add emacs emacs-dev
apk add qnx-test-emacs && /usr/local/bin/qnx-port-tests/test_emacs    # its on-target test, from tests-8.0-<arch>
apk del emacs emacs-dev qnx-test-emacs
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/emacs.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/emacs.list
rm var/lib/manual/emacs.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (25 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses gnutls libxml2 zlib dbus lcms2 sqlite tree-sitter
```

<details><summary>Full build chain (25)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`lcms2`](../lcms2/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`tree-sitter`](../tree-sitter/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa emacs
ls dists/8.0/*/ | grep '^emacs-'
```

This writes `emacs-<ver>-r0.tar.gz` (run time) and `emacs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add emacs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/emacs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/emacs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_emacs`](../../tests/src/) links emacs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-emacs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-emacs
/usr/local/bin/qnx-port-tests/test_emacs
```

```
== emacs ==
  ok   ...
PASS emacs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_emacs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_emacs`](../../tests/src/) -- a program linked against emacs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | emacs 30.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/emacs-30.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | emacs 30.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/emacs-30.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | emacs 30.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/emacs-30.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | emacs 30.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/emacs-30.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **30.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add emacs=<version>`), sources in `ported/emacs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates emacs`; port one with `scripts/port-update emacs`.

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
