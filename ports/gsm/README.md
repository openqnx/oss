# gsm 1.0.22

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.quut.com/gsm/gsm-1.0.22.tar.gz |
| SHA-256 | `f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_misc_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gsm
oss-get install gsm-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gsm` performs exactly the same steps (2–5) in one command.

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
URL=https://www.quut.com/gsm/gsm-1.0.22.tar.gz
mkdir -p dist src/gsm
curl -fL -o dist/$(basename $URL) $URL
echo "f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gsm --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gsm; B=build/$QNX_ARCH/gsm; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B/o"
  for f in "$S"/src/*.c; do
    "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
      -DSASR -DWAV49 -DNeedFunctionPrototypes=1 -I"$S/inc" \
      -c "$f" -o "$B/o/$(basename "${f%.c}").o"
  done
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libgsm.so.1 \
    -o "$B/libgsm.so.1.0.22" "$B"/o/*.o
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/gsm"
  install -m 755 "$B/libgsm.so.1.0.22" "$STAGE$PREFIX/lib/"
  ln -sfn libgsm.so.1.0.22 "$STAGE$PREFIX/lib/libgsm.so.1"
  ln -sfn libgsm.so.1      "$STAGE$PREFIX/lib/libgsm.so"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/gsm/"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The upstream Makefile hardcodes the host compiler and installs nowhere useful; the library is eight objects, so build it directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gsm`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libgsm.so
/usr/local/lib/libgsm.so.1
/usr/local/lib/libgsm.so.1.0.22
```

The complete list is `stage/<arch>/.manifest/gsm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gsm
ls repo/8.0/*/ | grep '^gsm-'
```

This writes `gsm-<ver>-r0.tar.gz` (run time) and `gsm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gsm
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gsm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gsm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links gsm the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_misc_libs'
```

```
== gsm ==
  ok   ...
PASS gsm
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.22
TARBALL=https://www.quut.com/gsm/gsm-1.0.22.tar.gz
SHA256=f0072e91f6bb85a878b2f6dbf4a0b7c850c4deb8049d554c65340b3bf69df0ac
BUILDSYS=custom
# The upstream Makefile hardcodes the host compiler and installs nowhere useful;
# the library is eight objects, so build it directly.
do_build() {
  mkdir -p "$B/o"
  for f in "$S"/src/*.c; do
    "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
      -DSASR -DWAV49 -DNeedFunctionPrototypes=1 -I"$S/inc" \
      -c "$f" -o "$B/o/$(basename "${f%.c}").o"
  done
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libgsm.so.1 \
    -o "$B/libgsm.so.1.0.22" "$B"/o/*.o
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/gsm"
  install -m 755 "$B/libgsm.so.1.0.22" "$STAGE$PREFIX/lib/"
  ln -sfn libgsm.so.1.0.22 "$STAGE$PREFIX/lib/libgsm.so.1"
  ln -sfn libgsm.so.1      "$STAGE$PREFIX/lib/libgsm.so"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/gsm/"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
