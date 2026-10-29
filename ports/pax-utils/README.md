# pax-utils 1.3.8

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://dev.gentoo.org/~sam/distfiles/app-misc/pax-utils/pax-utils-1.3.8.tar.xz |
| SHA-256 | `12a168d1aeae2626efdbde3979da88a60ca80bb38e1db565e95ee441ff1d5a0c` |
| Build system | Meson |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pax-utils
oss-get install pax-utils-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pax-utils` performs exactly the same steps (2–5) in one command.

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
URL=https://dev.gentoo.org/~sam/distfiles/app-misc/pax-utils/pax-utils-1.3.8.tar.xz
mkdir -p dist src/pax-utils
curl -fL -o dist/$(basename $URL) $URL
echo "12a168d1aeae2626efdbde3979da88a60ca80bb38e1db565e95ee441ff1d5a0c  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pax-utils --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pax-utils < ports/pax-utils/patches/0001-aports-fix-qnx.patch
patch -p1 -d src/pax-utils < ports/pax-utils/patches/0002-aports-fix-recursive.patch
patch -p1 -d src/pax-utils < ports/pax-utils/patches/0003-link-libregex.patch
```

**[`0001-aports-fix-qnx.patch`](patches/0001-aports-fix-qnx.patch)** — QNX change from QNX's aports (core/pax-utils/fix-qnx.patch).

> Taken from qnx-ports/aports core/pax-utils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `pax-utils-1.3.8/porting.h` · Upstream status: QNX-specific

**[`0002-aports-fix-recursive.patch`](patches/0002-aports-fix-recursive.patch)** — QNX change from QNX's aports (core/pax-utils/fix-recursive.patch).

> Taken from qnx-ports/aports core/pax-utils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `pax-utils-1.3.8/scanelf.c` · Upstream status: QNX-specific

**[`0003-link-libregex.patch`](patches/0003-link-libregex.patch)** — Link libregex where regcomp() lives outside libc (QNX).

> scanelf uses regcomp()/regexec(); QNX provides them in libregex, so the link
> fails with undefined references. Find the library and link it when present.

Touches: `meson.build` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/pax-utils src/pax-utils \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dlddtree_implementation=sh \
      -Duse_seccomp=false \
      -Dbuild_manpages=disabled
ninja -C build/$QNX_ARCH/pax-utils
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/pax-utils install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pax-utils (scanelf, lddtree, ...). Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pax-utils`.

### 5. What it installs

10 files under `/usr/local`, 6 programs.

```
/usr/local/bin/dumpelf
/usr/local/bin/lddtree
/usr/local/bin/pspax
/usr/local/bin/scanelf
/usr/local/bin/scanmacho
/usr/local/bin/symtree
```

The complete list is `stage/<arch>/.manifest/pax-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pax-utils
ls repo/8.0/*/ | grep '^pax-utils-'
```

This writes `pax-utils-<ver>-r0.tar.gz` (run time) and `pax-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pax-utils
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pax-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pax-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# pax-utils (scanelf, lddtree, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.3.8
TARBALL=https://dev.gentoo.org/~sam/distfiles/app-misc/pax-utils/pax-utils-1.3.8.tar.xz
SHA256=12a168d1aeae2626efdbde3979da88a60ca80bb38e1db565e95ee441ff1d5a0c
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlddtree_implementation=sh -Duse_seccomp=false -Dbuild_manpages=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
