# qnx-desktop-graphics: codecs and rendering -- Raspberry Pi OS' rpd-graphics.
# GStreamer with the base, good, bad and ugly plugin sets and gst-libav (the
# FFmpeg decoders as GStreamer elements), the ffmpeg and ffprobe programs, and
# Mesa (under /usr/local/opt/mesa, never on the default library path: software
# OpenGL for when the board's own GL driver is not there; a program opts in).
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   librsvg               SVG rendering (also listed in qnx-desktop-core)
#   hardware video decoding elements for the board (none exist for QNX yet)
VERSION=1.0
DEPS="gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav ffmpeg mesa"
RUNTIME_DEPS="gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav ffmpeg mesa"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-graphics/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/graphics.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/graphics.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/graphics.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/graphics.list"
}
