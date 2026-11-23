VERSION=1.56.4; GIT=https://gitlab.gnome.org/GNOME/pango.git; REF=1.56.4
# Also linked at run time (found by scripts/gen-dependency-index): libxft
DEPS="glib harfbuzz fribidi cairo fontconfig freetype libxft"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocumentation=false -Dintrospection=disabled -Dbuild-testsuite=false
                   -Dbuild-examples=false -Dcairo=enabled -Dfontconfig=enabled
                   -Dfreetype=enabled -Dlibthai=disabled -Dsysprof=disabled )
