# ippusbxd: IPP-over-USB daemon -- makes a USB printer that speaks IPP over
# USB (IPP Everywhere / AirPrint class) reachable as a network IPP printer on
# localhost, advertised through Avahi.
VERSION=1.34; GIT=https://github.com/OpenPrinting/ippusbxd.git; REF=1.34
SUBDIR=src
BUILDSYS=cmake
DEPS="libusb avahi libxml2 cups"
# The project has no install rule (its makefile only builds); install the
# daemon and its manual.
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man8" &&
  install -m 755 "$B/ippusbxd" "$STAGE$PREFIX/sbin/" &&
  install -m 644 "$SRCROOT/doc/ippusbxd.8" "$STAGE$PREFIX/share/man/man8/"
}
