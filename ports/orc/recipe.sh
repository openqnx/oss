VERSION=0.4.44; GIT=https://gitlab.freedesktop.org/gstreamer/orc.git; REF=0.4.44
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dorc-test=disabled -Dbenchmarks=disabled -Dexamples=disabled
                   -Dtests=disabled -Dtools=disabled -Dhotdoc=disabled )
