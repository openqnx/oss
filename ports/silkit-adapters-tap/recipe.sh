# Vector SIL Kit Adapter for TAP devices: connects a host network interface
# of type TAP to a SIL Kit Ethernet network, so that the host's own network
# stack (ping, sockets) talks to simulated Ethernet participants. Upstream
# supports QNX: on QNX it opens io-sock's /dev/tapN (`ifconfig tap0
# create`), elsewhere Linux's /dev/net/tun. Built against the silkit
# component (SILKIT_PACKAGE_DIR: otherwise its CMake downloads a SIL Kit
# release); asio is the adapter's bundled copy (git submodule). Not built:
# the demos (a Linux shell-script ping demo and an ICMP echo device).
VERSION=1.2.0; GIT=https://github.com/vectorgrp/sil-kit-adapters-tap.git; REF=v1.2.0
DEPS="silkit"
CMAKE_ARGS_EXTRA=( -DSILKIT_PACKAGE_DIR="$STAGE$PREFIX" -DBUILD_LINUX_PACKAGE=OFF )
