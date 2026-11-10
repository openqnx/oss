# zenoh-router 1.2.1

> zenohd, the Eclipse Zenoh router (pub/sub, queries, liveliness across TCP/UDP/QUIC/TLS/WS)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/eclipse-zenoh/zenoh.git (tag `1.2.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`rust`](../rust/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zenoh_router`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `zenoh-router`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zenoh-router`](https://openqnx.github.io/oss/package/x86_64/zenoh-router.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-router.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zenoh-router` (dependencies come along); one version: `apk add zenoh-router=<version>` | `apk del zenoh-router` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zenoh-router` (dependencies first) | `oss-get remove zenoh-router` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-zenoh-router && /usr/local/bin/qnx-port-tests/test_zenoh_router    # its on-target test, from tests-8.0-<arch>-m-z
apk del zenoh-router qnx-test-zenoh-router
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zenoh-router.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zenoh-router.list
rm var/lib/manual/zenoh-router.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zenoh-router` performs exactly the same steps (2–5) in one command.

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
    https://github.com/eclipse-zenoh/zenoh.git src/zenoh-router
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zenoh-router < ports/zenoh-router/patches/0001-qnx-bind-to-device.patch
```

**[`0001-qnx-bind-to-device.patch`](patches/0001-qnx-bind-to-device.patch)** — zenoh-util: bind-to-device stubs for every non-Linux system.

> set_bind_to_device_{tcp,udp}_socket() are implemented with SO_BINDTODEVICE on
> Linux/Android and stubbed (a warning, then Ok) on macOS and Windows; other
> systems, QNX among them, get no definition and zenoh-link-commons fails to
> compile ("cannot find function set_bind_to_device_tcp_socket"). Use the stubs
> everywhere but Linux and Android.
>
> The same change as ports/zenoh-c/crate-patches/zenoh-4af922f-qnx-bind-to-device.patch
> (zenoh-c builds this zenoh commit, 4af922f701c5 = tag 1.2.1, as a dependency).

Touches: `commons/zenoh-util/src/net/mod.rs` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zenoh-router; B=build/$QNX_ARCH/zenoh-router; mkdir -p $B
```

```bash
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
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$S" && qnx_rust_env cargo build --release --target "$(qnx_rust_triple)" --target-dir "$B/target" \
    -p zenohd --no-default-features --features "$ZENOHD_FEATURES"
}
```

```bash
do_install() { install -Dm755 "$B/target/$(qnx_rust_triple)/release/zenohd" "$STAGE$PREFIX/bin/zenohd"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- zenohd, the Eclipse Zenoh router: routes pub/sub, queries and liveliness between Zenoh clients and peers (zenoh-c, zenoh-cpp, zenoh-pico, rmw_zenoh) across TCP/UDP/QUIC/TLS/WebSocket links, with its admin space (@/<zid>/router). The same Zenoh release as the zenoh-c component (1.2.1), built with the QNX-enabled Rust toolchain (ports/rust; scripts/lib/rust.sh). QNX ships zenohd as a prebuilt Linux binary for the host (qnx-ports build-files conan/recipes/zenoh-router); this is zenohd running on the QNX target. Only zenohd: the REST and storage-manager plugins are not built (the REST plugin pulls in async-io 1.x / rustix 0.37, which has no QNX support). Features: zenoh's defaults except transport_unixsock-stream (the nix crate has no QNX support), as for zenoh-c.
- Crates that need a QNX change: the released crate at the checksum Cargo.lock pins, patched with zenoh-c's crate patches (the same versions), and pointed at with [patch.crates-io] in the workspace Cargo.toml.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zenoh-router`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/zenohd
```

The complete list is `stage/<arch>/.manifest/zenoh-router` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zenoh-router
ls dists/8.0/*/ | grep '^zenoh-router-'
```

This writes `zenoh-router-<ver>-r0.tar.gz` (run time) and `zenoh-router-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zenoh-router
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zenoh-router-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zenoh-router-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zenoh_router`](../../tests/src/) links zenoh-router the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zenoh-router`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zenoh-router
/usr/local/bin/qnx-port-tests/test_zenoh_router
```

```
== zenoh-router ==
  ok   ...
PASS zenoh-router
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zenoh_router` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zenoh_router`](../../tests/src/) -- a program linked against zenoh-router the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - start zenohd listening on tcp/127.0.0.1:17457
   - client session A connects to the router
   - client session B connects to the router
   - B subscribes to qnx/router-test/**
   - A's sample reached B through the router, payload intact
   - query the router's admin space @/*/router
   - with zenohd stopped, nothing reaches B (no direct client-to-client path)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zenoh-router 1.2.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/zenoh-router-1.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zenoh-router 1.2.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/zenoh-router-1.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zenoh-router=<version>`), sources in `ported/zenoh-router/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zenoh-router`; port one with `scripts/port-update zenoh-router`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
