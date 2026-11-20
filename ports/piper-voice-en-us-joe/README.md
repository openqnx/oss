# piper-voice-en-us-joe 1.0.0

> Piper voice en_US-joe-medium: U.S. English, male, 22050 Hz (CC0 recordings)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_piper`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `piper-voice-en-us-joe`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`piper-voice-en-us-joe`](https://openqnx.github.io/oss/package/x86_64/piper-voice-en-us-joe.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/piper-voice-en-us-joe.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add piper-voice-en-us-joe` (dependencies come along); one version: `apk add piper-voice-en-us-joe=<version>` | `apk del piper-voice-en-us-joe` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install piper-voice-en-us-joe` (dependencies first) | `oss-get remove piper-voice-en-us-joe` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-piper && /usr/local/bin/qnx-port-tests/test_piper    # its on-target test, from tests-8.0-<arch>-m-z
apk del piper-voice-en-us-joe qnx-test-piper
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/piper-voice-en-us-joe.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/piper-voice-en-us-joe.list
rm var/lib/manual/piper-voice-en-us-joe.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild piper-voice-en-us-joe` performs exactly the same steps (2–5) in one command.

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

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() {
  local e f sum c="$DIST_DIR/piper-voice-$VOICE-$VOICE_REV"
  mkdir -p "$c" "$SRCROOT" || return 1
  for e in $VOICE_FILES; do
    f=${e%%:*}; sum=${e##*:}
    [ -f "$c/$f" ] || curl -fsSL --retry 3 -o "$c/$f" "$VOICE_URL/$f" || { rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    echo "$sum  $c/$f" | sha256sum -c - || { echo "!! checksum mismatch: $f"; rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    cp "$c/$f" "$SRCROOT/$f" || return 1
  done
}
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/piper-voice-en-us-joe; B=build/$QNX_ARCH/piper-voice-en-us-joe; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local d="$STAGE$PREFIX/share/piper-voices"
  install -Dm644 "$S/$VOICE.onnx" "$d/$VOICE.onnx" &&
  install -m644 "$S/$VOICE.onnx.json" "$d/$VOICE.onnx.json" &&
  install -m644 "$S/MODEL_CARD" "$d/$VOICE.MODEL_CARD"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- piper-voice-en-us-joe: a voice for Piper (ports/piper): U.S. English, one male speaker, "medium" quality, 22050 Hz -- en_US-joe-medium.onnx (63 MB, the VITS model) and en_US-joe-medium.onnx.json (its configuration: the phoneme table, the espeak-ng voice "en-us", inference defaults), from the Piper project's voice collection (huggingface.co/rhasspy/piper-voices), installed in share/piper-voices/ with the model card. echo 'Hello.' | piper_exe -m /usr/local/share/piper-voices/en_US-joe-medium.onnx -f hello.wav License: the collection is published under the MIT license; this voice's recordings are CC0 (github.com/OHF-Voice/voice-datasets) and it was fine-tuned from a base model that is CC0 as well (MODEL_CARD, installed next to the model). It was chosen for that: most of the collection's small English voices were fine-tuned from a voice whose recordings are CC BY-NC-SA or have no stated license. Data, the same for both architectures; nothing is compiled. The files are not in a tarball or a git repository that could be cloned (the collection is 100+ GB of git-lfs): do_fetch downloads the three files at a pinned revision of the collection and checks each SHA-256. The model needs Piper 1.5 or newer (model card). VERSION: the collection has no per-voice versions; 1.0.0 is this component's own, raised when the pinned revision changes the files.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH piper-voice-en-us-joe`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/piper-voice-en-us-joe` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa piper-voice-en-us-joe
ls dists/8.0/*/ | grep '^piper-voice-en-us-joe-'
```

This writes `piper-voice-en-us-joe-<ver>-r0.tar.gz` (run time) and `piper-voice-en-us-joe-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add piper-voice-en-us-joe
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/piper-voice-en-us-joe-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/piper-voice-en-us-joe-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_piper`](../../tests/src/) links piper-voice-en-us-joe the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-piper`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-piper
/usr/local/bin/qnx-port-tests/test_piper
```

```
== piper-voice-en-us-joe ==
  ok   ...
PASS piper-voice-en-us-joe
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_piper` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_piper`](../../tests/src/) -- a program linked against piper-voice-en-us-joe the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - piper_version() is 1.8.0
   - voice en_US-joe-medium installed (piper-voice-en-us-joe)
   - piper_exe installed
   - a missing model is refused
   - piper_create: model, configuration and espeak-ng data loaded
   - defaults from the voice's configuration (length 1, noise 0.5)
   - two sentences: two chunks, the last with PIPER_DONE
   - 22050 Hz
   - phonemes of \"Hello world.\" (en-us, IPA)
   - phonemes of \"This is a test.\"
   - begin and end marks around them
   - phoneme ids: two per phoneme
   - \"Hello world.\": between 0.4 s and 4 s
   - \"This is a test.\": between 0.4 s and 4 s
   - samples finite and within [-1.5, 1.5]
   - real signal in both chunks
   - one sentence at length_scale 2: one chunk
   - clearly longer than at length_scale 1 (more than 1.4 times)
   - echo 'Hello world.' | piper_exe -m <voice> -f out.wav exits 0
   - WAV file written
   - RIFF / WAVE / fmt / data chunks
   - IEEE float, mono, 32 bit
   - 22050 Hz, 88200 bytes per second, 4 bytes per frame
   - stream header: length fields \"unknown\"
   - the data is whole frames
   - ... and 2 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | piper 1.8.0-r1 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/piper-1.8.0-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | piper-voice-en-us-joe 1.0.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/piper-voice-en-us-joe-1.0.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | piper 1.8.0-r1 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/piper-1.8.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | piper-voice-en-us-joe 1.0.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/piper-voice-en-us-joe-1.0.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add piper-voice-en-us-joe=<version>`), sources in `ported/piper-voice-en-us-joe/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates piper-voice-en-us-joe`; port one with `scripts/port-update piper-voice-en-us-joe`.

<details><summary>recipe.sh</summary>

```bash
# piper-voice-en-us-joe: a voice for Piper (ports/piper): U.S. English, one
# male speaker, "medium" quality, 22050 Hz -- en_US-joe-medium.onnx (63 MB,
# the VITS model) and en_US-joe-medium.onnx.json (its configuration: the
# phoneme table, the espeak-ng voice "en-us", inference defaults), from the
# Piper project's voice collection (huggingface.co/rhasspy/piper-voices),
# installed in share/piper-voices/ with the model card.
#   echo 'Hello.' | piper_exe -m /usr/local/share/piper-voices/en_US-joe-medium.onnx -f hello.wav
#
# License: the collection is published under the MIT license; this voice's
# recordings are CC0 (github.com/OHF-Voice/voice-datasets) and it was
# fine-tuned from a base model that is CC0 as well (MODEL_CARD, installed
# next to the model). It was chosen for that: most of the collection's small
# English voices were fine-tuned from a voice whose recordings are
# CC BY-NC-SA or have no stated license.
#
# Data, the same for both architectures; nothing is compiled. The files are
# not in a tarball or a git repository that could be cloned (the collection
# is 100+ GB of git-lfs): do_fetch downloads the three files at a pinned
# revision of the collection and checks each SHA-256. The model needs Piper
# 1.5 or newer (model card).
# VERSION: the collection has no per-voice versions; 1.0.0 is this component's
# own, raised when the pinned revision changes the files.
VERSION=1.0.0
DEPS=""
BUILDSYS=custom
VOICE=en_US-joe-medium
VOICE_REV=ae9d8352b73c1b1750748843e753a504b00e05b0
VOICE_URL=https://huggingface.co/rhasspy/piper-voices/resolve/$VOICE_REV/en/en_US/joe/medium
VOICE_FILES="$VOICE.onnx:abf4b998e34045cbd00eaf050de2de620f81ded6eb8b85a24b022a1c2b3f08d2
$VOICE.onnx.json:7b5f0293c0bf5bda8e377be1696da84af798befeef31038e43beefc01fe858da
MODEL_CARD:4b319248135916e8f5433ce7a9fe5271026b05cb598bab05807b22cb17fd129b"
do_fetch() {
  local e f sum c="$DIST_DIR/piper-voice-$VOICE-$VOICE_REV"
  mkdir -p "$c" "$SRCROOT" || return 1
  for e in $VOICE_FILES; do
    f=${e%%:*}; sum=${e##*:}
    [ -f "$c/$f" ] || curl -fsSL --retry 3 -o "$c/$f" "$VOICE_URL/$f" || { rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    echo "$sum  $c/$f" | sha256sum -c - || { echo "!! checksum mismatch: $f"; rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    cp "$c/$f" "$SRCROOT/$f" || return 1
  done
}
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/piper-voices"
  install -Dm644 "$S/$VOICE.onnx" "$d/$VOICE.onnx" &&
  install -m644 "$S/$VOICE.onnx.json" "$d/$VOICE.onnx.json" &&
  install -m644 "$S/MODEL_CARD" "$d/$VOICE.MODEL_CARD"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
