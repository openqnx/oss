VERSION=1.7.0; GIT=https://github.com/xkbcommon/libxkbcommon.git; REF=xkbcommon-1.7.0
DEPS="libxml2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable-docs=false -Denable-wayland=false -Denable-x11=false
                   -Denable-xkbregistry=true -Denable-tools=false -Denable-bash-completion=false )
