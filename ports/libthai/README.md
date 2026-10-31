# libthai 0.1.29

> Thai language support library (word breaking, input, sorting)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/tlwg/libthai.git (tag `v0.1.29`) |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`libdatrie`](../libdatrie/) |
| Run-time dependencies | [`libdatrie`](../libdatrie/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libthai`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libthai` (programs, shared libraries, run-time data) and `libthai-dev` (headers, pkg-config/CMake files, static libraries; needs `libthai`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libthai`](https://openqnx.github.io/oss/package/x86_64/libthai.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libthai.html)) · [`libthai-dev`](https://openqnx.github.io/oss/package/x86_64/libthai-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libthai-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libthai` (dependencies come along); one version: `apk add libthai=<version>` | `apk del libthai` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libthai` (dependencies first) | `oss-get remove libthai` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libthai-dev` the same way, by its own name (`apk add libthai-dev`, `oss-get install libthai-dev`, by hand with its own file and list `libthai-dev.list`); remove it before `libthai`, which it needs.

```sh
# on the QNX target, as root
apk add libthai libthai-dev
apk add qnx-test-libthai && /usr/local/bin/qnx-port-tests/test_libthai    # its on-target test
apk del libthai libthai-dev qnx-test-libthai
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libthai.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libthai.list
rm var/lib/manual/libthai.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libthai` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libdatrie
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.1.29 --recurse-submodules --shallow-submodules \
    https://github.com/tlwg/libthai.git src/libthai
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libthai < ports/libthai/patches/0001-drop-ax-compare-version.patch
```

**[`0001-drop-ax-compare-version.patch`](patches/0001-drop-ax-compare-version.patch)** — libthai: do not use AX_COMPARE_VERSION in configure.

> The macro comes from autoconf-archive, which is not a build dependency, and
> the shipped configure contains it unexpanded, so the shell cannot parse
> configure even with the doxygen docs off. The docs are not built here.

Touches: `configure.ac` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libthai && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libthai && cd build/$QNX_ARCH/libthai
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libthai/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doxygen-doc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Same two problems as libdatrie: build-aux is kept out of git, and the shipped ./configure contains an unexpanded AX_COMPARE_VERSION that the shell cannot parse. Patch 0001 replaces that call; here we drop in automake's own auxiliary scripts and regenerate. (autogen.sh is no good either -- it runs ./configure for the build host, and the cross configure then refuses with "already configured".)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libthai`.

### 5. What it installs

21 files under `/usr/local`: 2 shared-library entries, 17 headers.

```
/usr/local/lib/libthai.so
/usr/local/lib/libthai.so.3
```

The complete list is `stage/<arch>/.manifest/libthai` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libthai
ls dists/8.0/*/ | grep '^libthai-'
```

This writes `libthai-<ver>-r0.tar.gz` (run time) and `libthai-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libthai
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libthai-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libthai-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libthai`](../../tests/src/) links libthai the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libthai`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libthai
/usr/local/bin/qnx-port-tests/test_libthai
```

```
== libthai ==
  ok   ...
PASS libthai
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libthai` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libthai`](../../tests/src/) -- a program linked against libthai the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - th_isthai on a Thai byte
   - th_isthai on ASCII
   - th_isupvowel classifies a consonant correctly
   - th_brk found a word boundary (the dictionary loaded)
   - the break is inside the string
   - break positions are ordered
   - th_brk_line inserted the delimiter

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libthai 0.1.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libthai-0.1.29-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libthai 0.1.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libthai-0.1.29-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.1.29**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libthai=<version>`), sources in `ported/libthai/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libthai`; port one with `scripts/port-update libthai`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.1.29; GIT=https://github.com/tlwg/libthai.git; REF=v0.1.29
DEPS="libdatrie"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doxygen-doc )
# Same two problems as libdatrie: build-aux is kept out of git, and the shipped
# ./configure contains an unexpanded AX_COMPARE_VERSION that the shell cannot
# parse. Patch 0001 replaces that call; here we drop in automake's own
# auxiliary scripts and regenerate.
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
