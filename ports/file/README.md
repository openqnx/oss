# file 5.46

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://astron.com/pub/file/file-5.46.tar.gz |
| SHA-256 | `c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install file
oss-get install file-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild file` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz zstd
```

<details><summary>Full build chain (8)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://astron.com/pub/file/file-5.46.tar.gz
mkdir -p dist src/file
curl -fL -o dist/$(basename $URL) $URL
echo "c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/file --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/file < ports/file/patches/0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch
```

**[`0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch`](patches/0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch)** — QNX change from QNX's aports (extra/file/file-5.46-pr-579-net147-fix-stack-overrun.patch).

> Taken from qnx-ports/aports extra/file -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> PR/579: net147: Fix stack overrun.
>
> ---
>  src/readelf.c | 4 ++--
>  1 file changed, 2 insertions(+), 2 deletions(-)

Touches: `src/readelf.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/file; B=build/$QNX_ARCH/file; mkdir -p $B
```

```bash
do_build() {
  local h="$B/host-file"
  [ -x "$h/src/file" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc "$S/configure" --disable-shared --disable-libseccomp \
      --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib && _host_env make -C "$h" -j"$(nproc)"; } || return 1
  PATH="$h/src:$PATH" make -C "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- file(1) and libmagic. Version as in the QNX Everywhere 8.0.4r1 image.
- libmagic uses regcomp()/regexec(), which QNX keeps in libregex.
- Cross-compiling the magic database needs a build-host file(1) of exactly this version; build one from the same source and put it first in PATH.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH file`.

### 5. What it installs

9 files under `/usr/local`: 2 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libmagic.so
/usr/local/lib/libmagic.so.1
/usr/local/bin/file
```

The complete list is `stage/<arch>/.manifest/file` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH file
ls repo/8.0/*/ | grep '^file-'
```

This writes `file-<ver>-r0.tar.gz` (run time) and `file-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install file
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/file-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/file-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# file(1) and libmagic. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.46
TARBALL=https://astron.com/pub/file/file-5.46.tar.gz
SHA256=c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088
DEPS="zlib bzip2 xz zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-libseccomp )
# libmagic uses regcomp()/regexec(), which QNX keeps in libregex.
CONF_ENV=( LIBS="-lregex" )
# Cross-compiling the magic database needs a build-host file(1) of exactly
# this version; build one from the same source and put it first in PATH.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
do_build() {
  local h="$B/host-file"
  [ -x "$h/src/file" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc "$S/configure" --disable-shared --disable-libseccomp \
      --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib && _host_env make -C "$h" -j"$(nproc)"; } || return 1
  PATH="$h/src:$PATH" make -C "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
