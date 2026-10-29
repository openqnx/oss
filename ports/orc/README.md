# orc 0.4.40

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/gstreamer/orc.git (tag `0.4.40`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_orc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install orc
oss-get install orc-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild orc` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.4.40 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/gstreamer/orc.git src/orc
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/orc < ports/orc/patches/0001-qnx-aarch64-neon-is-mandatory.patch
```

**[`0001-qnx-aarch64-neon-is-mandatory.patch`](patches/0001-qnx-aarch64-neon-is-mandatory.patch)** — orc: NEON is always present on aarch64 QNX.

> QNX has no /proc/cpuinfo or auxv to probe, so orc found no NEON, marked its
> NEON backend unusable and interpreted every program. Advanced SIMD is
> mandatory in ARMv8-A, the reasoning orc already uses for Windows and macOS.

Touches: `orc/orccpu-arm.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/orc src/orc \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dorc-test=disabled \
      -Dbenchmarks=disabled \
      -Dexamples=disabled \
      -Dtests=disabled \
      -Dtools=disabled \
      -Dgtk_doc=disabled
ninja -C build/$QNX_ARCH/orc
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/orc install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH orc`.

### 5. What it installs

37 files under `/usr/local`: 3 shared-library entries, 32 headers.

```
/usr/local/lib/liborc-0.4.so
/usr/local/lib/liborc-0.4.so.0
/usr/local/lib/liborc-0.4.so.0.40.0
```

The complete list is `stage/<arch>/.manifest/orc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH orc
ls repo/8.0/*/ | grep '^orc-'
```

This writes `orc-<ver>-r0.tar.gz` (run time) and `orc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install orc
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/orc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/orc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_orc`](../../tests/src/) links orc the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_orc root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_orc'
```

```
== orc ==
  ok   ...
PASS orc
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.4.40; GIT=https://gitlab.freedesktop.org/gstreamer/orc.git; REF=0.4.40
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dorc-test=disabled -Dbenchmarks=disabled -Dexamples=disabled
                   -Dtests=disabled -Dtools=disabled -Dgtk_doc=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
