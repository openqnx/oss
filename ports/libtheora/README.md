# libtheora 1.1.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2 |
| SHA-256 | `b6ae1ee2fa3d42ac489287d3ec34c5885730b1296f0801ae577a35193d3affbc` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libogg`](../libogg/) [`libvorbis`](../libvorbis/) |
| Run-time dependencies | [`libogg`](../libogg/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libtheora
apk add libtheora-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libtheora`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtheora` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libogg libvorbis
```

### 2. Get the upstream source

```bash
URL=https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2
mkdir -p dist src/libtheora
curl -fL -o dist/$(basename $URL) $URL
echo "b6ae1ee2fa3d42ac489287d3ec34c5885730b1296f0801ae577a35193d3affbc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libtheora --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libtheora && cd build/$QNX_ARCH/libtheora
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libtheora/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-examples \
    --disable-spec \
    --disable-doc \
    --with-ogg="$STAGE$PREFIX" \
    --with-vorbis="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtheora`.

### 5. What it installs

18 files under `/usr/local`: 6 shared-library entries, 4 headers.

```
/usr/local/lib/libtheora.so
/usr/local/lib/libtheora.so.3
/usr/local/lib/libtheoradec.so
/usr/local/lib/libtheoradec.so.2
/usr/local/lib/libtheoraenc.so
/usr/local/lib/libtheoraenc.so.2
```

The complete list is `stage/<arch>/.manifest/libtheora` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtheora
ls dists/8.0/*/ | grep '^libtheora-'
```

This writes `libtheora-<ver>-r0.tar.gz` (run time) and `libtheora-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtheora
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtheora-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtheora-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtheora=<version>`), sources in `ported/libtheora/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtheora`; port one with `scripts/port-update libtheora`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.1.1
TARBALL=https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.bz2
SHA256=b6ae1ee2fa3d42ac489287d3ec34c5885730b1296f0801ae577a35193d3affbc
BUILDSYS=autotools
DEPS="libogg libvorbis"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-examples
                  --disable-spec --disable-doc
                  --with-ogg="$STAGE$PREFIX" --with-vorbis="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
