# gnutls 3.8.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-3.8.8.tar.xz |
| SHA-256 | `ac4f020e583880b51380ed226e59033244bc536cad2623f2e26f5afa2939d8fb` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`nettle`](../nettle/) [`gmp`](../gmp/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libidn2`](../libidn2/) [`zlib`](../zlib/) [`brotli`](../brotli/) [`zstd`](../zstd/) |
| Run-time dependencies | [`brotli`](../brotli/) [`gmp`](../gmp/) [`libidn2`](../libidn2/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`nettle`](../nettle/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gnutls`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gnutls
oss-get install gnutls-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gnutls` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd
```

<details><summary>Full build chain (11)</summary>

[`brotli`](../brotli/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-3.8.8.tar.xz
mkdir -p dist src/gnutls
curl -fL -o dist/$(basename $URL) $URL
echo "ac4f020e583880b51380ed226e59033244bc536cad2623f2e26f5afa2939d8fb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gnutls --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gnutls && cd build/$QNX_ARCH/gnutls
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/gnutls/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-tests \
    --disable-tools \
    --disable-cxx \
    --without-p11-kit \
    --with-included-libtasn1=no \
    --disable-guile \
    --disable-nls \
    --with-included-unistring=no
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): brotli zstd

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gnutls`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 17 headers.

```
/usr/local/lib/libgnutls.so
/usr/local/lib/libgnutls.so.70
```

The complete list is `stage/<arch>/.manifest/gnutls` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gnutls
ls repo/8.0/*/ | grep '^gnutls-'
```

This writes `gnutls-<ver>-r0.tar.gz` (run time) and `gnutls-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gnutls
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gnutls-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gnutls-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gnutls`](../../tests/src/) links gnutls the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gnutls root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gnutls'
```

```
== gnutls ==
  ok   ...
PASS gnutls
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.8.8
TARBALL=https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-3.8.8.tar.xz
SHA256=ac4f020e583880b51380ed226e59033244bc536cad2623f2e26f5afa2939d8fb
BUILDSYS=autotools
# Also linked at run time (found by scripts/gen-dependency-index): brotli zstd
DEPS="nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-tools --disable-cxx --without-p11-kit --with-included-libtasn1=no
                  --disable-guile --disable-nls --with-included-unistring=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
