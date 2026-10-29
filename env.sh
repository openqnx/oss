#!/bin/bash
# QNX SDP 8.0 OSS porting workspace environment
# usage: source env.sh [aarch64le|x86_64]

export QNX_SDP_ROOT="${QNX_SDP_ROOT:-$HOME/qnx800}"
[ -f "$QNX_SDP_ROOT/qnxsdp-env.sh" ] || { echo "ERROR: QNX SDP not found at $QNX_SDP_ROOT -- export QNX_SDP_ROOT=<your SDP 8.0 install>" >&2; return 1; }

# This file is sourced twice per package by the driver, and both it and the
# SDP's qnxsdp-env.sh PREPEND to PATH (and ACLOCAL_PATH) each time.  At 218
# packages PATH passed the kernel's 128 KB limit for a single environment
# string, every exec failed with "Argument list too long" -- including the
# dirname that computes WS below, which then silently became the current
# directory -- and the second half of a clean rebuild reported "no recipe" for
# every package.  Remember the caller's values once and rebuild from them.
if [ -z "${QNX_PORTS_BASE_PATH+set}" ]; then
  export QNX_PORTS_BASE_PATH="$PATH" QNX_PORTS_BASE_ACLOCAL_PATH="${ACLOCAL_PATH-}"
fi
export PATH="$QNX_PORTS_BASE_PATH"
if [ -n "$QNX_PORTS_BASE_ACLOCAL_PATH" ]; then export ACLOCAL_PATH="$QNX_PORTS_BASE_ACLOCAL_PATH"; else unset ACLOCAL_PATH; fi

source "$QNX_SDP_ROOT/qnxsdp-env.sh" >/dev/null

export WS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f "$WS/env.sh" ] || { echo "ERROR: env.sh could not locate its workspace (WS=$WS)"; return 1; }
export SRC_DIR="${QNX_OSS_SRC_DIR:-$WS/src}"
export BUILD_DIR="${QNX_OSS_BUILD_DIR:-$WS/build}"
export STAGE_DIR="${QNX_OSS_STAGE_DIR:-$WS/stage}"
export LOG_DIR="${QNX_OSS_LOG_DIR:-$WS/logs}"
export RECIPE_DIR="$WS/ports"
export TOOLCHAIN_DIR="$WS/toolchain"
export DIST_DIR="$WS/dist"
export PKGROOT_DIR="${QNX_OSS_PKGROOT_DIR:-$WS/pkgroot}"

# Target prefix used *on the QNX target*; staging mirrors it per-arch on the host.
export PREFIX="${PREFIX:-/usr/local}"

# CMake 4 removed compatibility with cmake_minimum_required(VERSION < 3.5), which
# plenty of these projects still declare. The -D form only reaches the top-level
# configure; nested "cmake -P" script invocations (OpenCV's pkgconfig generator,
# for one) need it in the environment.
export CMAKE_POLICY_VERSION_MINIMUM=3.5

# host tools (uv-installed cmake/ninja live in ~/.local/bin)
# Native helper tools (xgettext, msgfmt ...) that cross builds must *run*.
export PATH="$WS/hosttools/bin:$HOME/.local/bin:$PATH"
# autoreconf looks for third-party m4 macros here. The X.Org libraries need
# xorg-macros, which build-host-tools.sh installs into hosttools/.
export ACLOCAL_PATH="$WS/hosttools/share/aclocal${ACLOCAL_PATH:+:$ACLOCAL_PATH}"

# Meson reads its cross and native files literally -- no environment variable
# expansion, no relative paths -- so they cannot be checked in with real paths
# and still work on another machine. They are templates; generate them here
# from wherever this workspace and the SDP actually live.
qnx_make_meson_files() {
  local t
  # Generated per staging tree (not into toolchain/), so a second workspace
  # configuration -- QNX_OSS_STAGE_DIR pointing at an empty stage to prove a
  # component builds standalone -- cannot overwrite the main build's files.
  export QNX_MESON_DIR="$STAGE_DIR/.toolchain"
  mkdir -p "$QNX_MESON_DIR"
  for t in "$TOOLCHAIN_DIR"/meson-*.ini.in; do
    [ -e "$t" ] || continue
    local out="$QNX_MESON_DIR/$(basename "${t%.in}")"
    local tag="# generated for $WS with $QNX_SDP_ROOT, stage $STAGE_DIR"
    # Regenerate only when the template or the paths changed.
    if [ ! -f "$out" ] || [ "$t" -nt "$out" ] || ! grep -qxF "$tag" "$out" 2>/dev/null; then
      { echo "# GENERATED from $(basename "$t") by env.sh -- do not edit, edit the template"
        echo "$tag"
        sed -e "s#@QNX_SDP_ROOT@#$QNX_SDP_ROOT#g" -e "s#@WS@#$WS#g" -e "s#@STAGE_DIR@#$STAGE_DIR#g" "$t"
      } > "$out"
    fi
  done
}

