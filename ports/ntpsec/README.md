# ntpsec 1.2.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.com/NTPsec/ntpsec.git (tag `NTPsec_1_2_4`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add ntpsec
apk add ntpsec-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install ntpsec`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ntpsec` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b NTPsec_1_2_4 --recurse-submodules --shallow-submodules \
    https://gitlab.com/NTPsec/ntpsec.git src/ntpsec
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ntpsec < ports/ntpsec/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add qnx-support

> Imported from qnx-ports/ntpsec (NTPsec_1_2_4...qnx-ntpsec-1.2.4); original authorship kept.
>
>
> ---
>  include/ntp_refclock.h  |  2 +-
>  include/ntp_syscall.h   |  7 +++++++
>  include/ntp_tty.h       |  2 +-
>  include/ntp_types.h     |  3 ++-
>  libntp/clockwork.c      |  2 ++
>  libntp/ntp_c.c          |  5 +++++
>  libntp/statestr.c       | 36 ++++++++++++++++++++++----------
>  ntpd/ntp_control.c      |  9 +++++++-
>  ntpd/ntp_loopfilter.c   | 46 +++++++++++++++++++++++++++++++++++------
>  ntpd/wscript            |  3 ++-
>  tests/libntp/statestr.c |  8 ++++++-
>  wscript                 | 20 +++++++++++++-----
>  12 files changed, 115 insertions(+), 28 deletions(-)

Touches: `include/ntp_refclock.h`, `include/ntp_syscall.h`, `include/ntp_tty.h`, `include/ntp_types.h`, `libntp/clockwork.c`, `libntp/ntp_c.c` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ntpsec; B=build/$QNX_ARCH/ntpsec; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  printf '#!/bin/sh\nexec qcc "$@"\n' > "$B/qnx-qcc" && chmod +x "$B/qnx-qcc" && \
  env -u CC -u CFLAGS -u LDFLAGS -u CPPFLAGS PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 ./waf configure --prefix="$PREFIX" \
    --cross-compiler="$B/qnx-qcc -V$QCC_TARGET" \
    --cross-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --cross-ldflags="-L$STAGE$PREFIX/lib -lsocket -lm" \
    --disable-mdns-registration --disable-doc --disable-manpage
}
```

```bash
do_build() { cd "$B" && python3 ./waf build -j"$(nproc)"; }
```

```bash
do_install() { cd "$B" && python3 ./waf install --destdir="$STAGE"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- NTPsec builds with waf (Python). Cross compiling is a waf feature: the compiler and flags for the target are passed as --cross-* options, and waf itself runs on the build host. The QNX patch recognises a QNX build by "qnx" appearing in the compiler's name (it skips run-time probes and the struct timex check then). Call the compiler through a wrapper named qnx-qcc, so that holds wherever the SDP is.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ntpsec`.

### 5. What it installs

47 files under `/usr/local`: 3 shared-library entries, 13 programs.

```
/usr/local/lib/libntpc.so
/usr/local/lib/libntpc.so.1
/usr/local/lib/libntpc.so.1.1.0
/usr/local/bin/ntpdig
/usr/local/bin/ntpfrob
/usr/local/bin/ntpkeygen
/usr/local/bin/ntpleapfetch
/usr/local/bin/ntplogtemp
/usr/local/bin/ntpmon
/usr/local/bin/ntpq
/usr/local/bin/ntpsnmpd
/usr/local/bin/ntpsweep
/usr/local/bin/ntptrace
/usr/local/bin/ntpviz
/usr/local/bin/ntpwait
/usr/local/sbin/ntpd
```

The complete list is `stage/<arch>/.manifest/ntpsec` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ntpsec
ls dists/8.0/*/ | grep '^ntpsec-'
```

This writes `ntpsec-<ver>-r0.tar.gz` (run time) and `ntpsec-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ntpsec
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ntpsec-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ntpsec-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ntpsec 1.2.4-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ntpsec 1.2.4-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ntpsec=<version>`), sources in `ported/ntpsec/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ntpsec`; port one with `scripts/port-update ntpsec`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.2.4; GIT=https://gitlab.com/NTPsec/ntpsec.git; REF=NTPsec_1_2_4
DEPS="openssl"
BUILDSYS=custom
# NTPsec builds with waf (Python). Cross compiling is a waf feature: the
# compiler and flags for the target are passed as --cross-* options, and waf
# itself runs on the build host.
# The QNX patch recognises a QNX build by "qnx" appearing in the compiler's
# name (it skips run-time probes and the struct timex check then). Call the
# compiler through a wrapper named qnx-qcc, so that holds wherever the SDP is.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  printf '#!/bin/sh\nexec qcc "$@"\n' > "$B/qnx-qcc" && chmod +x "$B/qnx-qcc" && \
  env -u CC -u CFLAGS -u LDFLAGS -u CPPFLAGS PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" \
  python3 ./waf configure --prefix="$PREFIX" \
    --cross-compiler="$B/qnx-qcc -V$QCC_TARGET" \
    --cross-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --cross-ldflags="-L$STAGE$PREFIX/lib -lsocket -lm" \
    --disable-mdns-registration --disable-doc --disable-manpage
}
do_build() { cd "$B" && python3 ./waf build -j"$(nproc)"; }
do_install() { cd "$B" && python3 ./waf install --destdir="$STAGE"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
