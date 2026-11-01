# gettext 0.22.5

> GNU internationalization tools and libraries for translated messages

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.gz |
| SHA-256 | `ec1705b1e969b83a9f073144ec806151db88127f5e40fe5a94cb6c8fa48996a0` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gettext`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gettext` (programs, shared libraries, run-time data) and `gettext-dev` (headers, pkg-config/CMake files, static libraries; needs `gettext`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gettext`](https://openqnx.github.io/oss/package/x86_64/gettext.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gettext.html)) · [`gettext-dev`](https://openqnx.github.io/oss/package/x86_64/gettext-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gettext-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gettext` (dependencies come along); one version: `apk add gettext=<version>` | `apk del gettext` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gettext` (dependencies first) | `oss-get remove gettext` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gettext-dev` the same way, by its own name (`apk add gettext-dev`, `oss-get install gettext-dev`, by hand with its own file and list `gettext-dev.list`); remove it before `gettext`, which it needs.

```sh
# on the QNX target, as root
apk add gettext gettext-dev
apk add qnx-test-gettext && /usr/local/bin/qnx-port-tests/test_gettext    # its on-target test, from tests-8.0-<arch>
apk del gettext gettext-dev qnx-test-gettext
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gettext.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gettext.list
rm var/lib/manual/gettext.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gettext` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses libiconv libunistring
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.gz
mkdir -p dist src/gettext
curl -fL -o dist/$(basename $URL) $URL
echo "ec1705b1e969b83a9f073144ec806151db88127f5e40fe5a94cb6c8fa48996a0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gettext --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gettext < ports/gettext/patches/0001-qnx-getcwd-use-system.patch
```

**[`0001-qnx-getcwd-use-system.patch`](patches/0001-qnx-getcwd-use-system.patch)** — QNX: use the system getcwd in gnulib's getcwd replacement.

> Cross-compiled, gnulib cannot run its getcwd probes and replaces getcwd
> with an implementation that walks up the tree: openat(fd, "..", O_RDONLY)
> then fdopendir().  On QNX a directory fd opened without O_DIRECTORY makes
> openat()/fdopendir() fail with ENOTDIR, so every getcwd() call failed.
> msgfmt -o <anything> writes through open_supersede, which canonicalizes
> the output name starting from getcwd(): "error while opening ... for
> writing: Not a directory", even for a file in the current directory.

Touches: `gettext-tools/gnulib-lib/getcwd.c`

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gettext && cd build/$QNX_ARCH/gettext
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
gt_cv_func_printf_posix=yes \
$OLDPWD/src/gettext/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-java \
    --disable-csharp \
    --enable-nls \
    --without-emacs \
    --disable-openmp \
    --with-included-libxml
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ncurses: libtextstyle links it for terminal styling. Undeclared, gettext built before ncurses in a from-scratch run and picked up the SDP's prebuilt copy (soname libncursesw.so.1 against our .so.6). ncurses itself needs nothing here, so there is no cycle. NOT libxml2: this builds --with-included-libxml, so gettext uses its own bundled copy and never links ours. Declaring it created the workspace's only dependency cycle -- xz -> gettext -> libxml2 -> xz -- which was invisible while everything was already built (the stamps short-circuit the recursion) and recursed until the shell segfaulted on the first from-scratch build on a new machine. Also linked at run time (found by scripts/gen-dependency-index): libiconv libunistring
- --enable-nls (was --disable-nls): with NLS off, gettext installs no libintl at all, and 56 staged libraries silently linked the SDP's prebuilt libintl.so.1 instead -- the one thing this workspace must never do.  scripts/check-sdp-leak could not see it, because it only flagged sonames we had also staged; it now works from an allowlist of genuine OS libraries. gt_cv_func_printf_posix=yes: gettext asks whether printf supports positional arguments ("%2$s") by RUNNING a probe, which a cross build cannot do, so it assumes no and its <libintl.h> then does "#define printf libintl_printf". That macro renames members of any C++ class with a printf method -- WebKit's WTF::PrintStream fails to compile against the SDP's header for exactly this reason.  QNX does support positional arguments; verified on the target: snprintf(b, n, "%2$s-%1$s", "a", "b")  ->  "b-a"
- gnulib has no O_SEARCH on QNX and falls back to O_RDONLY, so save_cwd() and the fdopendir()/*at() emulations open directories without O_DIRECTORY, which QNX then refuses with ENOTDIR (hazards 25, 120).  patches/0001 keeps getcwd() off its openat("..") walk for the same reason: msgfmt -o could not write any file ("error while opening ... for writing: Not a directory").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gettext`.

### 5. What it installs

2030 files under `/usr/local`: 12 shared-library entries, 22 programs, 7 headers.

```
/usr/local/lib/libasprintf.so
/usr/local/lib/libasprintf.so.0
/usr/local/lib/libgettextlib-0.22.5.so
/usr/local/lib/libgettextlib.so
/usr/local/lib/libgettextpo.so
/usr/local/lib/libgettextpo.so.5
/usr/local/lib/libgettextsrc-0.22.5.so
/usr/local/lib/libgettextsrc.so
/usr/local/lib/libintl.so
/usr/local/lib/libintl.so.12
/usr/local/lib/libtextstyle.so
/usr/local/lib/libtextstyle.so.2
/usr/local/bin/autopoint
/usr/local/bin/envsubst
/usr/local/bin/gettext
/usr/local/bin/gettext.sh
...
```

The complete list is `stage/<arch>/.manifest/gettext` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gettext
ls dists/8.0/*/ | grep '^gettext-'
```

This writes `gettext-<ver>-r0.tar.gz` (run time) and `gettext-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gettext
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gettext-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gettext-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gettext`](../../tests/src/) links gettext the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gettext`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gettext
/usr/local/bin/qnx-port-tests/test_gettext
```

```
== gettext ==
  ok   ...
PASS gettext
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gettext` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gettext`](../../tests/src/) -- a program linked against gettext the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gettext 0.22.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gettext-0.22.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gettext 0.22.5-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gettext-0.22.5-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gettext 0.22.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gettext-0.22.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gettext 0.22.5-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gettext-0.22.5-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.22.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gettext=<version>`), sources in `ported/gettext/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gettext`; port one with `scripts/port-update gettext`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.22.5
TARBALL=https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.gz
SHA256=ec1705b1e969b83a9f073144ec806151db88127f5e40fe5a94cb6c8fa48996a0
BUILDSYS=autotools
# ncurses: libtextstyle links it for terminal styling. Undeclared, gettext built
# before ncurses in a from-scratch run and picked up the SDP's prebuilt copy
# (soname libncursesw.so.1 against our .so.6). ncurses itself needs nothing here,
# so there is no cycle.
# NOT libxml2: this builds --with-included-libxml, so gettext uses its own
# bundled copy and never links ours. Declaring it created the workspace's only
# dependency cycle -- xz -> gettext -> libxml2 -> xz -- which was invisible
# while everything was already built (the stamps short-circuit the recursion)
# and recursed until the shell segfaulted on the first from-scratch build on a
# new machine.
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libunistring
DEPS="ncurses libiconv libunistring"
# --enable-nls (was --disable-nls): with NLS off, gettext installs no libintl at
# all, and 56 staged libraries silently linked the SDP's prebuilt libintl.so.1
# instead -- the one thing this workspace must never do.  scripts/check-sdp-leak
# could not see it, because it only flagged sonames we had also staged; it now
# works from an allowlist of genuine OS libraries.
#
# gt_cv_func_printf_posix=yes: gettext asks whether printf supports positional
# arguments ("%2$s") by RUNNING a probe, which a cross build cannot do, so it
# assumes no and its <libintl.h> then does "#define printf libintl_printf".
# That macro renames members of any C++ class with a printf method -- WebKit's
# WTF::PrintStream fails to compile against the SDP's header for exactly this
# reason.  QNX does support positional arguments; verified on the target:
#   snprintf(b, n, "%2$s-%1$s", "a", "b")  ->  "b-a"
CONF_ENV=( gt_cv_func_printf_posix=yes )
# gnulib has no O_SEARCH on QNX and falls back to O_RDONLY, so save_cwd()
# and the fdopendir()/*at() emulations open directories without O_DIRECTORY,
# which QNX then refuses with ENOTDIR (hazards 25, 120).  patches/0001 keeps
# getcwd() off its openat("..") walk for the same reason: msgfmt -o could not
# write any file ("error while opening ... for writing: Not a directory").
CFLAGS_APPEND="-DO_SEARCH=O_DIRECTORY"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-java --disable-csharp
                  --enable-nls --without-emacs --disable-openmp --with-included-libxml )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
