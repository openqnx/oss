# xfce4-alsa-plugin: a volume control for the Xfce panel on ALSA -- scroll on
# the button to change the volume, click for a slider and mute, a settings
# dialog for the card and the mixer element. On QNX it runs on the SDP's
# libasound (an ALSA-compatible API; the sound comes from io-snd): no
# PulseAudio, which xfce4-pulseaudio-plugin needs. Written in Vala: valac
# (build machine, docker/Dockerfile) translates it to C.
#
# Its Vala bindings come with the release (src/vapi/: alsa, libxfce4panel-2.0
# -- depending only on gtk+-3.0 -- and config), so neither our xfce4-panel
# (built without introspection/Vala) nor the build machine has to provide
# them. patches/0001: libasound without alsa.pc on QNX.
VERSION=0.4.0
TARBALL=https://github.com/equeim/xfce4-alsa-plugin/archive/refs/tags/0.4.0.tar.gz
SHA256=16971c3c7d6324238db98c210da17755f27bc5baa080feae22f11005164ac3e2
DEPS="gtk3 xfce4-panel"
# libasound is part of the QNX image (not a package here); the panel loads the plugin.
RUNTIME_DEPS="xfce4-panel"
BUILDSYS=meson
