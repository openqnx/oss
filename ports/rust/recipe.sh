VERSION=1.90.0
TARBALL=https://static.rust-lang.org/dist/rustc-1.90.0-src.tar.xz
SHA256=6bfeaddd90ffda2f063492b092bfed925c4b8c701579baf4b1316e021470daac
BUILDSYS=custom
# The Rust toolchain itself: rustc and cargo for the build host, with the
# standard library for both QNX 8 targets (x86_64-pc-nto-qnx800,
# aarch64-unknown-nto-qnx800). Upstream supports these targets at Tier 3, so
# it ships no prebuilt std for them -- the compiler is built here. 1.90.0 is
# the release qnx-ports tests on QNX 8.0 (newer upstream releases dropped the
# qnx800 target definitions); patches/ holds its two std fixes from
# qnx-ports/rust qnx-1.90.0 (process spawning, backtraces).
#
# It is a build-host tool: it installs into hosttools/rust (once, for both
# architectures), and Rust components (zenoh-c) build with it. The staging
# tree only gets a note saying where the toolchain is.
RUST_HOME="$WS/hosttools/rust"
do_configure() {
  [ -x "$RUST_HOME/bin/rustc" ] && return 0
  cat > "$S/config.toml" <<TOML
change-id = "ignore"
[llvm]
download-ci-llvm = false
[build]
target = ['x86_64-unknown-linux-gnu', 'x86_64-pc-nto-qnx800', 'aarch64-unknown-nto-qnx800']
docs = false
extended = true
tools = ['cargo', 'rustfmt', 'clippy', 'src']
build-dir = "$B/build"
[target.x86_64-pc-nto-qnx800]
cc = 'qcc'
cxx = 'q++'
ar = 'ntox86_64-ar'
[target.aarch64-unknown-nto-qnx800]
cc = 'qcc'
cxx = 'q++'
ar = 'ntoaarch64-ar'
[install]
prefix = "$RUST_HOME"
sysconfdir = "etc"
TOML
}
# qcc picks its target with -V; given none it compiles for x86_64, and
# compiler-rt's aarch64 sources then stop with "intended only for
# aarch64-based targets". The cc crate reads per-target flags from these
# (the settings the Rust book's nto-qnx page gives).
_rust_env() {
  env -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH QNX_TARGET="$QNX_TARGET_SDP" \
    CFLAGS_aarch64_unknown_nto_qnx800=-Vgcc_ntoaarch64le_cxx CXXFLAGS_aarch64_unknown_nto_qnx800=-Vgcc_ntoaarch64le_cxx \
    CFLAGS_x86_64_pc_nto_qnx800=-Vgcc_ntox86_64_cxx CXXFLAGS_x86_64_pc_nto_qnx800=-Vgcc_ntox86_64_cxx "$@"
}
do_build() {
  [ -x "$RUST_HOME/bin/rustc" ] && return 0
  # QNX_TARGET must be the real SDP target tree here: the std build links
  # against it with qcc directly, not through this workspace's link sysroot.
  # The compiler itself is a build-host program: keep pkg-config away from the
  # QNX staging tree (host crates would otherwise link QNX libraries).
  cd "$S" && _rust_env python3 x.py build -j"$(nproc)"
}
do_install() {
  [ -x "$RUST_HOME/bin/rustc" ] || { cd "$S" && _rust_env python3 x.py install; } || return 1
  install -d "$STAGE$PREFIX/share/doc/rust-qnx"
  echo "Rust $VERSION for QNX: host toolchain in hosttools/rust (targets x86_64-pc-nto-qnx800, aarch64-unknown-nto-qnx800)." \
    > "$STAGE$PREFIX/share/doc/rust-qnx/README"
}
