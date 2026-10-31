# abuild 3.16.0

> Alpine's tools for building APK packages

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/alpinelinux/abuild.git (tag `3.16.0`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`apk-tools`](../apk-tools/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_abuild`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `abuild`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`abuild`](https://openqnx.github.io/oss/package/x86_64/abuild.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/abuild.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add abuild` (dependencies come along); one version: `apk add abuild=<version>` | `apk del abuild` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install abuild` (dependencies first) | `oss-get remove abuild` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-abuild && /usr/local/bin/qnx-port-tests/test_abuild    # its on-target test
apk del abuild qnx-test-abuild
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/abuild.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/abuild.list
rm var/lib/manual/abuild.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild abuild` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib apk-tools
```

<details><summary>Full build chain (4)</summary>

[`apk-tools`](../apk-tools/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.16.0 --recurse-submodules --shallow-submodules \
    https://github.com/alpinelinux/abuild.git src/abuild
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/abuild < ports/abuild/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/abuild < ports/abuild/patches/0002-qnx-no-nproc.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx: remove features not used by QNX

> - /usr merge, no longer using /bin, /sbin, /lib, /usr/sbin and /usr/libexec
>   - also applies to openrc
> - add qnx target triplets
> - remove -pyc auto splitting
> - remove auto so dependency, only prompt a warning
> - remove capability detections
> - remove nftables auto splitting
> - remove multi-libc support
>
> ---
>  abuild-sudo.c   |  10 +--
>  abuild.in       | 193 +++++++++++++++++++++++++-----------------------
>  functions.sh.in |  62 +---------------
>  3 files changed, 107 insertions(+), 158 deletions(-)

Touches: `abuild-sudo.c`, `abuild.in`, `functions.sh.in`, `abuild.in`, `abuild.in`, `default.conf` … · Upstream status: QNX-specific (from qnx-ports)

**[`0002-qnx-no-nproc.patch`](patches/0002-qnx-no-nproc.patch)** — QNX has no nproc: take the CPU count from getconf

Touches: `default.conf` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/abuild; B=build/$QNX_ARCH/abuild; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _mk abuild-tar abuild-gzsplit abuild-sudo abuild abuild-keygen abuild-sign newapkbuild abump apkgrel buildlab checkapk functions.sh; }
```

```bash
do_install() { _mk DESTDIR="$STAGE" install && rm -rf "$STAGE$PREFIX/share/man"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- abuild: Alpine's APK package build tool (shell scripts plus small C helpers) "adapted for QNX": upstream 3.16.0 (GitHub mirror of gitlab.alpinelinux.org) with qnx-ports' QNX commits (patches/, the release QNX's aports builds, 3.16.0_p5; the image ships 3.15.0); the scripts expect a POSIX shell, apk-tools and the usual build utilities on the target.
- The man pages are made with scdoc, which the build host does not have; SCDOC=true leaves them empty, and they are not installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH abuild`.

### 5. What it installs

30 files under `/usr/local`, 19 programs.

```
/usr/local/bin/abuild
/usr/local/bin/abuild-addgroup
/usr/local/bin/abuild-adduser
/usr/local/bin/abuild-apk
/usr/local/bin/abuild-fetch
/usr/local/bin/abuild-gzsplit
/usr/local/bin/abuild-keygen
/usr/local/bin/abuild-rmtemp
/usr/local/bin/abuild-sign
/usr/local/bin/abuild-sudo
/usr/local/bin/abuild-tar
/usr/local/bin/abump
/usr/local/bin/apkbuild-cpan
/usr/local/bin/apkbuild-gem-resolver
/usr/local/bin/apkbuild-pypi
/usr/local/bin/apkgrel
...
```

The complete list is `stage/<arch>/.manifest/abuild` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa abuild
ls dists/8.0/*/ | grep '^abuild-'
```

This writes `abuild-<ver>-r0.tar.gz` (run time) and `abuild-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add abuild
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/abuild-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/abuild-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_abuild`](../../tests/src/) links abuild the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-abuild`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-abuild
/usr/local/bin/qnx-port-tests/test_abuild
```

```
== abuild ==
  ok   ...
PASS abuild
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_abuild` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_abuild`](../../tests/src/) -- a program linked against abuild the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | abuild 3.16.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/abuild-3.16.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | abuild 3.16.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/abuild-3.16.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | abuild 3.16.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/abuild-3.16.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | abuild 3.16.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/abuild-3.16.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.16.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add abuild=<version>`), sources in `ported/abuild/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates abuild`; port one with `scripts/port-update abuild`.

<details><summary>recipe.sh</summary>

```bash
# abuild: Alpine's APK package build tool (shell scripts plus small C
# helpers) "adapted for QNX": upstream 3.16.0 (GitHub mirror of
# gitlab.alpinelinux.org) with qnx-ports' QNX commits (patches/, the release
# QNX's aports builds, 3.16.0_p5; the image ships 3.15.0); the scripts expect a POSIX shell, apk-tools and the
# usual build utilities on the target.
VERSION=3.16.0; GIT=https://github.com/alpinelinux/abuild.git; REF=3.16.0
DEPS="openssl zlib apk-tools"
BUILDSYS=custom
# The man pages are made with scdoc, which the build host does not have;
# SCDOC=true leaves them empty, and they are not installed.
_mk() { make -C "$B" SCDOC=true CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" prefix="$PREFIX" sysconfdir="$PREFIX/etc" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _mk abuild-tar abuild-gzsplit abuild-sudo abuild abuild-keygen abuild-sign newapkbuild abump apkgrel buildlab checkapk functions.sh; }
do_install() { _mk DESTDIR="$STAGE" install && rm -rf "$STAGE$PREFIX/share/man"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
