# libepoxy 1.5.10

> Library for managing OpenGL, EGL and GLX function pointers

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/anholt/libepoxy.git (tag `1.5.10`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libepoxy`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libepoxy
apk add libepoxy-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libepoxy && /usr/local/bin/qnx-port-tests/test_libepoxy    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libepoxy`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libepoxy` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libx11
```

<details><summary>Full build chain (8)</summary>

[`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.5.10 --recurse-submodules --shallow-submodules \
    https://github.com/anholt/libepoxy.git src/libepoxy
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libepoxy src/libepoxy \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dglx=yes \
      -Degl=yes \
      -Dx11=true \
      -Dtests=false \
      -Ddocs=false
ninja -C build/$QNX_ARCH/libepoxy
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libepoxy install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- EGL dispatch for QNX's Screen EGL, plus GLX dispatch.  GLX was off at first ("QNX has no X11"), but GTK3's X11 backend -- needed by Thunar -- includes <epoxy/glx.h> unconditionally.  Building it needs only the X11 headers: epoxy dlopens libGL on first use, and GTK asks through epoxy_has_glx(), which loads with exit_on_fail=false and simply answers false when there is no libGL (dispatch_glx.c).  No GL implementation is built or linked here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libepoxy`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 7 headers.

```
/usr/local/lib/libepoxy.so
/usr/local/lib/libepoxy.so.0
/usr/local/lib/libepoxy.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/libepoxy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libepoxy
ls dists/8.0/*/ | grep '^libepoxy-'
```

This writes `libepoxy-<ver>-r0.tar.gz` (run time) and `libepoxy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libepoxy
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libepoxy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libepoxy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libepoxy`](../../tests/src/) links libepoxy the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libepoxy`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libepoxy
/usr/local/bin/qnx-port-tests/test_libepoxy
```

```
== libepoxy ==
  ok   ...
PASS libepoxy
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libepoxy` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libepoxy`](../../tests/src/) -- a program linked against libepoxy the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - finds a trailing extension
   - finds a middle extension
   - finds a leading extension
   - rejects a prefix of a present name
   - rejects an absent extension
   - rejects against an empty list

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libepoxy 1.5.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libepoxy-1.5.10-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libepoxy 1.5.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libepoxy-1.5.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.5.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libepoxy=<version>`), sources in `ported/libepoxy/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libepoxy`; port one with `scripts/port-update libepoxy`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.5.10; GIT=https://github.com/anholt/libepoxy.git; REF=1.5.10
DEPS="libx11"
BUILDSYS=meson
# EGL dispatch for QNX's Screen EGL, plus GLX dispatch.  GLX was off at first
# ("QNX has no X11"), but GTK3's X11 backend -- needed by Thunar -- includes
# <epoxy/glx.h> unconditionally.  Building it needs only the X11 headers:
# epoxy dlopens libGL on first use, and GTK asks through epoxy_has_glx(), which
# loads with exit_on_fail=false and simply answers false when there is no libGL
# (dispatch_glx.c).  No GL implementation is built or linked here.
MESON_ARGS_EXTRA=( -Dglx=yes -Degl=yes -Dx11=true -Dtests=false -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
