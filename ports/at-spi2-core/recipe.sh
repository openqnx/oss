VERSION=2.54.0
TARBALL=https://download.gnome.org/sources/at-spi2-core/2.54/at-spi2-core-2.54.0.tar.xz
SHA256=d7eee7e75beddcc272cedc2b60535600f3aae6e481589ebc667afc437c0a6079
DEPS="glib dbus libxml2 libx11 libxtst"
BUILDSYS=meson
# The accessibility bus, the ATK bridge GTK loads as a module -- and ATK
# itself.  Upstream merged the standalone atk module into at-spi2-core at
# 2.52, so 2.54 installs libatk-1.0 and atk.pc from its own atk/ subdirectory.
# There was a separate atk recipe here (2.38.0) and building this one silently
# overwrote its library: two libatk-1.0.so.0.* files ended up in staging and
# whichever package built last won.  That recipe is retired, this one is the
# ATK provider, and every DEPS entry that said "atk" now says "at-spi2-core".
#
# The overwrite was visible only on target: ATK 2.54 makes ATK_ROLE_PUSH_BUTTON
# an alias of ATK_ROLE_BUTTON, so atk_role_get_name() started answering
# "button" where 2.38 answered "push button", and test_atk failed.
# -Dintrospection=no: gobject-introspection has to *run* the target's own
# binaries to scan them, which a cross build cannot do.
# systemd_user_dir takes a path ("no" became a directory named no); QNX has no
# systemd, so the unit installed there is removed again after the install.
# X11 stays on -- libXtst is what AT-SPI uses to synthesise events, and the
# whole X11 client stack is already staged.
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Ddocs=false -Dsystemd_user_dir=$PREFIX/lib/systemd/user
                   -Datk_only=false -Dx11=enabled )
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
