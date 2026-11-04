# pinentry 1.3.3

> PIN and passphrase entry dialogs for GnuPG (curses and tty)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.3.3.tar.bz2 |
| SHA-256 | `c2970f16d6afb66ecddfca767d743936c86239bff936eed7fd7597a678414b63` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`libassuan`](../libassuan/) [`libgpg-error`](../libgpg-error/) [`ncurses`](../ncurses/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libassuan`](../libassuan/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pinentry`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pinentry` (programs, shared libraries, run-time data) and `pinentry-dev` (headers, pkg-config/CMake files, static libraries; needs `pinentry`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pinentry`](https://openqnx.github.io/oss/package/x86_64/pinentry.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pinentry.html)) · [`pinentry-dev`](https://openqnx.github.io/oss/package/x86_64/pinentry-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pinentry-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pinentry` (dependencies come along); one version: `apk add pinentry=<version>` | `apk del pinentry` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pinentry` (dependencies first) | `oss-get remove pinentry` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pinentry-dev` the same way, by its own name (`apk add pinentry-dev`, `oss-get install pinentry-dev`, by hand with its own file and list `pinentry-dev.list`); remove it before `pinentry`, which it needs.

```sh
# on the QNX target, as root
apk add pinentry pinentry-dev
apk add qnx-test-pinentry && /usr/local/bin/qnx-port-tests/test_pinentry    # its on-target test, from tests-8.0-<arch>-m-z
apk del pinentry pinentry-dev qnx-test-pinentry
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pinentry.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pinentry.list
rm var/lib/manual/pinentry.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pinentry` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libassuan libgpg-error ncurses libiconv
```

<details><summary>Full build chain (6)</summary>

[`gettext`](../gettext/) [`libassuan`](../libassuan/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.3.3.tar.bz2
mkdir -p dist src/pinentry
curl -fL -o dist/$(basename $URL) $URL
echo "c2970f16d6afb66ecddfca767d743936c86239bff936eed7fd7597a678414b63  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pinentry --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pinentry < ports/pinentry/patches/0001-qnx-wcswidth.patch
```

**[`0001-qnx-wcswidth.patch`](patches/0001-qnx-wcswidth.patch)** — Provide wcswidth() on QNX, which only has wcwidth().

> pinentry-curses measures prompts with wcswidth(), which QNX's C library
> does not have (it has wcwidth()): the link fails with "undefined reference
> to `wcswidth'". Provide it on QNX as POSIX defines it, the sum of the
> wcwidth() of each character, -1 for a non-printable one. (The QNX ports
> stub it as the character count, which mis-sizes double-width text.)

Touches: `pinentry/pinentry-curses.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/pinentry && cd build/$QNX_ARCH/pinentry
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" \
$OLDPWD/src/pinentry/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-rpath \
    --enable-pinentry-curses \
    --enable-pinentry-tty \
    --enable-fallback-curses \
    --disable-pinentry-gtk2 \
    --disable-pinentry-gnome3 \
    --disable-pinentry-qt \
    --disable-pinentry-qt5 \
    --disable-pinentry-emacs \
    --disable-inside-emacs \
    --disable-pinentry-fltk \
    --disable-pinentry-efl \
    --disable-libsecret \
    --with-libgpg-error-prefix="$STAGE$PREFIX" \
    --with-libassuan-prefix="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pinentry: the PIN/passphrase entry dialogs that gpg-agent (and other Assuan clients) drive. The terminal flavours: pinentry-curses (also the default `pinentry`) and pinentry-tty; the GTK/Qt/GNOME ones need a desktop session and stay off, as does libsecret caching.
- --disable-rpath: lib-link.m4 links libiconv by its staging path and would record that build-machine directory as the programs' RPATH.
- gpg-error.m4/libassuan.m4 must ask the staged gpgrt-config, not the build machine's (as ports/libassuan does).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pinentry`.

### 5. What it installs

4 files under `/usr/local`, 3 programs.

```
/usr/local/bin/pinentry
/usr/local/bin/pinentry-curses
/usr/local/bin/pinentry-tty
```

The complete list is `stage/<arch>/.manifest/pinentry` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pinentry
ls dists/8.0/*/ | grep '^pinentry-'
```

This writes `pinentry-<ver>-r0.tar.gz` (run time) and `pinentry-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pinentry
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pinentry-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pinentry-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pinentry`](../../tests/src/) links pinentry the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pinentry`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pinentry
/usr/local/bin/qnx-port-tests/test_pinentry
```

```
== pinentry ==
  ok   ...
PASS pinentry
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pinentry` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pinentry`](../../tests/src/) -- a program linked against pinentry the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pinentry-tty started (Assuan on pipes, dialog on a pty)
   - Assuan greeting: OK Pleased to meet you
   - GETINFO version: 1.3.3
   - GETINFO flavor: tty
   - SETDESC, SETPROMPT
   - GETPIN: description and prompt shown on the terminal
   - the typed PIN comes back percent-escaped: D s3cr3t%25x
   - CONFIRM: the buttons prompt answered o -> OK
   - SETCANCEL: the prompt offers [oc]
   - CONFIRM answered c -> ERR ... cancelled
   - pinentry (-> pinentry-curses) started on a vt100 pty
   - Assuan greeting
   - GETINFO flavor: curses
   - the curses dialog is drawn (description and the OK button label)
   - the PIN typed into the dialog: D curses42

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> pinentry`.

## Versions

This recipe builds **1.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pinentry=<version>`), sources in `ported/pinentry/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pinentry`; port one with `scripts/port-update pinentry`.

<details><summary>recipe.sh</summary>

```bash
# pinentry: the PIN/passphrase entry dialogs that gpg-agent (and other
# Assuan clients) drive. The terminal flavours: pinentry-curses (also the
# default `pinentry`) and pinentry-tty; the GTK/Qt/GNOME ones need a
# desktop session and stay off, as does libsecret caching.
VERSION=1.3.3
TARBALL=https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.3.3.tar.bz2
SHA256=c2970f16d6afb66ecddfca767d743936c86239bff936eed7fd7597a678414b63
DEPS="libassuan libgpg-error ncurses libiconv"
BUILDSYS=autotools
# --disable-rpath: lib-link.m4 links libiconv by its staging path and would
# record that build-machine directory as the programs' RPATH.
CONF_ARGS_EXTRA=( --disable-rpath --enable-pinentry-curses --enable-pinentry-tty --enable-fallback-curses
                  --disable-pinentry-gtk2 --disable-pinentry-gnome3 --disable-pinentry-qt
                  --disable-pinentry-qt5 --disable-pinentry-emacs --disable-inside-emacs
                  --disable-pinentry-fltk --disable-pinentry-efl --disable-libsecret
                  --with-libgpg-error-prefix="$STAGE$PREFIX" --with-libassuan-prefix="$STAGE$PREFIX" )
# gpg-error.m4/libassuan.m4 must ask the staged gpgrt-config, not the build
# machine's (as ports/libassuan does).
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
