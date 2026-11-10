# log4cpp 1.1.4

> C++ logging library modelled on log4j

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/log4cpp/log4cpp-1.1.4.tar.gz |
| SHA-256 | `696113659e426540625274a8b251052cc04306d8ee5c42a0c7639f39ca90c9d6` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_log4cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `log4cpp` (programs, shared libraries, run-time data) and `log4cpp-dev` (headers, pkg-config/CMake files, static libraries; needs `log4cpp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`log4cpp`](https://openqnx.github.io/oss/package/x86_64/log4cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/log4cpp.html)) · [`log4cpp-dev`](https://openqnx.github.io/oss/package/x86_64/log4cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/log4cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add log4cpp` (dependencies come along); one version: `apk add log4cpp=<version>` | `apk del log4cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install log4cpp` (dependencies first) | `oss-get remove log4cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`log4cpp-dev` the same way, by its own name (`apk add log4cpp-dev`, `oss-get install log4cpp-dev`, by hand with its own file and list `log4cpp-dev.list`); remove it before `log4cpp`, which it needs.

```sh
# on the QNX target, as root
apk add log4cpp log4cpp-dev
apk add qnx-test-log4cpp && /usr/local/bin/qnx-port-tests/test_log4cpp    # its on-target test, from tests-8.0-<arch>
apk del log4cpp log4cpp-dev qnx-test-log4cpp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/log4cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/log4cpp.list
rm var/lib/manual/log4cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild log4cpp` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/log4cpp/log4cpp-1.1.4.tar.gz
mkdir -p dist src/log4cpp
curl -fL -o dist/$(basename $URL) $URL
echo "696113659e426540625274a8b251052cc04306d8ee5c42a0c7639f39ca90c9d6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/log4cpp --strip-components=1
```

The project lives in the `log4cpp/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/log4cpp && cd build/$QNX_ARCH/log4cpp
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_func_snprintf=yes \
$OLDPWD/src/log4cpp/log4cpp/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-static \
    --disable-doxygen \
    --disable-dot \
    --without-idsa
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- log4cpp: a C++ logging library after log4j -- categories, appenders (file, rolling file, syslog, ostream, remote), layouts and configuration files. (Not log4cxx, which is ported separately.)
- The tarball's top directory is log4cpp/.
- configure's snprintf check is a run-test, so a cross build assumes there is none and compiles log4cpp's bundled portable_vsnprintf, which drops characters on QNX ("hello %d" came out "hell42"). QNX's vsnprintf is C99-conforming: say so.
- As qnx-ports builds it: shared only, no doxygen/dot documentation, no IDSA (intrusion detection) appender.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH log4cpp`.

### 5. What it installs

58 files under `/usr/local`: 2 shared-library entries, 1 programs, 53 headers.

```
/usr/local/lib/liblog4cpp.so
/usr/local/lib/liblog4cpp.so.5
/usr/local/bin/log4cpp-config
```

The complete list is `stage/<arch>/.manifest/log4cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa log4cpp
ls dists/8.0/*/ | grep '^log4cpp-'
```

This writes `log4cpp-<ver>-r0.tar.gz` (run time) and `log4cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add log4cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/log4cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/log4cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_log4cpp`](../../tests/src/) links log4cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-log4cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-log4cpp
/usr/local/bin/qnx-port-tests/test_log4cpp
```

```
== log4cpp ==
  ok   ...
PASS log4cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_log4cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_log4cpp`](../../tests/src/) -- a program linked against log4cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - root: DEBUG dropped under INFO; printf-style message
   - child category inherits the root appender; stream API
   - child inherits priority INFO
   - child priority DEBUG overrides the parent
   - additivity off: nothing reaches the root appender
   - FileAppender wrote \"[req-7] to file only\" (NDC in %x)
   - RollingFileAppender rolled over to r.log.1
   - current r.log holds the newest lines
   - PropertyConfigurator read the file
   - configured category wrote cfg|DEBUG|configured

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | log4cpp 1.1.4-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/log4cpp-1.1.4-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | log4cpp 1.1.4-r1 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/log4cpp-1.1.4-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.1.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add log4cpp=<version>`), sources in `ported/log4cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates log4cpp`; port one with `scripts/port-update log4cpp`.

<details><summary>recipe.sh</summary>

```bash
# log4cpp: a C++ logging library after log4j -- categories, appenders
# (file, rolling file, syslog, ostream, remote), layouts and configuration
# files. (Not log4cxx, which is ported separately.)
VERSION=1.1.4
TARBALL=https://downloads.sourceforge.net/log4cpp/log4cpp-1.1.4.tar.gz
SHA256=696113659e426540625274a8b251052cc04306d8ee5c42a0c7639f39ca90c9d6
# The tarball's top directory is log4cpp/.
SUBDIR=log4cpp
BUILDSYS=autotools
# configure's snprintf check is a run-test, so a cross build assumes there
# is none and compiles log4cpp's bundled portable_vsnprintf, which drops
# characters on QNX ("hello %d" came out "hell42"). QNX's vsnprintf is
# C99-conforming: say so.
CONF_ENV=( ac_cv_func_snprintf=yes )
# As qnx-ports builds it: shared only, no doxygen/dot documentation, no
# IDSA (intrusion detection) appender.
CONF_ARGS_EXTRA=( --disable-static --disable-doxygen --disable-dot --without-idsa )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
