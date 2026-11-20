# espeak-ng 1.52.0

> eSpeak NG speech synthesizer: library, espeak-ng program and voice data for 100+ languages

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/espeak-ng/espeak-ng.git (tag `1.52.0`) |
| Build system | CMake |
| QNX patches | 4 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_espeak_ng`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `espeak-ng` (programs, shared libraries, run-time data) and `espeak-ng-dev` (headers, pkg-config/CMake files, static libraries; needs `espeak-ng`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`espeak-ng`](https://openqnx.github.io/oss/package/x86_64/espeak-ng.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/espeak-ng.html)) · [`espeak-ng-dev`](https://openqnx.github.io/oss/package/x86_64/espeak-ng-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/espeak-ng-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add espeak-ng` (dependencies come along); one version: `apk add espeak-ng=<version>` | `apk del espeak-ng` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install espeak-ng` (dependencies first) | `oss-get remove espeak-ng` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`espeak-ng-dev` the same way, by its own name (`apk add espeak-ng-dev`, `oss-get install espeak-ng-dev`, by hand with its own file and list `espeak-ng-dev.list`); remove it before `espeak-ng`, which it needs.

```sh
# on the QNX target, as root
apk add espeak-ng espeak-ng-dev
apk add qnx-test-espeak-ng && /usr/local/bin/qnx-port-tests/test_espeak_ng    # its on-target test, from tests-8.0-<arch>
apk del espeak-ng espeak-ng-dev qnx-test-espeak-ng
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/espeak-ng.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/espeak-ng.list
rm var/lib/manual/espeak-ng.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild espeak-ng` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.52.0 --recurse-submodules --shallow-submodules \
    https://github.com/espeak-ng/espeak-ng.git src/espeak-ng
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/espeak-ng < ports/espeak-ng/patches/0001-qnx-endian.patch
patch -p1 -d src/espeak-ng < ports/espeak-ng/patches/0002-host-data-compiler.patch
patch -p1 -d src/espeak-ng < ports/espeak-ng/patches/0003-qnx-relocatable-data.patch
patch -p1 -d src/espeak-ng < ports/espeak-ng/patches/0004-backport-phonemes-with-terminator.patch
```

**[`0001-qnx-endian.patch`](patches/0001-qnx-endian.patch)** — Byte order conversions on QNX from <gulliver.h>.

> QNX has no <endian.h> or <sys/endian.h>, so the compatibility header ended
> in "#error platform not supported". The le16toh() family is declared in
> <net/netbyte.h>, but only as functions that live in libsocket; the same
> conversions are macros in <gulliver.h> (ENDIAN_LE16() ...), part of libc's
> headers, so the source asks for nothing outside libc and libm.

Touches: `src/include/compat/endian.h` · Upstream status: QNX-specific

**[`0002-host-data-compiler.patch`](patches/0002-host-data-compiler.patch)** — Let a native espeak-ng compile the data in a cross build.

> The voice data (phontab, phondata, intonations, the dictionaries) is
> compiled during the build by running the espeak-ng program that was just
> built. In a cross build that is a QNX executable and cannot run on the
> build machine. ESPEAK_NG_HOST_BIN names a native espeak-ng of the same
> version that does the compiling instead; the files it writes are the same
> (little endian on the build machine and on both QNX targets). Without the
> option the build behaves as before.

Touches: `cmake/data.cmake` · Upstream status: QNX-specific

**[`0003-qnx-relocatable-data.patch`](patches/0003-qnx-relocatable-data.patch)** — Find the data next to the library when the configured directory is missing.

> The data directory is compiled in (PATH_ESPEAK_DATA,
> /usr/local/share/espeak-ng-data). On QNX targets the packages are also
> installed under another root (/data/oss/usr/local), where that directory
> does not exist and every program using the library would need
> ESPEAK_DATA_PATH set. When the compiled-in directory is missing, the
> library now asks dladdr() -- in QNX's libc -- which file it was loaded
> from and uses <that file's prefix>/share/espeak-ng-data if it exists. An
> explicit path argument and ESPEAK_DATA_PATH keep their precedence.

Touches: `src/libespeak-ng/speech.c` · Upstream status: QNX-specific

