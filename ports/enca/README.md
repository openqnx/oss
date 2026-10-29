# enca 1.19

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/nijel/enca.git (tag `1.19`) |
| Build system | GNU Autotools |
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
oss-get install enca
oss-get install enca-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild enca` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.19 --recurse-submodules --shallow-submodules \
    https://github.com/nijel/enca.git src/enca
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/enca; B=build/$QNX_ARCH/enca; mkdir -p $B
```

```bash
do_patch() {
  cd "$SRCROOT" && autoreconf -fi
}
```

```bash
do_build() {
  cc -O1 -o "$B/make_hash_native" "$S/tools/make_hash.c" -lm
  make -C "$B" -j"$(nproc)" || true
  # Replace the cross-built generator with the native one and resume.
  install -m 755 "$B/make_hash_native" "$B/tools/make_hash"
  make -C "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tools/make_hash is compiled and then *run* to generate encodings.h. Built for the target it is a QNX binary, and the build dies in the binfmt handler ("Could not open /usr/lib/ldqnx-64.so.2"). Build a native copy first and make the target one a wrapper around it -- the generated header is architecture-independent.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH enca`.

### 5. What it installs

21 files under `/usr/local`: 2 shared-library entries, 2 programs, 1 headers.

```
/usr/local/lib/libenca.so
/usr/local/lib/libenca.so.5
/usr/local/bin/enca
/usr/local/bin/enconv
```

The complete list is `stage/<arch>/.manifest/enca` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH enca
ls repo/8.0/*/ | grep '^enca-'
```

This writes `enca-<ver>-r0.tar.gz` (run time) and `enca-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install enca
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/enca-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/enca-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links enca the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_misc_libs'
```

```
== enca ==
  ok   ...
PASS enca
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.19; GIT=https://github.com/nijel/enca.git; REF=1.19
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-external )
# tools/make_hash is compiled and then *run* to generate encodings.h. Built for
# the target it is a QNX binary, and the build dies in the binfmt handler
# ("Could not open /usr/lib/ldqnx-64.so.2"). Build a native copy first and make
# the target one a wrapper around it -- the generated header is
# architecture-independent.
do_patch() {
  cd "$SRCROOT" && autoreconf -fi
}
do_build() {
  cc -O1 -o "$B/make_hash_native" "$S/tools/make_hash.c" -lm
  make -C "$B" -j"$(nproc)" || true
  # Replace the cross-built generator with the native one and resume.
  install -m 755 "$B/make_hash_native" "$B/tools/make_hash"
  make -C "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
