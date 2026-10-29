VERSION=0.25.5; GIT=https://github.com/p11-glue/p11-kit.git; REF=0.25.5
DEPS="libffi libtasn1"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtrust_module=disabled -Dsystemd=disabled -Dbash_completion=disabled
                   -Dgtk_doc=false -Dman=false -Dnls=false -Dtest=false
                   -Dlibffi=enabled )
