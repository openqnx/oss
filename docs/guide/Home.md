# openqnx/oss Wiki

How to try the packages of [openqnx/oss](https://github.com/openqnx/oss) yourself: open-source
components ported to **QNX SDP 8.0**, for **x86_64** and **aarch64le**, as signed apk packages and
plain `.tar.gz` archives in this repository's
[releases](https://github.com/openqnx/oss/releases).

You need a QNX SDP 8.0 installation (a [free non-commercial licence](https://www.qnx.com/getqnx) is
enough) on a Linux host. Everything else comes from this repository.

| Step | Page |
|---|---|
| 1. Make a base QEMU image (x86_64 and/or aarch64le) and boot it | [Base QEMU images](Base-QEMU-Images) |
| 2a. Install packages with **apk**: all of them, or only the ones you want | [Install with apk](Install-with-apk) |
| 2b. Install the plain **`.tar.gz` archives** (no apk): all at once, or one by one | [Install the archives](Install-the-archives) |
| 3. Optional: compile on the target itself (`cc`, `c++`, cmake) | [Compile on the target](Native-compiler) |
| When something does not work | [Troubleshooting](Troubleshooting) |

Use **one** of 2a and 2b per target: apk does not know about files unpacked by hand.

**Package site:** <https://openqnx.github.io/oss/>. It has a page per package with its versions,
files, dependencies, the dependency versions it works with, and the log of its test on QNX.

## Conventions on these pages

`host$` is a shell on the Linux host, `qnx#` a root shell on the QNX target. Commands use these
variables (set them as the pages show):

| Variable | Example | Meaning |
|---|---|---|
| `SDP_DIR` | `~/qnx800` | QNX SDP 8.0 install path |
| `SSH_KEY` | `~/.ssh/qnx-target` | SSH key for the target (public key `$SSH_KEY.pub`) |
| `TARGET_NAME` | `qnx-x86`, `qnx-a64` | the target's host name (different per VM) |
| `TARGET_IP` | `192.168.122.50` | the target's IP (`mkqnximage --getip`) |
| `SDP_ARCH` | `x86_64`, `aarch64le` | the SDP's architecture name (run-time library paths) |
| `ARCH` | `x86_64`, `aarch64` | the package architecture name (in release names) |
| `GH_RELEASES` | `https://github.com/openqnx/oss/releases/download` | GitHub releases address |

## How the releases are organised

GitHub allows 1000 files per release, so each kind is split by the first letter of the package name:

| Kind | names 0-9, a-l | names m-z |
|---|---|---|
| apk repository | `apk-8.0-<ARCH>` | `apk-8.0-<ARCH>-m-z` |
| `.tar.gz` + `INDEX` | `packages-8.0-<ARCH>` | `packages-8.0-<ARCH>-m-z` |
| on-target tests (`qnx-test-<name>`) | `tests-8.0-<ARCH>` | `tests-8.0-<ARCH>-m-z` (with the runner `qnx-port-tests`) |

Each component is a package `<name>` (programs, libraries, data) and usually `<name>-dev` (headers,
pkg-config/CMake files, static libraries). Every version stays published. One `INDEX` line is
`name|version|depends|file|sha256|size|description|conflicts`, newest version first.

*An independent community project, not affiliated with or endorsed by BlackBerry or QNX. QNX is a
trademark of BlackBerry Limited.*
