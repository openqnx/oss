# tcl 8.6.17

> Tcl scripting language: tclsh, the embeddable libtcl and the bundled packages

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/sourceforge/tcl/tcl8.6.17-src.tar.gz |
| SHA-256 | `a3903371efcce8a405c5c245d029e9f6850258a60fa3761c4d58995610949b31` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tclsh`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `tcl` (programs, shared libraries, run-time data) and `tcl-dev` (headers, pkg-config/CMake files, static libraries; needs `tcl`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tcl`](https://openqnx.github.io/oss/package/x86_64/tcl.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tcl.html)) · [`tcl-dev`](https://openqnx.github.io/oss/package/x86_64/tcl-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tcl-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tcl` (dependencies come along); one version: `apk add tcl=<version>` | `apk del tcl` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tcl` (dependencies first) | `oss-get remove tcl` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`tcl-dev` the same way, by its own name (`apk add tcl-dev`, `oss-get install tcl-dev`, by hand with its own file and list `tcl-dev.list`); remove it before `tcl`, which it needs.

```sh
# on the QNX target, as root
apk add tcl tcl-dev
apk add qnx-test-tclsh && /usr/local/bin/qnx-port-tests/test_tclsh    # its on-target test, from tests-8.0-<arch>-m-z
apk del tcl tcl-dev qnx-test-tclsh
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tcl.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tcl.list
rm var/lib/manual/tcl.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tcl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/sourceforge/tcl/tcl8.6.17-src.tar.gz
mkdir -p dist src/tcl
curl -fL -o dist/$(basename $URL) $URL
echo "a3903371efcce8a405c5c245d029e9f6850258a60fa3761c4d58995610949b31  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tcl --strip-components=1
```

The project lives in the `unix/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tcl/unix; B=build/$QNX_ARCH/tcl; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" INSTALL_ROOT="$STAGE" install install-private-headers || return 1
  ln -sf tclsh8.6 "$STAGE$PREFIX/bin/tclsh"
  local f
  for f in $(find "$STAGE$PREFIX/lib" -name '*Config.sh'); do tcl_fix_config "$f" || return 1; done
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Tcl: the Tool Command Language -- tclsh8.6 (and tclsh), libtcl8.6.so for programs that embed it, libtclstub8.6.a for extensions, the script library (lib/tcl8.6) and the bundled packages built with it (Thread, TDBC, itcl, SQLite's Tcl binding). Version as in QNX's own package list.
- tclConfig.sh, itclConfig.sh and tdbcConfig.sh also describe the build tree (*_BUILD_*, *_SRC_DIR: for building extensions against an uninstalled Tcl). In the package those name this machine's directories: each build-tree value becomes its installed counterpart (TCL_BUILD_LIB_SPEC -> TCL_LIB_SPEC's value), the include spec the installed headers, and the source and library directories empty.
- tclsh: the unversioned name scripts' #! lines use (as Alpine and QNX ship it).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tcl`.

### 5. What it installs

1156 files under `/usr/local`: 1 shared-library entries, 3 programs, 31 headers.

```
/usr/local/lib/libtcl8.6.so
/usr/local/bin/sqlite3_analyzer
/usr/local/bin/tclsh
/usr/local/bin/tclsh8.6
```

The complete list is `stage/<arch>/.manifest/tcl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tcl
ls dists/8.0/*/ | grep '^tcl-'
```

This writes `tcl-<ver>-r0.tar.gz` (run time) and `tcl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tcl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tcl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tcl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tclsh`](../../tests/src/) links tcl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tclsh`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tclsh
/usr/local/bin/qnx-port-tests/test_tclsh
```

```
== tcl ==
  ok   ...
PASS tcl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tclsh` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tclsh`](../../tests/src/) -- a program linked against tcl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tcl 8.6.17-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/tcl-8.6.17-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tcl 8.6.17-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/tcl-8.6.17-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.6.17**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tcl=<version>`), sources in `ported/tcl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tcl`; port one with `scripts/port-update tcl`.

<details><summary>recipe.sh</summary>

```bash
# Tcl: the Tool Command Language -- tclsh8.6 (and tclsh), libtcl8.6.so for
# programs that embed it, libtclstub8.6.a for extensions, the script library
# (lib/tcl8.6) and the bundled packages built with it (Thread, TDBC, itcl,
# SQLite's Tcl binding). Version as in QNX's own package list.
VERSION=8.6.17
TARBALL=https://downloads.sourceforge.net/sourceforge/tcl/tcl8.6.17-src.tar.gz
SHA256=a3903371efcce8a405c5c245d029e9f6850258a60fa3761c4d58995610949b31
BUILDSYS=autotools
SUBDIR=unix
DEPS="zlib"
CONF_ARGS_EXTRA=( --enable-threads --enable-64bit --enable-shared )
# tclConfig.sh, itclConfig.sh and tdbcConfig.sh also describe the build
# tree (*_BUILD_*, *_SRC_DIR: for building extensions against an
# uninstalled Tcl). In the package those name this machine's directories:
# each build-tree value becomes its installed counterpart (TCL_BUILD_LIB_SPEC
# -> TCL_LIB_SPEC's value), the include spec the installed headers, and the
# source and library directories empty.
tcl_fix_config() {
  awk -v ws="$WS" -v inc="-I$PREFIX/include" '
    { line[NR] = $0; if (match($0, /^[A-Za-z_]+=/)) { n = substr($0, 1, RLENGTH - 1); val[n] = substr($0, RLENGTH + 1) } }
    END {
      for (i = 1; i <= NR; i++) {
        l = line[i]
        if (index(l, ws) && match(l, /^[A-Za-z_]+=/)) {
          n = substr(l, 1, RLENGTH - 1); q = substr(l, RLENGTH + 1, 1); if (q != "\"" && q != "\047") q = ""
          c = n; sub(/_BUILD_/, "_", c)
          if (c != n && (c in val) && !index(val[c], ws)) l = n "=" val[c]
          else if (n ~ /_SRC_DIR$|_BUILD_LIBRARY_PATH$/) l = n "=" q q
          else if (n ~ /INCLUDE_SPEC$/) l = n "=" q inc q
        }
        print l
      }
    }' "$1" > "$1.tmp" && mv "$1.tmp" "$1"
}
# tclsh: the unversioned name scripts' #! lines use (as Alpine and QNX ship it).
do_install() {
  make -C "$B" INSTALL_ROOT="$STAGE" install install-private-headers || return 1
  ln -sf tclsh8.6 "$STAGE$PREFIX/bin/tclsh"
  local f
  for f in $(find "$STAGE$PREFIX/lib" -name '*Config.sh'); do tcl_fix_config "$f" || return 1; done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
