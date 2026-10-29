VERSION=0-git-84a8011; GIT=https://pdfium.googlesource.com/pdfium.git; COMMIT=84a8011ec69d0e2de271c05be7d62979608040d9
# PDFium (Chromium's PDF engine), at the upstream commit qnx-ports' QNX port
# is based on. Google's toolchain: depot_tools, fetched into
# hosttools/depot_tools. qnx-patches/0000: PDFium's DEPS for QNX (QNX dependency
# set, hooks applying qnx-ports/build-files' patch series -- pinned); the
# series themselves are kept in qnx-patches/<subproject>/ for review.
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
# As ports/webrtc: resumable stages; the patched DEPS committed as the
# "[QNX Base]" commit qnx-ports' apply_patches.py hook applies its series onto
# (otherwise none of the QNX patches were applied -- "linux/limits.h: No such
# file");
# and the commits the hooks check out fetched into the shallow clones first.
_qnx_hook_commits() {   # <DEPS file> -> "path sha" lines
  python3 - "$1" <<'PY'
import re, sys
for p, c in re.findall(r"'--path',\s*'([^']+)',\s*'--sha',\s*'([0-9a-f]+)'", open(sys.argv[1]).read()):
    print(p, c)
PY
}
do_fetch() {
  _dt || return 1
  local top="$SRC_DIR/pdfium-gclient" p c; mkdir -p "$top"
  if [ ! -f "$top/.qnx-checkout" ]; then
    rm -rf "$top/pdfium" && git init -q "$top/pdfium" && git -C "$top/pdfium" remote add origin "$GIT" && \
    git -C "$top/pdfium" fetch -q --depth 1 origin "$COMMIT" && git -C "$top/pdfium" -c advice.detachedHead=false checkout -q FETCH_HEAD && \
    git -C "$top/pdfium" apply "$RECIPE_DIR/pdfium/qnx-patches/0000-qnx-deps-from-qnx-ports.patch" && \
    git -C "$top/pdfium" -c user.name=qnxbuild -c user.email=qnxbuild@localhost commit -q -am "[QNX Base] dependencies from qnx-ports" && \
    touch "$top/.qnx-checkout" || return 1
  fi
  ( cd "$top" && { [ -f .gclient ] || { gclient config --name=pdfium --unmanaged --custom-var checkout_qnx=True "$GIT" && \
      echo 'target_os = ["qnx"]' >> .gclient; }; } && gclient sync --no-history --nohooks -D ) || return 1
  _qnx_hook_commits "$top/pdfium/DEPS" | while read -r p c; do
    git -C "$top/$p" fetch -q --depth 1 origin "$c" || exit 1
  done || return 1
  ( cd "$top" && gclient runhooks ) || return 1
  ln -sfn "$top/pdfium" "$SRCROOT"
}
# qnx-ports' makefile builds PDFium's test programs (pdfium_all) for both
# architectures and installs nothing; the library comes from a second step in
# each build directory: pdf_is_complete_lib makes libpdfium.a self-contained
# (all of PDFium's own static dependencies inside), which is what an
# application links against.
do_build() {
  _dt || return 1
  [ -f "$S/out/.qnx-built" ] || { make -C "$S" && touch "$S/out/.qnx-built"; } || return 1
  local d; d=$(ls -d "$S"/out/nto-${QNX_CPU}* 2>/dev/null | head -1)
  [ -n "$d" ] || { echo "no build directory for $QNX_CPU in $S/out"; return 1; }
  grep -q '^pdf_is_complete_lib' "$d/args.gn" || echo 'pdf_is_complete_lib = true' >> "$d/args.gn"
  ( cd "$S" && gn gen "$d" && autoninja -C "$d" pdfium )
}
do_install() {
  local d; d=$(ls -d "$S"/out/nto-${QNX_CPU}* 2>/dev/null | head -1)
  [ -n "$d" ] || { echo "no build output for $QNX_CPU in $S/out"; return 1; }
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/pdfium" && \
  # The build makes a shared or a static library, not always both: copy what
  # is there (the check below requires one).
  { cp -a "$d"/libpdfium* "$d"/obj/libpdfium.a "$STAGE$PREFIX/lib/" 2>/dev/null || true; }; cp -a "$S"/public/*.h "$S/public/cpp" "$STAGE$PREFIX/include/pdfium/"
  ls "$STAGE$PREFIX/lib" | grep -q pdfium || return 1
  # The complete static library carries its debug information: 8 GB, which
  # made pdfium-dev a 1.9 GB download -- just under GitHub's 2 GiB limit per
  # release asset. Without it the library is what an application links.
  [ ! -f "$STAGE$PREFIX/lib/libpdfium.a" ] || "$QNX_HOST/usr/bin/$QNX_TRIPLE-strip" --strip-debug "$STAGE$PREFIX/lib/libpdfium.a"
}
