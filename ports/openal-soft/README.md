# openal-soft 1.24.1

> Cross-platform 3D audio library implementing the OpenAL API

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/kcat/openal-soft.git (tag `1.24.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openal_soft`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add openal-soft
apk add openal-soft-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-openal-soft && /usr/local/bin/qnx-port-tests/test_openal_soft    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install openal-soft`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openal-soft` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.24.1 --recurse-submodules --shallow-submodules \
    https://github.com/kcat/openal-soft.git src/openal-soft
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openal-soft < ports/openal-soft/patches/0001-qnx-current-path-nothrow.patch
```

**[`0001-qnx-current-path-nothrow.patch`](patches/0001-qnx-current-path-nothrow.patch)** — QNX: do not abort when the working directory cannot be queried.

> SearchDataFiles() called the throwing std::filesystem::current_path() outside
> any try block. libc++ on QNX sizes the getcwd buffer with
> pathconf(".", _PC_PATH_MAX); when that fails the exception is uncaught and
> the program aborts (openal-info died with SIGABRT while enumerating HRTFs).
> Use the error_code overload; an unknown cwd just means no app-local files.

Touches: `core/helpers.cpp`

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/openal-soft -B build/$QNX_ARCH/openal-soft -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DALSOFT_EXAMPLES=OFF \
      -DALSOFT_UTILS=ON \
      -DALSOFT_TESTS=OFF \
      -DALSOFT_NO_CONFIG_UTIL=ON \
      -DALSOFT_BACKEND_PIPEWIRE=OFF \
      -DALSOFT_BACKEND_PULSEAUDIO=OFF \
      -DALSOFT_BACKEND_ALSA=OFF \
      -DALSOFT_BACKEND_OSS=OFF \
      -DALSOFT_BACKEND_JACK=OFF \
      -DALSOFT_BACKEND_PORTAUDIO=OFF \
      -DALSOFT_BACKEND_SDL2=OFF \
      -DALSOFT_BACKEND_SNDIO=OFF \
      -DALSOFT_BACKEND_WAVE=ON
cmake --build build/$QNX_ARCH/openal-soft -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/openal-soft
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openal-soft`.

### 5. What it installs

24 files under `/usr/local`: 3 shared-library entries, 1 programs, 6 headers.

```
/usr/local/lib/libopenal.so
/usr/local/lib/libopenal.so.1
/usr/local/lib/libopenal.so.1.24.1
/usr/local/bin/openal-info
```

The complete list is `stage/<arch>/.manifest/openal-soft` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openal-soft
ls dists/8.0/*/ | grep '^openal-soft-'
```

This writes `openal-soft-<ver>-r0.tar.gz` (run time) and `openal-soft-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openal-soft
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openal-soft-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openal-soft-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openal_soft`](../../tests/src/) links openal-soft the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openal-soft`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openal-soft
/usr/local/bin/qnx-port-tests/test_openal_soft
```

```
== openal-soft ==
  ok   ...
PASS openal-soft
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openal_soft` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openal_soft`](../../tests/src/) -- a program linked against openal-soft the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - ALC_SOFT_loopback present
   - loopback entry points
   - alcLoopbackOpenDeviceSOFT
   - 44.1 kHz stereo int16 render format supported
   - loopback context
   - buffer, source, play: no AL error
   - rendered output is not silent
   - 1 kHz tone: ~400 zero crossings in 0.2 s
   - centred source: left == right
   - AL_GAIN 0.5 halves the amplitude
   - alcOpenDevice(NULL) with drivers = wave
   - device is the Wave File Writer
   - configured frequency 22050 applied
   - 0.5 s buffer played to the end
   - wave backend wrote RIFF/WAVE
   - WAV header: 22050 Hz stereo
   - the tone is in the file (>0.25 s of loud stereo samples)
   - openal-info exits 0
   - openal-info: vendor OpenAL Community
   - openal-info lists the Wave File Writer
   - openal-info lists ALC_SOFT_loopback

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openal-soft 1.24.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/openal-soft-1.24.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openal-soft 1.24.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/openal-soft-1.24.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openal-soft=<version>`), sources in `ported/openal-soft/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openal-soft`; port one with `scripts/port-update openal-soft`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.1; GIT=https://github.com/kcat/openal-soft.git; REF=1.24.1
CMAKE_ARGS_EXTRA=( -DALSOFT_EXAMPLES=OFF -DALSOFT_UTILS=ON -DALSOFT_TESTS=OFF -DALSOFT_NO_CONFIG_UTIL=ON
                   -DALSOFT_BACKEND_PIPEWIRE=OFF -DALSOFT_BACKEND_PULSEAUDIO=OFF -DALSOFT_BACKEND_ALSA=OFF
                   -DALSOFT_BACKEND_OSS=OFF -DALSOFT_BACKEND_JACK=OFF -DALSOFT_BACKEND_PORTAUDIO=OFF
                   -DALSOFT_BACKEND_SDL2=OFF -DALSOFT_BACKEND_SNDIO=OFF -DALSOFT_BACKEND_WAVE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
