VERSION=0.23.3; GIT=https://github.com/rockdaboot/libpsl.git; REF=0.23.3
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libidn2 libunistring libiconv"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Druntime=libidn2 -Dbuiltin=true -Dtests=false -Ddocs=false )
