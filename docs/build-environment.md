# Build environment

Everything here cross-builds on a Linux host for QNX SDP 8.0 targets
(`aarch64le`, `x86_64`). There are two ways to set the host up; both produce
the same result.

| | Host install | Container (`docker/`) |
|---|---|---|
| Needs on the host | Ubuntu 22.04/24.04 packages, pip | Docker only |
| QNX SDP | installed and licensed | installed and licensed — mounted read-only |
| Host tool versions | whatever you install | pinned in `docker/Dockerfile` |
| Good for | day-to-day porting | CI, reproducing a build, customers |

The SDP is licensed software and is never put into an image or into this
repository. Both ways expect `QNX_SDP_ROOT` to point at your installation.

## 0. Check the SDP

QNX splits eventfd, epoll, signalfd and timerfd out of libc into shim
libraries, and the toolchain files link all four into everything. An SDP
installed without them links nothing at all, not even zlib (`cannot find
-leventfd`):

```bash
export QNX_SDP_ROOT=$HOME/qnx800
ls $QNX_SDP_ROOT/target/qnx/aarch64le/usr/lib/lib{eventfd,epoll,signalfd,timerfd}.so
```

If any are missing, add the package in QNX Software Center.

## 1a. On the host

```bash
sudo apt install build-essential git curl pkg-config patch file xz-utils bzip2 \
     autoconf automake autopoint libtool gettext bison flex gperf texinfo nasm \
     python3 python3-pip python3-venv python3-jinja2 python3-mako python3-yaml
# Meson >= 1.3 and CMake 4.x: the distribution's are often too old
pip3 install --user --upgrade meson cmake ninja
```

## 1b. In the container

```bash
export QNX_SDP_ROOT=$HOME/qnx800
docker/run.sh                       # builds the image on first use, opens a shell
docker/run.sh ./scripts/qnxbuild zlib   # or run one command
```

`docker/run.sh` mounts:

| Host | Container | |
|---|---|---|
| this repository | `/work` | read-write (src/, build/, stage/ live here) |
| `$QNX_SDP_ROOT` | `/opt/qnx800` | read-only |
| `~/.qnx` | `~/.qnx` | read-only — the SDP license |

Nothing else from the host is visible, so a build cannot depend on what
happens to be installed on the build machine.

## 2. Native tools (once per workspace)

Cross builds have to *run* some generators on the build machine — gettext,
wayland-scanner, a CPython of the same version as the target's, nasm, gperf
and a few more. They are built from source into `hosttools/`:

```bash
./scripts/build-host-tools.sh       # ~15-40 min, fetches its own sources
```

Standalone components (see [DEPENDENCIES.md](../DEPENDENCIES.md)) that use
CMake or Autotools generally need none of them.

## 3. Enter the environment

```bash
source ./env.sh aarch64le           # or x86_64 -- once per shell, per architecture
```

`env.sh` sets:

| Variable | Meaning |
|---|---|
| `QNX_ARCH`, `QNX_TRIPLE`, `QCC_TARGET` | the target architecture and compiler target |
| `STAGE` | `stage/<arch>` — files install to `$STAGE/usr/local`, which is `/usr/local` on the target |
| `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_SYSROOT_DIR` | pkg-config sees only staged packages, never the host's |
| `QNX_MESON_DIR` | the generated Meson cross and native files |
| `QNX_TARGET` | a link sysroot in which staged libraries shadow the SDP's older copies of the same soname |

It also creates empty `librt.a`, `libdl.a`, `libpthread.a` (and a few more)
in staging: QNX folds those into libc, and portable build systems still ask
for them.

The CMake toolchain files are `toolchain/qnx-<arch>.cmake`; use them for your
own CMake projects too:

```bash
cmake -S myapp -B build/myapp -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake
```

## Building one component in isolation

Every directory the driver uses can be redirected, which is how
`scripts/verify-standalone` proves a component builds with nothing but the
SDP:

| Variable | Default | |
|---|---|---|
| `QNX_OSS_SRC_DIR` | `src/` | upstream source trees |
| `QNX_OSS_BUILD_DIR` | `build/` | build directories |
| `QNX_OSS_STAGE_DIR` | `stage/` | staging trees (one per arch) |
| `QNX_OSS_PKGROOT_DIR` | `pkgroot/` | per-component install trees (input to `qnxpkg`) |
| `QNX_OSS_LOG_DIR` | `logs/` | build logs |

```bash
# Build zlib against an empty staging tree
QNX_OSS_STAGE_DIR=/tmp/empty-stage QNX_OSS_BUILD_DIR=/tmp/b \
QNX_OSS_PKGROOT_DIR=/tmp/p ./scripts/qnxbuild -f -a aarch64le zlib
```

## Workspace layout

```
ports/<component>/   recipe.sh, patches/, README.md         (tracked)
toolchain/           CMake toolchain files, Meson templates (tracked)
scripts/             qnxbuild, qnxpkg, mkpatch, checks, doc generators
target/oss-get       the on-target package installer
docker/              the build container
tests/               on-target test programs and runners
src/ build/ stage/ pkgroot/ dists/ logs/ dist/ hosttools/   generated, not tracked (dists/: the packages)
```
