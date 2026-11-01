VERSION=1.0.0-git-20250319; GIT=https://github.com/eclipse-uprotocol/up-transport-zenoh-cpp.git; COMMIT=04678fc6035e9c88bb088b95960924a8f1987fd8
# uProtocol's UTransport over Eclipse Zenoh (zenoh-cpp on zenoh-c), for
# up-cpp. The last release, 1.0.0-rc3 (what qnx-ports' branch
# qnx-v1.0.0-rc3 ports), is written against zenoh-cpp 1.0.0-rc5; this commit
# moved it to zenoh 1.2.1, the version zenoh-c and zenoh-cpp are at here, and
# is the last one that builds against up-cpp 1.0.1 (the next changes
# UTransport::cleanupListener's signature, which is in up-cpp after 1.0.1).
# qnx-ports' QNX changes switch the backend to zenoh-pico; zenoh-c is ported
# here, so its full backend (configuration files, all transports) is used and
# they are not needed.
DEPS="zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
