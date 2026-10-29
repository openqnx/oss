# mpg123 1.32.9

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.mpg123.de/download/mpg123-1.32.9.tar.bz2 |
| SHA-256 | `03b61e4004e960bacf2acdada03ed94d376e6aab27a601447bd4908d8407b291` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mpg123`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install mpg123
oss-get install mpg123-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mpg123` performs exactly the same steps (2–5) in one command.

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
URL=https://www.mpg123.de/download/mpg123-1.32.9.tar.bz2
mkdir -p dist src/mpg123
curl -fL -o dist/$(basename $URL) $URL
echo "03b61e4004e960bacf2acdada03ed94d376e6aab27a601447bd4908d8407b291  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mpg123 --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mpg123 && cd build/$QNX_ARCH/mpg123
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/mpg123/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-cpu=generic_fpu \
    --disable-components \
    --enable-libmpg123 \
    --enable-libsyn123
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The decoder (libmpg123) and resampler (libsyn123) libraries only. QNX audio goes through io-snd/libasound and mpg123 has no output module for it, so libout123 and the player are left out: applications decode with libmpg123 and play through their own audio path. --disable-components turns everything off; each --enable-lib* then turns one library back on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mpg123`.

### 5. What it installs

9 files under `/usr/local`: 4 shared-library entries, 3 headers.

```
/usr/local/lib/libmpg123.so
/usr/local/lib/libmpg123.so.48
/usr/local/lib/libsyn123.so
/usr/local/lib/libsyn123.so.2
```

The complete list is `stage/<arch>/.manifest/mpg123` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH mpg123
ls repo/8.0/*/ | grep '^mpg123-'
```

This writes `mpg123-<ver>-r0.tar.gz` (run time) and `mpg123-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install mpg123
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/mpg123-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/mpg123-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_mpg123`](../../tests/src/) links mpg123 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mpg123 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_mpg123'
```

```
== mpg123 ==
  ok   ...
PASS mpg123
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.32.9
TARBALL=https://www.mpg123.de/download/mpg123-1.32.9.tar.bz2
SHA256=03b61e4004e960bacf2acdada03ed94d376e6aab27a601447bd4908d8407b291
BUILDSYS=autotools
# The decoder (libmpg123) and resampler (libsyn123) libraries only. QNX audio
# goes through io-snd/libasound and mpg123 has no output module for it, so
# libout123 and the player are left out: applications decode with libmpg123 and
# play through their own audio path. --disable-components turns everything off;
# each --enable-lib* then turns one library back on.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-cpu=generic_fpu
                  --disable-components --enable-libmpg123 --enable-libsyn123 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
