# qnx-desktop-preferences: the services behind the settings -- Raspberry Pi OS'
# rpd-preferences. Its settings programs are Raspberry Pi's own and are not
# ported; the Xfce settings manager (xfce4-settings, in qnx-desktop-core) is
# what the user sees. Here: printing (CUPS, its filters, IPP over USB, paper
# sizes), mDNS/DNS-SD (avahi: network printers, <host>.local), time (chrony,
# the time zone database).
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   a network applet, a volume plugin (need io-sock / io-snd front ends: to be written)
#   an "Add / Remove Software" front end for apk (to be written)
#   system-config-printer or another printer set-up program
#   qnx-pi-config (raspi-config's counterpart: to be written)
VERSION=1.0
DEPS="cups cups-filters ippusbxd avahi chrony tzdata libpaper"
RUNTIME_DEPS="cups cups-filters ippusbxd avahi chrony tzdata libpaper"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-preferences/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/preferences.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/preferences.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/preferences.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/preferences.list"
}
