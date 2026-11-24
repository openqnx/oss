VERSION=1.10.2; GIT=https://github.com/eclipse-zenoh/zenoh-pico.git; REF=1.10.2
# Eclipse Zenoh for constrained devices, in C. 1.10.2 is the release zenoh-c
# here is at (ports/zenoh-c), so the two speak the same protocol and zenoh-cpp
# can sit on either. patches/0001 is qnx-ports' QNX support (branch
# qnx-1.2.1, rebased onto 1.10.2's platform profiles): QNX as a unix
# platform; their raweth fix (0002) is upstream. 0003 replaces its random()
# calls with /dev/urandom.
# Examples, tools and the test programs are not installed; the shared library
# with the default feature set is (TCP and UDP unicast, UDP multicast and
# scouting; raw ethernet, serial and Bluetooth are off as upstream has them).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DBUILD_TOOLS=OFF
                   -DBUILD_INTEGRATION=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
