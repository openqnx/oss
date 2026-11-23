VERSION=2.1.4; GIT=https://github.com/eclipse/mosquitto.git; REF=v2.1.4
DEPS="openssl cjson"
# 2.1: DOCUMENTATION became WITH_DOCS, and the GoogleTest unit tests are on
# unless WITH_TESTS=OFF (CMake runs them at build time to list their cases,
# which cross-built binaries cannot do). patches/0001: <arpa/inet.h> for
# ntohl() in mosquitto_db_dump.
CMAKE_ARGS_EXTRA=( -DWITH_DOCS=OFF -DWITH_TESTS=OFF -DWITH_SRV=OFF -DWITH_WEBSOCKETS=OFF -DWITH_STATIC_LIBRARIES=OFF
                   -DWITH_TLS=ON -DWITH_CJSON=ON -DWITH_PLUGINS=ON -DWITH_APPS=ON )
