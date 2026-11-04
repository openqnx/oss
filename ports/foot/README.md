# foot 1.23.1

> Fast, lightweight and minimalistic Wayland terminal emulator

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://codeberg.org/dnkl/foot.git (tag `1.23.1`) |
| Build system | Meson |
| QNX patches | 6 |
| Build dependencies | [`pixman`](../pixman/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`fontconfig`](../fontconfig/) [`utf8proc`](../utf8proc/) [`tllist`](../tllist/) [`fcft`](../fcft/) [`ncurses`](../ncurses/) |
| Run-time dependencies | [`fcft`](../fcft/) [`fontconfig`](../fontconfig/) [`libxkbcommon`](../libxkbcommon/) [`pixman`](../pixman/) [`utf8proc`](../utf8proc/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_foot`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `foot`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`foot`](https://openqnx.github.io/oss/package/x86_64/foot.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/foot.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add foot` (dependencies come along); one version: `apk add foot=<version>` | `apk del foot` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install foot` (dependencies first) | `oss-get remove foot` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-foot && /usr/local/bin/qnx-port-tests/test_foot    # its on-target test, from tests-8.0-<arch>
apk del foot qnx-test-foot
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/foot.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/foot.list
rm var/lib/manual/foot.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild foot` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (32 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild pixman wayland wayland-protocols libxkbcommon fontconfig utf8proc tllist fcft ncurses
```

<details><summary>Full build chain (32)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fcft`](../fcft/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`tllist`](../tllist/) [`utf8proc`](../utf8proc/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.23.1 --recurse-submodules --shallow-submodules \
    https://codeberg.org/dnkl/foot.git src/foot
```

### 3. Apply the QNX patches

6 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/foot < ports/foot/patches/0001-qnx-no-epoll-pwait.patch
patch -p1 -d src/foot < ports/foot/patches/0002-qnx-stdlib-min-max.patch
patch -p1 -d src/foot < ports/foot/patches/0003-qnx-no-timerfd-gettime.patch
patch -p1 -d src/foot < ports/foot/patches/0004-qnx-libregex.patch
patch -p1 -d src/foot < ports/foot/patches/0005-qnx-tiocsctty-already-ctty.patch
patch -p1 -d src/foot < ports/foot/patches/0006-rename-spawn.patch
```

**[`0001-qnx-no-epoll-pwait.patch`](patches/0001-qnx-no-epoll-pwait.patch)** — fdm: epoll_wait() with the signal mask set around it on QNX (no epoll_pwait)

> QNX's epoll emulation (libepoll) has epoll_wait() but no epoll_pwait().
> foot keeps its signals blocked except while it waits, so that a signal
> (SIGCHLD from the shell, SIGINT, SIGTERM ...) interrupts the wait. On QNX
> set the wait mask with sigprocmask() around epoll_wait(); a signal that
> arrives between the unmask and the wait has already set got_signal, which is
> checked first, so it is not left pending until the next event. And where
> epoll_pwait() returns EINTR for a signal, QNX's epoll_wait() can return events
> as well: the signal handlers (SIGCHLD ends the terminal) then freed FD
> handlers those events pointed to, and foot crashed on exit (SIGSEGV in
> fdm_poll). Drop the events when a signal came in; they are level-triggered
> and are reported again.

Touches: `fdm.c` · Upstream status: QNX-specific

**[`0002-qnx-stdlib-min-max.patch`](patches/0002-qnx-stdlib-min-max.patch)** — util.h: #undef QNX's <stdlib.h> min()/max()

> QNX's <stdlib.h> defines min() and max() macros (C, default _QNX_SOURCE);
> util.h defines its own and the redefinition is an error under -Werror.
> #undef them first; the definitions are the same expressions.

Touches: `util.h` · Upstream status: QNX-specific

**[`0003-qnx-no-timerfd-gettime.patch`](patches/0003-qnx-no-timerfd-gettime.patch)** — selection: no timerfd_gettime() on QNX; restart the auto-scroll timer

> QNX's timerfd (libtimerfd) implements timerfd_create() and
> timerfd_settime() but not timerfd_gettime(). The selection auto-scroll reads
> the timer only to keep a running timer's remaining time when the scroll
> speed changes; on QNX the timer is restarted to fire at once instead (the
> next scroll step comes immediately, then at the new interval).

Touches: `selection.c` · Upstream status: QNX-specific

**[`0004-qnx-libregex.patch`](patches/0004-qnx-libregex.patch)** — meson: link libregex when there is one (QNX)

> QNX keeps the POSIX regex functions (regcomp, regexec, regerror, regfree)
> in libregex, not in libc; foot (its configuration and URL mode) fails to
> link with "undefined reference to regcomp". Link libregex where the
> library exists (an optional find_library, as for stdthreads).

Touches: `meson.build` · Upstream status: QNX-specific

**[`0005-qnx-tiocsctty-already-ctty.patch`](patches/0005-qnx-tiocsctty-already-ctty.patch)** — slave: a refused TIOCSCTTY is fine when the pty already is the controlling terminal (QNX)

> QNX makes a terminal the controlling terminal of a session leader that
> opens it (System V behaviour), so after setsid() and open() of the pty slave
> it already is, and ioctl(TIOCSCTTY) then fails with EPERM. foot treated that
> as fatal: the shell was never started ("failed to configure controlling
> terminal: Operation not permitted"). On QNX accept the refusal when the pty
> is this session's terminal already (tcgetsid() == getsid(0)), as the vte
> port does.

Touches: `slave.c` · Upstream status: QNX-specific

**[`0006-rename-spawn.patch`](patches/0006-rename-spawn.patch)** — Rename foot's spawn() to foot_spawn(): it overrode QNX libc's spawn()

> foot names its helper for running programs spawn(). QNX's libc has a
> spawn() of its own (<spawn.h>), and implements execvp()/execvpe() through
> spawnp() -> spawn(): with foot's executable exporting a spawn symbol, the
> libc call binds to foot's function instead, and the terminal's child
> process crashed in execvpe() (in foot's reaper_add(), signal 11) before the
> shell ever ran. Rename the function foot_spawn().

Touches: `input.c`, `notify.c`, `spawn.c`, `spawn.h`, `terminal.c`, `url-mode.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/foot src/foot \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=disabled \
      -Dtests=false \
      -Dterminfo=disabled \
      -Ddefault-terminfo=foot \
      -Dgrapheme-clustering=enabled \
      -Dime=true \
      -Dthemes=true \
      -Dutmp-backend=none
ninja -C build/$QNX_ARCH/foot
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/foot install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- foot: a fast, lightweight and minimalistic Wayland terminal emulator (fcft fonts, pixman rendering, a server/client mode, sixel images).
- The "foot" terminfo entries come with ncurses' terminfo database, so foot's own copy is not installed (they would be the same files twice).
- - grapheme clustering with utf8proc; IME support; the colour themes; - no utmp logging (no libutempter on QNX), no man pages (scdoc), no tests.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH foot`.

### 5. What it installs

97 files under `/usr/local`, 2 programs.

```
/usr/local/bin/foot
/usr/local/bin/footclient
```

The complete list is `stage/<arch>/.manifest/foot` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa foot
ls dists/8.0/*/ | grep '^foot-'
```

This writes `foot-<ver>-r0.tar.gz` (run time) and `foot-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add foot
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/foot-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/foot-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_foot`](../../tests/src/) links foot the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-foot`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-foot
/usr/local/bin/qnx-port-tests/test_foot
```

```
== foot ==
  ok   ...
PASS foot
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_foot` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_foot`](../../tests/src/) -- a program linked against foot the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> foot`.

## Versions

This recipe builds **1.23.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add foot=<version>`), sources in `ported/foot/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates foot`; port one with `scripts/port-update foot`.

<details><summary>recipe.sh</summary>

```bash
# foot: a fast, lightweight and minimalistic Wayland terminal emulator
# (fcft fonts, pixman rendering, a server/client mode, sixel images).
VERSION=1.23.1; GIT=https://codeberg.org/dnkl/foot.git; REF=1.23.1
DEPS="pixman wayland wayland-protocols libxkbcommon fontconfig utf8proc tllist fcft ncurses"
# The "foot" terminfo entries come with ncurses' terminfo database, so foot's
# own copy is not installed (they would be the same files twice).
RUNTIME_DEPS="ncurses"
BUILDSYS=meson
# - grapheme clustering with utf8proc; IME support; the colour themes;
# - no utmp logging (no libutempter on QNX), no man pages (scdoc), no tests.
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dtests=false -Dterminfo=disabled -Ddefault-terminfo=foot
                   -Dgrapheme-clustering=enabled -Dime=true -Dthemes=true -Dutmp-backend=none )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