qnx_arch() {
  local a="${1:-aarch64le}"
  case "$a" in
    aarch64le) export QNX_ARCH=aarch64le QNX_CPU=aarch64  QNX_TRIPLE=aarch64-unknown-nto-qnx8.0.0 ;;
    x86_64)    export QNX_ARCH=x86_64    QNX_CPU=x86_64   QNX_TRIPLE=x86_64-pc-nto-qnx8.0.0 ;;
    *) echo "unknown arch: $a (use aarch64le|x86_64)"; return 1 ;;
  esac
  export QCC_TARGET="gcc_nto${QNX_ARCH}"
  export STAGE="$STAGE_DIR/$QNX_ARCH"
  # The staging tree to compile and link against. STAGE means the same thing
  # except during do_install, when qnxbuild points it at the component's own
  # pkgroot; a recipe whose install step rebuilds (git's Makefile does when
  # its flags change) uses STAGE_SYSROOT for -I/-L so the flags stay the same.
  export STAGE_SYSROOT="$STAGE"
  export SYSROOT="$QNX_TARGET"
  # pkg-config must see SDP libs AND our staging area, and must not leak host paths
  # .pc files we install record the on-target prefix (/usr/local); the sysroot
  # maps that onto the staging tree. The SDP ships no .pc files of its own.
  export PKG_CONFIG_SYSROOT_DIR="$STAGE"
  # The build machine's pkg-config, by name, for CMake's FindPkgConfig: staging
  # holds a QNX pkgconf (the pkgconf component), and a project whose program
  # search reached stage/.../bin (EmulationStation) picked that, could not run
  # it, and reported PkgConfig missing. PATH here has no staging directory.
  export PKG_CONFIG="$(command -v pkg-config)"
  export PKG_CONFIG_LIBDIR="$STAGE$PREFIX/lib/pkgconfig:$STAGE$PREFIX/share/pkgconfig"
  unset PKG_CONFIG_PATH
  mkdir -p "$STAGE/$PREFIX"/{lib/pkgconfig,include,bin,share}
  echo "QNX_ARCH=$QNX_ARCH  STAGE=$STAGE  QCC_TARGET=$QCC_TARGET"
}

# QNX folds librt / libdl / libpthread / libcrypt / libutil / libresolv / libanl
# (and has no libatomic at all -- GCC open-codes the atomics)
# into libc, so those -l flags -- which portable build systems emit
# unconditionally -- fail to resolve. Ship empty archives under our staging lib
# dir (already on every link line) so the flags become harmless no-ops instead
# of patching every package's build files.
qnx_make_compat_stubs() {
  local libdir="$STAGE$PREFIX/lib"
  local marker="$libdir/.compat-stubs"   # separate stmt: bash expands all words of one `local` before assigning
  [ -f "$marker" ] && return 0
  mkdir -p "$libdir"
  local tmp; tmp=$(mktemp -d)
  echo 'static int _qnx_compat_stub_unused;' > "$tmp/stub.c"
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -c "$tmp/stub.c" -o "$tmp/stub.o" 2>/dev/null || { rm -rf "$tmp"; return 1; }
  local l
  # NOT regex: QNX does ship a real libregex.so (in $QNX_TARGET/<arch>/lib).
  # An empty libregex.a here sits earlier on every link line and shadows it,
  # so -lregex resolves to nothing and the consumer fails on regcomp/regexec.
  for l in rt dl pthread anl crypt util resolv atomic; do
    "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$libdir/lib$l.a" "$tmp/stub.o" 2>/dev/null
  done
  rm -rf "$tmp"; touch "$marker"
}

