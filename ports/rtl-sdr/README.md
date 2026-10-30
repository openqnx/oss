# rtl-sdr 2.0.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/osmocom/rtl-sdr.git (tag `v2.0.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libusb`](../libusb/) |
| Run-time dependencies | [`libusb`](../libusb/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add rtl-sdr
apk add rtl-sdr-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install rtl-sdr`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild rtl-sdr` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libusb
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.0.2 --recurse-submodules --shallow-submodules \
    https://github.com/osmocom/rtl-sdr.git src/rtl-sdr
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/rtl-sdr -B build/$QNX_ARCH/rtl-sdr -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DINSTALL_UDEV_RULES=OFF \
      -DDETACH_KERNEL_DRIVER=OFF
cmake --build build/$QNX_ARCH/rtl-sdr -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/rtl-sdr
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH rtl-sdr`.

### 5. What it installs

19 files under `/usr/local`: 3 shared-library entries, 8 programs, 2 headers.

```
/usr/local/lib/librtlsdr.so
/usr/local/lib/librtlsdr.so.0
/usr/local/lib/librtlsdr.so.2.0.1
/usr/local/bin/rtl_adsb
/usr/local/bin/rtl_biast
/usr/local/bin/rtl_eeprom
/usr/local/bin/rtl_fm
/usr/local/bin/rtl_power
/usr/local/bin/rtl_sdr
/usr/local/bin/rtl_tcp
/usr/local/bin/rtl_test
```

The complete list is `stage/<arch>/.manifest/rtl-sdr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa rtl-sdr
ls dists/8.0/*/ | grep '^rtl-sdr-'
```

This writes `rtl-sdr-<ver>-r0.tar.gz` (run time) and `rtl-sdr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add rtl-sdr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/rtl-sdr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/rtl-sdr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | rtl-sdr 2.0.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | rtl-sdr 2.0.2-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.0.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add rtl-sdr=<version>`), sources in `ported/rtl-sdr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates rtl-sdr`; port one with `scripts/port-update rtl-sdr`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.0.2; GIT=https://github.com/osmocom/rtl-sdr.git; REF=v2.0.2
DEPS="libusb"
CMAKE_ARGS_EXTRA=( -DINSTALL_UDEV_RULES=OFF -DDETACH_KERNEL_DRIVER=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
