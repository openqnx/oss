#!/bin/sh
# apk trigger of python3 (APK_TRIGGERS in recipe.sh): a package added, changed
# or removed files in site-packages -- drop the byte code it leaves behind.
#
# The packages carry no .pyc: python3 writes __pycache__/*.pyc on the first
# import, and apk does not own those files, so "apk del" leaves them and the
# directories holding them. Python then imports such a directory as an empty
# namespace package: a removed numpy still imports, without its contents
# (porting-hazards: "apk del leaves __pycache__").
#
# This removes every .pyc whose .py is gone, and the directories that become
# empty through that; directories that were empty before are left alone (a
# package may own one). It runs in the install root with only PATH set (see
# ports/glib/apk.trigger), so it uses the python3 of this package.
r=$(pwd)
L=$r/usr/local/lib
[ -d "$r/qnx-runtime/lib" ] && L=$L:$r/qnx-runtime/lib
[ -x ./usr/local/bin/python3 ] || exit 0
LD_LIBRARY_PATH=$L ./usr/local/bin/python3 -I -B -S -c '
import os, sys
sp = "./usr/local/lib/python3.14/site-packages"
if not os.path.isdir(sp):
    sys.exit(0)
emptied = set()
for d, dirs, files in os.walk(sp, topdown=False):
    if os.path.basename(d) == "__pycache__":
        src = os.path.dirname(d)
        for f in files:
            if f.endswith(".pyc") and not os.path.exists(os.path.join(src, f.split(".", 1)[0] + ".py")):
                os.remove(os.path.join(d, f))
                emptied.add(d)
    if d != sp and (d in emptied or os.path.basename(d) == "__pycache__") and not os.listdir(d):
        os.rmdir(d)
        emptied.add(os.path.dirname(d))
' || exit 1
exit 0
