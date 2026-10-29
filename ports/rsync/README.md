# rsync 3.3.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.samba.org/pub/rsync/src/rsync-3.3.0.tar.gz |
| SHA-256 | `7399e9a6708c32d678a72a63219e96f23be0be2336e50fd1348498d07041df90` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`zstd`](../zstd/) [`lz4`](../lz4/) [`xxhash`](../xxhash/) [`openssl`](../openssl/) |
| Run-time dependencies | [`lz4`](../lz4/) [`openssl`](../openssl/) [`xxhash`](../xxhash/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install rsync
oss-get install rsync-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild rsync` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zstd lz4 xxhash openssl
```

### 2. Get the upstream source

```bash
URL=https://download.samba.org/pub/rsync/src/rsync-3.3.0.tar.gz
mkdir -p dist src/rsync
curl -fL -o dist/$(basename $URL) $URL
echo "7399e9a6708c32d678a72a63219e96f23be0be2336e50fd1348498d07041df90  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/rsync --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/rsync && cd build/$QNX_ARCH/rsync
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
rsync_cv_MAKEDEV_TAKES_3_ARGS=yes \
$OLDPWD/src/rsync/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-md2man \
    --disable-debug \
    --with-included-popt \
    --with-included-zlib
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- makedev() takes three arguments on QNX (node, major, minor). configure finds that with a run test, which cannot execute when cross compiling and then defaults to "no"; give it the answer.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH rsync`.

### 5. What it installs

2 files under `/usr/local`, 2 programs.

```
/usr/local/bin/rsync
/usr/local/bin/rsync-ssl
```

The complete list is `stage/<arch>/.manifest/rsync` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH rsync
ls repo/8.0/*/ | grep '^rsync-'
```

This writes `rsync-<ver>-r0.tar.gz` (run time) and `rsync-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install rsync
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/rsync-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/rsync-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.3.0
TARBALL=https://download.samba.org/pub/rsync/src/rsync-3.3.0.tar.gz
SHA256=7399e9a6708c32d678a72a63219e96f23be0be2336e50fd1348498d07041df90
DEPS="zstd lz4 xxhash openssl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-md2man --disable-debug --with-included-popt --with-included-zlib )
# makedev() takes three arguments on QNX (node, major, minor). configure finds
# that with a run test, which cannot execute when cross compiling and then
# defaults to "no"; give it the answer.
CONF_ENV=( rsync_cv_MAKEDEV_TAKES_3_ARGS=yes )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
