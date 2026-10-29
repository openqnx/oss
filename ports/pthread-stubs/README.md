# pthread-stubs 0.5

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz |
| SHA-256 | `59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pthread-stubs
oss-get install pthread-stubs-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pthread-stubs` performs exactly the same steps (2–5) in one command.

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
URL=https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz
mkdir -p dist src/pthread-stubs
curl -fL -o dist/$(basename $URL) $URL
echo "59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pthread-stubs --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/pthread-stubs && cd build/$QNX_ARCH/pthread-stubs
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/pthread-stubs/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The gitlab.freedesktop.org repo for this one is not publicly clonable (git asks for credentials); the release tarball is.
- Pure .pc file. It exists so that libxcb can say "I need pthreads" portably: on platforms where libc already provides them -- QNX among them -- the package resolves to no library at all, which is exactly the right answer here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pthread-stubs`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/pthread-stubs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pthread-stubs
ls repo/8.0/*/ | grep '^pthread-stubs-'
```

This writes `pthread-stubs-<ver>-r0.tar.gz` (run time) and `pthread-stubs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pthread-stubs
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pthread-stubs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pthread-stubs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.5
# The gitlab.freedesktop.org repo for this one is not publicly clonable
# (git asks for credentials); the release tarball is.
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz
SHA256=59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc
BUILDSYS=autotools
# Pure .pc file. It exists so that libxcb can say "I need pthreads" portably:
# on platforms where libc already provides them -- QNX among them -- the package
# resolves to no library at all, which is exactly the right answer here.
CONF_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
