# SOEM: Simple Open EtherCAT Master -- the EtherCAT master stack (slave
# enumeration and configuration, process data, CoE/SoE/FoE/EoE mailboxes,
# distributed clocks) and its sample tools (slaveinfo, eepromtool,
# firm_update, simple_ng, ec_sample).
VERSION=2.0.1; GIT=https://github.com/OpenEtherCATsociety/SOEM.git; REF=v2.0.1
# SOEM's OS layer comes per CMAKE_SYSTEM_NAME (cmake/<system>.cmake): Linux,
# Windows and rt-kernel. QNX gets its own (patch 0001): the POSIX osal of the
# Linux port unchanged, and an oshw whose NIC driver sends and receives the
# raw EtherCAT frames (ethertype 0x88a4) through BPF, by way of libpcap --
# QNX has no AF_PACKET sockets, which the Linux driver is built on. That is
# the approach SOEM's Windows port (WinPcap/Npcap) and its former macOS port
# took. Opening a NIC needs root (BPF), as raw sockets do on Linux.
DEPS="libpcap"
# A static library, as upstream builds it by default: the libpcap port
# installs only a static, non-PIC libpcap.a, which cannot go into a shared
# libsoem.so.
# eni_test runs the build machine's python on an ENI XML file at build time;
# it is a test program, not a tool, and is left out with eoe_test (Linux only
# upstream: it needs a TAP device).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=OFF -DSOEM_BUILD_SAMPLES=ON -DCMAKE_DISABLE_FIND_PACKAGE_Python3=ON )
