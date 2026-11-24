VERSION=1.10.2; GIT=https://github.com/eclipse-zenoh/zenoh-c.git; REF=1.10.2
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
ring|0.17.14|a4689e6c2294d81e88dc6261c768b63bc4fcdb852be6d1352498b114f61383b7
quinn-udp|0.5.16|af66907df18639dcf4db56ca65490cabc4b27a97dbadd96f2926cca73298f016
pnet_datalink|0.35.0|e79e70ec0be163102a332e1d2d5586d362ad76b01cec86f830241f2b6452a7b7
socket2|0.5.10|e22376abed350d73dd1cd119b57ffccad95b4e585a7cda43e286245ce23c0678
socket2|0.6.5|c3d1e2c7f27f8d4cb10542a02c49005dbd6e93095799d6f3be745fae9f8fedd4
ctor|0.4.3|ec09e802f5081de6157da9a75701d6c713d8dc3ba52571fd4bd25f412644e8a6"
# The libc crate from 0.2.187 on knows QNX 8 only as target_os = "qnx" (what
# newer rustc calls it); ports/rust 1.90 says target_os = "nto" with
# target_env = "nto80", for which libc 0.2.191 (the lock's) stops with
# "Unsupported arch". 0.2.186 is the last release with nto80 and meets every
# libc requirement in the lock (nix 0.31's ^0.2.186 is the highest).
ZENOH_LIBC=0.2.186
# Cargo manifests that get the [patch.crates-io] section: zenoh-c's (Cargo.toml
# and the Cargo.toml.in it is generated from) and build-resources/opaque-types',
# a separate workspace that build.rs builds for the target to read the sizes of
# zenoh's types -- unpatched, that build failed on the same crates and build.rs
# stopped with "no sigatures found for building generic z_call".
_cargo_append() { tee -a "$SRCROOT/Cargo.toml" "$SRCROOT/Cargo.toml.in" >> "$SRCROOT/build-resources/opaque-types/Cargo.toml"; }
do_patch() {
  local e name ver sum c d p
  # libc held at $ZENOH_LIBC (see above). Before the [patch] sections below,
  # which would otherwise be resolved into the lock as well.
  ( cd "$SRCROOT" && qnx_rust_env cargo update -q -p libc --precise "$ZENOH_LIBC" ) || return 1
  printf '\n[patch.crates-io]\n' | _cargo_append
  for e in $ZENOHC_CRATE_PATCHES; do
    IFS='|' read -r name ver sum <<<"$e"
    c="$WS/dist/$name-$ver.crate"; d="$SRCROOT/qnx-crates/$name-$ver"
    [ -f "$c" ] || curl -fsSL -o "$c" "https://static.crates.io/crates/$name/$name-$ver.crate" || return 1
    echo "$sum  $c" | sha256sum -c --status || { echo "checksum mismatch: $c"; rm -f "$c"; return 1; }
    mkdir -p "$d" && tar xzf "$c" -C "$d" --strip-components=1 || return 1
    for p in "$RECIPE_DIR/zenoh-c/crate-patches/$name-$ver"-*.patch; do
      echo "-- crate patch $(basename "$p")"; patch -p1 -d "$d" < "$p" || return 1
    done
    echo "${name}-${ver//./-} = { package = \"$name\", path = \"$d\" }" | _cargo_append
  done
  _zenoh_git_patch
}
# zenoh itself is a git dependency (release/1.10.2, pinned to a commit in
# Cargo.lock): it is checked out at that commit, crate-patches/zenoh-*.patch
# are applied to the workspace, and every crate Cargo.lock takes from that
# repository is pointed at the checkout with [patch."<repository>"]. In
# opaque-types the zenoh git dependencies are made path dependencies instead:
# build.rs builds it --offline with the main build's Cargo.lock, which then
# has the checkout's (path) packages and no git commit, so a [patch] there
# made cargo look up the git branch offline and fail ("failed to get `zenoh`
# as a dependency of package opaque-types").
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
  printf '\n[patch."%s"]\n' "$ZENOH_GIT" | tee -a "$SRCROOT/Cargo.toml" >> "$SRCROOT/Cargo.toml.in"
  # Crate names from the lock file entries with that source, and the
  # directory of each in the workspace (the Cargo.toml naming it).
  awk -v src="source = \"git+$ZENOH_GIT" '/^name = /{n=$3} index($0,src)==1{gsub(/"/,"",n); print n}' "$SRCROOT/Cargo.lock" | sort -u | \
  while read -r name; do
    c=$(grep -rlx --include=Cargo.toml "name = \"$name\"" "$d" | grep -v /target/ | head -1)
    [ -n "$c" ] || { echo "zenoh crate $name not found in $d"; return 1; }
    dir=$(dirname "$c")
    echo "$name = { path = \"$dir\" }" | tee -a "$SRCROOT/Cargo.toml" >> "$SRCROOT/Cargo.toml.in"
    # opaque-types: the git dependency itself becomes a path dependency.
    sed -i "s|^$name = { \(version = \"[^\"]*\", \)\?git = \"$ZENOH_GIT\", branch = \"[^\"]*\"|$name = { \1path = \"$dir\"|" \
      "$SRCROOT/build-resources/opaque-types/Cargo.toml"
  done || return 1
  ! grep -n "$ZENOH_GIT" "$SRCROOT/build-resources/opaque-types/Cargo.toml" || { echo "opaque-types: git dependency left"; return 1; }
}
# zenoh-c's default features except transport_unixsock-stream: that transport
# is built on the nix crate, which has no QNX support (no errno table for
# target_os = "nto", in 0.29 or upstream). TCP, UDP, QUIC (with datagrams),
# TLS and WebSocket transports are all in.
ZENOHC_FEATURES="auth_pubkey,auth_usrpwd,transport_multilink,transport_compression,transport_quic,transport_quic_datagram,transport_tcp,transport_tls,transport_udp,transport_ws"
do_configure() {
  qnx_rust_env cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_TESTING=OFF -DZENOHC_BUILD_IN_SOURCE_TREE=OFF \
    -DZENOHC_CUSTOM_TARGET="$(qnx_rust_triple)" -DZENOHC_INSTALL_STATIC_LIBRARY=FALSE \
    -DZENOHC_CARGO_FLAGS="--no-default-features;--features=$ZENOHC_FEATURES"
}
do_build()   { qnx_rust_env cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" qnx_rust_env cmake --install "$B"; }
