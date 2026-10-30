# lcm: Lightweight Communications and Marshalling -- message types (lcm-gen) and UDP multicast publish/subscribe.
VERSION=1.5.2; GIT=https://github.com/lcm-proj/lcm.git; REF=v1.5.2
DEPS="glib"
BUILDSYS=cmake
# The C and C++ libraries and lcm-gen; no Python/Java/Lua/Go bindings.
CMAKE_ARGS_EXTRA=( -DLCM_ENABLE_PYTHON=OFF -DLCM_ENABLE_JAVA=OFF -DLCM_ENABLE_LUA=OFF -DLCM_ENABLE_GO=OFF -DLCM_ENABLE_TESTS=OFF -DLCM_ENABLE_EXAMPLES=OFF -DLCM_INSTALL_M4MACROS=OFF -DLCM_ENABLE_DOCS=OFF )
