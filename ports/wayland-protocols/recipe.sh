VERSION=1.38
GIT=https://gitlab.freedesktop.org/wayland/wayland-protocols.git; REF=1.38
DEPS="wayland"
BUILDSYS=meson
# Data only: the XML protocol definitions that wayland-scanner turns into C in
# each consumer's own build. Installs the files plus wayland-protocols.pc.
MESON_ARGS_EXTRA=( -Dtests=false )
