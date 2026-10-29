# QNX-OSS — project overview

Reference for the QNX SDP 8.0 open source porting effort.

## openqnx/oss (2026-10)

This repository continues the 222-component set described below
(`qnx-oss-build-files` at `2b94b0a`) and adds:

* **48 more components** — industrial and IoT protocols (open62541,
  libmodbus, mosquitto, libcoap, ZeroMQ, nng), embedded web servers, libssh,
  libgit2, data formats, compression, C++ utilities, robotics (KDL, OctoMap,
  FCL), QuickJS, audio, and shell tools (bash, tmux, nano, less, rsync). Each
  was built for both architectures from an empty source tree, and tested on
  `mkqnximage` QEMU targets.
* **Dependency classes computed from the binaries** — standalone,
  build-deps only, dependent — with standalone builds proven against an
  empty staging tree (`scripts/verify-standalone`). See
  [DEPENDENCIES.md](../DEPENDENCIES.md).
* **Per-component install manifests** (`pkgroot/`, `stage/<arch>/.manifest/`),
  which make per-component packages possible.
* **Packages for the target** — `scripts/qnxpkg` writes runtime and `-dev`
  packages as tarballs and APK v2; `target/oss-get` installs them on a stock
  QNX 8.0 image. See [packages.md](packages.md).
* **A build container** (`docker/`), redirectable workspace directories, and
  pinned SHA-256 checksums for every tarball download.
* **Per-component guides** rewritten as upstream → patch → configure → build →
  package → deploy walkthroughs, and a [porting guide](porting-guide.md).
  `scripts/mkpatch` turns edits into patch files in the house format.

The rest of this document describes the inherited set and its history.

## The inherited set (as of 2026-10-04, `6af149e`)

## What this is

222 open source components cross-built from upstream source for **QNX SDP 8.0**
on **aarch64le** and **x86_64**. Nothing prebuilt from the SDP or from
`repo.oss.qnx.com` is reused, so every version is pinned here and every change
to upstream is a visible patch.

