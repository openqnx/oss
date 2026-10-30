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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tmux
apk add tmux-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tmux`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tmux
ls dists/8.0/*/ | grep '^tmux-'
```

This writes `tmux-<ver>-r0.tar.gz` (run time) and `tmux-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tmux
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tmux-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tmux-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tmux 3.5a-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tmux 3.5a-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **3.5a**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tmux=<version>`), sources in `ported/tmux/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tmux`; port one with `scripts/port-update tmux`.

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
