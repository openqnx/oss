# pistache: a C++17 REST/HTTP server and client framework, with QNX's port
# (patches/0001, from github.com/qnx-ports/pistache: its event loop on
# libevent).
VERSION=0.4.26; GIT=https://github.com/pistacheio/pistache.git; REF=v0.4.26
DEPS="libevent rapidjson openssl"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -DPISTACHE_BUILD_TESTS=false -DPISTACHE_BUILD_EXAMPLES=false
                   -DPISTACHE_BUILD_DOCS=false -DPISTACHE_USE_SSL=true -DPISTACHE_USE_RAPIDJSON=true )
