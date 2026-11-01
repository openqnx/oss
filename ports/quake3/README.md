# quake3 1.36-git-20261012

> ioquake3, the maintained open-source Quake III Arena engine

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/ioquake/ioq3.git |
| Build system | CMake |
| QNX patches | 4 |
| Build dependencies | [`sdl2`](../sdl2/) |
| Run-time dependencies | [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_quake3`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `quake3`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`quake3`](https://openqnx.github.io/oss/package/x86_64/quake3.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/quake3.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add quake3` (dependencies come along); one version: `apk add quake3=<version>` | `apk del quake3` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install quake3` (dependencies first) | `oss-get remove quake3` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-quake3 && /usr/local/bin/qnx-port-tests/test_quake3    # its on-target test, from tests-8.0-<arch>-m-z
apk del quake3 qnx-test-quake3
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/quake3.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/quake3.list
rm var/lib/manual/quake3.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild quake3` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/ioquake/ioq3.git src/quake3
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/quake3 < ports/quake3/patches/0001-qnx-platform.patch
patch -p1 -d src/quake3 < ports/quake3/patches/0002-cmake-qcc-is-gcc.patch
patch -p1 -d src/quake3 < ports/quake3/patches/0003-cmake-default-basedir.patch
patch -p1 -d src/quake3 < ports/quake3/patches/0004-qnx-getifaddrs.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — Add QNX to q_platform.h.

> Without a platform block every file stops with "Operating system not
> supported" (and CMake's architecture probe fails first). QNX is a BSD-like
> Unix here: byte order from <sys/param.h>, .so modules, the x86_64 VM JIT;
> like Linux, aarch64 runs the VM interpreter.
> QNX's <sys/types.h> declares msg_t, a message-passing type, under
> _QNX_SOURCE; the engine's own msg_t (qcommon.h) is renamed by a macro on
> QNX so the two do not collide.

Touches: `code/qcommon/q_platform.h` · Upstream status: Pending

**[`0002-cmake-qcc-is-gcc.patch`](patches/0002-cmake-qcc-is-gcc.patch)** — Treat CMake's QCC compiler like GCC.

> CMake identifies QNX's qcc as "QCC", not "GNU", so gnu.cmake was skipped:
> no -fvisibility=hidden (every module then exports all its symbols, and the
> engine and game modules interpose each other's), no -fno-strict-aliasing
> (the code relies on union type punning), and no ftola/snapvector sources.

Touches: `cmake/compilers/gnu.cmake` · Upstream status: Pending

**[`0003-cmake-default-basedir.patch`](patches/0003-cmake-default-basedir.patch)** — Make DEFAULT_BASEDIR settable from CMake.

> sys_main.c takes fs_basepath, where game data and modules are, from
> DEFAULT_BASEDIR, or else the directory argv[0] names: started from PATH
> or through a link, that is not the install directory. A packaged install
> needs the fixed path; the cache variable passes it (empty keeps the
> binary's directory).

Touches: `cmake/platforms/unix.cmake` · Upstream status: Pending

**[`0004-qnx-getifaddrs.patch`](patches/0004-qnx-getifaddrs.patch)** — Find the local addresses with getifaddrs() on QNX too.

> NET_GetLocalAddress() uses getifaddrs() on Linux, macOS and the BSDs and
> otherwise resolves the machine's own host name with getaddrinfo(). On QNX
> that fallback went to DNS: a target whose name is not in /etc/hosts (a
> stock image) waited for the resolver's timeouts -- 30 s with no server
> running -- before the server opened its socket, and then knew only the
> addresses the name resolved to. QNX has getifaddrs() (libsocket), as on
> the BSDs its stack comes from.

Touches: `code/qcommon/net_ip.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/quake3; B=build/$QNX_ARCH/quake3; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  mkdir -p "$STAGE$PREFIX/bin"
  ln -sf ../opt/quake3/ioq3ded "$STAGE$PREFIX/bin/ioq3ded"
  ln -sf ../opt/quake3/ioquake3 "$STAGE$PREFIX/bin/ioquake3"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ioquake3: the maintained open-source Quake III Arena engine -- the dedicated server (ioq3ded), the client engine (ioquake3, on SDL2, with the GL1/GL2 renderers it loads at run time) and the native game modules (qagame, cgame, ui for baseq3 and missionpack). No game data: the retail/demo pak files are not free; the engine runs any pk3 it is given. ioquake3 makes no releases any more ("rolling" builds of master): a pinned commit of master.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH quake3`.

### 5. What it installs

12 files under `/usr/local`, 2 programs.

```
/usr/local/bin/ioq3ded
/usr/local/bin/ioquake3
```

The complete list is `stage/<arch>/.manifest/quake3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa quake3
ls dists/8.0/*/ | grep '^quake3-'
```

This writes `quake3-<ver>-r0.tar.gz` (run time) and `quake3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add quake3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/quake3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/quake3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_quake3`](../../tests/src/) links quake3 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-quake3`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-quake3
/usr/local/bin/qnx-port-tests/test_quake3
```

```
== quake3 ==
  ok   ...
PASS quake3
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_quake3` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_quake3`](../../tests/src/) -- a program linked against quake3 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ioq3ded and baseq3/qagame.so are installed
   - temporary directory
   - without game data the server exits by itself
   - and says pak0.pk3 is missing
   - the client engine (ioquake3) starts in dedicated mode and refuses missing data
   - wrote a pk3 with default.cfg and maps/qnx.bsp
   - linked the native game module into the game directory
   - getstatus is answered (the map is running)
   - status: mapname qnx
   - status: sv_hostname from the pk3's default.cfg
   - status: protocol 71, version string names QNX
   - status: the game module answers (gamename baseq3)
   - getinfo echoes the challenge, 0 clients, map qnx
   - rcon with a wrong password is refused
   - rcon with the password is accepted
   - rcon changed sv_hostname
   - server still running after the queries
   - the native game module was initialised
   - SIGTERM shuts the game down

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | quake3 1.36_git20261012-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/quake3-1.36_git20261012-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | quake3 1.36_git20261012-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/quake3-1.36_git20261012-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.36-git-20261012**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add quake3=<version>`), sources in `ported/quake3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates quake3`; port one with `scripts/port-update quake3`.

<details><summary>recipe.sh</summary>

```bash
# ioquake3: the maintained open-source Quake III Arena engine -- the
# dedicated server (ioq3ded), the client engine (ioquake3, on SDL2, with
# the GL1/GL2 renderers it loads at run time) and the native game modules
# (qagame, cgame, ui for baseq3 and missionpack). No game data: the
# retail/demo pak files are not free; the engine runs any pk3 it is given.
# ioquake3 makes no releases any more ("rolling" builds of master): a
# pinned commit of master.
VERSION=1.36-git-20261012; GIT=https://github.com/ioquake/ioq3.git; COMMIT=87a99d18206e878a20289f05ac3c16005dcae5c3
DEPS="sdl2"
CMAKE_ARGS_EXTRA=( -DBUILD_SERVER=ON -DBUILD_CLIENT=ON -DBUILD_GAME_LIBRARIES=ON
                   # QVM bytecode needs q3lcc/q3asm run on the build machine:
                   # the native modules do the same job.
                   -DBUILD_GAME_QVMS=OFF
                   -DUSE_RENDERER_DLOPEN=ON
                   # No OpenAL port (SDL audio is used), no HTTP downloads,
                   # no Mumble link (a Linux/Windows shared-memory protocol).
                   -DUSE_OPENAL=OFF -DUSE_HTTP=OFF -DUSE_MUMBLE=OFF
                   # zlib, libjpeg, ogg/vorbis/opus: the engine's bundled copies.
                   -DUSE_INTERNAL_LIBS=ON -DUSE_INTERNAL_SDL=OFF
                   # The engine's own layout (binaries, renderers and the
                   # baseq3/missionpack module directories side by side),
                   # in a prefix of its own as on Linux (/opt/quake3); the
                   # game data goes into its baseq3/. It looks for both
                   # next to its binary -- the directory argv[0] names, so
                   # not when started through the bin/ links -- and then in
                   # DEFAULT_BASEDIR, which is the install directory (patches/0003).
                   -DCMAKE_INSTALL_PREFIX="$PREFIX/opt/quake3"
                   -DDEFAULT_BASEDIR="$PREFIX/opt/quake3" )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  mkdir -p "$STAGE$PREFIX/bin"
  ln -sf ../opt/quake3/ioq3ded "$STAGE$PREFIX/bin/ioq3ded"
  ln -sf ../opt/quake3/ioquake3 "$STAGE$PREFIX/bin/ioquake3"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
