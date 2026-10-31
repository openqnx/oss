# zenoh-c 1.2.1

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/eclipse-zenoh/zenoh-c.git (tag `1.2.1`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`rust`](../rust/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zenoh_c`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add zenoh-c
apk add zenoh-c-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-zenoh-c && /usr/local/bin/qnx-port-tests/test_zenoh_c    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install zenoh-c`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zenoh-c` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild rust
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.2.1 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-zenoh/zenoh-c.git src/zenoh-c
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zenoh-c < ports/zenoh-c/patches/0001-async-std-1.13.patch
patch -p1 -d src/zenoh-c < ports/zenoh-c/patches/0002-opaque-types-same-features.patch
```

**[`0001-async-std-1.13.patch`](patches/0001-async-std-1.13.patch)** — Use async-std 1.13.

> zenoh-c pins async-std 1.12.0, which brings in async-io 1.13 and with it
> rustix 0.37 -- a rustix without QNX support (no preadv/pwritev, statfs or
> DT_* constants for target_os = "nto"). async-std 1.13 moved to async-io 2
> and rustix 0.38, which zenoh itself already uses and which builds for QNX.
> zenoh-c uses only async_std::task::sleep, unchanged between the two.
>
> ports/zenoh-c/recipe.sh updates the async-std entry of Cargo.lock to match
> (cargo update -p async-std --precise 1.13.0).

Touches: `Cargo.toml.in`, `Cargo.toml` · Upstream status: Pending

**[`0002-opaque-types-same-features.patch`](patches/0002-opaque-types-same-features.patch)** — build.rs: build the opaque-types crate with this build's features only.

> build.rs builds build-resources/opaque-types for the target to learn the
> sizes of zenoh's types, passing on each feature this build has with -F --
> but not --no-default-features, so that crate's default features are always
> added. A build without one of them (here transport_unixsock-stream, whose
> nix dependency does not support QNX) still compiled it in the nested build,
> failed there, and build.rs then stopped with "no sigatures found for
> building generic z_call".

Touches: `build.rs` · Upstream status: Pending

More QNX changes, outside the main source tree:

- [`crate-patches/`](crate-patches/) — 7 patches for Rust crates the build uses, each applied to the released crate at the version Cargo.lock pins

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zenoh-c; B=build/$QNX_ARCH/zenoh-c; mkdir -p $B
```

```bash
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
```

```bash
do_configure() {
  qnx_rust_env cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_TESTING=OFF -DZENOHC_BUILD_IN_SOURCE_TREE=OFF \
    -DZENOHC_CUSTOM_TARGET="$(qnx_rust_triple)" -DZENOHC_INSTALL_STATIC_LIBRARY=FALSE \
    -DZENOHC_CARGO_FLAGS="--no-default-features;--features=$ZENOHC_FEATURES"
}
```

```bash
do_build()   { qnx_rust_env cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" qnx_rust_env cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Eclipse Zenoh's C API. The library is Rust (zenoh) behind a C ABI: its CMake runs cargo, here with the QNX-enabled Rust toolchain from ports/rust (ZENOHC_CUSTOM_TARGET names the QNX target; scripts/lib/rust.sh sets the qcc linker and the cc crate's compiler). Cargo fetches the crates zenoh-c's Cargo.lock pins.
- Crates that need a QNX change (crate-patches/<crate>-<version>-*.patch): the released crate is fetched at the checksum Cargo.lock pins, patched, and cargo is pointed at it with [patch.crates-io]. Cargo.toml is generated from Cargo.toml.in, so the entry goes into both.
- Cargo manifests that get the [patch] sections: zenoh-c's (Cargo.toml and the Cargo.toml.in it is generated from) and build-resources/opaque-types', a separate workspace that build.rs builds for the target to read the sizes of zenoh's types -- unpatched, that build failed on the same crates and build.rs stopped with "no sigatures found for building generic z_call".
- zenoh itself is a git dependency (release/1.2.1, pinned to a commit in Cargo.lock): it is checked out at that commit, crate-patches/zenoh-*.patch are applied to the workspace, and every crate Cargo.lock takes from that repository is pointed at the checkout with [patch."<repository>"].
- zenoh-c's default features except transport_unixsock-stream: that transport is built on the nix crate, which has no QNX support (no errno table for target_os = "nto", in 0.29 or upstream). TCP, UDP, QUIC, TLS and WebSocket transports are all in.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zenoh-c`.

### 5. What it installs

13 files under `/usr/local`: 1 shared-library entries, 8 headers.

```
/usr/local/lib/libzenohc.so
```

The complete list is `stage/<arch>/.manifest/zenoh-c` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zenoh-c
ls dists/8.0/*/ | grep '^zenoh-c-'
```

This writes `zenoh-c-<ver>-r0.tar.gz` (run time) and `zenoh-c-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zenoh-c
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zenoh-c-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zenoh-c-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zenoh_c`](../../tests/src/) links zenoh-c the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zenoh-c`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zenoh-c
/usr/local/bin/qnx-port-tests/test_zenoh_c
```

```
== zenoh-c ==
  ok   ...
PASS zenoh-c
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zenoh_c` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zenoh_c`](../../tests/src/) -- a program linked against zenoh-c the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - open a session listening on TCP
   - open a second session connecting to it
   - key expression
   - declare a subscriber
   - the subscriber received a sample over TCP
   - with the payload intact

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zenoh-c 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/zenoh-c-1.2.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zenoh-c 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/zenoh-c-1.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zenoh-c=<version>`), sources in `ported/zenoh-c/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zenoh-c`; port one with `scripts/port-update zenoh-c`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
