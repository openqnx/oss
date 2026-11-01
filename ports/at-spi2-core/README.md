# at-spi2-core 2.54.0

> Accessibility infrastructure: AT-SPI D-Bus interfaces, libatspi and ATK

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
| On-target test | [`test_at_spi2_core`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `at-spi2-core` (programs, shared libraries, run-time data) and `at-spi2-core-dev` (headers, pkg-config/CMake files, static libraries; needs `at-spi2-core`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`at-spi2-core`](https://openqnx.github.io/oss/package/x86_64/at-spi2-core.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/at-spi2-core.html)) · [`at-spi2-core-dev`](https://openqnx.github.io/oss/package/x86_64/at-spi2-core-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/at-spi2-core-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add at-spi2-core` (dependencies come along); one version: `apk add at-spi2-core=<version>` | `apk del at-spi2-core` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install at-spi2-core` (dependencies first) | `oss-get remove at-spi2-core` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`at-spi2-core-dev` the same way, by its own name (`apk add at-spi2-core-dev`, `oss-get install at-spi2-core-dev`, by hand with its own file and list `at-spi2-core-dev.list`); remove it before `at-spi2-core`, which it needs.

```sh
# on the QNX target, as root
apk add at-spi2-core at-spi2-core-dev
apk add qnx-test-at-spi2-core && /usr/local/bin/qnx-port-tests/test_at_spi2_core    # its on-target test, from tests-8.0-<arch>
apk del at-spi2-core at-spi2-core-dev qnx-test-at-spi2-core
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/at-spi2-core.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/at-spi2-core.list
rm var/lib/manual/at-spi2-core.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (31 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib dbus libxml2 libx11 libxtst
```

<details><summary>Full build chain (31)</summary>

[`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxml2`](../libxml2/) [`libxtst`](../libxtst/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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

### Test

[`test_at_spi2_core`](../../tests/src/) links at-spi2-core the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-at-spi2-core`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-at-spi2-core
/usr/local/bin/qnx-port-tests/test_at_spi2_core
```

```
== at-spi2-core ==
  ok   ...
PASS at-spi2-core
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_at_spi2_core` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_at_spi2_core`](../../tests/src/) -- a program linked against at-spi2-core the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - atk_role_get_name(BUTTON) (PUSH_BUTTON is its alias)
   - atk_role_for_name(\"check box\")
   - register a custom role
   - AtkObject name and role
   - AtkObject description
   - add LABELLED_BY relation
   - relation target is the label
   - relation type name
   - AtkStateSet AND
   - AtkStateSet XOR
   - state type name
   - atspi_role_get_name(BUTTON)
   - atspi_role_get_name(TABLE_CELL)
   - AtspiStateSet from an array
   - add + set_by_name(\"enabled\", FALSE)
   - empty state set
   - equal sets compare equal
   - compare() returns the one differing state

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/at-spi2-core-2.54.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/at-spi2-core-2.54.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/at-spi2-core-2.54.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | at-spi2-core 2.54.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/at-spi2-core-2.54.0-r0.log) |

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
