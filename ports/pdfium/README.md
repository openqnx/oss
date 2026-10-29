# pdfium 0-git-84a8011

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://pdfium.googlesource.com/pdfium.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pdfium
oss-get install pdfium-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pdfium` performs exactly the same steps (2–5) in one command.

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
    https://pdfium.googlesource.com/pdfium.git src/pdfium
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`qnx-patches/`](qnx-patches/) — 30 patch series for sub-projects (applied after the sub-projects are fetched)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pdfium; B=build/$QNX_ARCH/pdfium; mkdir -p $B
```

```bash
do_build() {
  _dt || return 1
  [ -f "$S/out/.qnx-built" ] || { make -C "$S" && touch "$S/out/.qnx-built"; } || return 1
  local d; d=$(ls -d "$S"/out/nto-${QNX_CPU}* 2>/dev/null | head -1)
  [ -n "$d" ] || { echo "no build directory for $QNX_CPU in $S/out"; return 1; }
  grep -q '^pdf_is_complete_lib' "$d/args.gn" || echo 'pdf_is_complete_lib = true' >> "$d/args.gn"
  ( cd "$S" && gn gen "$d" && autoninja -C "$d" pdfium )
}
```

```bash
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
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- PDFium (Chromium's PDF engine), at the upstream commit qnx-ports' QNX port is based on. Google's toolchain: depot_tools, fetched into hosttools/depot_tools. qnx-patches/0000: PDFium's DEPS for QNX (QNX dependency set, hooks applying qnx-ports/build-files' patch series -- pinned); the series themselves are kept in qnx-patches/<subproject>/ for review.
- As ports/webrtc: resumable stages; the patched DEPS committed as the "[QNX Base]" commit qnx-ports' apply_patches.py hook applies its series onto (otherwise none of the QNX patches were applied -- "linux/limits.h: No such file"); and the commits the hooks check out fetched into the shallow clones first.
- qnx-ports' makefile builds PDFium's test programs (pdfium_all) for both architectures and installs nothing; the library comes from a second step in each build directory: pdf_is_complete_lib makes libpdfium.a self-contained (all of PDFium's own static dependencies inside), which is what an application links against.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pdfium`.

### 5. What it installs

25 files under `/usr/local`, 24 headers.

The complete list is `stage/<arch>/.manifest/pdfium` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pdfium
ls repo/8.0/*/ | grep '^pdfium-'
```

This writes `pdfium-<ver>-r0.tar.gz` (run time) and `pdfium-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pdfium
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pdfium-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pdfium-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
