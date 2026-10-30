# tftp-hpa 5.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://git.kernel.org/pub/scm/network/tftp/tftp-hpa.git (tag `tftp-hpa-5.2`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`readline`](../readline/) [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tftp-hpa
apk add tftp-hpa-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tftp-hpa`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tftp-hpa 5.2-r0 | ✅ PASS | 2026-10-30 |

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
