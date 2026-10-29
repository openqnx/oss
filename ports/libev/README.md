# libev 4.33

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | http://dist.schmorp.de/libev/libev-4.33.tar.gz |
| SHA-256 | `507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libev`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libev
oss-get install libev-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libev` performs exactly the same steps (2–5) in one command.

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
URL=http://dist.schmorp.de/libev/libev-4.33.tar.gz
mkdir -p dist src/libev
curl -fL -o dist/$(basename $URL) $URL
echo "507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libev --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libev; B=build/$QNX_ARCH/libev; mkdir -p $B
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/include/event.h"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Once the driver started linking QNX's split libc (libepoll, libeventfd, libsignalfd, libfsnotify) into every configure probe, libev's configure began finding epoll_ctl, eventfd, signalfd and inotify_init, where the earlier build had found none and used poll(). epoll/eventfd/signalfd stay on: ev_epoll.c uses level-triggered epoll only (no EPOLLET/EPOLLONESHOT), which is what QNX 8.0 provides. inotify is turned off: ev.c's inotify path needs <sys/statfs.h> and statfs() to tell local from remote filesystems, and QNX has only statvfs.  ev_stat watchers fall back to periodic stat(), which is libev's portable path.
- libev's event.h is its emulation of the libevent API, under the same name as libevent's real header (the libevent component). Do not install it; ev.h is libev's own API.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libev`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libev.so
/usr/local/lib/libev.so.4
```

The complete list is `stage/<arch>/.manifest/libev` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libev
ls repo/8.0/*/ | grep '^libev-'
```

This writes `libev-<ver>-r0.tar.gz` (run time) and `libev-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libev
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libev-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libev-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libev`](../../tests/src/) links libev the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libev root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libev'
```

```
== libev ==
  ok   ...
PASS libev
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.33; TARBALL=http://dist.schmorp.de/libev/libev-4.33.tar.gz
SHA256=507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea
BUILDSYS=autotools
# Once the driver started linking QNX's split libc (libepoll, libeventfd,
# libsignalfd, libfsnotify) into every configure probe, libev's configure began
# finding epoll_ctl, eventfd, signalfd and inotify_init, where the earlier build
# had found none and used poll().
#
# epoll/eventfd/signalfd stay on: ev_epoll.c uses level-triggered epoll only
# (no EPOLLET/EPOLLONESHOT), which is what QNX 8.0 provides.
# inotify is turned off: ev.c's inotify path needs <sys/statfs.h> and statfs()
# to tell local from remote filesystems, and QNX has only statvfs.  ev_stat
# watchers fall back to periodic stat(), which is libev's portable path.
CONF_ENV=( ac_cv_func_inotify_init=no )
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# libev's event.h is its emulation of the libevent API, under the same name as
# libevent's real header (the libevent component). Do not install it; ev.h is
# libev's own API.
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/include/event.h"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
