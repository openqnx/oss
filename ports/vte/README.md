# vte 0.78.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/vte/0.78/vte-0.78.2.tar.xz |
| SHA-256 | `35d7bcde07356846b4a12881c8e016705b70a9004a9082285eee5834ccc49890` |
| Build system | Meson |
| QNX patches | 3 |
| Build dependencies | [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`pcre2`](../pcre2/) [`fribidi`](../fribidi/) [`icu`](../icu/) [`glib`](../glib/) [`gnutls`](../gnutls/) [`lz4`](../lz4/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`icu`](../icu/) [`lz4`](../lz4/) [`pango`](../pango/) [`pcre2`](../pcre2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vte`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add vte
apk add vte-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-vte && /usr/local/bin/qnx-port-tests/test_vte    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install vte`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vte` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (64 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4
```

<details><summary>Full build chain (64)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtasn1`](../libtasn1/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/vte/0.78/vte-0.78.2.tar.xz
mkdir -p dist src/vte
curl -fL -o dist/$(basename $URL) $URL
echo "35d7bcde07356846b4a12881c8e016705b70a9004a9082285eee5834ccc49890  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/vte --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/vte < ports/vte/patches/0001-qnx-no-floating-point-from-chars.patch
patch -p1 -d src/vte < ports/vte/patches/0002-qnx-define-w-exitcode.patch
patch -p1 -d src/vte < ports/vte/patches/0003-qnx-tiocpkt-defined-but-not-implemented.patch
```

**[`0001-qnx-no-floating-point-from-chars.patch`](patches/0001-qnx-no-floating-point-from-chars.patch)** — QNX 8.0's libc++ has no floating-point std::from_chars.

> parse_termprop_floating() calls the double overload of std::from_chars.
> QNX 8.0's libc++ implements from_chars for integral types only, so the
> call resolves into __charconv/from_chars_integral.h and fails to compile:
>
>     no type named 'type' in 'struct std::__2::enable_if<false, int>'
>
> std::strtod performs the conversion, but it accepts strictly more than
> std::chars_format::general does -- a leading '+', hexadecimal floats, and
> "inf"/"nan".  Rejecting those explicitly keeps a termprop parsed exactly
> as the standard overload would parse it, instead of more permissively on
> this one platform.  Everything else, including the ERANGE and isfinite
> checks and the requirement that the whole string be consumed, is the same
> as the original.

Touches: `src/termprops.hh` · Upstream status: QNX-specific

**[`0002-qnx-define-w-exitcode.patch`](patches/0002-qnx-define-w-exitcode.patch)** — Define W_EXITCODE where the platform does not.

> widget.cc composes a wait status with W_EXITCODE(0, SIGKILL).  That macro
> is a glibc/BSD extension, not POSIX, and QNX's <sys/wait.h> does not have
> it:
>
>     widget.cc:927:30: error: 'W_EXITCODE' was not declared in this scope
>
> The encoding itself is the same.  QNX's own <sys/wait.h> defines
> WEXITSTATUS(s) as ((s) >> 8) & 0xff and WTERMSIG(s) as (s) & 0x7f, so the
> usual definition composes a status QNX's accessors read back correctly.
> Guarded with #ifndef so any platform that does define it keeps its own.

Touches: `src/widget.cc` · Upstream status: QNX-specific

**[`0003-qnx-tiocpkt-defined-but-not-implemented.patch`](patches/0003-qnx-tiocpkt-defined-but-not-implemented.patch)** — QNX defines TIOCPKT but devc-pty does not implement it.

> vte_pty_new_sync() fails on QNX with:
>
>     Failed to open PTY: Inappropriate I/O control operation
>
> fd_set_cpkt() enables pty packet mode under #if defined(TIOCPKT), and
> treats failure as fatal.  QNX's <sys/ioctl.h> does define TIOCPKT --
>     #define TIOCPKT  _IOW('t', 112, int)   /* pty: set/clear packet mode */
> -- but its devc-pty resource manager does not implement the request and
> returns ENOTTY.  A macro being defined is not evidence that the kernel
> supports it.
>
> Ignoring the ENOTTY would be worse than the failure.  vte.cc's reader is
> compiled under the same #if and consumes a leading packet-header byte from
> every read(); with packet mode off but that branch compiled in, the first
> byte of real terminal output is eaten and misread as a header.  The two
> sites have to agree, so this turns both off together on QNX and the plain
> read() path already present as the #else is used instead.  What is lost is
> ^S/^Q flow-control notification, not correctness.

Touches: `src/pty.cc`, `src/vte.cc` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/vte src/vte \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgtk3=true \
      -Dgtk4=true \
      -Dgir=false \
      -Dvapi=false \
      -Ddocs=false \
      -D_systemd=false \
      -Dglade=false \
      -Dicu=true \
      -Dgnutls=true
ninja -C build/$QNX_ARCH/vte
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/vte install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): lz4
- The terminal widget behind xfce4-terminal and gnome-terminal.  Both toolkit bindings are built because both GTK3 and GTK4 are staged. -D_systemd=false: no systemd scope/cgroup handling on QNX. -Dvapi / -Dgir false: Vala and introspection both need to run target tools.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vte`.

### 5. What it installs

135 files under `/usr/local`: 5 shared-library entries, 2 programs, 24 headers.

```
/usr/local/lib/libvte-2.91-gtk4.so
/usr/local/lib/libvte-2.91-gtk4.so.0
/usr/local/lib/libvte-2.91.so
/usr/local/lib/libvte-2.91.so.0
/usr/local/lib/libvte-2.91.so.0.7800.2
/usr/local/bin/vte-2.91
/usr/local/bin/vte-2.91-gtk4
```

The complete list is `stage/<arch>/.manifest/vte` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vte
ls dists/8.0/*/ | grep '^vte-'
```

This writes `vte-<ver>-r0.tar.gz` (run time) and `vte-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vte
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vte-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vte-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vte`](../../tests/src/) links vte the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vte`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vte
/usr/local/bin/qnx-port-tests/test_vte
```

```
== vte ==
  ok   ...
PASS vte
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vte` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vte`](../../tests/src/) -- a program linked against vte the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - vte_get_major_version
   - vte_terminal_get_type registered
   - VteTerminal is a GtkWidget
   - vte_pty_get_type registered
   - vte_regex_get_type registered
   - vte_regex_new_for_match compiled a URL pattern
   - vte_regex_jit answered
   - vte_pty_new_sync opened a pty
   - vte_pty_get_fd
   - vte_pty_set_size 24x80

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vte 0.78.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vte 0.78.2-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.78.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vte=<version>`), sources in `ported/vte/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vte`; port one with `scripts/port-update vte`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.78.2
TARBALL=https://download.gnome.org/sources/vte/0.78/vte-0.78.2.tar.xz
SHA256=35d7bcde07356846b4a12881c8e016705b70a9004a9082285eee5834ccc49890
# Also linked at run time (found by scripts/gen-dependency-index): lz4
DEPS="gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4"
BUILDSYS=meson
# The terminal widget behind xfce4-terminal and gnome-terminal.  Both toolkit
# bindings are built because both GTK3 and GTK4 are staged.
# -D_systemd=false: no systemd scope/cgroup handling on QNX.
# -Dvapi / -Dgir false: Vala and introspection both need to run target tools.
MESON_ARGS_EXTRA=( -Dgtk3=true -Dgtk4=true -Dgir=false -Dvapi=false
                   -Ddocs=false -D_systemd=false -Dglade=false
                   -Dicu=true -Dgnutls=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
