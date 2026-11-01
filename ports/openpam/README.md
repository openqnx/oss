# openpam 20250531

> BSD-licensed implementation of the Pluggable Authentication Modules API

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://git.des.dev/OpenPAM/OpenPAM.git (tag `openpam-20250531`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openpam`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `openpam` (programs, shared libraries, run-time data) and `openpam-dev` (headers, pkg-config/CMake files, static libraries; needs `openpam`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`openpam`](https://openqnx.github.io/oss/package/x86_64/openpam.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openpam.html)) · [`openpam-dev`](https://openqnx.github.io/oss/package/x86_64/openpam-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openpam-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add openpam` (dependencies come along); one version: `apk add openpam=<version>` | `apk del openpam` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install openpam` (dependencies first) | `oss-get remove openpam` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`openpam-dev` the same way, by its own name (`apk add openpam-dev`, `oss-get install openpam-dev`, by hand with its own file and list `openpam-dev.list`); remove it before `openpam`, which it needs.

```sh
# on the QNX target, as root
apk add openpam openpam-dev
apk add qnx-test-openpam && /usr/local/bin/qnx-port-tests/test_openpam    # its on-target test, from tests-8.0-<arch>-m-z
apk del openpam openpam-dev qnx-test-openpam
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/openpam.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/openpam.list
rm var/lib/manual/openpam.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openpam` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b openpam-20250531 --recurse-submodules --shallow-submodules \
    https://git.des.dev/OpenPAM/OpenPAM.git src/openpam
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/openpam && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/openpam && cd build/$QNX_ARCH/openpam
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/openpam/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --without-doc \
    --with-pamtest \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- OpenPAM, the BSD PAM implementation (FreeBSD, NetBSD, macOS, and QNX: the SDP ships OpenPAM 20190224 "Tabebuia" as libpam.so.2). This is the current release, 20250531, built from source like every other library here.
- The git tree has no configure: qnxbuild runs autogen.sh (libtoolize, aclocal, automake, autoconf). libpam.so.2, the soname OpenPAM has kept since 2002 (and QNX's own copy carries): the same ABI, so a program built against either loads the other. Modules are looked up in /usr/lib (where QNX keeps its own: pam_qnx, pam_secpol, ...) and then /usr/local/lib (LOCALBASE), where this package installs pam_deny, pam_permit and pam_return; policies come from /etc/pam.d, /etc/pam.conf, then /usr/local/etc/pam.d. pamtest, OpenPAM's test client, is installed too. No documentation (it needs mandoc tools).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openpam`.

### 5. What it installs

17 files under `/usr/local`: 8 shared-library entries, 1 programs, 7 headers.

```
/usr/local/lib/libpam.so
/usr/local/lib/libpam.so.2
/usr/local/lib/pam_deny.so
/usr/local/lib/pam_deny.so.2
/usr/local/lib/pam_permit.so
/usr/local/lib/pam_permit.so.2
/usr/local/lib/pam_return.so
/usr/local/lib/pam_return.so.2
/usr/local/bin/pamtest
```

The complete list is `stage/<arch>/.manifest/openpam` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openpam
ls dists/8.0/*/ | grep '^openpam-'
```

This writes `openpam-<ver>-r0.tar.gz` (run time) and `openpam-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openpam
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openpam-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openpam-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openpam`](../../tests/src/) links openpam the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openpam`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openpam
/usr/local/bin/qnx-port-tests/test_openpam
```

```
== openpam ==
  ok   ...
PASS openpam
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openpam` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openpam`](../../tests/src/) -- a program linked against openpam the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temp dir
   - pam_start with a policy file path
   - pam_get_user asked the conversation (\"login: \") and got alice
   - pam_get_authtok: password through the conversation
   - two conversation calls
   - PAM_USER item
   - set/get PAM_RHOST
   - pam_putenv / pam_getenv
   - pam_authenticate: pam_permit -> PAM_SUCCESS
   - pam_acct_mgmt: pam_permit -> PAM_SUCCESS
   - pam_end
   - pam_authenticate: pam_deny -> PAM_AUTH_ERR
   - pam_return error=PAM_USER_UNKNOWN (module option)
   - pam_return error=PAM_ACCT_EXPIRED
   - pam_start refuses a policy file that does not exist
   - pam_strerror(PAM_AUTH_ERR) = \"Authentication error\"

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openpam 20250531-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/openpam-20250531-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openpam 20250531-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/openpam-20250531-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20250531**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openpam=<version>`), sources in `ported/openpam/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openpam`; port one with `scripts/port-update openpam`.

<details><summary>recipe.sh</summary>

```bash
# OpenPAM, the BSD PAM implementation (FreeBSD, NetBSD, macOS, and QNX: the
# SDP ships OpenPAM 20190224 "Tabebuia" as libpam.so.2). This is the current
# release, 20250531, built from source like every other library here.
VERSION=20250531; GIT=https://git.des.dev/OpenPAM/OpenPAM.git; REF=openpam-20250531
BUILDSYS=autotools
# The git tree has no configure: qnxbuild runs autogen.sh (libtoolize,
# aclocal, automake, autoconf).
# libpam.so.2, the soname OpenPAM has kept since 2002 (and QNX's own copy
# carries): the same ABI, so a program built against either loads the other.
# Modules are looked up in /usr/lib (where QNX keeps its own: pam_qnx,
# pam_secpol, ...) and then /usr/local/lib (LOCALBASE), where this package
# installs pam_deny, pam_permit and pam_return; policies come from /etc/pam.d,
# /etc/pam.conf, then /usr/local/etc/pam.d. pamtest, OpenPAM's test client,
# is installed too. No documentation (it needs mandoc tools).
CONF_ARGS_EXTRA=( --without-doc --with-pamtest --disable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
