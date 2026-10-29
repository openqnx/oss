# gettext 0.22.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.gz |
| SHA-256 | `ec1705b1e969b83a9f073144ec806151db88127f5e40fe5a94cb6c8fa48996a0` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gettext
apk add gettext-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gettext`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

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
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-java --disable-csharp
                  --enable-nls --without-emacs --disable-openmp --with-included-libxml )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
