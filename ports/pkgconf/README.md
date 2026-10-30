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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pkgconf
apk add pkgconf-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pkgconf`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pkgconf
ls dists/8.0/*/ | grep '^pkgconf-'
```

This writes `pkgconf-<ver>-r0.tar.gz` (run time) and `pkgconf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pkgconf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pkgconf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pkgconf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pkgconf 2.3.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pkgconf 2.3.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pkgconf=<version>`), sources in `ported/pkgconf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pkgconf`; port one with `scripts/port-update pkgconf`.

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
