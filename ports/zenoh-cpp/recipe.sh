VERSION=1.2.1; GIT=https://github.com/eclipse-zenoh/zenoh-cpp.git; REF=1.2.1
# C++ bindings for Eclipse Zenoh: headers only, over either zenoh-c (the
# full Rust implementation, ports/zenoh-c) or zenoh-pico. 1.2.1 is the
# release both of those are at here. qnx-ports' branch qnx-1.2.1 changes only
# the tests' CMake (no zenohd router when cross compiling); the headers need
# nothing for QNX.
# Configured with both backends, so the installed zenohcxxConfig.cmake gives
# zenohcxx::zenohc and zenohcxx::zenohpico to whichever of zenohc /
# zenohpico a consumer found first. Tests, examples and docs are not built.
DEPS="zenoh-c zenoh-pico"
# Headers only: what a program built with them loads is the backend.
RUNTIME_DEPS="zenoh-c"
CMAKE_ARGS_EXTRA=( -DZENOHCXX_ZENOHC=ON -DZENOHCXX_ZENOHPICO=ON -DZENOHCXX_ENABLE_TESTS=OFF
                   -DZENOHCXX_ENABLE_EXAMPLES=OFF -DZENOHCXX_EXAMPLES_PROTOBUF=OFF )
# The zenoh-c and zenoh-pico submodules are not needed: the staged ones are used.
do_fetch() { git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT"; }
