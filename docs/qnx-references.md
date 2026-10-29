# QNX references

Where this workspace sits in the wider QNX ecosystem, and the documents worth
reading alongside it.

## QNX Everywhere

[QNX Everywhere](https://www.qnx.com/developers/docs/qnxeverywhere/index.html)
is the entry point: a free QNX SDP 8.0 licence for non-commercial use, plus
ready-made target images. Two guides sit under it — the **Self-Hosted Developer
Desktop Guide** and the **Custom Target Image Guide**.

### Target images

| Image | What it is |
|---|---|
| [QSTI for Raspberry Pi](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.target_images/topic/qsti/intro.html) | Quick Start Target Image: a pre-configured QNX OS image for **Raspberry Pi 4 and 5** (8 GB RAM or more recommended). Ships graphics samples — Gears (OpenGL ES 2.x), Maze, VKCube (Vulkan). Obtain the `.img` from your SDP installation (`com.qnx.qnx800.quickstart.rpi4` / `rpi5`) and write it to a 32 GB or larger micro SD card with Raspberry Pi Imager. |
| [QSTI for QEMU](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.target_images/topic/qsti_qemu/about.html) | The same quick-start image for QEMU, so a target is available without hardware. |
| [Custom Target Image (CTI)](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.target_images/topic/cti/about.html) | A build framework for rolling your own image, and a SOAFEE Blueprint. Supports Raspberry Pi 4/5 and QEMU, Linux hosts only (Ubuntu 22.04 or 24.04). Requires QNX Software Center, an SDP 8.0 licence, and the usual open-source build dependencies. Includes the Developer Desktop by default. |

**QSTI vs CTI:** QSTI is pre-configured and fixed; CTI is the same targets with
the image contents under your control.

### QNX Developer Desktop (QDD)

[QDD](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.qdd/topic/about.html)
lets you build applications **on QNX OS 8 directly, without cross-compiling**.
It runs **XFCE** and uses **apk** (Alpine's package keeper) as its package
manager. It is included in both the Quick Start and Custom Target images.

This matters for the present repository in one specific way: QDD's desktop is
XFCE, and the XFCE stack — GTK3, Xfce core libraries, Thunar, the panel, VTE —
is part of what is built here from source, so the two are directly comparable.

## QNX open source distribution

| Resource | What it is |
|---|---|
| [oss.qnx.com](https://oss.qnx.com/) | The Open-Source Dashboard: build and test results for all official ports |
| [repo.oss.qnx.com](https://repo.oss.qnx.com/) | The QNX apk package repository — prebuilt binary packages |
| [qnx-ports/build-files](https://github.com/qnx-ports/build-files/) | QNX-specific build files, patches and workflows for the official ports; `ports/` holds one directory per project. **This repository follows its layout** |
| [qnx-ports/aports](https://github.com/qnx-ports/aports) | Alpine-style package recipes (`APKBUILD`) |
| [qnx-ports/qsc-apk](https://github.com/qnx-ports/qsc-apk) | QNX Software Center packages as apk |
| [Porting to QNX — existing ports](https://www.qnx.com/developers/docs/qnxeverywhere/com.qnx.doc.qpg/topic/port_Existing_ports.html) | The official guidance, which points at `build-files` as the starting point |

The Alpine toolchain flow is: **aports** (recipes) → **abuild** (`abuild -r`
builds a package from an `APKBUILD`) → **apk** (installs it).

### How this repository relates to that flow

This workspace deliberately does **not** consume `repo.oss.qnx.com`. Every
component is compiled from the upstream project so that versions are pinned
here and patches are visible here. Building an apk from a component built this
way is a separate, additive step: write an `APKBUILD` that packages
`stage/<arch>/usr/local`, then `abuild -r`.

## CMake on QNX

- [CMake cross-compiling for QNX](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-qnx)
- This repository's toolchain files: [`toolchain/qnx-aarch64le.cmake`](../toolchain/qnx-aarch64le.cmake),
  [`toolchain/qnx-x86_64.cmake`](../toolchain/qnx-x86_64.cmake)

Note that CMake identifies `qcc` as compiler ID **`QCC`**, not `GNU` — projects
that branch on `CMAKE_C_COMPILER_ID MATCHES "GNU"` silently take their
unknown-compiler path. That and 96 other traps are in
[porting hazards](porting-hazards.md).
