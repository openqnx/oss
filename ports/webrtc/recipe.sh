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
