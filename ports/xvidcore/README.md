# xvidcore 1.3.7

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz |
| SHA-256 | `abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xvidcore`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install xvidcore
oss-get install xvidcore-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xvidcore` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz
mkdir -p dist src/xvidcore
curl -fL -o dist/$(basename $URL) $URL
echo "abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xvidcore --strip-components=1
```

The project lives in the `build/generic/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xvidcore/build/generic; B=build/$QNX_ARCH/xvidcore; mkdir -p $B
```

```bash
do_configure() {
  cd "$S" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --disable-assembly
}
```

```bash
do_build()   { make -C "$S" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$S" install DESTDIR="$STAGE"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xvid ships its Makefile in the source directory and configure only writes platform.inc beside it, so there is nothing to build out-of-tree. Configure and build in place, the way its own bootstrap.sh expects.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xvidcore`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libxvidcore.so
/usr/local/lib/libxvidcore.so.4
/usr/local/lib/libxvidcore.so.4.3
```

The complete list is `stage/<arch>/.manifest/xvidcore` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH xvidcore
ls repo/8.0/*/ | grep '^xvidcore-'
```

This writes `xvidcore-<ver>-r0.tar.gz` (run time) and `xvidcore-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install xvidcore
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/xvidcore-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/xvidcore-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_xvidcore`](../../tests/src/) links xvidcore the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xvidcore root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_xvidcore'
```

```
== xvidcore ==
  ok   ...
PASS xvidcore
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.7
TARBALL=https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz
SHA256=abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d
SUBDIR=build/generic
BUILDSYS=custom
# xvid ships its Makefile in the source directory and configure only writes
# platform.inc beside it, so there is nothing to build out-of-tree. Configure and
# build in place, the way its own bootstrap.sh expects.
do_configure() {
  cd "$S" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --disable-assembly
}
do_build()   { make -C "$S" -j"$(nproc)"; }
do_install() { make -C "$S" install DESTDIR="$STAGE"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