**[`0004-backport-phonemes-with-terminator.patch`](patches/0004-backport-phonemes-with-terminator.patch)** — Backport espeak_TextToPhonemesWithTerminator().

> espeak_TextToPhonemes() does not tell the caller how a clause ended (full
> stop, comma, question mark ...), so a caller that needs the punctuation
> back -- Piper's voices are trained on phonemes with it -- cannot use the
> released library. Upstream added espeak_TextToPhonemesWithTerminator() for
> this after 1.52.0; this is that change, unmodified, so that libpiper links
> against this espeak-ng instead of building a private copy. It has nothing
> to do with QNX and goes away with the next release.

Touches: `src/include/espeak-ng/speak_lib.h`, `src/libespeak-ng/speech.c` · Upstream status: Backport (espeak-ng commit 2108b1e8, "Add espeak_TextToPhonemesWithTerminator (#2127)")

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/espeak-ng; B=build/$QNX_ARCH/espeak-ng; mkdir -p $B
```

```bash
do_configure() {
  if [ ! -f "$SONIC_DIR/sonic.c" ]; then
    local tb="$DIST_DIR/sonic-$SONIC_COMMIT.tar.gz"
    [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "https://github.com/waywardgeek/sonic/archive/$SONIC_COMMIT.tar.gz" || return 1
    echo "$SONIC_SHA256  $tb" | sha256sum -c - || return 1
    rm -rf "$SONIC_DIR"; mkdir -p "$SONIC_DIR" && tar xf "$tb" -C "$SONIC_DIR" --strip-components=1 || return 1
  fi
  # The native espeak-ng that compiles the data: the build machine's own
  # compiler, none of the cross environment.
  local HB="$BUILD_DIR/$QNX_ARCH/host/espeak-ng"
  if [ ! -x "$HB/src/espeak-ng" ]; then
    rm -rf "$HB"
    ( unset CC CXX CPP AR LD CFLAGS CXXFLAGS CPPFLAGS LDFLAGS PKG_CONFIG PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_PATH
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DBUILD_SHARED_LIBS=OFF -DUSE_ASYNC=OFF "${ESPEAK_ARGS[@]}" \
      && cmake --build "$HB" -j4 --target espeak-ng-bin ) || { echo "native espeak-ng build failed"; return 1; }
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIBS=ON -DPTHREAD_LIB=c -DUSE_ASYNC=ON \
    -DESPEAK_NG_HOST_BIN="$HB/src/espeak-ng" "${ESPEAK_ARGS[@]}"
}
```

```bash
do_build() { cmake --build "$B" -j4; }
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The licenses of the parts: GPL-3.0-or-later (espeak-ng), BSD-2-Clause
  # (ieee80.c), Apache-2.0 (sonic, getopt), Unicode (ucd-tools data).
  install -d "$STAGE$PREFIX/share/licenses/espeak-ng" || return 1
  install -m644 "$S"/COPYING "$S"/COPYING.APACHE "$S"/COPYING.BSD2 "$S"/COPYING.UCD "$STAGE$PREFIX/share/licenses/espeak-ng/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- espeak-ng: eSpeak NG, a compact formant speech synthesizer for more than a hundred languages: libespeak-ng (text to phonemes, text to PCM samples), the espeak-ng program and the compiled voice data (phoneme tables, one dictionary per language, voice and language definitions). What is different on QNX, and why: - No audio device output. pcaudiolib (ALSA/PulseAudio/OSS back ends) is not ported, so the program writes WAV files (-w, --stdout) and the library hands the samples to the application's callback (AUDIO_OUTPUT_RETRIEVAL, AUDIO_OUTPUT_SYNCHRONOUS). Playing them is the application's business. - The voice data is compiled by espeak-ng itself during the build. The QNX program cannot run on the build machine, so a native espeak-ng is built first from the same source (build/<arch>/host/espeak-ng, static, only the program) and compiles the data (patches/0002, ESPEAK_NG_HOST_BIN). The compiled files are little endian, like both QNX targets. - The data directory is found relative to the library when the configured /usr/local/share/espeak-ng-data does not exist (patches/0003): packages are also installed under /data/oss. ESPEAK_DATA_PATH still overrides. - sonic (faster speech rates, Apache-2.0) is not a component here; upstream fetches it with git while configuring. The same commit is downloaded as a checksummed archive instead and compiled into the library, as upstream does when no libsonic is installed. - MBROLA voices are left out (the mbrola program is not ported).
- The commit cmake/deps.cmake names for sonic.
- PTHREAD_LIB=c: upstream looks for a libpthread to decide whether the asynchronous API (espeak_Synth() returning at once, events, the command queue) can be built. QNX has no libpthread, the threads are in libc. USE_LIBPCAUDIO=OFF, USE_MBROLA=OFF: see above; stated so that nothing found in staging or on the build machine changes the result.
- -j4: other builds share the machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH espeak-ng`.

### 5. What it installs

380 files under `/usr/local`: 3 shared-library entries, 1 programs, 4 headers.

```
/usr/local/lib/libespeak-ng.so
/usr/local/lib/libespeak-ng.so.1
/usr/local/lib/libespeak-ng.so.1.52.0
/usr/local/bin/espeak-ng
```

The complete list is `stage/<arch>/.manifest/espeak-ng` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa espeak-ng
ls dists/8.0/*/ | grep '^espeak-ng-'
```

This writes `espeak-ng-<ver>-r0.tar.gz` (run time) and `espeak-ng-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add espeak-ng
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/espeak-ng-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/espeak-ng-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_espeak_ng`](../../tests/src/) links espeak-ng the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-espeak-ng`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-espeak-ng
/usr/local/bin/qnx-port-tests/test_espeak_ng
```

