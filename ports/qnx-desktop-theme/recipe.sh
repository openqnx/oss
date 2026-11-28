# qnx-desktop-theme: what the desktop looks like -- Raspberry Pi OS' rpd-theme.
# The icon themes the session and its programs name (Adwaita, hicolor as the
# fallback every theme inherits, nuoveXT2 from lxde-icon-theme) and the fonts:
# Adwaita Sans/Mono, DejaVu, Liberation (metric-compatible with Arial, Times
# New Roman, Courier New), Cantarell, Inter, Hack and Inconsolata for the
# terminal and the editors, Noto Sans CJK (Korean, Japanese, Chinese) and
# Noto Emoji; fontconfig finds them.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   a GTK theme and icon set of our own (PiXtrix is Raspberry Pi's: licence and
#     trademark to be checked), Nunito Sans, a wallpaper set
# Left out on purpose (ported; install by name when wanted):
#   font-noto-serif-cjk   183 MB installed; Noto Sans CJK covers the same scripts
VERSION=1.0
DEPS="adwaita-icon-theme hicolor-icon-theme lxde-icon-theme adwaita-fonts font-dejavu font-liberation font-cantarell font-inter font-hack font-inconsolata font-noto-sans-cjk font-noto-emoji fontconfig"
RUNTIME_DEPS="adwaita-icon-theme hicolor-icon-theme lxde-icon-theme adwaita-fonts font-dejavu font-liberation font-cantarell font-inter font-hack font-inconsolata font-noto-sans-cjk font-noto-emoji fontconfig"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-theme/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/theme.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/theme.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/theme.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/theme.list"
}