# ---------------------------------------------------------------------------
# Link sysroot: make our staged libraries outrank the SDP's prebuilt copies
# when the linker resolves a *transitive* DT_NEEDED.
#
# The QNX gcc spec injects its own
#     -rpath-link ${QNX_TARGET}/<arch>/lib/gcc/...:${QNX_TARGET}/<arch>/lib:
#                 ${QNX_TARGET}/<arch>/usr/lib:${QNX_TARGET}/<arch>/opt/lib
# ahead of anything we pass, and ld takes the first match -- so for a soname the
# SDP also ships (libssl.so.3, libcrypto.so.3, libxml2.so.2, libzstd.so.1,
# liblzma.so.5, libbz2.so.1, liblz4.so.1, libncursesw.so.6) the SDP's older copy
# won. That is how libcurl's perfectly valid call to SSL_get0_group_name came
# out as an undefined reference: it was checked against the SDP's OpenSSL, not
# the 3.5.1 we built.
#
# ld expands the literal ${QNX_TARGET} in that spec from the environment, so
# pointing QNX_TARGET at a shadow tree -- every SDP entry symlinked through,
# with <arch>/usr/lib a real directory where our libraries overwrite the SDP's
# names -- redirects the spec's own search to us without touching the SDP.
qnx_make_linkroot() {
  local real="$QNX_TARGET_SDP" root="$STAGE_DIR/$QNX_ARCH/linkroot"
  local libdir="$STAGE$PREFIX/lib" stamp="$root/.stamp"
  [ -d "$real" ] || return 1
  # Rebuild when staging changed -- and when the SDP changed. Watching only
  # staging was wrong: installing a missing SDP package (the eventfd, epoll,
  # signalfd and timerfd shims) left this shadow tree cached without them, and
  # every link then failed with "cannot find -leventfd" even though the
  # libraries were sitting in the SDP. A full pass is ~1700 symlinks.
  if [ -f "$stamp" ] \
     && [ -z "$(find "$libdir" -maxdepth 1 -newer "$stamp" -print -quit 2>/dev/null)" ] \
     && [ -z "$(find "$real/$QNX_ARCH/lib" "$real/$QNX_ARCH/usr/lib" -maxdepth 1 \
                     -newer "$stamp" -print -quit 2>/dev/null)" ]; then
    export QNX_TARGET="$root"; return 0
  fi
  # Built beside the live tree and swapped in with an atomic rename of a
  # symlink: another build running at the same time (two qnxbuild processes,
  # or the test suite) keeps a complete sysroot at every instant. Deleting and
  # recreating it in place made concurrent compiles fail with "stdio.h: No
  # such file or directory".
  local live="$root" new="$STAGE_DIR/$QNX_ARCH/linkroot.d/$$.$(date +%s%N)"
  [ -d "$live" ] && [ ! -L "$live" ] && rm -rf "$live"
  root="$new"; mkdir -p "$root/$QNX_ARCH/usr/lib"
  local e b
  for e in "$real"/*; do b=$(basename "$e"); [ "$b" = "$QNX_ARCH" ] && continue
    ln -sfn "$e" "$root/$b"; done
  for e in "$real/$QNX_ARCH"/*; do b=$(basename "$e"); [ "$b" = usr ] && continue
    ln -sfn "$e" "$root/$QNX_ARCH/$b"; done
  for e in "$real/$QNX_ARCH/usr"/*; do b=$(basename "$e"); [ "$b" = lib ] && continue
    ln -sfn "$e" "$root/$QNX_ARCH/usr/$b"; done
  for e in "$real/$QNX_ARCH/usr/lib"/*; do
    ln -sfn "$e" "$root/$QNX_ARCH/usr/lib/$(basename "$e")"; done
  # Ours last, so they replace the SDP entries of the same name.
  for e in "$libdir"/*.so*; do
    [ -e "$e" ] && ln -sfn "$e" "$root/$QNX_ARCH/usr/lib/$(basename "$e")"; done
  touch "$root/.stamp"
  ln -sfn "$root" "$live.tmp.$$" && mv -T "$live.tmp.$$" "$live"
  # Drop old trees; give processes that resolved the previous link a minute.
  find "$STAGE_DIR/$QNX_ARCH/linkroot.d" -mindepth 1 -maxdepth 1 -mmin +1 ! -path "$root" -exec rm -rf {} + 2>/dev/null
  root="$live"
  export QNX_TARGET="$root"
}

qnx_make_meson_files
qnx_arch "${1:-aarch64le}"
qnx_make_compat_stubs
export QNX_TARGET_SDP="${QNX_TARGET_SDP:-$QNX_TARGET}"
qnx_make_linkroot
