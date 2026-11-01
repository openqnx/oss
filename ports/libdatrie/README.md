# libdatrie 0.2.13

> Double-array trie data structure library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/tlwg/libdatrie.git (tag `v0.2.13`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libdatrie`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libdatrie` (programs, shared libraries, run-time data) and `libdatrie-dev` (headers, pkg-config/CMake files, static libraries; needs `libdatrie`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libdatrie`](https://openqnx.github.io/oss/package/x86_64/libdatrie.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdatrie.html)) · [`libdatrie-dev`](https://openqnx.github.io/oss/package/x86_64/libdatrie-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdatrie-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libdatrie` (dependencies come along); one version: `apk add libdatrie=<version>` | `apk del libdatrie` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libdatrie` (dependencies first) | `oss-get remove libdatrie` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libdatrie-dev` the same way, by its own name (`apk add libdatrie-dev`, `oss-get install libdatrie-dev`, by hand with its own file and list `libdatrie-dev.list`); remove it before `libdatrie`, which it needs.

```sh
# on the QNX target, as root
apk add libdatrie libdatrie-dev
apk add qnx-test-libdatrie && /usr/local/bin/qnx-port-tests/test_libdatrie    # its on-target test, from tests-8.0-<arch>
apk del libdatrie libdatrie-dev qnx-test-libdatrie
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libdatrie.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libdatrie.list
rm var/lib/manual/libdatrie.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdatrie` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.2.13 --recurse-submodules --shallow-submodules \
    https://github.com/tlwg/libdatrie.git src/libdatrie
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libdatrie < ports/libdatrie/patches/0001-drop-ax-compare-version.patch
```

**[`0001-drop-ax-compare-version.patch`](patches/0001-drop-ax-compare-version.patch)** — libdatrie: do not use AX_COMPARE_VERSION in configure.

> The macro comes from autoconf-archive, which is not a build dependency, and
> the shipped configure contains it unexpanded, so the shell cannot parse
> configure even with the doxygen docs off. The docs are not built here.

Touches: `configure.ac` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libdatrie && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libdatrie && cd build/$QNX_ARCH/libdatrie
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libdatrie/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doxygen-doc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Two problems, both from autoconf-archive not being installed here: build-aux is kept out of git, and configure.ac calls AX_COMPARE_VERSION -- which the shipped ./configure contains *unexpanded*, so the shell cannot even parse it, disabled doxygen or not. Patch 0001 replaces that call; here we drop in automake's own auxiliary scripts and regenerate. (autogen.sh is no good either -- it runs ./configure for the build host, and the cross configure then refuses with "already configured".)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdatrie`.

### 5. What it installs

11 files under `/usr/local`: 2 shared-library entries, 2 programs, 4 headers.

```
/usr/local/lib/libdatrie.so
/usr/local/lib/libdatrie.so.5
/usr/local/bin/trietool
/usr/local/bin/trietool-0.2
```

The complete list is `stage/<arch>/.manifest/libdatrie` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libdatrie
ls dists/8.0/*/ | grep '^libdatrie-'
```

This writes `libdatrie-<ver>-r0.tar.gz` (run time) and `libdatrie-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libdatrie
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libdatrie-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libdatrie-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libdatrie`](../../tests/src/) links libdatrie the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libdatrie`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libdatrie
/usr/local/bin/qnx-port-tests/test_libdatrie
```

```
== libdatrie ==
  ok   ...
PASS libdatrie
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdatrie` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libdatrie`](../../tests/src/) -- a program linked against libdatrie the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - alpha_map_new
   - add the a-z range
   - trie_new
   - trie_store(qnx)
   - trie_store(qnode)
   - trie_retrieve(qnx)
   - trie_retrieve(qnode)
   - an absent key is not found
   - trie_enumerate
   - enumeration visited both entries
   - trie_delete
   - the deleted key is gone

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdatrie 0.2.13-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libdatrie-0.2.13-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdatrie 0.2.13-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libdatrie-0.2.13-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.2.13**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libdatrie=<version>`), sources in `ported/libdatrie/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libdatrie`; port one with `scripts/port-update libdatrie`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.2.13; GIT=https://github.com/tlwg/libdatrie.git; REF=v0.2.13
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doxygen-doc )
# Two problems, both from autoconf-archive not being installed here:
# build-aux is kept out of git, and configure.ac calls AX_COMPARE_VERSION --
# which the shipped ./configure contains *unexpanded*, so the shell cannot
# even parse it, disabled doxygen or not. Patch 0001 replaces that call;
# here we drop in automake's own auxiliary scripts and regenerate.
# (autogen.sh is no good either -- it runs ./configure for the build host,
# and the cross configure then refuses with "already configured".)
do_patch() {
  local aux; aux=$(ls -d /usr/share/automake-*/ 2>/dev/null | head -1)
  [ -n "$aux" ] || { echo "no automake data directory found"; return 1; }
  mkdir -p "$SRCROOT/build-aux"
  local f
  for f in config.guess config.sub compile missing install-sh depcomp ar-lib; do
    [ -e "$aux/$f" ] && cp -f "$aux/$f" "$SRCROOT/build-aux/$f"
  done
  chmod +x "$SRCROOT/build-aux/"*
  ( cd "$SRCROOT" && autoreconf -fi )
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
