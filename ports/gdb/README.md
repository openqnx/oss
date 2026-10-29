# gdb 14.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/gdb/gdb-14.2.tar.xz |
| SHA-256 | `2d4dd8061d8ded12b6c63f55e45344881e8226105f4d2a9b234040efa5ce7772` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`gmp`](../gmp/) [`mpfr`](../mpfr/) [`ncurses`](../ncurses/) [`readline`](../readline/) [`expat`](../expat/) [`zlib`](../zlib/) [`libiconv`](../libiconv/) [`xxhash`](../xxhash/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`expat`](../expat/) [`gmp`](../gmp/) [`libiconv`](../libiconv/) [`mpfr`](../mpfr/) [`ncurses`](../ncurses/) [`readline`](../readline/) [`xxhash`](../xxhash/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gdb
oss-get install gdb-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gdb` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd
```

<details><summary>Full build chain (12)</summary>

[`expat`](../expat/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`mpfr`](../mpfr/) [`ncurses`](../ncurses/) [`readline`](../readline/) [`xxhash`](../xxhash/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/gdb/gdb-14.2.tar.xz
mkdir -p dist src/gdb
curl -fL -o dist/$(basename $URL) $URL
echo "2d4dd8061d8ded12b6c63f55e45344881e8226105f4d2a9b234040efa5ce7772  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gdb --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gdb < ports/gdb/patches/0001-aports-0001-qnx-gdb-changes.patch
patch -p1 -d src/gdb < ports/gdb/patches/0002-aports-0002-qnx-fix-registers.patch
```

**[`0001-aports-0001-qnx-gdb-changes.patch`](patches/0001-aports-0001-qnx-gdb-changes.patch)** — QNX change from QNX's aports (extra/gdb/0001-qnx-gdb-changes.patch).

> Taken from qnx-ports/aports extra/gdb -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> qnx-gdb changes
>
> ---
>  bfd/config.bfd                        |   12 +-
>  bfd/elf.c                             |    5 +-
>  bfd/elfnn-aarch64.c                   |    7 +-
>  gdb/Makefile.in                       |    5 +
>  gdb/aarch64-nto-tdep.c                |  574 ++++
>  gdb/aarch64-nto-tdep.h                |  102 +
>  gdb/amd64-nto-tdep.c                  |  370 +++
>  gdb/amd64-nto-tdep.h                  |   39 +
>  gdb/auxv.c                            |   16 +
>  gdb/auxv.h                            |    1 -
>  gdb/charset.c                         |   10 +
>  gdb/config/nm-nto.h                   |    4 +-
>  gdb/configure.host                    |    3 +-
>  gdb/configure.nat                     |   24 +-
>  gdb/configure.tgt                     |   14 +-
>  gdb/corelow.c                         |   66 +-
>  gdb/extension.c                       |    9 +
>  gdb/gdb_bfd.c                         |    6 +-
>  gdb/i386-nto-tdep.c                   |  413 +--
>  gdb/nm-nto.h                          |   29 +
>  gdb/nto-procfs.c                      | 1747 +++++-----
>  gdb/nto-share/debug.h                 |  305 ++
>  gdb/nto-share/dsmsgs.h                |  778 +++++
>  gdb/nto-share/qnx_linkmap_note.h      |  156 +
>  gdb/nto-tdep.c                        | 1416 ++++++--
>  gdb/nto-tdep.h                        |  229 +-
>  gdb/python/py-gdb-readline.c          |   10 +-
>  gdb/python/python-internal.h          |   24 +
>  gdb/python/python.c                   |   42 +-
>  gdb/remote-nto.c                      | 4353 +++++++++++++++++++++++++
>  gdb/solib-nto.c                       |  328 ++
>  gdb/solib-nto.h                       |   41 +
>  gdb/solib-svr4.c                      |  225 +-
>  gdb/solib-svr4.h                      |   19 +
>  gdb/solib.c                           |   66 +-
>  gdb/thread.c                          |    6 +
>  gdb/unittests/string_view-selftests.c |    2 +-
>  gdbsupport/common-defs.h              |    2 +-
>  gdbsupport/enum-flags.h               |   20 +-
>  gdbsupport/signals.cc                 |   19 +-
>  gnulib/configure                      |    2 +
>  gnulib/import/getcwd.c                |   15 +-
>  gnulib/import/inet_ntop.c             |    2 +-
>  gnulib/import/m4/getcwd-path-max.m4   |    2 +-
>  gnulib/import/m4/getcwd.m4            |    2 +
>  include/elf/common.h                  |   12 +-
>  include/elf/external.h                |    8 -
>  include/gdb/signals.def               |   15 +-
>  libiberty/pex-unix.c                  |    2 +-
>  49 files changed, 10068 insertions(+), 1489 deletions(-)
>  create mode 100644 gdb/aarch64-nto-tdep.c
>  create mode 100644 gdb/aarch64-nto-tdep.h
>  create mode 100644 gdb/amd64-nto-tdep.c
>  create mode 100644 gdb/amd64-nto-tdep.h
>  create mode 100644 gdb/nm-nto.h
>  create mode 100644 gdb/nto-share/debug.h
>  create mode 100644 gdb/nto-share/dsmsgs.h
>  create mode 100644 gdb/nto-share/qnx_linkmap_note.h
>  create mode 100644 gdb/remote-nto.c
>  create mode 100644 gdb/solib-nto.c
>  create mode 100644 gdb/solib-nto.h

Touches: `bfd/config.bfd`, `bfd/elf.c`, `bfd/elfnn-aarch64.c`, `gdb/Makefile.in`, `gdb/aarch64-nto-tdep.c`, `gdb/aarch64-nto-tdep.h` … · Upstream status: QNX-specific

**[`0002-aports-0002-qnx-fix-registers.patch`](patches/0002-aports-0002-qnx-fix-registers.patch)** — QNX change from QNX's aports (extra/gdb/0002-qnx-fix-registers.patch).

> Taken from qnx-ports/aports extra/gdb -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> fix nto_procfs::fetch_registers
>
> CLT-10000
> ---
>  gdb/nto-procfs.c | 55 ++++++++++++++++++++++++++++++------------------
>  1 file changed, 35 insertions(+), 20 deletions(-)

Touches: `gdb/nto-procfs.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gdb; B=build/$QNX_ARCH/gdb; mkdir -p $B
```

```bash
do_build() { make -C "$B" -j"$(nproc)" V=1; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install-gdb maybe-install-gdbserver; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU debugger, running on the target. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv xxhash xz zstd
- With Automake's silent rules, a bfd rule passes its "@echo" prefix to depcomp as the command ("@echo: command not found"); build verbosely.
- The QNX (nto) target code in the aports patch is guarded by __QNXTARGET__, which aports' APKBUILD defines on the command line.
- Install gdb and gdbserver only. The top-level install also installs the libbfd, libopcodes and libctf that gdb builds for itself -- static archives and headers that are the binutils component's, which then had two owners.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gdb`.

### 5. What it installs

37 files under `/usr/local`, 2 programs, 1 headers.

```
/usr/local/bin/gdb
/usr/local/bin/gdb-add-index
```

The complete list is `stage/<arch>/.manifest/gdb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gdb
ls repo/8.0/*/ | grep '^gdb-'
```

This writes `gdb-<ver>-r0.tar.gz` (run time) and `gdb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gdb
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gdb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gdb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# GNU debugger, running on the target. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=14.2
TARBALL=https://ftp.gnu.org/gnu/gdb/gdb-14.2.tar.xz
SHA256=2d4dd8061d8ded12b6c63f55e45344881e8226105f4d2a9b234040efa5ce7772
# Also linked at run time (found by scripts/gen-dependency-index): libiconv xxhash xz zstd
DEPS="gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --with-system-readline --without-python --without-guile --disable-sim --with-expat --with-system-zlib --disable-werror )
# With Automake's silent rules, a bfd rule passes its "@echo" prefix to
# depcomp as the command ("@echo: command not found"); build verbosely.
do_build() { make -C "$B" -j"$(nproc)" V=1; }
# The QNX (nto) target code in the aports patch is guarded by __QNXTARGET__,
# which aports' APKBUILD defines on the command line.
CFLAGS_APPEND="-D__QNXTARGET__"
# Install gdb and gdbserver only. The top-level install also installs the
# libbfd, libopcodes and libctf that gdb builds for itself -- static archives
# and headers that are the binutils component's, which then had two owners.
do_install() { make -C "$B" DESTDIR="$STAGE" install-gdb maybe-install-gdbserver; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
