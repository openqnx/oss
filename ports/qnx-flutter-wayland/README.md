# qnx-flutter-wayland 0.1

> A small Flutter embedder for QNX: Flutter apps in a Wayland window

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`flutter-engine`](../flutter-engine/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) |
| Run-time dependencies | [`flutter-engine`](../flutter-engine/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_flutter_wayland`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `qnx-flutter-wayland` and `qnx-flutter-wayland-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-flutter-wayland` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild flutter-engine wayland wayland-protocols
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`flutter-engine`](../flutter-engine/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-flutter-wayland/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-flutter-wayland; B=build/$QNX_ARCH/qnx-flutter-wayland; mkdir -p $B
```

```bash
do_build() {
  local X="$STAGE$PREFIX/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
  mkdir -p "$B" && cd "$B" && \
  "$_wls" client-header "$X" xdg-shell-client-protocol.h && \
  "$_wls" private-code "$X" xdg-shell-protocol.c && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -I. -I"$STAGE$PREFIX/include" \
      -DICU_DATA_DEFAULT="\"$PREFIX/share/flutter/icudtl.dat\"" \
      -o qnx-flutter-wayland "$S/qnx_flutter_wayland.c" xdg-shell-protocol.c \
      -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -Wl,--disable-new-dtags -Wl,-rpath,"$PREFIX/lib" \
      -lwayland-client -lflutter_engine
}
```

```bash
do_install() { install -Dm755 "$B/qnx-flutter-wayland" "$STAGE$PREFIX/bin/qnx-flutter-wayland"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-flutter-wayland: a small Flutter embedder for QNX on Wayland (labwc). Written for this tree (files/qnx_flutter_wayland.c, C): an xdg-shell window on wl_shm buffers that the engine's software renderer draws into; pointer (mouse) input; its own platform task runner (poll on the Wayland fd, a wake pipe and the next task's time). Keyboard and text input are not done yet. qnx-flutter-wayland <bundle dir>   (flutter_assets/, libapp.so[, icudtl.dat]) QNX's loader does not expand $ORIGIN in RPATH/RUNPATH, so the program names $PREFIX/lib as its RPATH (DT_RPATH) to find libflutter_engine.so and libwayland-client. The loader uses it for the program's own NEEDED only, not for those libraries' (libffi of libwayland-client): outside a desktop session (whose LD_LIBRARY_PATH has $PREFIX/lib) an app's launcher sets LD_LIBRARY_PATH, as flutter-counter does.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-flutter-wayland`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/qnx-flutter-wayland
```

The complete list is `stage/<arch>/.manifest/qnx-flutter-wayland` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-flutter-wayland
ls dists/8.0/*/ | grep '^qnx-flutter-wayland-'
```

This writes `qnx-flutter-wayland-<ver>-r0.tar.gz` (run time) and `qnx-flutter-wayland-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-flutter-wayland
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-flutter-wayland-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-flutter-wayland-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_flutter_wayland`](../../tests/src/) links qnx-flutter-wayland the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-flutter-wayland`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-flutter-wayland
/usr/local/bin/qnx-port-tests/test_qnx_flutter_wayland
```

```
== qnx-flutter-wayland ==
  ok   ...
PASS qnx-flutter-wayland
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_flutter_wayland` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_flutter_wayland`](../../tests/src/) -- a program linked against qnx-flutter-wayland the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> qnx-flutter-wayland`.

## Versions

This recipe builds **0.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-flutter-wayland=<version>`), sources in `ported/qnx-flutter-wayland/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-flutter-wayland`; port one with `scripts/port-update qnx-flutter-wayland`.

<details><summary>recipe.sh</summary>

```bash
# qnx-flutter-wayland: a small Flutter embedder for QNX on Wayland (labwc).
# Written for this tree (files/qnx_flutter_wayland.c, C): an xdg-shell window
# on wl_shm buffers that the engine's software renderer draws into; pointer
# (mouse) input; its own platform task runner (poll on the Wayland fd, a wake
# pipe and the next task's time). Keyboard and text input are not done yet.
#
#   qnx-flutter-wayland <bundle dir>   (flutter_assets/, libapp.so[, icudtl.dat])
#
# QNX's loader does not expand $ORIGIN in RPATH/RUNPATH, so the program names
# $PREFIX/lib as its RPATH (DT_RPATH) to find libflutter_engine.so and
# libwayland-client. The loader uses it for the program's own NEEDED only, not
# for those libraries' (libffi of libwayland-client): outside a desktop session
# (whose LD_LIBRARY_PATH has $PREFIX/lib) an app's launcher sets
# LD_LIBRARY_PATH, as flutter-counter does.
VERSION=0.1
DEPS="flutter-engine wayland wayland-protocols"
RUNTIME_DEPS="flutter-engine wayland"
BUILDSYS=custom
_wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$_wls" ] || _wls=$(command -v wayland-scanner)
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-flutter-wayland/files/." "$SRCROOT/"; }
do_build() {
  local X="$STAGE$PREFIX/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
  mkdir -p "$B" && cd "$B" && \
  "$_wls" client-header "$X" xdg-shell-client-protocol.h && \
  "$_wls" private-code "$X" xdg-shell-protocol.c && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -I. -I"$STAGE$PREFIX/include" \
      -DICU_DATA_DEFAULT="\"$PREFIX/share/flutter/icudtl.dat\"" \
      -o qnx-flutter-wayland "$S/qnx_flutter_wayland.c" xdg-shell-protocol.c \
      -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -Wl,--disable-new-dtags -Wl,-rpath,"$PREFIX/lib" \
      -lwayland-client -lflutter_engine
}
do_install() { install -Dm755 "$B/qnx-flutter-wayland" "$STAGE$PREFIX/bin/qnx-flutter-wayland"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
