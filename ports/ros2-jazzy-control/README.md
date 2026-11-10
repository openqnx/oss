# ros2-jazzy-control 20260128

> ros2_control and ros2_controllers for ROS 2 Jazzy: controller manager, hardware interfaces, standard controllers

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`ros2`](../ros2/) [`eigen`](../eigen/) [`boost`](../boost/) [`tinyxml2`](../tinyxml2/) [`fmt`](../fmt/) [`tl-expected`](../tl-expected/) |
| Run-time dependencies | [`console_bridge`](../console_bridge/) [`curl`](../curl/) [`fastcdr`](../fastcdr/) [`fmt`](../fmt/) [`orocos-kdl`](../orocos-kdl/) [`python3`](../python3/) [`tinyxml2`](../tinyxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ros2_jazzy_control`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `ros2-jazzy-control`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ros2-jazzy-control`](https://openqnx.github.io/oss/package/x86_64/ros2-jazzy-control.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ros2-jazzy-control.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ros2-jazzy-control` (dependencies come along); one version: `apk add ros2-jazzy-control=<version>` | `apk del ros2-jazzy-control` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ros2-jazzy-control` (dependencies first) | `oss-get remove ros2-jazzy-control` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-ros2-jazzy-control && /usr/local/bin/qnx-port-tests/test_ros2_jazzy_control    # its on-target test, from tests-8.0-<arch>-m-z
apk del ros2-jazzy-control qnx-test-ros2-jazzy-control
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ros2-jazzy-control.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ros2-jazzy-control.list
rm var/lib/manual/ros2-jazzy-control.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ros2-jazzy-control` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ros2 eigen boost tinyxml2 fmt tl-expected
```

<details><summary>Full build chain (37)</summary>

[`benchmark`](../benchmark/) [`boost`](../boost/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`console_bridge`](../console_bridge/) [`curl`](../curl/) [`cyclonedds`](../cyclonedds/) [`eigen`](../eigen/) [`expat`](../expat/) [`fastcdr`](../fastcdr/) [`fastdds`](../fastdds/) [`fmt`](../fmt/) [`foonathan_memory`](../foonathan_memory/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libyaml`](../libyaml/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`opencv`](../opencv/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`ros2`](../ros2/) [`sqlite`](../sqlite/) [`tinyxml2`](../tinyxml2/) [`tl-expected`](../tl-expected/) [`xz`](../xz/) [`yaml-cpp`](../yaml-cpp/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

More QNX changes, outside the main source tree:

- [`repo-patches/`](repo-patches/) — 3 one patch per repository of a multi-repository source (repo-patches/MAP)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (custom build system), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ros2-jazzy-control; B=build/$QNX_ARCH/ros2-jazzy-control; mkdir -p $B
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ros2_control for ROS 2 Jazzy -- the controller manager, hardware interface (with mock_components' GenericSystem), controller interface, joint limits, transmissions, ros2controlcli, and ros2_controllers (joint state / IMU / force-torque / range / pose broadcasters, forward command, joint trajectory, diff drive, tricycle, Ackermann/bicycle steering, mecanum, PID, admittance, gripper and GPIO controllers) with what they are built on: control_msgs, control_toolbox, realtime_tools, kinematics_interface (KDL), filters, diagnostics, angles, ackermann_msgs, xacro, pal_statistics, backward_ros, generate_parameter_library with RSL and cpp_polyfills, and topic_based_hardware_interfaces. An overlay workspace on the ros2 component, installed into its prefix (/usr/local/opt/ros/jazzy: source its setup.sh), built the way ports/ros2-rviz is. Upstream sources at the tags in control.repos -- the set and versions of qnx-ports/aports extra/ros2-jazzy-control (VERSION is its date); QNX changes in repo-patches/ (MAP says which repository each one is for). Not built: the GUIs (rqt_controller_manager, rqt_joint_trajectory_controller: rqt is not ported), kinematics_interface_pinocchio (Pinocchio is not ported), generate_parameter_library's example packages, and the ros2_control_demos (their launch files start RViz and Gazebo).
- ros2: the ROS libraries and setup.sh it runs on (in the ROS prefix, not /usr/local/lib, so NEEDED does not show them); xacro and ros2controlcli are Python (the controller spawner locks with filelock); generate_parameter_library's Python runs at build time only.
- STACK_DETAILS_AUTO_DETECT off: backward_ros otherwise found the binutils component's static libbfd.a in staging and linked it (with -liberty, which is not there); stack traces print addresses only, as without libdw/libbfd.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ros2-jazzy-control`.

### 5. What it installs

4756 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/ros2-jazzy-control` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ros2-jazzy-control
ls dists/8.0/*/ | grep '^ros2-jazzy-control-'
```

This writes `ros2-jazzy-control-<ver>-r0.tar.gz` (run time) and `ros2-jazzy-control-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ros2-jazzy-control
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ros2-jazzy-control-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ros2-jazzy-control-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ros2_jazzy_control`](../../tests/src/) links ros2-jazzy-control the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ros2-jazzy-control`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ros2-jazzy-control
/usr/local/bin/qnx-port-tests/test_ros2_jazzy_control
```

```
== ros2-jazzy-control ==
  ok   ...
PASS ros2-jazzy-control
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ros2_jazzy_control` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ros2_jazzy_control`](../../tests/src/) -- a program linked against ros2-jazzy-control the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ros2-jazzy-control 20260128-r2 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/ros2-jazzy-control-20260128-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ros2-jazzy-control 20260128-r2 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/ros2-jazzy-control-20260128-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20260128**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ros2-jazzy-control=<version>`), sources in `ported/ros2-jazzy-control/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ros2-jazzy-control`; port one with `scripts/port-update ros2-jazzy-control`.

