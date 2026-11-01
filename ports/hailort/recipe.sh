# hailort: Hailo's runtime for the Hailo-8/8L AI accelerators -- libhailort
# (the C/C++ API: devices over PCIe or Ethernet, HEF network files, vstreams)
# and the hailortcli command -- with QNX's change (patches/0001, from
# github.com/qnx-ports/hailort, branch qnx-4.14). HailoRT has a QNX OS layer of
# its own (events on pevents, PCIe through QNX's libpci and the driver's
# /dev/hailo* devctl interface); the PCIe driver itself is not public and not
# part of this package.
#
# 4.14.0, the release QNX's port is based on: in later releases (4.18 on) the
# QNX branch of the driver interface (hailo_ioctl_common.h) no longer defines
# the ioctl groups the newer runtime uses (HAILO_NNC_IOCTL_MAGIC and others),
# so they do not build for QNX, and the device-side numbers would have to come
# from a matching QNX driver. The runtime, the driver and the device firmware
# go together: use this with QNX's 4.14 driver.
VERSION=4.14.0; GIT=https://github.com/hailo-ai/hailort.git; REF=v4.14.0
DEPS=""
BUILDSYS=cmake
# The build fetches its own dependencies at configure time, each pinned to a
# commit (hailort/pre_build/external/CMakeLists.txt: spdlog, CLI11 with Hailo's
# completion changes, nlohmann json, protobuf with a build-machine protoc for
# its generated code, readerwriterqueue, DotWriter, pevents, ...), and links
# them statically: libhailort carries the versions it was tested with and
# exports only its own API. No HailoRT service (multi-process sharing over
# gRPC), no Python bindings, no GStreamer plugin (none on QNX upstream).
CMAKE_ARGS_EXTRA=( -DHAILO_BUILD_PYBIND=OFF -DHAILO_BUILD_EXAMPLES=OFF -DHAILO_BUILD_SERVICE=OFF
                   -DHAILO_BUILD_GSTREAMER=OFF -DHAILO_BUILD_UT=OFF -DHAILO_BUILD_HW_DEBUG_TOOL=OFF
                   -DHAILO_COMPILE_WARNING_AS_ERROR=OFF )
