# portaudio 19.7.0

> Portable cross-platform library for real-time audio input and output

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/PortAudio/portaudio.git (tag `v19.7.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_portaudio`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `portaudio` (programs, shared libraries, run-time data) and `portaudio-dev` (headers, pkg-config/CMake files, static libraries; needs `portaudio`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`portaudio`](https://openqnx.github.io/oss/package/x86_64/portaudio.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/portaudio.html)) · [`portaudio-dev`](https://openqnx.github.io/oss/package/x86_64/portaudio-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/portaudio-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add portaudio` (dependencies come along); one version: `apk add portaudio=<version>` | `apk del portaudio` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install portaudio` (dependencies first) | `oss-get remove portaudio` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`portaudio-dev` the same way, by its own name (`apk add portaudio-dev`, `oss-get install portaudio-dev`, by hand with its own file and list `portaudio-dev.list`); remove it before `portaudio`, which it needs.

```sh
# on the QNX target, as root
apk add portaudio portaudio-dev
apk add qnx-test-portaudio && /usr/local/bin/qnx-port-tests/test_portaudio    # its on-target test, from tests-8.0-<arch>-m-z
apk del portaudio portaudio-dev qnx-test-portaudio
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/portaudio.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/portaudio.list
rm var/lib/manual/portaudio.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild portaudio` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v19.7.0 --recurse-submodules --shallow-submodules \
    https://github.com/PortAudio/portaudio.git src/portaudio
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/portaudio -B build/$QNX_ARCH/portaudio -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DPA_USE_ALSA=ON \
      -DPA_ALSA_DYNAMIC=OFF \
      -DPA_USE_JACK=OFF \
      -DPA_BUILD_SHARED=ON \
      -DPA_BUILD_STATIC=OFF \
      -DPA_BUILD_TESTS=OFF \
      -DPA_BUILD_EXAMPLES=OFF
cmake --build build/$QNX_ARCH/portaudio -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/portaudio
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- portaudio: portable real-time audio input/output C library (python3- sounddevice, Audacity-like programs, many SDR and speech tools use it).
- The ALSA host API, on QNX 8's ALSA-compatible libasound (the SDP's, which talks to io-snd); linked directly, not dlopen()ed. No JACK (not ported). Shared library only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH portaudio`.

### 5. What it installs

10 files under `/usr/local`: 1 shared-library entries, 2 headers.

```
/usr/local/lib/libportaudio.so
```

The complete list is `stage/<arch>/.manifest/portaudio` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa portaudio
ls dists/8.0/*/ | grep '^portaudio-'
```

This writes `portaudio-<ver>-r0.tar.gz` (run time) and `portaudio-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add portaudio
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/portaudio-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/portaudio-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_portaudio`](../../tests/src/) links portaudio the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-portaudio`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-portaudio
/usr/local/bin/qnx-port-tests/test_portaudio
```

```
== portaudio ==
  ok   ...
PASS portaudio
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_portaudio` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_portaudio`](../../tests/src/) -- a program linked against portaudio the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version 19.7.0
   - error text for paInvalidDevice
   - sample sizes 2/4/3 for int16/float32/int24
   - Pa_Initialize
   - the ALSA host API is there
   - host API name 'ALSA'
   - device count is not an error
   - every device has a name and channel counts
   - no info for a device index past the end
   - opening a device that does not exist: paInvalidDevice
   - IsFormatSupported on it: paInvalidDevice
   - 0.2 s of silence written to the default output
   - no default output: opening one fails cleanly
   - Pa_Terminate
   - after Pa_Terminate, calls report paNotInitialized

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | portaudio 19.7.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/portaudio-19.7.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | portaudio 19.7.0-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/portaudio-19.7.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **19.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add portaudio=<version>`), sources in `ported/portaudio/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates portaudio`; port one with `scripts/port-update portaudio`.

<details><summary>recipe.sh</summary>

```bash
# portaudio: portable real-time audio input/output C library (python3-
# sounddevice, Audacity-like programs, many SDR and speech tools use it).
VERSION=19.7.0; GIT=https://github.com/PortAudio/portaudio.git; REF=v19.7.0
DEPS=""
BUILDSYS=cmake
# The ALSA host API, on QNX 8's ALSA-compatible libasound (the SDP's, which
# talks to io-snd); linked directly, not dlopen()ed. No JACK (not ported).
# Shared library only.
CMAKE_ARGS_EXTRA=( -DPA_USE_ALSA=ON -DPA_ALSA_DYNAMIC=OFF -DPA_USE_JACK=OFF
                   -DPA_BUILD_SHARED=ON -DPA_BUILD_STATIC=OFF -DPA_BUILD_TESTS=OFF -DPA_BUILD_EXAMPLES=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