<details><summary>recipe.sh</summary>

```bash
VERSION=20260128
# ros2_control for ROS 2 Jazzy -- the controller manager, hardware interface
# (with mock_components' GenericSystem), controller interface, joint limits,
# transmissions, ros2controlcli, and ros2_controllers (joint state / IMU /
# force-torque / range / pose broadcasters, forward command, joint trajectory,
# diff drive, tricycle, Ackermann/bicycle steering, mecanum, PID, admittance,
# gripper and GPIO controllers) with what they are built on: control_msgs,
# control_toolbox, realtime_tools, kinematics_interface (KDL), filters,
# diagnostics, angles, ackermann_msgs, xacro, pal_statistics, backward_ros,
# generate_parameter_library with RSL and cpp_polyfills, and
# topic_based_hardware_interfaces. An overlay workspace on the ros2
# component, installed into its prefix (/usr/local/opt/ros/jazzy: source its
# setup.sh), built the way ports/ros2-rviz is.
#
# Upstream sources at the tags in control.repos -- the set and versions of
# qnx-ports/aports extra/ros2-jazzy-control (VERSION is its date); QNX changes
# in repo-patches/ (MAP says which repository each one is for).
# Not built: the GUIs (rqt_controller_manager, rqt_joint_trajectory_controller:
# rqt is not ported), kinematics_interface_pinocchio (Pinocchio is not
# ported), generate_parameter_library's example packages, and the
# ros2_control_demos (their launch files start RViz and Gazebo).
DEPS="ros2 eigen boost tinyxml2 fmt tl-expected"
# ros2: the ROS libraries and setup.sh it runs on (in the ROS prefix, not
# /usr/local/lib, so NEEDED does not show them); xacro and ros2controlcli are
# Python (the controller spawner locks with filelock); generate_parameter_library's
# Python runs at build time only.
RUNTIME_DEPS="ros2 python3-pyyaml python3-lxml python3-filelock"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/ros2-jazzy-control/control.repos"
OVERLAY_PATCHES="$RECIPE_DIR/ros2-jazzy-control/repo-patches"
OVERLAY_IGNORE="src/ros-controls/ros2_control/rqt_controller_manager src/ros-controls/ros2_controllers/rqt_joint_trajectory_controller
src/ros-controls/kinematics_interface/kinematics_interface_pinocchio
src/PickNikRobotics/generate_parameter_library/example src/PickNikRobotics/generate_parameter_library/example_python
src/PickNikRobotics/generate_parameter_library/example_external"
# STACK_DETAILS_AUTO_DETECT off: backward_ros otherwise found the binutils
# component's static libbfd.a in staging and linked it (with -liberty, which
# is not there); stack traces print addresses only, as without libdw/libbfd.
OVERLAY_CMAKE_ARGS=( -DSTACK_DETAILS_AUTO_DETECT=OFF -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" )
OVERLAY_PIP=""
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
