# webrtc m132-afaf497

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://webrtc.googlesource.com/src.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add webrtc
apk add webrtc-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install webrtc`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild webrtc` performs exactly the same steps (2–5) in one command.

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
    https://webrtc.googlesource.com/src.git src/webrtc
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`qnx-patches/`](qnx-patches/) — 60 patch series for sub-projects (applied after the sub-projects are fetched)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/webrtc; B=build/$QNX_ARCH/webrtc; mkdir -p $B
```

```bash
do_build() {
  _dt || return 1
  # The QNX make target builds both architectures and both network/audio stack
  # combinations; the result for this architecture is copied in do_install.
  [ -f "$S/out/.qnx-built" ] || { make -C "$S" && touch "$S/out/.qnx-built"; }
}
```

```bash
do_install() {
  local d; d=$(ls -d "$S"/out/nto-${QNX_CPU}*-iosock-iosnd 2>/dev/null | head -1)
  [ -n "$d" ] || { echo "no build output for $QNX_CPU in $S/out"; return 1; }
  install -d "$STAGE$PREFIX/lib/webrtc" "$STAGE$PREFIX/include/webrtc" && \
  # Static and shared outputs and the example programs vary with the build
  # arguments: copy what is there (the check below requires something).
  { cp -a "$d"/*.a "$d"/*.so "$STAGE$PREFIX/lib/webrtc/" 2>/dev/null || true; }
  { cp -a "$d"/peerconnection_* "$STAGE$PREFIX/lib/webrtc/" 2>/dev/null || true; }
  ( cd "$S" && find api rtc_base modules media pc call common_video system_wrappers -name '*.h' -print0 | \
      cpio -0pdm "$STAGE$PREFIX/include/webrtc" ) 2>/dev/null || true   # find: some dirs vary by version
  [ -n "$(ls "$STAGE$PREFIX/lib/webrtc")" ]
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- WebRTC (native C++ library and examples), Chromium milestone M132 -- the base of qnx-ports' QNX port. Google's toolchain: depot_tools (gclient, gn, ninja), fetched into hosttools/depot_tools. qnx-patches/0000-qnx-deps-from-qnx-ports.patch  WebRTC's DEPS for QNX: the QNX dependency set and hooks that apply the per-subproject QNX patch series of qnx-ports/build-files (pinned) after gclient sync. qnx-patches/<subproject>/*.patch                 those series, kept here too for reference and review (the hooks apply the copies in qnx_build). gclient sync downloads ~20 GB; the build takes about an hour on 12 cores.
- Each stage is skipped when already done, so a failed fetch (a 20 GB gclient sync) can be resumed; re-running "git remote add" on a half-made tree used to fail with "remote origin already exists". The patched DEPS is committed as "[QNX Base] ...": qnx-ports' apply_patches.py hook applies the QNX patch series only on top of a clean tree whose HEAD is the commit so labelled -- in qnx-ports' own workflow the DEPS change is that commit of their fork. Left uncommitted ("the local repo is not clean") or committed under another message ("unexpected head sha"), it applied nothing. Its checkout_third_party_version.py hooks move some third-party repositories (libvpx, libaom) to older commits; the sync is --no-history, so every commit those hooks name is fetched into its shallow clone before the hooks run ("reference is not a tree" otherwise).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH webrtc`.

### 5. What it installs

1579 files under `/usr/local`, 1575 headers.

The complete list is `stage/<arch>/.manifest/webrtc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa webrtc
ls dists/8.0/*/ | grep '^webrtc-'
```

This writes `webrtc-<ver>-r0.tar.gz` (run time) and `webrtc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add webrtc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/webrtc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/webrtc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | webrtc 0_git-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | webrtc 0_git-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **m132-afaf497**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add webrtc=<version>`), sources in `ported/webrtc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates webrtc`; port one with `scripts/port-update webrtc`.

<details><summary>recipe.sh</summary>

```bash
VERSION=m132-afaf497; GIT=https://webrtc.googlesource.com/src.git; COMMIT=afaf497805cbb502da89991c2dcd783201efdd08
# WebRTC (native C++ library and examples), Chromium milestone M132 -- the
# base of qnx-ports' QNX port. Google's toolchain: depot_tools (gclient, gn,
# ninja), fetched into hosttools/depot_tools.
#
#   qnx-patches/0000-qnx-deps-from-qnx-ports.patch  WebRTC's DEPS for QNX: the QNX
#       dependency set and hooks that apply the per-subproject QNX patch
#       series of qnx-ports/build-files (pinned) after gclient sync.
#   qnx-patches/<subproject>/*.patch                 those series, kept here too for
#       reference and review (the hooks apply the copies in qnx_build).
#
# gclient sync downloads ~20 GB; the build takes about an hour on 12 cores.
BUILDSYS=custom
DEPOT_TOOLS="$WS/hosttools/depot_tools"
_dt() {
  [ -x "$DEPOT_TOOLS/gclient" ] || git clone -q https://chromium.googlesource.com/chromium/tools/depot_tools.git "$DEPOT_TOOLS"
  export PATH="$DEPOT_TOOLS:$PATH" DEPOT_TOOLS_UPDATE=0
  # DEPOT_TOOLS_UPDATE=0 keeps the checkout as it is, and also skips the
  # one-time bootstrap (its Python, the gn/ninja wrappers' setup): gn then
  # stops with "python3_bin_reldir.txt not found". Bootstrap once here.
  [ -f "$DEPOT_TOOLS/python3_bin_reldir.txt" ] || "$DEPOT_TOOLS/ensure_bootstrap"
}
# Each stage is skipped when already done, so a failed fetch (a 20 GB gclient
# sync) can be resumed; re-running "git remote add" on a half-made tree used
# to fail with "remote origin already exists".
# The patched DEPS is committed as "[QNX Base] ...": qnx-ports' apply_patches.py
# hook applies the QNX patch series only on top of a clean tree whose HEAD is
# the commit so labelled -- in qnx-ports' own workflow the DEPS change is that
# commit of their fork. Left uncommitted ("the local repo is not clean") or
# committed under another message ("unexpected head sha"), it applied nothing.
# Its checkout_third_party_version.py hooks move some third-party
# repositories (libvpx, libaom) to older commits; the sync is --no-history,
# so every commit those hooks name is fetched into its shallow clone before
# the hooks run ("reference is not a tree" otherwise).
_qnx_hook_commits() {   # <DEPS file> -> "path sha" lines
  python3 - "$1" <<'PY'
import re, sys
for p, c in re.findall(r"'--path',\s*'([^']+)',\s*'--sha',\s*'([0-9a-f]+)'", open(sys.argv[1]).read()):
    print(p, c)
PY
}
do_fetch() {
  _dt || return 1
  local top="$SRC_DIR/webrtc-gclient" p c; mkdir -p "$top"
  if [ ! -f "$top/.qnx-checkout" ]; then
    rm -rf "$top/src" && git init -q "$top/src" && git -C "$top/src" remote add origin "$GIT" && \
    git -C "$top/src" fetch -q --depth 1 origin "$COMMIT" && git -C "$top/src" -c advice.detachedHead=false checkout -q FETCH_HEAD && \
    git -C "$top/src" apply "$RECIPE_DIR/webrtc/qnx-patches/0000-qnx-deps-from-qnx-ports.patch" && \
    git -C "$top/src" -c user.name=qnxbuild -c user.email=qnxbuild@localhost commit -q -am "[QNX Base] dependencies from qnx-ports" && \
    touch "$top/.qnx-checkout" || return 1
  fi
  ( cd "$top" && { [ -f .gclient ] || { gclient config --name=src --unmanaged --custom-var checkout_qnx=True "$GIT" && \
      echo 'target_os = ["qnx"]' >> .gclient; }; } && gclient sync --no-history --nohooks -D ) || return 1
  _qnx_hook_commits "$top/src/DEPS" | while read -r p c; do
    git -C "$top/$p" fetch -q --depth 1 origin "$c" || exit 1
  done || return 1
  ( cd "$top" && gclient runhooks ) || return 1
  ln -sfn "$top/src" "$SRCROOT"
}
do_build() {
  _dt || return 1
  # The QNX make target builds both architectures and both network/audio stack
  # combinations; the result for this architecture is copied in do_install.
  [ -f "$S/out/.qnx-built" ] || { make -C "$S" && touch "$S/out/.qnx-built"; }
}
do_install() {
  local d; d=$(ls -d "$S"/out/nto-${QNX_CPU}*-iosock-iosnd 2>/dev/null | head -1)
  [ -n "$d" ] || { echo "no build output for $QNX_CPU in $S/out"; return 1; }
  install -d "$STAGE$PREFIX/lib/webrtc" "$STAGE$PREFIX/include/webrtc" && \
  # Static and shared outputs and the example programs vary with the build
  # arguments: copy what is there (the check below requires something).
  { cp -a "$d"/*.a "$d"/*.so "$STAGE$PREFIX/lib/webrtc/" 2>/dev/null || true; }
  { cp -a "$d"/peerconnection_* "$STAGE$PREFIX/lib/webrtc/" 2>/dev/null || true; }
  ( cd "$S" && find api rtc_base modules media pc call common_video system_wrappers -name '*.h' -print0 | \
      cpio -0pdm "$STAGE$PREFIX/include/webrtc" ) 2>/dev/null || true   # find: some dirs vary by version
  [ -n "$(ls "$STAGE$PREFIX/lib/webrtc")" ]
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
