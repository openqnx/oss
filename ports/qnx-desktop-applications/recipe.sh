# qnx-desktop-applications: the main applications -- Raspberry Pi OS'
# rpd-applications (a browser, VLC, Geany, Thonny). Epiphany (WebKitGTK) is
# the browser, Parole (GStreamer) and VLC the media players, AbiWord the word
# processor and Gnumeric the spreadsheet (Raspberry Pi OS has LibreOffice in
# their place), and Geany (in qnx-desktop-utilities) gets its plugins here.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   thonny                needs Tk (X11 only: would run in Xwayland)
#   chromium / firefox    not planned (a porting project of its own)
VERSION=1.0
# vlc: to add back when 3.0.24 builds for QNX (its contrib set is being re-ported;
# the meta package cannot depend on a component that does not build).
DEPS="epiphany parole geany-plugins abiword gnumeric"
RUNTIME_DEPS="epiphany parole geany-plugins abiword gnumeric"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-applications/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/applications.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/applications.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/applications.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/applications.list"
}
