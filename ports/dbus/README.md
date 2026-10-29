# dbus 1.16.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/dbus/dbus.git (tag `dbus-1.16.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dbus`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install dbus
oss-get install dbus-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dbus` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild expat
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b dbus-1.16.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/dbus/dbus.git src/dbus
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/dbus src/dbus \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Depoll=disabled \
      -Dsystemd=disabled \
      -Dselinux=disabled \
      -Dapparmor=disabled \
      -Dx11_autolaunch=disabled \
      -Ddoxygen_docs=disabled \
      -Dducktype_docs=disabled \
      -Dxml_docs=disabled \
      -Dmodular_tests=disabled \
      -Dtools=false \
      -Dstats=false
ninja -C build/$QNX_ARCH/dbus
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/dbus install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dbus`.

### 5. What it installs

41 files under `/usr/local`: 3 shared-library entries, 1 programs, 17 headers.

```
/usr/local/lib/libdbus-1.so
/usr/local/lib/libdbus-1.so.3
/usr/local/lib/libdbus-1.so.3.38.3
/usr/local/bin/dbus-daemon
```

The complete list is `stage/<arch>/.manifest/dbus` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH dbus
ls repo/8.0/*/ | grep '^dbus-'
```

This writes `dbus-<ver>-r0.tar.gz` (run time) and `dbus-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install dbus
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/dbus-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/dbus-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_dbus`](../../tests/src/) links dbus the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dbus root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_dbus'
```

```
== dbus ==
  ok   ...
PASS dbus
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.16.0; GIT=https://gitlab.freedesktop.org/dbus/dbus.git; REF=dbus-1.16.0
DEPS="expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Depoll=disabled -Dsystemd=disabled -Dselinux=disabled -Dapparmor=disabled
                   -Dx11_autolaunch=disabled -Ddoxygen_docs=disabled
                   -Dducktype_docs=disabled -Dxml_docs=disabled
                   -Dmodular_tests=disabled -Dtools=false -Dstats=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
