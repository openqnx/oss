# numpy 2.3.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/numpy/numpy.git (tag `v2.3.4`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`openblas`](../openblas/) |
| Run-time dependencies | [`openblas`](../openblas/) [`python3`](../python3/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install numpy
oss-get install numpy-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild numpy` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (16 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 openblas
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.3.4 --recurse-submodules --shallow-submodules \
    https://github.com/numpy/numpy.git src/numpy
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/numpy < ports/numpy/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port QNX changes made to v1.25.0

> Imported from qnx-ports/numpy (v2.3.4...qnx_v2.3.4); original authorship kept.
>
>
> ---
>  numpy/_core/include/numpy/npy_endian.h |   4 +
>  numpy/distutils/ccompiler.py           |   5 +
>  numpy/distutils/command/build_clib.py  |   5 +
>  numpy/distutils/command/build_ext.py   |  11 +
>  numpy/distutils/command/config.py      |   7 +
>  numpy/distutils/cpuinfo.py             | 340 +++++++++++++++++++++++++
>  numpy/distutils/system_info.py         |  27 +-
>  numpy/distutils/unixccompiler.py       |   7 +-
>  numpy/tests/test_public_api.py         |  14 +-
>  9 files changed, 403 insertions(+), 17 deletions(-)

Touches: `numpy/_core/include/numpy/npy_endian.h`, `numpy/distutils/ccompiler.py`, `numpy/distutils/command/build_clib.py`, `numpy/distutils/command/build_ext.py`, `numpy/distutils/command/config.py`, `numpy/distutils/cpuinfo.py` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/numpy; B=build/$QNX_ARCH/numpy; mkdir -p $B
```

```bash
do_configure() {
  qnx_py_venv "meson-python>=0.18" "Cython>=3.0.6" "meson>=1.5" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile "longdouble_format = '$([ "$QNX_ARCH" = aarch64le ] && echo IEEE_QUAD_LE || echo INTEL_EXTENDED_16_BYTES_LE)'"
}
```

```bash
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dallow-noblas=false" -Csetup-args="-Dblas=openblas" -Csetup-args="-Dlapack=openblas"
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- A Python extension package, cross compiled the way qnx-ports does it: - the build runs in a venv of the *host* Python of the same version as the target's (hosttools/bin/python3.14), with numpy's build requirements; - _PYTHON_SYSCONFIGDATA_NAME points it at the target's sysconfig data (staged by the python3 recipe), so extension names and flags are QNX's; - meson-python gets a cross file for the QNX compiler and the long double format of each architecture (quad on aarch64, x87 80-bit on x86_64). The wheel is then unpacked into the target's site-packages in staging. The steps are shared by every Python package: scripts/lib/pywheel.sh.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH numpy`.

### 5. What it installs

897 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/numpy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH numpy
ls repo/8.0/*/ | grep '^numpy-'
```

This writes `numpy-<ver>-r0.tar.gz` (run time) and `numpy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install numpy
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/numpy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/numpy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.4; GIT=https://github.com/numpy/numpy.git; REF=v2.3.4
DEPS="python3 openblas"
BUILDSYS=custom
# A Python extension package, cross compiled the way qnx-ports does it:
#  - the build runs in a venv of the *host* Python of the same version as the
#    target's (hosttools/bin/python3.14), with numpy's build requirements;
#  - _PYTHON_SYSCONFIGDATA_NAME points it at the target's sysconfig data
#    (staged by the python3 recipe), so extension names and flags are QNX's;
#  - meson-python gets a cross file for the QNX compiler and the long double
#    format of each architecture (quad on aarch64, x87 80-bit on x86_64).
# The wheel is then unpacked into the target's site-packages in staging.
# The steps are shared by every Python package: scripts/lib/pywheel.sh.
source "$WS/scripts/lib/pywheel.sh"
do_configure() {
  qnx_py_venv "meson-python>=0.18" "Cython>=3.0.6" "meson>=1.5" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile "longdouble_format = '$([ "$QNX_ARCH" = aarch64le ] && echo IEEE_QUAD_LE || echo INTEL_EXTENDED_16_BYTES_LE)'"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dallow-noblas=false" -Csetup-args="-Dblas=openblas" -Csetup-args="-Dlapack=openblas"
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