```
== espeak-ng ==
  ok   ...
PASS espeak-ng
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_espeak_ng` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_espeak_ng`](../../tests/src/) -- a program linked against espeak-ng the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - espeak_Initialize: data found, 22050 Hz
   - espeak_Info: version 1.x
   - an unknown voice is refused
   - three clauses in \"Is it done? Yes, it is.\"
   - terminators: question, comma, full stop
   - espeak_Synth (synchronous)
   - \"hello world\": between 0.5 s and 4 s of audio
   - the samples are not silent
   - two WORD events
   - default rate 175 words per minute
   - twice the rate: clearly fewer samples
   - espeak_Initialize (asynchronous retrieval)
   - espeak_Synth (asynchronous)
   - espeak_Synchronize: the queue drained
   - as many samples as synchronously (within 5 %)
   - asynchronous samples not silent, message terminated
   - espeak-ng program installed
   - espeak-ng -q -x 'hello world' -> h@l'oU w'3:ld
   - espeak-ng -w file.wav exits 0
   - WAV file written
   - RIFF / WAVE / fmt / data chunks
   - PCM, mono, 16 bit
   - 22050 Hz, 44100 bytes per second, 2 bytes per frame
   - RIFF and data lengths match the file size
   - at least as long as the library's synthesis, less than twice
   - ... and 1 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | espeak-ng 1.52.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/espeak-ng-1.52.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | espeak-ng 1.52.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/espeak-ng-1.52.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.52.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add espeak-ng=<version>`), sources in `ported/espeak-ng/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates espeak-ng`; port one with `scripts/port-update espeak-ng`.

<details><summary>recipe.sh</summary>

