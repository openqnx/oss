VERSION=1.0.1; GIT=https://github.com/eclipse-uprotocol/up-cpp.git; REF=v1.0.1
# Eclipse uProtocol's C++ library: builders, serializers and validators for
# the uProtocol data model (UUri, UUID v8, UMessage) and the communication
# layer (Publisher/Subscriber, RPC client/server, notifications) over any
# UTransport. 1.0.1 is the release qnx-ports' branch qnx-v1.0.1 and the
# up-conan-recipes pair with up-core-api 1.6.0-alpha2. qnx-ports' QNX changes
# are to the unit tests (no -lpthread, gtest_add_tests when cross compiling),
# which are not built here; the library needs none.
# patches/0001: the CMake package (up-cpp::up-cpp) Conan would generate, a
# versioned shared library, tests only with BUILD_TESTING, and -lgcov only for
# the Coverage build.
DEPS="protobuf up-core-api"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
