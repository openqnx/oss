# openssh 10.2p1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-10.2p1.tar.gz |
| SHA-256 | `ccc42c0419937959263fa1dbd16dafc18c56b984c03562d2937ce56a60f798b2` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install openssh
oss-get install openssh-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openssh` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib libxcrypt
```

### 2. Get the upstream source

```bash
URL=https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-10.2p1.tar.gz
mkdir -p dist src/openssh
curl -fL -o dist/$(basename $URL) $URL
echo "ccc42c0419937959263fa1dbd16dafc18c56b984c03562d2937ce56a60f798b2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/openssh --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openssh < ports/openssh/patches/0001-aports-00-qnx-configure.patch
patch -p1 -d src/openssh < ports/openssh/patches/0002-aports-fix-qnx-build.patch
```

**[`0001-aports-00-qnx-configure.patch`](patches/0001-aports-00-qnx-configure.patch)** — QNX change from QNX's aports (core/openssh/00-qnx-configure.patch).

> Taken from qnx-ports/aports core/openssh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `configure.ac` · Upstream status: QNX-specific

**[`0002-aports-fix-qnx-build.patch`](patches/0002-aports-fix-qnx-build.patch)** — QNX change from QNX's aports (core/openssh/fix-qnx-build.patch).

> Taken from qnx-ports/aports core/openssh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `openssh-10.3p1/includes.h`, `openssh-10.3p1/openbsd-compat/openbsd-compat.h`, `openssh-10.3p1/openbsd-compat/rresvport.c`, `sshpty.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/openssh && cd build/$QNX_ARCH/openssh
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/openssh/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --sysconfdir=/usr/local/etc/ssh \
    --with-privsep-path=/var/empty \
    --with-pid-dir=/var/run \
    --without-pam \
    --without-selinux \
    --disable-strip \
    --with-ssl-dir="$STAGE$PREFIX" \
    --with-zlib="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- OpenSSH client and server. Version as in the QNX Everywhere 8.0.4r1 image. (The QNX SDP ships its own OpenSSH; this one installs under /usr/local.)
- Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
- The QNX patches change configure.ac; regenerate configure from it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openssh`.

### 5. What it installs

32 files under `/usr/local`, 8 programs.

```
/usr/local/bin/scp
/usr/local/bin/sftp
/usr/local/bin/ssh
/usr/local/bin/ssh-add
/usr/local/bin/ssh-agent
/usr/local/bin/ssh-keygen
/usr/local/bin/ssh-keyscan
/usr/local/sbin/sshd
```

The complete list is `stage/<arch>/.manifest/openssh` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH openssh
ls repo/8.0/*/ | grep '^openssh-'
```

This writes `openssh-<ver>-r0.tar.gz` (run time) and `openssh-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install openssh
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/openssh-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/openssh-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# OpenSSH client and server. Version as in the QNX Everywhere 8.0.4r1 image.
# (The QNX SDP ships its own OpenSSH; this one installs under /usr/local.)
VERSION=10.2p1
TARBALL=https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-10.2p1.tar.gz
SHA256=ccc42c0419937959263fa1dbd16dafc18c56b984c03562d2937ce56a60f798b2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
DEPS="openssl zlib libxcrypt"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc/ssh --with-privsep-path=/var/empty --with-pid-dir=/var/run
                  --without-pam --without-selinux --disable-strip --with-ssl-dir="$STAGE$PREFIX" --with-zlib="$STAGE$PREFIX" )
# The QNX patches change configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
