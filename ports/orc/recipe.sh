VERSION=0.4.40; GIT=https://gitlab.freedesktop.org/gstreamer/orc.git; REF=0.4.40
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dorc-test=disabled -Dbenchmarks=disabled -Dexamples=disabled
                   -Dtests=disabled -Dtools=disabled -Dgtk_doc=disabled )
