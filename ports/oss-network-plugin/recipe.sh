# oss-network-plugin: the network indicator of the desktop panel -- the place
# of Ubuntu's network icon at the top right. A "Network" plugin for the Xfce
# panel, written for this tree (files/, C and GTK 3): an icon for the state
# of the connection (connected; connected without a default route;
# disconnected; no network -- also when io-sock does not answer or there is
# no interface), a tooltip with the interface, its addresses, the gateway and
# the DNS servers, and on a click a window with every interface (state,
# hardware address, IPv4/IPv6, gateway, DHCP lease) and "Renew DHCP Lease"
# (dhcpcd -N, run in the background, its result on a line of the window;
# root sessions only, since dhcpcd's control socket is root's, and only on an
# interface the running dhcpcd holds a lease for). Not -n: that also makes
# dhcpcd reload its configuration -- from the default file -- and it then
# took on lo0, enc0, pflog0 and pfsync0 too (seen on the QEMU image).
# What it reads, without privileges (QNX's io-sock speaks the BSD interfaces):
# getifaddrs() for interfaces, addresses and link state (if_data), a routing
# socket for the default route (RTM_GET) and for changes -- it refreshes on
# every routing message, and every 10 s besides -- and dhcpcd -U for the
# lease (dhcpcd's unprivileged socket). bin/oss-network-info prints the same
# (--dump) and counts routing messages (--events N), for tests.
# Wi-Fi [unverified: no Wi-Fi device in QEMU; to check on a Raspberry Pi 5]:
# an IEEE 802.11 interface gets the wireless icons, and its network name and
# state come from `wpa_cli -i <if> status`. Choosing a network, static
# addresses and other connection management are not part of it.
VERSION=1.0
DEPS="gtk3 glib xfce4-panel"
RUNTIME_DEPS="xfce4-panel adwaita-icon-theme"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-network-plugin/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -shared -fPIC -DPACKAGE_LOCALE_DIR="\"$PREFIX/share/locale\"" \
      -o liboss-network.so "$S/oss-network-plugin.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 libxfce4panel-2.0) -lsocket && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-network-info "$S/oss-network-info.c" "$S/netinfo.c" \
      $(pkg-config --cflags --libs glib-2.0) -lsocket
}
do_install() {
  install -Dm755 "$B/liboss-network.so" "$STAGE$PREFIX/lib/xfce4/panel/plugins/liboss-network.so" && \
  install -Dm644 "$S/oss-network.desktop" "$STAGE$PREFIX/share/xfce4/panel/plugins/oss-network.desktop" && \
  install -Dm755 "$B/oss-network-info" "$STAGE$PREFIX/bin/oss-network-info"
}
