# lua-compat53 0.14.4

> Lua 5.3-style standard library functions for Lua 5.1, 5.2 and LuaJIT

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/lunarmodules/lua-compat-5.3.git (tag `v0.14.4`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`lua5.1`](../lua5.1/) [`lua5.2`](../lua5.2/) [`lua5.3`](../lua5.3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lua_compat53`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `lua-compat53` (programs, shared libraries, run-time data) and `lua-compat53-dev` (headers, pkg-config/CMake files, static libraries; needs `lua-compat53`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lua-compat53`](https://openqnx.github.io/oss/package/x86_64/lua-compat53.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua-compat53.html)) · [`lua-compat53-dev`](https://openqnx.github.io/oss/package/x86_64/lua-compat53-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lua-compat53-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lua-compat53` (dependencies come along); one version: `apk add lua-compat53=<version>` | `apk del lua-compat53` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lua-compat53` (dependencies first) | `oss-get remove lua-compat53` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`lua-compat53-dev` the same way, by its own name (`apk add lua-compat53-dev`, `oss-get install lua-compat53-dev`, by hand with its own file and list `lua-compat53-dev.list`); remove it before `lua-compat53`, which it needs.

```sh
# on the QNX target, as root
apk add lua-compat53 lua-compat53-dev
apk add qnx-test-lua-compat53 && /usr/local/bin/qnx-port-tests/test_lua_compat53    # its on-target test, from tests-8.0-<arch>
apk del lua-compat53 lua-compat53-dev qnx-test-lua-compat53
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lua-compat53.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lua-compat53.list
rm var/lib/manual/lua-compat53.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lua-compat53` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild lua5.1 lua5.2 lua5.3
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.14.4 --recurse-submodules --shallow-submodules \
    https://github.com/lunarmodules/lua-compat-5.3.git src/lua-compat53
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lua-compat53; B=build/$QNX_ARCH/lua-compat53; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  local v m
  for v in $LC53_LUAS; do
    mkdir -p "$B/$v"
    # As luarocks' builtin type builds the rockspec's modules: one shared
    # object per C file, unresolved Lua API symbols left to the interpreter
    # that loads it (liblua5.<n>.so is already in the process).
    for m in string:lstrlib table:ltablib utf8:lutf8lib io:liolib; do
      qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$S" -I"$STAGE_SYSROOT$PREFIX/include/lua$v" \
          -o "$B/$v/${m%%:*}.so" "$S/${m#*:}.c" || return 1
    done
  done
}
```

```bash
do_install() {
  local v
  for v in $LC53_LUAS; do
    install -d "$STAGE$PREFIX/lib/lua/$v/compat53" "$STAGE$PREFIX/share/lua/$v/compat53"
    install -m755 "$B/$v"/*.so "$STAGE$PREFIX/lib/lua/$v/compat53/"
    install -m644 "$S"/compat53/*.lua "$STAGE$PREFIX/share/lua/$v/compat53/"
  done
  install -d "$STAGE$PREFIX/include/lua-compat53/c-api"
  install -m644 "$S"/c-api/compat-5.3.h "$S"/c-api/compat-5.3.c "$STAGE$PREFIX/include/lua-compat53/c-api/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- lua-compat53: Lua 5.3's standard-library behaviour for Lua 5.1 and 5.2 (and LuaJIT, which loads 5.1 modules): require "compat53" patches the global functions; require "compat53.module" returns them without touching the globals. The string, table, utf8 and io parts are C modules, built here once per Lua version: share/lua/5.<n>/compat53/{init,module,file_mt}.lua lib/lua/5.<n>/compat53/{string,table,utf8,io}.so for 5.1, 5.2 and 5.3 (on 5.3 it is a no-op, so code can require it unconditionally), and the C API shim c-api/compat-5.3.{h,c} (C modules written against the 5.3 API) under include/lua-compat53. Version as in QNX's own package list.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lua-compat53`.

### 5. What it installs

23 files under `/usr/local`, 2 headers.

The complete list is `stage/<arch>/.manifest/lua-compat53` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lua-compat53
ls dists/8.0/*/ | grep '^lua-compat53-'
```

This writes `lua-compat53-<ver>-r0.tar.gz` (run time) and `lua-compat53-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lua-compat53
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lua-compat53-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lua-compat53-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lua_compat53`](../../tests/src/) links lua-compat53 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lua-compat53`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lua-compat53
/usr/local/bin/qnx-port-tests/test_lua_compat53
```

```
== lua-compat53 ==
  ok   ...
PASS lua-compat53
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lua_compat53` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lua_compat53`](../../tests/src/) -- a program linked against lua-compat53 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - lua5.3: require compat53 loads and changes nothing

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lua-compat53 0.14.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/lua-compat53-0.14.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lua-compat53 0.14.4-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/lua-compat53-0.14.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.14.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lua-compat53=<version>`), sources in `ported/lua-compat53/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lua-compat53`; port one with `scripts/port-update lua-compat53`.

<details><summary>recipe.sh</summary>

```bash
# lua-compat53: Lua 5.3's standard-library behaviour for Lua 5.1 and 5.2
# (and LuaJIT, which loads 5.1 modules): require "compat53" patches the
# global functions; require "compat53.module" returns them without touching
# the globals. The string, table, utf8 and io parts are C modules, built
# here once per Lua version:
#   share/lua/5.<n>/compat53/{init,module,file_mt}.lua
#   lib/lua/5.<n>/compat53/{string,table,utf8,io}.so
# for 5.1, 5.2 and 5.3 (on 5.3 it is a no-op, so code can require it
# unconditionally), and the C API shim c-api/compat-5.3.{h,c} (C modules
# written against the 5.3 API) under include/lua-compat53. Version as in
# QNX's own package list.
VERSION=0.14.4; GIT=https://github.com/lunarmodules/lua-compat-5.3.git; REF=v0.14.4
BUILDSYS=custom
DEPS="lua5.1 lua5.2 lua5.3"
RUNTIME_DEPS=""
LC53_LUAS="5.1 5.2 5.3"
do_configure() { :; }
do_build() {
  local v m
  for v in $LC53_LUAS; do
    mkdir -p "$B/$v"
    # As luarocks' builtin type builds the rockspec's modules: one shared
    # object per C file, unresolved Lua API symbols left to the interpreter
    # that loads it (liblua5.<n>.so is already in the process).
    for m in string:lstrlib table:ltablib utf8:lutf8lib io:liolib; do
      qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$S" -I"$STAGE_SYSROOT$PREFIX/include/lua$v" \
          -o "$B/$v/${m%%:*}.so" "$S/${m#*:}.c" || return 1
    done
  done
}
do_install() {
  local v
  for v in $LC53_LUAS; do
    install -d "$STAGE$PREFIX/lib/lua/$v/compat53" "$STAGE$PREFIX/share/lua/$v/compat53"
    install -m755 "$B/$v"/*.so "$STAGE$PREFIX/lib/lua/$v/compat53/"
    install -m644 "$S"/compat53/*.lua "$STAGE$PREFIX/share/lua/$v/compat53/"
  done
  install -d "$STAGE$PREFIX/include/lua-compat53/c-api"
  install -m644 "$S"/c-api/compat-5.3.h "$S"/c-api/compat-5.3.c "$STAGE$PREFIX/include/lua-compat53/c-api/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
