# quickjs-ng 0.8.0

> Small and embeddable JavaScript engine

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/quickjs-ng/quickjs.git (tag `v0.8.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_quickjs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `quickjs-ng` (programs, shared libraries, run-time data) and `quickjs-ng-dev` (headers, pkg-config/CMake files, static libraries; needs `quickjs-ng`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`quickjs-ng`](https://openqnx.github.io/oss/package/x86_64/quickjs-ng.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/quickjs-ng.html)) · [`quickjs-ng-dev`](https://openqnx.github.io/oss/package/x86_64/quickjs-ng-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/quickjs-ng-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add quickjs-ng` (dependencies come along); one version: `apk add quickjs-ng=<version>` | `apk del quickjs-ng` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install quickjs-ng` (dependencies first) | `oss-get remove quickjs-ng` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`quickjs-ng-dev` the same way, by its own name (`apk add quickjs-ng-dev`, `oss-get install quickjs-ng-dev`, by hand with its own file and list `quickjs-ng-dev.list`); remove it before `quickjs-ng`, which it needs.

```sh
# on the QNX target, as root
apk add quickjs-ng quickjs-ng-dev
apk add qnx-test-quickjs && /usr/local/bin/qnx-port-tests/test_quickjs    # its on-target test
apk del quickjs-ng quickjs-ng-dev qnx-test-quickjs
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/quickjs-ng.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/quickjs-ng.list
rm var/lib/manual/quickjs-ng.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild quickjs-ng` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.8.0 --recurse-submodules --shallow-submodules \
    https://github.com/quickjs-ng/quickjs.git src/quickjs-ng
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/quickjs-ng < ports/quickjs-ng/patches/0001-qnx-sighandler-t-and-test262.patch
```

**[`0001-qnx-sighandler-t-and-test262.patch`](patches/0001-qnx-sighandler-t-and-test262.patch)** — Build on QNX: sighandler_t and the test262 runner.

> - sighandler_t is a glibc name and QNX has neither it nor the BSD sig_t;
>   declare the plain function-pointer type.
> - run-test262 is a conformance-suite driver, not part of the engine, and
>   relies on struct dirent's d_type, which QNX does not have. Skip it.
> - test_conv (an internal unit test) trips a GCC 12 aarch64 false positive
>   under -Werror; it is not built on QNX.

Touches: `quickjs-libc.c`, `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/quickjs-ng -B build/$QNX_ARCH/quickjs-ng -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DQJS_BUILD_EXAMPLES=OFF
cmake --build build/$QNX_ARCH/quickjs-ng -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/quickjs-ng
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH quickjs-ng`.

### 5. What it installs

17 files under `/usr/local`: 3 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libqjs.so
/usr/local/lib/libqjs.so.0
/usr/local/lib/libqjs.so.0.8.0
/usr/local/bin/qjs
/usr/local/bin/qjsc
```

The complete list is `stage/<arch>/.manifest/quickjs-ng` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa quickjs-ng
ls dists/8.0/*/ | grep '^quickjs-ng-'
```

This writes `quickjs-ng-<ver>-r0.tar.gz` (run time) and `quickjs-ng-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add quickjs-ng
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/quickjs-ng-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/quickjs-ng-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_quickjs`](../../tests/src/) links quickjs-ng the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-quickjs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-quickjs
/usr/local/bin/qnx-port-tests/test_quickjs
```

```
== quickjs-ng ==
  ok   ...
PASS quickjs-ng
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_quickjs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_quickjs`](../../tests/src/) -- a program linked against quickjs-ng the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - arrow functions, map/reduce
   - JSON

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | quickjs-ng 0.8.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/quickjs-ng-0.8.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | quickjs-ng 0.8.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/quickjs-ng-0.8.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.8.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add quickjs-ng=<version>`), sources in `ported/quickjs-ng/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates quickjs-ng`; port one with `scripts/port-update quickjs-ng`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.8.0; GIT=https://github.com/quickjs-ng/quickjs.git; REF=v0.8.0
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DQJS_BUILD_EXAMPLES=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
