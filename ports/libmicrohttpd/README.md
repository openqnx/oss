# libmicrohttpd 1.0.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-1.0.1.tar.gz |
| SHA-256 | `a89e09fc9b4de34dde19f4fcb4faaa1ce10299b9908db1132bbfa1de47882b94` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libmicrohttpd`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libmicrohttpd
oss-get install libmicrohttpd-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libmicrohttpd` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-1.0.1.tar.gz
mkdir -p dist src/libmicrohttpd
curl -fL -o dist/$(basename $URL) $URL
echo "a89e09fc9b4de34dde19f4fcb4faaa1ce10299b9908db1132bbfa1de47882b94  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libmicrohttpd --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libmicrohttpd && cd build/$QNX_ARCH/libmicrohttpd
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libmicrohttpd/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-examples \
    --disable-tools \
    --disable-https \
    --without-gnutls \
    --disable-epoll
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- QNX 8 epoll (libepoll) has no EPOLLET or EPOLLRDHUP, both of which the epoll backend relies on; the poll() backend is used instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libmicrohttpd`.

### 5. What it installs

4 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libmicrohttpd.so
/usr/local/lib/libmicrohttpd.so.74
```

The complete list is `stage/<arch>/.manifest/libmicrohttpd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libmicrohttpd
ls repo/8.0/*/ | grep '^libmicrohttpd-'
```

This writes `libmicrohttpd-<ver>-r0.tar.gz` (run time) and `libmicrohttpd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libmicrohttpd
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libmicrohttpd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libmicrohttpd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libmicrohttpd`](../../tests/src/) links libmicrohttpd the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libmicrohttpd root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libmicrohttpd'
```

```
== libmicrohttpd ==
  ok   ...
PASS libmicrohttpd
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.1
TARBALL=https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-1.0.1.tar.gz
SHA256=a89e09fc9b4de34dde19f4fcb4faaa1ce10299b9908db1132bbfa1de47882b94
BUILDSYS=autotools
# QNX 8 epoll (libepoll) has no EPOLLET or EPOLLRDHUP, both of which the epoll
# backend relies on; the poll() backend is used instead.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-examples --disable-tools
                  --disable-https --without-gnutls
                  --disable-epoll )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
