# pythran 0.19.0

> Ahead-of-time compiler of Python/numpy kernels to C++ extension modules

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/bb/a3/03098a8a5d2c9f801c227cf941d85976934f5f47307ef3d819734370acdc/pythran-0.19.0.tar.gz |
| SHA-256 | `18a096daa8b4bbe3ace06ce5d7325be5ff57aec4cf67b91834cf1b757b3e5392` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`python3-ply`](../python3-ply/) [`python3-setuptools`](../python3-setuptools/) [`python3-gast`](../python3-gast/) [`python3-beniget`](../python3-beniget/) [`numpy`](../numpy/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_buildtools`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `pythran`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pythran`](https://openqnx.github.io/oss/package/x86_64/pythran.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pythran.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pythran` (dependencies come along); one version: `apk add pythran=<version>` | `apk del pythran` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pythran` (dependencies first) | `oss-get remove pythran` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-buildtools && /usr/local/bin/qnx-port-tests/test_python3_buildtools    # its on-target test, from tests-8.0-<arch>-m-z
apk del pythran qnx-test-python3-buildtools
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pythran.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pythran.list
rm var/lib/manual/pythran.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pythran` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild python3 python3-ply python3-setuptools python3-gast python3-beniget numpy
```

<details><summary>Full build chain (21)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-beniget`](../python3-beniget/) [`python3-gast`](../python3-gast/) [`python3-ply`](../python3-ply/) [`python3-setuptools`](../python3-setuptools/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/bb/a3/03098a8a5d2c9f801c227cf941d85976934f5f47307ef3d819734370acdc/pythran-0.19.0.tar.gz
mkdir -p dist src/pythran
curl -fL -o dist/$(basename $URL) $URL
echo "18a096daa8b4bbe3ace06ce5d7325be5ff57aec4cf67b91834cf1b757b3e5392  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pythran --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pythran < ports/pythran/patches/0001-qnx-omp-soname.patch
```

**[`0001-qnx-omp-soname.patch`](patches/0001-qnx-omp-soname.patch)** — Look up OpenMP by soname on QNX instead of spawning ldconfig and ld

> The bundled omp module called ctypes.util.find_library() for omp, gomp
> and iomp5 on every import attempt; on QNX that spawns ldconfig, gcc and
> ld, none of which a target has (144 failed spawns, about 50 s of each
> "import pythran" on an x86_64 VM). The runtime loader is asked directly.

Touches: `omp/__init__.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pythran; B=build/$QNX_ARCH/pythran; mkdir -p $B
```

```bash
do_build() { qnx_py_wheel_pure; }
```

```bash
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Pythran: an ahead-of-time compiler from a subset of Python with numpy to C++ extension modules (scipy uses it). The compiler and its C++ headers run on the target; compiling there needs a C++ compiler on the target. Pure Python, built from the source distribution (scripts/lib/pywheel.sh) so that patch 0001 applies: its bundled "omp" module looked for an OpenMP library with ctypes.util.find_library() on every import attempt, which on QNX spawns ldconfig and ld (neither exists) -- 144 failed spawns, 50 s of every "import pythran" on an x86_64 VM, far longer on an emulated aarch64. Its console scripts are written as launchers in bin/ (qnx_py_scripts).
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pythran`.

### 5. What it installs

4143 files under `/usr/local`, 2 programs.

```
/usr/local/bin/pythran
/usr/local/bin/pythran-config
```

The complete list is `stage/<arch>/.manifest/pythran` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pythran
ls dists/8.0/*/ | grep '^pythran-'
```

This writes `pythran-<ver>-r0.tar.gz` (run time) and `pythran-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pythran
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pythran-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pythran-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_buildtools`](../../tests/src/) links pythran the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-buildtools`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-buildtools
/usr/local/bin/qnx-port-tests/test_python3_buildtools
```

```
== pythran ==
  ok   ...
PASS pythran
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_buildtools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_buildtools`](../../tests/src/) -- a program linked against pythran the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gyp 0.22.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/gyp-0.22.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pythran 0.19.0-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/pythran-0.19.0-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | scons 4.11.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/scons-4.11.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gyp 0.22.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/gyp-0.22.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pythran 0.19.0-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/pythran-0.19.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | scons 4.11.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/scons-4.11.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.19.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pythran=<version>`), sources in `ported/pythran/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pythran`; port one with `scripts/port-update pythran`.

<details><summary>recipe.sh</summary>

```bash
# Pythran: an ahead-of-time compiler from a subset of Python with numpy to
# C++ extension modules (scipy uses it). The compiler and its C++ headers
# run on the target; compiling there needs a C++ compiler on the target.
# Pure Python, built from the source distribution (scripts/lib/pywheel.sh)
# so that patch 0001 applies: its bundled "omp" module looked for an OpenMP
# library with ctypes.util.find_library() on every import attempt, which on
# QNX spawns ldconfig and ld (neither exists) -- 144 failed spawns, 50 s of
# every "import pythran" on an x86_64 VM, far longer on an emulated aarch64.
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=0.19.0
TARBALL=https://files.pythonhosted.org/packages/bb/a3/03098a8a5d2c9f801c227cf941d85976934f5f47307ef3d819734370acdc/pythran-0.19.0.tar.gz
SHA256=18a096daa8b4bbe3ace06ce5d7325be5ff57aec4cf67b91834cf1b757b3e5392
DEPS="python3 python3-ply python3-setuptools python3-gast python3-beniget numpy"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-ply python3-setuptools python3-gast python3-beniget numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
