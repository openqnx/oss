# luajit 2.1.c6ffc141

> Just-in-time compiler for the Lua programming language

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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `luajit` (programs, shared libraries, run-time data) and `luajit-dev` (headers, pkg-config/CMake files, static libraries; needs `luajit`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`luajit`](https://openqnx.github.io/oss/package/x86_64/luajit.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/luajit.html)) · [`luajit-dev`](https://openqnx.github.io/oss/package/x86_64/luajit-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/luajit-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add luajit` (dependencies come along); one version: `apk add luajit=<version>` | `apk del luajit` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install luajit` (dependencies first) | `oss-get remove luajit` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`luajit-dev` the same way, by its own name (`apk add luajit-dev`, `oss-get install luajit-dev`, by hand with its own file and list `luajit-dev.list`); remove it before `luajit`, which it needs.

```sh
# on the QNX target, as root
apk add luajit luajit-dev
apk add qnx-test-luajit && /usr/local/bin/qnx-port-tests/test_luajit    # its on-target test, from tests-8.0-<arch>
apk del luajit luajit-dev qnx-test-luajit
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/luajit.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/luajit.list
rm var/lib/manual/luajit.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa luajit
ls dists/8.0/*/ | grep '^luajit-'
```

This writes `luajit-<ver>-r0.tar.gz` (run time) and `luajit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add luajit
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/luajit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/luajit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_luajit`](../../tests/src/) links luajit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-luajit`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-luajit
/usr/local/bin/qnx-port-tests/test_luajit
```

```
== luajit ==
  ok   ...
PASS luajit
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_luajit` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_luajit`](../../tests/src/) -- a program linked against luajit the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - luaL_newstate
   - jit.os is POSIX (lj_arch.h saw __QNX__)
   - hot loop computes the right answer
   - the hot loop was compiled into a trace
   - require('luv')
   - uv.fs_stat('/') reports a directory

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | luajit 2.1_git-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/luajit-2.1_git-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | luajit 2.1_git-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/luajit-2.1_git-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.1.c6ffc141**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add luajit=<version>`), sources in `ported/luajit/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates luajit`; port one with `scripts/port-update luajit`.

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
