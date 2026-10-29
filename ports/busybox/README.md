# busybox 1.36.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/vda-linux/busybox_mirror.git (tag `1_36_1`) |
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
oss-get install busybox
oss-get install busybox-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild busybox` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1_36_1 --recurse-submodules --shallow-submodules \
    https://github.com/vda-linux/busybox_mirror.git src/busybox
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/busybox < ports/busybox/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — migrate qnx patch from aports

> Imported from qnx-ports (vda-linux/busybox_mirror 1_36_1...qnx-1_36_1_p0); original authorship kept.
>
>
>
> ---
>  coreutils/ls.c          |  6 +++---
>  coreutils/stat.c        | 17 ++++++++++-------
>  coreutils/uname.c       | 17 +++++++++++++++++
>  include/libbb.h         |  4 ++--
>  include/platform.h      | 20 ++++++++++++++++++++
>  libbb/makedev.c         |  9 ++++++++-
>  libbb/mode_string.c     | 10 +++++++++-
>  scripts/kconfig/mconf.c |  2 +-
>  8 files changed, 70 insertions(+), 15 deletions(-)

Touches: `coreutils/ls.c`, `coreutils/stat.c`, `coreutils/uname.c`, `include/libbb.h`, `include/platform.h`, `libbb/makedev.c` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/busybox; B=build/$QNX_ARCH/busybox; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" && { yes "" || :; } | _bbmake oldconfig >/dev/null; }
```

```bash
do_build() { _bbmake busybox; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- BusyBox 1.36.1 with qnx-ports' QNX support (patches/: the fork's QNX commit), built with the applet configuration of the QNX Everywhere 8.0.4r1 image (busybox.config, from qnx-ports/aports core/busybox).
- oldconfig asks about options the saved .config does not mention; "yes" takes the defaults. yes ends on SIGPIPE when make is done, which under the driver's pipefail failed the whole step, hence the "|| :".

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH busybox`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/busybox
```

The complete list is `stage/<arch>/.manifest/busybox` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH busybox
ls repo/8.0/*/ | grep '^busybox-'
```

This writes `busybox-<ver>-r0.tar.gz` (run time) and `busybox-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install busybox
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/busybox-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/busybox-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# BusyBox 1.36.1 with qnx-ports' QNX support (patches/: the fork's QNX
# commit), built with the applet configuration of the QNX Everywhere 8.0.4r1
# image (busybox.config, from qnx-ports/aports core/busybox).
VERSION=1.36.1; GIT=https://github.com/vda-linux/busybox_mirror.git; REF=1_36_1
BUILDSYS=custom
_bbmake() { make -C "$B" -j"$(nproc)" CROSS_COMPILE="$QNX_HOST/usr/bin/nto${QNX_CPU}-" CC="qcc -V$QCC_TARGET" \
  HOSTCC=gcc HOSTLDFLAGS="" LDFLAGS="-Wl,-z,stack-size=4194304" \
  CONFIG_EXTRA_CFLAGS="-D_QNX_SOURCE -DBB_GLOBAL_CONST=''" CONFIG_EXTRA_LDLIBS="regex socket" "$@"; }
# oldconfig asks about options the saved .config does not mention; "yes" takes
# the defaults. yes ends on SIGPIPE when make is done, which under the
# driver's pipefail failed the whole step, hence the "|| :".
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" && { yes "" || :; } | _bbmake oldconfig >/dev/null; }
do_build() { _bbmake busybox; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
