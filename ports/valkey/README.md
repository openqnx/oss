# valkey 9.1.3

> High-performance key/value data store forked from Redis (server, client, benchmark)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/valkey-io/valkey.git (tag `9.1.3`) |
| Build system | custom (see recipe) |
| QNX patches | 7 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_valkey`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `valkey`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`valkey`](https://openqnx.github.io/oss/package/x86_64/valkey.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/valkey.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add valkey` (dependencies come along); one version: `apk add valkey=<version>` | `apk del valkey` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install valkey` (dependencies first) | `oss-get remove valkey` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-valkey && /usr/local/bin/qnx-port-tests/test_valkey    # its on-target test, from tests-8.0-<arch>-m-z
apk del valkey qnx-test-valkey
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/valkey.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/valkey.list
rm var/lib/manual/valkey.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild valkey` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 9.1.3 --recurse-submodules --shallow-submodules \
    https://github.com/valkey-io/valkey.git src/valkey
```

### 3. Apply the QNX patches

7 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/valkey < ports/valkey/patches/0001-qnx-xopen-source.patch
patch -p1 -d src/valkey < ports/valkey/patches/0002-malloc-stats-name.patch
patch -p1 -d src/valkey < ports/valkey/patches/0003-iov-max.patch
patch -p1 -d src/valkey < ports/valkey/patches/0004-qnx-epoll.patch
patch -p1 -d src/valkey < ports/valkey/patches/0005-qnx-timezone.patch
patch -p1 -d src/valkey < ports/valkey/patches/0006-qnx-bswap-macros.patch
patch -p1 -d src/valkey < ports/valkey/patches/0007-qnx-daemonize-gil.patch
```

**[`0001-qnx-xopen-source.patch`](patches/0001-qnx-xopen-source.patch)** — Define _XOPEN_SOURCE 700 on QNX.

> On anything but Linux, OpenBSD and NetBSD, fmacros.h defines _XOPEN_SOURCE
> with no value. QNX's <sys/platform.h> accepts only the X/Open versions it
> knows (500, 600, 700) and stops with "#error This version of XOPEN_SOURCE is
> not supported" in every file. QNX 8.0 implements POSIX.1-2008 / XPG7, the
> level Linux is given: define 700 for QNX as well (as ports/redis does).

Touches: `src/fmacros.h` · Upstream status: QNX-specific

**[`0002-malloc-stats-name.patch`](patches/0002-malloc-stats-name.patch)** — Rename struct malloc_stats, which QNX's <malloc.h> declares.

> QNX's <malloc.h> (reached through the system headers) declares its own
> `struct malloc_stats` for mallinfo-style statistics, and server.h's struct of
> the same name is then a redefinition. The struct is internal (one member of
> struct valkeyServer), so it is renamed.

Touches: `src/server.h` · Upstream status: QNX-specific

**[`0003-iov-max.patch`](patches/0003-iov-max.patch)** — Fall back to UIO_MAXIOV where IOV_MAX is not defined.

> The connection layers size their writev() batches with IOV_MAX, which QNX's
> <limits.h> undefines; <sys/uio.h> carries the same limit (1024) as
> UIO_MAXIOV (tmux and redis needed the same).

Touches: `src/connection.h` · Upstream status: QNX-specific

**[`0004-qnx-epoll.patch`](patches/0004-qnx-epoll.patch)** — Use the epoll event loop on QNX.

> Without a known polling API Valkey falls back to select(), and QNX's
> FD_SETSIZE is 256: ae_select.c refuses an event loop larger than that, and
> the server sizes its loop for maxclients (10000 by default) plus its own
> descriptors. QNX 8.0 provides epoll (libepoll) with level-triggered
> semantics, which is all ae_epoll.c uses (EPOLLIN/EPOLLOUT/EPOLLERR/EPOLLHUP,
> no EPOLLET or EPOLLONESHOT) -- the same choice as ports/redis. The recipe
> links -lepoll.

Touches: `src/config.h` · Upstream status: QNX-specific

