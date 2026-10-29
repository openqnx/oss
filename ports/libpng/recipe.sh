VERSION=1.6.44; GIT=https://github.com/pnggroup/libpng.git; REF=v1.6.44
DEPS="zlib"
BUILDSYS=autotools
# libpng's CMake path generates pnglibconf by *running* a freshly built helper.
# That cannot work when cross-compiling -- the host's qemu binfmt handler picks
# up the QNX aarch64 binary and dies on /usr/lib/ldqnx-64.so.2. Its configure
# script handles cross builds properly and falls back to the prebuilt config.
CONF_ARGS_EXTRA=( --enable-shared --enable-static --disable-tools )
