# python3 3.14.0

> Python 3 programming language interpreter and standard library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.python.org/ftp/python/3.14.0/Python-3.14.0.tar.xz |
| SHA-256 | `2299dae542d395ce3883aca00d3c910307cd68e0b2f7336098c8e7b7eee9f3e9` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) [`libffi`](../libffi/) [`expat`](../expat/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`sqlite`](../sqlite/) [`ncurses`](../ncurses/) [`readline`](../readline/) [`gdbm`](../gdbm/) [`zstd`](../zstd/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `python3` (programs, shared libraries, run-time data) and `python3-dev` (headers, pkg-config/CMake files, static libraries; needs `python3`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3`](https://openqnx.github.io/oss/package/x86_64/python3.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3.html)) · [`python3-dev`](https://openqnx.github.io/oss/package/x86_64/python3-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3` (dependencies come along); one version: `apk add python3=<version>` | `apk del python3` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3` (dependencies first) | `oss-get remove python3` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`python3-dev` the same way, by its own name (`apk add python3-dev`, `oss-get install python3-dev`, by hand with its own file and list `python3-dev.list`); remove it before `python3`, which it needs.

```sh
# on the QNX target, as root
apk add python3 python3-dev
apk add qnx-test-python && /usr/local/bin/qnx-port-tests/test_python    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3 python3-dev qnx-test-python
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3.list
rm var/lib/manual/python3.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (14 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd
```

<details><summary>Full build chain (14)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://www.python.org/ftp/python/3.14.0/Python-3.14.0.tar.xz
mkdir -p dist src/python3
curl -fL -o dist/$(basename $URL) $URL
echo "2299dae542d395ce3883aca00d3c910307cd68e0b2f7336098c8e7b7eee9f3e9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3 --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/python3 < ports/python3/patches/0001-qnx-support.patch
patch -p1 -d src/python3 < ports/python3/patches/0002-qnx-dirfd-o-directory.patch
```

**[`0001-qnx-support.patch`](patches/0001-qnx-support.patch)** — CPython: build for QNX.

> - configure: recognise *-nto-qnx* hosts (ac_sys_system=QNX); do not define
>   _XOPEN_SOURCE, which on QNX hides everything outside that standard
>   (declarations are gated on _QNX_SOURCE); link with $(CC), which in a cross
>   build carries qcc's -V target; build a real shared libpython with
>   --enable-shared (it silently came out static).
> - mimalloc, pycore_time.h, faulthandler, signal, posix, select, termios: the
>   QNX-specific declarations and the Linux-only interfaces QNX lacks.
> configure is changed with configure.ac.

Touches: `configure.ac`, `configure`, `Objects/mimalloc/prim/unix/prim.c`, `Include/internal/pycore_time.h`, `Modules/faulthandler.c`, `Modules/signalmodule.c` … · Upstream status: QNX-specific

**[`0002-qnx-dirfd-o-directory.patch`](patches/0002-qnx-dirfd-o-directory.patch)** — QNX: open directory fds for scandir/*at with O_DIRECTORY (shutil.rmtree, os.fwalk)

> On QNX, *at() calls and fdopendir() on a directory fd that was opened
> without O_DIRECTORY fail with ENOTDIR (porting hazards 25, 103). The
> fd-based shutil.rmtree() opened each directory O_RDONLY | O_NONBLOCK and
> then scandir()ed and unlinkat()ed under it, so every rmtree() of a tree
> failed -- tempfile.TemporaryDirectory cleanup and setuptools' bdist_wheel
> ("[Errno 20] Not a directory: '...egg-info'") with it. os.fwalk() opened
> its directories the same way. Add O_DIRECTORY to those opens on QNX
> (os.fwalk keeps returning nothing for a non-directory top, as before).
> glob already opens with O_DIRECTORY.

Touches: `Lib/os.py`, `Lib/shutil.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/python3 && cd build/$QNX_ARCH/python3
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
# \
These \
probe \
for \
device \
files, \
which \
configure \
cannot \
do \
across \
a \
cross \
# \
build; \
QNX \
has \
/dev/ptmx \
but \
no \
/dev/ptc. \
ac_cv_file__dev_ptmx=yes \
ac_cv_file__dev_ptc=no \
# \
configure \
otherwise \
tries \
to \
*run* \
a \
target \
binary \
to \
decide \
these. \
ac_cv_buggy_getaddrinfo=no \
ac_cv_have_long_long_format=yes \
ac_cv_little_endian_double=yes \
# \
QNX \
ships \
<sys/timerfd.h> \
with \
timerfd_create() \
and \
timerfd_settime() \
but \
# \
no \
timerfd_gettime(), \
so \
os.timerfd_* \
cannot \
be \
implemented \
completely. \
# \
QNX's \
own \
Python \
3.13 \
port \
reaches \
the \
same \
conclusion \
-- \
its \
pyconfig.h \
# \
leaves \
HAVE_SYS_TIMERFD_H \
and \
HAVE_TIMERFD_CREATE \
undefined. \
ac_cv_header_sys_timerfd_h=no \
ac_cv_func_timerfd_create=no \
# \
QNX's \
<sys/termio.h> \
and \
<termios.h> \
are \
mutually \
exclusive \
by \
design: \
the \
# \
header \
ends \
in \
#error \
termio/termios \
incompatibility \
when \
both \
are \
pulled \
# \
in. \
The \
termios \
module \
includes \
both \
if \
configure \
finds \
sys/termio.h. \
ac_cv_header_sys_termio_h=no \
$OLDPWD/src/python3/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --without-ensurepip \
    --disable-test-modules \
    --with-system-ffi \
    --with-system-expat \
    --with-system-libmpdec \
    --with-build-python="$WS/hosttools/bin/python3.14" \
    --build=x86_64-pc-linux-gnu \
    --without-static-libpython
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): zstd
- Cross-building CPython needs a *native* interpreter of the same version: the build freezes its own stdlib modules and runs setup steps with it. scripts/build-host-tools.sh puts one in hosttools/.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3`.

### 5. What it installs

3026 files under `/usr/local`: 3 shared-library entries, 8 programs, 283 headers.

```
/usr/local/lib/libpython3.14.so
/usr/local/lib/libpython3.14.so.1.0
/usr/local/lib/libpython3.so
/usr/local/bin/idle3
/usr/local/bin/idle3.14
/usr/local/bin/pydoc3
/usr/local/bin/pydoc3.14
/usr/local/bin/python3
/usr/local/bin/python3-config
/usr/local/bin/python3.14
/usr/local/bin/python3.14-config
```

The complete list is `stage/<arch>/.manifest/python3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3
ls dists/8.0/*/ | grep '^python3-'
```

This writes `python3-<ver>-r0.tar.gz` (run time) and `python3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python`](../../tests/src/) links python3 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python
/usr/local/bin/qnx-port-tests/test_python
```

```
== python3 ==
  ok   ...
PASS python3
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python`](../../tests/src/) -- a program linked against python3 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - QNX_PORT_PYTHONHOME names the bundled stdlib
   - Py_InitializeFromConfig
   - Py_IsInitialized
   - stdlib and extension modules all load from the bundle
   - run a generator expression
   - core extension modules import
   - Py_FinalizeEx

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | python3 3.14.0-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/aarch64le/python3-3.14.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3 3.14.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/python3-3.14.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3 3.14.0-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-3.14.0-r2.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3 3.14.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/python3-3.14.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3 3.14.0-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-3.14.0-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.14.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3=<version>`), sources in `ported/python3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3`; port one with `scripts/port-update python3`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.14.0
TARBALL=https://www.python.org/ftp/python/3.14.0/Python-3.14.0.tar.xz
SHA256=2299dae542d395ce3883aca00d3c910307cd68e0b2f7336098c8e7b7eee9f3e9
# Also linked at run time (found by scripts/gen-dependency-index): zstd
DEPS="zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd"
BUILDSYS=autotools
# Cross-building CPython needs a *native* interpreter of the same version: the
# build freezes its own stdlib modules and runs setup steps with it.
# scripts/build-host-tools.sh puts one in hosttools/.
CONF_ENV=(
  # These probe for device files, which configure cannot do across a cross
  # build; QNX has /dev/ptmx but no /dev/ptc.
  ac_cv_file__dev_ptmx=yes
  ac_cv_file__dev_ptc=no
  # configure otherwise tries to *run* a target binary to decide these.
  ac_cv_buggy_getaddrinfo=no
  ac_cv_have_long_long_format=yes
  ac_cv_little_endian_double=yes
  # QNX ships <sys/timerfd.h> with timerfd_create() and timerfd_settime() but
  # no timerfd_gettime(), so os.timerfd_* cannot be implemented completely.
  # QNX's own Python 3.13 port reaches the same conclusion -- its pyconfig.h
  # leaves HAVE_SYS_TIMERFD_H and HAVE_TIMERFD_CREATE undefined.
  ac_cv_header_sys_timerfd_h=no
  ac_cv_func_timerfd_create=no
  # QNX's <sys/termio.h> and <termios.h> are mutually exclusive by design: the
  # header ends in "#error termio/termios incompatibility" when both are pulled
  # in. The termios module includes both if configure finds sys/termio.h.
  ac_cv_header_sys_termio_h=no
)
CONF_ARGS_EXTRA=(
  --enable-shared --without-ensurepip --disable-test-modules
  --with-system-ffi --with-system-expat --with-system-libmpdec
  --with-build-python="$WS/hosttools/bin/python3.14"
  --build=x86_64-pc-linux-gnu
  --without-static-libpython
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
