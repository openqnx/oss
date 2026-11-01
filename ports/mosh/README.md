# mosh 1.4.0

> Mobile shell that supports roaming and intelligent local echo

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/mobile-shell/mosh/releases/download/mosh-1.4.0/mosh-1.4.0.tar.gz |
| SHA-256 | `872e4b134e5df29c8933dff12350785054d2fd2839b5ae6b5587b14db1465ddd` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`protobuf`](../protobuf/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Run-time dependencies | [`abseil`](../abseil/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`protobuf`](../protobuf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mosh`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mosh` (programs, shared libraries, run-time data) and `mosh-dev` (headers, pkg-config/CMake files, static libraries; needs `mosh`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mosh`](https://openqnx.github.io/oss/package/x86_64/mosh.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mosh.html)) · [`mosh-dev`](https://openqnx.github.io/oss/package/x86_64/mosh-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mosh-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mosh` (dependencies come along); one version: `apk add mosh=<version>` | `apk del mosh` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mosh` (dependencies first) | `oss-get remove mosh` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mosh-dev` the same way, by its own name (`apk add mosh-dev`, `oss-get install mosh-dev`, by hand with its own file and list `mosh-dev.list`); remove it before `mosh`, which it needs.

```sh
# on the QNX target, as root
apk add mosh mosh-dev
apk add qnx-test-mosh && /usr/local/bin/qnx-port-tests/test_mosh    # its on-target test, from tests-8.0-<arch>-m-z
apk del mosh mosh-dev qnx-test-mosh
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mosh.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mosh.list
rm var/lib/manual/mosh.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mosh` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild protobuf ncurses openssl zlib
```

<details><summary>Full build chain (5)</summary>

[`abseil`](../abseil/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`protobuf`](../protobuf/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/mobile-shell/mosh/releases/download/mosh-1.4.0/mosh-1.4.0.tar.gz
mkdir -p dist src/mosh
curl -fL -o dist/$(basename $URL) $URL
echo "872e4b134e5df29c8933dff12350785054d2fd2839b5ae6b5587b14db1465ddd  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mosh --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mosh < ports/mosh/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/mosh (mosh-1.4.0...qnx-v1.4.0: a0d5332, 4927667,
> 5d4b338); original authorship kept.
>
> QNX declares forkpty() in <unix.h> (there is no <pty.h>, <util.h> or
> <libutil.h>), and struct winsize comes from <sys/ioctl.h>. QNX's
> `stty -g` prints a format that must be passed back as separate words, so the
> 80th-column emulation test restores it unquoted.
>
> ---

Touches: `src/examples/parse.cc`, `src/examples/termemu.cc`, `src/frontend/mosh-server.cc`, `src/tests/emulation-80th-column.test`, `src/tests/inpty.cc` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mosh && cd build/$QNX_ARCH/mosh
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
PROTOC="$BUILD_DIR/host/protobuf/protoc" \
CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include -std=gnu++17" \
$OLDPWD/src/mosh/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --with-crypto-library=openssl \
    --enable-client \
    --enable-server \
    --disable-examples
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mosh generates its wire-format classes with protoc at build time: that has to be the build machine's protoc of the same version as the staged runtime (protobuf 29 gencode checks PROTOBUF_VERSION exactly) -- the protobuf recipe's host build. configure asks for C++11 for any protobuf >= 3.6 and appends -std=gnu++11 to CXX; protobuf 29 and abseil need C++17. CXXFLAGS comes after it on every compile line, so -std=gnu++17 there wins (the rest repeats qnxbuild's CXXFLAGS, which this assignment replaces).
- OpenSSL for AES-OCB (the staged one, not the SDP's); the bash completion and the perl wrapper's ssh path are not specific to QNX and stay as upstream.
- protobuf's pkg-config file lists every abseil library it was built with; mosh calls into a handful. --as-needed keeps the rest out of the programs' NEEDED lists (libc stays needed: qnxbuild names it first under --no-as-needed).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mosh`.

### 5. What it installs

6 files under `/usr/local`, 3 programs.

```
/usr/local/bin/mosh
/usr/local/bin/mosh-client
/usr/local/bin/mosh-server
```

The complete list is `stage/<arch>/.manifest/mosh` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mosh
ls dists/8.0/*/ | grep '^mosh-'
```

This writes `mosh-<ver>-r0.tar.gz` (run time) and `mosh-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mosh
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mosh-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mosh-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mosh`](../../tests/src/) links mosh the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mosh`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mosh
/usr/local/bin/qnx-port-tests/test_mosh
```

```
== mosh ==
  ok   ...
PASS mosh
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mosh` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mosh`](../../tests/src/) -- a program linked against mosh the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mosh-server --version: mosh 1.4.0
   - mosh-client usage names mosh 1.4.0
   - terminfo: xterm-256color has 256 colors
   - terminfo: vt100 has no color count (-1)
   - mosh-server new prints MOSH CONNECT
   - MOSH CONNECT names the requested UDP port
   - MOSH CONNECT carries a 22-character session key
   - mosh-server detached and is running
   - forkpty for mosh-client
   - mosh-client shows the session's output (MOSH-QNX-42)
   - Ctrl-^ . ends mosh-client (exit 0)
   - mosh-server stopped

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mosh 1.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/mosh-1.4.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mosh 1.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/mosh-1.4.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.4.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mosh=<version>`), sources in `ported/mosh/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mosh`; port one with `scripts/port-update mosh`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.0
TARBALL=https://github.com/mobile-shell/mosh/releases/download/mosh-1.4.0/mosh-1.4.0.tar.gz
SHA256=872e4b134e5df29c8933dff12350785054d2fd2839b5ae6b5587b14db1465ddd
BUILDSYS=autotools
DEPS="protobuf ncurses openssl zlib"
# mosh generates its wire-format classes with protoc at build time: that has to
# be the build machine's protoc of the same version as the staged runtime
# (protobuf 29 gencode checks PROTOBUF_VERSION exactly) -- the protobuf
# recipe's host build.
# configure asks for C++11 for any protobuf >= 3.6 and appends -std=gnu++11 to
# CXX; protobuf 29 and abseil need C++17. CXXFLAGS comes after it on every
# compile line, so -std=gnu++17 there wins (the rest repeats qnxbuild's
# CXXFLAGS, which this assignment replaces).
CONF_ENV=( PROTOC="$BUILD_DIR/host/protobuf/protoc"
           CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include -std=gnu++17" )
# OpenSSL for AES-OCB (the staged one, not the SDP's); the bash completion and
# the perl wrapper's ssh path are not specific to QNX and stay as upstream.
CONF_ARGS_EXTRA=( --with-crypto-library=openssl --enable-client --enable-server
                  --disable-examples )
# protobuf's pkg-config file lists every abseil library it was built with;
# mosh calls into a handful. --as-needed keeps the rest out of the programs'
# NEEDED lists (libc stays needed: qnxbuild names it first under --no-as-needed).
LDFLAGS_APPEND="-Wl,--as-needed"
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
