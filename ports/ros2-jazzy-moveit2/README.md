# ros2-jazzy-moveit2 20260128

> MoveIt 2 for ROS 2 Jazzy (headless core): motion planning (OMPL, Pilz, CHOMP, STOMP), kinematics, collision checking, move_group, Servo

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`ros2`](../ros2/) [`ros2-jazzy-control`](../ros2-jazzy-control/) [`ompl`](../ompl/) [`ruckig`](../ruckig/) [`fcl`](../fcl/) [`libccd`](../libccd/) [`octomap`](../octomap/) [`bullet3`](../bullet3/) [`osqp`](../osqp/) [`qhull`](../qhull/) [`assimp`](../assimp/) [`boost`](../boost/) [`eigen`](../eigen/) [`tinyxml2`](../tinyxml2/) [`fmt`](../fmt/) |
| Run-time dependencies | [`assimp`](../assimp/) [`boost`](../boost/) [`bullet3`](../bullet3/) [`console_bridge`](../console_bridge/) [`fastcdr`](../fastcdr/) [`fcl`](../fcl/) [`fmt`](../fmt/) [`octomap`](../octomap/) [`ompl`](../ompl/) [`openssl`](../openssl/) [`orocos-kdl`](../orocos-kdl/) [`osqp`](../osqp/) [`python3`](../python3/) [`qhull`](../qhull/) [`ruckig`](../ruckig/) [`tinyxml2`](../tinyxml2/) [`urdfdom`](../urdfdom/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ros2_jazzy_moveit2`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `ros2-jazzy-moveit2`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ros2-jazzy-moveit2`](https://openqnx.github.io/oss/package/x86_64/ros2-jazzy-moveit2.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ros2-jazzy-moveit2.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ros2-jazzy-moveit2` (dependencies come along); one version: `apk add ros2-jazzy-moveit2=<version>` | `apk del ros2-jazzy-moveit2` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ros2-jazzy-moveit2` (dependencies first) | `oss-get remove ros2-jazzy-moveit2` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-ros2-jazzy-moveit2 && /usr/local/bin/qnx-port-tests/test_ros2_jazzy_moveit2    # its on-target test, from tests-8.0-<arch>-m-z
apk del ros2-jazzy-moveit2 qnx-test-ros2-jazzy-moveit2
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ros2-jazzy-moveit2.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ros2-jazzy-moveit2.list
rm var/lib/manual/ros2-jazzy-moveit2.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ros2-jazzy-moveit2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (47 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ros2 ros2-jazzy-control ompl ruckig fcl libccd octomap bullet3 osqp qhull assimp boost eigen tinyxml2 fmt
```

<details><summary>Full build chain (47)</summary>

[`assimp`](../assimp/) [`benchmark`](../benchmark/) [`boost`](../boost/) [`brotli`](../brotli/) [`bullet3`](../bullet3/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`console_bridge`](../console_bridge/) [`curl`](../curl/) [`cyclonedds`](../cyclonedds/) [`eigen`](../eigen/) [`expat`](../expat/) [`fastcdr`](../fastcdr/) [`fastdds`](../fastdds/) [`fcl`](../fcl/) [`fmt`](../fmt/) [`foonathan_memory`](../foonathan_memory/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libccd`](../libccd/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libyaml`](../libyaml/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`numpy`](../numpy/) [`octomap`](../octomap/) [`ompl`](../ompl/) [`openblas`](../openblas/) [`opencv`](../opencv/) [`openssl`](../openssl/) [`osqp`](../osqp/) [`python3`](../python3/) [`qhull`](../qhull/) [`readline`](../readline/) [`ros2`](../ros2/) [`ros2-jazzy-control`](../ros2-jazzy-control/) [`ruckig`](../ruckig/) [`sqlite`](../sqlite/) [`tinyxml2`](../tinyxml2/) [`tl-expected`](../tl-expected/) [`xz`](../xz/) [`yaml-cpp`](../yaml-cpp/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`repo-patches/`](repo-patches/) — 8 one patch per repository of a multi-repository source (repo-patches/MAP)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (custom build system), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ros2-jazzy-moveit2; B=build/$QNX_ARCH/ros2-jazzy-moveit2; mkdir -p $B
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MoveIt 2 for ROS 2 Jazzy, a core that builds and runs headless: moveit_core (robot model and state, kinematics, collision detection with FCL and Bullet, planning scene, trajectory processing with Ruckig and TOTG), the planners (OMPL, Pilz industrial motion planner, CHOMP, STOMP), the KDL/SRV/cached IK kinematics plugins, moveit_ros planning, warehouse, move_group and its capabilities, MoveItCpp and the planning interface (MoveGroupInterface), MoveIt Servo, benchmarks, and the controller managers (simple, ros2_control) -- with geometric_shapes, srdfdom, random_numbers, eigen_stl_containers, moveit_msgs, octomap_msgs, object_recognition_msgs, warehouse_ros and the STOMP library. An overlay on the ros2 and ros2-jazzy-control components, installed into the ROS prefix (/usr/local/opt/ros/jazzy), built as ports/ros2-rviz is. Upstream sources at the tags in moveit2.repos -- qnx-ports/aports extra/ ros2-jazzy-moveit2's set and versions (VERSION is its date); QNX changes in repo-patches/ (MAP says where). Built against this tree's OMPL, Ruckig, FCL, libccd, octomap, Bullet, OSQP, qhull and assimp. Not built: the GUIs -- moveit_ros_visualization (RViz plugins), moveit_setup_assistant and its framework/plugins --, moveit_ros_robot_interaction (interactive markers, which come with ros2-rviz), moveit_ros_perception (OpenGL depth-image filter, octomap updater; QNX has no desktop GL) and moveit_py (Python bindings).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ros2-jazzy-moveit2`.

### 5. What it installs

4722 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/ros2-jazzy-moveit2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ros2-jazzy-moveit2
ls dists/8.0/*/ | grep '^ros2-jazzy-moveit2-'
```

This writes `ros2-jazzy-moveit2-<ver>-r0.tar.gz` (run time) and `ros2-jazzy-moveit2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ros2-jazzy-moveit2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ros2-jazzy-moveit2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ros2-jazzy-moveit2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ros2_jazzy_moveit2`](../../tests/src/) links ros2-jazzy-moveit2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ros2-jazzy-moveit2`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ros2-jazzy-moveit2
/usr/local/bin/qnx-port-tests/test_ros2_jazzy_moveit2
```

```
== ros2-jazzy-moveit2 ==
  ok   ...
PASS ros2-jazzy-moveit2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ros2_jazzy_moveit2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ros2_jazzy_moveit2`](../../tests/src/) -- a program linked against ros2-jazzy-moveit2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ros2-jazzy-moveit2 20260128-r1 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/ros2-jazzy-moveit2-20260128-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ros2-jazzy-moveit2 20260128-r1 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/x86_64/ros2-jazzy-moveit2-20260128-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20260128**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ros2-jazzy-moveit2=<version>`), sources in `ported/ros2-jazzy-moveit2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ros2-jazzy-moveit2`; port one with `scripts/port-update ros2-jazzy-moveit2`.

<details><summary>recipe.sh</summary>

```bash
VERSION=20260128
# MoveIt 2 for ROS 2 Jazzy, a core that builds and runs headless: moveit_core
# (robot model and state, kinematics, collision detection with FCL and
# Bullet, planning scene, trajectory processing with Ruckig and TOTG),
# the planners (OMPL, Pilz industrial motion planner, CHOMP, STOMP), the
# KDL/SRV/cached IK kinematics plugins, moveit_ros planning, warehouse,
# move_group and its capabilities, MoveItCpp and the planning interface
# (MoveGroupInterface), MoveIt Servo, benchmarks, and the controller
# managers (simple, ros2_control) -- with geometric_shapes, srdfdom,
# random_numbers, eigen_stl_containers, moveit_msgs, octomap_msgs,
# object_recognition_msgs, warehouse_ros and the STOMP library. An overlay
# on the ros2 and ros2-jazzy-control components, installed into the ROS
# prefix (/usr/local/opt/ros/jazzy), built as ports/ros2-rviz is.
#
# Upstream sources at the tags in moveit2.repos -- qnx-ports/aports extra/
# ros2-jazzy-moveit2's set and versions (VERSION is its date); QNX changes in
# repo-patches/ (MAP says where). Built against this tree's OMPL, Ruckig,
# FCL, libccd, octomap, Bullet, OSQP, qhull and assimp.
# Not built: the GUIs -- moveit_ros_visualization (RViz
# plugins), moveit_setup_assistant and its framework/plugins --,
# moveit_ros_robot_interaction (interactive markers, which come with
# ros2-rviz), moveit_ros_perception (OpenGL depth-image filter, octomap
# updater; QNX has no desktop GL) and moveit_py (Python bindings).
DEPS="ros2 ros2-jazzy-control ompl ruckig fcl libccd octomap bullet3 osqp qhull assimp boost eigen tinyxml2 fmt"
RUNTIME_DEPS="ros2 ros2-jazzy-control"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/ros2-jazzy-moveit2/moveit2.repos"
OVERLAY_PATCHES="$RECIPE_DIR/ros2-jazzy-moveit2/repo-patches"
OVERLAY_IGNORE="src/moveit/moveit2/moveit_ros/visualization src/moveit/moveit2/moveit_setup_assistant
src/moveit/moveit2/moveit_ros/robot_interaction src/moveit/moveit2/moveit_ros/perception src/moveit/moveit2/moveit_py
src/moveit/moveit2/moveit_ros/moveit_ros src/moveit/moveit2/moveit src/moveit/moveit2/moveit_runtime
src/moveit/moveit2/moveit_planners/moveit_planners src/moveit/moveit2/moveit_kinematics/test"
OVERLAY_CMAKE_ARGS=( -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" -DWITH_OPENGL=OFF -DBUILD_TESTING=OFF )
OVERLAY_PIP=""
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
