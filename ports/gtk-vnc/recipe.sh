# gtk-vnc: VNC client as a GTK 3 widget (libgtk-vnc, libgvnc) and gvncviewer,
# a VNC viewer window for the QNX desktop (labwc). Tight/JPEG, ZRLE, VeNCrypt
# TLS -- what TurboVNC servers speak (VirtualGL's vglrun in a TurboVNC session
# shows OpenGL programs such as RViz here).
VERSION=1.3.1; GIT=https://gitlab.gnome.org/GNOME/gtk-vnc.git; REF=v1.3.1
DEPS="glib gtk3 gdk-pixbuf cairo gnutls libgcrypt zlib libx11"
BUILDSYS=meson
# Coroutines on GLib threads: portable, no makecontext/swapcontext needed.
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dpulseaudio=disabled -Dsasl=disabled -Dwith-vala=disabled
                   -Dwith-coroutine=gthread )
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  install -Dm755 "$B/examples/gvncviewer" "$STAGE$PREFIX/bin/gvncviewer"
}
