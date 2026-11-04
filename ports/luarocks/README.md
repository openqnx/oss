# luarocks 3.12.2

> LuaRocks, the package manager for Lua modules (rocks)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://luarocks.org/releases/luarocks-3.12.2.tar.gz |
| SHA-256 | `b0e0c85205841ddd7be485f53d6125766d18a81d226588d2366931e9a1484492` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_luarocks`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `luarocks`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`luarocks`](https://openqnx.github.io/oss/package/x86_64/luarocks.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/luarocks.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add luarocks` (dependencies come along); one version: `apk add luarocks=<version>` | `apk del luarocks` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install luarocks` (dependencies first) | `oss-get remove luarocks` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-luarocks && /usr/local/bin/qnx-port-tests/test_luarocks    # its on-target test, from tests-8.0-<arch>
apk del luarocks qnx-test-luarocks
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/luarocks.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/luarocks.list
rm var/lib/manual/luarocks.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild luarocks` performs exactly the same steps (2–5) in one command.

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
URL=https://luarocks.org/releases/luarocks-3.12.2.tar.gz
mkdir -p dist src/luarocks
curl -fL -o dist/$(basename $URL) $URL
echo "b0e0c85205841ddd7be485f53d6125766d18a81d226588d2366931e9a1484492  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/luarocks --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/luarocks; B=build/$QNX_ARCH/luarocks; mkdir -p $B
```

```bash
do_configure() {
  cat > "$S/config.unix" <<CFG
prefix=$PREFIX
sysconfdir=$PREFIX/etc
rocks_tree=$PREFIX
LUA_VERSION=5.3
LUA=$PREFIX/bin/lua5.3
LUA_DIR=$PREFIX
LUA_BINDIR=$PREFIX/bin
LUA_INCDIR=$PREFIX/include/lua5.3
LUA_LIBDIR=$PREFIX/lib
FORCE_CONFIG=
CFG
}
```

```bash
do_build() { make -C "$S" build; }
```

```bash
do_install() {
  make -C "$S" install DESTDIR="$STAGE" || return 1
  lr_config 5.3 lua5.3 > "$STAGE$PREFIX/etc/luarocks/config-5.3.lua"
  lr_config 5.2 lua5.2 > "$STAGE$PREFIX/etc/luarocks/config-5.2.lua"
  lr_config 5.1 lua5.1 > "$STAGE$PREFIX/etc/luarocks/config-5.1.lua"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LuaRocks, the Lua package manager: luarocks and luarocks-admin, run by the Lua 5.3 interpreter (lua5.3: the newest Lua here with an interpreter), its modules in share/lua/5.3/luarocks. It installs rocks for Lua 5.3 by default and, with --lua-version=5.1 / 5.2, for the lua5.1 (and LuaJIT) and lua5.2 interpreters: one configuration per version in etc/luarocks. The rocks tree is the prefix (share/lua/5.x, lib/lua/5.x, lib/luarocks/ rocks-5.x), where the interpreters look by default. Fetching from the network uses curl; .tar.gz sources are unpacked with tar and gzip (all in the QNX base system); .rock/.zip archives need zip/unzip. Version as in QNX's own package list.
- upstream's configure runs the Lua interpreter to find out about it, which a cross build cannot do: write what it would have found (config.unix) and let the makefile generate the scripts and configuration from it.
- The configuration of one Lua version: the interpreter and its headers and library (C rocks), as LuaRocks' configure writes it. LUALIB names the versioned library (liblua5.<n>.so): LuaRocks would look for liblua.so.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH luarocks`.

### 5. What it installs

106 files under `/usr/local`, 2 programs.

```
/usr/local/bin/luarocks
/usr/local/bin/luarocks-admin
```

The complete list is `stage/<arch>/.manifest/luarocks` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa luarocks
ls dists/8.0/*/ | grep '^luarocks-'
```

This writes `luarocks-<ver>-r0.tar.gz` (run time) and `luarocks-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add luarocks
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/luarocks-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/luarocks-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_luarocks`](../../tests/src/) links luarocks the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-luarocks`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-luarocks
/usr/local/bin/qnx-port-tests/test_luarocks
```

```
== luarocks ==
  ok   ...
PASS luarocks
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_luarocks` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_luarocks`](../../tests/src/) -- a program linked against luarocks the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> luarocks`.

## Versions

This recipe builds **3.12.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add luarocks=<version>`), sources in `ported/luarocks/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates luarocks`; port one with `scripts/port-update luarocks`.

<details><summary>recipe.sh</summary>

```bash
# LuaRocks, the Lua package manager: luarocks and luarocks-admin, run by the
# Lua 5.3 interpreter (lua5.3: the newest Lua here with an interpreter), its
# modules in share/lua/5.3/luarocks. It installs rocks for Lua 5.3 by
# default and, with --lua-version=5.1 / 5.2, for the lua5.1 (and LuaJIT) and
# lua5.2 interpreters: one configuration per version in etc/luarocks. The
# rocks tree is the prefix (share/lua/5.x, lib/lua/5.x, lib/luarocks/
# rocks-5.x), where the interpreters look by default. Fetching from the
# network uses curl; .tar.gz sources are unpacked with tar and gzip (all in
# the QNX base system); .rock/.zip archives need zip/unzip. Version as in
# QNX's own package list.
VERSION=3.12.2
TARBALL=https://luarocks.org/releases/luarocks-3.12.2.tar.gz
SHA256=b0e0c85205841ddd7be485f53d6125766d18a81d226588d2366931e9a1484492
BUILDSYS=custom
DEPS=""
RUNTIME_DEPS="lua5.3"
# upstream's configure runs the Lua interpreter to find out about it, which
# a cross build cannot do: write what it would have found (config.unix) and
# let the makefile generate the scripts and configuration from it.
do_configure() {
  cat > "$S/config.unix" <<CFG
prefix=$PREFIX
sysconfdir=$PREFIX/etc
rocks_tree=$PREFIX
LUA_VERSION=5.3
LUA=$PREFIX/bin/lua5.3
LUA_DIR=$PREFIX
LUA_BINDIR=$PREFIX/bin
LUA_INCDIR=$PREFIX/include/lua5.3
LUA_LIBDIR=$PREFIX/lib
FORCE_CONFIG=
CFG
}
do_build() { make -C "$S" build; }
# The configuration of one Lua version: the interpreter and its headers and
# library (C rocks), as LuaRocks' configure writes it. LUALIB names the
# versioned library (liblua5.<n>.so): LuaRocks would look for liblua.so.
lr_config() {
  local v=$1 lua=$2
  cat <<CFG
-- LuaRocks configuration for Lua $v
rocks_trees = {
   { name = "user", root = home .. "/.luarocks" };
   { name = "system", root = "$PREFIX" };
}
variables = {
   LUA_DIR = "$PREFIX";
   LUA_INCDIR = "$PREFIX/include/lua$v";
   LUA_BINDIR = "$PREFIX/bin";
   LUA_LIBDIR = "$PREFIX/lib";
   LUA_VERSION = "$v";
   LUA = "$PREFIX/bin/$lua";
   LUALIB = "liblua$v.so";
}
CFG
}
do_install() {
  make -C "$S" install DESTDIR="$STAGE" || return 1
  lr_config 5.3 lua5.3 > "$STAGE$PREFIX/etc/luarocks/config-5.3.lua"
  lr_config 5.2 lua5.2 > "$STAGE$PREFIX/etc/luarocks/config-5.2.lua"
  lr_config 5.1 lua5.1 > "$STAGE$PREFIX/etc/luarocks/config-5.1.lua"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
