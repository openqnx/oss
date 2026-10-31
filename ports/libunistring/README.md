# libunistring 1.3

> GNU library for manipulating Unicode strings

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/libunistring/libunistring-1.3.tar.gz |
| SHA-256 | `8ea8ccf86c09dd801c8cac19878e804e54f707cf69884371130d20bde68386b7` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libunistring`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libunistring` (programs, shared libraries, run-time data) and `libunistring-dev` (headers, pkg-config/CMake files, static libraries; needs `libunistring`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libunistring`](https://openqnx.github.io/oss/package/x86_64/libunistring.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libunistring.html)) · [`libunistring-dev`](https://openqnx.github.io/oss/package/x86_64/libunistring-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libunistring-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libunistring` (dependencies come along); one version: `apk add libunistring=<version>` | `apk del libunistring` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libunistring` (dependencies first) | `oss-get remove libunistring` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libunistring-dev` the same way, by its own name (`apk add libunistring-dev`, `oss-get install libunistring-dev`, by hand with its own file and list `libunistring-dev.list`); remove it before `libunistring`, which it needs.

```sh
# on the QNX target, as root
apk add libunistring libunistring-dev
apk add qnx-test-libunistring && /usr/local/bin/qnx-port-tests/test_libunistring    # its on-target test
apk del libunistring libunistring-dev qnx-test-libunistring
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libunistring.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libunistring.list
rm var/lib/manual/libunistring.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libunistring` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/libunistring/libunistring-1.3.tar.gz
mkdir -p dist src/libunistring
curl -fL -o dist/$(basename $URL) $URL
echo "8ea8ccf86c09dd801c8cac19878e804e54f707cf69884371130d20bde68386b7  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libunistring --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libunistring && cd build/$QNX_ARCH/libunistring
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libunistring/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libunistring`.

### 5. What it installs

50 files under `/usr/local`: 3 shared-library entries, 20 headers.

```
/usr/local/lib/libunistring.so
/usr/local/lib/libunistring.so.5
/usr/local/lib/libunistring.so.5.2.0
```

The complete list is `stage/<arch>/.manifest/libunistring` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libunistring
ls dists/8.0/*/ | grep '^libunistring-'
```

This writes `libunistring-<ver>-r0.tar.gz` (run time) and `libunistring-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libunistring
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libunistring-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libunistring-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libunistring`](../../tests/src/) links libunistring the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libunistring`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libunistring
/usr/local/bin/qnx-port-tests/test_libunistring
```

```
== libunistring ==
  ok   ...
PASS libunistring
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libunistring` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libunistring`](../../tests/src/) -- a program linked against libunistring the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - u8_strlen counts bytes
   - u8_mbsnlen counts characters
   - u8_toupper
   - ß uppercases to SS
   - NFC composes to U+00FC
   - UTF-8 to UTF-32 keeps ß at U+00DF

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libunistring 1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libunistring-1.3-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libunistring 1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libunistring-1.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libunistring=<version>`), sources in `ported/libunistring/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libunistring`; port one with `scripts/port-update libunistring`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3
TARBALL=https://ftp.gnu.org/gnu/libunistring/libunistring-1.3.tar.gz
SHA256=8ea8ccf86c09dd801c8cac19878e804e54f707cf69884371130d20bde68386b7
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
