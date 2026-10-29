# luajit 2.1.c6ffc141

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/LuaJIT/LuaJIT.git (tag `v2.1`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_luajit`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install luajit
oss-get install luajit-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild luajit` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.1 --recurse-submodules --shallow-submodules \
    https://github.com/LuaJIT/LuaJIT.git src/luajit
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/luajit; B=build/$QNX_ARCH/luajit; mkdir -p $B
```

```bash
do_patch() {
  cd "$SRCROOT" || return 1
  git checkout -q -- . && git fetch -q --depth 1 origin "$PIN" && git checkout -q "$PIN" || return 1
  patch -p1 < "$RECIPE_DIR/luajit/files/0001-cross-qnx-buildvm-posix-layout.patch"
}
```

```bash
do_configure() { :; }
```

```bash
do_build()   { _ljmake clean >/dev/null 2>&1; _ljmake amalg; }
```

```bash
do_install() { _ljmake install DESTDIR="$STAGE"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LuaJIT is a rolling release off the v2.1 branch; pin the commit that was built and tested here, not the moving branch head.
- Upstream already knows QNX on the target side: lj_arch.h maps __QNX__ to LUAJIT_OS_POSIX and lj_prng.c / lj_profile.c have QNX branches. The build runs buildvm on THIS machine to generate the target's VM, and for a TARGET_SYS the Makefile does not list it compiles buildvm with LUAJIT_OS=OTHER. That is NOT harmless (an earlier version of this comment said it was): OTHER disables LJ_HASPROFILE, which shrinks jit_State, which moves the dispatch table the assembler VM addresses global state from.  Every such access was 16 bytes off and LuaJIT crashed on the first line of any script. files/0001 makes buildvm use POSIX for TARGET_SYS=QNX. The patch lives in files/, not patches/: the driver applies patches/ right after cloning the moving v2.1 head, before do_patch() checks out the pin -- so do_patch() applies it itself, to the pinned tree.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH luajit`.

### 5. What it installs

33 files under `/usr/local`: 3 shared-library entries, 2 programs, 6 headers.

```
/usr/local/lib/libluajit-5.1.so
/usr/local/lib/libluajit-5.1.so.2
/usr/local/lib/libluajit-5.1.so.2.1.1788856981
/usr/local/bin/luajit
/usr/local/bin/luajit-2.1.1788856981
```

The complete list is `stage/<arch>/.manifest/luajit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH luajit
ls repo/8.0/*/ | grep '^luajit-'
```

This writes `luajit-<ver>-r0.tar.gz` (run time) and `luajit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install luajit
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/luajit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/luajit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_luajit`](../../tests/src/) links luajit the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_luajit root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_luajit'
```

```
== luajit ==
  ok   ...
PASS luajit
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
# LuaJIT is a rolling release off the v2.1 branch; pin the commit that was
# built and tested here, not the moving branch head.
VERSION=2.1.c6ffc141; GIT=https://github.com/LuaJIT/LuaJIT.git; REF=v2.1
PIN=c6ffc141a8762b41703f9287d63d93622a13dd8f
BUILDSYS=custom
# Upstream already knows QNX on the target side: lj_arch.h maps __QNX__ to
# LUAJIT_OS_POSIX and lj_prng.c / lj_profile.c have QNX branches.
#
# The build runs buildvm on THIS machine to generate the target's VM, and for a
# TARGET_SYS the Makefile does not list it compiles buildvm with LUAJIT_OS=OTHER.
# That is NOT harmless (an earlier version of this comment said it was): OTHER
# disables LJ_HASPROFILE, which shrinks jit_State, which moves the dispatch
# table the assembler VM addresses global state from.  Every such access was
# 16 bytes off and LuaJIT crashed on the first line of any script.
# files/0001 makes buildvm use POSIX for TARGET_SYS=QNX.
#
# The patch lives in files/, not patches/: the driver applies patches/ right
# after cloning the moving v2.1 head, before do_patch() checks out the pin --
# so do_patch() applies it itself, to the pinned tree.
do_patch() {
  cd "$SRCROOT" || return 1
  git checkout -q -- . && git fetch -q --depth 1 origin "$PIN" && git checkout -q "$PIN" || return 1
  patch -p1 < "$RECIPE_DIR/luajit/files/0001-cross-qnx-buildvm-posix-layout.patch"
}
do_configure() { :; }
_ljmake() {
  make -C "$S" -j"$(nproc)" \
    HOST_CC="gcc" \
    CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    TARGET_SYS=QNX \
    TARGET_FLAGS="-D_QNX_SOURCE" \
    PREFIX="$PREFIX" \
    LDCONFIG=: \
    "$@"
}
do_build()   { _ljmake clean >/dev/null 2>&1; _ljmake amalg; }
do_install() { _ljmake install DESTDIR="$STAGE"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
