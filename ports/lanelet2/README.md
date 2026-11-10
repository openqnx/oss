# lanelet2 1.2.3

> Lanelet2 HD map framework for automated driving: maps, OSM I/O, projection, traffic rules, routing

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`ros2`](../ros2/) [`boost`](../boost/) [`eigen`](../eigen/) [`geographiclib`](../geographiclib/) [`pugixml`](../pugixml/) |
| Run-time dependencies | [`boost`](../boost/) [`geographiclib`](../geographiclib/) [`pugixml`](../pugixml/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_lanelet2`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `lanelet2`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`lanelet2`](https://openqnx.github.io/oss/package/x86_64/lanelet2.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/lanelet2.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add lanelet2` (dependencies come along); one version: `apk add lanelet2=<version>` | `apk del lanelet2` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install lanelet2` (dependencies first) | `oss-get remove lanelet2` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-lanelet2 && /usr/local/bin/qnx-port-tests/test_lanelet2    # its on-target test, from tests-8.0-<arch>
apk del lanelet2 qnx-test-lanelet2
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/lanelet2.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/lanelet2.list
rm var/lib/manual/lanelet2.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lanelet2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (37 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ros2 boost eigen geographiclib pugixml
```

<details><summary>Full build chain (37)</summary>

[`benchmark`](../benchmark/) [`boost`](../boost/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`console_bridge`](../console_bridge/) [`curl`](../curl/) [`cyclonedds`](../cyclonedds/) [`eigen`](../eigen/) [`expat`](../expat/) [`fastcdr`](../fastcdr/) [`fastdds`](../fastdds/) [`foonathan_memory`](../foonathan_memory/) [`gdbm`](../gdbm/) [`geographiclib`](../geographiclib/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libyaml`](../libyaml/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`opencv`](../opencv/) [`openssl`](../openssl/) [`pugixml`](../pugixml/) [`python3`](../python3/) [`readline`](../readline/) [`ros2`](../ros2/) [`sqlite`](../sqlite/) [`tinyxml2`](../tinyxml2/) [`xz`](../xz/) [`yaml-cpp`](../yaml-cpp/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`repo-patches/`](repo-patches/) — 1 one patch per repository of a multi-repository source (repo-patches/MAP)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (custom build system), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lanelet2; B=build/$QNX_ARCH/lanelet2; mkdir -p $B
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Lanelet2: the map framework for automated driving (Autoware's HD map format) -- lanelets, areas, regulatory elements and their geometry (lanelet2_core), OSM map reading/writing (lanelet2_io), projections between lat/lon and the map frame (lanelet2_projection: UTM, local transverse Mercator with GeographicLib), traffic rules, routing graphs and shortest routes (lanelet2_routing), map validation and the example maps (lanelet2_maps). Built as ROS 2 packages with KIT-MRT's mrt_cmake_modules, as an overlay on the ros2 component, installed into its prefix (/usr/local/opt/ros/jazzy). Not built: lanelet2_python and lanelet2_examples (Boost.Python, which the boost component leaves out). QNX ships this as Autoware.Auto's lanelet2_vendor (gitlab.com/qnx), whose QNX patch (std::localeconv) this tree's toolchain does not need.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lanelet2`.

### 5. What it installs

537 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/lanelet2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lanelet2
ls dists/8.0/*/ | grep '^lanelet2-'
```

This writes `lanelet2-<ver>-r0.tar.gz` (run time) and `lanelet2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lanelet2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lanelet2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lanelet2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_lanelet2`](../../tests/src/) links lanelet2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-lanelet2`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-lanelet2
/usr/local/bin/qnx-port-tests/test_lanelet2
```

```
== lanelet2 ==
  ok   ...
PASS lanelet2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_lanelet2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_lanelet2`](../../tests/src/) -- a program linked against lanelet2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - write the map as OSM (lat/lon, UTM around 49 N 8 E)
   - German vehicle rules: the road lanelet is passable
   - one-way: no path from the last lanelet back to the first

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lanelet2 1.2.3-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/lanelet2-1.2.3-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lanelet2 1.2.3-r2 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/lanelet2-1.2.3-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lanelet2 1.2.3-r1 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/lanelet2-1.2.3-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lanelet2 1.2.3-r2 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/x86_64/lanelet2-1.2.3-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lanelet2=<version>`), sources in `ported/lanelet2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lanelet2`; port one with `scripts/port-update lanelet2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.2.3
# Lanelet2: the map framework for automated driving (Autoware's HD map
# format) -- lanelets, areas, regulatory elements and their geometry
# (lanelet2_core), OSM map reading/writing (lanelet2_io), projections
# between lat/lon and the map frame (lanelet2_projection: UTM, local
# transverse Mercator with GeographicLib), traffic rules, routing graphs and
# shortest routes (lanelet2_routing), map validation and the example maps
# (lanelet2_maps). Built as ROS 2 packages with KIT-MRT's mrt_cmake_modules,
# as an overlay on the ros2 component, installed into its prefix
# (/usr/local/opt/ros/jazzy). Not built: lanelet2_python and
# lanelet2_examples (Boost.Python, which the boost component leaves out).
# QNX ships this as Autoware.Auto's lanelet2_vendor (gitlab.com/qnx), whose
# QNX patch (std::localeconv) this tree's toolchain does not need.
DEPS="ros2 boost eigen geographiclib pugixml"
RUNTIME_DEPS="ros2"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/lanelet2/lanelet2.repos"
OVERLAY_PATCHES="$RECIPE_DIR/lanelet2/repo-patches"
OVERLAY_IGNORE="src/fzi-forschungszentrum-informatik/Lanelet2/lanelet2_python src/fzi-forschungszentrum-informatik/Lanelet2/lanelet2_examples"
OVERLAY_CMAKE_ARGS=( -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" -DMRT_ENABLE_COVERAGE=OFF )
OVERLAY_PIP=""
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
