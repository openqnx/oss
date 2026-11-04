# vala 0.56.18

> Vala compiler (valac), translating Vala to C over GLib/GObject

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/vala/0.56/vala-0.56.18.tar.xz |
| SHA-256 | `f2affe7d40ab63db8e7b9ecc3f6bdc9c2fc7e3134c84ff2d795f482fe926a382` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vala`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `vala` (programs, shared libraries, run-time data) and `vala-dev` (headers, pkg-config/CMake files, static libraries; needs `vala`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`vala`](https://openqnx.github.io/oss/package/x86_64/vala.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vala.html)) · [`vala-dev`](https://openqnx.github.io/oss/package/x86_64/vala-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vala-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add vala` (dependencies come along); one version: `apk add vala=<version>` | `apk del vala` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install vala` (dependencies first) | `oss-get remove vala` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`vala-dev` the same way, by its own name (`apk add vala-dev`, `oss-get install vala-dev`, by hand with its own file and list `vala-dev.list`); remove it before `vala`, which it needs.

```sh
# on the QNX target, as root
apk add vala vala-dev
apk add qnx-test-vala && /usr/local/bin/qnx-port-tests/test_vala    # its on-target test, from tests-8.0-<arch>-m-z
apk del vala vala-dev qnx-test-vala
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/vala.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/vala.list
rm var/lib/manual/vala.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vala` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gettext
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/vala/0.56/vala-0.56.18.tar.xz
mkdir -p dist src/vala
curl -fL -o dist/$(basename $URL) $URL
echo "f2affe7d40ab63db8e7b9ecc3f6bdc9c2fc7e3134c84ff2d795f482fe926a382  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/vala --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/vala && cd build/$QNX_ARCH/vala
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/vala/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-valadoc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Vala: the compiler (valac-0.56, with valac and vapigen) that translates Vala -- a C#-like language over GLib/GObject -- to C, plus libvala-0.56 and the VAPI bindings (share/vala-0.56/vapi). The release archive carries the compiler's own C translation, so it builds with a C compiler and GLib, no Vala needed. On a QNX target without a C compiler, valac -C (and --header, --vapi) produce the C sources to build elsewhere. Version as in QNX's own package list.
- valadoc needs graphviz (libgvc); not built.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vala`.

### 5. What it installs

369 files under `/usr/local`: 2 shared-library entries, 8 programs, 2 headers.

```
/usr/local/lib/libvala-0.56.so
/usr/local/lib/libvala-0.56.so.0
/usr/local/bin/vala
/usr/local/bin/vala-0.56
/usr/local/bin/vala-gen-introspect
/usr/local/bin/vala-gen-introspect-0.56
/usr/local/bin/valac
/usr/local/bin/valac-0.56
/usr/local/bin/vapigen
/usr/local/bin/vapigen-0.56
```

The complete list is `stage/<arch>/.manifest/vala` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vala
ls dists/8.0/*/ | grep '^vala-'
```

This writes `vala-<ver>-r0.tar.gz` (run time) and `vala-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vala
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vala-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vala-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vala`](../../tests/src/) links vala the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vala`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vala
/usr/local/bin/qnx-port-tests/test_vala
```

```
== vala ==
  ok   ...
PASS vala
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vala` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vala`](../../tests/src/) -- a program linked against vala the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> vala`.

## Versions

This recipe builds **0.56.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vala=<version>`), sources in `ported/vala/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vala`; port one with `scripts/port-update vala`.

<details><summary>recipe.sh</summary>

```bash
# Vala: the compiler (valac-0.56, with valac and vapigen) that translates
# Vala -- a C#-like language over GLib/GObject -- to C, plus libvala-0.56 and
# the VAPI bindings (share/vala-0.56/vapi). The release archive carries the
# compiler's own C translation, so it builds with a C compiler and GLib, no
# Vala needed. On a QNX target without a C compiler, valac -C (and
# --header, --vapi) produce the C sources to build elsewhere. Version as in
# QNX's own package list.
VERSION=0.56.18
TARBALL=https://download.gnome.org/sources/vala/0.56/vala-0.56.18.tar.xz
SHA256=f2affe7d40ab63db8e7b9ecc3f6bdc9c2fc7e3134c84ff2d795f482fe926a382
BUILDSYS=autotools
DEPS="glib gettext"
# valadoc needs graphviz (libgvc); not built.
CONF_ARGS_EXTRA=( --disable-valadoc )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
