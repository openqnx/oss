# dconf 51.0

> GSettings storage backend: dconf database, D-Bus service, GIO module and the dconf tool

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/dconf/51/dconf-51.0.tar.xz |
| SHA-256 | `e65c1b7867f836faad9f9ee04acf5c5a6cae2bbc784833fe093207e0cb590248` |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`dbus`](../dbus/) |
| Run-time dependencies | [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dconf`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `dconf` and `dconf-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dconf` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib dbus
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/dconf/51/dconf-51.0.tar.xz
mkdir -p dist src/dconf
curl -fL -o dist/$(basename $URL) $URL
echo "e65c1b7867f836faad9f9ee04acf5c5a6cae2bbc784833fe093207e0cb590248  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/dconf --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dconf < ports/dconf/patches/0001-no-unit-tests-in-cross-build.patch
```

**[`0001-no-unit-tests-in-cross-build.patch`](patches/0001-no-unit-tests-in-cross-build.patch)** — build: leave the unit tests out of a cross build

> The tests directory is built unconditionally: eleven test programs that
> replace D-Bus, gvdb and the shm file with mock objects (and interpose libc
> functions through dlsym(RTLD_NEXT)), a symbol check that runs nm on the build
> machine, and a Python test that starts dbus-daemon. None of them can run when
> the build machine is not the target, and there is no option to turn them
> off, so they are skipped when meson cross-compiles. The port has its own
> test on the target (write, read back, watch, reset through dconf-service).

Touches: `meson.build` · Upstream status: Inappropriate [cross build]

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dconf; B=build/$QNX_ARCH/dconf; mkdir -p $B
```

```bash
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- dconf: the GSettings storage of the GNOME/GTK desktop -- a binary key/value database per user (~/.config/dconf/user), the D-Bus service that writes it (ca.desrt.dconf, started by the session bus on first use), the GIO module that makes it the GSettings backend (lib/gio/modules/libdconfsettings.so), the client library and the dconf command (read, write, list, dump, load, watch, reset, compile, update). With it GTK programs keep their settings between runs; without it GSettings falls back to the memory backend.
- dbus: the build reads session_bus_services_dir from dbus-1.pc, and at run time readers work on the mapped database alone but every write goes through dconf-service on the session bus.
- - no man pages: they are made with xsltproc from DocBook stylesheets fetched by URL; no gtk-doc API reference; - no Vala binding (vapigen is asked for only to find the vapi directory) and no bash completion file (it would add bash-completion as a build dependency); - systemduserunitdir: QNX has no systemd. The unit is installed whatever the option says, so it is given a path of its own and removed after the install.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dconf`.

### 5. What it installs

13 files under `/usr/local`: 3 shared-library entries, 1 programs, 5 headers.

```
/usr/local/lib/libdconf.so
/usr/local/lib/libdconf.so.1
/usr/local/lib/libdconf.so.1.0.0
/usr/local/bin/dconf
```

The complete list is `stage/<arch>/.manifest/dconf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dconf
ls dists/8.0/*/ | grep '^dconf-'
```

This writes `dconf-<ver>-r0.tar.gz` (run time) and `dconf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dconf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dconf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dconf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dconf`](../../tests/src/) links dconf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dconf`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dconf
/usr/local/bin/qnx-port-tests/test_dconf
```

```
== dconf ==
  ok   ...
PASS dconf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dconf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dconf`](../../tests/src/) -- a program linked against dconf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> dconf`.

## Versions

This recipe builds **51.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dconf=<version>`), sources in `ported/dconf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dconf`; port one with `scripts/port-update dconf`.

<details><summary>recipe.sh</summary>

```bash
# dconf: the GSettings storage of the GNOME/GTK desktop -- a binary key/value
# database per user (~/.config/dconf/user), the D-Bus service that writes it
# (ca.desrt.dconf, started by the session bus on first use), the GIO module that
# makes it the GSettings backend (lib/gio/modules/libdconfsettings.so), the
# client library and the dconf command (read, write, list, dump, load, watch,
# reset, compile, update). With it GTK programs keep their settings between
# runs; without it GSettings falls back to the memory backend.
VERSION=51.0
TARBALL=https://download.gnome.org/sources/dconf/51/dconf-51.0.tar.xz
SHA256=e65c1b7867f836faad9f9ee04acf5c5a6cae2bbc784833fe093207e0cb590248
# dbus: the build reads session_bus_services_dir from dbus-1.pc, and at run time
# readers work on the mapped database alone but every write goes through
# dconf-service on the session bus.
DEPS="glib dbus"
RUNTIME_DEPS="dbus"
BUILDSYS=meson
# - no man pages: they are made with xsltproc from DocBook stylesheets fetched
#   by URL; no gtk-doc API reference;
# - no Vala binding (vapigen is asked for only to find the vapi directory) and
#   no bash completion file (it would add bash-completion as a build dependency);
# - systemduserunitdir: QNX has no systemd. The unit is installed whatever the
#   option says, so it is given a path of its own and removed after the install.
MESON_ARGS_EXTRA=( -Dman=false -Dgtk_doc=false -Dvapi=false -Dbash_completion=false
                   -Dsystemduserunitdir=$PREFIX/lib/systemd/user )
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
