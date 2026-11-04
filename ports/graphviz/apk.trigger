#!/bin/sh
# apk trigger of graphviz (APK_TRIGGERS in recipe.sh): a package added,
# changed or removed plugins in lib/graphviz -- rewrite the plugin
# configuration (config6) with `dot -c`, or dot knows no output formats
# ("Format: "png" not recognized"). Like glib's trigger: run in the install
# root with only PATH set, paths relative to the current directory.
r=$(pwd)
PATH=$PATH:/proc/boot:/system/bin:$r/usr/local/bin
L=$r/usr/local/lib
[ -d "$r/qnx-runtime/lib" ] && L=$L:$r/qnx-runtime/lib
[ -x ./usr/local/bin/dot ] || exit 0
# dot -c writes the configuration next to the plugins it finds; the
# directory it searches is compiled in (/usr/local/lib/graphviz): point it
# at this root's copy.
GVBINDIR=$r/usr/local/lib/graphviz LD_LIBRARY_PATH=$L ./usr/local/bin/dot -c || exit 1
exit 0
