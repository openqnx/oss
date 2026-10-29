# Helpers for recipes that build Rust code with the QNX-enabled toolchain of
# ports/rust (installed in hosttools/rust). Sourced by recipe.sh files:
#
#   source "$WS/scripts/lib/rust.sh"
#   do_build() { qnx_rust_env cargo build --release --target "$(qnx_rust_triple)"; }
#
# Upstream Rust has the QNX 8.0 targets (aarch64-unknown-nto-qnx800,
# x86_64-pc-nto-qnx800) but ships no standard library for them; ports/rust
# builds one. Linking goes through qcc with the -V target selector, and the
# cc crate gets the same selector for any C it compiles.
qnx_rust_triple() { case "$QNX_ARCH" in aarch64le) echo aarch64-unknown-nto-qnx800 ;; x86_64) echo x86_64-pc-nto-qnx800 ;; esac; }
qnx_rust_env() {
  local t; t=$(qnx_rust_triple); local T; T=$(echo "$t" | tr 'a-z-' 'A-Z_'); local u; u=$(echo "$t" | tr '-' '_')
  env PATH="$WS/hosttools/rust/bin:$PATH" \
    "CARGO_TARGET_${T}_LINKER=qcc" "CARGO_TARGET_${T}_RUSTFLAGS=-C link-arg=-V$QCC_TARGET" \
    "CC_$u=qcc" "CXX_$u=q++" "CFLAGS_$u=-V$QCC_TARGET -D_QNX_SOURCE" "CXXFLAGS_$u=-V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    "AR_$u=$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" "$@"
}
