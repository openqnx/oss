# tmux 3.5a

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz |
| SHA-256 | `16216bd0877170dfcc64157085ba9013610b12b082548c7c9542cc0103198951` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`libevent`](../libevent/) [`ncurses`](../ncurses/) |
| Run-time dependencies | [`libevent`](../libevent/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install tmux
oss-get install tmux-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tmux` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libevent ncurses
```

<details><summary>Full build chain (3)</summary>

[`libevent`](../libevent/) [`ncurses`](../ncurses/) [`openssl`](../openssl/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz
mkdir -p dist src/tmux
curl -fL -o dist/$(basename $URL) $URL
echo "16216bd0877170dfcc64157085ba9013610b12b082548c7c9542cc0103198951  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tmux --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tmux < ports/tmux/patches/0001-qnx-sa-restart-iov-max.patch
```

**[`0001-qnx-sa-restart-iov-max.patch`](patches/0001-qnx-sa-restart-iov-max.patch)** — Provide SA_RESTART and IOV_MAX on QNX.

> - QNX's <signal.h> leaves SA_RESTART commented out ("not supported yet"), so
>   client.c and proc.c do not compile. tmux retries on EINTR already; asking
>   for no flag is the correct behaviour on QNX.
> - QNX's <limits.h> undefines IOV_MAX on purpose; the bundled imsg-buffer.c
>   sizes arrays with it. Use the kernel limit, UIO_MAXIOV from <sys/uio.h>.

Touches: `compat.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/tmux && cd build/$QNX_ARCH/tmux
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
LIBS="-lregex" \
$OLDPWD/src/tmux/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-utf8proc \
    --enable-static=no
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tmux calls regcomp/regexec, which QNX keeps in libregex rather than libc.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tmux`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/bin/tmux
```

The complete list is `stage/<arch>/.manifest/tmux` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH tmux
ls repo/8.0/*/ | grep '^tmux-'
```

This writes `tmux-<ver>-r0.tar.gz` (run time) and `tmux-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install tmux
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/tmux-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/tmux-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.5a
TARBALL=https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz
SHA256=16216bd0877170dfcc64157085ba9013610b12b082548c7c9542cc0103198951
DEPS="libevent ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-utf8proc --enable-static=no )
# tmux calls regcomp/regexec, which QNX keeps in libregex rather than libc.
CONF_ENV=( LIBS="-lregex" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
