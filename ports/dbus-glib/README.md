# dbus-glib 0.116

> Legacy GLib bindings for D-Bus

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.116.tar.gz |
| SHA-256 | `e3f3d4487e2883800770ed5899ed111bdc4ba7056af34a255c4c46ad8a2486f3` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`dbus`](../dbus/) [`glib`](../glib/) [`expat`](../expat/) |
| Run-time dependencies | [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dbus_glib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `dbus-glib` (programs, shared libraries, run-time data) and `dbus-glib-dev` (headers, pkg-config/CMake files, static libraries; needs `dbus-glib`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`dbus-glib`](https://openqnx.github.io/oss/package/x86_64/dbus-glib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dbus-glib.html)) · [`dbus-glib-dev`](https://openqnx.github.io/oss/package/x86_64/dbus-glib-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/dbus-glib-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add dbus-glib` (dependencies come along); one version: `apk add dbus-glib=<version>` | `apk del dbus-glib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install dbus-glib` (dependencies first) | `oss-get remove dbus-glib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`dbus-glib-dev` the same way, by its own name (`apk add dbus-glib-dev`, `oss-get install dbus-glib-dev`, by hand with its own file and list `dbus-glib-dev.list`); remove it before `dbus-glib`, which it needs.

```sh
# on the QNX target, as root
apk add dbus-glib dbus-glib-dev
apk add qnx-test-dbus-glib && /usr/local/bin/qnx-port-tests/test_dbus_glib    # its on-target test, from tests-8.0-<arch>
apk del dbus-glib dbus-glib-dev qnx-test-dbus-glib
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/dbus-glib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/dbus-glib.list
rm var/lib/manual/dbus-glib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dbus-glib` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild dbus glib expat
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.116.tar.gz
mkdir -p dist src/dbus-glib
curl -fL -o dist/$(basename $URL) $URL
echo "e3f3d4487e2883800770ed5899ed111bdc4ba7056af34a255c4c46ad8a2486f3  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/dbus-glib --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dbus-glib; B=build/$QNX_ARCH/dbus-glib; mkdir -p $B
```

```bash
do_build() { ( _dbg_native ) && make -C "$B" -j"$(nproc)"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- dbus-glib: the legacy GLib bindings for D-Bus (DBusGConnection, DBusGProxy, the dbus-binding-tool code generator). Deprecated upstream in favour of GDBus; kept for older software that still uses it.
- The build generates dbus-glib-bindings.h by running dbus-binding-tool, which a cross build can only do with a native copy (hazard 65): build one from the same source against the build machine's dbus-1/glib-2.0 first, and point configure at it. Tests and the bash-completion helper are off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dbus-glib`.

### 5. What it installs

11 files under `/usr/local`: 3 shared-library entries, 1 programs, 5 headers.

```
/usr/local/lib/libdbus-glib-1.so
/usr/local/lib/libdbus-glib-1.so.2
/usr/local/lib/libdbus-glib-1.so.2.3.7
/usr/local/bin/dbus-binding-tool
```

The complete list is `stage/<arch>/.manifest/dbus-glib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dbus-glib
ls dists/8.0/*/ | grep '^dbus-glib-'
```

This writes `dbus-glib-<ver>-r0.tar.gz` (run time) and `dbus-glib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dbus-glib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dbus-glib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dbus-glib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dbus_glib`](../../tests/src/) links dbus-glib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dbus-glib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dbus-glib
/usr/local/bin/qnx-port-tests/test_dbus_glib
```

```
== dbus-glib ==
  ok   ...
PASS dbus-glib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dbus_glib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dbus_glib`](../../tests/src/) -- a program linked against dbus-glib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - private dbus-daemon listening on a UNIX socket
   - dbus_g_bus_get(DBUS_BUS_SESSION)
   - GetId: a 32-hex-digit bus id
   - NameHasOwner: TRUE
   - GetNameOwner: our own unique name
   - begin_call/end_call in the main loop: the same bus id
   - a missing method: GError DBUS_GERROR_UNKNOWN_METHOD
   - dbus-binding-tool --mode=glib-client: a stub org_qnx_Test_echo(proxy, IN_text, OUT_reply, ...)

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> dbus-glib`.

## Versions

This recipe builds **0.116**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dbus-glib=<version>`), sources in `ported/dbus-glib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dbus-glib`; port one with `scripts/port-update dbus-glib`.

<details><summary>recipe.sh</summary>

```bash
# dbus-glib: the legacy GLib bindings for D-Bus (DBusGConnection,
# DBusGProxy, the dbus-binding-tool code generator). Deprecated upstream in
# favour of GDBus; kept for older software that still uses it.
VERSION=0.116
TARBALL=https://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.116.tar.gz
SHA256=e3f3d4487e2883800770ed5899ed111bdc4ba7056af34a255c4c46ad8a2486f3
DEPS="dbus glib expat"
BUILDSYS=autotools
# The build generates dbus-glib-bindings.h by running dbus-binding-tool,
# which a cross build can only do with a native copy (hazard 65): build one
# from the same source against the build machine's dbus-1/glib-2.0 first,
# and point configure at it. Tests and the bash-completion helper are off.
_DBG_NATIVE="$BUILD_DIR/native-dbus-glib"
CONF_ARGS_EXTRA=( --disable-static --disable-tests --disable-bash-completion
                  --with-dbus-binding-tool="$_DBG_NATIVE/dbus/dbus-binding-tool" )
_dbg_native() {
  [ -x "$_DBG_NATIVE/dbus/dbus-binding-tool" ] && return 0
  rm -rf "$_DBG_NATIVE" && mkdir -p "$_DBG_NATIVE" && cd "$_DBG_NATIVE" &&
  env -u PKG_CONFIG -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH \
      PATH=/usr/bin:/bin CC=gcc CFLAGS=-O1 "$S/configure" --disable-tests --disable-bash-completion --disable-static &&
  env PATH=/usr/bin:/bin make -C dbus-gmain -j"$(nproc)" &&
  env PATH=/usr/bin:/bin make -C dbus -j"$(nproc)" dbus-binding-tool
}
do_build() { ( _dbg_native ) && make -C "$B" -j"$(nproc)"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
