VERSION=1.16.0; GIT=https://gitlab.freedesktop.org/dbus/dbus.git; REF=dbus-1.16.0
DEPS="expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Depoll=disabled -Dsystemd=disabled -Dselinux=disabled -Dapparmor=disabled
                   -Dx11_autolaunch=disabled -Ddoxygen_docs=disabled
                   -Dducktype_docs=disabled -Dxml_docs=disabled
                   -Dmodular_tests=disabled -Dtools=false -Dstats=false )
