# webrtc-screencast 1.1

> Watch the desktop in a web browser over WebRTC (view only)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`webrtc`](../webrtc/) [`wayland`](../wayland/) [`wlr-protocols`](../wlr-protocols/) [`libxkbcommon`](../libxkbcommon/) |
| Run-time dependencies | [`libxkbcommon`](../libxkbcommon/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_webrtc_screencast`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `webrtc-screencast`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`webrtc-screencast`](https://openqnx.github.io/oss/package/x86_64/webrtc-screencast.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/webrtc-screencast.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add webrtc-screencast` (dependencies come along); one version: `apk add webrtc-screencast=<version>` | `apk del webrtc-screencast` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install webrtc-screencast` (dependencies first) | `oss-get remove webrtc-screencast` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-webrtc-screencast && /usr/local/bin/qnx-port-tests/test_webrtc_screencast    # its on-target test, from tests-8.0-<arch>-m-z
apk del webrtc-screencast qnx-test-webrtc-screencast
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/webrtc-screencast.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/webrtc-screencast.list
rm var/lib/manual/webrtc-screencast.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild webrtc-screencast` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild webrtc wayland wlr-protocols libxkbcommon
```

<details><summary>Full build chain (20)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`webrtc`](../webrtc/) [`wlr-protocols`](../wlr-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/webrtc-screencast/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/webrtc-screencast; B=build/$QNX_ARCH/webrtc-screencast; mkdir -p $B
```

```bash
do_build() {
  local W="$STAGE$PREFIX/include/webrtc" x="$STAGE$PREFIX/share/wlr-protocols/unstable/wlr-screencopy-unstable-v1.xml"
  local wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$wls" ] || wls=$(command -v wayland-scanner)
  [ -f "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" ] || { echo "the webrtc component has no libwebrtc.a (rebuild webrtc)"; return 1; }
  mkdir -p "$B" && cd "$B" && \
  local n f; for n in wlr-screencopy-unstable-v1 wlr-virtual-pointer-unstable-v1 virtual-keyboard-unstable-v1; do
    f="$(dirname "$x")/$n.xml"; [ -f "$f" ] || f="$S/$n.xml"    # virtual-keyboard: wlroots' copy, in files/
    "$wls" client-header "$f" "$n-client-protocol.h" && "$wls" private-code "$f" "$n-protocol.c" && \
    qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -I. -I"$STAGE$PREFIX/include" -c "$n-protocol.c" -o "$n-protocol.o" || return 1
  done
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/input.c" -o input.o && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/capture.c" -o capture.o && \
  q++ -V"${QCC_TARGET}_cxx" $_WDEFS -std=c++17 -fno-exceptions -fno-rtti -O2 -fPIC -fdata-sections -ffunction-sections \
      -Wall -Wno-deprecated-declarations -Wno-psabi \
      -I"$W" -I"$W/third_party/abseil-cpp" -I"$W/third_party/libyuv/include" -I"$QNX_TARGET/usr/include/io-sock" -I"$S" \
      -c "$S/main.cc" -o main.o && \
  # -lscreen -lEGL -lGLESv2: libwebrtc.a's own QNX camera and renderer code
  # (not used here, dropped by --gc-sections where nothing refers to it).
  q++ -V"${QCC_TARGET}_cxx" -Wl,--gc-sections -Wl,--as-needed -o webrtc-screencast main.o capture.o input.o *-protocol.o \
      "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" -L"$STAGE$PREFIX/lib" -lwayland-client -lxkbcommon \
      -lsocket -lm
}
```

```bash
do_install() {
  install -Dm755 "$B/webrtc-screencast" "$STAGE$PREFIX/bin/webrtc-screencast" && \
  install -Dm755 "$S/webrtc-screencast-session" "$STAGE$PREFIX/bin/webrtc-screencast-session" && \
  install -Dm644 "$S/webrtc-screencast.desktop" "$STAGE$PREFIX/etc/xdg/autostart/webrtc-screencast.desktop" && \
  install -Dm644 "$S/etc/webrtc-screencast.conf" "$STAGE$PREFIX/etc/webrtc-screencast.conf"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- webrtc-screencast: the desktop of the target in a web browser. A Wayland client that asks the compositor (labwc, any wlroots one) for the picture of an output (wlr-screencopy), sends it as a WebRTC video track (libwebrtc of the webrtc component, VP8 in software) and serves the page a browser opens, with the SDP exchange, from its own small HTTP server. View only, or with --input the browser's mouse and keyboard, sent over a data channel and given to the compositor's virtual pointer and keyboard (a US layout). Nothing of QNX Screen is used directly: the picture is the compositor's. This component is the program (files/): one static binary on libwebrtc, the session starter and its XDG autostart entry, so that a desktop image (oss-desktop) shows itself at http://<address>:8080/ once it is installed.
- The compile flags are those of the webrtc build itself (its obj/examples/peerconnection_client.ninja): libwebrtc.a is built without RTTI and exceptions, and its headers change layout with these macros.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH webrtc-screencast`.

### 5. What it installs

4 files under `/usr/local`, 2 programs.

```
/usr/local/bin/webrtc-screencast
/usr/local/bin/webrtc-screencast-session
```

The complete list is `stage/<arch>/.manifest/webrtc-screencast` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa webrtc-screencast
ls dists/8.0/*/ | grep '^webrtc-screencast-'
```

This writes `webrtc-screencast-<ver>-r0.tar.gz` (run time) and `webrtc-screencast-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add webrtc-screencast
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/webrtc-screencast-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/webrtc-screencast-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_webrtc_screencast`](../../tests/src/) links webrtc-screencast the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-webrtc-screencast`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-webrtc-screencast
/usr/local/bin/qnx-port-tests/test_webrtc_screencast
```

```
== webrtc-screencast ==
  ok   ...
PASS webrtc-screencast
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_webrtc_screencast` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_webrtc_screencast`](../../tests/src/) -- a program linked against webrtc-screencast the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - XDG_RUNTIME_DIR
   - labwc (headless) is up: the compositor whose picture is sent
   - GET /status: 200, nobody is watching yet
   - GET /: the page with the <video> and the WebRTC script, no STUN server by default
   - an unknown path: 404
   - a broken offer is refused (503)
   - POST /offer: 200 and an SDP answer
   - the answer sends video only
   - the answer's codec is VP8
   - the answer has its DTLS fingerprint and role
   - the answer has ICE credentials and gathered candidates
   - the answer names the track (msid)
   - GET /status: one viewer now
   - webrtc-screencast still running
   - with --token: a request without the token is refused (403)
   - with --token: a wrong token is refused (403)
   - with --token: the right token is let in (200)
   - with --input: the answer accepts the data channel
   - webrtc-screencast --input still running

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> webrtc-screencast`.

## Versions

This recipe builds **1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add webrtc-screencast=<version>`), sources in `ported/webrtc-screencast/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates webrtc-screencast`; port one with `scripts/port-update webrtc-screencast`.

<details><summary>recipe.sh</summary>

```bash
# webrtc-screencast: the desktop of the target in a web browser. A Wayland
# client that asks the compositor (labwc, any wlroots one) for the picture of
# an output (wlr-screencopy), sends it as a WebRTC video track (libwebrtc of
# the webrtc component, VP8 in software) and serves the page a browser
# opens, with the SDP exchange, from its own small HTTP server. View only, or
# with --input the browser's mouse and keyboard, sent over a data channel and
# given to the compositor's virtual pointer and keyboard (a US layout).
# Nothing of QNX Screen is used directly: the picture is the compositor's.
#
# This component is the program (files/): one static binary on libwebrtc,
# the session starter and its XDG autostart entry, so that a desktop image
# (oss-desktop) shows itself at http://<address>:8080/ once it is installed.
VERSION=1.1
DEPS="webrtc wayland wlr-protocols libxkbcommon"
RUNTIME_DEPS="wayland libxkbcommon xkeyboard-config"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/webrtc-screencast/files/." "$SRCROOT/"; }
# The compile flags are those of the webrtc build itself (its
# obj/examples/peerconnection_client.ninja): libwebrtc.a is built without
# RTTI and exceptions, and its headers change layout with these macros.
_WDEFS="-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D_FILE_OFFSET_BITS=64 \
-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DQNX -D_QNX_SOURCE -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE \
-DNDEBUG -DWEBRTC_STRICT_FIELD_TRIALS=0 -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE -DRTC_ENABLE_VP9 \
-DRTC_DAV1D_IN_INTERNAL_DECODER_FACTORY -DWEBRTC_HAVE_SCTP -DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=1 \
-DWEBRTC_POSIX -DWEBRTC_QNX -DABSL_ALLOCATOR_NOTHROW=1 -DLIBYUV_DISABLE_NEON -DLIBYUV_DISABLE_SVE \
-DLIBYUV_DISABLE_SME -DHAVE_WEBRTC_VIDEO"
do_build() {
  local W="$STAGE$PREFIX/include/webrtc" x="$STAGE$PREFIX/share/wlr-protocols/unstable/wlr-screencopy-unstable-v1.xml"
  local wls="$WS/hosttools/bin/wayland-scanner"; [ -x "$wls" ] || wls=$(command -v wayland-scanner)
  [ -f "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" ] || { echo "the webrtc component has no libwebrtc.a (rebuild webrtc)"; return 1; }
  mkdir -p "$B" && cd "$B" && \
  local n f; for n in wlr-screencopy-unstable-v1 wlr-virtual-pointer-unstable-v1 virtual-keyboard-unstable-v1; do
    f="$(dirname "$x")/$n.xml"; [ -f "$f" ] || f="$S/$n.xml"    # virtual-keyboard: wlroots' copy, in files/
    "$wls" client-header "$f" "$n-client-protocol.h" && "$wls" private-code "$f" "$n-protocol.c" && \
    qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -I. -I"$STAGE$PREFIX/include" -c "$n-protocol.c" -o "$n-protocol.o" || return 1
  done
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/input.c" -o input.o && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -fPIC -I. -I"$S" -I"$STAGE$PREFIX/include" \
      -c "$S/capture.c" -o capture.o && \
  q++ -V"${QCC_TARGET}_cxx" $_WDEFS -std=c++17 -fno-exceptions -fno-rtti -O2 -fPIC -fdata-sections -ffunction-sections \
      -Wall -Wno-deprecated-declarations -Wno-psabi \
      -I"$W" -I"$W/third_party/abseil-cpp" -I"$W/third_party/libyuv/include" -I"$QNX_TARGET/usr/include/io-sock" -I"$S" \
      -c "$S/main.cc" -o main.o && \
  # -lscreen -lEGL -lGLESv2: libwebrtc.a's own QNX camera and renderer code
  # (not used here, dropped by --gc-sections where nothing refers to it).
  q++ -V"${QCC_TARGET}_cxx" -Wl,--gc-sections -Wl,--as-needed -o webrtc-screencast main.o capture.o input.o *-protocol.o \
      "$STAGE$PREFIX/lib/webrtc/libwebrtc.a" -L"$STAGE$PREFIX/lib" -lwayland-client -lxkbcommon \
      -lsocket -lm
}
do_install() {
  install -Dm755 "$B/webrtc-screencast" "$STAGE$PREFIX/bin/webrtc-screencast" && \
  install -Dm755 "$S/webrtc-screencast-session" "$STAGE$PREFIX/bin/webrtc-screencast-session" && \
  install -Dm644 "$S/webrtc-screencast.desktop" "$STAGE$PREFIX/etc/xdg/autostart/webrtc-screencast.desktop" && \
  install -Dm644 "$S/etc/webrtc-screencast.conf" "$STAGE$PREFIX/etc/webrtc-screencast.conf"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
