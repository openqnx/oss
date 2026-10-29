VERSION=1.2.1; GIT=https://github.com/eclipse-zenoh/zenoh-c.git; REF=1.2.1
# Eclipse Zenoh's C API. The library is Rust (zenoh) behind a C ABI: its CMake
# runs cargo, here with the QNX-enabled Rust toolchain from ports/rust
# (ZENOHC_CUSTOM_TARGET names the QNX target; scripts/lib/rust.sh sets the
# qcc linker and the cc crate's compiler). Cargo fetches the crates zenoh-c's
# Cargo.lock pins.
DEPS="rust"
BUILDSYS=custom
source "$WS/scripts/lib/rust.sh"
# Crates that need a QNX change (crate-patches/<crate>-<version>-*.patch): the
# released crate is fetched at the checksum Cargo.lock pins, patched, and
# cargo is pointed at it with [patch.crates-io]. Cargo.toml is generated from
# Cargo.toml.in, so the entry goes into both.
ZENOHC_CRATE_PATCHES="pnet_sys|0.35.0|7d4643d3d4db6b08741050c2f3afa9a892c4244c085a72fcda93c9c2c9a00f4b
ring|0.17.8|c17fa4cb658e3583423e915b9f3acc01cceaee1860e33d59ebae66adc3a2dc0d
quinn-udp|0.5.5|4fe68c2e9e1a1234e218683dbdf9f9dfcb094113c5ac2b938dfcb9bab4c4140b
pnet_datalink|0.35.0|e79e70ec0be163102a332e1d2d5586d362ad76b01cec86f830241f2b6452a7b7
polling|3.7.3|cc2790cd301dec6cd3b7a025e4815cf825724a51c98dccfe6a3e55f05ffb6511
socket2|0.5.7|ce305eb0b4296696835b71df73eb912e0f1ffd2556a501fcede6e0c50349191c"
# Cargo manifests that get the [patch] sections: zenoh-c's (Cargo.toml and the
# Cargo.toml.in it is generated from) and build-resources/opaque-types', a
# separate workspace that build.rs builds for the target to read the sizes of
# zenoh's types -- unpatched, that build failed on the same crates and build.rs
# stopped with "no sigatures found for building generic z_call".
_cargo_append() { tee -a "$SRCROOT/Cargo.toml" "$SRCROOT/Cargo.toml.in" >> "$SRCROOT/build-resources/opaque-types/Cargo.toml"; }
do_patch() {
  local e name ver sum c d p
  # patches/0001 moves async-std to 1.13; bring its Cargo.lock entry (and only
  # the crates that change with it) along. Before the [patch] sections below,
  # which would otherwise be resolved into the lock as well.
  ( cd "$SRCROOT" && qnx_rust_env cargo update -q -p async-std --precise 1.13.0 ) || return 1
  printf '\n[patch.crates-io]\n' | _cargo_append
  for e in $ZENOHC_CRATE_PATCHES; do
    IFS='|' read -r name ver sum <<<"$e"
    c="$WS/dist/$name-$ver.crate"; d="$SRCROOT/qnx-crates/$name"
    [ -f "$c" ] || curl -fsSL -o "$c" "https://static.crates.io/crates/$name/$name-$ver.crate" || return 1
    echo "$sum  $c" | sha256sum -c --status || { echo "checksum mismatch: $c"; rm -f "$c"; return 1; }
    mkdir -p "$d" && tar xzf "$c" -C "$d" --strip-components=1 || return 1
    for p in "$RECIPE_DIR/zenoh-c/crate-patches/$name-$ver"-*.patch; do
      echo "-- crate patch $(basename "$p")"; patch -p1 -d "$d" < "$p" || return 1
    done
    echo "$name = { path = \"$d\" }" | _cargo_append
  done
  _zenoh_git_patch
}
# zenoh itself is a git dependency (release/1.2.1, pinned to a commit in
# Cargo.lock): it is checked out at that commit, crate-patches/zenoh-*.patch
# are applied to the workspace, and every crate Cargo.lock takes from that
# repository is pointed at the checkout with [patch."<repository>"].
ZENOH_GIT=https://github.com/eclipse-zenoh/zenoh.git
_zenoh_git_patch() {
  local commit d p c name dir
  commit=$(grep -o "git+$ZENOH_GIT?[^#]*#[0-9a-f]*" "$SRCROOT/Cargo.lock" | head -1 | sed 's/.*#//')
  [ -n "$commit" ] || { echo "no $ZENOH_GIT commit in Cargo.lock"; return 1; }
  d="$SRCROOT/qnx-crates/zenoh"
  git init -q "$d" && git -C "$d" fetch -q --depth 1 "$ZENOH_GIT" "$commit" && \
    git -C "$d" -c advice.detachedHead=false checkout -q FETCH_HEAD || return 1
  for p in "$RECIPE_DIR/zenoh-c/crate-patches/zenoh-${commit:0:7}"-*.patch; do
    echo "-- zenoh patch $(basename "$p")"; patch -p1 -d "$d" < "$p" || return 1
  done
  printf '\n[patch."%s"]\n' "$ZENOH_GIT" | _cargo_append
  # Crate names from the lock file entries with that source, and the
  # directory of each in the workspace (the Cargo.toml naming it).
  awk -v src="source = \"git+$ZENOH_GIT" '/^name = /{n=$3} index($0,src)==1{gsub(/"/,"",n); print n}' "$SRCROOT/Cargo.lock" | sort -u | \
  while read -r name; do
    c=$(grep -rlx --include=Cargo.toml "name = \"$name\"" "$d" | grep -v /target/ | head -1)
    [ -n "$c" ] || { echo "zenoh crate $name not found in $d"; return 1; }
    dir=$(dirname "$c")
    echo "$name = { path = \"$dir\" }" | _cargo_append
  done
}
# zenoh-c's default features except transport_unixsock-stream: that transport
# is built on the nix crate, which has no QNX support (no errno table for
# target_os = "nto", in 0.29 or upstream). TCP, UDP, QUIC, TLS and WebSocket
# transports are all in.
ZENOHC_FEATURES="auth_pubkey,auth_usrpwd,transport_multilink,transport_compression,transport_quic,transport_tcp,transport_tls,transport_udp,transport_ws"
do_configure() {
  qnx_rust_env cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_TESTING=OFF -DZENOHC_BUILD_IN_SOURCE_TREE=OFF \
    -DZENOHC_CUSTOM_TARGET="$(qnx_rust_triple)" -DZENOHC_INSTALL_STATIC_LIBRARY=FALSE \
    -DZENOHC_CARGO_FLAGS="--no-default-features;--features=$ZENOHC_FEATURES"
}
do_build()   { qnx_rust_env cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" qnx_rust_env cmake --install "$B"; }
