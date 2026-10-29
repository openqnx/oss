VERSION=10.1.0; GIT=https://github.com/harfbuzz/harfbuzz.git; REF=10.1.0
DEPS="freetype glib icu graphite2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Ddocs=disabled -Dbenchmark=disabled
                   -Dutilities=disabled -Dintrospection=disabled
                   -Dfreetype=enabled -Dglib=enabled -Dicu=enabled
                   -Dgraphite2=enabled -Dcairo=disabled -Dchafa=disabled )
