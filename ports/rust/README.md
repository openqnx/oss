# rust 1.90.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://static.rust-lang.org/dist/rustc-1.90.0-src.tar.xz |
| SHA-256 | `6bfeaddd90ffda2f063492b092bfed925c4b8c701579baf4b1316e021470daac` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add rust
apk add rust-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install rust`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild rust` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://static.rust-lang.org/dist/rustc-1.90.0-src.tar.xz
mkdir -p dist src/rust
curl -fL -o dist/$(basename $URL) $URL
echo "6bfeaddd90ffda2f063492b092bfed925c4b8c701579baf4b1316e021470daac  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/rust --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/rust < ports/rust/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — add support for QNX 8.0

> Imported from qnx-ports/rust (1.90.0...qnx-1.90.0); original authorship kept.
>
>
> ---
>  library/std/src/sys/backtrace.rs         |  2 +-
>  library/std/src/sys/process/unix/unix.rs | 21 ++++++++++++++-------
>  2 files changed, 15 insertions(+), 8 deletions(-)

Touches: `library/std/src/sys/backtrace.rs`, `library/std/src/sys/process/unix/unix.rs` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/rust; B=build/$QNX_ARCH/rust; mkdir -p $B
```

```bash
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
```

```bash
do_build() {
  [ -x "$RUST_HOME/bin/rustc" ] && return 0
  # QNX_TARGET must be the real SDP target tree here: the std build links
  # against it with qcc directly, not through this workspace's link sysroot.
  # The compiler itself is a build-host program: keep pkg-config away from the
  # QNX staging tree (host crates would otherwise link QNX libraries).
  cd "$S" && _rust_env python3 x.py build -j"$(nproc)"
}
```

```bash
do_install() {
  [ -x "$RUST_HOME/bin/rustc" ] || { cd "$S" && _rust_env python3 x.py install; } || return 1
  install -d "$STAGE$PREFIX/share/doc/rust-qnx"
  echo "Rust $VERSION for QNX: host toolchain in hosttools/rust (targets x86_64-pc-nto-qnx800, aarch64-unknown-nto-qnx800)." \
    > "$STAGE$PREFIX/share/doc/rust-qnx/README"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Rust toolchain itself: rustc and cargo for the build host, with the standard library for both QNX 8 targets (x86_64-pc-nto-qnx800, aarch64-unknown-nto-qnx800). Upstream supports these targets at Tier 3, so it ships no prebuilt std for them -- the compiler is built here. 1.90.0 is the release qnx-ports tests on QNX 8.0 (newer upstream releases dropped the qnx800 target definitions); patches/ holds its two std fixes from qnx-ports/rust qnx-1.90.0 (process spawning, backtraces). It is a build-host tool: it installs into hosttools/rust (once, for both architectures), and Rust components (zenoh-c) build with it. The staging tree only gets a note saying where the toolchain is.
- qcc picks its target with -V; given none it compiles for x86_64, and compiler-rt's aarch64 sources then stop with "intended only for aarch64-based targets". The cc crate reads per-target flags from these (the settings the Rust book's nto-qnx page gives).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH rust`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/rust` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa rust
ls dists/8.0/*/ | grep '^rust-'
```

This writes `rust-<ver>-r0.tar.gz` (run time) and `rust-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add rust
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/rust-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/rust-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | rust 1.90.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | rust 1.90.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.90.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add rust=<version>`), sources in `ported/rust/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates rust`; port one with `scripts/port-update rust`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