| | |
|---|---|
| Repository | [`github.com/openqnx/oss`](https://github.com/openqnx/oss) |
| Components | 222 — **220 build**, 2 structurally blocked |
| Patches | 70, across 51 components |
| Tests | 165 QNX binaries, one per component with runtime code |
| Verified | Clean build from an empty tree, 27 h; Raspberry Pi 5 **165/165** |
| Build host | Ubuntu 22.04 / 24.04, QNX SDP 8.0 |
| Targets | Raspberry Pi 5 (aarch64le), QEMU x86_64 / aarch64le images from `mkqnximage` |

The two that do not build are `weston` (needs DRM/KMS) and `zenoh-c` (no Rust
toolchain targets QNX 8). Both are documented, not open work items.

## Repository layout

```
README.md               project overview, prerequisites, how to build
PORTS-INDEX.md          all 222 components: version, upstream, patches, status
DEPENDENCIES.md         93 standalone vs 129 dependent, with full chains
ports/<component>/      recipe.sh, patches/, README.md  (one dir per component)
scripts/qnxbuild        the build driver
scripts/check-all       runs all five staging audits, both architectures
scripts/gen-*           regenerate the docs from the recipes
toolchain/              CMake toolchain files, Meson cross-file templates
tests/                  165 test sources, build and on-target runners
docs/                   guides, hazards, references, upstream comparison
```

Generated trees (`src/ build/ stage/ logs/ dist/ hosttools/`, ~30 GB) are not
tracked; they are reproducible from the recipes.

## Working with it

```bash
source ./env.sh aarch64le          # or x86_64
./scripts/qnxbuild <component>     # dependencies resolved recursively
./scripts/qnxbuild $(ls ports)     # everything, both architectures
./scripts/check-all                # five audits, both architectures
./tests/build-tests.sh aarch64le
./tests/deploy-and-run.sh aarch64le <target-ip>
```

Each `ports/<component>/README.md` is a standalone walkthrough: prerequisites,
download, patches, build, the exact files to copy to a board, and the one
command to run that component's test there. Those documents are generated from
the recipes, so they cannot drift — edit the recipe, then run
`./scripts/gen-port-docs`.

## Prerequisites that actually bite

1. **The SDP must be complete.** QNX splits `eventfd`, `epoll`, `signalfd` and
   `timerfd` out of libc into shim libraries, and the toolchain puts all four on
   every link line. An SDP without them links nothing — even zlib fails with
   `cannot find -leventfd`.
2. **CMake 4.x and Meson >= 1.3.** Distribution packages are often older. With
   CMake 3.31, x265 and libwebsockets fail, x265 with a libc++ C++03 message
   that points nowhere near the cause. Ubuntu 22.04's Meson 0.61 fails all 45
   Meson components.
3. **Target SSH access.** `tests/ssh/` is untracked (it holds a private key), so
   a fresh clone has none; the deploy scripts fall back to your own SSH setup.

## Quality gates

Five audits, each written after a real defect got past the previous ones:

| Audit | Catches |
|---|---|
| `check-undefined` | a library that links but cannot load on the target |
| `check-sdp-leak` | a binary quietly using an SDP prebuilt we also build |
| `check-dupes` | one library staged under two sonames |
| `check-devlinks` | a missing `.so` symlink, or a `.so` exporting nothing |
| `check-artifacts` | a non-QNX or wrong-architecture binary in the tree |

`check-test-coverage` separately reports any component no test exercises —
currently 2 of 222, both of which install no runtime code.

## History worth remembering

The port was developed on one machine (Ubuntu 24.04) and reached 220/222 with
all audits clean and 165/165 on hardware. Moving it to a second machine
(Ubuntu 22.04) in October 2026 revealed that **it had never actually been built
from nothing**: the original tree was always populated, and the stamp files hid
every ordering and fetch bug. Nine were found and fixed:

- dependency cycle `xz → gettext → libxml2 → xz`, which recursed until the
  shell segfaulted — gettext builds `--with-included-libxml` and never needed
  libxml2 at all
- the link sysroot cache ignored SDP changes, so newly installed libraries
  stayed invisible
- `dist/` was never created, so the first download failed as a network error
- a failed fetch left an empty source directory that made the next run skip
  downloading
- the native wayland-scanner cloned into the wayland recipe's own source path,
  which silently skipped that recipe's patch
- `pkg-config-native` was referenced by the Meson native file but created by
  nothing — it had only ever existed by hand
- jq's generated lexer/parser existed only as leftovers from an earlier build
- `GDBUS_CODEGEN` resolution depended on which pkg-config the host shipped
- `build-host-tools.sh` required a source the documented first step had not
  fetched yet

A clean build from an empty tree now completes in a single pass: 27 hours on an
8-core host, 220/222, no cascading failures, five audits clean, 165/165 on the
Pi.

## Known limits

| | Why |
|---|---|
| MIT-SHM paths in X11/cairo | QNX has no SysV shared memory at all |
| Mesa, Vulkan, GStreamer-GL | the only EGL belongs to Screen, with a different native display type |
| Weston | needs the DRM/KMS and input stack QNX does not expose |
| zenoh-c, Kuksa | no Rust toolchain targets QNX 8 |
| can-utils | SocketCAN is a Linux kernel interface |
| gobject-introspection | scans by executing target binaries on the build host |
| On-screen WebKitGTK | the engine builds and runs; the window backend needs Screen |

## Open items

- **x86_64 on-target testing.** The x86_64 binaries build and pass all audits,
  but have not been run on an x86_64 QNX target from the current build — the
  QEMU VMs lived on the retired machine. A QSTI-for-QEMU image would restore it.
- **Ubuntu 24.04 clean build.** Verified on 22.04; 24.04 should be easier
  (its Meson is new enough) but has not been run from nothing.
