# libdatrie 0.2.13

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

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libdatrie
oss-get install libdatrie-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH libdatrie
ls repo/8.0/*/ | grep '^libdatrie-'
```

This writes `libdatrie-<ver>-r0.tar.gz` (run time) and `libdatrie-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libdatrie
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libdatrie-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libdatrie-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libdatrie`](../../tests/src/) links libdatrie the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdatrie root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libdatrie'
```

```
== libdatrie ==
  ok   ...
PASS libdatrie
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
