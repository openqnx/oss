# Avahi mDNS/DNS-SD: avahi-daemon, libavahi-common/-core/-client, the
# avahi-browse/-publish/-resolve tools and the libdns_sd compatibility
# library. 0.9-rc3 with qnx-ports' QNX support (patches/0001: SA_RESTART,
# <stdint.h> types for the howl headers, avahi-common/malloc.h no longer
# shadowing <malloc.h>), the version that fork carries; 0002 makes 0001 work
# in an out-of-tree build, 0003 lets "avahi-daemon --no-drop-root" run
# without an avahi account (a stock QNX system has none), 0004 stops the
# client tools aborting on SIGINT/SIGTERM (QNX's POLLIN is two bits).
VERSION=0.9-rc3; GIT=https://github.com/avahi/avahi.git; REF=v0.9-rc3
DEPS="dbus expat libdaemon"
BUILDSYS=autotools
# QNX's network stack is FreeBSD's: interfaces and addresses come from the
# PF_ROUTE socket (avahi-core/iface-pfroute.c; configure picks it from
# <net/route.h>, as on the BSDs). Netlink does not exist.
# Turned off: the GUI and language bindings (GTK, Qt, Python, Mono), the
# glib/libevent main-loop adapters, gdbm (the service-type name database
# avahi-browse uses for "human readable" names; the tools print the raw
# type), avahi-autoipd (IPv4LL through Linux packet sockets), systemd, and the
# man pages (xmltoman). --with-distro=none: no init scripts.
CONF_ARGS_EXTRA=(
  --enable-shared --disable-static --with-distro=none --disable-nls
  --disable-glib --disable-gobject --disable-libevent --disable-qt3 --disable-qt4 --disable-qt5
  --disable-gtk --disable-gtk3 --disable-python --disable-pygobject --disable-python-dbus
  --disable-mono --disable-monodoc --disable-gdbm --disable-dbm --disable-autoipd
  --disable-libsystemd --with-systemdsystemunitdir=no --disable-manpages --disable-xmltoman
  --disable-doxygen-doc --disable-tests --enable-compat-libdns_sd --disable-compat-howl
  --with-xml=expat --enable-dbus --enable-libdaemon
  # The system bus where the dbus port puts it (its system_bus_default_address).
  --with-dbus-system-address=unix:path=/var/local/run/dbus/system_bus_socket
  # A stock QNX target has no "avahi" account: run the daemon with
  # --no-drop-root (as root), or create the account.
  --with-avahi-user=avahi --with-avahi-group=avahi
)
