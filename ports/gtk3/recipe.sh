VERSION=3.24.43; GIT=https://gitlab.gnome.org/GNOME/gtk.git; REF=3.24.43
DEPS="glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  # All three backends.  Wayland is the target desktop's native one.  X11 was
  # off at first ("QNX has no X11 server"), but the X11 client stack is staged
  # now and Thunar 4.20 hard-requires gdk/gdkx.h (XDT_CHECK_LIBX11_REQUIRE), so
  # without it the Xfce file manager cannot build at all.  Broadway (GTK's HTML5
  # backend) needs no compositor, so it is the one way to drive real widgets on
  # a target that has none; broadwayd serves them over HTTP.
  -Dwayland_backend=true -Dx11_backend=true -Dbroadway_backend=true
  -Dprint_backends=file
  -Dintrospection=false -Dgtk_doc=false -Dman=false
  -Ddemos=false -Dexamples=false -Dtests=false -Dinstalled_tests=false
  -Dcolord=no -Dcloudproviders=false -Dtracker3=false -Dxinerama=yes )
