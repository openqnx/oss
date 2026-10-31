# strongswan 6.0.6

> IPsec-based VPN solution

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/strongswan/strongswan.git (tag `6.0.6`) |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_strongswan`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `strongswan` (programs, shared libraries, run-time data) and `strongswan-dev` (headers, pkg-config/CMake files, static libraries; needs `strongswan`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`strongswan`](https://openqnx.github.io/oss/package/x86_64/strongswan.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/strongswan.html)) · [`strongswan-dev`](https://openqnx.github.io/oss/package/x86_64/strongswan-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/strongswan-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add strongswan` (dependencies come along); one version: `apk add strongswan=<version>` | `apk del strongswan` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install strongswan` (dependencies first) | `oss-get remove strongswan` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`strongswan-dev` the same way, by its own name (`apk add strongswan-dev`, `oss-get install strongswan-dev`, by hand with its own file and list `strongswan-dev.list`); remove it before `strongswan`, which it needs.

```sh
# on the QNX target, as root
apk add strongswan strongswan-dev
apk add qnx-test-strongswan && /usr/local/bin/qnx-port-tests/test_strongswan    # its on-target test
apk del strongswan strongswan-dev qnx-test-strongswan
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/strongswan.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/strongswan.list
rm var/lib/manual/strongswan.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild strongswan` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 6.0.6 --recurse-submodules --shallow-submodules \
    https://github.com/strongswan/strongswan.git src/strongswan
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/strongswan < ports/strongswan/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/strongswan < ports/strongswan/patches/0005-file-logger-setvbuf.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add qnx support to strongswan

> Imported from qnx-ports/strongswan (4d709df8b25f...qnx-6.0.6); original authorship kept.
>
>
> ---
>  configure.ac                                  |   2 +-
>  .../plugins/kernel_pfkey/kernel_pfkey_ipsec.c | 142 +++++++++++++++++-
>  .../socket_default/socket_default_socket.c    |  17 +++
>  src/libcharon/plugins/vici/Makefile.am        |   4 +
>  src/libcharon/tests/Makefile.am               |   4 +
>  src/libipsec/tests/Makefile.am                |   4 +
>  src/libstrongswan/networking/tun_device.c     |   2 +
>  .../plugins/sshkey/sshkey_builder.c           |  58 +++++++
>  src/libstrongswan/tests/Makefile.am           |   6 +-
>  .../tests/suites/test_threading.c             |   7 +-
>  src/libstrongswan/utils/utils/align.h         |   6 +-
>  src/libtls/tests/Makefile.am                  |   4 +
>  src/starter/tests/Makefile.am                 |   4 +
>  13 files changed, 252 insertions(+), 8 deletions(-)

Touches: `configure.ac`, `src/libcharon/plugins/kernel_pfkey/kernel_pfkey_ipsec.c`, `src/libcharon/plugins/socket_default/socket_default_socket.c`, `src/libcharon/plugins/vici/Makefile.am`, `src/libcharon/tests/Makefile.am`, `src/libipsec/tests/Makefile.am` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0005-file-logger-setvbuf.patch`](patches/0005-file-logger-setvbuf.patch)** — Use setvbuf(_IOLBF) instead of setlinebuf().

> setlinebuf() is a BSD extension QNX's stdio does not have; setvbuf() with
> _IOLBF is the standard spelling of the same thing.

Touches: `src/libcharon/bus/listeners/file_logger.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/strongswan && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/strongswan && cd build/$QNX_ARCH/strongswan
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/strongswan/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-kernel-netlink \
    --enable-kernel-pfkey \
    --enable-kernel-pfroute \
    --disable-gmp \
    --enable-openssl \
    --disable-scripts \
    --disable-stroke \
    --enable-swanctl \
    --disable-defaults \
    --enable-charon \
    --enable-ikev2 \
    --enable-nonce \
    --enable-random \
    --enable-pem \
    --enable-pkcs1 \
    --enable-pkcs8 \
    --enable-x509 \
    --enable-pubkey \
    --enable-sha2 \
    --enable-hmac \
    --enable-socket-default \
    --enable-vici \
    --enable-updown \
    --enable-kdf \
    --with-capabilities=no \
    --enable-pki \
    --enable-constraints \
    --enable-revocation
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the build turns into an error.
- --disable-defaults also drops three defaults that matter: * pki, the certificate tool (keys, CSRs, self-signed and issued certs); * constraints, which enforces X.509 name/path-length/policy constraints -- without it charon trusts a chain that violates its CA's name constraints, and libstrongswan_tests' certnames suite fails 0/10; * revocation (CRL/OCSP checking of peer certificates).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH strongswan`.

### 5. What it installs

78 files under `/usr/local`, 5 programs.

```
/usr/local/bin/exchange_tests
/usr/local/bin/libcharon_tests
/usr/local/bin/libstrongswan_tests
/usr/local/bin/vici_tests
/usr/local/sbin/swanctl
```

The complete list is `stage/<arch>/.manifest/strongswan` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa strongswan
ls dists/8.0/*/ | grep '^strongswan-'
```

This writes `strongswan-<ver>-r0.tar.gz` (run time) and `strongswan-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add strongswan
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/strongswan-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/strongswan-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_strongswan`](../../tests/src/) links strongswan the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-strongswan`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-strongswan
/usr/local/bin/qnx-port-tests/test_strongswan
```

```
== strongswan ==
  ok   ...
PASS strongswan
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_strongswan` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_strongswan`](../../tests/src/) -- a program linked against strongswan the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | strongswan 6.0.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/strongswan-6.0.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | strongswan 6.0.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/strongswan-6.0.6-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | strongswan 6.0.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/strongswan-6.0.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | strongswan 6.0.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/strongswan-6.0.6-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.0.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add strongswan=<version>`), sources in `ported/strongswan/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates strongswan`; port one with `scripts/port-update strongswan`.

<details><summary>recipe.sh</summary>

```bash
VERSION=6.0.6; GIT=https://github.com/strongswan/strongswan.git; REF=6.0.6
DEPS="openssl"
BUILDSYS=autotools
# GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the
# build turns into an error.
CFLAGS_APPEND="-Wno-error=maybe-uninitialized"
# --disable-defaults also drops three defaults that matter:
#  * pki, the certificate tool (keys, CSRs, self-signed and issued certs);
#  * constraints, which enforces X.509 name/path-length/policy constraints --
#    without it charon trusts a chain that violates its CA's name constraints,
#    and libstrongswan_tests' certnames suite fails 0/10;
#  * revocation (CRL/OCSP checking of peer certificates).
CONF_ARGS_EXTRA=( --disable-kernel-netlink --enable-kernel-pfkey --enable-kernel-pfroute --disable-gmp --enable-openssl
                  --disable-scripts --disable-stroke --enable-swanctl --disable-defaults --enable-charon
                  --enable-ikev2 --enable-nonce --enable-random --enable-pem --enable-pkcs1 --enable-pkcs8 --enable-x509
                  --enable-pubkey --enable-sha2 --enable-hmac --enable-socket-default --enable-vici --enable-updown
                  --enable-kdf --with-capabilities=no
                  --enable-pki --enable-constraints --enable-revocation )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
