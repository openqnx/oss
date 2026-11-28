# qnx-desktop-utilities: the small programs of a desktop -- Raspberry Pi OS'
# rpd-utilities (mousepad, eom, galculator, xarchiver, lxtask). With what is
# ported: Mousepad as the text editor (Geany for programming), gThumb as the
# image viewer, MATE Calculator, the Xfce Task Manager (htop in a terminal too),
# the Xfce Screenshooter (screen, window or region; through wlr-screencopy),
# Xarchiver, 7-Zip and zip for archives, Evince for PDF, nano, "Software"
# (oss-software: add, remove and update programs with apk), and
# ffmpegthumbnailer for the video thumbnails Thunar shows through tumbler.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   xfce4-genmon-plugin xfce4-cpugraph-plugin   panel monitors
# Left out on purpose (ported; install by name when wanted):
#   qdirstat              brings Qt 6 (166 MB installed) for one program
#   pqiv btop             a second image viewer / a second task monitor
VERSION=1.0
DEPS="geany gthumb mate-calc htop ffmpegthumbnailer 7zip zip nano evince xdg-user-dirs xfce4-genmon-plugin xfce4-cpugraph-plugin xarchiver mousepad xfce4-taskmanager xfce4-screenshooter oss-software"
RUNTIME_DEPS="geany gthumb mate-calc htop ffmpegthumbnailer 7zip zip nano evince xdg-user-dirs xfce4-genmon-plugin xfce4-cpugraph-plugin xarchiver mousepad xfce4-taskmanager xfce4-screenshooter oss-software"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-utilities/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/utilities.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/utilities.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
}
