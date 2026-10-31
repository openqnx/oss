# wayland 1.24.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/wayland/wayland.git (tag `1.24.0`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`libffi`](../libffi/) [`expat`](../expat/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`libffi`](../libffi/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wayland`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add wayland
apk add wayland-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-wayland && /usr/local/bin/qnx-port-tests/test_wayland    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install wayland`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
git clone --depth 1 -b 1.24.0 --recurse-submodules --shallow-submodules \
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
/usr/local/lib/libwayland-client.so.0.24.0
/usr/local/lib/libwayland-cursor.so
/usr/local/lib/libwayland-cursor.so.0
/usr/local/lib/libwayland-cursor.so.0.24.0
/usr/local/lib/libwayland-egl.so
/usr/local/lib/libwayland-egl.so.1
/usr/local/lib/libwayland-egl.so.1.24.0
/usr/local/lib/libwayland-server.so
/usr/local/lib/libwayland-server.so.0
/usr/local/lib/libwayland-server.so.0.24.0
```

The complete list is `stage/<arch>/.manifest/wayland` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa wayland
ls dists/8.0/*/ | grep '^wayland-'
```

This writes `wayland-<ver>-r0.tar.gz` (run time) and `wayland-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add wayland
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/wayland-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/wayland-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_wayland`](../../tests/src/) links wayland the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-wayland`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-wayland
/usr/local/bin/qnx-port-tests/test_wayland
```

```
== wayland ==
  ok   ...
PASS wayland
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wayland` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_wayland`](../../tests/src/) -- a program linked against wayland the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - wl_display_create
   - wl_display_get_event_loop
   - wl_event_loop_dispatch (epoll shim works)
   - wl_display_add_socket_auto
   - wl_display_destroy
   - wl_display_connect fails cleanly with no compositor
   - xkb_context_new
   - keymap has modifiers
   - xkb_state_new
   - keycode resolved to a keysym
   - xkb_keymap_new_from_names returned without crashing

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wayland 1.23.1-r0 | ❌ FAIL | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wayland 1.24.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/wayland-1.24.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wayland 1.23.1-r0 | ❌ FAIL | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wayland 1.24.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/wayland-1.24.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.0**. Kept from earlier: `1.23.1` ([recipe and patches](versions/1.23.1/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add wayland=<version>`), sources in `ported/wayland/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates wayland`; port one with `scripts/port-update wayland`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.0; GIT=https://gitlab.freedesktop.org/wayland/wayland.git; REF=1.24.0
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
