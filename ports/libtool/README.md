# libtool 2.5.4

> GNU libltdl, a portable dynamic module loading library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.gz |
| SHA-256 | `da8ebb2ce4dcf46b90098daf962cffa68f4b4f62ea60f798d0ef12929ede6adf` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtool`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libtool` (programs, shared libraries, run-time data) and `libtool-dev` (headers, pkg-config/CMake files, static libraries; needs `libtool`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libtool`](https://openqnx.github.io/oss/package/x86_64/libtool.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtool.html)) · [`libtool-dev`](https://openqnx.github.io/oss/package/x86_64/libtool-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtool-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libtool` (dependencies come along); one version: `apk add libtool=<version>` | `apk del libtool` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libtool` (dependencies first) | `oss-get remove libtool` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libtool-dev` the same way, by its own name (`apk add libtool-dev`, `oss-get install libtool-dev`, by hand with its own file and list `libtool-dev.list`); remove it before `libtool`, which it needs.

```sh
# on the QNX target, as root
apk add libtool libtool-dev
apk add qnx-test-libtool && /usr/local/bin/qnx-port-tests/test_libtool    # its on-target test
apk del libtool libtool-dev qnx-test-libtool
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libtool.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libtool.list
rm var/lib/manual/libtool.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtool` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.gz
mkdir -p dist src/libtool
curl -fL -o dist/$(basename $URL) $URL
echo "da8ebb2ce4dcf46b90098daf962cffa68f4b4f62ea60f798d0ef12929ede6adf  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libtool --strip-components=1
```

The project lives in the `libltdl/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libtool && cd build/$QNX_ARCH/libtool
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libtool/libltdl/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --enable-ltdl-install
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Only libltdl is useful on the target; the m4 macros and the wrapper script are build-machine tools and already present on this host.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtool`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/libltdl.so
/usr/local/lib/libltdl.so.7
/usr/local/lib/libltdl.so.7.3.3
```

The complete list is `stage/<arch>/.manifest/libtool` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtool
ls dists/8.0/*/ | grep '^libtool-'
```

This writes `libtool-<ver>-r0.tar.gz` (run time) and `libtool-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtool
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtool-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtool-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libtool`](../../tests/src/) links libtool the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libtool`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libtool
/usr/local/bin/qnx-port-tests/test_libtool
```

```
== libtool ==
  ok   ...
PASS libtool
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtool` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libtool`](../../tests/src/) -- a program linked against libtool the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - lt_dlinit
   - set + add search directories
   - search path reads back
   - lt_dlopen libm
   - lt_dlsym cos + sqrt
   - called through ltdl: known values
   - lt_dlgetinfo: module refcount
   - missing symbol -> NULL
   - lt_dlerror explains a missing symbol
   - lt_dlclose
   - missing module -> NULL
   - lt_dlerror explains a missing module
   - lt_dlexit

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libtool 2.5.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libtool-2.5.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libtool 2.5.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libtool-2.5.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libtool 2.5.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libtool-2.5.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libtool 2.5.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libtool-2.5.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.5.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtool=<version>`), sources in `ported/libtool/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtool`; port one with `scripts/port-update libtool`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.5.4; TARBALL=https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.gz
SHA256=da8ebb2ce4dcf46b90098daf962cffa68f4b4f62ea60f798d0ef12929ede6adf
BUILDSYS=autotools
# Only libltdl is useful on the target; the m4 macros and the wrapper script are
# build-machine tools and already present on this host.
SUBDIR=libltdl
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-ltdl-install )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
