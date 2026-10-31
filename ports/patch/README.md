# patch 2.8

> GNU patch, applies diff files to original files

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/patch/patch-2.8.tar.xz |
| SHA-256 | `f87cee69eec2b4fcbf60a396b030ad6aa3415f192aa5f7ee84cad5e11f7f5ae3` |
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
apk add patch
apk add patch-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cli-tools && /usr/local/bin/qnx-port-tests/test_cli_tools    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install patch`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild patch` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/patch/patch-2.8.tar.xz
mkdir -p dist src/patch
curl -fL -o dist/$(basename $URL) $URL
echo "f87cee69eec2b4fcbf60a396b030ad6aa3415f192aa5f7ee84cad5e11f7f5ae3  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/patch --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/patch && cd build/$QNX_ARCH/patch
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/patch/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-xattr
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- patch: GNU patch -- applies diff files to originals (on the target, for building there).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH patch`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/bin/patch
```

The complete list is `stage/<arch>/.manifest/patch` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa patch
ls dists/8.0/*/ | grep '^patch-'
```

This writes `patch-<ver>-r0.tar.gz` (run time) and `patch-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add patch
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/patch-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/patch-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cli_tools`](../../tests/src/) links patch the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cli-tools`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cli-tools
/usr/local/bin/qnx-port-tests/test_cli_tools
```

```
== patch ==
  ok   ...
PASS patch
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cli_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cli_tools`](../../tests/src/) -- a program linked against patch the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ed-1.22.5-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/lzip-1.26-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/patch-2.8-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/wget-1.25.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ed-1.22.5-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/lzip-1.26-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/patch-2.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/wget-1.25.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add patch=<version>`), sources in `ported/patch/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates patch`; port one with `scripts/port-update patch`.

<details><summary>recipe.sh</summary>

```bash
# patch: GNU patch -- applies diff files to originals (on the target, for building there).
VERSION=2.8
TARBALL=https://ftp.gnu.org/gnu/patch/patch-2.8.tar.xz
SHA256=f87cee69eec2b4fcbf60a396b030ad6aa3415f192aa5f7ee84cad5e11f7f5ae3
DEPS=""
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-xattr )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
