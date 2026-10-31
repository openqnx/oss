# libxml2 2.13.5

> XML parser and toolkit library in C

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libxml2.git (tag `v2.13.5`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`xz`](../xz/) |
| Run-time dependencies | [`xz`](../xz/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxml2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxml2
apk add libxml2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libxml2 && /usr/local/bin/qnx-port-tests/test_libxml2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxml2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxml2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib xz
```

<details><summary>Full build chain (6)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.13.5 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libxml2.git src/libxml2
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libxml2 < ports/libxml2/patches/0001-python-module-not-libpython.patch
```

**[`0001-python-module-not-libpython.patch`](patches/0001-python-module-not-libpython.patch)** — Python bindings as a plain extension module (no libpython NEEDED, no version suffix)

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libxml2 -B build/$QNX_ARCH/libxml2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DLIBXML2_WITH_PYTHON=ON \
      -DLIBXML2_WITH_TESTS=OFF \
      -DLIBXML2_WITH_PROGRAMS=OFF \
      -DLIBXML2_WITH_ICONV=OFF \
      -DLIBXML2_WITH_LZMA=ON \
      -DLIBXML2_WITH_ZLIB=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libxml2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libxml2
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Python bindings (libxml2.py + libxml2mod.so) for the target's python3, which itstool imports. The generator runs on the build host's Python of the same version; headers and library come from staging. They install into site-packages; the module links no libpython (patches/0001), so libxml2 itself does not depend on python3.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxml2`.

### 5. What it installs

118 files under `/usr/local`: 3 shared-library entries, 1 programs, 46 headers.

```
/usr/local/lib/libxml2.so
/usr/local/lib/libxml2.so.2
/usr/local/lib/libxml2.so.2.13.5
/usr/local/bin/xml2-config
```

The complete list is `stage/<arch>/.manifest/libxml2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxml2
ls dists/8.0/*/ | grep '^libxml2-'
```

This writes `libxml2-<ver>-r0.tar.gz` (run time) and `libxml2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxml2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxml2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxml2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxml2`](../../tests/src/) links libxml2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxml2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxml2
/usr/local/bin/qnx-port-tests/test_libxml2
```

```
== libxml2 ==
  ok   ...
PASS libxml2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxml2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxml2`](../../tests/src/) -- a program linked against libxml2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - xmlReadMemory
   - XPath found both <p>
   - attribute read
   - xmlDocDumpMemory
   - mismatched tags are rejected

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxml2 2.13.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxml2-2.13.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxml2 2.13.5-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxml2-2.13.5-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxml2 2.13.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxml2-2.13.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxml2 2.13.5-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxml2-2.13.5-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxml2=<version>`), sources in `ported/libxml2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxml2`; port one with `scripts/port-update libxml2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.13.5; GIT=https://gitlab.gnome.org/GNOME/libxml2.git; REF=v2.13.5
DEPS="zlib xz python3"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXML2_WITH_PYTHON=ON
                   -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF
                   -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=ON -DLIBXML2_WITH_ZLIB=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# Python bindings (libxml2.py + libxml2mod.so) for the target's python3, which
# itstool imports. The generator runs on the build host's Python of the same
# version; headers and library come from staging. They install into
# site-packages; the module links no libpython (patches/0001), so libxml2
# itself does not depend on python3.
_pv=3.14
CMAKE_ARGS_EXTRA+=( -DPython_EXECUTABLE="$WS/hosttools/bin/python$_pv"
                    -DPython_INCLUDE_DIR="$STAGE$PREFIX/include/python$_pv"
                    -DPython_LIBRARY="$STAGE$PREFIX/lib/libpython$_pv.so"
                    -DLIBXML2_PYTHON_INSTALL_DIR="$PREFIX/lib/python$_pv/site-packages" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
