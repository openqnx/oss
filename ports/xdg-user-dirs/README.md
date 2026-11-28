# xdg-user-dirs 0.20

> Well-known user directories (Desktop, Downloads, Documents, ...): xdg-user-dirs-update and xdg-user-dir

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/xdg-user-dirs.git (tag `v0.20`) |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`gettext`](../gettext/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`gettext`](../gettext/) [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xdg_user_dirs`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xdg-user-dirs` and `xdg-user-dirs-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xdg-user-dirs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gettext libiconv
```

<details><summary>Full build chain (4)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.20 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xdg/xdg-user-dirs.git src/xdg-user-dirs
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xdg-user-dirs < ports/xdg-user-dirs/patches/0001-link-libiconv-where-iconv-is-not-in-libc.patch
patch -p1 -d src/xdg-user-dirs < ports/xdg-user-dirs/patches/0002-absolute-xdgconfdir-and-localedir.patch
```

**[`0001-link-libiconv-where-iconv-is-not-in-libc.patch`](patches/0001-link-libiconv-where-iconv-is-not-in-libc.patch)** — build: link xdg-user-dirs-update with libiconv where iconv is not in libc

> xdg-user-dirs-update converts directory names from UTF-8 to the
> filename_encoding of user-dirs.conf with iconv_open()/iconv(). The meson
> build links only libintl: on QNX iconv is not part of libc but a library of
> its own (libiconv, whose <iconv.h> renames the functions to libiconv_open,
> libiconv), and the link fails with undefined references. Ask meson for its
> "iconv" dependency, which is empty where libc has the functions and
> -liconv where it does not.

Touches: `meson.build`, `src/meson.build` · Upstream status: Pending

**[`0002-absolute-xdgconfdir-and-localedir.patch`](patches/0002-absolute-xdgconfdir-and-localedir.patch)** — build: compile in absolute configuration and locale directories

> XDGCONFDIR and LOCALEDIR are taken from meson's sysconfdir and localedir
> options as they are. Those are relative to the prefix ("etc",
> "share/locale") for every prefix but /usr, where meson makes sysconfdir
> /etc: built for /usr/local the program looked for etc/xdg/user-dirs.defaults
> and share/locale under the current directory, found neither, and created
> no directories unless it happened to be started in /usr/local. Join them
> with the prefix (an absolute sysconfdir, as for /usr, stays what it is).

Touches: `meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xdg-user-dirs; B=build/$QNX_ARCH/xdg-user-dirs; mkdir -p $B
```

```bash
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xdg-user-dirs: the well-known folders of a home directory (Desktop, Downloads, Documents, Music, Pictures, Videos, Templates, Public, Projects). xdg-user-dirs-update creates them at the first login, named in the user's language, and records them in ~/.config/user-dirs.dirs, which GLib (g_get_user_special_dir), the file manager's side pane, file choosers and the desktop read; xdg-user-dir prints one of them. etc/xdg/autostart runs the update when a session starts.
- gettext: libintl (the translated folder names) and msgfmt at build time; libiconv: the folder names are converted from UTF-8 to the configured filename encoding, and QNX's libc has no iconv.
- No man pages: they are made with xsltproc from DocBook stylesheets named by URL. The translations are installed.
- The systemd user unit is installed without an option; QNX has no systemd (the XDG autostart entry does the same job).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xdg-user-dirs`.

### 5. What it installs

85 files under `/usr/local`, 2 programs.

```
/usr/local/bin/xdg-user-dir
/usr/local/bin/xdg-user-dirs-update
```

The complete list is `stage/<arch>/.manifest/xdg-user-dirs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xdg-user-dirs
ls dists/8.0/*/ | grep '^xdg-user-dirs-'
```

This writes `xdg-user-dirs-<ver>-r0.tar.gz` (run time) and `xdg-user-dirs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xdg-user-dirs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xdg-user-dirs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xdg-user-dirs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xdg_user_dirs`](../../tests/src/) links xdg-user-dirs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xdg-user-dirs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xdg-user-dirs
/usr/local/bin/qnx-port-tests/test_xdg_user_dirs
```

```
== xdg-user-dirs ==
  ok   ...
PASS xdg-user-dirs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xdg_user_dirs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xdg_user_dirs`](../../tests/src/) -- a program linked against xdg-user-dirs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xdg-user-dirs`.

## Versions

This recipe builds **0.20**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xdg-user-dirs=<version>`), sources in `ported/xdg-user-dirs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xdg-user-dirs`; port one with `scripts/port-update xdg-user-dirs`.

<details><summary>recipe.sh</summary>

```bash
# xdg-user-dirs: the well-known folders of a home directory (Desktop,
# Downloads, Documents, Music, Pictures, Videos, Templates, Public, Projects).
# xdg-user-dirs-update creates them at the first login, named in the user's
# language, and records them in ~/.config/user-dirs.dirs, which GLib
# (g_get_user_special_dir), the file manager's side pane, file choosers and
# the desktop read; xdg-user-dir prints one of them. etc/xdg/autostart runs
# the update when a session starts.
VERSION=0.20; GIT=https://gitlab.freedesktop.org/xdg/xdg-user-dirs.git; REF=v0.20
# gettext: libintl (the translated folder names) and msgfmt at build time;
# libiconv: the folder names are converted from UTF-8 to the configured
# filename encoding, and QNX's libc has no iconv.
DEPS="gettext libiconv"
BUILDSYS=meson
# No man pages: they are made with xsltproc from DocBook stylesheets named by
# URL. The translations are installed.
MESON_ARGS_EXTRA=( -Ddocs=false -Dnls=true )
# The systemd user unit is installed without an option; QNX has no systemd
# (the XDG autostart entry does the same job).
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
