# duktape 2.7.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://duktape.org/duktape-2.7.0.tar.xz |
| SHA-256 | `90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_duktape`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install duktape
oss-get install duktape-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild duktape` performs exactly the same steps (2–5) in one command.

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
URL=https://duktape.org/duktape-2.7.0.tar.xz
mkdir -p dist src/duktape
curl -fL -o dist/$(basename $URL) $URL
echo "90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/duktape --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/duktape; B=build/$QNX_ARCH/duktape; mkdir -p $B
```

```bash
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
    -I"$S/src" -c "$S/src/duktape.c" -o "$B/duktape.o"
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libduktape.so.207 \
    -o "$B/libduktape.so.207.0.0" "$B/duktape.o" -lm
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m 755 "$B/libduktape.so.207.0.0" "$STAGE$PREFIX/lib/"
  ln -sfn libduktape.so.207.0.0 "$STAGE$PREFIX/lib/libduktape.so.207"
  ln -sfn libduktape.so.207     "$STAGE$PREFIX/lib/libduktape.so"
  install -m 644 "$S/src/duktape.h" "$S/src/duk_config.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Duktape ships a prepared amalgamation rather than a build system.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH duktape`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libduktape.so
/usr/local/lib/libduktape.so.207
/usr/local/lib/libduktape.so.207.0.0
```

The complete list is `stage/<arch>/.manifest/duktape` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH duktape
ls repo/8.0/*/ | grep '^duktape-'
```

This writes `duktape-<ver>-r0.tar.gz` (run time) and `duktape-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install duktape
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/duktape-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/duktape-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_duktape`](../../tests/src/) links duktape the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_duktape root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_duktape'
```

```
== duktape ==
  ok   ...
PASS duktape
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.7.0
TARBALL=https://duktape.org/duktape-2.7.0.tar.xz
SHA256=90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890
BUILDSYS=custom
# Duktape ships a prepared amalgamation rather than a build system.
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
    -I"$S/src" -c "$S/src/duktape.c" -o "$B/duktape.o"
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libduktape.so.207 \
    -o "$B/libduktape.so.207.0.0" "$B/duktape.o" -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m 755 "$B/libduktape.so.207.0.0" "$STAGE$PREFIX/lib/"
  ln -sfn libduktape.so.207.0.0 "$STAGE$PREFIX/lib/libduktape.so.207"
  ln -sfn libduktape.so.207     "$STAGE$PREFIX/lib/libduktape.so"
  install -m 644 "$S/src/duktape.h" "$S/src/duk_config.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
