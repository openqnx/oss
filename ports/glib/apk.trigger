#!/bin/sh
# apk trigger of glib (APK_TRIGGERS in recipe.sh): a package added, changed or
# removed files in a GSettings schema directory -- compile the schemas, or
# GSettings programs fail with "No GSettings schemas are installed".
#
# apk passes the changed directories as root-relative paths
# (/usr/local/share/glib-2.0/schemas) and runs this in the install root
# (/data/oss under apk-setup); its environment holds only PATH, without
# /proc/boot where a QNX image keeps rm, ls and the like. The wrapper
# runs apk with --force-no-chroot: the root has no /bin/sh or QNX libc to
# chroot into. Paths are taken relative to the current directory, so this also
# works for a root of "/".
r=$(pwd)
PATH=$PATH:/proc/boot:/system/bin:$r/usr/local/bin
L=$r/usr/local/lib
[ -d "$r/qnx-runtime/lib" ] && L=$L:$r/qnx-runtime/lib
dirs="$*"
for d in $dirs; do
  d=./${d#/}
  [ -d "$d" ] || continue
  set -- "$d"/*.gschema.xml
  if [ -e "$1" ]; then
    LD_LIBRARY_PATH=$L ./usr/local/bin/glib-compile-schemas "$d" || exit 1
  else
    rm -f "$d/gschemas.compiled"
  fi
done
exit 0
