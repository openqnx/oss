# oss-software: "Add or Remove Software" for the desktop -- a GTK 3 front end
# to apk-tools (the place of Raspberry Pi OS' rp-prefapps / gui-pkinst):
# browse and search the repository, install, remove, check for updates and
# update everything, with a progress bar. Written for this tree (files/).
#
# apk runs as a child process (GSubprocess), never in the window's thread:
# the lists from `apk query --format json` (read with json-glib), the
# progress of an operation from --progress-fd, apk's own error lines in a
# dialog when it fails; while another apk holds the database lock it waits
# and says so. C rather than Python: GTK has no GObject introspection data
# in this tree (see GAP.md), which PyGObject would need.
# The desktop entry is named after the application id (org.openqnx.Software):
# that is the Wayland app_id the panel's window buttons look it up by.
VERSION=1.0
DEPS="gtk3 glib json-glib"
RUNTIME_DEPS="apk-tools adwaita-icon-theme"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-software/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-software "$S/oss-software.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 json-glib-1.0)
}
do_install() {
  install -Dm755 "$B/oss-software" "$STAGE$PREFIX/bin/oss-software" && \
  install -Dm644 "$S/org.openqnx.Software.desktop" \
                 "$STAGE$PREFIX/share/applications/org.openqnx.Software.desktop"
}
