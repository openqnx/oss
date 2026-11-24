# zenohd, the Eclipse Zenoh router: routes pub/sub, queries and liveliness
# between Zenoh clients and peers (zenoh-c, zenoh-cpp, zenoh-pico, rmw_zenoh)
# across TCP/UDP/QUIC/TLS/WebSocket links, with its admin space (@/<zid>/router).
# The same Zenoh release as the zenoh-c component (1.10.2), built with the
# QNX-enabled Rust toolchain (ports/rust; scripts/lib/rust.sh). QNX ships
# zenohd as a prebuilt Linux binary for the host (qnx-ports build-files
# conan/recipes/zenoh-router); this is zenohd running on the QNX target.
# Only zenohd: the REST and storage-manager plugins are not built (the REST
# plugin pulls in async-io 1.x / rustix 0.37, which has no QNX support).
# Features: zenoh's defaults except transport_unixsock-stream (the nix crate
# has no QNX support), as for zenoh-c.
VERSION=1.10.2; GIT=https://github.com/eclipse-zenoh/zenoh.git; REF=1.10.2
DEPS="rust"
BUILDSYS=custom
source "$WS/scripts/lib/rust.sh"
# Crates that need a QNX change: the released crate at the checksum
# Cargo.lock pins, patched with zenoh-c's crate patches (the same versions),
# and pointed at with [patch.crates-io] in the workspace Cargo.toml.
ZENOHD_CRATE_PATCHES="pnet_sys|0.35.0|7d4643d3d4db6b08741050c2f3afa9a892c4244c085a72fcda93c9c2c9a00f4b
ring|0.17.14|a4689e6c2294d81e88dc6261c768b63bc4fcdb852be6d1352498b114f61383b7
quinn-udp|0.5.16|af66907df18639dcf4db56ca65490cabc4b27a97dbadd96f2926cca73298f016
pnet_datalink|0.35.0|e79e70ec0be163102a332e1d2d5586d362ad76b01cec86f830241f2b6452a7b7
socket2|0.5.10|e22376abed350d73dd1cd119b57ffccad95b4e585a7cda43e286245ce23c0678
socket2|0.6.5|c3d1e2c7f27f8d4cb10542a02c49005dbd6e93095799d6f3be745fae9f8fedd4
tokio|1.53.2|e95f91fcc7a621e8b030f6aa23c71fe9838ae2fb4d8118b75602a328f5144044"
# (tokio 1.53.2 is zenohd's lock only; zenoh-c's lock has 1.54.1, which needs
# no change.)
# libc held at 0.2.186, the last release for target_env = "nto80" (the QNX 8
# target of ports/rust 1.90); see ports/zenoh-c/recipe.sh.
ZENOH_LIBC=0.2.186
do_patch() {
  local e name ver sum c d p
  # Before the [patch] section, which would otherwise be resolved into the lock.
  ( cd "$SRCROOT" && qnx_rust_env cargo update -q -p libc --precise "$ZENOH_LIBC" ) || return 1
  printf '\n[patch.crates-io]\n' >> "$SRCROOT/Cargo.toml"
  for e in $ZENOHD_CRATE_PATCHES; do
    IFS='|' read -r name ver sum <<<"$e"
    c="$WS/dist/$name-$ver.crate"; d="$SRCROOT/qnx-crates/$name-$ver"
    [ -f "$c" ] || curl -fsSL -o "$c" "https://static.crates.io/crates/$name/$name-$ver.crate" || return 1
    echo "$sum  $c" | sha256sum -c --status || { echo "checksum mismatch: $c"; rm -f "$c"; return 1; }
    mkdir -p "$d" && tar xzf "$c" -C "$d" --strip-components=1 || return 1
    for p in "$RECIPE_DIR/zenoh-c/crate-patches/$name-$ver"-*.patch; do
      echo "-- crate patch $(basename "$p")"; patch -p1 -d "$d" < "$p" || return 1
    done
    echo "${name}-${ver//./-} = { package = \"$name\", path = \"$d\" }" >> "$SRCROOT/Cargo.toml"
  done
}
ZENOHD_FEATURES="zenoh/auth_pubkey,zenoh/auth_usrpwd,zenoh/transport_multilink,zenoh/transport_compression,zenoh/transport_quic,zenoh/transport_quic_datagram,zenoh/transport_tcp,zenoh/transport_tls,zenoh/transport_udp,zenoh/transport_ws"
do_configure() { :; }
do_build() {
  cd "$S" && qnx_rust_env cargo build --release --target "$(qnx_rust_triple)" --target-dir "$B/target" \
    -p zenohd --no-default-features --features "$ZENOHD_FEATURES"
}
do_install() { install -Dm755 "$B/target/$(qnx_rust_triple)/release/zenohd" "$STAGE$PREFIX/bin/zenohd"; }
