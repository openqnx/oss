# ncurses 6.5

> Terminal handling and text user interface library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz |
| SHA-256 | `136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ncurses`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ncurses` (programs, shared libraries, run-time data) and `ncurses-dev` (headers, pkg-config/CMake files, static libraries; needs `ncurses`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ncurses`](https://openqnx.github.io/oss/package/x86_64/ncurses.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ncurses.html)) · [`ncurses-dev`](https://openqnx.github.io/oss/package/x86_64/ncurses-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ncurses-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ncurses` (dependencies come along); one version: `apk add ncurses=<version>` | `apk del ncurses` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ncurses` (dependencies first) | `oss-get remove ncurses` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ncurses-dev` the same way, by its own name (`apk add ncurses-dev`, `oss-get install ncurses-dev`, by hand with its own file and list `ncurses-dev.list`); remove it before `ncurses`, which it needs.

```sh
# on the QNX target, as root
apk add ncurses ncurses-dev
apk add qnx-test-ncurses && /usr/local/bin/qnx-port-tests/test_ncurses    # its on-target test, from tests-8.0-<arch>-m-z
apk del ncurses ncurses-dev qnx-test-ncurses
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ncurses.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ncurses.list
rm var/lib/manual/ncurses.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ncurses` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz
mkdir -p dist src/ncurses
curl -fL -o dist/$(basename $URL) $URL
echo "136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/ncurses --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ncurses < ports/ncurses/patches/0001-qnx-shared-library-soname.patch
```

**[`0001-qnx-shared-library-soname.patch`](patches/0001-qnx-shared-library-soname.patch)** — Give QNX shared libraries an soname.

> configure builds QNX (nto-qnx*) shared libraries with the old BSD rule
> (${LD} -Bshareable), which sets no DT_SONAME. Programs linked against
> libncursesw.so then record that file name -- the -dev symlink -- instead of
> libncursesw.so.6, and fail to start where only the run-time library is
> installed ("Could not load library libtinfo.so"). Use the soname scheme
> of the Linux case: lib<name>.so.<ABI_VERSION>.
>
> aclocal.m4 (CF_SHARED_OPTS) is changed with configure, so a regenerated
> configure keeps it.

Touches: `configure`, `aclocal.m4` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ncurses; B=build/$QNX_ARCH/ncurses; mkdir -p $B
```

```bash
do_install() {
  cd "$B" && make install DESTDIR="$STAGE" >/dev/null
  local d="$STAGE$PREFIX/lib"
  for n in ncurses form menu panel; do
    [ -e "$d/lib${n}w.so" ] && ln -sfn "lib${n}w.so" "$d/lib${n}.so"
    [ -e "$d/lib${n}w.a" ]  && ln -sfn "lib${n}w.a"  "$d/lib${n}.a"
  done
  # ncurses builds terminfo into libncursesw unless --with-termlib is used, so
  # -ltinfo has to resolve to the same library.
  [ -e "$d/libncursesw.so" ] && ln -sfn "libncursesw.so" "$d/libtinfo.so"
  [ -e "$d/libncursesw.a" ]  && ln -sfn "libncursesw.a"  "$d/libtinfo.a"
  # And -ltermcap (readline users such as gawk and tftp-hpa link it): unmet
  # here it was the SDP's libtermcap.so -> libncursesw.so.1 that got linked.
  [ -e "$d/libncursesw.so" ] && ln -sfn "libncursesw.so" "$d/libtermcap.so"
  [ -e "$d/libncursesw.a" ]  && ln -sfn "libncursesw.a"  "$d/libtermcap.a"
  for p in ncurses form menu panel tinfo; do
    [ -e "$d/pkgconfig/${p}w.pc" ] && ln -sfn "${p}w.pc" "$d/pkgconfig/${p}.pc"
  done
  find "$STAGE$PREFIX" -name '*.la' -delete
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- QNX's <sys/termio.h> and <termios.h> are mutually exclusive by design -- the header ends in "#error termio/termios incompatibility" when both are pulled in. ncurses includes sys/termio.h whenever configure finds it, so tell configure it is not there and let it use termios alone. ncurses links its shared libraries by invoking ld directly, so LDFLAGS must not carry compiler-driver syntax (ld rejects -Wl,-rpath-link) -- but configure's own link test runs through qcc, which rejects the bare ld spelling just as hard. Neither form works for both, so drop -rpath-link altogether: ncurses needs nothing from staging but libc, and transitive resolution now goes through the link sysroot that env.sh builds. The SDP library directory has to be named explicitly: ncurses links its shared libraries with a raw "ld" invocation that inherits only LDFLAGS, and it links -lregex -- which QNX keeps in $QNX_TARGET/<arch>/lib, a directory only the compiler driver knows about.
- --without-progs: reset/tset use TAB3, a Linux termios delay constant QNX does not define. The target needs the library, not the terminal utilities.
- The .pc directory is the target path: make install puts DESTDIR in front (with the staging tree here, the files went to <stage><stage>/... and the package carried a home/.../stage/... directory).
- --enable-widec renames every library to the "w" variant, so a consumer asking for -lncurses or -ltinfo finds nothing here and the linker falls through to the SDP's own prebuilt ncurses (soname libncursesw.so.1 against our .so.6). Distributions solve this with compatibility symlinks; do the same.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ncurses`.

### 5. What it installs

2932 files under `/usr/local`: 18 shared-library entries, 1 programs, 17 headers.

```
/usr/local/lib/libform.so
/usr/local/lib/libformw.so
/usr/local/lib/libformw.so.6
/usr/local/lib/libformw.so.6.5
/usr/local/lib/libmenu.so
/usr/local/lib/libmenuw.so
/usr/local/lib/libmenuw.so.6
/usr/local/lib/libmenuw.so.6.5
/usr/local/lib/libncurses.so
/usr/local/lib/libncursesw.so
/usr/local/lib/libncursesw.so.6
/usr/local/lib/libncursesw.so.6.5
/usr/local/lib/libpanel.so
/usr/local/lib/libpanelw.so
/usr/local/lib/libpanelw.so.6
/usr/local/lib/libpanelw.so.6.5
...
```

The complete list is `stage/<arch>/.manifest/ncurses` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ncurses
ls dists/8.0/*/ | grep '^ncurses-'
```

This writes `ncurses-<ver>-r0.tar.gz` (run time) and `ncurses-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ncurses
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ncurses-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ncurses-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ncurses`](../../tests/src/) links ncurses the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ncurses`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ncurses
/usr/local/bin/qnx-port-tests/test_ncurses
```

```
== ncurses ==
  ok   ...
PASS ncurses
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ncurses` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ncurses`](../../tests/src/) -- a program linked against ncurses the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - setupterm found a terminal description
   - tigetnum(\"cols\") is a valid capability
   - tigetstr(\"clear\") is a valid capability
   - unknown capability is reported
   - open /dev/null for the screen
   - newterm created a screen
   - screen has a size
   - newwin
   - mvwaddstr
   - wrefresh

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r0 | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ncurses-6.5-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ncurses-6.5-r2.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r0 | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ncurses-6.5-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ncurses 6.5-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ncurses-6.5-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ncurses=<version>`), sources in `ported/ncurses/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ncurses`; port one with `scripts/port-update ncurses`.

<details><summary>recipe.sh</summary>

```bash
VERSION=6.5
TARBALL=https://ftp.gnu.org/gnu/ncurses/ncurses-6.5.tar.gz
SHA256=136d91bc269a9a5785e5f9e980bc76ab57428f604ce3e5a5a90cebc767971cc6
BUILDSYS=autotools
# QNX's <sys/termio.h> and <termios.h> are mutually exclusive by design -- the
# header ends in "#error termio/termios incompatibility" when both are pulled
# in. ncurses includes sys/termio.h whenever configure finds it, so tell
# configure it is not there and let it use termios alone.
# ncurses links its shared libraries by invoking ld directly, so LDFLAGS must
# not carry compiler-driver syntax (ld rejects -Wl,-rpath-link) -- but configure's
# own link test runs through qcc, which rejects the bare ld spelling just as
# hard. Neither form works for both, so drop -rpath-link altogether: ncurses
# needs nothing from staging but libc, and transitive resolution now goes
# through the link sysroot that env.sh builds.
# The SDP library directory has to be named explicitly: ncurses links its
# shared libraries with a raw "ld" invocation that inherits only LDFLAGS, and
# it links -lregex -- which QNX keeps in $QNX_TARGET/<arch>/lib, a directory
# only the compiler driver knows about.
CONF_ENV=( ac_cv_header_sys_termio_h=no ac_cv_header_termio_h=no
           LDFLAGS="-L$STAGE$PREFIX/lib -L$QNX_TARGET/$QNX_ARCH/lib" )
# --without-progs: reset/tset use TAB3, a Linux termios delay constant QNX does
# not define. The target needs the library, not the terminal utilities.
CONF_ARGS_EXTRA=( --with-shared --without-debug --without-ada --without-tests --without-progs
                  --enable-widec --enable-pc-files --without-manpages
                  --with-pkg-config-libdir="$PREFIX/lib/pkgconfig" )
# The .pc directory is the target path: make install puts DESTDIR in front
# (with the staging tree here, the files went to <stage><stage>/... and the
# package carried a home/.../stage/... directory).

# --enable-widec renames every library to the "w" variant, so a consumer asking
# for -lncurses or -ltinfo finds nothing here and the linker falls through to
# the SDP's own prebuilt ncurses (soname libncursesw.so.1 against our .so.6).
# Distributions solve this with compatibility symlinks; do the same.
do_install() {
  cd "$B" && make install DESTDIR="$STAGE" >/dev/null
  local d="$STAGE$PREFIX/lib"
  for n in ncurses form menu panel; do
    [ -e "$d/lib${n}w.so" ] && ln -sfn "lib${n}w.so" "$d/lib${n}.so"
    [ -e "$d/lib${n}w.a" ]  && ln -sfn "lib${n}w.a"  "$d/lib${n}.a"
  done
  # ncurses builds terminfo into libncursesw unless --with-termlib is used, so
  # -ltinfo has to resolve to the same library.
  [ -e "$d/libncursesw.so" ] && ln -sfn "libncursesw.so" "$d/libtinfo.so"
  [ -e "$d/libncursesw.a" ]  && ln -sfn "libncursesw.a"  "$d/libtinfo.a"
  # And -ltermcap (readline users such as gawk and tftp-hpa link it): unmet
  # here it was the SDP's libtermcap.so -> libncursesw.so.1 that got linked.
  [ -e "$d/libncursesw.so" ] && ln -sfn "libncursesw.so" "$d/libtermcap.so"
  [ -e "$d/libncursesw.a" ]  && ln -sfn "libncursesw.a"  "$d/libtermcap.a"
  for p in ncurses form menu panel tinfo; do
    [ -e "$d/pkgconfig/${p}w.pc" ] && ln -sfn "${p}w.pc" "$d/pkgconfig/${p}.pc"
  done
  find "$STAGE$PREFIX" -name '*.la' -delete
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
