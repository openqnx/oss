VERSION=1.54.0; GIT=https://gitlab.gnome.org/GNOME/pango.git; REF=1.54.0
# Also linked at run time (found by scripts/gen-dependency-index): libxft
DEPS="glib harfbuzz fribidi cairo fontconfig freetype libxft"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgtk_doc=false -Dintrospection=disabled -Dbuild-testsuite=false
                   -Dbuild-examples=false -Dcairo=enabled -Dfontconfig=enabled
                   -Dfreetype=enabled -Dlibthai=disabled -Dsysprof=disabled )
