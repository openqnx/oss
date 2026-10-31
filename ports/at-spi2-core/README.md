# at-spi2-core 2.54.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/at-spi2-core/2.54/at-spi2-core-2.54.0.tar.xz |
| SHA-256 | `d7eee7e75beddcc272cedc2b60535600f3aae6e481589ebc667afc437c0a6079` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`dbus`](../dbus/) [`libxml2`](../libxml2/) [`libx11`](../libx11/) [`libxtst`](../libxtst/) |
| Run-time dependencies | [`dbus`](../dbus/) [`gettext`](../gettext/) [`glib`](../glib/) [`libx11`](../libx11/) [`libxi`](../libxi/) [`libxtst`](../libxtst/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add at-spi2-core
apk add at-spi2-core-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install at-spi2-core`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild at-spi2-core` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib dbus libxml2 libx11 libxtst
```

<details><summary>Full build chain (25)</summary>

[`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxml2`](../libxml2/) [`libxtst`](../libxtst/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/at-spi2-core/2.54/at-spi2-core-2.54.0.tar.xz
mkdir -p dist src/at-spi2-core
curl -fL -o dist/$(basename $URL) $URL
echo "d7eee7e75beddcc272cedc2b60535600f3aae6e481589ebc667afc437c0a6079  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/at-spi2-core --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/at-spi2-core; B=build/$QNX_ARCH/at-spi2-core; mkdir -p $B
```

```bash
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The accessibility bus, the ATK bridge GTK loads as a module -- and ATK itself.  Upstream merged the standalone atk module into at-spi2-core at 2.52, so 2.54 installs libatk-1.0 and atk.pc from its own atk/ subdirectory. There was a separate atk recipe here (2.38.0) and building this one silently overwrote its library: two libatk-1.0.so.0.* files ended up in staging and whichever package built last won.  That recipe is retired, this one is the ATK provider, and every DEPS entry that said "atk" now says "at-spi2-core". The overwrite was visible only on target: ATK 2.54 makes ATK_ROLE_PUSH_BUTTON an alias of ATK_ROLE_BUTTON, so atk_role_get_name() started answering "button" where 2.38 answered "push button", and test_atk failed. -Dintrospection=no: gobject-introspection has to *run* the target's own binaries to scan them, which a cross build cannot do. systemd_user_dir takes a path ("no" became a directory named no); QNX has no systemd, so the unit installed there is removed again after the install. X11 stays on -- libXtst is what AT-SPI uses to synthesise events, and the whole X11 client stack is already staged.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH at-spi2-core`.

### 5. What it installs

195 files under `/usr/local`: 9 shared-library entries, 68 headers.

```
/usr/local/lib/libatk-1.0.so
/usr/local/lib/libatk-1.0.so.0
/usr/local/lib/libatk-1.0.so.0.25409.1
/usr/local/lib/libatk-bridge-2.0.so
/usr/local/lib/libatk-bridge-2.0.so.0
/usr/local/lib/libatk-bridge-2.0.so.0.0.0
/usr/local/lib/libatspi.so
/usr/local/lib/libatspi.so.0
/usr/local/lib/libatspi.so.0.0.1
```

The complete list is `stage/<arch>/.manifest/at-spi2-core` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa at-spi2-core
ls dists/8.0/*/ | grep '^at-spi2-core-'
```

This writes `at-spi2-core-<ver>-r0.tar.gz` (run time) and `at-spi2-core-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add at-spi2-core
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/at-spi2-core-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/at-spi2-core-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/at-spi2-core-2.54.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/at-spi2-core-2.54.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.54.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add at-spi2-core=<version>`), sources in `ported/at-spi2-core/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates at-spi2-core`; port one with `scripts/port-update at-spi2-core`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.54.0
TARBALL=https://download.gnome.org/sources/at-spi2-core/2.54/at-spi2-core-2.54.0.tar.xz
SHA256=d7eee7e75beddcc272cedc2b60535600f3aae6e481589ebc667afc437c0a6079
DEPS="glib dbus libxml2 libx11 libxtst"
BUILDSYS=meson
# The accessibility bus, the ATK bridge GTK loads as a module -- and ATK
# itself.  Upstream merged the standalone atk module into at-spi2-core at
# 2.52, so 2.54 installs libatk-1.0 and atk.pc from its own atk/ subdirectory.
# There was a separate atk recipe here (2.38.0) and building this one silently
# overwrote its library: two libatk-1.0.so.0.* files ended up in staging and
# whichever package built last won.  That recipe is retired, this one is the
# ATK provider, and every DEPS entry that said "atk" now says "at-spi2-core".
#
# The overwrite was visible only on target: ATK 2.54 makes ATK_ROLE_PUSH_BUTTON
# an alias of ATK_ROLE_BUTTON, so atk_role_get_name() started answering
# "button" where 2.38 answered "push button", and test_atk failed.
# -Dintrospection=no: gobject-introspection has to *run* the target's own
# binaries to scan them, which a cross build cannot do.
# systemd_user_dir takes a path ("no" became a directory named no); QNX has no
# systemd, so the unit installed there is removed again after the install.
# X11 stays on -- libXtst is what AT-SPI uses to synthesise events, and the
# whole X11 client stack is already staged.
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Ddocs=false -Dsystemd_user_dir=$PREFIX/lib/systemd/user
                   -Datk_only=false -Dx11=enabled )
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
