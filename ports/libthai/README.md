# libthai 0.1.29

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

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libthai
apk add libthai-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libthai && /usr/local/bin/qnx-port-tests/test_libthai    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libthai`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

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
