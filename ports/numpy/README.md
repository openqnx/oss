# numpy 2.3.4

> Fundamental package for array computing in Python

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/numpy/numpy.git (tag `v2.3.4`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`openblas`](../openblas/) |
| Run-time dependencies | [`openblas`](../openblas/) [`python3`](../python3/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_numpy`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `numpy` (programs, shared libraries, run-time data) and `numpy-dev` (headers, pkg-config/CMake files, static libraries; needs `numpy`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`numpy`](https://openqnx.github.io/oss/package/x86_64/numpy.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/numpy.html)) · [`numpy-dev`](https://openqnx.github.io/oss/package/x86_64/numpy-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/numpy-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add numpy` (dependencies come along); one version: `apk add numpy=<version>` | `apk del numpy` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install numpy` (dependencies first) | `oss-get remove numpy` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`numpy-dev` the same way, by its own name (`apk add numpy-dev`, `oss-get install numpy-dev`, by hand with its own file and list `numpy-dev.list`); remove it before `numpy`, which it needs.

```sh
# on the QNX target, as root
apk add numpy numpy-dev
apk add qnx-test-numpy && /usr/local/bin/qnx-port-tests/test_numpy    # its on-target test
apk del numpy numpy-dev qnx-test-numpy
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/numpy.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/numpy.list
rm var/lib/manual/numpy.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa numpy
ls dists/8.0/*/ | grep '^numpy-'
```

This writes `numpy-<ver>-r0.tar.gz` (run time) and `numpy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add numpy
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/numpy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/numpy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_numpy`](../../tests/src/) links numpy the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-numpy`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-numpy
/usr/local/bin/qnx-port-tests/test_numpy
```

```
== numpy ==
  ok   ...
PASS numpy
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_numpy` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_numpy`](../../tests/src/) -- a program linked against numpy the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | numpy 2.3.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/numpy-2.3.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | numpy 2.3.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/numpy-2.3.4-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | numpy 2.3.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/numpy-2.3.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | numpy 2.3.4-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/numpy-2.3.4-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add numpy=<version>`), sources in `ported/numpy/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates numpy`; port one with `scripts/port-update numpy`.

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
