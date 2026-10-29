# zsh 5.9

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz |
| SHA-256 | `9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install zsh
oss-get install zsh-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zsh` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses pcre2 libiconv
```

<details><summary>Full build chain (5)</summary>

[`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
mkdir -p dist src/zsh
curl -fL -o dist/$(basename $URL) $URL
echo "9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/zsh --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zsh < ports/zsh/patches/0001-aports-CODESET.patch
patch -p1 -d src/zsh < ports/zsh/patches/0002-aports-configure-shared.patch
```

**[`0001-aports-CODESET.patch`](patches/0001-aports-CODESET.patch)** — QNX change from QNX's aports (extra/zsh/CODESET.patch).

> Taken from qnx-ports/aports extra/zsh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `zsh-5.9/Src/Modules/langinfo.c` · Upstream status: QNX-specific

**[`0002-aports-configure-shared.patch`](patches/0002-aports-configure-shared.patch)** — QNX change from QNX's aports (extra/zsh/configure-shared.patch).

> Taken from qnx-ports/aports extra/zsh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `a/configure.ac` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/zsh && cd build/$QNX_ARCH/zsh
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/zsh/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-multibyte \
    --with-tcsetpgrp \
    --disable-gdbm
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Z shell. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zsh`.

### 5. What it installs

1279 files under `/usr/local`, 2 programs.

```
/usr/local/bin/zsh
/usr/local/bin/zsh-5.9
```

The complete list is `stage/<arch>/.manifest/zsh` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH zsh
ls repo/8.0/*/ | grep '^zsh-'
```

This writes `zsh-<ver>-r0.tar.gz` (run time) and `zsh-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install zsh
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/zsh-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/zsh-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# Z shell. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.9
TARBALL=https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
SHA256=9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses pcre2 libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-multibyte --with-tcsetpgrp --disable-gdbm )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
