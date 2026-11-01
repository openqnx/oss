# fastfetch: system information at a glance (OS, kernel, CPU, memory,
# disks, uptime, shell, terminal, network addresses ...), as text or JSON.
# 2.66.1, the last release in C11: from 2.67.0 fastfetch requires C23
# (nullptr, enums with a fixed type, auto) and GCC 13, and QNX SDP 8.0's
# compiler is GCC 12.2. qnx-ports' fastfetch branch (qnx-2.68.1) carries no
# QNX changes; QNX support is patches/0001-0005 here: a QNX platform with
# its own detection code (system page, /proc, statvfs; 0001-0002), no
# d_type (0003), the FreeBSD network code QNX's io-sock shares (0004),
# QNX's min/max macros (0005), and two stub files fixed upstream-style
# (0006).
VERSION=2.66.1; GIT=https://github.com/fastfetch-cli/fastfetch.git; REF=2.66.1
# Built-in yyjson; no Lua format scripting, libzfs or zlib (image logos).
# Everything optional fastfetch would dlopen on Linux (X11, Wayland,
# Vulkan, OpenCL, D-Bus ...) is off for QNX by the build's own platform
# switches. LTO off: not tried with the QNX toolchain.
CMAKE_ARGS_EXTRA=( -DENABLE_SYSTEM_YYJSON=OFF -DENABLE_ZLIB=OFF -DENABLE_LUA=OFF -DENABLE_LIBZFS=OFF
                   -DENABLE_THREADS=ON -DBUILD_TESTS=OFF -DBUILD_FLASHFETCH=OFF -DENABLE_LTO=OFF )
