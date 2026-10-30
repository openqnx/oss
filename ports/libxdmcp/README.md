# libxdmcp 1.1.5

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxdmcp.git (tag `libXdmcp-1.1.5`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`xorgproto`](../xorgproto/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxdmcp
apk add libxdmcp-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxdmcp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxdmcp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild xorgproto
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b libXdmcp-1.1.5 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxdmcp.git src/libxdmcp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxdmcp && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxdmcp && cd build/$QNX_ARCH/libxdmcp
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxdmcp/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --without-xmlto \
    --without-fop
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxdmcp`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libXdmcp.so
/usr/local/lib/libXdmcp.so.6
```

The complete list is `stage/<arch>/.manifest/libxdmcp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxdmcp
ls dists/8.0/*/ | grep '^libxdmcp-'
```

This writes `libxdmcp-<ver>-r0.tar.gz` (run time) and `libxdmcp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxdmcp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxdmcp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxdmcp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxdmcp 1.1.5-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxdmcp 1.1.5-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.1.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxdmcp=<version>`), sources in `ported/libxdmcp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxdmcp`; port one with `scripts/port-update libxdmcp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.1.5; GIT=https://gitlab.freedesktop.org/xorg/lib/libxdmcp.git; REF=libXdmcp-1.1.5
DEPS="xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
