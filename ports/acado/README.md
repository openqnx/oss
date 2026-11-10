# acado 1.2.2_beta

> ACADO Toolkit: optimal control, MPC/MHE and embedded solver code generation

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/acado/acado.git (tag `v1.2.2beta`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_acado`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `acado` (programs, shared libraries, run-time data) and `acado-dev` (headers, pkg-config/CMake files, static libraries; needs `acado`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`acado`](https://openqnx.github.io/oss/package/x86_64/acado.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/acado.html)) · [`acado-dev`](https://openqnx.github.io/oss/package/x86_64/acado-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/acado-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add acado` (dependencies come along); one version: `apk add acado=<version>` | `apk del acado` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install acado` (dependencies first) | `oss-get remove acado` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`acado-dev` the same way, by its own name (`apk add acado-dev`, `oss-get install acado-dev`, by hand with its own file and list `acado-dev.list`); remove it before `acado`, which it needs.

```sh
# on the QNX target, as root
apk add acado acado-dev
apk add qnx-test-acado && /usr/local/bin/qnx-port-tests/test_acado    # its on-target test, from tests-8.0-<arch>
apk del acado acado-dev qnx-test-acado
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/acado.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/acado.list
rm var/lib/manual/acado.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild acado` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.2.2beta --recurse-submodules --shallow-submodules \
    https://github.com/acado/acado.git src/acado
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/acado < ports/acado/patches/0001-no-march-native-cross.patch
patch -p1 -d src/acado < ports/acado/patches/0002-templates-installed-only.patch
patch -p1 -d src/acado < ports/acado/patches/0003-templates-in-share.patch
```

**[`0001-no-march-native-cross.patch`](patches/0001-no-march-native-cross.patch)** — Do not add -march=native when cross-compiling

> ACADO adds -march=native to the release flags of every GCC/Clang build.
> Cross-compiling, "native" is the build host, not the target: QNX's
> aarch64 compiler rejects it ("bad value 'native' for '-march='") and on
> x86_64 it would tune the target code for whatever CPU built it.

Touches: `cmake/CompilerOptions.cmake` · Upstream status: Pending

**[`0002-templates-installed-only.patch`](patches/0002-templates-installed-only.patch)** — Code generation: only the installed templates when cross-compiling

> The code generation tool looks for its templates in the source tree first,
> then in the installed include/acado/code_generation/templates, and the
> source tree's absolute path is compiled into libacado_toolkit_s. Cross-
> compiled, that is a path on the build host, which the target does not have
> (and a build-machine path in the shipped library): search the installed
> templates only.

Touches: `acado/code_generation/templates/CMakeLists.txt` · Upstream status: Pending

**[`0003-templates-in-share.patch`](patches/0003-templates-in-share.patch)** — Code generation: install the templates as data (share/acado/templates)

> The code generator reads its templates (*.in) at run time, from the
> installed include/acado/code_generation/templates. Installed with the
> headers, they are packaged with them, in acado-dev: on a target with only
> acado, exportCode() could not open acado_auxiliary_functions.h.in and
> wrote an incomplete solver. Install them as data, to share/acado/templates,
> and look for them there (templates.hpp, the header naming the directory,
> stays with the headers).

Touches: `acado/code_generation/templates/CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/acado -B build/$QNX_ARCH/acado -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_TESTING=OFF \
      -DACADO_WITH_EXAMPLES=OFF \
      -DACADO_WITH_TESTING=OFF
cmake --build build/$QNX_ARCH/acado -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/acado
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ACADO Toolkit: automatic control and dynamic optimisation -- optimal control (multiple shooting, SQP), parameter and state estimation, MPC, and the code generation tool that exports fast embedded C solvers for MPC/MHE (qpOASES-based). Autoware's MPC controller uses the exported solvers. The libraries (libacado_toolkit_s, libacado_casadi, qpOASES) and the CGT; the examples are not built: several of them run their own code generator at build time, which cannot run on the build host. patches/: -march=native is dropped when cross-compiling; the code generator looks for its templates in the installed tree only, and they are installed as data (share/acado/templates, in the run-time package).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH acado`.

### 5. What it installs

842 files under `/usr/local`: 3 shared-library entries, 757 headers.

```
/usr/local/lib/libacado_toolkit_s.so
/usr/local/lib/libacado_toolkit_s.so.1
/usr/local/lib/libacado_toolkit_s.so.1.2.2beta
```

The complete list is `stage/<arch>/.manifest/acado` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa acado
ls dists/8.0/*/ | grep '^acado-'
```

This writes `acado-<ver>-r0.tar.gz` (run time) and `acado-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add acado
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/acado-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/acado-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_acado`](../../tests/src/) links acado the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-acado`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-acado
/usr/local/bin/qnx-port-tests/test_acado
```

```
== acado ==
  ok   ...
PASS acado
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_acado` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_acado`](../../tests/src/) -- a program linked against acado the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - OCPexport::exportCode of a double-integrator MPC
   - acado_solver.c written
   - acado_common.h defines ACADO_N 20 and ACADO_NX 2
   - the qpOASES interface was exported from the installed templates

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | acado 1.2.2_beta-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/acado-1.2.2_beta-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | acado 1.2.2_beta-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/acado-1.2.2_beta-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.2_beta**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add acado=<version>`), sources in `ported/acado/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates acado`; port one with `scripts/port-update acado`.

<details><summary>recipe.sh</summary>

```bash
# ACADO Toolkit: automatic control and dynamic optimisation -- optimal
# control (multiple shooting, SQP), parameter and state estimation, MPC, and
# the code generation tool that exports fast embedded C solvers for MPC/MHE
# (qpOASES-based). Autoware's MPC controller uses the exported solvers.
# The libraries (libacado_toolkit_s, libacado_casadi, qpOASES) and the CGT;
# the examples are not built: several of them run their own code generator
# at build time, which cannot run on the build host.
# patches/: -march=native is dropped when cross-compiling; the code
# generator looks for its templates in the installed tree only, and they are
# installed as data (share/acado/templates, in the run-time package).
VERSION=1.2.2_beta; GIT=https://github.com/acado/acado.git; REF=v1.2.2beta
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DACADO_WITH_EXAMPLES=OFF -DACADO_WITH_TESTING=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
