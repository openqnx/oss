# gsettings-desktop-schemas 49.0

> GSettings schemas for settings shared by desktop components

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git (tag `49.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gsettings_desktop_schemas`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gsettings-desktop-schemas` (programs, shared libraries, run-time data) and `gsettings-desktop-schemas-dev` (headers, pkg-config/CMake files, static libraries; needs `gsettings-desktop-schemas`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gsettings-desktop-schemas`](https://openqnx.github.io/oss/package/x86_64/gsettings-desktop-schemas.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gsettings-desktop-schemas.html)) · [`gsettings-desktop-schemas-dev`](https://openqnx.github.io/oss/package/x86_64/gsettings-desktop-schemas-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gsettings-desktop-schemas-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gsettings-desktop-schemas` (dependencies come along); one version: `apk add gsettings-desktop-schemas=<version>` | `apk del gsettings-desktop-schemas` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gsettings-desktop-schemas` (dependencies first) | `oss-get remove gsettings-desktop-schemas` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gsettings-desktop-schemas-dev` the same way, by its own name (`apk add gsettings-desktop-schemas-dev`, `oss-get install gsettings-desktop-schemas-dev`, by hand with its own file and list `gsettings-desktop-schemas-dev.list`); remove it before `gsettings-desktop-schemas`, which it needs.

```sh
# on the QNX target, as root
apk add gsettings-desktop-schemas gsettings-desktop-schemas-dev
apk add qnx-test-gsettings-desktop-schemas && /usr/local/bin/qnx-port-tests/test_gsettings_desktop_schemas    # its on-target test, from tests-8.0-<arch>
apk del gsettings-desktop-schemas gsettings-desktop-schemas-dev qnx-test-gsettings-desktop-schemas
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gsettings-desktop-schemas.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gsettings-desktop-schemas.list
rm var/lib/manual/gsettings-desktop-schemas.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gsettings-desktop-schemas` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 49.0 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git src/gsettings-desktop-schemas
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gsettings-desktop-schemas src/gsettings-desktop-schemas \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dintrospection=false
ninja -C build/$QNX_ARCH/gsettings-desktop-schemas
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gsettings-desktop-schemas install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GSettings schemas shared by GNOME/GTK applications (data only).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gsettings-desktop-schemas`.

### 5. What it installs

98 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/gsettings-desktop-schemas` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gsettings-desktop-schemas
ls dists/8.0/*/ | grep '^gsettings-desktop-schemas-'
```

This writes `gsettings-desktop-schemas-<ver>-r0.tar.gz` (run time) and `gsettings-desktop-schemas-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gsettings-desktop-schemas
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gsettings-desktop-schemas-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gsettings-desktop-schemas-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gsettings_desktop_schemas`](../../tests/src/) links gsettings-desktop-schemas the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gsettings-desktop-schemas`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gsettings-desktop-schemas
/usr/local/bin/qnx-port-tests/test_gsettings_desktop_schemas
```

```
== gsettings-desktop-schemas ==
  ok   ...
PASS gsettings-desktop-schemas
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gsettings_desktop_schemas` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gsettings_desktop_schemas`](../../tests/src/) -- a program linked against gsettings-desktop-schemas the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - share/glib-2.0/schemas/org.gnome.desktop.*.gschema.xml installed
   - glib-compile-schemas available
   - glib-compile-schemas --strict compiled them
   - schema source loads gschemas.compiled
   - 30+ schemas, wm.preferences and system.proxy among them
   - relocatable app-folders.folder
   - GSettings for org.gnome.desktop.interface (memory backend)
   - defaults: font-name 'Adwaita Sans 11', gtk-theme 'Adwaita'
   - enum defaults match gdesktop-enums.h: 24h, default
   - set color-scheme = G_DESKTOP_COLOR_SCHEME_PREFER_DARK
   - reads back as nick 'prefer-dark'
   - 'neon' is not a valid color-scheme
   - wm num-workspaces: range 1..36
   - num-workspaces default 4
   - org.gnome.desktop.wm.preferences
   - gsettings get org.gnome.desktop.interface font-name

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gsettings-desktop-schemas-49.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gsettings-desktop-schemas-49.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gsettings-desktop-schemas-49.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gsettings-desktop-schemas-49.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **49.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gsettings-desktop-schemas=<version>`), sources in `ported/gsettings-desktop-schemas/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gsettings-desktop-schemas`; port one with `scripts/port-update gsettings-desktop-schemas`.

<details><summary>recipe.sh</summary>

```bash
# GSettings schemas shared by GNOME/GTK applications (data only).
VERSION=49.0; GIT=https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git; REF=49.0
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
