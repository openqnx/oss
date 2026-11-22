# libdecor: client-side window decorations for Wayland clients whose compositor
# does not draw them (SDL, GLFW, imv ...): a library and its plugins.
VERSION=0.2.4; GIT=https://gitlab.freedesktop.org/libdecor/libdecor.git; REF=0.2.4
DEPS="wayland wayland-protocols cairo pango dbus gtk3 linux-headers-input-codes"
BUILDSYS=meson
# - both plugins: cairo (pango-drawn title bars) and GTK 3;
# - D-Bus for the desktop's cursor theme settings (falls back without a bus);
# - no demo programs.
MESON_ARGS_EXTRA=( -Ddemo=false -Dinstall_demo=false -Ddbus=enabled -Dgtk=enabled )
