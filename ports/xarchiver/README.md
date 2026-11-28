# xarchiver 0.5.4.27

> GTK 3 archive manager: a front end for tar, 7-Zip, zip, xz and other archivers

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/ib/xarchiver.git (tag `0.5.4.27`) |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`intltool`](../intltool/) [`gettext`](../gettext/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xarchiver`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xarchiver` and `xarchiver-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xarchiver` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (59 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 intltool gettext
```

<details><summary>Full build chain (59)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`intltool`](../intltool/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.5.4.27 --recurse-submodules --shallow-submodules \
    https://github.com/ib/xarchiver.git src/xarchiver
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xarchiver < ports/xarchiver/patches/0001-qnx-netbyte-endian.patch
patch -p1 -d src/xarchiver < ports/xarchiver/patches/0002-info-without-archive-argument.patch
patch -p1 -d src/xarchiver < ports/xarchiver/patches/0003-qnx-pipe-eof-is-not-an-error.patch
```

**[`0001-qnx-netbyte-endian.patch`](patches/0001-qnx-netbyte-endian.patch)** — xarchiver: byte-order macros from <net/netbyte.h> on QNX

> exe.c, gzip_et_al.c and window.c take le16toh(), le32toh() and htole32() from
> <endian.h>, with branches for macOS and FreeBSD. QNX has neither <endian.h>
> nor <sys/endian.h>; the same macros are in <net/netbyte.h>.
>
> ---

Touches: `src/exe.c`, `src/gzip_et_al.c`, `src/window.c` · Upstream status: Pending

**[`0002-info-without-archive-argument.patch`](patches/0002-info-without-archive-argument.patch)** — xarchiver: --info without an archive argument crashed.

> main() calls xa_init_structure_from_cmd_line(argv[1]) for every
> command-line mode, also for "xarchiver --info", which takes no archive:
> argv[1] is NULL there and xa_detect_archive_type(NULL) dereferences it.
> On QNX that is a SIGSEGV (exit 139) before the list of archivers is
> printed. Return NULL for a missing file name, as for an unknown type.

Touches: `src/main.c` · Upstream status: Pending

**[`0003-qnx-pipe-eof-is-not-an-error.patch`](patches/0003-qnx-pipe-eof-is-not-an-error.patch)** — xarchiver: the end of a child's output is not an error on QNX.

> xa_process_stdout() and xa_process_stderr() read the archiver's output
> through GIOChannel watches and, when a watch ends, report "failed" unless
> the last condition was exactly G_IO_HUP. Linux's poll() gives POLLHUP when
> the writer has closed the pipe; QNX's never does for a pipe: it reports
> POLLIN, and read() then returns 0 (EOF). After the EOF the condition left
> was 0, not G_IO_HUP, so every command counted as failed: --extract-to
> stopped with an empty "An error occurred while accessing the archive"
> dialog although gzip/tar had exited 0. Only G_IO_ERR and G_IO_NVAL now mean
> failure (the exit status still decides the rest).

Touches: `src/archive.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xarchiver; B=build/$QNX_ARCH/xarchiver; mkdir -p $B
```

```bash
do_configure() {
  _xa_hostbin || return 1
  # The pre-built HTML help is in the source tree; a git checkout gives it the
  # same time as its DocBook source, and make then rebuilds it with rules that
  # only work inside the source directory ("cp: cannot stat 'xarchiver.css'").
  # Mark the shipped pages as up to date so they are installed as they are.
  touch "$S"/doc/html/* 2>/dev/null
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
```

```bash
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Xarchiver: a GTK 3 archive manager -- opens, creates, extracts and tests archives by running the command-line archivers it finds in PATH (tar, 7zz, zip, xz, bzip2, zstd, lz4, lzip, gzip, cpio, ar, ...), with a file list, drag and drop, and non-interactive modes the file manager uses ("xarchiver --extract-to=DIR ARCHIVE", --ensure-directory, --compress). "xarchiver --info" lists the archive types the installed programs give.
- intltool: a build tool here (the translated .desktop file and the message catalogues are made with intltool-merge/-update).
- The archivers it drives: what is installed decides the formats offered. GNU tar for tar and every tar.* (its options are GNU tar's), 7zip for 7z, zip (reading), rar (reading), iso and more, zip for writing zip archives.
- The repository ships its configure script. intltool's programs are Perl scripts, and the staged ones are the target's (they start with #!/usr/local/bin/perl, which the build machine does not have). configure calls "intltool-update --version" by name before it looks for the programs, so setting INTLTOOL_UPDATE is not enough: three wrappers in the build directory run the staged scripts with the build machine's perl, and that directory is first on PATH for configure, make and the install.
- The cross environment is the one scripts/qnxbuild gives every Autotools component (compiler, staging before the SDP, the libraries QNX splits out of libc); a hook replaces the whole step, so it is spelled out here.
- The wrappers were written at configure time, with the staging tree's path: during the install $STAGE names this component's package root.
- GTK 3 (not --enable-gtk2). The HTML manual (Help > Contents) is the pre-built one from the repository; the icons and the Thunar archive plugin script (libexec/thunar-archive-plugin/xarchiver.tap) are installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xarchiver`.

### 5. What it installs

100 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xarchiver
```

The complete list is `stage/<arch>/.manifest/xarchiver` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xarchiver
ls dists/8.0/*/ | grep '^xarchiver-'
```

This writes `xarchiver-<ver>-r0.tar.gz` (run time) and `xarchiver-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xarchiver
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xarchiver-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xarchiver-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xarchiver`](../../tests/src/) links xarchiver the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xarchiver`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xarchiver
/usr/local/bin/qnx-port-tests/test_xarchiver
```

```
== xarchiver ==
  ok   ...
PASS xarchiver
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xarchiver` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xarchiver`](../../tests/src/) -- a program linked against xarchiver the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xarchiver`.

## Versions

This recipe builds **0.5.4.27**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xarchiver=<version>`), sources in `ported/xarchiver/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xarchiver`; port one with `scripts/port-update xarchiver`.

<details><summary>recipe.sh</summary>

```bash
# Xarchiver: a GTK 3 archive manager -- opens, creates, extracts and tests
# archives by running the command-line archivers it finds in PATH (tar, 7zz,
# zip, xz, bzip2, zstd, lz4, lzip, gzip, cpio, ar, ...), with a file list,
# drag and drop, and non-interactive modes the file manager uses
# ("xarchiver --extract-to=DIR ARCHIVE", --ensure-directory, --compress).
# "xarchiver --info" lists the archive types the installed programs give.
VERSION=0.5.4.27; GIT=https://github.com/ib/xarchiver.git; REF=0.5.4.27
# intltool: a build tool here (the translated .desktop file and the message
# catalogues are made with intltool-merge/-update).
DEPS="glib gtk3 intltool gettext"
# The archivers it drives: what is installed decides the formats offered.
# GNU tar for tar and every tar.* (its options are GNU tar's), 7zip for 7z, zip
# (reading), rar (reading), iso and more, zip for writing zip archives.
RUNTIME_DEPS="tar 7zip zip xz bzip2 zstd lz4 lzip hicolor-icon-theme adwaita-icon-theme"
BUILDSYS=autotools
# The repository ships its configure script. intltool's programs are Perl
# scripts, and the staged ones are the target's (they start with
# #!/usr/local/bin/perl, which the build machine does not have). configure
# calls "intltool-update --version" by name before it looks for the programs,
# so setting INTLTOOL_UPDATE is not enough: three wrappers in the build
# directory run the staged scripts with the build machine's perl, and that
# directory is first on PATH for configure, make and the install.
_xa_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
# The cross environment is the one scripts/qnxbuild gives every Autotools
# component (compiler, staging before the SDP, the libraries QNX splits out of
# libc); a hook replaces the whole step, so it is spelled out here.
do_configure() {
  _xa_hostbin || return 1
  # The pre-built HTML help is in the source tree; a git checkout gives it the
  # same time as its DocBook source, and make then rebuilds it with rules that
  # only work inside the source directory ("cp: cannot stat 'xarchiver.css'").
  # Mark the shipped pages as up to date so they are installed as they are.
  touch "$S"/doc/html/* 2>/dev/null
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
# The wrappers were written at configure time, with the staging tree's path:
# during the install $STAGE names this component's package root.
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
# GTK 3 (not --enable-gtk2). The HTML manual (Help > Contents) is the pre-built
# one from the repository; the icons and the Thunar archive plugin script
# (libexec/thunar-archive-plugin/xarchiver.tap) are installed.
CONF_ARGS_EXTRA=( --disable-gtk2 --enable-doc --enable-icons --enable-plugin )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
