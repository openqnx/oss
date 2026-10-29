# man-db 2.13.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz |
| SHA-256 | `8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libpipeline`](../libpipeline/) [`gdbm`](../gdbm/) [`groff`](../groff/) [`libiconv`](../libiconv/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gdbm`](../gdbm/) [`libiconv`](../libiconv/) [`libpipeline`](../libpipeline/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install man-db
oss-get install man-db-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild man-db` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libpipeline gdbm groff libiconv zlib
```

### 2. Get the upstream source

```bash
URL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
mkdir -p dist src/man-db
curl -fL -o dist/$(basename $URL) $URL
echo "8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/man-db --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/man-db; B=build/$QNX_ARCH/man-db; mkdir -p $B
```

```bash
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
- The "manual" directory typesets man-db's user guide with groff's -me macros on the build host, where they are often not installed (Debian's groff-base lacks them). The guide is not needed to run man; leave that directory out of the generated top-level Makefile. (Not with SUBDIRS=... on the make command line: that reaches every sub-make too, and gl/lib then looks for "docs".)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH man-db`.

### 5. What it installs

29 files under `/usr/local`, 9 programs.

```
/usr/local/bin/apropos
/usr/local/bin/catman
/usr/local/bin/lexgrog
/usr/local/bin/man
/usr/local/bin/man-recode
/usr/local/bin/mandb
/usr/local/bin/manpath
/usr/local/bin/whatis
/usr/local/sbin/accessdb
```

The complete list is `stage/<arch>/.manifest/man-db` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH man-db
ls repo/8.0/*/ | grep '^man-db-'
```

This writes `man-db-<ver>-r0.tar.gz` (run time) and `man-db-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install man-db
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/man-db-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/man-db-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.1
TARBALL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
SHA256=8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9
# Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
DEPS="libpipeline gdbm groff libiconv zlib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-setuid --without-libseccomp --disable-cache-owner --with-systemdtmpfilesdir=no --with-systemdsystemunitdir=no )
# The "manual" directory typesets man-db's user guide with groff's -me macros
# on the build host, where they are often not installed (Debian's groff-base
# lacks them). The guide is not needed to run man; leave that directory out of
# the generated top-level Makefile. (Not with SUBDIRS=... on the make command
# line: that reaches every sub-make too, and gl/lib then looks for "docs".)
_no_manual() { sed -i '/^SUBDIRS = /s/ manual / /' "$B/Makefile"; }
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
