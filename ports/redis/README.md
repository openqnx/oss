# redis 8.10.3

> Advanced key-value store (server and command-line client)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/redis/redis.git (tag `8.10.3`) |
| Build system | custom (see recipe) |
| QNX patches | 5 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_redis`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `redis`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`redis`](https://openqnx.github.io/oss/package/x86_64/redis.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/redis.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add redis` (dependencies come along); one version: `apk add redis=<version>` | `apk del redis` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install redis` (dependencies first) | `oss-get remove redis` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-redis && /usr/local/bin/qnx-port-tests/test_redis    # its on-target test, from tests-8.0-<arch>-m-z
apk del redis qnx-test-redis
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/redis.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/redis.list
rm var/lib/manual/redis.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild redis` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 8.10.3 --recurse-submodules --shallow-submodules \
    https://github.com/redis/redis.git src/redis
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/redis < ports/redis/patches/0001-qnx-xopen-source.patch
patch -p1 -d src/redis < ports/redis/patches/0002-malloc-stats-name.patch
patch -p1 -d src/redis < ports/redis/patches/0003-iov-max.patch
patch -p1 -d src/redis < ports/redis/patches/0004-qnx-epoll.patch
patch -p1 -d src/redis < ports/redis/patches/0005-qnx-timezone.patch
```

**[`0001-qnx-xopen-source.patch`](patches/0001-qnx-xopen-source.patch)** — Define _XOPEN_SOURCE 700 on QNX.

> On anything but Linux, OpenBSD and NetBSD, fmacros.h defines _XOPEN_SOURCE
> with no value. QNX's <sys/platform.h> accepts only the X/Open versions it
> knows (500, 600, 700) and stops with "#error This version of XOPEN_SOURCE is
> not supported" in every file. QNX 8.0 implements POSIX.1-2008 / XPG7, the
> level Linux is given: define 700 for QNX as well.

Touches: `src/fmacros.h` · Upstream status: QNX-specific

**[`0002-malloc-stats-name.patch`](patches/0002-malloc-stats-name.patch)** — Rename struct malloc_stats, which QNX's <malloc.h> declares.

> QNX's <malloc.h> (reached through the system headers) declares its own
> `struct malloc_stats` for mallinfo-style statistics, and server.h's struct of
> the same name is then a redefinition. The struct is Redis-internal (one
> member of struct redisServer), so it is renamed.

Touches: `src/server.h` · Upstream status: QNX-specific

**[`0003-iov-max.patch`](patches/0003-iov-max.patch)** — Fall back to UIO_MAXIOV where IOV_MAX is not defined.

> The connection layers size their writev() batches with IOV_MAX, which QNX's
> <limits.h> undefines; <sys/uio.h> carries the same limit (1024) as
> UIO_MAXIOV (hazard: tmux needed the same).

Touches: `src/connection.h` · Upstream status: QNX-specific

**[`0004-qnx-epoll.patch`](patches/0004-qnx-epoll.patch)** — Use the epoll event loop on QNX.

> Without a known polling API Redis falls back to select(), and QNX's
> FD_SETSIZE is 256: ae_select.c refuses an event loop larger than that, and
> the server sizes its loop for maxclients (10000 by default) plus its own
> descriptors. QNX 8.0 provides epoll (libepoll) with level-triggered
> semantics, which is all ae_epoll.c uses (EPOLLIN/EPOLLOUT/EPOLLERR/EPOLLHUP,
> no EPOLLET or EPOLLONESHOT) -- the same basis the libev and libzmq ports use.
> The recipe links -lepoll.

Touches: `src/config.h` · Upstream status: QNX-specific

**[`0005-qnx-timezone.patch`](patches/0005-qnx-timezone.patch)** — Take the timezone offset from the timezone variable on QNX.

> getTimeZone() reads the global timezone on Linux and Solaris and otherwise
> calls gettimeofday(NULL, &tz) for the struct timezone. QNX's
> gettimeofday() writes through its first argument unconditionally: every
> redis-server start (even --version) died with SIGSEGV in initServerConfig()
> (found from the core on the target). QNX has the POSIX/XSI timezone
> variable, set by the tzset() main() calls first; use it.

Touches: `src/util.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/redis; B=build/$QNX_ARCH/redis; mkdir -p $B
```

```bash
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _redis_make redis-server redis-sentinel redis-cli redis-benchmark redis-check-rdb redis-check-aof; }
```

```bash
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in redis-server redis-cli redis-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in redis-check-rdb redis-check-aof redis-sentinel; do ln -sf redis-server "$bin/$p" || return 1; done
  install -Dm644 "$B/redis.conf" "$STAGE$PREFIX/etc/redis/redis.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/redis/sentinel.conf"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Redis: in-memory data structure server (redis-server, redis-sentinel) and its command-line client, benchmark and RDB/AOF checkers. The core server only: the Redis 8 modules (JSON, search, time series, probabilistic) are separate repositories that scripts/build.sh builds when cloned under modules/, and they need Rust and other toolchains; they are not part of this port. TLS is built in, against the staged OpenSSL.
