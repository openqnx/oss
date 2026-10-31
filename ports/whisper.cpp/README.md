# whisper.cpp 0_git20260728

> OpenAI Whisper speech recognition inference in C/C++

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ggml-org/whisper.cpp.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_whisper`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `whisper.cpp`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`whisper.cpp`](https://openqnx.github.io/oss/package/x86_64/whisper.cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/whisper.cpp.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add whisper.cpp` (dependencies come along); one version: `apk add whisper.cpp=<version>` | `apk del whisper.cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install whisper.cpp` (dependencies first) | `oss-get remove whisper.cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-whisper && /usr/local/bin/qnx-port-tests/test_whisper    # its on-target test
apk del whisper.cpp qnx-test-whisper
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/whisper.cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/whisper.cpp.list
rm var/lib/manual/whisper.cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild whisper.cpp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/ggml-org/whisper.cpp.git src/whisper.cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/whisper.cpp < ports/whisper.cpp/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — whisper.cpp on QNX, from QNX's port (github.com/qnx-ports/whisper.cpp,

> branch qnx-8.0): ggml recognises QNX's aarch64le processor name (the CPU
> backend's Arm features are chosen from it), the server example links
> QNX's libsocket, and the Vulkan backend's QNX changes (not built here: the
> SDP has no Vulkan). QNX's hand-over notes and its test fix are left out.

Touches: `examples/server/CMakeLists.txt`, `ggml/cmake/common.cmake`, `ggml/src/ggml-vulkan/CMakeLists.txt`, `ggml/src/ggml-vulkan/ggml-vulkan.cpp`, `ggml/src/ggml-vulkan/vulkan-shaders/vulkan-shaders-gen.cpp` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/whisper.cpp; B=build/$QNX_ARCH/whisper.cpp; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  # The Silero voice-activity model the project tests with and its JFK
  # speech sample (both small): to try it, and for the test.
  install -Dm644 "$S/models/for-tests-silero-v6.2.0-ggml.bin" "$STAGE$OPT/share/models/ggml-silero-v6.2.0.bin" || return 1
  install -Dm644 "$S/samples/jfk.wav" "$STAGE$OPT/share/samples/jfk.wav" || return 1
  for p in whisper-cli whisper-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/whisper.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- whisper.cpp: OpenAI Whisper speech recognition in C/C++ on ggml, CPU inference (the library, whisper-cli, the server), with QNX's changes (patches/0001, from github.com/qnx-ports/whisper.cpp; the commit QNX's port is based on).
- It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and whisper.cpp each do, in versions that are not interchangeable: each gets a prefix of its own, /usr/local/opt/whisper.cpp, its commands linked into bin/. The run path is that prefix's lib/, absolute: QNX's loader does not expand $ORIGIN (docs/porting-hazards.md).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH whisper.cpp`.

### 5. What it installs

54 files under `/usr/local`, 2 programs.

```
/usr/local/bin/whisper-cli
/usr/local/bin/whisper-server
```

The complete list is `stage/<arch>/.manifest/whisper.cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa whisper.cpp
ls dists/8.0/*/ | grep '^whisper.cpp-'
```

This writes `whisper.cpp-<ver>-r0.tar.gz` (run time) and `whisper.cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add whisper.cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/whisper.cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/whisper.cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_whisper`](../../tests/src/) links whisper.cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-whisper`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-whisper
/usr/local/bin/qnx-port-tests/test_whisper
```

```
== whisper.cpp ==
  ok   ...
PASS whisper.cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_whisper` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_whisper`](../../tests/src/) -- a program linked against whisper.cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - language table (99+ languages)
   - en is language 0
   - fr known
   - jfk.wav: 16 kHz, 11 s
   - Silero VAD model loaded
   - speech found, in more than one segment
   - speech starts in the first second
   - speech ends between 9 s and the end (11 s)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | whisper.cpp 0_git20260728-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/whisper.cpp-0_git20260728-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | whisper.cpp 0_git20260728-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/whisper.cpp-0_git20260728-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_git20260728**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add whisper.cpp=<version>`), sources in `ported/whisper.cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates whisper.cpp`; port one with `scripts/port-update whisper.cpp`.

<details><summary>recipe.sh</summary>

```bash
# whisper.cpp: OpenAI Whisper speech recognition in C/C++ on ggml, CPU
# inference (the library, whisper-cli, the server), with QNX's changes
# (patches/0001, from github.com/qnx-ports/whisper.cpp; the commit QNX's
# port is based on).
VERSION=0_git20260728; GIT=https://github.com/ggml-org/whisper.cpp.git; COMMIT=97c56f1dc1d1100a9d859c865a20c82d22f823ed
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/whisper.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
OPT="$PREFIX/opt/whisper.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DBUILD_SHARED_LIBS=ON -DGGML_STATIC=OFF -DWHISPER_BUILD_TESTS=OFF -DWHISPER_BUILD_EXAMPLES=ON
                   -DWHISPER_BUILD_SERVER=ON -DWHISPER_SDL2=OFF -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF -DWHISPER_CURL=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  # The Silero voice-activity model the project tests with and its JFK
  # speech sample (both small): to try it, and for the test.
  install -Dm644 "$S/models/for-tests-silero-v6.2.0-ggml.bin" "$STAGE$OPT/share/models/ggml-silero-v6.2.0.bin" || return 1
  install -Dm644 "$S/samples/jfk.wav" "$STAGE$OPT/share/samples/jfk.wav" || return 1
  for p in whisper-cli whisper-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/whisper.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
