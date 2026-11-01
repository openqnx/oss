# libdazzle 3.44.0

> Companion library of extra widgets and utilities for GObject and GTK 3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libdazzle.git (tag `3.44.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libdazzle`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libdazzle` (programs, shared libraries, run-time data) and `libdazzle-dev` (headers, pkg-config/CMake files, static libraries; needs `libdazzle`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libdazzle`](https://openqnx.github.io/oss/package/x86_64/libdazzle.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdazzle.html)) · [`libdazzle-dev`](https://openqnx.github.io/oss/package/x86_64/libdazzle-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdazzle-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libdazzle` (dependencies come along); one version: `apk add libdazzle=<version>` | `apk del libdazzle` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libdazzle` (dependencies first) | `oss-get remove libdazzle` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libdazzle-dev` the same way, by its own name (`apk add libdazzle-dev`, `oss-get install libdazzle-dev`, by hand with its own file and list `libdazzle-dev.list`); remove it before `libdazzle`, which it needs.

```sh
# on the QNX target, as root
apk add libdazzle libdazzle-dev
apk add qnx-test-libdazzle && /usr/local/bin/qnx-port-tests/test_libdazzle    # its on-target test, from tests-8.0-<arch>
apk del libdazzle libdazzle-dev qnx-test-libdazzle
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libdazzle.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libdazzle.list
rm var/lib/manual/libdazzle.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdazzle` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (58 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3
```

<details><summary>Full build chain (58)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.44.0 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libdazzle.git src/libdazzle
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libdazzle src/libdazzle \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Denable_tests=false \
      -Denable_tools=false \
      -Dwith_introspection=false \
      -Dwith_vapi=false \
      -Denable_gtk_doc=false
ninja -C build/$QNX_ARCH/libdazzle
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libdazzle install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdazzle`.

### 5. What it installs

154 files under `/usr/local`: 2 shared-library entries, 144 headers.

```
/usr/local/lib/libdazzle-1.0.so
/usr/local/lib/libdazzle-1.0.so.0
```

The complete list is `stage/<arch>/.manifest/libdazzle` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libdazzle
ls dists/8.0/*/ | grep '^libdazzle-'
```

This writes `libdazzle-<ver>-r0.tar.gz` (run time) and `libdazzle-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libdazzle
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libdazzle-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libdazzle-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libdazzle`](../../tests/src/) links libdazzle the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libdazzle`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libdazzle
/usr/local/bin/qnx-port-tests/test_libdazzle
```

```
== libdazzle ==
  ok   ...
PASS libdazzle
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdazzle` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libdazzle`](../../tests/src/) -- a program linked against libdazzle the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - \"gws\" matches only gtk_widget_show
   - case-insensitive \"GTKW\" matches the three gtk_ keys
   - matches sorted by score
   - removed key no longer matches
   - highlight markup
   - trie lookup
   - three keys under the prefix \"car\"
   - remove keeps the prefix key
   - heap top is the maximum
   - extraction is non-increasing
   - ring keeps the last three of five values
   - pattern spec needs every keyword
   - time-span labels

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdazzle 3.44.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libdazzle-3.44.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdazzle 3.44.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libdazzle-3.44.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdazzle 3.44.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libdazzle-3.44.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdazzle 3.44.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libdazzle-3.44.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.44.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libdazzle=<version>`), sources in `ported/libdazzle/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libdazzle`; port one with `scripts/port-update libdazzle`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.44.0; GIT=https://gitlab.gnome.org/GNOME/libdazzle.git; REF=3.44.0
DEPS="glib gtk3"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable_tests=false -Denable_tools=false
                   -Dwith_introspection=false -Dwith_vapi=false -Denable_gtk_doc=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
