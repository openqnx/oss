# mpc 1.3.1

> GNU MPC library for complex number arithmetic with arbitrary precision

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz |
| SHA-256 | `ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gmp`](../gmp/) [`mpfr`](../mpfr/) |
| Run-time dependencies | [`gmp`](../gmp/) [`mpfr`](../mpfr/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mpc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mpc` (programs, shared libraries, run-time data) and `mpc-dev` (headers, pkg-config/CMake files, static libraries; needs `mpc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mpc`](https://openqnx.github.io/oss/package/x86_64/mpc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mpc.html)) · [`mpc-dev`](https://openqnx.github.io/oss/package/x86_64/mpc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mpc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mpc` (dependencies come along); one version: `apk add mpc=<version>` | `apk del mpc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mpc` (dependencies first) | `oss-get remove mpc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mpc-dev` the same way, by its own name (`apk add mpc-dev`, `oss-get install mpc-dev`, by hand with its own file and list `mpc-dev.list`); remove it before `mpc`, which it needs.

```sh
# on the QNX target, as root
apk add mpc mpc-dev
apk add qnx-test-mpc && /usr/local/bin/qnx-port-tests/test_mpc    # its on-target test, from tests-8.0-<arch>-m-z
apk del mpc mpc-dev qnx-test-mpc
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mpc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mpc.list
rm var/lib/manual/mpc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mpc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gmp mpfr
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
mkdir -p dist src/mpc
curl -fL -o dist/$(basename $URL) $URL
echo "ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mpc --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mpc && cd build/$QNX_ARCH/mpc
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/mpc/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-gmp="$STAGE$PREFIX" \
    --with-mpfr="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mpc`.

### 5. What it installs

4 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libmpc.so
/usr/local/lib/libmpc.so.6
```

The complete list is `stage/<arch>/.manifest/mpc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mpc
ls dists/8.0/*/ | grep '^mpc-'
```

This writes `mpc-<ver>-r0.tar.gz` (run time) and `mpc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mpc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mpc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mpc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mpc`](../../tests/src/) links mpc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mpc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mpc
/usr/local/bin/qnx-port-tests/test_mpc
```

```
== mpc ==
  ok   ...
PASS mpc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mpc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mpc`](../../tests/src/) -- a program linked against mpc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - |3+4i| == 5
   - (3+4i)^2 == -7+24i
   - |e^(i*pi) + 1| is essentially zero

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mpc 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mpc-1.3.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mpc 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mpc-1.3.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mpc=<version>`), sources in `ported/mpc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mpc`; port one with `scripts/port-update mpc`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.1; TARBALL=https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
SHA256=ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8
DEPS="gmp mpfr"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static
                  --with-gmp="$STAGE$PREFIX" --with-mpfr="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
