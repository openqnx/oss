# picocom 3.1

> Minimal dumb-terminal emulator for serial ports

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/npat-efault/picocom.git (tag `3.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_picocom`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add picocom
apk add picocom-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-picocom && /usr/local/bin/qnx-port-tests/test_picocom    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install picocom`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild picocom` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.1 --recurse-submodules --shallow-submodules \
    https://github.com/npat-efault/picocom.git src/picocom
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/picocom < ports/picocom/patches/0001-qnx-select-and-hw-flow-control.patch
```

**[`0001-qnx-select-and-hw-flow-control.patch`](patches/0001-qnx-select-and-hw-flow-control.patch)** — Build on QNX: select() headers, ioctl, RTS/CTS flow control.

> - picocom.c uses select(), fd_set and struct timeval without including
>   <sys/select.h>/<sys/time.h>, which glibc pulls in through other headers
>   and QNX does not.
> - term.c only uses the modem-control ioctls (TIOCMGET/TIOCMSET) on a list of
>   known systems; QNX supports them too.
> - QNX has no CRTSCTS: hardware flow control is the pair of c_cflag bits
>   IHFLOW | OHFLOW. Defining CRTSCTS as that pair keeps "--flow h" working
>   (defining it as 0, as other QNX ports do, silently disables it).
>
> Based on qnx-ports/build-files ports/picocom/0001-qnx-compat.patch.

Touches: `picocom.c`, `term.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/picocom; B=build/$QNX_ARCH/picocom; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-D_QNX_SOURCE -DVERSION_STR=\\\"$VERSION\\\" -DHIGH_BAUD" LDFLAGS=""; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/picocom" "$STAGE$PREFIX/bin/"; }
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH picocom`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/picocom
```

The complete list is `stage/<arch>/.manifest/picocom` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa picocom
ls dists/8.0/*/ | grep '^picocom-'
```

This writes `picocom-<ver>-r0.tar.gz` (run time) and `picocom-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add picocom
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/picocom-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/picocom-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_picocom`](../../tests/src/) links picocom the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-picocom`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-picocom
/usr/local/bin/qnx-port-tests/test_picocom
```

```
== picocom ==
  ok   ...
PASS picocom
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_picocom` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_picocom`](../../tests/src/) -- a program linked against picocom the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | picocom 3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/picocom-3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | picocom 3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/picocom-3.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add picocom=<version>`), sources in `ported/picocom/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates picocom`; port one with `scripts/port-update picocom`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.1; GIT=https://github.com/npat-efault/picocom.git; REF=3.1
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-D_QNX_SOURCE -DVERSION_STR=\\\"$VERSION\\\" -DHIGH_BAUD" LDFLAGS=""; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/picocom" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
