# ed 1.22.5

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/ed/ed-1.22.5.tar.lz |
| SHA-256 | `56e107ddc2f29dad6690376c15bf9751509e1ee3b8241710e44edbe5c3a158cc` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cli_tools`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add ed
apk add ed-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cli-tools && /usr/local/bin/qnx-port-tests/test_cli_tools    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install ed`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ed` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/ed/ed-1.22.5.tar.lz
mkdir -p dist src/ed
curl -fL -o dist/$(basename $URL) $URL
echo "56e107ddc2f29dad6690376c15bf9751509e1ee3b8241710e44edbe5c3a158cc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/ed --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/ed && cd build/$QNX_ARCH/ed
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/ed/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    CC="qcc -V$QCC_TARGET" \
    CFLAGS="-O2 -D_QNX_SOURCE" \
    LDFLAGS="-lregex"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ed: the POSIX line editor (scripts and the shell's fc use it; QNX ships it too).
- Its own configure (not autoconf) ignores --host and the environment and takes the compiler as CC=...: without it, it built ed for the build machine (a Linux executable, for both architectures). POSIX regcomp/regexec live in libregex on QNX.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ed`.

### 5. What it installs

5 files under `/usr/local`, 2 programs.

```
/usr/local/bin/ed
/usr/local/bin/red
```

The complete list is `stage/<arch>/.manifest/ed` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ed
ls dists/8.0/*/ | grep '^ed-'
```

This writes `ed-<ver>-r0.tar.gz` (run time) and `ed-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ed
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ed-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ed-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cli_tools`](../../tests/src/) links ed the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cli-tools`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cli-tools
/usr/local/bin/qnx-port-tests/test_cli_tools
```

```
== ed ==
  ok   ...
PASS ed
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cli_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cli_tools`](../../tests/src/) -- a program linked against ed the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r1 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.22.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ed=<version>`), sources in `ported/ed/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ed`; port one with `scripts/port-update ed`.

<details><summary>recipe.sh</summary>

```bash
# ed: the POSIX line editor (scripts and the shell's fc use it; QNX ships it too).
VERSION=1.22.5
TARBALL=https://ftp.gnu.org/gnu/ed/ed-1.22.5.tar.lz
SHA256=56e107ddc2f29dad6690376c15bf9751509e1ee3b8241710e44edbe5c3a158cc
DEPS=""
BUILDSYS=autotools
# Its own configure (not autoconf) ignores --host and the environment and
# takes the compiler as CC=...: without it, it built ed for the build machine
# (a Linux executable, for both architectures).
# POSIX regcomp/regexec live in libregex on QNX.
CONF_ARGS_EXTRA=( CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lregex" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
