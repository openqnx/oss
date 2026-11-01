# log4cxx 1.8.0

> C++ logging framework modeled after Apache log4j

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/apache/logging-log4cxx.git (tag `rel/v1.8.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`apr`](../apr/) [`apr-util`](../apr-util/) [`expat`](../expat/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`apr`](../apr/) [`apr-util`](../apr-util/) [`expat`](../expat/) [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_log4cxx`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `log4cxx` (programs, shared libraries, run-time data) and `log4cxx-dev` (headers, pkg-config/CMake files, static libraries; needs `log4cxx`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`log4cxx`](https://openqnx.github.io/oss/package/x86_64/log4cxx.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/log4cxx.html)) · [`log4cxx-dev`](https://openqnx.github.io/oss/package/x86_64/log4cxx-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/log4cxx-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add log4cxx` (dependencies come along); one version: `apk add log4cxx=<version>` | `apk del log4cxx` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install log4cxx` (dependencies first) | `oss-get remove log4cxx` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`log4cxx-dev` the same way, by its own name (`apk add log4cxx-dev`, `oss-get install log4cxx-dev`, by hand with its own file and list `log4cxx-dev.list`); remove it before `log4cxx`, which it needs.

```sh
# on the QNX target, as root
apk add log4cxx log4cxx-dev
apk add qnx-test-log4cxx && /usr/local/bin/qnx-port-tests/test_log4cxx    # its on-target test, from tests-8.0-<arch>
apk del log4cxx log4cxx-dev qnx-test-log4cxx
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/log4cxx.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/log4cxx.list
rm var/lib/manual/log4cxx.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild log4cxx` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild apr apr-util expat libiconv
```

<details><summary>Full build chain (8)</summary>

[`apr`](../apr/) [`apr-util`](../apr-util/) [`expat`](../expat/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`sqlite`](../sqlite/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b rel/v1.8.0 --recurse-submodules --shallow-submodules \
    https://github.com/apache/logging-log4cxx.git src/log4cxx
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/log4cxx -B build/$QNX_ARCH/log4cxx -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON \
      -DLOG4CXX_ENABLE_ODBC=OFF \
      -DLOG4CXX_ENABLE_ESMTP=OFF \
      -DLOG4CXX_QT_SUPPORT=OFF
cmake --build build/$QNX_ARCH/log4cxx -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/log4cxx
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Apache log4cxx on APR and APR-util (found through their pkg-config files), with XML (DOMConfigurator, via APR-util's expat) and socket appenders. The optional fmt layout is left out so the library does not pick up the workspace's fmt unasked; no ODBC, SMTP or Qt. Tests and examples off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH log4cxx`.

### 5. What it installs

212 files under `/usr/local`: 3 shared-library entries, 204 headers.

```
/usr/local/lib/liblog4cxx.so
/usr/local/lib/liblog4cxx.so.15
/usr/local/lib/liblog4cxx.so.15.8.0
```

The complete list is `stage/<arch>/.manifest/log4cxx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa log4cxx
ls dists/8.0/*/ | grep '^log4cxx-'
```

This writes `log4cxx-<ver>-r0.tar.gz` (run time) and `log4cxx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add log4cxx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/log4cxx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/log4cxx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_log4cxx`](../../tests/src/) links log4cxx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-log4cxx`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-log4cxx
/usr/local/bin/qnx-port-tests/test_log4cxx
```

```
== log4cxx ==
  ok   ...
PASS log4cxx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_log4cxx` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_log4cxx`](../../tests/src/) -- a program linked against log4cxx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - root level INFO: info enabled, debug not
   - properties config: levels, inheritance, MDC and NDC in the file
   - additivity=false: app.own only in its own appender
   - XML config (DOMConfigurator): root level debug
   - RollingFileAppender rolled over at ~1000 bytes into roll.1.log
   - the last line is in the current file

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | log4cxx 1.8.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/log4cxx-1.8.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | log4cxx 1.8.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/log4cxx-1.8.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.8.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add log4cxx=<version>`), sources in `ported/log4cxx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates log4cxx`; port one with `scripts/port-update log4cxx`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.8.0; GIT=https://github.com/apache/logging-log4cxx.git; REF=rel/v1.8.0
DEPS="apr apr-util expat libiconv"
# Apache log4cxx on APR and APR-util (found through their pkg-config files),
# with XML (DOMConfigurator, via APR-util's expat) and socket appenders. The
# optional fmt layout is left out so the library does not pick up the
# workspace's fmt unasked; no ODBC, SMTP or Qt. Tests and examples off.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON -DLOG4CXX_ENABLE_ODBC=OFF
                   -DLOG4CXX_ENABLE_ESMTP=OFF -DLOG4CXX_QT_SUPPORT=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
