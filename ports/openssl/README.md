# openssl 3.5.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/openssl/openssl.git (tag `openssl-3.5.1`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openssl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install openssl
oss-get install openssl-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openssl` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b openssl-3.5.1 --recurse-submodules --shallow-submodules \
    https://github.com/openssl/openssl.git src/openssl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/openssl; B=build/$QNX_ARCH/openssl; mkdir -p $B
```

```bash
do_configure() {
  cp "$RECIPE_DIR/openssl/files/50-qnx.conf" "$S/Configurations/50-qnx.conf"
  cd "$B"
  # no-asm: the perlasm backends emit GNU-as syntax that qcc's assembler driver
  # does not accept for every module; correctness first, revisit for perf later.
  perl "$S/Configure" "qnx-$QNX_ARCH" \
      --prefix="$PREFIX" --openssldir="$PREFIX/ssl" \
      --libdir=lib \
      shared no-asm no-tests no-docs no-apps threads \
      -D_QNX_SOURCE
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install_sw; }
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openssl`.

### 5. What it installs

157 files under `/usr/local`: 4 shared-library entries, 142 headers.

```
/usr/local/lib/libcrypto.so
/usr/local/lib/libcrypto.so.3
/usr/local/lib/libssl.so
/usr/local/lib/libssl.so.3
```

The complete list is `stage/<arch>/.manifest/openssl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH openssl
ls repo/8.0/*/ | grep '^openssl-'
```

This writes `openssl-<ver>-r0.tar.gz` (run time) and `openssl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install openssl
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/openssl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/openssl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_openssl`](../../tests/src/) links openssl the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openssl root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_openssl'
```

```
== openssl ==
  ok   ...
PASS openssl
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.5.1; GIT=https://github.com/openssl/openssl.git; REF=openssl-3.5.1
BUILDSYS=custom

do_configure() {
  cp "$RECIPE_DIR/openssl/files/50-qnx.conf" "$S/Configurations/50-qnx.conf"
  cd "$B"
  # no-asm: the perlasm backends emit GNU-as syntax that qcc's assembler driver
  # does not accept for every module; correctness first, revisit for perf later.
  perl "$S/Configure" "qnx-$QNX_ARCH" \
      --prefix="$PREFIX" --openssldir="$PREFIX/ssl" \
      --libdir=lib \
      shared no-asm no-tests no-docs no-apps threads \
      -D_QNX_SOURCE
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install_sw; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
