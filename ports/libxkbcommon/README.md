# libxkbcommon 1.7.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/xkbcommon/libxkbcommon.git (tag `xkbcommon-1.7.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libxml2`](../libxml2/) |
| Run-time dependencies | [`libxml2`](../libxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxkbcommon
apk add libxkbcommon-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxkbcommon`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxkbcommon` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libxml2
```

<details><summary>Full build chain (7)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b xkbcommon-1.7.0 --recurse-submodules --shallow-submodules \
    https://github.com/xkbcommon/libxkbcommon.git src/libxkbcommon
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libxkbcommon src/libxkbcommon \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Denable-docs=false \
      -Denable-wayland=false \
      -Denable-x11=false \
      -Denable-xkbregistry=true \
      -Denable-tools=false \
      -Denable-bash-completion=false
ninja -C build/$QNX_ARCH/libxkbcommon
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libxkbcommon install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxkbcommon`.

### 5. What it installs

16 files under `/usr/local`: 6 shared-library entries, 6 headers.

```
/usr/local/lib/libxkbcommon.so
/usr/local/lib/libxkbcommon.so.0
/usr/local/lib/libxkbcommon.so.0.0.0
/usr/local/lib/libxkbregistry.so
/usr/local/lib/libxkbregistry.so.0
/usr/local/lib/libxkbregistry.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/libxkbcommon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxkbcommon
ls dists/8.0/*/ | grep '^libxkbcommon-'
```

This writes `libxkbcommon-<ver>-r0.tar.gz` (run time) and `libxkbcommon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxkbcommon
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxkbcommon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxkbcommon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxkbcommon=<version>`), sources in `ported/libxkbcommon/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxkbcommon`; port one with `scripts/port-update libxkbcommon`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.7.0; GIT=https://github.com/xkbcommon/libxkbcommon.git; REF=xkbcommon-1.7.0
DEPS="libxml2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable-docs=false -Denable-wayland=false -Denable-x11=false
                   -Denable-xkbregistry=true -Denable-tools=false -Denable-bash-completion=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