**[`0005-qnx-timezone.patch`](patches/0005-qnx-timezone.patch)** — Take the timezone offset from the timezone variable on QNX.

> getTimeZone() reads the global timezone on Linux and Solaris and otherwise
> calls gettimeofday(NULL, &tz) for the struct timezone. QNX's gettimeofday()
> writes through its first argument unconditionally (hazard 145): the server
> would die with SIGSEGV in initServerConfig() on every start, as redis did.
> QNX has the POSIX/XSI timezone variable, set by the tzset() main() calls
> first; use it.

Touches: `src/util.c` · Upstream status: QNX-specific

**[`0006-qnx-bswap-macros.patch`](patches/0006-qnx-bswap-macros.patch)** — Undefine QNX's bswap16/32/64 macros before endianconv.h's functions.

> endianconv.h defines static inline bswap16/bswap32/bswap64 functions. QNX's
> <net/netbyte.h>, which the socket headers include, defines macros of the
> same names, so the function definitions expand into nonsense ("expected
> declaration specifiers"). Both do the same byte swap: on QNX, drop the
> macros before the functions are defined.

Touches: `src/endianconv.h` · Upstream status: QNX-specific

**[`0007-qnx-daemonize-gil.patch`](patches/0007-qnx-daemonize-gil.patch)** — Give a daemonized valkey-server a GIL its own thread owns.

> moduleInitModulesSystem() locks the modules' GIL early in main(), and the
> event loop unlocks it before every sleep and locks it again after. With
> --daemonize yes the server then fork()s, and the child goes on running the
> event loop. On QNX the child's thread does not own the mutex its parent
> locked: pthread_mutex_unlock() returns EPERM (hazard 127), and the next
> pthread_mutex_lock() in afterSleep() blocks for good -- a daemonized
> valkey-server accepted connections and never answered (seen with the static
> Lua module loaded, i.e. always). Re-initialise the GIL in the daemonized
> child and lock it again, which is the state the parent had.

Touches: `src/module.c`, `src/module.h`, `src/server.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/valkey; B=build/$QNX_ARCH/valkey; mkdir -p $B
```

```bash
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _valkey_make valkey-server valkey-sentinel valkey-cli valkey-benchmark valkey-check-rdb valkey-check-aof; }
```

```bash
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in valkey-server valkey-cli valkey-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in valkey-check-rdb valkey-check-aof valkey-sentinel; do ln -sf valkey-server "$bin/$p" || return 1; done
  install -Dm644 "$B/valkey.conf" "$STAGE$PREFIX/etc/valkey/valkey.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/valkey/sentinel.conf"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Valkey: the open source key/value data store forked from Redis 7.2 (valkey-server, valkey-sentinel) with its command-line client, benchmark and RDB/AOF checkers. TLS is built in, against the staged OpenSSL.
- Built like ports/redis (the same Makefile lineage). Notes on the variables: - uname_S=QNX: the Makefile asks the build machine's uname, which says Linux; QNX takes its generic "other OSes" branch. - CC is the cross-GCC driver, not qcc: the vendored dependencies compile with -MMD -MF, which qcc misreads (hazards 18/21). The bare driver lacks the __unix__/__unix that qcc predefines, and the vendored libvalkey picks its pthread_once() path by __unix__ (#error otherwise). - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's allocator is used instead (zmalloc keeps its own size header then). - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's ar/ranlib do not take part in for the static deps. - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38). - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the server runs Lua scripts and its RDB/AOF loaders on the main thread.
- Built in a copy of the tree (the Makefiles write objects next to the sources). The programs are named as targets: `all` also builds test modules that link with the build machine's bare `ld`.
- What `make install` does, without its dependency on `all`: the three programs, and the checkers and sentinel as links to valkey-server (it dispatches on argv[0]). The sample configurations go to etc/valkey.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH valkey`.

### 5. What it installs

8 files under `/usr/local`, 6 programs.

```
/usr/local/bin/valkey-benchmark
/usr/local/bin/valkey-check-aof
/usr/local/bin/valkey-check-rdb
/usr/local/bin/valkey-cli
/usr/local/bin/valkey-sentinel
/usr/local/bin/valkey-server
```

The complete list is `stage/<arch>/.manifest/valkey` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa valkey
ls dists/8.0/*/ | grep '^valkey-'
```

This writes `valkey-<ver>-r0.tar.gz` (run time) and `valkey-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add valkey
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/valkey-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/valkey-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_valkey`](../../tests/src/) links valkey the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-valkey`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-valkey
/usr/local/bin/qnx-port-tests/test_valkey
```

```
== valkey ==
  ok   ...
PASS valkey
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_valkey` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_valkey`](../../tests/src/) -- a program linked against valkey the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - valkey-server answers PING on 127.0.0.1
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
   - EVAL Lua script (server.call): (42+8)*2 = 100
   - Lua cjson library
   - PUBLISH reaches one subscriber
   - the subscriber receives the message
   - INFO server: valkey_version 9.1.3
   - valkey-cli SET prints OK
   - the key valkey-cli set is there
   - valkey-cli formats a list reply
   - valkey-benchmark SET/GET run (1000 requests)
   - SAVE
   - dump.rdb written
   - valkey-check-rdb: RDB looks OK
   - BGSAVE started (forked child)
   - ... and 9 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | valkey 9.1.3-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/valkey-9.1.3-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | valkey 9.1.3-r1 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/valkey-9.1.3-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **9.1.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add valkey=<version>`), sources in `ported/valkey/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates valkey`; port one with `scripts/port-update valkey`.

<details><summary>recipe.sh</summary>

```bash
# Valkey: the open source key/value data store forked from Redis 7.2
# (valkey-server, valkey-sentinel) with its command-line client, benchmark
# and RDB/AOF checkers. TLS is built in, against the staged OpenSSL.
VERSION=9.1.3; GIT=https://github.com/valkey-io/valkey.git; REF=9.1.3
DEPS="openssl"
BUILDSYS=custom
# Built like ports/redis (the same Makefile lineage). Notes on the variables:
#  - uname_S=QNX: the Makefile asks the build machine's uname, which says
#    Linux; QNX takes its generic "other OSes" branch.
#  - CC is the cross-GCC driver, not qcc: the vendored dependencies compile
#    with -MMD -MF, which qcc misreads (hazards 18/21). The bare driver lacks
#    the __unix__/__unix that qcc predefines, and the vendored libvalkey
#    picks its pthread_once() path by __unix__ (#error otherwise).
#  - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's
#    allocator is used instead (zmalloc keeps its own size header then).
#  - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's
#    ar/ranlib do not take part in for the static deps.
#  - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38).
#  - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the
#    server runs Lua scripts and its RDB/AOF loaders on the main thread.
_valkey_make() {
  make -C "$B/src" -j"$(nproc)" uname_S=QNX uname_M="$QNX_CPU" \
    CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE -D__unix__ -D__unix" \
    AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" \
    MALLOC=libc USE_JEMALLOC=no BUILD_TLS=yes USE_SYSTEMD=no OPTIMIZATION=-O2 \
    OPENSSL_PREFIX="$STAGE_SYSROOT$PREFIX" \
    SERVER_LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket -lepoll -Wl,-z,stack-size=8388608" "$@"
}
# Built in a copy of the tree (the Makefiles write objects next to the
# sources). The programs are named as targets: `all` also builds test modules
# that link with the build machine's bare `ld`.
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _valkey_make valkey-server valkey-sentinel valkey-cli valkey-benchmark valkey-check-rdb valkey-check-aof; }
# What `make install` does, without its dependency on `all`: the three
# programs, and the checkers and sentinel as links to valkey-server (it
# dispatches on argv[0]). The sample configurations go to etc/valkey.
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in valkey-server valkey-cli valkey-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in valkey-check-rdb valkey-check-aof valkey-sentinel; do ln -sf valkey-server "$bin/$p" || return 1; done
  install -Dm644 "$B/valkey.conf" "$STAGE$PREFIX/etc/valkey/valkey.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/valkey/sentinel.conf"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
