# zenoh-c 1.10.2

> C bindings for Eclipse Zenoh pub/sub, storage and query protocol

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/eclipse-zenoh/zenoh-c.git (tag `1.10.2`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`rust`](../rust/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zenoh_c`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zenoh-c` (programs, shared libraries, run-time data) and `zenoh-c-dev` (headers, pkg-config/CMake files, static libraries; needs `zenoh-c`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zenoh-c`](https://openqnx.github.io/oss/package/x86_64/zenoh-c.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-c.html)) · [`zenoh-c-dev`](https://openqnx.github.io/oss/package/x86_64/zenoh-c-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-c-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zenoh-c` (dependencies come along); one version: `apk add zenoh-c=<version>` | `apk del zenoh-c` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zenoh-c` (dependencies first) | `oss-get remove zenoh-c` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zenoh-c-dev` the same way, by its own name (`apk add zenoh-c-dev`, `oss-get install zenoh-c-dev`, by hand with its own file and list `zenoh-c-dev.list`); remove it before `zenoh-c`, which it needs.

```sh
# on the QNX target, as root
apk add zenoh-c zenoh-c-dev
apk add qnx-test-zenoh-c && /usr/local/bin/qnx-port-tests/test_zenoh_c    # its on-target test, from tests-8.0-<arch>-m-z
apk del zenoh-c zenoh-c-dev qnx-test-zenoh-c
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zenoh-c.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zenoh-c.list
rm var/lib/manual/zenoh-c.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
git clone --depth 1 -b 1.10.2 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-zenoh/zenoh-c.git src/zenoh-c
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zenoh-c < ports/zenoh-c/patches/0001-qnx-soname.patch
```

**[`0001-qnx-soname.patch`](patches/0001-qnx-soname.patch)** — zenoh-c: give libzenohc.so its SONAME on QNX too.

> build.rs passes -Wl,-soname,libzenohc.so only when the target OS is
> linux. Without a SONAME on QNX (target_os "nto"), a program linked
> against the library through CMake's imported target records the build
> machine's absolute path to it as DT_NEEDED
> (/home/.../stage/<arch>/usr/local/lib/libzenohc.so), and on the target
> ldd and the loader fail with "Could not load library". zenoh-cpp's and
> up-transport-zenoh-cpp's programs and tests were hit.

Touches: `build.rs` · Upstream status: Pending

More QNX changes, outside the main source tree:

- [`crate-patches/`](crate-patches/) — 11 patches for Rust crates the build uses, each applied to the released crate at the version Cargo.lock pins

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zenoh-c; B=build/$QNX_ARCH/zenoh-c; mkdir -p $B
```

```bash
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
- The libc crate from 0.2.187 on knows QNX 8 only as target_os = "qnx" (what newer rustc calls it); ports/rust 1.90 says target_os = "nto" with target_env = "nto80", for which libc 0.2.191 (the lock's) stops with "Unsupported arch". 0.2.186 is the last release with nto80 and meets every libc requirement in the lock (nix 0.31's ^0.2.186 is the highest).
- Cargo manifests that get the [patch.crates-io] section: zenoh-c's (Cargo.toml and the Cargo.toml.in it is generated from) and build-resources/opaque-types', a separate workspace that build.rs builds for the target to read the sizes of zenoh's types -- unpatched, that build failed on the same crates and build.rs stopped with "no sigatures found for building generic z_call".
- zenoh itself is a git dependency (release/1.10.2, pinned to a commit in Cargo.lock): it is checked out at that commit, crate-patches/zenoh-*.patch are applied to the workspace, and every crate Cargo.lock takes from that repository is pointed at the checkout with [patch."<repository>"]. In opaque-types the zenoh git dependencies are made path dependencies instead: build.rs builds it --offline with the main build's Cargo.lock, which then has the checkout's (path) packages and no git commit, so a [patch] there made cargo look up the git branch offline and fail ("failed to get `zenoh` as a dependency of package opaque-types").
- zenoh-c's default features except transport_unixsock-stream: that transport is built on the nix crate, which has no QNX support (no errno table for target_os = "nto", in 0.29 or upstream). TCP, UDP, QUIC (with datagrams), TLS and WebSocket transports are all in.

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

[`test_zenoh_c`](../../tests/src/) links zenoh-c the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zenoh-c`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zenoh-c 1.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/zenoh-c-1.2.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zenoh-c 1.2.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/zenoh-c-1.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.10.2**. Kept from earlier: `1.2.1` ([recipe and patches](versions/1.2.1/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zenoh-c=<version>`), sources in `ported/zenoh-c/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zenoh-c`; port one with `scripts/port-update zenoh-c`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
