VERSION=1.8.1; GIT=https://github.com/dgibson/dtc.git; REF=v1.8.1
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtools=false -Dtests=false -Dpython=disabled -Dvalgrind=disabled
                   -Dyaml=disabled )
