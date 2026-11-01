# appstream 1.0.6

> Tools and libraries to work with AppStream metadata

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.freedesktop.org/software/appstream/releases/AppStream-1.0.6.tar.xz |
| SHA-256 | `db4439db6a33de3ca1041473501610844ddf1b72ae23016c05242c681c380b4d` |
| Build system | Meson |
| QNX patches | 3 |
| Build dependencies | [`glib`](../glib/) [`curl`](../curl/) [`libxml2`](../libxml2/) [`libyaml`](../libyaml/) [`libxmlb`](../libxmlb/) [`zstd`](../zstd/) |
| Run-time dependencies | [`curl`](../curl/) [`gettext`](../gettext/) [`glib`](../glib/) [`libxml2`](../libxml2/) [`libxmlb`](../libxmlb/) [`libyaml`](../libyaml/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_appstream`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `appstream` (programs, shared libraries, run-time data) and `appstream-dev` (headers, pkg-config/CMake files, static libraries; needs `appstream`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`appstream`](https://openqnx.github.io/oss/package/x86_64/appstream.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/appstream.html)) · [`appstream-dev`](https://openqnx.github.io/oss/package/x86_64/appstream-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/appstream-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add appstream` (dependencies come along); one version: `apk add appstream=<version>` | `apk del appstream` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install appstream` (dependencies first) | `oss-get remove appstream` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`appstream-dev` the same way, by its own name (`apk add appstream-dev`, `oss-get install appstream-dev`, by hand with its own file and list `appstream-dev.list`); remove it before `appstream`, which it needs.

```sh
# on the QNX target, as root
apk add appstream appstream-dev
apk add qnx-test-appstream && /usr/local/bin/qnx-port-tests/test_appstream    # its on-target test, from tests-8.0-<arch>
apk del appstream appstream-dev qnx-test-appstream
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/appstream.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/appstream.list
rm var/lib/manual/appstream.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild appstream` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (24 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib curl libxml2 libyaml libxmlb zstd
```

<details><summary>Full build chain (24)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`libxmlb`](../libxmlb/) [`libyaml`](../libyaml/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://www.freedesktop.org/software/appstream/releases/AppStream-1.0.6.tar.xz
mkdir -p dist src/appstream
curl -fL -o dist/$(basename $URL) $URL
echo "db4439db6a33de3ca1041473501610844ddf1b72ae23016c05242c681c380b4d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/appstream --strip-components=1
```

The project lives in the `AppStream-1.0.6/` subdirectory of the repository.

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/appstream < ports/appstream/patches/0001-cross-skip-cli-metainfo.patch
patch -p1 -d src/appstream < ports/appstream/patches/0002-optional-man-pages.patch
patch -p1 -d src/appstream < ports/appstream/patches/0003-qnx-physical-memory.patch
```

**[`0001-cross-skip-cli-metainfo.patch`](patches/0001-cross-skip-cli-metainfo.patch)** — Do not generate appstreamcli's metainfo in a cross build.

> In a cross build data/meson.build requires a native AppStream (it runs the
> build machine's appstreamcli to merge NEWS into appstreamcli's own metainfo
> file) and then itstool to translate it. This port builds no native
> AppStream and the build machine has no itstool; the file is metadata about
> the appstreamcli tool, not something the library or the tool needs. Skip
> it, and its validation test, in cross builds; everything else in data/ is
> installed as before.

Touches: `AppStream-1.0.6/data/meson.build` · Upstream status: QNX-specific

**[`0002-optional-man-pages.patch`](patches/0002-optional-man-pages.patch)** — Make the manual pages optional when DocBook XSL is missing.

> docs/meson.build stops the configuration when xsltproc cannot load the
> DocBook manpages stylesheet from the local XML catalog, even with docs and
> apidocs off. The build machine here has xsltproc but not the DocBook XSL
> stylesheets. Build the appstreamcli manual page when the stylesheet is
> there, and otherwise warn and go on without it.

Touches: `AppStream-1.0.6/docs/meson.build` · Upstream status: QNX-specific

**[`0003-qnx-physical-memory.patch`](patches/0003-qnx-physical-memory.patch)** — Determine the physical memory size on QNX with sysconf().

> as_get_physical_memory_total() has a branch per OS and #errors on any
> other. QNX's sysconf() answers _SC_PHYS_PAGES and _SC_PAGESIZE, which is
> what the Solaris branch uses: take that branch (and its <unistd.h>) on QNX.

Touches: `AppStream-1.0.6/src/as-system-info.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/appstream/AppStream-1.0.6; B=build/$QNX_ARCH/appstream; mkdir -p $B
```

```bash
do_configure() {
  mkdir -p "$B/qnx-pc" && cat > "$B/qnx-pc/yaml-0.1.pc" <<PC || return 1
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: LibYAML
Description: Library to parse and emit YAML
Version: 0.2.5
Cflags: -I\${includedir}
Libs: -L\${libdir} -lyaml
PC
  PKG_CONFIG_PATH="$B/qnx-pc${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
  meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$arch.ini" \
    --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- AppStream: library and tool (appstreamcli) to read, write and validate AppStream software metadata (metainfo/catalog XML and YAML). Dependency of libadwaita, whose AdwAboutDialog/AdwAboutWindow can be filled in from an application's metainfo. The 1.0 series: 1.1 and later replace libyaml with libfyaml, which is not ported here, and libadwaita needs nothing newer.
- The tarball's paths start with ./AppStream-1.0.6/ (one level more than usual).
- No systemd, stemming (libstemmer), Qt, GObject introspection/Vala, SVG icon rendering (the compose module) or documentation.
- meson looks libyaml up as pkg-config's yaml-0.1, and the libyaml component (built with CMake) installs no yaml-0.1.pc. A minimal one, private to this build, names the staged library; appstream.pc itself does not require it (AppStream strips its Requires.private).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH appstream`.

### 5. What it installs

99 files under `/usr/local`: 3 shared-library entries, 1 programs, 40 headers.

```
/usr/local/lib/libappstream.so
/usr/local/lib/libappstream.so.1.0.6
/usr/local/lib/libappstream.so.5
/usr/local/bin/appstreamcli
```

The complete list is `stage/<arch>/.manifest/appstream` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa appstream
ls dists/8.0/*/ | grep '^appstream-'
```

This writes `appstream-<ver>-r0.tar.gz` (run time) and `appstream-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add appstream
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/appstream-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/appstream-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_appstream`](../../tests/src/) links appstream the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-appstream`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-appstream
/usr/local/bin/qnx-port-tests/test_appstream
```

```
== appstream ==
  ok   ...
PASS appstream
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_appstream` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_appstream`](../../tests/src/) -- a program linked against appstream the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - metainfo parsed
   - component id and kind console-application
   - name and summary
   - developer name
   - project license
   - two releases, newest (1.10.0) first
   - release date 2026-03-01 as a timestamp
   - provides binary porttest
   - homepage URL
   - written back as metainfo XML
   - catalog YAML (DEP-11) output
   - version comparison: 1.10.0 > 1.9.2, 1.0~rc1 < 1.0
   - SPDX license expressions
   - free license check
   - validator accepts the good file
   - validator rejects a file without an id, with issues listed
   - appstreamcli validate --no-net: valid
   - appstreamcli convert to YAML

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | appstream 1.0.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/appstream-1.0.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | appstream 1.0.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/appstream-1.0.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add appstream=<version>`), sources in `ported/appstream/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates appstream`; port one with `scripts/port-update appstream`.

<details><summary>recipe.sh</summary>

```bash
# AppStream: library and tool (appstreamcli) to read, write and validate
# AppStream software metadata (metainfo/catalog XML and YAML). Dependency of
# libadwaita, whose AdwAboutDialog/AdwAboutWindow can be filled in from an
# application's metainfo.
# The 1.0 series: 1.1 and later replace libyaml with libfyaml, which is not
# ported here, and libadwaita needs nothing newer.
VERSION=1.0.6
TARBALL=https://www.freedesktop.org/software/appstream/releases/AppStream-1.0.6.tar.xz
SHA256=db4439db6a33de3ca1041473501610844ddf1b72ae23016c05242c681c380b4d
DEPS="glib curl libxml2 libyaml libxmlb zstd"
BUILDSYS=meson
# The tarball's paths start with ./AppStream-1.0.6/ (one level more than usual).
SUBDIR=AppStream-1.0.6
# No systemd, stemming (libstemmer), Qt, GObject introspection/Vala, SVG
# icon rendering (the compose module) or documentation.
MESON_ARGS_EXTRA=( -Dstemming=false -Dsystemd=false -Dvapi=false -Dqt=false -Dcompose=false
                   -Dapt-support=false -Dgir=false -Dsvg-support=false -Dzstd-support=true
                   -Ddocs=false -Dapidocs=false -Dinstall-docs=false )
# meson looks libyaml up as pkg-config's yaml-0.1, and the libyaml component
# (built with CMake) installs no yaml-0.1.pc. A minimal one, private to this
# build, names the staged library; appstream.pc itself does not require it
# (AppStream strips its Requires.private).
do_configure() {
  mkdir -p "$B/qnx-pc" && cat > "$B/qnx-pc/yaml-0.1.pc" <<PC || return 1
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: LibYAML
Description: Library to parse and emit YAML
Version: 0.2.5
Cflags: -I\${includedir}
Libs: -L\${libdir} -lyaml
PC
  PKG_CONFIG_PATH="$B/qnx-pc${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
  meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$arch.ini" \
    --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
