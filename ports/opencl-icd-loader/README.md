# opencl-icd-loader 2026.05.29

> Khronos OpenCL ICD loader (libOpenCL.so): dispatches OpenCL calls to the installed implementations

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/KhronosGroup/OpenCL-ICD-Loader.git (tag `v2026.05.29`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`opencl-headers`](../opencl-headers/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_opencl_icd_loader`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `opencl-icd-loader` (programs, shared libraries, run-time data) and `opencl-icd-loader-dev` (headers, pkg-config/CMake files, static libraries; needs `opencl-icd-loader`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`opencl-icd-loader`](https://openqnx.github.io/oss/package/x86_64/opencl-icd-loader.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/opencl-icd-loader.html)) · [`opencl-icd-loader-dev`](https://openqnx.github.io/oss/package/x86_64/opencl-icd-loader-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/opencl-icd-loader-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add opencl-icd-loader` (dependencies come along); one version: `apk add opencl-icd-loader=<version>` | `apk del opencl-icd-loader` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install opencl-icd-loader` (dependencies first) | `oss-get remove opencl-icd-loader` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`opencl-icd-loader-dev` the same way, by its own name (`apk add opencl-icd-loader-dev`, `oss-get install opencl-icd-loader-dev`, by hand with its own file and list `opencl-icd-loader-dev.list`); remove it before `opencl-icd-loader`, which it needs.

```sh
# on the QNX target, as root
apk add opencl-icd-loader opencl-icd-loader-dev
apk add qnx-test-opencl-icd-loader && /usr/local/bin/qnx-port-tests/test_opencl_icd_loader    # its on-target test, from tests-8.0-<arch>-m-z
apk del opencl-icd-loader opencl-icd-loader-dev qnx-test-opencl-icd-loader
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/opencl-icd-loader.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/opencl-icd-loader.list
rm var/lib/manual/opencl-icd-loader.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild opencl-icd-loader` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild opencl-headers
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2026.05.29 --recurse-submodules --shallow-submodules \
    https://github.com/KhronosGroup/OpenCL-ICD-Loader.git src/opencl-icd-loader
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/opencl-icd-loader < ports/opencl-icd-loader/patches/0001-extra-config-dir.patch
```

**[`0001-extra-config-dir.patch`](patches/0001-extra-config-dir.patch)** — Read ICD and layer files from the installation prefix's etc/OpenCL as well as /etc/OpenCL.

> The loader reads its ICD files (*.icd) from /etc/OpenCL/vendors and its
> layers from /etc/OpenCL/layers only. Components here are installed under a
> prefix of their own (/usr/local), and the base system's /etc is not where
> their packages put configuration: an OpenCL implementation installed from a
> package registers itself in <prefix>/etc/OpenCL/vendors, which the loader
> never looked at, so clGetPlatformIDs() found no platform.
>
> OPENCL_ICD_LOADER_EXTRA_CONFIG_DIR (a CMake cache entry, empty by default:
> upstream behaviour) names a second configuration directory. Its vendors/ is
> enumerated before /etc/OpenCL/vendors and its layers/ after
> /etc/OpenCL/layers; an ICD registered in both is loaded once (the loader
> already skips a library it has loaded). OCL_ICD_VENDORS and OPENCL_LAYER_PATH
> keep their meaning: when set they replace both directories.

Touches: `CMakeLists.txt`, `loader/linux/icd_linux.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/opencl-icd-loader; B=build/$QNX_ARCH/opencl-icd-loader; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  local d="$STAGE$PREFIX/libexec/opencl-icd-loader"
  install -d "$d"
  install -m755 "$B/icd_loader_test" "$B/libIcdLog.so" "$B/libOpenCLDriverStub.so" "$B/libOpenCLDriverStubICD2.so" \
                "$B/test/layer/libPrintLayer.so" "$d/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- opencl-icd-loader: the Khronos OpenCL ICD loader, libOpenCL.so.1 -- the library OpenCL programs link. It implements no OpenCL itself: at the first call it reads the *.icd files of the installed implementations (each names a driver library, e.g. pocl's), loads them and dispatches every call to the platform it belongs to. With no implementation installed clGetPlatformIDs() returns CL_PLATFORM_NOT_FOUND_KHR (-1001). Also cllayerinfo, which lists the installed OpenCL layers. Upstream supports QNX (it builds its "linux" back end there: dlopen(), readdir(), and getenv() ignored in set-id programs). What is changed: - ICD files are looked up in /usr/local/etc/OpenCL/vendors first, then in /etc/OpenCL/vendors (layers: /etc/OpenCL/layers, then /usr/local/etc/OpenCL/layers): the components here install under /usr/local and never write to the base system's /etc (patches/0001, OPENCL_ICD_LOADER_EXTRA_CONFIG_DIR). OCL_ICD_VENDORS=<dir> replaces both directories, OCL_ICD_FILENAMES=<lib>[:<lib>] adds drivers by name, OCL_ICD_ENABLE_TRACE=1 prints what the loader does. - the headers are opencl-headers' (same release), not a copy in inc/. - upstream's test programs are built and installed in libexec/opencl-icd-loader/: a stub driver (libOpenCLDriverStub.so, every entry point logs its arguments), its ICD 2 variant, a layer (libPrintLayer.so) and icd_loader_test, which calls the whole API through the loader and compares what the application passed with what the driver received. They are how the loader is tested on a target that has no OpenCL implementation (tests/src/test_opencl_icd_loader.c). They carry no run path (CMake would write the build directory into them): run them with that directory in LD_LIBRARY_PATH, for libIcdLog.so.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH opencl-icd-loader`.

### 5. What it installs

14 files under `/usr/local`: 3 shared-library entries, 1 programs.

```
/usr/local/lib/libOpenCL.so
/usr/local/lib/libOpenCL.so.1
/usr/local/lib/libOpenCL.so.1.0.0
/usr/local/bin/cllayerinfo
```

The complete list is `stage/<arch>/.manifest/opencl-icd-loader` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa opencl-icd-loader
ls dists/8.0/*/ | grep '^opencl-icd-loader-'
```

This writes `opencl-icd-loader-<ver>-r0.tar.gz` (run time) and `opencl-icd-loader-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add opencl-icd-loader
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/opencl-icd-loader-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/opencl-icd-loader-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_opencl_icd_loader`](../../tests/src/) links opencl-icd-loader the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-opencl-icd-loader`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-opencl-icd-loader
/usr/local/bin/qnx-port-tests/test_opencl_icd_loader
```

```
== opencl-icd-loader ==
  ok   ...
PASS opencl-icd-loader
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_opencl_icd_loader` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_opencl_icd_loader`](../../tests/src/) -- a program linked against opencl-icd-loader the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - the stub drivers' log (libIcdLog.so) is open
   - no driver: clGetPlatformIDs returns CL_PLATFORM_NOT_FOUND_KHR (-1001)
   - no driver: 0 platforms
   - no driver: the same when asking for the list
   - <prefix>/etc/OpenCL/vendors: clGetPlatformIDs finds a platform
   - <prefix>/etc/OpenCL/vendors: the stub driver's platform is among them, once
   - ICD file and OCL_ICD_FILENAMES: 2 platforms
   - two different platform ids
   - the OCL_ICD_FILENAMES driver comes first, then the ICD file's
   - each platform's calls reach its own driver
   - clGetPlatformIDs returns its id
   - CL_PLATFORM_VENDOR is stubvendorxxx
   - CL_PLATFORM_VERSION is OpenCL 1.2 Stub
   - CL_PLATFORM_ICD_SUFFIX_KHR is ilts
   - the size query gives the name's length + 1
   - a buffer too small: CL_INVALID_VALUE
   - clGetDeviceIDs is dispatched to the driver: 1 device
   - the stub drivers are installed (libexec/opencl-icd-loader)
   - scratch directory
   - an ICD file naming the stub driver is written
   - case: no driver
   - case: ICD file in the OCL_ICD_VENDORS directory
   - the driver's log has the clGetDeviceIDs call with the arguments passed (CL_DEVICE_TYPE_ALL, 1 entry)
   - case: ICD 2 driver named in OCL_ICD_FILENAMES
   - case: ICD file and OCL_ICD_FILENAMES together
   - ... and 6 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | opencl-icd-loader 2026.05.29-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/opencl-icd-loader-2026.05.29-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | opencl-icd-loader 2026.05.29-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/opencl-icd-loader-2026.05.29-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2026.05.29**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add opencl-icd-loader=<version>`), sources in `ported/opencl-icd-loader/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates opencl-icd-loader`; port one with `scripts/port-update opencl-icd-loader`.

<details><summary>recipe.sh</summary>

```bash
# opencl-icd-loader: the Khronos OpenCL ICD loader, libOpenCL.so.1 -- the
# library OpenCL programs link. It implements no OpenCL itself: at the first
# call it reads the *.icd files of the installed implementations (each names
# a driver library, e.g. pocl's), loads them and dispatches every call to
# the platform it belongs to. With no implementation installed
# clGetPlatformIDs() returns CL_PLATFORM_NOT_FOUND_KHR (-1001). Also
# cllayerinfo, which lists the installed OpenCL layers.
#
# Upstream supports QNX (it builds its "linux" back end there: dlopen(),
# readdir(), and getenv() ignored in set-id programs). What is changed:
#  - ICD files are looked up in /usr/local/etc/OpenCL/vendors first, then in
#    /etc/OpenCL/vendors (layers: /etc/OpenCL/layers, then
#    /usr/local/etc/OpenCL/layers): the components here install under
#    /usr/local and never write to the base system's /etc
#    (patches/0001, OPENCL_ICD_LOADER_EXTRA_CONFIG_DIR). OCL_ICD_VENDORS=<dir>
#    replaces both directories, OCL_ICD_FILENAMES=<lib>[:<lib>] adds drivers
#    by name, OCL_ICD_ENABLE_TRACE=1 prints what the loader does.
#  - the headers are opencl-headers' (same release), not a copy in inc/.
#  - upstream's test programs are built and installed in
#    libexec/opencl-icd-loader/: a stub driver (libOpenCLDriverStub.so, every
#    entry point logs its arguments), its ICD 2 variant, a layer
#    (libPrintLayer.so) and icd_loader_test, which calls the whole API through
#    the loader and compares what the application passed with what the driver
#    received. They are how the loader is tested on a target that has no
#    OpenCL implementation (tests/src/test_opencl_icd_loader.c). They carry
#    no run path (CMake would write the build directory into them): run them
#    with that directory in LD_LIBRARY_PATH, for libIcdLog.so.
VERSION=2026.05.29; GIT=https://github.com/KhronosGroup/OpenCL-ICD-Loader.git; REF=v2026.05.29
DEPS="opencl-headers"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DOPENCL_ICD_LOADER_EXTRA_CONFIG_DIR="$PREFIX/etc/OpenCL"
                   -DBUILD_TESTING=ON -DOPENCL_ICD_LOADER_BUILD_TESTING=ON
                   -DCMAKE_SKIP_RPATH=ON )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  local d="$STAGE$PREFIX/libexec/opencl-icd-loader"
  install -d "$d"
  install -m755 "$B/icd_loader_test" "$B/libIcdLog.so" "$B/libOpenCLDriverStub.so" "$B/libOpenCLDriverStubICD2.so" \
                "$B/test/layer/libPrintLayer.so" "$d/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
