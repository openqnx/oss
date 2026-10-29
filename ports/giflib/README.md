# giflib 5.2.2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz |
| SHA-256 | `be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Built alone (empty stage) | aarch64le ✅ x86_64 ✅ |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_giflib`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install giflib
oss-get install giflib-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild giflib` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz
mkdir -p dist src/giflib
curl -fL -o dist/$(basename $URL) $URL
echo "be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/giflib --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/giflib; B=build/$QNX_ARCH/giflib; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  for f in dgif_lib egif_lib gifalloc gif_err gif_font gif_hash openbsd-reallocarray; do
    $CC -O2 -fPIC -D_QNX_SOURCE -I"$S" -c "$S/$f.c" -o "$f.o" || return 1
  done
  $CC -shared -Wl,-soname,libgif.so.7 -o libgif.so.7.2.0 ./*.o || return 1
  ln -sf libgif.so.7.2.0 libgif.so.7; ln -sf libgif.so.7.2.0 libgif.so
}
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libgif.so.7.2.0" "$STAGE$PREFIX/lib/"
  cp -a "$B/libgif.so.7" "$B/libgif.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/gif_lib.h" "$STAGE$PREFIX/include/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- giflib ships only a Makefile that assumes the host toolchain.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH giflib`.

### 5. What it installs

4 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libgif.so
/usr/local/lib/libgif.so.7
/usr/local/lib/libgif.so.7.2.0
```

The complete list is `stage/<arch>/.manifest/giflib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH giflib
ls repo/8.0/*/ | grep '^giflib-'
```

This writes `giflib-<ver>-r0.tar.gz` (run time) and `giflib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install giflib
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/giflib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/giflib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_giflib`](../../tests/src/) links giflib the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_giflib root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_giflib'
```

```
== giflib ==
  ok   ...
PASS giflib
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2.2
TARBALL=https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz
SHA256=be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb
BUILDSYS=custom
# giflib ships only a Makefile that assumes the host toolchain.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  for f in dgif_lib egif_lib gifalloc gif_err gif_font gif_hash openbsd-reallocarray; do
    $CC -O2 -fPIC -D_QNX_SOURCE -I"$S" -c "$S/$f.c" -o "$f.o" || return 1
  done
  $CC -shared -Wl,-soname,libgif.so.7 -o libgif.so.7.2.0 ./*.o || return 1
  ln -sf libgif.so.7.2.0 libgif.so.7; ln -sf libgif.so.7.2.0 libgif.so
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libgif.so.7.2.0" "$STAGE$PREFIX/lib/"
  cp -a "$B/libgif.so.7" "$B/libgif.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/gif_lib.h" "$STAGE$PREFIX/include/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