- The Makefile is used directly from src/ (the top-level one goes through scripts/build.sh and its module detection). Notes on the variables: - uname_S=QNX: the Makefile asks the build machine's uname, which says Linux; QNX takes its generic "other OSes" branch (-ldl -lrt -pthread are the empty compatibility archives, hazard 3). - CC is the cross-GCC driver, not qcc: xxhash's multiconf.make compiles with -MMD -MP -MF and qcc read that as a second input ("input file ... is the same as output file", hazard 18/21). - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's allocator is used instead (zmalloc keeps its own size header then). - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's ar/ranlib do not take part in for the static deps. - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38). - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the server runs Lua scripts and its RDB/AOF loaders on the main thread.
- Built in a copy of the tree: the Makefiles write their objects (and the dependency libraries) next to the sources. The programs are named as targets: `all` (and `install`, which depends on it) also builds the test-suite modules under tests/modules, which link with a bare `ld` -- the build machine's -- and fail on QNX objects. They are not installed anyway.
- What `make install` does, without its dependency on `all`: the three programs, and the checkers and sentinel as links to redis-server (it dispatches on argv[0]). The sample configurations go to etc/redis.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH redis`.

### 5. What it installs

8 files under `/usr/local`, 6 programs.

```
/usr/local/bin/redis-benchmark
/usr/local/bin/redis-check-aof
/usr/local/bin/redis-check-rdb
/usr/local/bin/redis-cli
/usr/local/bin/redis-sentinel
/usr/local/bin/redis-server
```

The complete list is `stage/<arch>/.manifest/redis` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa redis
ls dists/8.0/*/ | grep '^redis-'
```

This writes `redis-<ver>-r0.tar.gz` (run time) and `redis-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add redis
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/redis-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/redis-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_redis`](../../tests/src/) links redis the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-redis`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-redis
/usr/local/bin/qnx-port-tests/test_redis
```

```
== redis ==
  ok   ...
PASS redis
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_redis` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_redis`](../../tests/src/) -- a program linked against redis the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - redis-server answers PING on 127.0.0.1
   - SET/GET
   - binary-safe value (NUL, CR LF inside)
   - GET of a missing key: nil
   - INCR/INCRBY: 42
   - INCR on a non-integer: error reply
   - lists: RPUSH/LPUSH/LRANGE
   - hashes: HSET/HGET/HLEN
   - sorted set ordered by score
   - EXPIRE/TTL
   - a key with a 50 ms expiry is gone after 300 ms
   - MULTI/EXEC
   - EVAL Lua script: (42+8)*2 = 100
   - Lua cjson library
   - PUBLISH reaches one subscriber
   - the subscriber receives the message
   - INFO server: redis_version 8.10.3
   - redis-cli SET prints OK
   - the key redis-cli set is there
   - redis-cli formats a list reply
   - redis-benchmark SET/GET run (2000 requests)
   - SAVE
   - dump.rdb written
   - redis-check-rdb: RDB looks OK
   - BGSAVE started (forked child)
   - ... and 6 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | redis 8.10.3-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/redis-8.10.3-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | redis 8.10.3-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/redis-8.10.3-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.10.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add redis=<version>`), sources in `ported/redis/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates redis`; port one with `scripts/port-update redis`.

<details><summary>recipe.sh</summary>

```bash
# Redis: in-memory data structure server (redis-server, redis-sentinel) and
# its command-line client, benchmark and RDB/AOF checkers.
# The core server only: the Redis 8 modules (JSON, search, time series,
# probabilistic) are separate repositories that scripts/build.sh builds when
# cloned under modules/, and they need Rust and other toolchains; they are
# not part of this port. TLS is built in, against the staged OpenSSL.
VERSION=8.10.3; GIT=https://github.com/redis/redis.git; REF=8.10.3
DEPS="openssl"
BUILDSYS=custom
# The Makefile is used directly from src/ (the top-level one goes through
# scripts/build.sh and its module detection). Notes on the variables:
#  - uname_S=QNX: the Makefile asks the build machine's uname, which says
#    Linux; QNX takes its generic "other OSes" branch (-ldl -lrt -pthread are
#    the empty compatibility archives, hazard 3).
#  - CC is the cross-GCC driver, not qcc: xxhash's multiconf.make compiles
#    with -MMD -MP -MF and qcc read that as a second input ("input file ... is
#    the same as output file", hazard 18/21).
#  - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's
#    allocator is used instead (zmalloc keeps its own size header then).
#  - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's
#    ar/ranlib do not take part in for the static deps.
#  - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38).
#  - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the
#    server runs Lua scripts and its RDB/AOF loaders on the main thread.
_redis_make() {
  make -C "$B/src" -j"$(nproc)" uname_S=QNX uname_M="$QNX_CPU" \
    CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE" \
    AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" \
    MALLOC=libc BUILD_TLS=yes USE_SYSTEMD=no OPTIMIZATION=-O2 \
    OPENSSL_PREFIX="$STAGE_SYSROOT$PREFIX" \
    REDIS_LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket -lepoll -Wl,-z,stack-size=8388608" "$@"
}
# Built in a copy of the tree: the Makefiles write their objects (and the
# dependency libraries) next to the sources. The programs are named as
# targets: `all` (and `install`, which depends on it) also builds the
# test-suite modules under tests/modules, which link with a bare `ld` -- the
# build machine's -- and fail on QNX objects. They are not installed anyway.
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _redis_make redis-server redis-sentinel redis-cli redis-benchmark redis-check-rdb redis-check-aof; }
# What `make install` does, without its dependency on `all`: the three
# programs, and the checkers and sentinel as links to redis-server (it
# dispatches on argv[0]). The sample configurations go to etc/redis.
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in redis-server redis-cli redis-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in redis-check-rdb redis-check-aof redis-sentinel; do ln -sf redis-server "$bin/$p" || return 1; done
  install -Dm644 "$B/redis.conf" "$STAGE$PREFIX/etc/redis/redis.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/redis/sentinel.conf"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
