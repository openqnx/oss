#!/bin/sh
# apk trigger of gdk-pixbuf (APK_TRIGGERS in recipe.sh): a package added,
# changed or removed a gdk-pixbuf loader module (librsvg's SVG loader, ...) --
# rewrite loaders.cache, or GdkPixbuf programs do not know the format
# ("Could not load a pixbuf from ...svg"). gdk-pixbuf's own loaders are built
# in; only the modules of other packages are listed.
#
# Runs in the install root (/data/oss under apk-setup) with only PATH set, as
# ports/glib/apk.trigger. The cache names the modules by their path on the
# target (/usr/local/...): they are queried under the root, and the root is
# taken off the paths written.
r=$(pwd)
PATH=$PATH:/proc/boot:/system/bin:$r/usr/local/bin
L=$r/usr/local/lib
[ -d "$r/qnx-runtime/lib" ] && L=$L:$r/qnx-runtime/lib
dirs="$*"
for d in $dirs; do
  d=./${d#/}
  c=$(dirname "$d")/loaders.cache
  # The last loader package removed: apk takes the empty loaders/ directory
  # away too, and the cache still naming its modules makes every GTK program
  # abort on an SVG icon ("Unable to load image-loading module").
  [ -d "$d" ] || { rm -f "$c"; continue; }
  set -- "$d"/*.so
  if [ -e "$1" ]; then
    LD_LIBRARY_PATH=$L ./usr/local/bin/gdk-pixbuf-query-loaders "$r/${d#./}"/*.so > "$c.new" &&
      sed "s|\"$r/|\"/|" "$c.new" > "$c" && rm -f "$c.new" || exit 1
  else
    rm -f "$c"
  fi
done
exit 0
