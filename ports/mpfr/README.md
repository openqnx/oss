# mpfr 4.2.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz |
| SHA-256 | `277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gmp`](../gmp/) |
| Run-time dependencies | [`gmp`](../gmp/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mpfr`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install mpfr
oss-get install mpfr-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mpfr` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gmp
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz
mkdir -p dist src/mpfr
curl -fL -o dist/$(basename $URL) $URL
echo "277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mpfr --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mpfr && cd build/$QNX_ARCH/mpfr
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/mpfr/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-gmp="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mpfr`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libmpfr.so
/usr/local/lib/libmpfr.so.8
```

The complete list is `stage/<arch>/.manifest/mpfr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH mpfr
ls repo/8.0/*/ | grep '^mpfr-'
```

This writes `mpfr-<ver>-r0.tar.gz` (run time) and `mpfr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install mpfr
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/mpfr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/mpfr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_mpfr`](../../tests/src/) links mpfr the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mpfr root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_mpfr'
```

```
== mpfr ==
  ok   ...
PASS mpfr
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.2.1; TARBALL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz
SHA256=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
DEPS="gmp"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gmp="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
