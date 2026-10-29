# wayland 1.23.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/wayland/wayland.git (tag `1.23.1`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`libffi`](../libffi/) [`expat`](../expat/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`libffi`](../libffi/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wayland`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install wayland
oss-get install wayland-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wayland` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libffi expat libxml2
```

<details><summary>Full build chain (9)</summary>

[`expat`](../expat/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.23.1 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/wayland/wayland.git src/wayland
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/wayland < ports/wayland/patches/0001-qnx-peercred-and-memstream.patch
```

**[`0001-qnx-peercred-and-memstream.patch`](patches/0001-qnx-peercred-and-memstream.patch)** — Wayland: peer credentials and open_memstream on QNX.

> wayland-os.c: implement wl_os_socket_peercred() with getpeereid(), which gives
> uid and gid but no pid (reported as 0, as on FreeBSD). connection.c: QNX 8.0
> has no open_memstream() (nor fopencookie/funopen); wl_closure_print, the
> WAYLAND_DEBUG log, gets the same write-then-read-back behaviour from a
> tmpfile().

Touches: `src/wayland-os.c`, `src/connection.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/wayland; B=build/$QNX_ARCH/wayland; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  local pc="$STAGE$PREFIX/lib/pkgconfig/wayland-scanner.pc"
  mkdir -p "$(dirname "$pc")"
  cat > "$pc" <<PC
prefix=$WS/hosttools
datarootdir=\${prefix}/share
pkgdatadir=\${datarootdir}/wayland
bindir=\${prefix}/bin
wayland_scanner=\${bindir}/wayland-scanner

Name: Wayland Scanner
Description: Wayland scanner (build machine binary, for cross builds)
Version: $VERSION
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wayland-scanner has to run on the build machine to turn the XML protocol definitions into C; hosttools/ supplies a native one.
- -Dscanner=false means no wayland-scanner.pc is installed either, and packages ask for the scanner through pkg-config by name -- libxfce4windowing's configure stops at "dependencies missing: wayland-scanner >= 1.20". Publish a .pc in the staging tree that points at the native scanner in hosttools/, which is the one that must actually run.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wayland`.

### 5. What it installs

34 files under `/usr/local`: 12 shared-library entries, 12 headers.

```
/usr/local/lib/libwayland-client.so
/usr/local/lib/libwayland-client.so.0
/usr/local/lib/libwayland-client.so.0.23.1
/usr/local/lib/libwayland-cursor.so
/usr/local/lib/libwayland-cursor.so.0
/usr/local/lib/libwayland-cursor.so.0.23.1
/usr/local/lib/libwayland-egl.so
/usr/local/lib/libwayland-egl.so.1
/usr/local/lib/libwayland-egl.so.1.23.1
/usr/local/lib/libwayland-server.so
/usr/local/lib/libwayland-server.so.0
/usr/local/lib/libwayland-server.so.0.23.1
```

The complete list is `stage/<arch>/.manifest/wayland` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH wayland
ls repo/8.0/*/ | grep '^wayland-'
```

This writes `wayland-<ver>-r0.tar.gz` (run time) and `wayland-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install wayland
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/wayland-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/wayland-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_wayland`](../../tests/src/) links wayland the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wayland root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_wayland'
```

```
== wayland ==
  ok   ...
PASS wayland
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.23.1; GIT=https://gitlab.freedesktop.org/wayland/wayland.git; REF=1.23.1
DEPS="libffi expat libxml2"
BUILDSYS=meson
# wayland-scanner has to run on the build machine to turn the XML protocol
# definitions into C; hosttools/ supplies a native one.
MESON_ARGS_EXTRA=( -Dtests=false -Ddocumentation=false -Ddtd_validation=true
                   -Dscanner=false -Dlibraries=true )

# -Dscanner=false means no wayland-scanner.pc is installed either, and packages
# ask for the scanner through pkg-config by name -- libxfce4windowing's configure
# stops at "dependencies missing: wayland-scanner >= 1.20". Publish a .pc in the
# staging tree that points at the native scanner in hosttools/, which is the one
# that must actually run.
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  local pc="$STAGE$PREFIX/lib/pkgconfig/wayland-scanner.pc"
  mkdir -p "$(dirname "$pc")"
  cat > "$pc" <<PC
prefix=$WS/hosttools
datarootdir=\${prefix}/share
pkgdatadir=\${datarootdir}/wayland
bindir=\${prefix}/bin
wayland_scanner=\${bindir}/wayland-scanner

Name: Wayland Scanner
Description: Wayland scanner (build machine binary, for cross builds)
Version: $VERSION
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
