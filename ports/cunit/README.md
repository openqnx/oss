# cunit 2.1-3

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2 |
| SHA-256 | `f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install cunit
oss-get install cunit-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cunit` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
mkdir -p dist src/cunit
curl -fL -o dist/$(basename $URL) $URL
echo "f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/cunit --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cunit < ports/cunit/patches/0001-drop-ansi-flag.patch
```

**[`0001-drop-ansi-flag.patch`](patches/0001-drop-ansi-flag.patch)** — Do not force -ansi.

> configure.in adds "-Wall -W -pedantic -Wshadow -ansi" to CFLAGS. qcc, the
> QNX compiler driver, rejects -ansi outright ("cc: -ansi no longer
> supported"), so configure concludes the C compiler cannot create
> executables. The same line already asks for -std=c99, which is the standard
> the code is written to; drop the contradictory -ansi.

Touches: `configure.in` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/cunit; B=build/$QNX_ARCH/cunit; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --enable-shared --disable-static --disable-console
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The 2.1-3 tarball ships configure.in without configure, and a source tree autoconf considers "already configured" -- an out-of-tree build is refused. Build in a copy of the tree instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cunit`.

### 5. What it installs

40 files under `/usr/local`: 2 shared-library entries, 9 headers.

```
/usr/local/lib/libcunit.so
/usr/local/lib/libcunit.so.1
```

The complete list is `stage/<arch>/.manifest/cunit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH cunit
ls repo/8.0/*/ | grep '^cunit-'
```

This writes `cunit-<ver>-r0.tar.gz` (run time) and `cunit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install cunit
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/cunit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/cunit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.1-3
TARBALL=https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
SHA256=f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214
BUILDSYS=custom
# The 2.1-3 tarball ships configure.in without configure, and a source tree
# autoconf considers "already configured" -- an out-of-tree build is refused.
# Build in a copy of the tree instead.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --enable-shared --disable-static --disable-console
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
