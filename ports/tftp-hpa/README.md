# tftp-hpa 5.2

> TFTP client and server

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git (tag `tftp-hpa-5.2`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`readline`](../readline/) [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tftp_hpa`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `tftp-hpa` (programs, shared libraries, run-time data) and `tftp-hpa-dev` (headers, pkg-config/CMake files, static libraries; needs `tftp-hpa`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tftp-hpa`](https://openqnx.github.io/oss/package/x86_64/tftp-hpa.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tftp-hpa.html)) · [`tftp-hpa-dev`](https://openqnx.github.io/oss/package/x86_64/tftp-hpa-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tftp-hpa-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tftp-hpa` (dependencies come along); one version: `apk add tftp-hpa=<version>` | `apk del tftp-hpa` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tftp-hpa` (dependencies first) | `oss-get remove tftp-hpa` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`tftp-hpa-dev` the same way, by its own name (`apk add tftp-hpa-dev`, `oss-get install tftp-hpa-dev`, by hand with its own file and list `tftp-hpa-dev.list`); remove it before `tftp-hpa`, which it needs.

```sh
# on the QNX target, as root
apk add tftp-hpa tftp-hpa-dev
apk add qnx-test-tftp-hpa && /usr/local/bin/qnx-port-tests/test_tftp_hpa    # its on-target test
apk del tftp-hpa tftp-hpa-dev qnx-test-tftp-hpa
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tftp-hpa.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tftp-hpa.list
rm var/lib/manual/tftp-hpa.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tftp-hpa` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild readline ncurses
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b tftp-hpa-5.2 --recurse-submodules --shallow-submodules \
    https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git src/tftp-hpa
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tftp-hpa < ports/tftp-hpa/patches/0001-qnx-sa-restart.patch
```

**[`0001-qnx-sa-restart.patch`](patches/0001-qnx-sa-restart.patch)** — Define SA_RESTART as 0 where the system lacks it.

> QNX's <signal.h> leaves SA_RESTART commented out ("not supported yet");
> bsd_signal() only uses it to request restartable system calls.

Touches: `lib/bsdsignal.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tftp-hpa; B=build/$QNX_ARCH/tftp-hpa; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE -fcommon" LIBS="-lsocket" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --without-tcpwrappers --without-ipv6; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" INSTALLROOT="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): readline
- tftp-hpa's Makefiles only work in the source tree, so build in a copy of it. -fcommon: tftp.c and main.c both define "toplevel" (old-style common symbol), an error by default since GCC 10.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tftp-hpa`.

### 5. What it installs

5 files under `/usr/local`, 2 programs.

```
/usr/local/bin/tftp
/usr/local/sbin/in.tftpd
```

The complete list is `stage/<arch>/.manifest/tftp-hpa` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tftp-hpa
ls dists/8.0/*/ | grep '^tftp-hpa-'
```

This writes `tftp-hpa-<ver>-r0.tar.gz` (run time) and `tftp-hpa-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tftp-hpa
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tftp-hpa-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tftp-hpa-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tftp_hpa`](../../tests/src/) links tftp-hpa the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tftp-hpa`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tftp-hpa
/usr/local/bin/qnx-port-tests/test_tftp_hpa
```

```
== tftp-hpa ==
  ok   ...
PASS tftp-hpa
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tftp_hpa` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tftp_hpa`](../../tests/src/) -- a program linked against tftp-hpa the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/tftp-hpa-5.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/tftp-hpa-5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/tftp-hpa-5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/tftp-hpa-5.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tftp-hpa=<version>`), sources in `ported/tftp-hpa/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tftp-hpa`; port one with `scripts/port-update tftp-hpa`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2; GIT=https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git; REF=tftp-hpa-5.2
# Also linked at run time (found by scripts/gen-dependency-index): readline
DEPS="readline ncurses"
BUILDSYS=custom
# tftp-hpa's Makefiles only work in the source tree, so build in a copy of it.
# -fcommon: tftp.c and main.c both define "toplevel" (old-style common symbol),
# an error by default since GCC 10.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE -fcommon" LIBS="-lsocket" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --without-tcpwrappers --without-ipv6; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" INSTALLROOT="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
