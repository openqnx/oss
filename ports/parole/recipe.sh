# Parole: the Xfce media player (GTK 3, GStreamer). Plays local files and
# network streams (HTTP, HLS/DASH through GStreamer) on the Wayland desktop.
# No DRM: Widevine-protected services (Netflix and the like) cannot play --
# their decryption module is closed and not available for QNX.
VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/apps/parole/4.20/parole-4.20.0.tar.xz
SHA256=5cf753e670d6518701133eb860d8bceb3a08a496af6a2b7cc67b93320230c983
DEPS="glib gtk3 dbus dbus-glib libxfce4util libxfce4ui xfconf gstreamer gst-plugins-base taglib libnotify"
RUNTIME_DEPS="dbus gst-plugins-good gst-plugins-bad gst-libav"
BUILDSYS=meson
# Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11).
# patches/0001: video through gtksink (gst-plugins-good) -- upstream Parole
# shows video only in an X11 overlay (ximagesink/xvimagesink), which a
# Wayland-only desktop does not have; without it Parole did not start.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled )
