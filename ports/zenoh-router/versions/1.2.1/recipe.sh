# zenohd, the Eclipse Zenoh router: routes pub/sub, queries and liveliness
# between Zenoh clients and peers (zenoh-c, zenoh-cpp, zenoh-pico, rmw_zenoh)
# across TCP/UDP/QUIC/TLS/WebSocket links, with its admin space (@/<zid>/router).
# The same Zenoh release as the zenoh-c component (1.2.1), built with the
# QNX-enabled Rust toolchain (ports/rust; scripts/lib/rust.sh). QNX ships
# zenohd as a prebuilt Linux binary for the host (qnx-ports build-files
# conan/recipes/zenoh-router); this is zenohd running on the QNX target.
# Only zenohd: the REST and storage-manager plugins are not built (the REST
# plugin pulls in async-io 1.x / rustix 0.37, which has no QNX support).
# Features: zenoh's defaults except transport_unixsock-stream (the nix crate
# has no QNX support), as for zenoh-c.
VERSION=1.2.1; GIT=https://github.com/eclipse-zenoh/zenoh.git; REF=1.2.1
DEPS="rust"
BUILDSYS=custom
source "$WS/scripts/lib/rust.sh"
# Crates that need a QNX change: the released crate at the checksum
# Cargo.lock pins, patched with zenoh-c's crate patches (the same versions),
# and pointed at with [patch.crates-io] in the workspace Cargo.toml.
ZENOHD_CRATE_PATCHES="pnet_sys|0.35.0|7d4643d3d4db6b08741050c2f3afa9a892c4244c085a72fcda93c9c2c9a00f4b
ring|0.17.8|c17fa4cb658e3583423e915b9f3acc01cceaee1860e33d59ebae66adc3a2dc0d
quinn-udp|0.5.5|4fe68c2e9e1a1234e218683dbdf9f9dfcb094113c5ac2b938dfcb9bab4c4140b
pnet_datalink|0.35.0|e79e70ec0be163102a332e1d2d5586d362ad76b01cec86f830241f2b6452a7b7
polling|3.7.3|cc2790cd301dec6cd3b7a025e4815cf825724a51c98dccfe6a3e55f05ffb6511
socket2|0.5.7|ce305eb0b4296696835b71df73eb912e0f1ffd2556a501fcede6e0c50349191c"
do_patch() {
  local e name ver sum c d p
  printf '\n[patch.crates-io]\n' >> "$SRCROOT/Cargo.toml"
  for e in $ZENOHD_CRATE_PATCHES; do
    IFS='|' read -r name ver sum <<<"$e"
    c="$WS/dist/$name-$ver.crate"; d="$SRCROOT/qnx-crates/$name"
    [ -f "$c" ] || curl -fsSL -o "$c" "https://static.crates.io/crates/$name/$name-$ver.crate" || return 1
    echo "$sum  $c" | sha256sum -c --status || { echo "checksum mismatch: $c"; rm -f "$c"; return 1; }
    mkdir -p "$d" && tar xzf "$c" -C "$d" --strip-components=1 || return 1
    for p in "$RECIPE_DIR/zenoh-c/crate-patches/$name-$ver"-*.patch; do
      echo "-- crate patch $(basename "$p")"; patch -p1 -d "$d" < "$p" || return 1
    done
    echo "$name = { path = \"$d\" }" >> "$SRCROOT/Cargo.toml"
  done
}
ZENOHD_FEATURES="zenoh/auth_pubkey,zenoh/auth_usrpwd,zenoh/transport_multilink,zenoh/transport_compression,zenoh/transport_quic,zenoh/transport_tcp,zenoh/transport_tls,zenoh/transport_udp,zenoh/transport_ws"
do_configure() { :; }
do_build() {
  cd "$S" && qnx_rust_env cargo build --release --target "$(qnx_rust_triple)" --target-dir "$B/target" \
    -p zenohd --no-default-features --features "$ZENOHD_FEATURES"
}
do_install() { install -Dm755 "$B/target/$(qnx_rust_triple)/release/zenohd" "$STAGE$PREFIX/bin/zenohd"; }
