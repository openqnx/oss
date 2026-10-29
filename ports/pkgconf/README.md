# pkgconf 2.3.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://distfiles.ariadne.space/pkgconf/pkgconf-2.3.0.tar.xz |
| SHA-256 | `3a9080ac51d03615e7c1910a0a2a8df08424892b5f13b0628a204d3fcce0ea8b` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pkgconf
oss-get install pkgconf-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pkgconf` performs exactly the same steps (2–5) in one command.

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
URL=https://distfiles.ariadne.space/pkgconf/pkgconf-2.3.0.tar.xz
mkdir -p dist src/pkgconf
curl -fL -o dist/$(basename $URL) $URL
echo "3a9080ac51d03615e7c1910a0a2a8df08424892b5f13b0628a204d3fcce0ea8b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pkgconf --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/pkgconf src/pkgconf \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled
ninja -C build/$QNX_ARCH/pkgconf
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/pkgconf install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball: pkgconf's git tree needs autoreconf and the tarball ships a generated configure.  It also carries its own meson.build, which is the simpler path here.
- A *target* pkg-config, for software built on the target itself.  The cross build on this machine uses hosttools/bin/pkg-config-native and never this binary -- so nothing in the workspace depends on it, and it is here because a QNX image that carries development headers wants one.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pkgconf`.

### 5. What it installs

18 files under `/usr/local`: 3 shared-library entries, 1 programs, 5 headers.

```
/usr/local/lib/libpkgconf.so
/usr/local/lib/libpkgconf.so.5
/usr/local/lib/libpkgconf.so.5.0.0
/usr/local/bin/pkgconf
```

The complete list is `stage/<arch>/.manifest/pkgconf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pkgconf
ls repo/8.0/*/ | grep '^pkgconf-'
```

This writes `pkgconf-<ver>-r0.tar.gz` (run time) and `pkgconf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pkgconf
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pkgconf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pkgconf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.0
# The release tarball: pkgconf's git tree needs autoreconf and the tarball
# ships a generated configure.  It also carries its own meson.build, which is
# the simpler path here.
TARBALL=https://distfiles.ariadne.space/pkgconf/pkgconf-2.3.0.tar.xz
SHA256=3a9080ac51d03615e7c1910a0a2a8df08424892b5f13b0628a204d3fcce0ea8b
BUILDSYS=meson
# A *target* pkg-config, for software built on the target itself.  The cross
# build on this machine uses hosttools/bin/pkg-config-native and never this
# binary -- so nothing in the workspace depends on it, and it is here because
# a QNX image that carries development headers wants one.
MESON_ARGS_EXTRA=( -Dtests=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
