# QNX OSS — open source for QNX SDP 8.0

Build files, QNX patches and step-by-step guides for **520 open source
components**, cross-built from their upstream source for **QNX SDP 8.0** on
**aarch64le** and **x86_64** — and the same components as prebuilt packages
a QNX target can download and install directly.

## QNX resources

| | |
|---|---|
| [QNX on GitHub](https://github.com/qnx/) | QNX's own repositories: BSPs (Raspberry Pi 4/5), samples, workshops |
| [QNX Everywhere](https://blackberry.qnx.com/en/products/qnx-everywhere) | the product page |
| [Get QNX](https://www.qnx.com/getqnx) | quick steps to the free non-commercial developer kit (QNX SDP 8.0) |
| [QNX Developer Blog](https://devblog.qnx.com/) | articles and how-tos |
| [qnx-ports on GitHub](https://github.com/qnx-ports) | QNX's open source ports |
| [QNX Build Files Repository](https://github.com/qnx-ports/build-files) | QNX's build files for those ports -- this repository started from it |
| [QNX Open-Source Dashboard](https://oss.qnx.com/) | build and test status of QNX's official ports; their apk repository is [repo.oss.qnx.com](https://repo.oss.qnx.com/) |

How this set compares with QNX's: [docs/upstream-comparison.md](docs/upstream-comparison.md).

> **Ported by Claude.** The ports, QNX patches, scripts, tests, packages and
> documentation here were ported, built, tested and documented by Claude
> (Anthropic's AI model, through Claude Code) and contributed to this
> repository. See [NOTICE.md](NOTICE.md).

| | |
|---|---|
| Components | **520** (520 build for every architecture they support -- both, but for a few aarch64-only ones) |
| 🟢 Standalone | **223** — QNX SDP only, at build time and at run time |
| 🟡 Build-deps only | **63** — need other components to build, run on their own |
| 🔴 Dependent | **234** — need other components to build and to run |
| QNX patches | **403**, one file per change, each with its reason |
| On-target tests | **230** test programs |
| Packages | runtime + `-dev` per component, `.tar.gz` and APK v2 |

**→ [Component index](PORTS-INDEX.md)** · **[Dependencies](DEPENDENCIES.md)** ·
**[Status by area](PORTS.md)**

## Three ways to use it

### 1. Install a prebuilt package on the target

With `apk`, from the GitHub releases (x86_64 and aarch64):

```sh
./scripts/target-bootstrap <target-ip>   # on the host, once per target: apk + CA bundle + key

# then on the target
. /data/oss/etc/oss-env.sh
apk add mosquitto                        # dependencies come along, signatures checked
apk upgrade
```

Or from your own build: `./scripts/serve-dists` serves `dists/` (everything
`scripts/qnxpkg` wrote) and `./scripts/target-bootstrap <target> -r
http://<host>:8000` points the target at it. Every component's on-target test
is a package too: `apk add qnx-test-<name>`.

Or with `oss-get`, a single shell script that needs nothing beyond a stock
image (`scp target/oss-get root@<target>:/data/`, then
`/data/oss-get install mosquitto`; the first `https://` download needs
`SSL_CERT_FILE`, as a stock image has no CA bundle).

→ [docs/packages.md](docs/packages.md)

### 2. Build one component from upstream, by hand

Every component has its own guide, `ports/<name>/README.md`, that takes you
from the upstream source to a binary on the target:

1. get the upstream source at the pinned version (git tag, or tarball + SHA-256)
2. apply the QNX patches in `ports/<name>/patches/` — each patch is separate
   and says why it is needed
3. configure — the exact command, every option explained
4. build and install into a staging tree
5. package it, copy it to the target, run its test

Start with a 🟢 **standalone** component: it needs nothing but the QNX SDP.

### 3. Build everything with the driver

```bash
export QNX_SDP_ROOT=$HOME/qnx800          # your SDP 8.0 installation
./scripts/build-host-tools.sh             # once: native generators the cross builds run
source ./env.sh aarch64le                 # or x86_64
./scripts/qnxbuild mosquitto              # fetch, patch, build deps first, install
./scripts/qnxpkg mosquitto                # -> dists/8.0/aarch64/mosquitto-*.tar.gz / .apk
```

Or in a container that needs only the SDP mounted — no host packages:

```bash
QNX_SDP_ROOT=$HOME/qnx800 docker/run.sh ./scripts/qnxbuild mosquitto
```

→ [docs/build-environment.md](docs/build-environment.md)

### All of it at once, on a vanilla QEMU image

```bash
vanilla-vm/run-all both               # x86_64 + aarch64 images, every component installed with apk and tested
vanilla-vm/run-all x86_64 --from github   # the same from the GitHub releases
vanilla-vm/desktop x86_64             # an image with graphics: labwc + Xfce panel, every application in its menu
```

→ [vanilla-vm/README.md](vanilla-vm/README.md)

## Standalone vs dependent

Every component is classified from its recipe **and from its binaries**
(`DT_NEEDED`), so the class says what really happens on the target:

| Class | To build | On the target | |
|---|---|---|---|
| 🟢 standalone | QNX SDP only | nothing else needed | proven by building against an *empty* staging tree (`scripts/verify-standalone`) |
| 🟡 build-deps only | other components first (headers, header-only or static libraries) | runs on its own | |
| 🔴 dependent | other components first | their shared libraries must be installed too | `oss-get` installs them |

The class is at the top of every component README, and
[DEPENDENCIES.md](DEPENDENCIES.md) lists each component's full build chain
and run-time closure.

## Requirements

* Linux build host (Ubuntu 22.04 / 24.04), or Docker
* QNX SDP 8.0, installed and licensed — including the eventfd/epoll/signalfd/
  timerfd shim libraries:
  `ls $QNX_SDP_ROOT/target/qnx/aarch64le/usr/lib/lib{eventfd,epoll,signalfd,timerfd}.so`
* A target: any QNX 8.0 board (verified on Raspberry Pi 5), or a QEMU image
  made with `mkqnximage` (verified on x86_64 and aarch64le)

## What is delivered

| Directory | | |
|---|---|---|
| `dists/8.0/<arch>/` | built packages, signed, every version kept | `apk add <name>` on a target, from this machine (`scripts/serve-dists`) or the GitHub releases |
| `ported/<name>/<version>/` | the original upstream source, the QNX patches (each with its reason), the patched source, and where the original comes from | build it yourself |
| `test_suites/8.0/<arch>/` | each component's on-target test, as a package (`qnx-test-<name>`) and as a plain program | `apk add qnx-test-<name>` |

→ [docs/target-guide.md](docs/target-guide.md): preparing a vanilla x86_64
(QEMU) or aarch64 (QEMU, Raspberry Pi 5) image, installing or building every
component -- by kind, standalone or dependent -- and testing it; each
component's README says how it was verified and where.

## Checking, updating, publishing

```bash
./scripts/verify-on-target -t "<target>" <target> <name>|--standalone|--dependent|--changed   # on a real target, recorded
./scripts/check-updates                       # newer upstream releases (also: Actions -> "Check upstream updates")
./scripts/port-update <name> [<version>]      # port one; the old version is kept
./scripts/publish-github-release --verified --upload [--changed]   # what passed, or only what is new
```

## Adding a component

`scripts/new-port <name> <git-url|tarball-url> [<tag>]` starts one (recipe,
test skeleton, first build).
[docs/porting-guide.md](docs/porting-guide.md) — recipe fields, the QNX
problems that come up again and again and how each was solved, turning an
edit into a patch with `scripts/mkpatch`, the standalone check, a test on the
target, and packaging.

## Documentation

| | |
|---|---|
| [Component index](PORTS-INDEX.md) | every component: version, upstream, patches, class, status |
| [Dependencies](DEPENDENCIES.md) | standalone / build-deps only / dependent, with full chains |
| [Status by area](PORTS.md) | components grouped by what they are for |
| [Build environment](docs/build-environment.md) | host or container setup, `env.sh`, isolated builds |
| [Target guide](docs/target-guide.md) | vanilla x86_64 / aarch64 / Raspberry Pi 5 targets: install, build, test every component |
| [Packages and oss-get](docs/packages.md) | prebuilt packages, the target installer, publishing |
| [Upstream updates](docs/UPDATES.md) | newer upstream releases than the ones ported (`scripts/check-updates`) |
| [Porting guide](docs/porting-guide.md) | how to port a new project |
| [Porting hazards](docs/porting-hazards.md) | ~100 QNX-specific traps and their fixes |
| [Upstream comparison](docs/upstream-comparison.md) | this set vs qnx-ports/build-files |
| [QNX references](docs/qnx-references.md) | QNX Everywhere, apk, the OSS dashboard |

## Layout

```
ports/<component>/   recipe.sh, patches/*.patch, README.md (generated)
ports/dependencies.json   classes, build chains, run-time closures
toolchain/           CMake toolchain files, Meson cross-file templates
scripts/qnxbuild     the build driver           scripts/qnxpkg    packager
scripts/mkpatch      edits -> patch file         scripts/verify-standalone
scripts/gen-*        regenerate READMEs, index, dependency classes
target/oss-get       installer for the QNX target    target/apk-setup  apk on the target
scripts/target-bootstrap  set a target up for apk  scripts/publish-github-release
scripts/serve-dists  serve dists/ + test_suites/ to targets
scripts/verify-on-target  install + test on a target, record   scripts/record-results
scripts/check-updates  newer upstream releases     scripts/port-update  port one, keep the old
scripts/new-port     start a new component         scripts/gen-ported  ported/ sources
tests/results/       what passed on which target, per version
vanilla-vm/          QEMU images: every port installed and tested (run-all), the desktop (desktop)
scripts/workspace-pack / workspace-setup   continue this workspace on another PC
docs/STATUS.md       where the work stands
keys/*.rsa.pub       package-signing public key (private half: keys/private/, untracked)
docker/              the build container
tests/               on-target test programs and runners
```

Generated trees (`src/ build/ stage/ pkgroot/ logs/ dist/ hosttools/` and the
deliverables `dists/ ported/ test_suites/`) are not tracked.

## Relationship to qnx-ports/build-files

Same layout as [qnx-ports/build-files](https://github.com/qnx-ports/build-files)
— one directory per component holding its build files, patches and README —
with a recipe driver instead of QNX recursive make, computed dependency
classes, and packages for the target. See the
[upstream comparison](docs/upstream-comparison.md).

## License

Each component keeps its own upstream license; the patches in
`ports/<name>/patches/` are offered under the license of the component they
modify. The build files, scripts, tests and documentation of this repository are
under the [Apache License 2.0](LICENSE). See also [NOTICE.md](NOTICE.md).
