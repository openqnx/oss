# strongswan 6.0.6

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/strongswan/strongswan.git (tag `6.0.6`) |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install strongswan
oss-get install strongswan-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
    --with-capabilities=no
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the build turns into an error.

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
./scripts/qnxpkg -a $QNX_ARCH strongswan
ls repo/8.0/*/ | grep '^strongswan-'
```

This writes `strongswan-<ver>-r0.tar.gz` (run time) and `strongswan-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install strongswan
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/strongswan-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/strongswan-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=6.0.6; GIT=https://github.com/strongswan/strongswan.git; REF=6.0.6
DEPS="openssl"
BUILDSYS=autotools
# GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the
# build turns into an error.
CFLAGS_APPEND="-Wno-error=maybe-uninitialized"
CONF_ARGS_EXTRA=( --disable-kernel-netlink --enable-kernel-pfkey --enable-kernel-pfroute --disable-gmp --enable-openssl
                  --disable-scripts --disable-stroke --enable-swanctl --disable-defaults --enable-charon
                  --enable-ikev2 --enable-nonce --enable-random --enable-pem --enable-pkcs1 --enable-pkcs8 --enable-x509
                  --enable-pubkey --enable-sha2 --enable-hmac --enable-socket-default --enable-vici --enable-updown
                  --enable-kdf --with-capabilities=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
