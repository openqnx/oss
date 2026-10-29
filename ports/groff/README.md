# groff 1.23.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz |
| SHA-256 | `6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install groff
oss-get install groff-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild groff` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libiconv
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz
mkdir -p dist src/groff
curl -fL -o dist/$(basename $URL) $URL
echo "6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/groff --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/groff; B=build/$QNX_ARCH/groff; mkdir -p $B
```

```bash
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv
- The build formats groff's own documentation and examples with groff -- the one it has just built is a QNX binary. A distribution's groff often lacks the preprocessors this needs (grn, for instance), so a groff for the build host is built from the same source first and used for that step.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH groff`.

### 5. What it installs

667 files under `/usr/local`, 39 programs.

```
/usr/local/bin/addftinfo
/usr/local/bin/afmtodit
/usr/local/bin/chem
/usr/local/bin/eqn
/usr/local/bin/eqn2graph
/usr/local/bin/gdiffmk
/usr/local/bin/glilypond
/usr/local/bin/gperl
/usr/local/bin/gpinyin
/usr/local/bin/grap2graph
/usr/local/bin/grn
/usr/local/bin/grodvi
/usr/local/bin/groff
/usr/local/bin/grog
/usr/local/bin/grolbp
/usr/local/bin/grolj4
...
```

The complete list is `stage/<arch>/.manifest/groff` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH groff
ls repo/8.0/*/ | grep '^groff-'
```

This writes `groff-<ver>-r0.tar.gz` (run time) and `groff-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install groff
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/groff-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/groff-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.23.0
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz
SHA256=6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-x --with-doc=no --without-gs --without-uchardet )
# The build formats groff's own documentation and examples with groff -- the
# one it has just built is a QNX binary. A distribution's groff often lacks
# the preprocessors this needs (grn, for instance), so a groff for the build
# host is built from the same source first and used for that step.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_host_groff() {
  local h="$B/host-groff"
  [ -x "$h/groff" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc CXX=g++ "$S/configure" --without-x --with-doc=no \
      --without-gs --without-uchardet && _host_env make -C "$h" -j"$(nproc)"; } || return 1
}
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff"; }
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
