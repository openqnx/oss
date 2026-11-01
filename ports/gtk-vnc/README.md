# gtk-vnc 1.3.1

> VNC client library and GTK 3 widget, with the gvncviewer viewer

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gtk-vnc.git (tag `v1.3.1`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gdk-pixbuf`](../gdk-pixbuf/) [`cairo`](../cairo/) [`gnutls`](../gnutls/) [`libgcrypt`](../libgcrypt/) [`zlib`](../zlib/) [`libx11`](../libx11/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gnutls`](../gnutls/) [`gtk3`](../gtk3/) [`libgcrypt`](../libgcrypt/) [`libx11`](../libx11/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gtk_vnc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gtk-vnc` (programs, shared libraries, run-time data) and `gtk-vnc-dev` (headers, pkg-config/CMake files, static libraries; needs `gtk-vnc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gtk-vnc`](https://openqnx.github.io/oss/package/x86_64/gtk-vnc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gtk-vnc.html)) · [`gtk-vnc-dev`](https://openqnx.github.io/oss/package/x86_64/gtk-vnc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gtk-vnc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gtk-vnc` (dependencies come along); one version: `apk add gtk-vnc=<version>` | `apk del gtk-vnc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gtk-vnc` (dependencies first) | `oss-get remove gtk-vnc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gtk-vnc-dev` the same way, by its own name (`apk add gtk-vnc-dev`, `oss-get install gtk-vnc-dev`, by hand with its own file and list `gtk-vnc-dev.list`); remove it before `gtk-vnc`, which it needs.

```sh
# on the QNX target, as root
apk add gtk-vnc gtk-vnc-dev
apk add qnx-test-gtk-vnc && /usr/local/bin/qnx-port-tests/test_gtk_vnc    # its on-target test, from tests-8.0-<arch>
apk del gtk-vnc gtk-vnc-dev qnx-test-gtk-vnc
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gtk-vnc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gtk-vnc.list
rm var/lib/manual/gtk-vnc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtk-vnc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (65 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11
```

<details><summary>Full build chain (65)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.3.1 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gtk-vnc.git src/gtk-vnc
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gtk-vnc; B=build/$QNX_ARCH/gtk-vnc; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  install -Dm755 "$B/examples/gvncviewer" "$STAGE$PREFIX/bin/gvncviewer"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gtk-vnc: VNC client as a GTK 3 widget (libgtk-vnc, libgvnc) and gvncviewer, a VNC viewer window for the QNX desktop (labwc). Tight/JPEG, ZRLE, VeNCrypt TLS -- what TurboVNC servers speak (VirtualGL's vglrun in a TurboVNC session shows OpenGL programs such as RViz here).
- Coroutines on GLib threads: portable, no makecontext/swapcontext needed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtk-vnc`.

### 5. What it installs

82 files under `/usr/local`: 6 shared-library entries, 2 programs, 19 headers.

```
/usr/local/lib/libgtk-vnc-2.0.so
/usr/local/lib/libgtk-vnc-2.0.so.0
/usr/local/lib/libgtk-vnc-2.0.so.0.0.2
/usr/local/lib/libgvnc-1.0.so
/usr/local/lib/libgvnc-1.0.so.0
/usr/local/lib/libgvnc-1.0.so.0.0.1
/usr/local/bin/gvnccapture
/usr/local/bin/gvncviewer
```

The complete list is `stage/<arch>/.manifest/gtk-vnc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtk-vnc
ls dists/8.0/*/ | grep '^gtk-vnc-'
```

This writes `gtk-vnc-<ver>-r0.tar.gz` (run time) and `gtk-vnc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtk-vnc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtk-vnc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtk-vnc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gtk_vnc`](../../tests/src/) links gtk-vnc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gtk-vnc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gtk-vnc
/usr/local/bin/qnx-port-tests/test_gtk_vnc
```

```
== gtk-vnc ==
  ok   ...
PASS gtk-vnc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk_vnc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gtk_vnc`](../../tests/src/) -- a program linked against gtk-vnc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - vnc_connection_open_host
   - connected and initialised
   - name and size
   - whole screen updated
   - the image, exactly

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gtk-vnc 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gtk-vnc-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gtk-vnc 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gtk-vnc-1.3.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtk-vnc=<version>`), sources in `ported/gtk-vnc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtk-vnc`; port one with `scripts/port-update gtk-vnc`.

<details><summary>recipe.sh</summary>

```bash
# gtk-vnc: VNC client as a GTK 3 widget (libgtk-vnc, libgvnc) and gvncviewer,
# a VNC viewer window for the QNX desktop (labwc). Tight/JPEG, ZRLE, VeNCrypt
# TLS -- what TurboVNC servers speak (VirtualGL's vglrun in a TurboVNC session
# shows OpenGL programs such as RViz here).
VERSION=1.3.1; GIT=https://gitlab.gnome.org/GNOME/gtk-vnc.git; REF=v1.3.1
DEPS="glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11"
BUILDSYS=meson
# Coroutines on GLib threads: portable, no makecontext/swapcontext needed.
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dpulseaudio=disabled -Dsasl=disabled -Dwith-vala=disabled
                   -Dwith-coroutine=gthread )
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  install -Dm755 "$B/examples/gvncviewer" "$STAGE$PREFIX/bin/gvncviewer"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
