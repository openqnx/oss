# unibilium 2.1.2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/neovim/unibilium.git (tag `v2.1.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_unibilium`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add unibilium
apk add unibilium-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-unibilium && /usr/local/bin/qnx-port-tests/test_unibilium    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install unibilium`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild unibilium` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.1.2 --recurse-submodules --shallow-submodules \
    https://github.com/neovim/unibilium.git src/unibilium
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/unibilium < ports/unibilium/patches/0001-cmake-libtool-compatible-soname.patch
```

**[`0001-cmake-libtool-compatible-soname.patch`](patches/0001-cmake-libtool-compatible-soname.patch)** — Give the shared library the soname of the libtool build: libunibilium.so.4.

> The CMake build names the library version "${VERSION_MAJOR}.${VERSION_MINOR}",
> two variables nothing defines (project() sets PROJECT_VERSION_MAJOR), so the
> library came out as "libunibilium.so.." with that as its soname, and every
> program linking it -- neovim -- needed a file called "libunibilium.so..".
> Use what the Makefile build of the same release produces (-version-info
> 4:1:0): libunibilium.so.4.0.1, soname libunibilium.so.4, as distributions
> ship it.

Touches: `CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/unibilium -B build/$QNX_ARCH/unibilium -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DTERMINFO_DIRS=/usr/local/share/terminfo:/usr/share/terminfo:/etc/terminfo:/usr/lib/terminfo
cmake --build build/$QNX_ARCH/unibilium -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/unibilium
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- TERMINFO_DIRS: the compiled-in search path. Left alone, CMakeLists asks the BUILD HOST's ncurses6-config and bakes in the host's directories (/etc/terminfo:/lib/terminfo:/usr/share/terminfo), none of which holds this tree's database on the target: the ncurses component installs it in /usr/local/share/terminfo. TERMINFO / TERMINFO_DIRS still override it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH unibilium`.

### 5. What it installs

4 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libunibilium.so
/usr/local/lib/libunibilium.so.4
/usr/local/lib/libunibilium.so.4.0.1
```

The complete list is `stage/<arch>/.manifest/unibilium` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa unibilium
ls dists/8.0/*/ | grep '^unibilium-'
```

This writes `unibilium-<ver>-r0.tar.gz` (run time) and `unibilium-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add unibilium
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/unibilium-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/unibilium-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_unibilium`](../../tests/src/) links unibilium the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-unibilium`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-unibilium
/usr/local/bin/qnx-port-tests/test_unibilium
```

```
== unibilium ==
  ok   ...
PASS unibilium
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_unibilium` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_unibilium`](../../tests/src/) -- a program linked against unibilium the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - unibi_from_term found a terminal description
   - unibi_get_name
   - a numeric capability is readable
   - a boolean capability
   - unibi_dummy
   - capabilities are writable
   - unibi_name_str
   - unibi_short_name_str

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.1.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add unibilium=<version>`), sources in `ported/unibilium/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates unibilium`; port one with `scripts/port-update unibilium`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.1.2; GIT=https://github.com/neovim/unibilium.git; REF=v2.1.2
BUILDSYS=cmake
# TERMINFO_DIRS: the compiled-in search path. Left alone, CMakeLists asks the
# BUILD HOST's ncurses6-config and bakes in the host's directories
# (/etc/terminfo:/lib/terminfo:/usr/share/terminfo), none of which holds this
# tree's database on the target: the ncurses component installs it in
# /usr/local/share/terminfo. TERMINFO / TERMINFO_DIRS still override it.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON
                   -DTERMINFO_DIRS=/usr/local/share/terminfo:/usr/share/terminfo:/etc/terminfo:/usr/lib/terminfo )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
