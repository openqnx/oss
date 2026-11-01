# python3-ml-dtypes 0.6.0

> NumPy dtype extensions for machine learning (bfloat16, float8, int4)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/12/72/307d7c4bd0600601c7133fba5cb78af7db968152951c1cd473abb1cda782/ml_dtypes-0.6.0.tar.gz |
| SHA-256 | `5e60251d32ced5598972e4d5e06a2f044341f9291402551a3f6f0ec44f9299b0` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`numpy`](../numpy/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_ml_dtypes`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-ml-dtypes`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-ml-dtypes`](https://openqnx.github.io/oss/package/x86_64/python3-ml-dtypes.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-ml-dtypes.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-ml-dtypes` (dependencies come along); one version: `apk add python3-ml-dtypes=<version>` | `apk del python3-ml-dtypes` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-ml-dtypes` (dependencies first) | `oss-get remove python3-ml-dtypes` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-ml-dtypes && /usr/local/bin/qnx-port-tests/test_python3_ml_dtypes    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-ml-dtypes qnx-test-python3-ml-dtypes
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-ml-dtypes.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-ml-dtypes.list
rm var/lib/manual/python3-ml-dtypes.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-ml-dtypes` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild numpy
```

<details><summary>Full build chain (17)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/12/72/307d7c4bd0600601c7133fba5cb78af7db968152951c1cd473abb1cda782/ml_dtypes-0.6.0.tar.gz
mkdir -p dist src/python3-ml-dtypes
curl -fL -o dist/$(basename $URL) $URL
echo "5e60251d32ced5598972e4d5e06a2f044341f9291402551a3f6f0ec44f9299b0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-ml-dtypes --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/python3-ml-dtypes < ports/python3-ml-dtypes/patches/0001-qnx-libcxx-isnan-eigen-half.patch
```

**[`0001-qnx-libcxx-isnan-eigen-half.patch`](patches/0001-qnx-libcxx-isnan-eigen-half.patch)** — std::isnan/isinf/isfinite overloads for Eigen::half and bfloat16 on QNX libc++

> QNX's libc++ (from SDP 8.0) declares std::isnan, std::isinf and
> std::isfinite only as templates constrained to arithmetic types; there are
> no float/double overloads that a type converting to float could use.
> Eigen::half and Eigen::bfloat16 are such types, and the complex-number
> comparisons (include/complex_types.h) and integer casts (_src/intn_numpy.h)
> call std::isnan on them: "no matching function for call to
> 'isnan(Eigen::half&)'". libstdc++ and newer libc++ accept it. Add the
> overloads for the two Eigen types, forwarding to Eigen::numext, the way
> ml_dtypes already provides std:: overloads for its own float8 types.

Touches: `ml_dtypes/_src/common.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-ml-dtypes; B=build/$QNX_ARCH/python3-ml-dtypes; mkdir -p $B
```

```bash
do_configure() {
  qnx_py_venv "scikit-build-core>=0.10" "numpy==2.3.4" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
}
```

```bash
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" \
    -Ccmake.define.CMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -Ccmake.define.Python_INCLUDE_DIR="$STAGE$PREFIX/include/python$QNX_PYV" \
    -Ccmake.define.Python_SOABI=cpython-${QNX_PYV/./} \
    -Ccmake.define.CMAKE_BUILD_TYPE=Release
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ml_dtypes: NumPy dtype extensions for machine learning (bfloat16, the float8 and float4/6 families, int2/int4/uint4), a C++ extension module built with Eigen (vendored in the source distribution) against the ported numpy. Built from the source distribution, which carries the Eigen copy the release was made with (the git tag has it as a submodule).
- scikit-build-core with the build venv's numpy for the build requirement, whose headers are replaced by the *target* numpy's (as for pandas: the generated _numpyconfig.h differs per platform).
- CMake is pointed at the QNX toolchain and the target's Python headers; FindPython would otherwise take the build machine's, and name the module with its SOABI (cpython-314-x86_64-linux-gnu) -- the target's is cpython-314.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-ml-dtypes`.

### 5. What it installs

10 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-ml-dtypes` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-ml-dtypes
ls dists/8.0/*/ | grep '^python3-ml-dtypes-'
```

This writes `python3-ml-dtypes-<ver>-r0.tar.gz` (run time) and `python3-ml-dtypes-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-ml-dtypes
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-ml-dtypes-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-ml-dtypes-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_ml_dtypes`](../../tests/src/) links python3-ml-dtypes the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-ml-dtypes`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-ml-dtypes
/usr/local/bin/qnx-port-tests/test_python3_ml_dtypes
```

```
== python3-ml-dtypes ==
  ok   ...
PASS python3-ml-dtypes
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_ml_dtypes` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_ml_dtypes`](../../tests/src/) -- a program linked against python3-ml-dtypes the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-ml-dtypes 0.6.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-ml-dtypes-0.6.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-ml-dtypes 0.6.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-ml-dtypes-0.6.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.6.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-ml-dtypes=<version>`), sources in `ported/python3-ml-dtypes/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-ml-dtypes`; port one with `scripts/port-update python3-ml-dtypes`.

<details><summary>recipe.sh</summary>

```bash
# ml_dtypes: NumPy dtype extensions for machine learning (bfloat16, the
# float8 and float4/6 families, int2/int4/uint4), a C++ extension module built
# with Eigen (vendored in the source distribution) against the ported numpy.
# Built from the source distribution, which carries the Eigen copy the
# release was made with (the git tag has it as a submodule).
VERSION=0.6.0
TARBALL=https://files.pythonhosted.org/packages/12/72/307d7c4bd0600601c7133fba5cb78af7db968152951c1cd473abb1cda782/ml_dtypes-0.6.0.tar.gz
SHA256=5e60251d32ced5598972e4d5e06a2f044341f9291402551a3f6f0ec44f9299b0
DEPS="numpy"
RUNTIME_DEPS="numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# scikit-build-core with the build venv's numpy for the build requirement,
# whose headers are replaced by the *target* numpy's (as for pandas: the
# generated _numpyconfig.h differs per platform).
do_configure() {
  qnx_py_venv "scikit-build-core>=0.10" "numpy==2.3.4" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
}
# CMake is pointed at the QNX toolchain and the target's Python headers;
# FindPython would otherwise take the build machine's, and name the module
# with its SOABI (cpython-314-x86_64-linux-gnu) -- the target's is
# cpython-314.
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" \
    -Ccmake.define.CMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -Ccmake.define.Python_INCLUDE_DIR="$STAGE$PREFIX/include/python$QNX_PYV" \
    -Ccmake.define.Python_SOABI=cpython-${QNX_PYV/./} \
    -Ccmake.define.CMAKE_BUILD_TYPE=Release
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
