# hyphen 2.8.8

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz |
| SHA-256 | `304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705` |
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
oss-get install hyphen
oss-get install hyphen-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hyphen` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz
mkdir -p dist src/hyphen
curl -fL -o dist/$(basename $URL) $URL
echo "304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/hyphen --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/hyphen && cd build/$QNX_ARCH/hyphen
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/hyphen/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hyphen`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libhyphen.so
/usr/local/lib/libhyphen.so.3
/usr/local/bin/substrings.pl
```

The complete list is `stage/<arch>/.manifest/hyphen` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH hyphen
ls repo/8.0/*/ | grep '^hyphen-'
```

This writes `hyphen-<ver>-r0.tar.gz` (run time) and `hyphen-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install hyphen
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/hyphen-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/hyphen-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_misc_libs`](../../tests/src/) links hyphen the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_misc_libs root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_misc_libs'
```

```
== hyphen ==
  ok   ...
PASS hyphen
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.8.8
TARBALL=https://downloads.sourceforge.net/hunspell/hyphen-2.8.8.tar.gz
SHA256=304636d4eccd81a14b6914d07b84c79ebb815288c76fe027b9ebff6ff24d5705
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