```bash
# espeak-ng: eSpeak NG, a compact formant speech synthesizer for more than a
# hundred languages: libespeak-ng (text to phonemes, text to PCM samples), the
# espeak-ng program and the compiled voice data (phoneme tables, one
# dictionary per language, voice and language definitions).
#
# What is different on QNX, and why:
# - No audio device output. pcaudiolib (ALSA/PulseAudio/OSS back ends) is not
#   ported, so the program writes WAV files (-w, --stdout) and the library
#   hands the samples to the application's callback (AUDIO_OUTPUT_RETRIEVAL,
#   AUDIO_OUTPUT_SYNCHRONOUS). Playing them is the application's business.
# - The voice data is compiled by espeak-ng itself during the build. The QNX
#   program cannot run on the build machine, so a native espeak-ng is built
#   first from the same source (build/<arch>/host/espeak-ng, static, only the
#   program) and compiles the data (patches/0002, ESPEAK_NG_HOST_BIN). The
#   compiled files are little endian, like both QNX targets.
# - The data directory is found relative to the library when the configured
#   /usr/local/share/espeak-ng-data does not exist (patches/0003): packages
#   are also installed under /data/oss. ESPEAK_DATA_PATH still overrides.
# - sonic (faster speech rates, Apache-2.0) is not a component here; upstream
#   fetches it with git while configuring. The same commit is downloaded as
#   a checksummed archive instead and compiled into the library, as upstream
#   does when no libsonic is installed.
# - MBROLA voices are left out (the mbrola program is not ported).
VERSION=1.52.0; GIT=https://github.com/espeak-ng/espeak-ng.git; REF=1.52.0
DEPS=""
BUILDSYS=cmake
# The commit cmake/deps.cmake names for sonic.
SONIC_COMMIT=fbf75c3d6d846bad3bb3d456cbc5d07d9fd8c104
SONIC_SHA256=715827b5a39b79e56e44397d7b845910df996d4cca74777b3b61629b1ddc98c1
SONIC_DIR="$SRC_DIR/sonic-espeak-ng-$SONIC_COMMIT"
# PTHREAD_LIB=c: upstream looks for a libpthread to decide whether the
# asynchronous API (espeak_Synth() returning at once, events, the command
# queue) can be built. QNX has no libpthread, the threads are in libc.
# USE_LIBPCAUDIO=OFF, USE_MBROLA=OFF: see above; stated so that nothing found
# in staging or on the build machine changes the result.
ESPEAK_ARGS=( -DFETCHCONTENT_SOURCE_DIR_SONIC-GIT="$SONIC_DIR" -DFETCHCONTENT_FULLY_DISCONNECTED=ON
              -DUSE_LIBSONIC=ON -DUSE_LIBPCAUDIO=OFF -DUSE_MBROLA=OFF -DUSE_KLATT=ON -DUSE_SPEECHPLAYER=ON
              -DCMAKE_DISABLE_FIND_PACKAGE_Git=ON )
do_configure() {
  if [ ! -f "$SONIC_DIR/sonic.c" ]; then
    local tb="$DIST_DIR/sonic-$SONIC_COMMIT.tar.gz"
    [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "https://github.com/waywardgeek/sonic/archive/$SONIC_COMMIT.tar.gz" || return 1
    echo "$SONIC_SHA256  $tb" | sha256sum -c - || return 1
    rm -rf "$SONIC_DIR"; mkdir -p "$SONIC_DIR" && tar xf "$tb" -C "$SONIC_DIR" --strip-components=1 || return 1
  fi
  # The native espeak-ng that compiles the data: the build machine's own
  # compiler, none of the cross environment.
  local HB="$BUILD_DIR/$QNX_ARCH/host/espeak-ng"
  if [ ! -x "$HB/src/espeak-ng" ]; then
    rm -rf "$HB"
    ( unset CC CXX CPP AR LD CFLAGS CXXFLAGS CPPFLAGS LDFLAGS PKG_CONFIG PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_PATH
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DBUILD_SHARED_LIBS=OFF -DUSE_ASYNC=OFF "${ESPEAK_ARGS[@]}" \
      && cmake --build "$HB" -j4 --target espeak-ng-bin ) || { echo "native espeak-ng build failed"; return 1; }
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIBS=ON -DPTHREAD_LIB=c -DUSE_ASYNC=ON \
    -DESPEAK_NG_HOST_BIN="$HB/src/espeak-ng" "${ESPEAK_ARGS[@]}"
}
# -j4: other builds share the machine.
do_build() { cmake --build "$B" -j4; }
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The licenses of the parts: GPL-3.0-or-later (espeak-ng), BSD-2-Clause
  # (ieee80.c), Apache-2.0 (sonic, getopt), Unicode (ucd-tools data).
  install -d "$STAGE$PREFIX/share/licenses/espeak-ng" || return 1
  install -m644 "$S"/COPYING "$S"/COPYING.APACHE "$S"/COPYING.BSD2 "$S"/COPYING.UCD "$STAGE$PREFIX/share/licenses/espeak-ng/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
