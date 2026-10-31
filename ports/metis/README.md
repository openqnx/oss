# metis 5.2.1

> Programs and library for partitioning graphs and finite element meshes

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/KarypisLab/METIS.git (tag `v5.2.1`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | [`gklib`](../gklib/) |
| Run-time dependencies | [`gklib`](../gklib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_metis`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `metis` (programs, shared libraries, run-time data) and `metis-dev` (headers, pkg-config/CMake files, static libraries; needs `metis`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`metis`](https://openqnx.github.io/oss/package/x86_64/metis.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/metis.html)) · [`metis-dev`](https://openqnx.github.io/oss/package/x86_64/metis-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/metis-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add metis` (dependencies come along); one version: `apk add metis=<version>` | `apk del metis` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install metis` (dependencies first) | `oss-get remove metis` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`metis-dev` the same way, by its own name (`apk add metis-dev`, `oss-get install metis-dev`, by hand with its own file and list `metis-dev.list`); remove it before `metis`, which it needs.

```sh
# on the QNX target, as root
apk add metis metis-dev
apk add qnx-test-metis && /usr/local/bin/qnx-port-tests/test_metis    # its on-target test
apk del metis metis-dev qnx-test-metis
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/metis.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/metis.list
rm var/lib/manual/metis.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild metis` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gklib
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v5.2.1 --recurse-submodules --shallow-submodules \
    https://github.com/KarypisLab/METIS.git src/metis
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/metis < ports/metis/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/metis < ports/metis/patches/0002-qnx-libmetis-needs-gklib.patch
patch -p1 -d src/metis < ports/metis/patches/0003-qnx-small-filename-buffers.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port METIS minus tests.

> Imported from qnx-ports/METIS (f5ae915a84d3...qnx_v5.2.1); original authorship kept.
>
>
> ---
>  conf/gkbuild.cmake      | 2 +-
>  include/CMakeLists.txt  | 6 +++++-
>  programs/CMakeLists.txt | 2 +-
>  test/CMakeLists.txt     | 8 ++++++++
>  4 files changed, 15 insertions(+), 3 deletions(-)
>  create mode 100644 test/CMakeLists.txt

Touches: `conf/gkbuild.cmake`, `include/CMakeLists.txt`, `programs/CMakeLists.txt`, `test/CMakeLists.txt`, `programs/CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-libmetis-needs-gklib.patch`](patches/0002-qnx-libmetis-needs-gklib.patch)** — Link libmetis against GKlib.

> libmetis calls GKlib throughout (gk_malloc, gk_sigcatch's thread-local jump
> buffers, the random and sorting helpers) but its target links nothing, so
> libmetis.so has no DT_NEEDED for libGKlib.so. It links only because shared
> libraries are linked with --allow-shlib-undefined; an application that links
> just -lmetis, as METIS's documentation says to, then fails to load with
> undefined gk_* symbols, and only executables that list GKlib themselves work.

Touches: `libmetis/CMakeLists.txt` · Upstream status: QNX-specific

**[`0003-qnx-small-filename-buffers.patch`](patches/0003-qnx-small-filename-buffers.patch)** — Do not put a 1.28 MB file-name buffer on the stack.

> WritePartition() and WritePermutation() declare char filename[MAXLINE],
> where MAXLINE is 1280000. On Linux the 8 MB main-thread stack hides it;
> QNX's main thread stack is far smaller, so gpmetis and ndmetis died with
> SIGSEGV ("Memory fault") as soon as they wrote their result file, while
> gpmetis -nooutput worked. A 4 KB buffer with snprintf is ample for a file
> name.

Touches: `programs/io.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/metis -B build/$QNX_ARCH/metis -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_INSTALL_BINDIR=bin \
      -DGKLIB_PATH="$STAGE$PREFIX" \
      -DSHARED=ON
cmake --build build/$QNX_ARCH/metis -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/metis
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- METIS's CMake build expects "make config" to have run first: it writes build/xinclude/metis.h with the index and real widths (32-bit here, as upstream's default) and copies the header's CMakeLists next to it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH metis`.

### 5. What it installs

8 files under `/usr/local`: 1 shared-library entries, 6 programs, 1 headers.

```
/usr/local/lib/libmetis.so
/usr/local/bin/cmpfillin
/usr/local/bin/gpmetis
/usr/local/bin/graphchk
/usr/local/bin/m2gmetis
/usr/local/bin/mpmetis
/usr/local/bin/ndmetis
```

The complete list is `stage/<arch>/.manifest/metis` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa metis
ls dists/8.0/*/ | grep '^metis-'
```

This writes `metis-<ver>-r0.tar.gz` (run time) and `metis-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add metis
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/metis-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/metis-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_metis`](../../tests/src/) links metis the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-metis`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-metis
/usr/local/bin/qnx-port-tests/test_metis
```

```
== metis ==
  ok   ...
PASS metis
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_metis` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_metis`](../../tests/src/) -- a program linked against metis the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - graph built: 13 edges
   - METIS_PartGraphRecursive returns METIS_OK
   - bisection cuts exactly the one bridge edge
   - each clique stays in one part
   - METIS_PartGraphKway finds the same 1-edge cut
   - METIS_NodeND gives a permutation and its inverse
   - scratch directory
   - graphchk accepts the graph file
   - gpmetis reports Edgecut: 1
   - gpmetis wrote g.graph.part.2 splitting the cliques
   - ndmetis writes g.graph.iperm

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/metis-5.2.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/metis-5.2.1-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/metis-5.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | metis 5.2.1-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/metis-5.2.1-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add metis=<version>`), sources in `ported/metis/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates metis`; port one with `scripts/port-update metis`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2.1; GIT=https://github.com/KarypisLab/METIS.git; REF=v5.2.1
DEPS="gklib"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_INCLUDEDIR=include -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_BINDIR=bin -DGKLIB_PATH="$STAGE$PREFIX" -DSHARED=ON )
# METIS's CMake build expects "make config" to have run first: it writes
# build/xinclude/metis.h with the index and real widths (32-bit here, as
# upstream's default) and copies the header's CMakeLists next to it.
do_patch() {
  cd "$SRCROOT" && mkdir -p build/xinclude && \
  { echo "#define IDXTYPEWIDTH 32"; echo "#define REALTYPEWIDTH 32"; cat include/metis.h; } > build/xinclude/metis.h && \
  cp include/CMakeLists.txt build/xinclude/
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
