# dnsmasq 2.93test4

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://thekelleys.org.uk/git/dnsmasq.git (tag `v2.93test4`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install dnsmasq
oss-get install dnsmasq-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dnsmasq` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.93test4 --recurse-submodules --shallow-submodules \
    https://thekelleys.org.uk/git/dnsmasq.git src/dnsmasq
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dnsmasq < ports/dnsmasq/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port dnsmasq for QNX 8.0

> Imported from qnx-ports/dnsmasq (e4d0bb45799a...qnx-v2.92); original authorship kept.
>
>
> ---
>  src/bpf.c     | 2 +-
>  src/dhcp.c    | 2 +-
>  src/dnsmasq.h | 4 ++--
>  3 files changed, 4 insertions(+), 4 deletions(-)

Touches: `src/bpf.c`, `src/dhcp.c`, `src/dnsmasq.h`, `src/bpf.c`, `src/config.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dnsmasq; B=build/$QNX_ARCH/dnsmasq; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && _dnsmasq_make; }
```

```bash
do_install() { _dnsmasq_make DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The same make variables for "install": its target depends on "all", and without CC there it rebuilt dnsmasq with the build host's cc -- both architectures shipped an x86-64 Linux executable (scripts/check-artifacts).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dnsmasq`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/dnsmasq
```

The complete list is `stage/<arch>/.manifest/dnsmasq` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH dnsmasq
ls repo/8.0/*/ | grep '^dnsmasq-'
```

This writes `dnsmasq-<ver>-r0.tar.gz` (run time) and `dnsmasq-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install dnsmasq
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/dnsmasq-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/dnsmasq-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.93test4; GIT=https://thekelleys.org.uk/git/dnsmasq.git; REF=v2.93test4
BUILDSYS=custom
# The same make variables for "install": its target depends on "all", and
# without CC there it rebuilt dnsmasq with the build host's cc -- both
# architectures shipped an x86-64 Linux executable (scripts/check-artifacts).
_dnsmasq_make() { make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE" LDFLAGS="-lsocket -lm" PREFIX="$PREFIX" "$@"; }
do_build() { cp -a "$S/." "$B/" && _dnsmasq_make; }
do_install() { _dnsmasq_make DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
