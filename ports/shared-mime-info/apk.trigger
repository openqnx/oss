#!/bin/sh
# apk trigger of shared-mime-info (APK_TRIGGERS in recipe.sh): a package added,
# changed or removed MIME definitions in <mime>/packages -- rebuild the MIME
# database there (the build does not: -Dupdate-mimedb=false), or programs
# that look up file types (gthumb, Thunar, GIO's content types) find none.
#
# apk passes the changed directories as root-relative paths
# (/usr/local/share/mime/packages) and runs this in the install root
# (/data/oss under apk-setup, whose apk wrapper passes --force-no-chroot);
# its environment holds only PATH, without /proc/boot. Paths are taken
# relative to the current directory, so this also works for a root of "/".
r=$(pwd)
PATH=$PATH:/proc/boot:/system/bin:$r/usr/local/bin
L=$r/usr/local/lib
[ -d "$r/qnx-runtime/lib" ] && L=$L:$r/qnx-runtime/lib
dirs="$*"
for d in $dirs; do
  d=./${d#/}
  [ -d "$d" ] || continue
  LD_LIBRARY_PATH=$L ./usr/local/bin/update-mime-database "${d%/packages}" || exit 1
done
exit 0
