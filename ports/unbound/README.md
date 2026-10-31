# unbound 1.24.1

> Validating, recursive and caching DNS resolver and library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/NLnetLabs/unbound.git (tag `release-1.24.1`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_unbound`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `unbound` (programs, shared libraries, run-time data) and `unbound-dev` (headers, pkg-config/CMake files, static libraries; needs `unbound`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`unbound`](https://openqnx.github.io/oss/package/x86_64/unbound.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/unbound.html)) · [`unbound-dev`](https://openqnx.github.io/oss/package/x86_64/unbound-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/unbound-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add unbound` (dependencies come along); one version: `apk add unbound=<version>` | `apk del unbound` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install unbound` (dependencies first) | `oss-get remove unbound` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`unbound-dev` the same way, by its own name (`apk add unbound-dev`, `oss-get install unbound-dev`, by hand with its own file and list `unbound-dev.list`); remove it before `unbound`, which it needs.

```sh
# on the QNX target, as root
apk add unbound unbound-dev
apk add qnx-test-unbound && /usr/local/bin/qnx-port-tests/test_unbound    # its on-target test
apk del unbound unbound-dev qnx-test-unbound
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/unbound.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/unbound.list
rm var/lib/manual/unbound.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild unbound` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl expat
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-1.24.1 --recurse-submodules --shallow-submodules \
    https://github.com/NLnetLabs/unbound.git src/unbound
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/unbound < ports/unbound/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX Porting support for unbound branch-1.24.1

> Imported from qnx-ports/unbound (a33f0638e1da...qnx-release-1.24.1); original authorship kept.
>
>
> * qnx Porting support for version release-1.24.1
>
> * updating __QNXNTO__ with __QNX__
> ---
>  compat/arc4random.c        |  7 +++++++
>  compat/getentropy_linux.c  | 20 ++++++++++++++++++++
>  services/outside_network.h |  4 ++++
>  testcode/replay.h          |  4 ++++
>  testcode/unitldns.c        |  4 ++++
>  testcode/unitmsgparse.c    |  4 ++++
>  testcode/unitverify.c      |  5 ++++-
>  testcode/unitzonemd.c      |  4 ++++
>  util/data/msgreply.h       |  4 ++++
>  util/timehist.h            |  4 ++++
>  10 files changed, 59 insertions(+), 1 deletion(-)

Touches: `compat/arc4random.c`, `compat/getentropy_linux.c`, `services/outside_network.h`, `testcode/replay.h`, `testcode/unitldns.c`, `testcode/unitmsgparse.c` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/unbound && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/unbound && cd build/$QNX_ARCH/unbound
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_lib_lex="none needed" \
cv_prog_cc_flag_xc99=no \
$OLDPWD/src/unbound/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --with-ssl="$STAGE$PREFIX" \
    --with-libexpat="$STAGE$PREFIX" \
    --disable-static \
    --enable-shared \
    --with-pidfile=/var/run/unbound.pid \
    --with-conf-file=/usr/local/etc/unbound/unbound.conf
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- AC_PROG_LEX looks for a target libfl to link and, finding none in a cross build, gives up on flex; the generated lexer needs no library (it is built with %option noyywrap), so say so. configure probes the Sun compiler flag -xc99; qcc accepts it but reads it as "-x c99" (input language), and every later link test then fails -- which surfaces as "OpenSSL ... version 0.9.7 or higher is required".

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH unbound`.

### 5. What it installs

49 files under `/usr/local`: 2 shared-library entries, 6 programs, 1 headers.

```
/usr/local/lib/libunbound.so
/usr/local/lib/libunbound.so.9
/usr/local/sbin/unbound
/usr/local/sbin/unbound-anchor
/usr/local/sbin/unbound-checkconf
/usr/local/sbin/unbound-control
/usr/local/sbin/unbound-control-setup
/usr/local/sbin/unbound-host
```

The complete list is `stage/<arch>/.manifest/unbound` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa unbound
ls dists/8.0/*/ | grep '^unbound-'
```

This writes `unbound-<ver>-r0.tar.gz` (run time) and `unbound-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add unbound
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/unbound-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/unbound-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_unbound`](../../tests/src/) links unbound the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-unbound`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-unbound
/usr/local/bin/qnx-port-tests/test_unbound
```

```
== unbound ==
  ok   ...
PASS unbound
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_unbound` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_unbound`](../../tests/src/) -- a program linked against unbound the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ub_ctx_create
   - ub_ctx_hosts reads a hosts file
   - add static local zone qnx.test.
   - add A record
   - add TXT record
   - A record from local data
   - TXT record from local data
   - NXDOMAIN inside the static zone
   - name from the hosts file
   - unbound daemon answers a UDP query on 127.0.0.1
   - daemon returns NXDOMAIN for an unknown name
   - libunbound forwards to the daemon
   - A record resolved through the daemon
   - TXT record resolved through the daemon
   - resolution through the daemon (daemon not up)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | unbound 1.24.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/unbound-1.24.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | unbound 1.24.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/unbound-1.24.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | unbound 1.24.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/unbound-1.24.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | unbound 1.24.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/unbound-1.24.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add unbound=<version>`), sources in `ported/unbound/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates unbound`; port one with `scripts/port-update unbound`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.1; GIT=https://github.com/NLnetLabs/unbound.git; REF=release-1.24.1
DEPS="openssl expat"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --with-ssl="$STAGE$PREFIX" --with-libexpat="$STAGE$PREFIX" --disable-static --enable-shared
                  --with-pidfile=/var/run/unbound.pid --with-conf-file=/usr/local/etc/unbound/unbound.conf )
# AC_PROG_LEX looks for a target libfl to link and, finding none in a cross
# build, gives up on flex; the generated lexer needs no library (it is built
# with %option noyywrap), so say so.
# configure probes the Sun compiler flag -xc99; qcc accepts it but reads it as
# "-x c99" (input language), and every later link test then fails -- which
# surfaces as "OpenSSL ... version 0.9.7 or higher is required".
CONF_ENV=( ac_cv_lib_lex="none needed" cv_prog_cc_flag_xc99=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
