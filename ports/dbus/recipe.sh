VERSION=1.16.0; GIT=https://gitlab.freedesktop.org/dbus/dbus.git; REF=dbus-1.16.0
DEPS="expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Depoll=disabled -Dsystemd=disabled -Dselinux=disabled -Dapparmor=disabled
                   -Dx11_autolaunch=disabled -Ddoxygen_docs=disabled -Dinotify=disabled -Dkqueue=disabled
                   -Dducktype_docs=disabled -Dxml_docs=disabled
                   -Dmodular_tests=disabled -Dtools=false -Dstats=false )
# -Dinotify=disabled: QNX's inotify is an emulation through /dev/fsnotify,
# and dbus-daemon's service-directory watcher blocked in it (state REPLY on
# the fsnotify server) while activating a service -- a deadlock: the
# activated service's authentication was never answered (hazard 136). The
# default watcher does nothing; the daemon reads the service directories at
# start and again when a requested service is not in its cache.
