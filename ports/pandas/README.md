# pandas 2.3.3

> Python data analysis library with labeled data structures

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pandas-dev/pandas.git (tag `v2.3.3`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`numpy`](../numpy/) [`python3-dateutil`](../python3-dateutil/) [`python3-pytz`](../python3-pytz/) [`python3-tzdata`](../python3-tzdata/) |
| Run-time dependencies | [`python3`](../python3/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pandas`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pandas
apk add pandas-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pandas && /usr/local/bin/qnx-port-tests/test_pandas    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pandas`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pandas` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (21 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild numpy python3-dateutil python3-pytz python3-tzdata
```

<details><summary>Full build chain (21)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-dateutil`](../python3-dateutil/) [`python3-pytz`](../python3-pytz/) [`python3-six`](../python3-six/) [`python3-tzdata`](../python3-tzdata/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.3.3 --recurse-submodules --shallow-submodules \
    https://github.com/pandas-dev/pandas.git src/pandas
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pandas; B=build/$QNX_ARCH/pandas; mkdir -p $B
```

```bash
do_configure() {
  qnx_py_venv "meson-python>=0.13.1" "meson>=1.2.1" "Cython~=3.0.5" "versioneer[toml]" "numpy==2.3.4" "patchelf>=0.11.0" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
  qnx_py_crossfile
}
```

```bash
do_build() { qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini"; }
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pandas 2.3.3: the first release line that supports Python 3.14 (the target's). Cross built like numpy (scripts/lib/pywheel.sh). pandas finds numpy's C headers by asking the build-host numpy for get_include(), and numpy's generated _numpyconfig.h differs per platform (long double size), so the host numpy in the build venv is given the *target* numpy's headers. qnx-ports' pandas change only touches setup.py, which the Meson build does not use -- no patch is needed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pandas`.

### 5. What it installs

1937 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/pandas` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pandas
ls dists/8.0/*/ | grep '^pandas-'
```

This writes `pandas-<ver>-r0.tar.gz` (run time) and `pandas-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pandas
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pandas-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pandas-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pandas`](../../tests/src/) links pandas the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pandas`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pandas
/usr/local/bin/qnx-port-tests/test_pandas
```

```
== pandas ==
  ok   ...
PASS pandas
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pandas` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pandas`](../../tests/src/) -- a program linked against pandas the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pandas 2.3.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pandas-2.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pandas 2.3.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pandas-2.3.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pandas=<version>`), sources in `ported/pandas/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pandas`; port one with `scripts/port-update pandas`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.3; GIT=https://github.com/pandas-dev/pandas.git; REF=v2.3.3
DEPS="numpy python3-dateutil python3-pytz python3-tzdata"
BUILDSYS=custom
# pandas 2.3.3: the first release line that supports Python 3.14 (the
# target's). Cross built like numpy (scripts/lib/pywheel.sh). pandas finds
# numpy's C headers by asking the build-host numpy for get_include(), and
# numpy's generated _numpyconfig.h differs per platform (long double size), so
# the host numpy in the build venv is given the *target* numpy's headers.
# qnx-ports' pandas change only touches setup.py, which the Meson build does
# not use -- no patch is needed.
source "$WS/scripts/lib/pywheel.sh"
do_configure() {
  qnx_py_venv "meson-python>=0.13.1" "meson>=1.2.1" "Cython~=3.0.5" "versioneer[toml]" "numpy==2.3.4" "patchelf>=0.11.0" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
  qnx_py_crossfile
}
do_build() { qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini"; }
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
