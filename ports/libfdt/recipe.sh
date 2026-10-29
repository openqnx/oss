VERSION=1.7.2; GIT=https://github.com/dgibson/dtc.git; REF=v1.7.2
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtools=false -Dtests=false -Dpython=disabled -Dvalgrind=disabled
                   -Dyaml=disabled -Dstatic-build=false )
